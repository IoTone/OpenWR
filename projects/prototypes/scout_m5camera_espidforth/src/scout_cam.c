/*
 * scout_cam.c — Scout's camera: the OV2640 on an M5Camera Model B, a 1-bit
 * stylized frame per shot, and a roll on the board's own flash.
 *
 * The rendering is the maplet's (cam_words.c): crop to the frame, stretch
 * the contrast between the 2 % and 98 % points, lift the midtones, dither.
 * Frames are packed 1-bit, MSB first, rows of ceil(w/8) bytes, 1 = ink, so
 * the lens's Bitmap.ts and RollView.ts read them as they read the maplet's.
 *
 *   caps?      ( -- )        caps scout shoot lv last roll sty fr web
 *   cam?       ( -- )        cam <ready|err> OV2640 0
 *   shoot      ( -- )        ok <n>: frame + thumbnail into the roll
 *   last       ( -- )        last <w> <h>, then base64 lines of the last shot
 *   lv         ( -- )        lv 80 53, then base64 lines of a live frame
 *   sty!       ( n -- )      0 grain (Floyd-Steinberg), 1 screen (Bayer 4x4)
 *   fr!        ( n -- )      0 212x104 (maplet), 1 120x80 (3:2), 2 96x96
 *   sty? fr?   ( -- )        sty <n> <name> / fr <n> <w> <h>
 *   flip!      ( n -- )      bit 0 vflip, bit 1 mirror; saved
 *   roll?      ( -- )        roll <count> fs <free KB>
 *   roll-ls    ( -- )        NNNN.BIN lines, then roll: <count> frames (as the maplet)
 *   roll-b64   ( n -- )      frame n as base64 lines (as the maplet)
 *   roll-th    ( n k -- )    rth <i> 80 53 1 + base64, for i = n, n-1 ... (k of them)
 *   roll-format ( -- )       wipe the roll
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"
#include "driver/ledc.h"
#include "nvs.h"

#include "forth_core.h"
#include "console.h"
#include "scout_cam.h"

static const char *TAG = "scout";
#define POP() ((int)forth_pop())

#define CAM_W 320
#define CAM_H 240
#define TH_W  80
#define TH_H  53
#define FRAME_MAX (((212 + 7) / 8) * 104)

static const struct { int w, h; } FR[] = { { 212, 104 }, { 120, 80 }, { 96, 96 } };
static const char *STY[] = { "grain", "screen" };

static uint8_t *s_grey;                      /* PSRAM, CAM_W x CAM_H */
static uint8_t *s_rgb;                       /* PSRAM, CAM_W x CAM_H x 3: the decoded JPEG */
static bool     s_cam_up, s_fs_up;
static int      s_sty, s_fr, s_flip, s_next;
static uint8_t  s_last[FRAME_MAX];
static int      s_last_w, s_last_h, s_last_n = -1;
static wl_handle_t s_wl = WL_INVALID_HANDLE;

/* ---- settings in NVS --------------------------------------------------- */

static int kv_get(const char *k, int def)
{
    nvs_handle_t h; int32_t v = def;
    if (nvs_open("scout", NVS_READONLY, &h) == ESP_OK) { nvs_get_i32(h, k, &v); nvs_close(h); }
    return (int)v;
}

static void kv_set(const char *k, int v)
{
    nvs_handle_t h;
    if (nvs_open("scout", NVS_READWRITE, &h) == ESP_OK) { nvs_set_i32(h, k, v); nvs_commit(h); nvs_close(h); }
}

/* ---- camera ------------------------------------------------------------ */

static void apply_flip(void)
{
    sensor_t *s = esp_camera_sensor_get();
    if (!s) return;
    s->set_vflip(s, s_flip & 1);
    s->set_hmirror(s, (s_flip >> 1) & 1);
}

