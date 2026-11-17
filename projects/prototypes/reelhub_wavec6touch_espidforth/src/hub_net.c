/*
 * hub_net.c — Wi-Fi station with saved networks (Scout 0.7.0's scout_net.c,
 * without the web server), the clock, and HTTP GET. See hub_net.h.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_http_client.h"
#include "esp_netif_sntp.h"
#include "nvs.h"
#include "esp_heap_caps.h"

#include "forth_core.h"
#include "console.h"
#include "hub_net.h"

static const char *TAG = "net";
#define NS "hubnet"

/* ---- NVS ---------------------------------------------------------------- */

static bool kv_str(const char *k, char *out, size_t cap)
{
    nvs_handle_t h; size_t n = cap;
    if (nvs_open(NS, NVS_READONLY, &h) != ESP_OK) return false;
    esp_err_t e = nvs_get_str(h, k, out, &n);
    nvs_close(h);
    return e == ESP_OK;
}

static void kv_str_set(const char *k, const char *v)
{
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_str(h, k, v); nvs_commit(h); nvs_close(h);
}

static int kv_int(const char *k, int def)
{
    nvs_handle_t h; int32_t v = def;
    if (nvs_open(NS, NVS_READONLY, &h) == ESP_OK) { nvs_get_i32(h, k, &v); nvs_close(h); }
    return (int)v;
}

static void kv_int_set(const char *k, int v)
{
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) == ESP_OK) { nvs_set_i32(h, k, v); nvs_commit(h); nvs_close(h); }
}

/* ---- Wi-Fi station: up to MAX_NETS saved networks ----------------------
 * Saved in NVS as w0s/w0p .. w4s/w4p, in order of preference. Scout scans,
 * tries the saved networks it can see (first saved first), two attempts
 * each, then scans again with a growing pause (5 s .. 60 s). A network that
 * dropped is tried again before the others. Hidden networks never show in a
 * scan, so when none is seen every saved network is tried in turn.
 * Passwords live only in NVS; wifi? never prints them. */

#define MAX_NETS 5
#define TRIES    2

typedef struct { char ssid[33]; char pass[65]; int rssi; } net_t;   /* rssi 0 = not seen */

static esp_netif_t       *s_netif;
static net_t              s_net[MAX_NETS];
static int                s_nnet;
static int                s_cand[MAX_NETS], s_ncand, s_ci = -1, s_tries;
static int                s_cur = -1;                /* slot we have an IP on */
static bool               s_started, s_up, s_scanning, s_seen, s_quiet;
static int                s_retry_s = 5;
static esp_ip4_addr_t     s_ip;
static esp_timer_handle_t s_retry;
static enum { DO_SCAN, DO_CONNECT } s_next;
#define SEEN_MAX 12
static struct { char ssid[33]; int8_t rssi; uint8_t ch, open; } s_seen_ap[SEEN_MAX];
static int                s_nseen;

static void (*s_on_up)(void);
static bool s_sntp;

/* the clock, for the shots' times: SNTP once Wi-Fi is up, TZ from NVS */
static void clock_start(void)
{
    if (s_sntp) return;
    char tz[48] = "UTC0";
    kv_str("tz", tz, sizeof tz);
    setenv("TZ", tz, 1); tzset();
    esp_sntp_config_t c = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&c);
    s_sntp = true;
}

static void nets_load(void)
{
    char k[4] = "w0s";
    s_nnet = 0;
    for (int i = 0; i < MAX_NETS; i++) {
        net_t *n = &s_net[s_nnet];
        memset(n, 0, sizeof *n);
        k[1] = '0' + i; k[2] = 's';
        if (!kv_str(k, n->ssid, sizeof n->ssid) || !n->ssid[0]) continue;
        k[2] = 'p';
        kv_str(k, n->pass, sizeof n->pass);
        s_nnet++;
    }
}

