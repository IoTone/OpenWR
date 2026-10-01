/*
 * net_words.c — WiFi station, SNTP, NVS, HTTP GET, and their Forth words.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "forth_core.h"
#include "console.h"
#include "net_words.h"

static const char *TAG = "net";

/* ------------------------------------------------------------------ */
/* NVS key/value                                                       */
/* ------------------------------------------------------------------ */

bool kv_get_str(const char *key, char *out, size_t cap)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    size_t len = cap;
    esp_err_t err = nvs_get_str(h, key, out, &len);
    nvs_close(h);
    return err == ESP_OK;
}

void kv_set_str(const char *key, const char *val)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_str(h, key, val);
    nvs_commit(h);
    nvs_close(h);
}

bool kv_get_i32(const char *key, int32_t *out)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    esp_err_t err = nvs_get_i32(h, key, out);
    nvs_close(h);
    return err == ESP_OK;
}

void kv_set_i32(const char *key, int32_t val)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_i32(h, key, val);
    nvs_commit(h);
    nvs_close(h);
}

bool kv_get_blob(const char *key, void *out, size_t len)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    size_t got = len;
    esp_err_t err = nvs_get_blob(h, key, out, &got);
    nvs_close(h);
    return err == ESP_OK && got == len;
}

void kv_set_blob(const char *key, const void *data, size_t len)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_blob(h, key, data, len);
    nvs_commit(h);
    nvs_close(h);
}

/* ------------------------------------------------------------------ */
/* WiFi station                                                        */
/* ------------------------------------------------------------------ */

#define BIT_CONNECTED BIT0

static EventGroupHandle_t s_ev;
static esp_netif_t       *s_netif;
static bool               s_wifi_started;
static bool               s_have_creds;
static volatile bool      s_just_connected;
static int                s_retry_s = 5;             /* backoff 5 s .. 300 s */
static esp_ip4_addr_t     s_ip;
static esp_timer_handle_t s_retry_timer;

static void on_retry(void *arg) { esp_wifi_connect(); }

static void on_wifi(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupClearBits(s_ev, BIT_CONNECTED);
        ESP_LOGW(TAG, "disconnected, retry in %d s", s_retry_s);
        esp_timer_start_once(s_retry_timer, (uint64_t)s_retry_s * 1000000ULL);
        if (s_retry_s < 300) s_retry_s *= 2;
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
        s_ip = e->ip_info.ip;
        s_retry_s = 5;
        s_just_connected = true;
        xEventGroupSetBits(s_ev, BIT_CONNECTED);
        ESP_LOGI(TAG, "got ip " IPSTR, IP2STR(&s_ip));
    }
}

static void wifi_apply_creds(const char *ssid, const char *pass)
{
    wifi_config_t cfg = { 0 };
    strncpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid) - 1);
    strncpy((char *)cfg.sta.password, pass, sizeof(cfg.sta.password) - 1);
    cfg.sta.threshold.authmode = pass[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    s_have_creds = true;
}

esp_err_t net_init(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) return err;

    s_ev = xEventGroupCreate();
    const esp_timer_create_args_t ta = { .callback = on_retry, .name = "wifi_retry" };
    esp_timer_create(&ta, &s_retry_timer);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t wcfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wcfg));
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi, NULL);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);

    char ssid[33] = "", pass[65] = "";
    if (kv_get_str("ssid", ssid, sizeof ssid) && ssid[0]) {
        kv_get_str("pass", pass, sizeof pass);
        wifi_apply_creds(ssid, pass);
        esp_wifi_start();
        s_wifi_started = true;
        ESP_LOGI(TAG, "connecting to %s", ssid);
    } else {
        ESP_LOGW(TAG, "no credentials; use: s\" ssid\" s\" pass\" wifi!");
    }
    return ESP_OK;
}

bool net_ok(void)
{
    return s_ev && (xEventGroupGetBits(s_ev) & BIT_CONNECTED);
}

uint32_t net_ip4(void) { return net_ok() ? s_ip.addr : 0; }

bool net_just_connected(void)
{
    bool j = s_just_connected;
    s_just_connected = false;
    return j;
}

void net_set_credentials(const char *ssid, const char *pass)
{
    kv_set_str("ssid", ssid);
    kv_set_str("pass", pass);
    wifi_apply_creds(ssid, pass);
    if (s_wifi_started) {
        esp_wifi_disconnect();
        esp_wifi_connect();
    } else {
        esp_wifi_start();
        s_wifi_started = true;
    }
}

/* ------------------------------------------------------------------ */
/* HTTP GET                                                            */
/* ------------------------------------------------------------------ */

