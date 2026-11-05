/*
 * sony_http.c — Sony Camera Remote API over HTTP. See sony_http.h.
 *
 * Ported from Alpha Fairy (github.com/frank26080115/alpha-fairy, commit
 * fd1f298), arduino_workspace/libraries/SonyHttpCamera/:
 *   SonyHttpCameraInit.cpp  ssdp_start, ssdp_checkurl, get_dd_xml, parse_dd_xml
 *   SonyHttpCamera.cpp      the init sequence in task(), parse_event, get_event
 *   SonyHttpCameraCmds.cpp  the command formats and cmd_* calls
 *   SonyHttpCameraUtils.cpp scan_json_for_key, parse_json_err_num, strcpy_no_slash
 * Copyright (c) Frank Zhao, MIT License (third_party/alpha-fairy/LICENSE).
 *
 * Licensed under the Apache License, Version 2.0 (the ported parts: MIT)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

#include "forth_core.h"
#include "console.h"
#include "net_words.h"
#include "sony_http.h"

static const char *TAG = "shc";

#define SHC_REPLY_CAP   (12 * 1024)    /* getEvent replies run 5-10 KB */
#define SHC_URL_CAP     160

static char        s_reply[SHC_REPLY_CAP];
static int         s_reply_len;
static bool        s_reply_trunc;
static shc_state_t s_state = SHC_OFF;
static char        s_model[64] = "-";
static char        s_service_url[SHC_URL_CAP];   /* .../sony/camera */
static char        s_liveview_url[SHC_URL_CAP];
static char        s_postview_url[SHC_URL_CAP];
static unsigned    s_req_id = 1;
static int         s_event_version = 3;            /* getEvent 1.3, stepped down on error 14 */
static bool        s_dbg;

shc_state_t shc_state(void)    { return s_state; }
const char *shc_model(void)    { return s_model; }
const char *shc_postview(void) { return s_postview_url; }
const char *shc_liveview(void) { return s_liveview_url; }
const char *shc_reply(void)    { return s_reply; }

const char *shc_state_name(void)
{
    static const char *n[] = { "off", "join", "api", "ready", "err" };
    if (s_state == SHC_READY || s_state == SHC_ERR) return n[s_state];
    if (!net_ok()) return "join";
    return s_service_url[0] ? "api" : "off";
}

/* ------------------------------------------------------------------ */
/* JSON helpers (SonyHttpCameraUtils.cpp)                              */
/* ------------------------------------------------------------------ */

/* scan_json_for_key, kept close to the original; bounds checks added. */
bool shc_scan_json(const char *data, int len, const char *key, int *start, int *end, char *tgt, int tgtlen)
{
    int slen = len > 0 ? len : (int)strlen(data);
    int sidx = -1, eidx = -1;
    int keylen = (int)strlen(key);
    bool found = false;
    if (start) *start = -1;
    if (end) *end = -1;

    for (int i = 0; i + keylen + 1 < slen && !found; i++) {
        if (data[i] != '"' || data[i + keylen + 1] != '"' || memcmp(&data[i + 1], key, keylen) != 0) continue;
        i += keylen + 2;
        char c = data[i], prev_c = c;
        for (; i < slen && c != 0 && !found; i++) {
            c = data[i];
            if (c == ':') {
                i++;
                sidx = i;
                for (; i < slen && c != 0 && !found; i++) {
                    c = data[i];
                    if (c == '"') {
                        i++; sidx = i; prev_c = c;
                        for (; i < slen && c != 0 && !found; i++) {
                            c = data[i];
                            if (c == '"' && prev_c != '\\') { eidx = i - 1; found = true; }
                            prev_c = c;
                        }
                    } else if (c == '[' || c == '{') {
                        char close = c == '[' ? ']' : '}';
                        bool in_quote = false;
                        int depth = 0;
                        i++; sidx = i; prev_c = c;
                        for (; i < slen && c != 0 && !found; i++) {
                            c = data[i];
                            if (c == '"' && prev_c != '\\') in_quote = !in_quote;
                            else if (!in_quote && (c == '[' || c == '{')) depth++;
                            else if (!in_quote && (c == ']' || c == '}')) {
                                if (depth == 0 && c == close) { eidx = i - 1; found = true; }
                                else depth--;
                            }
                            prev_c = c;
                        }
                    } else if (c == ',' || c == '}' || c == ']') {
                        eidx = i - 1; found = true;
                    } else if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                        if (i == sidx) sidx = i + 1;
                    }
                    prev_c = c;
                }
            } else if (c == ',') {
                break;
            } else if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
                break;
            }
            prev_c = c;
        }
        if (!found) { sidx = eidx = -1; }
    }
    if (!found || sidx < 0 || eidx < sidx - 1) return false;
    if (start) *start = sidx;
    if (end) *end = eidx;
    if (tgt && tgtlen > 0) {
        int j = 0;
        for (int i = sidx; i <= eidx && j < tgtlen - 1; i++, j++) tgt[j] = data[i];
        tgt[j] = 0;
    }
    return true;
}