static void nets_save(void)
{
    nvs_handle_t h; char k[4] = "w0s";
    if (nvs_open(NS, NVS_READWRITE, &h) != ESP_OK) return;
    for (int i = 0; i < MAX_NETS; i++) {
        k[1] = '0' + i;
        k[2] = 's'; if (i < s_nnet) nvs_set_str(h, k, s_net[i].ssid); else nvs_erase_key(h, k);
        k[2] = 'p'; if (i < s_nnet) nvs_set_str(h, k, s_net[i].pass); else nvs_erase_key(h, k);
    }
    nvs_commit(h); nvs_close(h);
}

/* 0.6.0 and older kept one network as ssid/pass */
static void nets_migrate(void)
{
    char ssid[33] = "", pass[65] = "", k[4] = "w0s";
    if (kv_str(k, ssid, sizeof ssid) || !kv_str("ssid", ssid, sizeof ssid) || !ssid[0]) return;
    kv_str("pass", pass, sizeof pass);
    kv_str_set("w0s", ssid);
    kv_str_set("w0p", pass);
    nvs_handle_t h;
    if (nvs_open(NS, NVS_READWRITE, &h) == ESP_OK) {
        nvs_erase_key(h, "ssid"); nvs_erase_key(h, "pass"); nvs_commit(h); nvs_close(h);
    }
    ESP_LOGI(TAG, "moved %s into the saved networks", ssid);
}

static void apply_ps(void)
{
    int ps = kv_int("wps", 1);
    esp_err_t e = esp_wifi_set_ps(ps ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE);
    if (e != ESP_OK) ESP_LOGW(TAG, "power save %s refused: %s", ps ? "on" : "off", esp_err_to_name(e));
}