static esp_err_t cam_init(void)
{
    /* M5Camera Model B (esp32-camera's CAMERA_MODEL_M5STACK_V2_PSRAM);
     * the factory firmware's boot log shows the same pins. */
    camera_config_t c = {
        .pin_pwdn = -1, .pin_reset = 15, .pin_xclk = 27, .pin_sccb_sda = 22, .pin_sccb_scl = 23,
        .pin_d7 = 19, .pin_d6 = 36, .pin_d5 = 18, .pin_d4 = 39, .pin_d3 = 5, .pin_d2 = 34, .pin_d1 = 35, .pin_d0 = 32,
        .pin_vsync = 25, .pin_href = 26, .pin_pclk = 21,
        .xclk_freq_hz = 10000000, .ledc_timer = LEDC_TIMER_0, .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_JPEG, .frame_size = FRAMESIZE_QVGA, .jpeg_quality = 12,
        .fb_count = 2, .fb_location = CAMERA_FB_IN_PSRAM, .grab_mode = CAMERA_GRAB_LATEST,
    };
    esp_err_t err = esp_camera_init(&c);
    if (err != ESP_OK) { ESP_LOGW(TAG, "camera init: %s", esp_err_to_name(err)); return err; }
    apply_flip();
    s_cam_up = true;
    return ESP_OK;
}

/* a fresh grey frame into s_grey; the first buffer may be stale. The sensor
 * runs in JPEG (the LAN frames are those JPEGs); this decodes one. */
static bool grab(void)
{
    if (!s_cam_up || !s_grey || !s_rgb) return false;
    camera_fb_t *fb = NULL;
    for (int i = 0; i < 2; i++) {
        if (fb) esp_camera_fb_return(fb);
        fb = esp_camera_fb_get();
        if (!fb) return false;
    }
    bool ok = fb->width == CAM_W && fb->height == CAM_H && fmt2rgb888(fb->buf, fb->len, fb->format, s_rgb);
    esp_camera_fb_return(fb);
    if (ok)                                     /* (r + 2g + b) / 4: the same for RGB or BGR order */
        for (int i = 0; i < CAM_W * CAM_H; i++)
            s_grey[i] = (uint8_t)((s_rgb[3 * i] + 2 * s_rgb[3 * i + 1] + s_rgb[3 * i + 2]) >> 2);
    return ok;
}

/* ---- render: crop, stretch, dither (from the maplet) ------------------- */

static void render(uint8_t *dst, int W, int H, int sty)
{
    static const uint8_t bayer[4][4] = { {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5} };
    float sx = (float)CAM_W / W, sy = (float)CAM_H / H;
    float sc = sx < sy ? sx : sy;                       /* cover */
    int cw = (int)(W * sc), ch = (int)(H * sc);
    int ox = (CAM_W - cw) / 2, oy = (CAM_H - ch) / 2;

    uint32_t hist[256] = { 0 }, total = 0, acc = 0;
    for (int y = oy; y < oy + ch; y += 2)
        for (int x = ox; x < ox + cw; x += 2) { hist[s_grey[y * CAM_W + x]]++; total++; }
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

    int stride = (W + 7) / 8;
    memset(dst, 0, stride * H);
    int16_t err[2][216 + 2];                            /* Floyd-Steinberg: this row and the next */
    memset(err, 0, sizeof err);
    for (int y = 0; y < H; y++) {
        int16_t *cur = err[y & 1], *nxt = err[(y + 1) & 1];
        memset(nxt, 0, sizeof err[0]);
        for (int x = 0; x < W; x++) {
            int x0 = ox + (int)(x * sc), x1 = ox + (int)((x + 1) * sc);
            int y0 = oy + (int)(y * sc), y1 = oy + (int)((y + 1) * sc);
            if (x1 <= x0) x1 = x0 + 1;
            if (y1 <= y0) y1 = y0 + 1;
            uint32_t sum = 0, n = 0;
            for (int yy = y0; yy < y1 && yy < CAM_H; yy++)
                for (int xx = x0; xx < x1 && xx < CAM_W; xx++) { sum += s_grey[yy * CAM_W + xx]; n++; }
            int v = lut[n ? sum / n : 0];
            bool ink;
            if (sty == 1) {
                ink = v < (bayer[y & 3][x & 3] + 1) * 16 - 8;
            } else {
                int want = v + cur[x + 1];
                ink = want < 128;
                int e = want - (ink ? 0 : 255);
                cur[x + 2] += e * 7 / 16;
                nxt[x]     += e * 3 / 16;
                nxt[x + 1] += e * 5 / 16;
                nxt[x + 2] += e / 16;
            }
            if (ink) dst[y * stride + (x >> 3)] |= 0x80 >> (x & 7);
        }
    }
}

/* ---- output ------------------------------------------------------------ */