/* parse_json_err_num: the first run of digits in the "error" value */
static bool err_num(const char *s, int *out)
{
    char tmp[16]; int j = 0;
    for (int i = 0; s[i] && j < 15; i++) {
        if (s[i] >= '0' && s[i] <= '9') tmp[j++] = s[i];
        else if (j > 0) break;
    }
    tmp[j] = 0;
    if (!j) return false;
    *out = atoi(tmp);
    return true;
}

/* strcpy_no_slash: 2\" -> 2" for display */
static void copy_no_slash(char *dst, const char *src, size_t cap)
{
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 1 < cap; i++) if (src[i] != '\\') dst[j++] = src[i];
    dst[j] = 0;
}

/* ------------------------------------------------------------------ */
/* HTTP                                                                */
/* ------------------------------------------------------------------ */

static esp_err_t on_http(esp_http_client_event_t *e)
{
    if (e->event_id != HTTP_EVENT_ON_DATA) return ESP_OK;
    int room = SHC_REPLY_CAP - 1 - s_reply_len;
    int n = e->data_len < room ? e->data_len : room;
    if (n > 0) { memcpy(s_reply + s_reply_len, e->data, n); s_reply_len += n; }
    if (e->data_len > room) s_reply_trunc = true;
    return ESP_OK;
}

/* GET when body is NULL, POST JSON otherwise. Returns the HTTP status, -1 on no reply. */
static int http_do(const char *url, const char *body, int timeout_ms)
{
    s_reply_len = 0; s_reply_trunc = false; s_reply[0] = 0;
    if (!net_ok()) return -1;
    esp_http_client_config_t cfg = {
        .url           = url,
        .method        = body ? HTTP_METHOD_POST : HTTP_METHOD_GET,
        .event_handler = on_http,
        .timeout_ms    = timeout_ms,
        .buffer_size   = 1024,
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) return -1;
    if (body) {
        esp_http_client_set_header(c, "Content-Type", "application/json");
        esp_http_client_set_post_field(c, body, (int)strlen(body));
    }
    if (s_dbg) con_printf("dbg > %s %s\n", url, body ? body : "");
    esp_err_t err = esp_http_client_perform(c);
    int status = esp_http_client_get_status_code(c);
    esp_http_client_cleanup(c);
    s_reply[s_reply_len] = 0;
    if (s_dbg) con_printf("dbg < %d %d B%s %.160s\n", status, s_reply_len, s_reply_trunc ? " (cut)" : "", s_reply);
    if (err != ESP_OK) { ESP_LOGW(TAG, "%s: %s", url, esp_err_to_name(err)); return -1; }
    return status;
}

/* ------------------------------------------------------------------ */
/* Discovery (SonyHttpCameraInit.cpp)                                  */
/* ------------------------------------------------------------------ */

/* ssdp_start + ssdp_checkurl: M-SEARCH, cycling the queries as Alpha Fairy
 * does, and take the LOCATION of the first reply. */
static bool ssdp_find(char *location, size_t cap)
{
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) return false;
    struct timeval tv = { .tv_sec = 1, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
    uint8_t ttl = 2;
    setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof ttl);

    struct sockaddr_in to = { .sin_family = AF_INET, .sin_port = htons(1900) };
    to.sin_addr.s_addr = inet_addr("239.255.255.250");
    static const char *queries[] = {
        "MX: 1\r\nST: urn:schemas-sony-com:service:ScalarWebAPI:1\r\nUSER-AGENT: xyz/1.0 abc/1.0\r\n",
        "MX: 1\r\nST: urn:dial-multiscreen-org:service:dial:1\r\nUSER-AGENT: Google Chrome/1.0 Windows\r\n",
        "MX: 1\r\nST: urn:schemas-sony-com:service:ScalarWebAPI:1\r\nUSER-AGENT: xyz/1.0 abc/1.0\r\n",
        "ST:ssdp:all\r\nMX: 1\r\n",
    };
    char pkt[256], rx[768];
    bool got = false;
    for (int t = 0; t < 8 && !got; t++) {
        int n = snprintf(pkt, sizeof pkt, "M-SEARCH * HTTP/1.1\r\nHOST: 239.255.255.250:1900\r\nMAN: \"ssdp:discover\"\r\n%s\r\n", queries[t % 4]);
        sendto(sock, pkt, n, 0, (struct sockaddr *)&to, sizeof to);
        for (;;) {
            int r = recv(sock, rx, sizeof rx - 1, 0);
            if (r <= 0) break;
            rx[r] = 0;
            for (char *line = rx; line && *line; ) {
                char *nl = strchr(line, '\n');
                if (strncasecmp(line, "LOCATION:", 9) == 0) {
                    char *u = strstr(line, "http");
                    if (u) {
                        size_t k = 0;
                        while (u[k] && u[k] != '\r' && u[k] != '\n' && k + 1 < cap) { location[k] = u[k]; k++; }
                        location[k] = 0;
                        got = true;
                    }
                }
                line = nl ? nl + 1 : NULL;
            }
            if (got) break;
        }
    }
    close(sock);
    return got;
}

