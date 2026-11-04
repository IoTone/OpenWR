/*
 * scout_net.c — Wi-Fi station (from AlphaFriend's net_words.c) and the web
 * server for the LAN frame experiment. See scout_net.h.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "esp_http_server.h"
#include "esp_camera.h"
#include "lwip/sockets.h"
#include "nvs.h"
#include "esp_heap_caps.h"

#include "forth_core.h"
#include "console.h"
#include "scout_net.h"

static const char *TAG = "net";
#define NS "scout"

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

/* ---- Wi-Fi station ------------------------------------------------------ */

static esp_netif_t       *s_netif;
static bool               s_started, s_up;
static int                s_retry_s = 5;
static esp_ip4_addr_t     s_ip;
static esp_timer_handle_t s_retry;
static httpd_handle_t     s_httpd;

static void web_start(void);

static void on_retry(void *arg) { esp_wifi_connect(); }

static void apply_ps(void)
{
    int ps = kv_int("wps", 1);
    esp_err_t e = esp_wifi_set_ps(ps ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE);
    if (e != ESP_OK) ESP_LOGW(TAG, "power save %s refused: %s", ps ? "on" : "off", esp_err_to_name(e));
}

static void on_wifi(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        apply_ps();
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *d = (wifi_event_sta_disconnected_t *)data;
        s_up = false;
        ESP_LOGW(TAG, "disconnected (reason %d), retry in %d s", d->reason, s_retry_s);
        esp_timer_start_once(s_retry, (uint64_t)s_retry_s * 1000000ULL);
        if (s_retry_s < 120) s_retry_s *= 2;
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_ip = ((ip_event_got_ip_t *)data)->ip_info.ip;
        s_retry_s = 5;
        s_up = true;
        ESP_LOGI(TAG, "got ip " IPSTR, IP2STR(&s_ip));
        con_printf("url http://" IPSTR "/ ws://" IPSTR "/ws\n", IP2STR(&s_ip), IP2STR(&s_ip));
        web_start();
    }
}

static void apply_creds(const char *ssid, const char *pass)
{
    wifi_config_t c = { 0 };
    strncpy((char *)c.sta.ssid, ssid, sizeof c.sta.ssid - 1);
    strncpy((char *)c.sta.password, pass, sizeof c.sta.password - 1);
    c.sta.threshold.authmode = pass[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    esp_wifi_set_config(WIFI_IF_STA, &c);
}

void scout_net_init(void)
{
    const esp_timer_create_args_t ta = { .callback = on_retry, .name = "wifi_retry" };
    esp_timer_create(&ta, &s_retry);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_netif = esp_netif_create_default_wifi_sta();
    esp_netif_set_hostname(s_netif, "scout");
    wifi_init_config_t w = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&w));
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi, NULL);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);

    char ssid[33] = "", pass[65] = "";
    if (kv_str("ssid", ssid, sizeof ssid) && ssid[0]) {
        kv_str("pass", pass, sizeof pass);
        apply_creds(ssid, pass);
        esp_wifi_start();
        s_started = true;
        ESP_LOGI(TAG, "connecting to %s", ssid);
    } else {
        ESP_LOGW(TAG, "no Wi-Fi yet: s\" ssid\" s\" pass\" wifi!");
    }
}

bool scout_net_up(void) { return s_up; }

/* ---- web server --------------------------------------------------------- */

static volatile unsigned s_n_jpg, s_n_ws, s_n_mjpeg;
static volatile uint32_t s_last_peer;
static volatile int64_t  s_last_us;

static void note_peer(httpd_req_t *r)
{
    struct sockaddr_in6 a; socklen_t l = sizeof a;
    int fd = httpd_req_to_sockfd(r);
    if (getpeername(fd, (struct sockaddr *)&a, &l) == 0) {
        if (a.sin6_family == AF_INET) s_last_peer = ((struct sockaddr_in *)&a)->sin_addr.s_addr;
        else s_last_peer = a.sin6_addr.un.u32_addr[3];     /* v4-mapped */
    }
    s_last_us = esp_timer_get_time();
}

