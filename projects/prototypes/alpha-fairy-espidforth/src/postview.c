/*
 * postview.c — the last shot, shrunk and dithered for the glasses.
 *
 * actTakePicture answers with a postview URL on the camera (usually the
 * 2M postview, 1616x1080). AlphaFriend has no PSRAM, so the JPEG is never
 * held: it streams from esp_http_client straight into the ROM's TJpgDec at
 * 1/8 scale (202x135 for a 2M postview), into a small grey buffer (bigger
 * postviews are sampled down to fit), then cropped to 3:2, contrast
 * stretched and Floyd-Steinberg dithered to PV_W x PV_H, 1 = ink, packed
 * MSB first: the same frame format as the maplet's and Scout's rolls.
 *
 *   pv   ( -- )   pv <w> <h> then base64 lines (57 bytes each) | pv none | pv err <what>
 *
 * The liveview goes the same way: `lv` opens the camera's liveview stream,
 * reads one packet (8-byte common header, 128-byte payload header with the
 * JPEG size, the JPEG, padding), decodes the JPEG at 1/4 (a 640x424
 * liveview gives 160x106), dithers it to LV_W x LV_H and closes the stream,
 * so every call is a fresh frame. About 2.1 KB, well under a second over BLE.
 *
 *   lv   ( -- )   lv <w> <h> then base64 lines | lv none | lv err <what>
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp32/rom/tjpgd.h"

#include "forth_core.h"
#include "console.h"
#include "sony_http.h"
#include "postview.h"

static const char *TAG = "pv";

#define PV_W        198               /* 3:2, 25 bytes a row, 3300 B a frame */
#define PV_H        132
#define LV_W        160               /* 3:2, 20 bytes a row, 2120 B a frame */
#define LV_H        106
#define GREY_CAP    (40 * 1024)       /* the decoded grey image, at most */
#define POOL_SZ     3200              /* TJpgDec's work area (3100 is ChaN's figure) */

typedef struct {
    esp_http_client_handle_t http;
    uint8_t *grey;
    int      gw, gh, step;            /* grey size; keep every step-th decoded pixel */
    int      got;                     /* bytes read */
    int      left;                    /* bytes the JPEG may still use; < 0: no limit */
} pv_ctx_t;

static UINT pv_in(JDEC *jd, BYTE *buf, UINT n)
{
    pv_ctx_t *c = (pv_ctx_t *)jd->device;
    static uint8_t skip[256];
    UINT done = 0;
    while (done < n) {
        int want = (int)(n - done);
        uint8_t *dst = buf ? buf + done : skip;
        if (!buf && want > (int)sizeof skip) want = sizeof skip;
        if (c->left >= 0 && want > c->left) want = c->left;
        if (want <= 0) break;
        int r = esp_http_client_read(c->http, (char *)dst, want);
        if (r <= 0) break;
        done += (UINT)r;
        c->got += r;
        if (c->left >= 0) c->left -= r;
    }
    return done;
}

static UINT pv_out(JDEC *jd, void *bitmap, JRECT *rect)
{
    pv_ctx_t *c = (pv_ctx_t *)jd->device;
    const uint8_t *p = (const uint8_t *)bitmap;
    for (int y = rect->top; y <= rect->bottom; y++) {
        for (int x = rect->left; x <= rect->right; x++, p += 3) {
            if (x % c->step || y % c->step) continue;
            int gx = x / c->step, gy = y / c->step;
            if (gx < c->gw && gy < c->gh) c->grey[gy * c->gw + gx] = (uint8_t)((p[0] + 2 * p[1] + p[2]) >> 2);
        }
    }
    return 1;
}