/* copy what follows key up to the next "</" */
static bool xml_after(const char *from, const char *key, char *out, size_t cap)
{
    const char *p = strstr(from, key);
    if (!p) return false;
    p += strlen(key);
    size_t k = 0;
    while (p[k] && !(p[k] == '<' && p[k + 1] == '/') && k + 1 < cap) { out[k] = p[k]; k++; }
    out[k] = 0;
    return k > 0;
}

/* parse_dd_xml: friendlyName, the camera service's ActionList_URL, liveview */
static bool parse_dd(const char *xml)
{
    if (!xml_after(xml, "<friendlyName>", s_model, sizeof s_model)) strcpy(s_model, "camera");
    const char *svc = strstr(xml, "X_ScalarWebAPI_ServiceType>camera</");
    char base[SHC_URL_CAP - 8];
    if (!svc || !xml_after(svc, "ActionList_URL>", base, sizeof base)) return false;
    snprintf(s_service_url, sizeof s_service_url, "%s/camera", base);
    s_liveview_url[0] = 0;
    xml_after(xml, "LiveView_URL>", s_liveview_url, sizeof s_liveview_url);
    return true;
}

/* get_dd_xml with Alpha Fairy's fallbacks when SSDP stays quiet */
static bool fetch_dd(const char *location)
{
    char url[SHC_URL_CAP];
    uint32_t gw = net_gw4();
    char ip[16];
    snprintf(ip, sizeof ip, "%u.%u.%u.%u", (unsigned)(gw & 255), (unsigned)((gw >> 8) & 255), (unsigned)((gw >> 16) & 255), (unsigned)(gw >> 24));
    for (int t = 0; t < 4; t++) {
        if (location && location[0] && t == 0) snprintf(url, sizeof url, "%s", location);
        else if (t % 2) snprintf(url, sizeof url, "http://%s:64321/dd.xml", ip);
        else snprintf(url, sizeof url, "http://%s:61000/scalarwebapi_dd.xml", ip);
        if (http_do(url, NULL, 4000) == 200 && parse_dd(s_reply)) return true;
    }
    return false;
}

/* ------------------------------------------------------------------ */
/* Calls                                                               */
/* ------------------------------------------------------------------ */

/* One talker to the camera at a time: the console (lens) and the button
 * shoot from different tasks and share s_reply. */
static SemaphoreHandle_t s_cam_lock;
static volatile int      s_shots;

void shc_lock(void)
{
    if (!s_cam_lock) s_cam_lock = xSemaphoreCreateRecursiveMutex();
    xSemaphoreTakeRecursive(s_cam_lock, portMAX_DELAY);
}
void shc_unlock(void) { xSemaphoreGiveRecursive(s_cam_lock); }
int  shc_shots(void)  { return s_shots; }

int shc_call(const char *method, const char *params, const char *version)
{
    if (!s_service_url[0]) return -1;
    shc_lock();
    char body[256];
    snprintf(body, sizeof body, "{\"method\": \"%s\", \"params\": [%s], \"id\": %u, \"version\": \"%s\"}",
             method, params ? params : "", s_req_id++, version ? version : "1.0");
    int timeout = strcmp(method, "actTakePicture") == 0 || strcmp(method, "awaitTakePicture") == 0 ? 15000 : 5000;
    int st = http_do(s_service_url, body, timeout);
    if (st < 0) { shc_unlock(); return -1; }
    char e[48];
    int code = 0;
    if (shc_scan_json(s_reply, s_reply_len, "error", NULL, NULL, e, sizeof e)) {
        if (!err_num(e, &code)) code = 1;
    } else if (st != 200) {
        code = st;
    }
    shc_unlock();
    return code;
}

/* The integer after "key": (the first one in the reply). Numbers are not
 * strings, so shc_scan_json can't read them. */
static bool json_int(const char *key, int *out)
{
    char pat[48];
    snprintf(pat, sizeof pat, "\"%s\"", key);
    const char *p = strstr(s_reply, pat);
    if (!p) return false;
    p += strlen(pat);
    while (*p == ' ' || *p == ':') p++;
    if (*p != '-' && (*p < '0' || *p > '9')) return false;
    *out = (int)strtol(p, NULL, 10);
    return true;
}