/* base64 in lines of 76 (57 bytes), padding only at the end (as roll-b64) */
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

/* ---- roll on flash ----------------------------------------------------- */

static int roll_scan(int *count)
{
    int n = 0, c = 0;
    DIR *d = opendir("/roll");
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL) {
            if (!strcasestr(e->d_name, ".BIN")) continue;
            int k = atoi(e->d_name);
            if (k >= n) n = k + 1;
            c++;
        }
        closedir(d);
    }
    if (count) *count = c;
    return n;
}

static esp_err_t fs_mount(void)
{
    const esp_vfs_fat_mount_config_t mc = { .format_if_mount_failed = true, .max_files = 4, .allocation_unit_size = 4096 };
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl("/roll", "roll", &mc, &s_wl);
    if (err != ESP_OK) { ESP_LOGW(TAG, "roll mount: %s", esp_err_to_name(err)); return err; }
    s_fs_up = true;
    s_next = roll_scan(NULL);
    return ESP_OK;
}

static bool put(const char *path, const uint8_t *b, size_t n)
{
    FILE *f = fopen(path, "wb");
    if (!f) { ESP_LOGW(TAG, "%s: %s", path, strerror(errno)); return false; }
    bool ok = fwrite(b, 1, n, f) == n;
    fclose(f);
    return ok;
}

static int get(const char *path, uint8_t *b, size_t cap)
{
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    int n = (int)fread(b, 1, cap, f);
    fclose(f);
    return n;
}

/* ---- words ------------------------------------------------------------- */

static void w_caps(void)  { con_printf("caps scout shoot lv last roll sty fr web\n"); }
static void w_cam_q(void) { con_printf("cam %s OV2640 0\n", s_cam_up ? "ready" : "err"); }

static void w_shoot(void)
{
    if (!grab()) { con_printf("err no-frame\n"); return; }
    int w = FR[s_fr].w, h = FR[s_fr].h, bytes = ((w + 7) / 8) * h;
    render(s_last, w, h, s_sty);
    s_last_w = w; s_last_h = h;
    if (!s_fs_up) { s_last_n = -1; con_printf("ok -1\n"); return; }
    static uint8_t th[((TH_W + 7) / 8) * TH_H];
    render(th, TH_W, TH_H, s_sty);
    char p[32];
    int n = s_next;
    snprintf(p, sizeof p, "/roll/%04d.BIN", n);
    bool ok = put(p, s_last, bytes);
    snprintf(p, sizeof p, "/roll/%04d.TH", n);
    ok = ok && put(p, th, sizeof th);
    if (!ok) { con_printf("err roll-write\n"); return; }
    s_next = n + 1;
    s_last_n = n;
    con_printf("ok %d\n", n);
}

static void w_last(void)
{
    if (!s_last_w) {                               /* after a reboot: the newest frame on the roll */
        char p[32];
        for (int n = s_next - 1; n >= 0 && !s_last_w; n--) {
            snprintf(p, sizeof p, "/roll/%04d.BIN", n);
            int k = get(p, s_last, sizeof s_last);
            for (size_t i = 0; k > 0 && i < sizeof FR / sizeof FR[0]; i++)
                if (k == ((FR[i].w + 7) / 8) * FR[i].h) { s_last_w = FR[i].w; s_last_h = FR[i].h; s_last_n = n; }
        }
        if (!s_last_w) { con_printf("err no-shot\n"); return; }
    }
    con_printf("last %d %d\n", s_last_w, s_last_h);
    b64_lines(s_last, ((s_last_w + 7) / 8) * s_last_h);
}

static void w_lv(void)
{
    static uint8_t lv[((TH_W + 7) / 8) * TH_H];
    if (!grab()) { con_printf("err no-frame\n"); return; }
    render(lv, TH_W, TH_H, 1);                     /* Bayer: steady from frame to frame */
    con_printf("lv %d %d\n", TH_W, TH_H);
    b64_lines(lv, sizeof lv);
}