static void cors(httpd_req_t *r)
{
    httpd_resp_set_hdr(r, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
}

/* a fresh JPEG: with GRAB_LATEST and two buffers the first may be stale */
static camera_fb_t *fresh(void)
{
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) { esp_camera_fb_return(fb); fb = esp_camera_fb_get(); }
    if (fb && fb->format != PIXFORMAT_JPEG) { esp_camera_fb_return(fb); fb = NULL; }
    return fb;
}

static esp_err_t h_frame(httpd_req_t *r)
{
    note_peer(r);
    camera_fb_t *fb = fresh();
    if (!fb) { httpd_resp_send_err(r, HTTPD_500_INTERNAL_SERVER_ERROR, "no frame"); return ESP_FAIL; }
    cors(r);
    httpd_resp_set_type(r, "image/jpeg");
    esp_err_t e = httpd_resp_send(r, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    s_n_jpg++;
    return e;
}

#define BOUNDARY "scoutframe"
static esp_err_t h_stream(httpd_req_t *r)
{
    note_peer(r);
    cors(r);
    httpd_resp_set_type(r, "multipart/x-mixed-replace;boundary=" BOUNDARY);
    char hdr[96];
    for (;;) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) return ESP_FAIL;
        int n = snprintf(hdr, sizeof hdr, "\r\n--" BOUNDARY "\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (unsigned)fb->len);
        esp_err_t e = httpd_resp_send_chunk(r, hdr, n);
        if (e == ESP_OK) e = httpd_resp_send_chunk(r, (const char *)fb->buf, fb->len);
        esp_camera_fb_return(fb);
        if (e != ESP_OK) return ESP_OK;               /* client went away */
        s_n_mjpeg++;
        s_last_us = esp_timer_get_time();
    }
}

static esp_err_t h_ws(httpd_req_t *r)
{
    if (r->method == HTTP_GET) { note_peer(r); return ESP_OK; }   /* the handshake */
    httpd_ws_frame_t in = { 0 };
    uint8_t buf[64];
    in.payload = buf;
    esp_err_t e = httpd_ws_recv_frame(r, &in, sizeof buf);
    if (e != ESP_OK) return e;
    if (in.type != HTTPD_WS_TYPE_TEXT && in.type != HTTPD_WS_TYPE_BINARY) return ESP_OK;
    camera_fb_t *fb = fresh();
    if (!fb) {
        httpd_ws_frame_t t = { .type = HTTPD_WS_TYPE_TEXT, .payload = (uint8_t *)"err no-frame", .len = 12 };
        return httpd_ws_send_frame(r, &t);
    }
    httpd_ws_frame_t out = { .type = HTTPD_WS_TYPE_BINARY, .payload = fb->buf, .len = fb->len, .final = true };
    e = httpd_ws_send_frame(r, &out);
    esp_camera_fb_return(fb);
    s_n_ws++;
    s_last_us = esp_timer_get_time();
    return e;
}

static esp_err_t h_info(httpd_req_t *r)
{
    note_peer(r);
    char j[96];
    snprintf(j, sizeof j, "{\"w\":320,\"h\":240,\"jpg\":%u,\"ws\":%u,\"mjpeg\":%u}", s_n_jpg, s_n_ws, s_n_mjpeg);
    cors(r);
    httpd_resp_set_type(r, "application/json");
    return httpd_resp_sendstr(r, j);
}

static const char PAGE[] =
    "<!doctype html><meta name=viewport content='width=device-width'>"
    "<title>Scout</title><body style='background:#111;color:#ddd;font:14px system-ui'>"
    "<p>Scout <span id=f>-</span> fps (frame.jpg loop) &middot; <a style='color:#8cf' href=/stream>/stream</a></p>"
    "<img id=i style='width:100%;max-width:640px;image-rendering:pixelated'>"
    "<script>let n=0,t=performance.now();const i=document.getElementById('i');"
    "function go(){i.src='/frame.jpg?'+Date.now()}"
    "i.onload=()=>{n++;const d=performance.now()-t;if(d>2000){f.textContent=(n*1000/d).toFixed(1);n=0;t=performance.now()}go()};"
    "i.onerror=()=>setTimeout(go,500);go()</script>";