/* The settings cam-step can move, in its numbering. type: the getEvent
 * entry's "type"; cand/cur: its keys; how the setter wants the value. */
enum { P_STR, P_NUM, P_DRIVE, P_WB };
static const struct step_def {
    const char *name, *type, *cur, *cand, *set;
    int param;
} STEPS[] = {
    { "ss",    "shutterSpeed",     "currentShutterSpeed", "shutterSpeedCandidates", "setShutterSpeed",  P_STR },
    { "f",     "fNumber",          "currentFNumber",      "fNumberCandidates",      "setFNumber",       P_STR },
    { "iso",   "isoSpeedRate",     "currentIsoSpeedRate", "isoSpeedRateCandidates", "setIsoSpeedRate",  P_STR },
    { "ev",    "exposureCompensation", NULL, NULL,                                  "setExposureCompensation", P_NUM },
    { "wb",    "whiteBalance",     NULL, NULL,                                      "setWhiteBalance",  P_WB },
    { "drive", "contShootingMode", "contShootingMode",    "candidate",              "setContShootingMode", P_DRIVE },
    { "timer", "selfTimer",        "currentSelfTimer",    "selfTimerCandidates",    "setSelfTimer",     P_NUM },
    { "focus", "focusMode",        "currentFocusMode",    "focusModeCandidates",    "setFocusMode",     P_STR },
};
#define STEP_N ((int)(sizeof STEPS / sizeof STEPS[0]))

static bool has_api(const char *method)
{
    char pat[48];
    snprintf(pat, sizeof pat, "\"%s\"", method);
    return strstr(s_reply, pat) != NULL;
}

/* "key":[ with something before the ] */
static bool has_values(const char *key)
{
    char pat[48];
    snprintf(pat, sizeof pat, "\"%s\":[", key);
    const char *p = strstr(s_reply, pat);
    return p && p[strlen(pat)] != ']';
}

/* the getEvent object whose "type" is t: [*s, *e] in s_reply */
static bool event_obj(const char *t, int *s, int *e)
{
    char pat[48];
    snprintf(pat, sizeof pat, "\"type\":\"%s\"", t);
    const char *p = strstr(s_reply, pat);
    if (!p) return false;
    int i = p - s_reply, depth = 0;
    for (; i >= 0; i--) {                    /* back to this object's { */
        if (s_reply[i] == '}') depth++;
        else if (s_reply[i] == '{') { if (depth == 0) break; depth--; }
    }
    if (i < 0) return false;
    *s = i;
    depth = 0;
    for (int j = i; j < s_reply_len; j++) {
        if (s_reply[j] == '{') depth++;
        else if (s_reply[j] == '}' && --depth == 0) { *e = j; return true; }
    }
    return false;
}

/* the raw token after "key": between s and e (a string without its quotes,
 * escapes kept; or a number) */
static bool raw_value(int s, int e, const char *key, char *out, size_t cap)
{
    char pat[48];
    snprintf(pat, sizeof pat, "\"%s\":", key);
    size_t pl = strlen(pat);
    for (int i = s; i + (int)pl <= e; i++) {
        if (memcmp(s_reply + i, pat, pl) != 0) continue;
        int j = i + pl;
        while (j <= e && s_reply[j] == ' ') j++;
        size_t k = 0;
        if (s_reply[j] == '"') {
            j++;
            while (j <= e && !(s_reply[j] == '"' && s_reply[j - 1] != '\\') && k + 1 < cap) out[k++] = s_reply[j++];
        } else {
            while (j <= e && s_reply[j] != ',' && s_reply[j] != '}' && s_reply[j] != ']' && k + 1 < cap) out[k++] = s_reply[j++];
        }
        out[k] = 0;
        return k > 0;
    }
    return false;
}

