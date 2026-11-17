/*
 * hub_web.c — see hub_web.h.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "mbedtls/base64.h"
#include "hub_web.h"
#include "hub_reel.h"
#include "console.h"

static const char *TAG = "web";
#define ADD_CAP (400 * 1024)          /* streamed to flash, so only a sanity limit */

static httpd_handle_t s_httpd;

static void cors(httpd_req_t *r)
{
    httpd_resp_set_hdr(r, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(r, "Cache-Control", "no-store");
}

static esp_err_t h_list(httpd_req_t *r)
{
    cors(r);
    httpd_resp_set_type(r, "text/plain");
    char line[80];
    for (int i = 0; ; i++) {
        int num = reel_num_at(i);
        if (num < 0) break;
        char label[17] = "?", at[10] = "?"; int w = 0, h = 0, b = 0;
        reel_meta(num, label, &w, &h, at, &b);
        int n = snprintf(line, sizeof line, "%04d %s %d %d %s %d\n", num, label, w, h, at, b);
        if (httpd_resp_send_chunk(r, line, n) != ESP_OK) return ESP_FAIL;
    }
    int show = reel_num_at(reel_showing());
    int n = show >= 0 ? snprintf(line, sizeof line, "showing %04d\n", show) : snprintf(line, sizeof line, "showing none\n");
    httpd_resp_send_chunk(r, line, n);
    return httpd_resp_send_chunk(r, NULL, 0);
}

static esp_err_t h_file(httpd_req_t *r)
{
    int num;
    if (sscanf(r->uri, "/reel/%d", &num) != 1) { httpd_resp_send_err(r, HTTPD_404_NOT_FOUND, "no such shot"); return ESP_FAIL; }
    char p[24];
    reel_file(num, p, sizeof p);
    FILE *f = fopen(p, "rb");
    if (!f) { httpd_resp_send_err(r, HTTPD_404_NOT_FOUND, "no such shot"); return ESP_FAIL; }
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    /* our own headers, so a streamed file still has a Content-Length (the
     * glasses' media loader wants one; the chunk API would leave it out) */
    char hdr[192];
    int hn = snprintf(hdr, sizeof hdr, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n"
                      "Access-Control-Allow-Origin: *\r\nCache-Control: no-store\r\n\r\n", len);
    char *buf = malloc(2048);
    bool ok = buf && httpd_send(r, hdr, hn) == hn;
    size_t n;
    while (ok && (n = fread(buf, 1, 2048, f)) > 0) {
        size_t off = 0;
        while (ok && off < n) { int k = httpd_send(r, buf + off, n - off); ok = k > 0; off += ok ? k : 0; }
    }
    fclose(f);
    free(buf);
    return ESP_OK;
}

static esp_err_t h_add(httpd_req_t *r)
{
    char q[48] = "", label[17] = "SPECS";
    if (httpd_req_get_url_query_str(r, q, sizeof q) == ESP_OK) {
        char v[17];
        if (httpd_query_key_value(q, "label", v, sizeof v) == ESP_OK && v[0]) {
            for (int i = 0; v[i]; i++) if (v[i] >= 'a' && v[i] <= 'z') v[i] -= 32;
            snprintf(label, sizeof label, "%s", v);
        }
    }
    int len = r->content_len;
    if (len <= 0 || len > ADD_CAP) { httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "size"); return ESP_FAIL; }
    enum { IN = 1536 };
    uint8_t *in = malloc(IN + 4), *out = malloc(IN);
    if (!in || !out || !reel_stream_begin(label, len)) {
        free(in); free(out);
        httpd_resp_send_err(r, HTTPD_500_INTERNAL_SERVER_ERROR, "memory"); return ESP_FAIL;
    }
    int got = 0, have = 0, raw = -1;           /* raw: 1 JPEG bytes, 0 base64 */
    bool ok = true;
    while (got < len && ok) {
        int n = httpd_req_recv(r, (char *)in + have, (len - got) < (IN - have) ? (len - got) : (IN - have));
        if (n == HTTPD_SOCK_ERR_TIMEOUT) continue;
        if (n <= 0) { ok = false; break; }
        got += n; have += n;
        if (raw < 0 && have >= 2) raw = in[0] == 0xFF && in[1] == 0xD8;
        if (raw == 1) { ok = reel_stream_write(in, have); have = 0; continue; }
        /* base64: drop whitespace, decode whole groups of 4, keep the rest */
        int k = 0;
        for (int i = 0; i < have; i++) if (in[i] != '\r' && in[i] != '\n' && in[i] != ' ') in[k++] = in[i];
        have = k;
        int use = got == len ? have : have - have % 4;
        if (use > 0) {
            size_t o = 0;
            if (mbedtls_base64_decode(out, IN, &o, in, use) != 0) { ok = false; break; }
            ok = reel_stream_write(out, o);
            memmove(in, in + use, have - use);
            have -= use;
        }
    }
    free(in); free(out);
    if (!ok || got != len) { reel_stream_abort(); httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "bad upload"); return ESP_FAIL; }
    int num = reel_stream_end();
    if (num <= 0) { httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "not a jpeg"); return ESP_FAIL; }
    con_printf("shot %s %d B in (upload) #%d\n", label, len, num);
    cors(r);
    char a[24]; int m = snprintf(a, sizeof a, "added %04d\n", num);
    return httpd_resp_send(r, a, m);
}

void hub_web_start(void)
{
    /* also from the console: web-start */
    if (s_httpd) return;
    httpd_config_t c = HTTPD_DEFAULT_CONFIG();
    c.stack_size = 6144;
    c.max_open_sockets = 4;
    c.lru_purge_enable = true;
    c.uri_match_fn = httpd_uri_match_wildcard;
    esp_err_t e = httpd_start(&s_httpd, &c);
    if (e != ESP_OK) { con_printf("web: httpd_start %s\n", esp_err_to_name(e)); s_httpd = NULL; return; }
    const httpd_uri_t u[] = {
        { .uri = "/reel.txt", .method = HTTP_GET,  .handler = h_list },
        { .uri = "/reel/add", .method = HTTP_POST, .handler = h_add },
        { .uri = "/reel/*",   .method = HTTP_GET,  .handler = h_file },
    };
    for (size_t i = 0; i < sizeof u / sizeof u[0]; i++) httpd_register_uri_handler(s_httpd, &u[i]);
    con_printf("web: up\n");
}