static void apply_creds(const char *ssid, const char *pass)
{
    wifi_config_t c = { 0 };
    strncpy((char *)c.sta.ssid, ssid, sizeof c.sta.ssid - 1);
    strncpy((char *)c.sta.password, pass, sizeof c.sta.password - 1);
    c.sta.threshold.authmode = pass[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    esp_wifi_set_config(WIFI_IF_STA, &c);
}

static void later(int what, int seconds)
{
    s_next = what;
    esp_timer_stop(s_retry);
    esp_timer_start_once(s_retry, (uint64_t)seconds * 1000000ULL);
}

static void scan(void)
{
    if (!s_nnet) return;
    esp_err_t e = esp_wifi_scan_start(NULL, false);
    if (e == ESP_OK) { s_scanning = true; return; }
    ESP_LOGW(TAG, "scan refused: %s", esp_err_to_name(e));
    later(DO_SCAN, s_retry_s);
}

static void try_cand(void)
{
    net_t *n = &s_net[s_cand[s_ci]];
    apply_creds(n->ssid, n->pass);
    ESP_LOGI(TAG, "trying %s (%d of %d)", n->ssid, s_ci + 1, s_ncand);
    esp_wifi_connect();
}

/* the next candidate, or a new scan when they are all used up */
static void next_cand(void)
{
    if (++s_ci < s_ncand) { s_tries = 0; try_cand(); return; }
    s_ci = -1;
    ESP_LOGW(TAG, "no saved network answered, scan again in %d s", s_retry_s);
    later(DO_SCAN, s_retry_s);
    if (s_retry_s < 60) s_retry_s *= 2;
}

static void on_scan_done(void)
{
    uint16_t n = 0;
    if (!s_scanning) { esp_wifi_clear_ap_list(); return; }       /* stopped by wifi! */
    s_scanning = false;
    esp_wifi_scan_get_ap_num(&n);
    if (n > 30) n = 30;
    wifi_ap_record_t *r = n ? calloc(n, sizeof *r) : NULL;
    if (r && esp_wifi_scan_get_ap_records(&n, r) != ESP_OK) n = 0;
    if (!r) { n = 0; esp_wifi_clear_ap_list(); }
    s_nseen = 0;
    for (int j = 0; j < n && s_nseen < SEEN_MAX; j++) {       /* records come strongest first */
        if (!r[j].ssid[0]) continue;
        strncpy(s_seen_ap[s_nseen].ssid, (char *)r[j].ssid, 32);
        s_seen_ap[s_nseen].rssi = r[j].rssi;
        s_seen_ap[s_nseen].ch = r[j].primary;
        s_seen_ap[s_nseen].open = r[j].authmode == WIFI_AUTH_OPEN;
        s_nseen++;
    }
    s_ncand = 0; s_seen = false;
    for (int i = 0; i < s_nnet; i++) {
        s_net[i].rssi = 0;
        for (int j = 0; j < n; j++)
            if (!strcmp((char *)r[j].ssid, s_net[i].ssid) && (!s_net[i].rssi || r[j].rssi > s_net[i].rssi))
                s_net[i].rssi = r[j].rssi;
        if (s_net[i].rssi) { s_cand[s_ncand++] = i; s_seen = true; }
    }
    free(r);
    if (!s_ncand) for (int i = 0; i < s_nnet; i++) s_cand[s_ncand++] = i;   /* hidden? try them all */
    ESP_LOGI(TAG, "scan: %u networks, %d saved seen", n, s_seen ? s_ncand : 0);
    if (!s_ncand) return;
    s_ci = -1;
    next_cand();
}

static void on_retry(void *arg)
{
    if (s_up) return;
    if (s_next == DO_CONNECT && s_ci >= 0) try_cand();
    else scan();
}

static void on_wifi(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        apply_ps();
        scan();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_SCAN_DONE) {
        on_scan_done();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *d = (wifi_event_sta_disconnected_t *)data;
        bool was_up = s_up;
        s_up = false;
        if (s_quiet) { s_quiet = false; return; }          /* wifi! hung up on purpose */
        if (was_up && s_cur >= 0) {
            /* it dropped: that network first, then the scan */
            ESP_LOGW(TAG, "lost %s (reason %d)", s_net[s_cur].ssid, d->reason);
            s_cand[0] = s_cur; s_ncand = 1; s_ci = 0; s_tries = 0; s_cur = -1;
            later(DO_CONNECT, 2);
            return;
        }
        if (s_ci < 0) return;
        ESP_LOGW(TAG, "%s: no (reason %d)", s_net[s_cand[s_ci]].ssid, d->reason);
        if (++s_tries < TRIES) later(DO_CONNECT, 3);
        else next_cand();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_ip = ((ip_event_got_ip_t *)data)->ip_info.ip;
        s_cur = s_ci >= 0 ? s_cand[s_ci] : -1;
        s_retry_s = 5;
        s_seen = true;
        s_up = true;
        ESP_LOGI(TAG, "on %s, ip " IPSTR, s_cur >= 0 ? s_net[s_cur].ssid : "?", IP2STR(&s_ip));
        con_printf("url http://" IPSTR "/ ws://" IPSTR "/ws\n", IP2STR(&s_ip), IP2STR(&s_ip));
        clock_start();
        if (s_on_up) s_on_up();
    }
}

static void wifi_go(void)
{
    if (!s_started) { esp_wifi_start(); s_started = true; }        /* STA_START scans */
}

void hub_net_init(void)
{
    const esp_timer_create_args_t ta = { .callback = on_retry, .name = "wifi_retry" };
    esp_timer_create(&ta, &s_retry);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_netif = esp_netif_create_default_wifi_sta();
    esp_netif_set_hostname(s_netif, "reelhub");
    wifi_init_config_t w = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&w));
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi, NULL);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);

    nets_migrate();
    nets_load();
    if (s_nnet) {
        ESP_LOGI(TAG, "%d saved networks, scanning", s_nnet);
        wifi_go();
    } else {
        ESP_LOGW(TAG, "no Wi-Fi yet: s\" ssid\" s\" pass\" wifi!");
    }
}

bool hub_net_up(void) { return s_up; }
/* should be up: a saved network was in the last scan (so no saved network
 * nearby, as at a venue without the hotspot on, is not a fault to reboot for) */