/* get_event + parse_event: refresh, print the keys we know */
static int get_event(bool print)
{
    char v[8];
    int code;
    for (;;) {
        snprintf(v, sizeof v, "1.%d", s_event_version);
        code = shc_call("getEvent", "false", v);
        if (code == 14 && s_event_version > 0) { s_event_version--; continue; }   /* unsupported version */
        break;
    }
    if (code != 0 || !print) return code;
    static const struct { const char *json, *key; } keys[] = {
        { "cameraStatus", "status" }, { "currentExposureMode", "mode" }, { "currentFNumber", "f" },
        { "currentShutterSpeed", "ss" }, { "currentIsoSpeedRate", "iso" },
        { "currentFocusMode", "focus" }, { "focusStatus", "af" }, { "currentShootMode", "shoot" },
        /* for the camera-screen layout on the glasses */
        { "currentWhiteBalanceMode", "wb" }, { "currentAspect", "aspect" }, { "stillQuality", "quality" },
        { "contShootingMode", "drive" }, { "storageID", "card" },
    };
    char val[40], clean[40];
    for (size_t i = 0; i < sizeof keys / sizeof keys[0]; i++) {
        if (!shc_scan_json(s_reply, s_reply_len, keys[i].json, NULL, NULL, val, sizeof val) || !val[0]) continue;
        copy_no_slash(clean, val, sizeof clean);
        con_printf("evt %s %s\n", keys[i].key, clean);
    }
    /* numbers: EV is an index in steps (stepIndex 1 = 1/3 EV), battery a fraction */
    static const struct { const char *json, *key; } nums[] = {
        { "currentExposureCompensation", "ev" }, { "minExposureCompensation", "evmin" },
        { "maxExposureCompensation", "evmax" }, { "stepIndexOfExposureCompensation", "evstep" },
        { "numberOfRecordableImages", "left" },
    };
    int n, d;
    for (size_t i = 0; i < sizeof nums / sizeof nums[0]; i++)
        if (json_int(nums[i].json, &n)) con_printf("evt %s %d\n", nums[i].key, n);
    if (json_int("levelNumer", &n) && json_int("levelDenom", &d) && n >= 0 && d > 0)
        con_printf("evt bat %d\n", n * 100 / d);
    if (json_int("currentSelfTimer", &n)) con_printf("evt timer %d\n", n);
    /* What the remote may change now: the body lists the setter in
     * availableApiList AND offers values. (The α7R III sends fNumber with no
     * candidates in S mode, and drops setExposureCompensation while the EV
     * dial is off 0.) */
    char can[96]; int cl = 0;
    can[0] = 0;
    for (int i = 0; i < STEP_N; i++) {
        bool ok = has_api(STEPS[i].set);
        if (ok && STEPS[i].cand) ok = has_values(STEPS[i].cand);
        if (ok) cl += snprintf(can + cl, sizeof can - cl, " %s", STEPS[i].name);
    }
    if (has_api("actHalfPressShutter")) cl += snprintf(can + cl, sizeof can - cl, " af");
    con_printf("evt can%s\n", can);
    if (s_reply_trunc) con_printf("evt cut %d\n", s_reply_len);
    return 0;
}

/* One notch of a setting (STEPS[which]) among the values the body offers
 * now: getEvent's candidate lists (white balance: getAvailableWhiteBalance).
 * now gets the value it is at afterwards. */
static int cam_step(int which, int dir, char *now, size_t cap)
{
    const struct step_def *d = &STEPS[which];
    now[0] = 0;
    shc_lock();
    int code = get_event(false);
    if (code) goto done;
    if (!has_api(d->set)) { code = -2; goto done; }

    if (which == 3) {                                  /* EV: an index between min and max */
        int cur, lo, hi;
        if (!json_int("currentExposureCompensation", &cur) || !json_int("minExposureCompensation", &lo) ||
            !json_int("maxExposureCompensation", &hi)) { code = -2; goto done; }
        int next = cur + dir;
        if (next < lo) next = lo;
        if (next > hi) next = hi;
        char p[16];
        snprintf(p, sizeof p, "%d", next);
        if (next != cur) {
            code = shc_call("setExposureCompensation", p, NULL);
            if (code == 3) { snprintf(p, sizeof p, "\"%d\"", next); code = shc_call("setExposureCompensation", p, NULL); }
        }
        if (code == 0) snprintf(now, cap, "%d", next);
        goto done;
    }

    static char vals[64][24];                          /* under the camera lock */
    char cur[24] = "";
    int nv = 0, at = -1;
    if (d->param == P_WB) {
        /* [{"whiteBalanceMode":"Auto WB",...}, [{"whiteBalanceMode":...}, ...]]:
         * the first is the current one; Color Temperature needs a K, skipped */
        code = shc_call("getAvailableWhiteBalance", "", NULL);
        if (code) goto done;
        const char *p = s_reply;
        bool first = true;
        while ((p = strstr(p, "\"whiteBalanceMode\":\"")) && nv < 64) {
            p += 20;
            size_t k = 0;
            char v[24];
            while (*p && *p != '"' && k + 1 < sizeof v) v[k++] = *p++;
            v[k] = 0;
            if (first) { strcpy(cur, v); first = false; continue; }
            if (strcmp(v, "Color Temperature") == 0) continue;
            strcpy(vals[nv], v);
            if (strcmp(v, cur) == 0) at = nv;
            nv++;
        }
    } else {
        int s, e;
        if (!event_obj(d->type, &s, &e) || !raw_value(s, e, d->cur, cur, sizeof cur)) { code = -2; goto done; }
        char pat[48];
        snprintf(pat, sizeof pat, "\"%s\":[", d->cand);
        char *q = strstr(s_reply + s, pat);
        if (!q || q - s_reply > e) { code = -2; goto done; }
        int i = q - s_reply + strlen(pat);
        while (i <= e && s_reply[i] != ']' && nv < 64) {
            if (s_reply[i] == '"') {
                int j = ++i;
                while (j <= e && !(s_reply[j] == '"' && s_reply[j - 1] != '\\')) j++;
                int len = j - i;
                if (len > 0 && len < (int)sizeof vals[0]) { memcpy(vals[nv], s_reply + i, len); vals[nv][len] = 0; nv++; }
                i = j + 1;
            } else if (s_reply[i] == '-' || (s_reply[i] >= '0' && s_reply[i] <= '9')) {
                int j = i;
                while (j <= e && s_reply[j] != ',' && s_reply[j] != ']') j++;
                int len = j - i;
                if (len < (int)sizeof vals[0]) { memcpy(vals[nv], s_reply + i, len); vals[nv][len] = 0; nv++; }
                i = j;
            } else {
                i++;
            }
        }
        for (int k = 0; k < nv; k++) if (strcmp(vals[k], cur) == 0) at = k;
    }
    if (nv == 0) { code = -2; goto done; }
    if (at < 0) at = dir > 0 ? -1 : nv;               /* not in the list (e.g. Color Temperature): start at an end */
    int next = at + dir;
    if (next < 0) next = 0;
    if (next >= nv) next = nv - 1;
    if (next != at) {
        char p[64];
        switch (d->param) {
        case P_NUM:   snprintf(p, sizeof p, "%s", vals[next]); break;
        case P_DRIVE: snprintf(p, sizeof p, "{\"contShootingMode\": \"%s\"}", vals[next]); break;
        case P_WB:    snprintf(p, sizeof p, "\"%s\", false, -1", vals[next]); break;
        default:      snprintf(p, sizeof p, "\"%s\"", vals[next]); break;
        }
        code = shc_call(d->set, p, NULL);
    }
    if (code == 0) copy_no_slash(now, vals[next], cap);
done:
    shc_unlock();
    return code;
}