/* crop to PV_W:PV_H (cover), box-average, stretch 2..98 %, Floyd-Steinberg */
static void dither(const uint8_t *g, int gw, int gh, uint8_t *dst, const int OW, const int OH)
{
    float sc = (float)gw / OW < (float)gh / OH ? (float)gw / OW : (float)gh / OH;
    int cw = (int)(OW * sc), ch = (int)(OH * sc), ox = (gw - cw) / 2, oy = (gh - ch) / 2;

    uint32_t hist[256] = { 0 }, total = 0, acc = 0;
    for (int y = oy; y < oy + ch; y++)
        for (int x = ox; x < ox + cw; x++) { hist[g[y * gw + x]]++; total++; }
    int lo = 0, hi = 255;
    for (int i = 0; i < 256; i++) { acc += hist[i]; if (acc >= total * 2 / 100) { lo = i; break; } }
    acc = 0;
    for (int i = 255; i >= 0; i--) { acc += hist[i]; if (acc >= total * 2 / 100) { hi = i; break; } }
    if (hi <= lo + 8) { lo = 0; hi = 255; }
    uint8_t lut[256];
    for (int i = 0; i < 256; i++) {
        float v = (float)(i - lo) / (float)(hi - lo);
        v = v < 0 ? 0 : v > 1 ? 1 : v;
        lut[i] = (uint8_t)(255.0f * powf(v, 0.7f));
    }

    const int stride = (OW + 7) / 8;
    memset(dst, 0, stride * OH);
    static int16_t err[2][200 + 2];                     /* widths up to 200 */
    memset(err, 0, sizeof err);
    for (int y = 0; y < OH; y++) {
        int16_t *cur = err[y & 1], *nxt = err[(y + 1) & 1];
        memset(nxt, 0, sizeof err[0]);
        int y0 = oy + (int)(y * sc), y1 = oy + (int)((y + 1) * sc);
        if (y1 <= y0) y1 = y0 + 1;
        for (int x = 0; x < OW; x++) {
            int x0 = ox + (int)(x * sc), x1 = ox + (int)((x + 1) * sc);
            if (x1 <= x0) x1 = x0 + 1;
            uint32_t sum = 0, n = 0;
            for (int yy = y0; yy < y1 && yy < gh; yy++)
                for (int xx = x0; xx < x1 && xx < gw; xx++) { sum += g[yy * gw + xx]; n++; }
            int want = lut[n ? sum / n : 0] + cur[x + 1];
            bool ink = want < 128;
            int e = want - (ink ? 0 : 255);
            cur[x + 2] += e * 7 / 16;
            nxt[x]     += e * 3 / 16;
            nxt[x + 1] += e * 5 / 16;
            nxt[x + 2] += e / 16;
            if (ink) dst[y * stride + (x >> 3)] |= 0x80 >> (x & 7);
        }
    }
}