static void w_sty_store(void) { int n = POP(); if (n >= 0 && n <= 1) { s_sty = n; kv_set("sty", n); } con_printf("sty %d %s\n", s_sty, STY[s_sty]); }
static void w_fr_store(void)  { int n = POP(); if (n >= 0 && n <= 2) { s_fr = n; kv_set("fr", n); } con_printf("fr %d %d %d\n", s_fr, FR[s_fr].w, FR[s_fr].h); }
static void w_sty_q(void)     { con_printf("sty %d %s\n", s_sty, STY[s_sty]); }
static void w_fr_q(void)      { con_printf("fr %d %d %d\n", s_fr, FR[s_fr].w, FR[s_fr].h); }
static void w_flip_store(void){ s_flip = POP() & 3; kv_set("flip", s_flip); apply_flip(); con_printf("flip %d\n", s_flip); }

static void w_roll_q(void)
{
    int count = 0;
    if (s_fs_up) roll_scan(&count);
    uint64_t total = 0, freeb = 0;
    if (s_fs_up) esp_vfs_fat_info("/roll", &total, &freeb);
    con_printf("roll %d fs %u\n", count, (unsigned)(freeb / 1024));
}

static void w_roll_ls(void)
{
    DIR *d = opendir("/roll");
    if (!d) { con_printf("roll: no fs\n"); return; }
    struct dirent *e; int n = 0;
    while ((e = readdir(d)) != NULL) {
        if (strcasestr(e->d_name, ".BIN")) { con_printf("%s\n", e->d_name); n++; }
    }
    closedir(d);
    con_printf("roll: %d frames\n", n);
}

static void w_roll_b64(void)
{
    int n = POP();
    char p[32];
    snprintf(p, sizeof p, "/roll/%04d.BIN", n);
    static uint8_t buf[FRAME_MAX];
    int k = get(p, buf, sizeof buf);
    if (k <= 0) { con_printf("roll-b64: no frame %d\n", n); return; }
    b64_lines(buf, k);
}

static void w_roll_th(void)
{
    int k = POP(), n = POP();
    static uint8_t th[((TH_W + 7) / 8) * TH_H];
    char p[32];
    for (int i = n; i >= 0 && k > 0; i--) {
        snprintf(p, sizeof p, "/roll/%04d.TH", i);
        if (get(p, th, sizeof th) != (int)sizeof th) continue;
        con_printf("rth %d %d %d 1\n", i, TH_W, TH_H);
        b64_lines(th, sizeof th);
        k--;
    }
}

/* roll-format ( -- ): wipe the roll and make a fresh FAT on it */
static void w_roll_format(void)
{
    esp_err_t e = esp_vfs_fat_spiflash_format_rw_wl("/roll", "roll");
    s_next = 0; s_last_w = 0; s_last_n = -1;
    con_printf("roll-format %s\n", e == ESP_OK ? "ok" : esp_err_to_name(e));
}

/* ---- init -------------------------------------------------------------- */

void scout_cam_init(void)
{
    s_sty = kv_get("sty", 0);  if (s_sty < 0 || s_sty > 1) s_sty = 0;
    s_fr = kv_get("fr", 0);    if (s_fr < 0 || s_fr > 2) s_fr = 0;
    s_flip = kv_get("flip", 0) & 3;
    s_grey = heap_caps_malloc(CAM_W * CAM_H, MALLOC_CAP_SPIRAM);
    s_rgb = heap_caps_malloc(CAM_W * CAM_H * 3, MALLOC_CAP_SPIRAM);
    if (!s_grey || !s_rgb) ESP_LOGW(TAG, "no PSRAM for the frame");
    fs_mount();                                     /* first: a format must not run under the camera */
    cam_init();
}

void scout_register_words(void)
{
    static const struct { const char *name; void (*fn)(void); } words[] = {
        { "caps?", w_caps }, { "cam?", w_cam_q }, { "shoot", w_shoot }, { "last", w_last }, { "lv", w_lv },
        { "sty!", w_sty_store }, { "fr!", w_fr_store }, { "sty?", w_sty_q }, { "fr?", w_fr_q }, { "flip!", w_flip_store },
        { "roll?", w_roll_q }, { "roll-ls", w_roll_ls }, { "roll-b64", w_roll_b64 }, { "roll-th", w_roll_th }, { "roll-format", w_roll_format },
    };
    for (size_t i = 0; i < sizeof words / sizeof words[0]; i++) forth_register_word(words[i].name, words[i].fn);
}

bool scout_cam_up(void) { return s_cam_up; }
int  scout_roll_count(void) { int c = 0; if (s_fs_up) roll_scan(&c); return c; }