bool hub_net_configured(void) { return s_started && s_seen; }


/* ---- words -------------------------------------------------------------- */

/* ( ssid u pass u -- ) save a network (a saved one keeps its place and
 * takes the new password; a new one goes last) and join it now */
static void w_wifi_store(void)
{
    int pl = (int)forth_pop(); const char *p = (const char *)forth_pop();
    int sl = (int)forth_pop(); const char *s = (const char *)forth_pop();
    char ssid[33] = "", pass[65] = "";
    while (s && sl > 0 && s[0] == ' ') { s++; sl--; }          /* s"  name" keeps the extra space */
    while (s && sl > 0 && s[sl - 1] == ' ') sl--;
    if (!s || sl <= 0 || sl > 32 || pl < 0 || pl > 64) { con_printf("? wifi! bad args\n"); return; }
    memcpy(ssid, s, sl);
    if (p && pl > 0) memcpy(pass, p, pl);
    int i = 0;
    while (i < s_nnet && strcmp(s_net[i].ssid, ssid)) i++;
    if (i == s_nnet) {
        if (s_nnet == MAX_NETS) { con_printf("? wifi! %d saved already: n wifi-forget\n", MAX_NETS); return; }
        s_nnet++;
        memset(&s_net[i], 0, sizeof s_net[i]);
        strcpy(s_net[i].ssid, ssid);
    }
    strcpy(s_net[i].pass, pass);
    nets_save();
    con_printf("wifi: saved %d %s, connecting\n", i + 1, ssid);
    esp_timer_stop(s_retry);
    s_retry_s = 5;
    s_cand[0] = i; s_ncand = 1; s_ci = 0; s_tries = 0;
    if (!s_started) { wifi_go(); return; }               /* STA_START scans, then tries */
    if (s_scanning) { s_scanning = false; esp_wifi_scan_stop(); }   /* its SCAN_DONE is ignored */
    if (s_up) { s_quiet = true; s_cur = -1; esp_wifi_disconnect(); }
    later(DO_CONNECT, 1);
}

/* ( -- ) the saved networks: wifi <n> <ssid> <rssi|-> [on], then wifi saved <n> */
static void w_wifi_q(void)
{
    for (int i = 0; i < s_nnet; i++) {
        if (s_net[i].rssi) con_printf("wifi %d %s %d%s\n", i + 1, s_net[i].ssid, s_net[i].rssi, s_up && s_cur == i ? " on" : "");
        else con_printf("wifi %d %s -%s\n", i + 1, s_net[i].ssid, s_up && s_cur == i ? " on" : "");
    }
    con_printf("wifi saved %d\n", s_nnet);
}

/* ( -- ) what the last scan saw (2.4 GHz only): seen <ssid> <rssi> ch <n> [open] */
static void w_wifi_seen(void)
{
    for (int i = 0; i < s_nseen; i++)
        con_printf("seen %s %d ch %u%s\n", s_seen_ap[i].ssid, s_seen_ap[i].rssi, s_seen_ap[i].ch, s_seen_ap[i].open ? " open" : "");
    con_printf("seen %d\n", s_nseen);
}

static bool slot_arg(const char *who, int *i)
{
    *i = (int)forth_pop() - 1;
    if (*i >= 0 && *i < s_nnet) return true;
    con_printf("? %s: 1..%d (wifi?)\n", who, s_nnet);
    return false;
}

/* ( n -- ) forget saved network n; if Scout is on it, it moves to another */
static void w_wifi_forget(void)
{
    int i;
    if (!slot_arg("wifi-forget", &i)) return;
    con_printf("wifi: forgot %s\n", s_net[i].ssid);
    bool was_on = s_up && s_cur == i;
    memmove(&s_net[i], &s_net[i + 1], (s_nnet - i - 1) * sizeof s_net[0]);
    s_nnet--;
    nets_save();
    if (s_cur > i) s_cur--;
    s_ci = -1; s_ncand = 0;
    if (was_on) { s_quiet = true; s_cur = -1; esp_wifi_disconnect(); }
    if (!s_up || was_on) later(DO_SCAN, 1);              /* start over with what is left */
}