static void b64_lines(const uint8_t *buf, size_t len)
{
    static const char A[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (size_t off = 0; off < len; off += 57) {
        size_t k = len - off < 57 ? len - off : 57;
        const uint8_t *b = buf + off;
        char line[80]; int o = 0;
        for (size_t i = 0; i < k; i += 3) {
            uint32_t v = (uint32_t)b[i] << 16 | (i + 1 < k ? (uint32_t)b[i + 1] << 8 : 0) | (i + 2 < k ? b[i + 2] : 0);
            line[o++] = A[v >> 18 & 63]; line[o++] = A[v >> 12 & 63];
            line[o++] = i + 1 < k ? A[v >> 6 & 63] : '=';
            line[o++] = i + 2 < k ? A[v & 63] : '=';
        }
        line[o] = 0;
        con_printf("%s\n", line);
    }
}

static void w_pv(void)
{
    const char *raw = shc_postview();
    if (!raw || !raw[0]) { con_printf("pv none\n"); return; }
    char url[256];
    size_t j = 0;                                       /* JSON may escape the slashes */
    for (size_t i = 0; raw[i] && j + 1 < sizeof url; i++) if (raw[i] != '\\') url[j++] = raw[i];
    url[j] = 0;

    int64_t t0 = esp_timer_get_time();
    pv_ctx_t c = { .left = -1 };
    void *pool = malloc(POOL_SZ);
    uint8_t *frame = malloc((PV_W + 7) / 8 * PV_H);
    esp_http_client_config_t cfg = { .url = url, .timeout_ms = 8000, .buffer_size = 1024 };
    c.http = esp_http_client_init(&cfg);
    if (!pool || !frame || !c.http) { con_printf("pv err memory\n"); goto out; }
    if (esp_http_client_open(c.http, 0) != ESP_OK) { con_printf("pv err connect\n"); goto out; }
    int len = esp_http_client_fetch_headers(c.http);
    int status = esp_http_client_get_status_code(c.http);
    if (status != 200) { con_printf("pv err http %d\n", status); goto out; }

    JDEC jd;
    JRESULT r = jd_prepare(&jd, pv_in, pool, POOL_SZ, &c);
    if (r != JDR_OK) { con_printf("pv err jpeg %d\n", r); goto out; }
    int sw = (jd.width + 7) >> 3, sh = (jd.height + 7) >> 3;      /* at 1/8 */
    c.step = 1;
    while ((sw / c.step) * (sh / c.step) > GREY_CAP) c.step++;
    c.gw = sw / c.step; c.gh = sh / c.step;
    c.grey = malloc(c.gw * c.gh);
    if (!c.grey) { con_printf("pv err memory %dx%d, free %u\n", c.gw, c.gh, (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT)); goto out; }
    memset(c.grey, 0, c.gw * c.gh);
    r = jd_decomp(&jd, pv_out, 3);
    if (r != JDR_OK) { con_printf("pv err decode %d\n", r); goto out; }
    dither(c.grey, c.gw, c.gh, frame, PV_W, PV_H);
    ESP_LOGI(TAG, "%ux%u jpeg (%d B), grey %dx%d, %lld ms", jd.width, jd.height, len > 0 ? len : c.got,
             c.gw, c.gh, (esp_timer_get_time() - t0) / 1000);
    con_printf("pv %d %d\n", PV_W, PV_H);
    b64_lines(frame, (PV_W + 7) / 8 * PV_H);

out:
    if (c.http) { esp_http_client_close(c.http); esp_http_client_cleanup(c.http); }
    free(c.grey);
    free(frame);
    free(pool);
}

/* read exactly n bytes of the stream (into buf, or dropped) */
static bool read_n(esp_http_client_handle_t h, uint8_t *buf, int n)
{
    static uint8_t skip[256];
    while (n > 0) {
        int k = buf ? n : (n < (int)sizeof skip ? n : (int)sizeof skip);
        int r = esp_http_client_read(h, (char *)(buf ? buf : skip), k);
        if (r <= 0) return false;
        if (buf) buf += r;
        n -= r;
    }
    return true;
}

static void w_lv(void)
{
    const char *raw = shc_liveview();
    const char *u0 = raw ? strstr(raw, "http") : NULL;
    if (!u0) { con_printf("lv none\n"); return; }
    char url[256];
    size_t j = 0;
    for (size_t i = 0; u0[i] && u0[i] != '"' && u0[i] != ']' && j + 1 < sizeof url; i++) if (u0[i] != '\\') url[j++] = u0[i];
    url[j] = 0;

    int64_t t0 = esp_timer_get_time();
    pv_ctx_t c = { .left = -1 };
    void *pool = malloc(POOL_SZ);
    uint8_t *frame = malloc((LV_W + 7) / 8 * LV_H);
    esp_http_client_config_t cfg = { .url = url, .timeout_ms = 4000, .buffer_size = 1024 };
    c.http = esp_http_client_init(&cfg);
    if (!pool || !frame || !c.http) { con_printf("lv err memory\n"); goto out; }
    if (esp_http_client_open(c.http, 0) != ESP_OK) { con_printf("lv err connect\n"); goto out; }
    esp_http_client_fetch_headers(c.http);
    int status = esp_http_client_get_status_code(c.http);
    if (status != 200) { con_printf("lv err http %d\n", status); goto out; }

    /* packets until an image one: 8-byte common header, 128-byte payload header */
    int jpeg = 0;
    for (int tries = 0; tries < 8 && !jpeg; tries++) {
        uint8_t ch[8], ph[128];
        if (!read_n(c.http, ch, 8) || ch[0] != 0xFF) { con_printf("lv err stream\n"); goto out; }
        if (!read_n(c.http, ph, 128) || ph[0] != 0x24 || ph[1] != 0x35 || ph[2] != 0x68 || ph[3] != 0x79) { con_printf("lv err header\n"); goto out; }
        int size = ph[4] << 16 | ph[5] << 8 | ph[6], pad = ph[7];
        if (ch[1] == 0x01) { jpeg = size; c.left = size; break; }
        if (!read_n(c.http, NULL, size + pad)) { con_printf("lv err stream\n"); goto out; }
    }
    if (!jpeg) { con_printf("lv err no-image\n"); goto out; }

    JDEC jd;
    JRESULT r = jd_prepare(&jd, pv_in, pool, POOL_SZ, &c);
    if (r != JDR_OK) { con_printf("lv err jpeg %d\n", r); goto out; }
    int scale = 2;                                      /* 1/4: 640x424 -> 160x106 */
    while (scale > 0 && ((int)jd.width >> scale) < LV_W) scale--;
    int sw = ((int)jd.width + (1 << scale) - 1) >> scale, sh = ((int)jd.height + (1 << scale) - 1) >> scale;
    c.step = 1;
    while ((sw / c.step) * (sh / c.step) > GREY_CAP) c.step++;
    c.gw = sw / c.step; c.gh = sh / c.step;
    c.grey = malloc(c.gw * c.gh);
    if (!c.grey) { con_printf("lv err memory\n"); goto out; }
    memset(c.grey, 0, c.gw * c.gh);
    r = jd_decomp(&jd, pv_out, scale);
    if (r != JDR_OK) { con_printf("lv err decode %d\n", r); goto out; }
    dither(c.grey, c.gw, c.gh, frame, LV_W, LV_H);
    ESP_LOGI(TAG, "lv %ux%u (%d B), %lld ms", jd.width, jd.height, jpeg, (esp_timer_get_time() - t0) / 1000);
    con_printf("lv %d %d\n", LV_W, LV_H);
    b64_lines(frame, (LV_W + 7) / 8 * LV_H);

out:
    if (c.http) { esp_http_client_close(c.http); esp_http_client_cleanup(c.http); }
    free(c.grey);
    free(frame);
    free(pool);
}

void postview_register_words(void)
{
    forth_register_word("pv", w_pv);
    forth_register_word("lv", w_lv);
}