/* AF as the shutter's half-press: focus, read focusStatus, let go (in AF-S
 * the lens stays where it focused). -> af <status> | af err <code> */
static void w_cam_af(void)
{
    shc_lock();
    int code = shc_call("actHalfPressShutter", "", NULL);
    if (code) { shc_unlock(); con_printf("af err %d\n", code); return; }
    char st[24] = "?";
    for (int t = 0; t < 8; t++) {                      /* up to ~2 s for Focused / Failed */
        vTaskDelay(pdMS_TO_TICKS(250));
        if (get_event(false) == 0 && shc_scan_json(s_reply, s_reply_len, "focusStatus", NULL, NULL, st, sizeof st) &&
            (strcmp(st, "Focused") == 0 || strcmp(st, "Failed") == 0)) break;
    }
    shc_call("cancelHalfPressShutter", "", NULL);
    shc_unlock();
    con_printf("af %s\n", st);
}

bool shc_connect(void)
{
    s_state = SHC_JOIN;
    s_service_url[0] = 0;
    if (!net_ok()) { con_printf("cam join no-wifi\n"); s_state = SHC_ERR; return false; }
    char loc[SHC_URL_CAP] = "";
    bool ssdp = ssdp_find(loc, sizeof loc);
    if (!fetch_dd(ssdp ? loc : NULL)) { con_printf("cam err no-dd%s\n", ssdp ? "" : " no-ssdp"); s_state = SHC_ERR; return false; }
    s_state = SHC_API;
    /* Alpha Fairy goes straight to startRecMode; some bodies need it, the
     * rest answer with an error that is safe to ignore. */
    int code = -1;
    for (int t = 0; t < 3 && code != 0; t++) code = shc_call("startRecMode", "", NULL);
    if (code != 0) ESP_LOGW(TAG, "startRecMode: %d", code);
    /* The α7R III (Smart Remote Control 2.1.7) shuts its AP about 5 s after
     * a client joins unless liveview is started, the way Imaging Edge Mobile
     * always does. The stream itself need not be read. */
    code = shc_call("startLiveview", "", NULL);
    if (code == 0) shc_scan_json(s_reply, s_reply_len, "result", NULL, NULL, s_liveview_url, sizeof s_liveview_url);
    else ESP_LOGW(TAG, "startLiveview: %d", code);
    vTaskDelay(pdMS_TO_TICKS(500));
    code = get_event(false);
    s_state = code == 0 ? SHC_READY : SHC_ERR;
    con_printf("cam %s %s %d\n", shc_state_name(), s_model, net_rssi());
    return s_state == SHC_READY;
}

/* ------------------------------------------------------------------ */
/* Forth words                                                         */
/* ------------------------------------------------------------------ */

static void reply(int code)
{
    if (code == 0) con_printf("ok\n");
    else con_printf("err %d\n", code);
}