/* ( n -- ) make saved network n the first choice (takes effect at the next scan) */
static void w_wifi_first(void)
{
    int i;
    if (!slot_arg("wifi-first", &i)) return;
    net_t t = s_net[i];
    memmove(&s_net[1], &s_net[0], i * sizeof s_net[0]);
    s_net[0] = t;
    nets_save();
    if (s_cur >= 0) s_cur = s_cur == i ? 0 : s_cur < i ? s_cur + 1 : s_cur;
    s_ci = -1; s_ncand = 0;
    if (!s_up) later(DO_SCAN, 1);
    con_printf("wifi: %s first\n", t.ssid);
}

static void w_wifi_ps(void)
{
    int on = forth_pop() ? 1 : 0;
    kv_int_set("wps", on);
    if (s_started) apply_ps();
    wifi_ps_type_t t = WIFI_PS_NONE;
    esp_wifi_get_ps(&t);
    con_printf("wifi-ps %s\n", t == WIFI_PS_NONE ? "off" : "on");
}

static void w_net_q(void)
{
    wifi_ap_record_t ap;
    if (s_up && esp_wifi_sta_get_ap_info(&ap) == ESP_OK) con_printf("net up " IPSTR " rssi %d %s\n", IP2STR(&s_ip), ap.rssi, (char *)ap.ssid);
    else con_printf("net down\n");
}

static void w_mem_q(void)
{
    con_printf("mem internal %u largest %u psram %u\n", (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
               (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void hub_net_on_up(void (*fn)(void)) { s_on_up = fn; }

/* ---- HTTP GET into a buffer ------------------------------------------------ */

int hub_http_get(const char *url, uint8_t *buf, int cap, int timeout_ms)
{
    if (!s_up) return -1;
    esp_http_client_config_t c = { .url = url, .timeout_ms = timeout_ms, .buffer_size = 1024, .disable_auto_redirect = true };
    esp_http_client_handle_t h = esp_http_client_init(&c);
    if (!h) return -2;
    int got = -3;
    if (esp_http_client_open(h, 0) == ESP_OK) {
        esp_http_client_fetch_headers(h);
        int st = esp_http_client_get_status_code(h);
        if (st == 200) {
            got = 0;
            while (got < cap) {
                int n = esp_http_client_read(h, (char *)buf + got, cap - got);
                if (n <= 0) break;
                got += n;
            }
            if (got >= cap) got = -4;                    /* too big for the buffer */
        } else got = -st;
    }
    esp_http_client_cleanup(h);
    return got;
}

/* ( addr u -- ) set the time zone (POSIX TZ, e.g. PST8PDT,M3.2.0,M11.1.0); saved */
static void w_tz_store(void)
{
    int l = (int)forth_pop(); const char *s = (const char *)forth_pop();
    char tz[48] = "";
    if (!s || l <= 0 || l >= (int)sizeof tz) { con_printf("? tz! bad args\n"); return; }
    memcpy(tz, s, l);
    kv_str_set("tz", tz);
    setenv("TZ", tz, 1); tzset();
    time_t t = time(NULL); struct tm tm; localtime_r(&t, &tm);
    con_printf("tz %s, now %02d:%02d\n", tz, tm.tm_hour, tm.tm_min);
}

void hub_net_register_words(void)
{
    forth_register_word("mem?", w_mem_q);
    forth_register_word("wifi!", w_wifi_store);
    forth_register_word("wifi?", w_wifi_q);
    forth_register_word("wifi-seen", w_wifi_seen);
    forth_register_word("wifi-forget", w_wifi_forget);
    forth_register_word("wifi-first", w_wifi_first);
    forth_register_word("wifi-ps!", w_wifi_ps);
    forth_register_word("net?", w_net_q);
    forth_register_word("tz!", w_tz_store);
}