static esp_err_t h_root(httpd_req_t *r)
{
    note_peer(r);
    httpd_resp_set_type(r, "text/html");
    return httpd_resp_send(r, PAGE, sizeof PAGE - 1);
}

static void web_start(void)
{
    if (s_httpd) return;
    httpd_config_t c = HTTPD_DEFAULT_CONFIG();
    c.stack_size = 6144;
    c.max_open_sockets = 5;
    c.lru_purge_enable = true;
    esp_err_t e = httpd_start(&s_httpd, &c);
    if (e != ESP_OK) {
        con_printf("web: httpd_start %s, internal free %u largest %u\n", esp_err_to_name(e),
                   (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
                   (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
        s_httpd = NULL;
        return;
    }
    con_printf("web: up\n");
    const httpd_uri_t u[] = {
        { .uri = "/",          .method = HTTP_GET, .handler = h_root },
        { .uri = "/frame.jpg", .method = HTTP_GET, .handler = h_frame },
        { .uri = "/stream",    .method = HTTP_GET, .handler = h_stream },
        { .uri = "/info",      .method = HTTP_GET, .handler = h_info },
        { .uri = "/ws",        .method = HTTP_GET, .handler = h_ws, .is_websocket = true },
    };
    for (size_t i = 0; i < sizeof u / sizeof u[0]; i++) httpd_register_uri_handler(s_httpd, &u[i]);
}

/* ---- words -------------------------------------------------------------- */

static void w_wifi_store(void)
{
    int pl = (int)forth_pop(); const char *p = (const char *)forth_pop();
    int sl = (int)forth_pop(); const char *s = (const char *)forth_pop();
    char ssid[33] = "", pass[65] = "";
    if (!s || sl <= 0 || sl > 32 || pl < 0 || pl > 64) { con_printf("? wifi! bad args\n"); return; }
    memcpy(ssid, s, sl);
    if (p && pl > 0) memcpy(pass, p, pl);
    kv_str_set("ssid", ssid);
    kv_str_set("pass", pass);
    apply_creds(ssid, pass);
    esp_timer_stop(s_retry);
    s_retry_s = 5;
    if (s_started) { esp_wifi_disconnect(); esp_wifi_connect(); }
    else { esp_wifi_start(); s_started = true; }
    con_printf("wifi: stored, connecting to %s\n", ssid);
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
    if (s_up && esp_wifi_sta_get_ap_info(&ap) == ESP_OK) con_printf("net up " IPSTR " rssi %d\n", IP2STR(&s_ip), ap.rssi);
    else con_printf("net down\n");
}

static void w_url_q(void)
{
    if (s_up && s_httpd) con_printf("url http://" IPSTR "/ ws://" IPSTR "/ws\n", IP2STR(&s_ip), IP2STR(&s_ip));
    else con_printf("url none\n");
}

static void w_web_q(void)
{
    uint32_t a = s_last_peer;
    long ago = s_last_us ? (long)((esp_timer_get_time() - s_last_us) / 1000) : -1;
    con_printf("web jpg %u ws %u mjpeg %u last %u.%u.%u.%u %ld\n", s_n_jpg, s_n_ws, s_n_mjpeg,
               (unsigned)(a & 255), (unsigned)(a >> 8 & 255), (unsigned)(a >> 16 & 255), (unsigned)(a >> 24), ago);
}

static void w_web_start(void) { if (s_up) web_start(); else con_printf("net down\n"); }
static void w_mem_q(void)
{
    con_printf("mem internal %u largest %u psram %u\n", (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
               (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

void scout_net_register_words(void)
{
    forth_register_word("web-start", w_web_start);
    forth_register_word("mem?", w_mem_q);
    forth_register_word("wifi!", w_wifi_store);
    forth_register_word("wifi-ps!", w_wifi_ps);
    forth_register_word("net?", w_net_q);
    forth_register_word("url?", w_url_q);
    forth_register_word("web?", w_web_q);
}