/* pull a Forth string ( addr u -- ) into buf */
static bool pop_str(char *buf, size_t cap)
{
    int u = (int)forth_pop();
    const char *a = (const char *)forth_pop();
    if (!a || u <= 0 || (size_t)u >= cap) { buf[0] = 0; return false; }
    memcpy(buf, a, u); buf[u] = 0;
    return true;
}

static void w_cam_join(void) { shc_connect(); }

/* Once a second from the housekeeping task: with Wi-Fi up and the camera not
 * linked, join again every 10 s, so a camera that slept or dropped us comes
 * back without the button. A link whose Wi-Fi went away is marked off. */
void shc_tick(void)
{
    static int wait;
    if (!net_ok()) {
        if (s_state == SHC_READY) { s_state = SHC_OFF; con_printf("cam off (wifi lost)\n"); }
        wait = 0;
        return;
    }
    if (s_state == SHC_READY || s_state == SHC_JOIN || s_state == SHC_API) { wait = 0; return; }
    if (++wait < 10) return;
    wait = 0;
    shc_connect();
}
static void w_cam_q(void)    { con_printf("cam %s %s %d\n", shc_state_name(), s_model, net_rssi()); }
static void w_cam_dbg(void)  { s_dbg = forth_pop() != 0; }

/* the whole last reply, 100 characters a line (to learn a body's getEvent) */
static void w_cam_dump(void)
{
    for (int i = 0; i < s_reply_len; i += 100) con_printf("raw %.100s\n", s_reply + i);
    con_printf("raw %d B%s\n", s_reply_len, s_reply_trunc ? " cut" : "");
}

static void w_cam_evt(void)
{
    int code = get_event(true);
    if (code) reply(code);
}

/* print the method names, a few per line so a BLE line stays short */
static void w_cam_api(void)
{
    int code = shc_call("getAvailableApiList", "", NULL);
    if (code) { reply(code); return; }
    int s, e;
    if (!shc_scan_json(s_reply, s_reply_len, "result", &s, &e, NULL, 0)) { con_printf("err no-result\n"); return; }
    char line[120]; int ll = 0, count = 0;
    ll = snprintf(line, sizeof line, "api");
    for (int i = s; i <= e; i++) {
        if (s_reply[i] != '"') continue;
        int j = i + 1;
        while (j <= e && s_reply[j] != '"') j++;
        int n = j - i - 1;
        if (n > 0 && n < 60) {
            if (ll + n + 2 >= (int)sizeof line) { con_printf("%s\n", line); ll = snprintf(line, sizeof line, "api"); }
            line[ll++] = ' '; memcpy(line + ll, s_reply + i + 1, n); ll += n; line[ll] = 0;
            count++;
        }
        i = j;
    }
    con_printf("%s\napi %d methods\n", line, count);
}

static void w_cam_rpc(void)
{
    char m[48];
    if (!pop_str(m, sizeof m)) { con_printf("? cam-rpc ( addr u -- )\n"); return; }
    int code = shc_call(m, "", NULL);
    con_printf("rpc %d %.200s\n", code, s_reply);
}

/* In Continuous drive the camera drops actTakePicture from its API list
 * (the call fails); it still offers setContShootingMode, so switch the
 * drive to Single and try once more. The drive stays Single. */
/* The whole shot, from the console (shoot) or AlphaFriend's A button. */
int shc_shoot(bool verbose)
{
    shc_lock();
    /* Half-press first: on the α7R III in AF-S a bare actTakePicture failed
     * with 40400 every time, and af then shoot worked every time (even when
     * the focus reported Failed). */
    int hp = shc_call("actHalfPressShutter", "", NULL);
    if (hp == 0) vTaskDelay(pdMS_TO_TICKS(1000));
    int code = shc_call("actTakePicture", "", NULL);
    if (code != 0 && code != 40403 &&
        shc_call("setContShootingMode", "{\"contShootingMode\": \"Single\"}", NULL) == 0) {
        if (verbose) con_printf("drive Single\n");
        /* the first try right after the switch can still fail (40400) */
        for (int t = 0; t < 3; t++) {
            vTaskDelay(pdMS_TO_TICKS(t ? 1000 : 500));
            code = shc_call("actTakePicture", "", NULL);
            if (code != 40400) break;
        }
    }
    if (code == 40403) code = shc_call("awaitTakePicture", "", NULL);   /* long exposure still running */
    if (code == 0) {
        const char *u = strstr(s_reply, "http");
        size_t k = 0;
        if (u) while (u[k] && u[k] != '"' && k + 1 < sizeof s_postview_url) { s_postview_url[k] = u[k]; k++; }
        s_postview_url[k] = 0;
        /* A shot ends liveview, and without liveview the α7R III drops the
         * client and shuts its AP about 10 s later (seen 2026-11-04). */
        int lv = shc_call("startLiveview", "", NULL);
        if (lv != 0) ESP_LOGW(TAG, "startLiveview after shot: %d", lv);
    }
    if (hp == 0) shc_call("cancelHalfPressShutter", "", NULL);
    if (code == 0) s_shots++;
    shc_unlock();
    return code;
}