typedef struct { char *buf; size_t cap; size_t len; bool overflow; } sink_t;

static esp_err_t on_http(esp_http_client_event_t *e)
{
    if (e->event_id != HTTP_EVENT_ON_DATA) return ESP_OK;
    sink_t *s = (sink_t *)e->user_data;
    size_t room = s->cap - 1 - s->len;
    size_t n = (size_t)e->data_len < room ? (size_t)e->data_len : room;
    memcpy(s->buf + s->len, e->data, n);
    s->len += n;
    if ((size_t)e->data_len > room) s->overflow = true;
    return ESP_OK;
}

int net_http_get(const char *url, char *buf, size_t cap)
{
    if (!net_ok()) return -1;
    sink_t sink = { .buf = buf, .cap = cap, .len = 0, .overflow = false };
    esp_http_client_config_t cfg = {
        .url               = url,
        .event_handler     = on_http,
        .user_data         = &sink,
        .timeout_ms        = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size       = 1024,
    };
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    if (!c) return -1;
    esp_http_client_set_header(c, "Accept", "application/json");
    esp_http_client_set_header(c, "User-Agent", "goggl1/0.1 (OpenWR)");
    esp_err_t err = esp_http_client_perform(c);
    int status = esp_http_client_get_status_code(c);
    esp_http_client_cleanup(c);
    buf[sink.len] = '\0';
    if (err != ESP_OK) { ESP_LOGE(TAG, "GET %s: %s", url, esp_err_to_name(err)); return -1; }
    if (status != 200)  { ESP_LOGE(TAG, "GET %s: HTTP %d", url, status); return -1; }
    if (sink.overflow)  { ESP_LOGE(TAG, "GET %s: body > %u bytes", url, (unsigned)cap); return -1; }
    return (int)sink.len;
}

/* ------------------------------------------------------------------ */
/* Clock                                                               */
/* ------------------------------------------------------------------ */

static bool    s_sntp_started;
static bool    s_offset_known;
static int32_t s_offset_s;

void clock_start_sntp(void)
{
    if (s_sntp_started) return;
    esp_sntp_config_t cfg = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    cfg.start = true;
    esp_netif_sntp_init(&cfg);
    s_sntp_started = true;
}

bool clock_synced(void)
{
    time_t now = time(NULL);
    return now > 1700000000;   /* anything after 2023 means SNTP has set it */
}

void clock_set_offset(int32_t s) { s_offset_s = s; s_offset_known = true; }
bool clock_ok(void)              { return clock_synced() && s_offset_known; }
uint32_t clock_unix(void)        { return (uint32_t)time(NULL); }

void clock_local_hm(int *h, int *m)
{
    uint32_t t = clock_unix() + (uint32_t)s_offset_s;
    uint32_t day = t % 86400;
    *h = (int)(day / 3600);
    *m = (int)((day % 3600) / 60);
}

/* ------------------------------------------------------------------ */
/* Forth words                                                         */
/* ------------------------------------------------------------------ */

static void w_net_ok(void)    { forth_push(net_ok() ? -1 : 0); }
static void w_clock_ok(void)  { forth_push(clock_ok() ? -1 : 0); }
static void w_clock_s(void)   { forth_push((intptr_t)clock_unix()); }
static void w_clock_hm(void)  { int h, m; clock_local_hm(&h, &m); forth_push(h); forth_push(m); }

static void w_net_ip(void)
{
    uint32_t a = net_ok() ? s_ip.addr : 0;
    forth_push(a & 0xFF); forth_push((a >> 8) & 0xFF);
    forth_push((a >> 16) & 0xFF); forth_push((a >> 24) & 0xFF);
}

/* ( ssid-addr u pass-addr u -- ) */
static void w_wifi_store(void)
{
    int plen = (int)forth_pop(); const char *p = (const char *)forth_pop();
    int slen = (int)forth_pop(); const char *s = (const char *)forth_pop();
    char ssid[33] = "", pass[65] = "";
    if (!s || slen <= 0 || slen > 32 || plen < 0 || plen > 64) { con_printf("? wifi! bad args\n"); return; }
    memcpy(ssid, s, slen);
    if (p && plen > 0) memcpy(pass, p, plen);
    net_set_credentials(ssid, pass);
    con_printf("wifi: stored, connecting to %s\n", ssid);
}

void net_register_words(void)
{
    forth_register_word("net-ok?",  w_net_ok);
    forth_register_word("wifi!",    w_wifi_store);
    forth_register_word("net-ip",   w_net_ip);
    forth_register_word("clock-ok?", w_clock_ok);
    forth_register_word("clock-hm", w_clock_hm);
    forth_register_word("clock-s",  w_clock_s);
}
