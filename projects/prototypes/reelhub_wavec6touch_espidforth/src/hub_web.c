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
#define ADD_CAP (128 * 1024)          /* base64 of a ~96 KB JPEG */

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
    cors(r);
    httpd_resp_set_type(r, "image/jpeg");
    char *buf = malloc(2048);
    esp_err_t e = buf ? ESP_OK : ESP_FAIL;
    size_t n;
    while (e == ESP_OK && (n = fread(buf, 1, 2048, f)) > 0) e = httpd_resp_send_chunk(r, buf, n);
    fclose(f);
    free(buf);
    if (e == ESP_OK) httpd_resp_send_chunk(r, NULL, 0);
    return e;
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
    uint8_t *body = malloc(len + 1);
    if (!body) { httpd_resp_send_err(r, HTTPD_500_INTERNAL_SERVER_ERROR, "memory"); return ESP_FAIL; }
    int got = 0;
    while (got < len) {
        int n = httpd_req_recv(r, (char *)body + got, len - got);
        if (n == HTTPD_SOCK_ERR_TIMEOUT) continue;
        if (n <= 0) break;
        got += n;
    }
    if (got != len) { free(body); httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "short"); return ESP_FAIL; }
    size_t jl = len;
    if (!(body[0] == 0xFF && body[1] == 0xD8)) {               /* base64 text: decode in place */
        size_t out = 0;
        if (mbedtls_base64_decode(body, len, &out, body, len) != 0 || out < 4 || body[0] != 0xFF || body[1] != 0xD8) {
            free(body); httpd_resp_send_err(r, HTTPD_400_BAD_REQUEST, "not a jpeg"); return ESP_FAIL;
        }
        jl = out;
    }
    int num = reel_add(body, jl, label);
    free(body);
    if (num <= 0) { httpd_resp_send_err(r, HTTPD_500_INTERNAL_SERVER_ERROR, "save failed"); return ESP_FAIL; }
    con_printf("shot %s %u B (upload) #%d\n", label, (unsigned)jl, num);
    cors(r);
    char a[24]; int n = snprintf(a, sizeof a, "added %04d\n", num);
    return httpd_resp_send(r, a, n);
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