static void w_shoot(void)  { reply(shc_shoot(true)); }

/* cam-step ( which dir -- )  which 0 ss 1 f 2 iso 3 ev 4 wb 5 drive 6 timer 7 focus; dir -1 or 1
 *   -> step <name> <value now> | step <name> err <code> (-2 not settable now) */
static void w_cam_step(void)
{
    int dir = (int)forth_pop(), which = (int)forth_pop();
    if (which < 0 || which >= STEP_N || (dir != 1 && dir != -1)) { con_printf("? cam-step ( which dir -- ) 0 ss 1 f 2 iso 3 ev 4 wb 5 drive 6 timer 7 focus\n"); return; }
    char now[24];
    int code = cam_step(which, dir, now, sizeof now);
    if (code) con_printf("step %s err %d\n", STEPS[which].name, code);
    else con_printf("step %s %s\n", STEPS[which].name, now);
}
static void w_shots_q(void) { con_printf("shots %d\n", s_shots); }

static void w_af(void)     { reply(shc_call("actHalfPressShutter", "", NULL)); }
static void w_af_off(void) { reply(shc_call("cancelHalfPressShutter", "", NULL)); }
static void w_rec(void)    { reply(shc_call("startMovieRec", "", NULL)); }
static void w_rec_off(void){ reply(shc_call("stopMovieRec", "", NULL)); }

/* 56 f! -> "5.6"; 80 -> "8.0"; 110 -> "11" (Sony's own candidate strings) */
static void w_f_store(void)
{
    int x = (int)forth_pop();
    char p[16];
    if (x <= 0 || x > 640) { con_printf("? f! ( x10 -- ) 56 is f/5.6\n"); return; }
    if (x < 100) snprintf(p, sizeof p, "\"%d.%d\"", x / 10, x % 10);
    else snprintf(p, sizeof p, "\"%d\"", x / 10);
    reply(shc_call("setFNumber", p, NULL));
}

/* s" 1/250" ss!   s" 2s" ss! (Forth strings can't hold a quote: 2s is 2\") */
static void w_ss_store(void)
{
    char s[16], p[24];
    if (!pop_str(s, sizeof s)) { con_printf("? ss! ( addr u -- )\n"); return; }
    size_t n = strlen(s);
    if (n > 1 && s[n - 1] == 's') { s[n - 1] = 0; snprintf(p, sizeof p, "\"%s\\\"\"", s); }
    else snprintf(p, sizeof p, "\"%s\"", s);
    reply(shc_call("setShutterSpeed", p, NULL));
}

static void w_iso_store(void)
{
    int x = (int)forth_pop();
    char p[16];
    if (x <= 0) snprintf(p, sizeof p, "\"AUTO\"");
    else snprintf(p, sizeof p, "\"%d\"", x);
    reply(shc_call("setIsoSpeedRate", p, NULL));
}

/* Sony's reference takes an int index; Alpha Fairy sends it as a string.
 * Try the int, fall back to the string on "illegal argument" (3). */
static void w_ev_store(void)
{
    int x = (int)forth_pop();
    char p[16];
    snprintf(p, sizeof p, "%d", x);
    int code = shc_call("setExposureCompensation", p, NULL);
    if (code == 3) { snprintf(p, sizeof p, "\"%d\"", x); code = shc_call("setExposureCompensation", p, NULL); }
    reply(code);
}

void sony_register_words(void)
{
    forth_register_word("cam-join", w_cam_join);
    forth_register_word("cam?",     w_cam_q);
    forth_register_word("cam-api",  w_cam_api);
    forth_register_word("cam-rpc",  w_cam_rpc);
    forth_register_word("cam-evt",  w_cam_evt);
    forth_register_word("cam-dbg",  w_cam_dbg);
    forth_register_word("cam-dump", w_cam_dump);
    forth_register_word("shoot",    w_shoot);
    forth_register_word("shots?",   w_shots_q);
    forth_register_word("cam-step", w_cam_step);
    forth_register_word("cam-af",   w_cam_af);
    forth_register_word("af",       w_af);
    forth_register_word("af-",      w_af_off);
    forth_register_word("rec",      w_rec);
    forth_register_word("rec-",     w_rec_off);
    forth_register_word("f!",       w_f_store);
    forth_register_word("ss!",      w_ss_store);
    forth_register_word("iso!",     w_iso_store);
    forth_register_word("ev!",      w_ev_store);
}
