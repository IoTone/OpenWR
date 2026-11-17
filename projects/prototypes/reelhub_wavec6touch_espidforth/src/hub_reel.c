/*
 * hub_reel.c — see hub_reel.h.
 *
 * Drawing: the ROM TJpgDec hands out MCU blocks (RGB888) left to right, top
 * to bottom. They collect in a 16-row strip at the decoded width; when a
 * strip row of MCUs is complete, the output rows that sample from it are
 * resampled (nearest) to the fitted size and blitted in 16-row bands. Two
 * strips' worth of RAM, whatever the picture's size.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "esp32c6/rom/tjpgd.h"
#include "esp_vfs_fat.h"
#include "wear_levelling.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "hub_reel.h"
#include "hub_lcd.h"
#include "forth_core.h"
#include "console.h"

static const char *TAG = "reel";
#define ROOT      "/reel"
#define MAX_SHOTS 1500
#define LABEL_MAX 16
#define KEEP_FREE (256 * 1024)       /* drop the oldest below this */
#define POOL_SZ   4096
#define MAX_DW    480                /* widest decoded row we accept */
#define BAND      16

static uint16_t s_num[MAX_SHOTS];    /* file numbers, oldest first */
static int      s_n, s_show = -1;
static wl_handle_t s_wl = WL_INVALID_HANDLE;
static SemaphoreHandle_t s_lock;
static char     s_note1[24], s_note2[24];
static uint16_t s_note_c;

/* ---- files ---------------------------------------------------------------- */

static void path(char *out, size_t cap, int num, const char *ext) { snprintf(out, cap, ROOT "/%04d.%s", num, ext); }

static int cmp_u16(const void *a, const void *b) { return (int)*(const uint16_t *)a - (int)*(const uint16_t *)b; }

static void scan(void)
{
    s_n = 0;
    DIR *d = opendir(ROOT);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) && s_n < MAX_SHOTS) {
        int num; char ext[4];
        if (sscanf(e->d_name, "%4d.%3s", &num, ext) == 2 && !strcasecmp(ext, "JPG") && num > 0) s_num[s_n++] = (uint16_t)num;
    }
    closedir(d);
    qsort(s_num, s_n, sizeof s_num[0], cmp_u16);
}

static uint64_t free_bytes(void)
{
    uint64_t total = 0, fr = 0;
    esp_vfs_fat_info(ROOT, &total, &fr);
    return fr;
}

static void drop_oldest(void)
{
    if (!s_n) return;
    char p[24];
    path(p, sizeof p, s_num[0], "JPG"); remove(p);
    path(p, sizeof p, s_num[0], "TXT"); remove(p);
    memmove(s_num, s_num + 1, (s_n - 1) * sizeof s_num[0]);
    s_n--;
    if (s_show > 0) s_show--;
}

esp_err_t reel_init(void)
{
    s_lock = xSemaphoreCreateRecursiveMutex();
    const esp_vfs_fat_mount_config_t mc = { .format_if_mount_failed = true, .max_files = 4, .allocation_unit_size = 4096 };
    esp_err_t e = esp_vfs_fat_spiflash_mount_rw_wl(ROOT, "reel", &mc, &s_wl);
    if (e != ESP_OK) { ESP_LOGW(TAG, "mount: %s", esp_err_to_name(e)); return e; }
    scan();
    s_show = s_n - 1;
    ESP_LOGI(TAG, "%d shots, %llu KB free", s_n, free_bytes() / 1024);
    return ESP_OK;
}

int reel_count(void) { return s_n; }
int reel_showing(void) { return s_show; }

/* "SCOUT 320 240 10:42:07 8180" */
static bool read_meta(int num, char *label, int *w, int *h, char *at, int *bytes)
{
    char p[24], line[64];
    path(p, sizeof p, num, "TXT");
    FILE *f = fopen(p, "r");
    if (!f) return false;
    bool ok = fgets(line, sizeof line, f) && sscanf(line, "%16s %d %d %9s %d", label, w, h, at, bytes) == 5;
    fclose(f);
    return ok;
}

bool reel_meta(int num, char *label, int *w, int *h, char *at, int *bytes) { return read_meta(num, label, w, h, at, bytes); }
int  reel_num_at(int i) { return i >= 0 && i < s_n ? s_num[i] : -1; }
void reel_file(int num, char *out, size_t cap) { path(out, cap, num, "JPG"); }

/* tell every console what's on screen: the glasses follow it */
static void announce(void)
{
    if (s_show < 0) { con_printf("show none 0/0\n"); return; }
    char label[17] = "?", at[10] = ""; int w, h, b;
    read_meta(s_num[s_show], label, &w, &h, at, &b);
    con_printf("show %04d %s %d/%d\n", s_num[s_show], label, s_show + 1, s_n);
}

/* ---- JPEG to screen ---------------------------------------------------- */

typedef struct {
    FILE *f;
    int sw, sh;          /* decoded size */
    int dx, dy, dw, dh;  /* where on screen */
    int oy;              /* next output row */
    uint16_t *strip;     /* sw x 16 decoded rows */
    int top;             /* first source row in the strip */
    uint16_t *band;      /* dw x BAND output rows */
    int band_rows;
} draw_t;

static UINT d_in(JDEC *jd, BYTE *buf, UINT n)
{
    draw_t *d = jd->device;
    if (!buf) return fseek(d->f, n, SEEK_CUR) == 0 ? n : 0;
    return fread(buf, 1, n, d->f);
}

static void flush_band(draw_t *d)
{
    if (!d->band_rows) return;
    lcd_blit(d->dx, d->dy + d->oy - d->band_rows, d->dw, d->band_rows, d->band);
    d->band_rows = 0;
}

static UINT d_out(JDEC *jd, void *bitmap, JRECT *r)
{
    draw_t *d = jd->device;
    const BYTE *src = bitmap;
    int bw = r->right - r->left + 1;
    if (r->left == 0) d->top = r->top;
    for (int y = r->top; y <= r->bottom; y++) {
        int sy = y - d->top;
        if (sy < 0 || sy >= 16) { src += bw * 3; continue; }
        uint16_t *row = d->strip + sy * d->sw;
        for (int x = r->left; x <= r->right; x++, src += 3)
            if (x < d->sw) row[x] = lcd_rgb(src[0], src[1], src[2]);
    }
    if (r->right + 1 < d->sw) return 1;               /* the MCU row isn't finished */
    /* emit every output row whose source row lies in this strip */
    while (d->oy < d->dh) {
        int sy = d->oy * d->sh / d->dh;
        if (sy > r->bottom) break;
        int ly = sy - d->top;
        if (ly >= 0 && ly < 16) {
            const uint16_t *row = d->strip + ly * d->sw;
            uint16_t *out = d->band + d->band_rows * d->dw;
            for (int ox = 0; ox < d->dw; ox++) out[ox] = row[ox * d->sw / d->dw];
        }
        d->band_rows++; d->oy++;
        if (d->band_rows == BAND) flush_band(d);
    }
    return 1;
}

/* fit the JPEG at p into (bx, by, bw, bh), centred, background bg */
static bool draw_jpeg(const char *p, int bx, int by, int bw, int bh, uint16_t bg)
{
    draw_t d = { 0 };
    void *pool = malloc(POOL_SZ);
    d.f = fopen(p, "rb");
    bool ok = false;
    if (!pool || !d.f) goto out;
    JDEC jd;
    JRESULT r = jd_prepare(&jd, d_in, pool, POOL_SZ, &d);
    if (r != JDR_OK) { ESP_LOGW(TAG, "%s: prepare %d", p, r); goto out; }
    int scale = 0;
    while (scale < 3 && ((jd.width >> scale) > MAX_DW || (jd.width >> (scale + 1)) >= (unsigned)bw)) scale++;
    d.sw = (jd.width + (1 << scale) - 1) >> scale;
    d.sh = (jd.height + (1 << scale) - 1) >> scale;
    if (d.sw * bh > d.sh * bw) { d.dw = bw; d.dh = d.sh * bw / d.sw; }
    else                       { d.dh = bh; d.dw = d.sw * bh / d.sh; }
    d.dx = bx + (bw - d.dw) / 2;
    d.dy = by + (bh - d.dh) / 2;
    d.strip = malloc(d.sw * 16 * 2);
    d.band = malloc(d.dw * BAND * 2);
    if (!d.strip || !d.band) { ESP_LOGW(TAG, "no memory for %dx%d", d.sw, d.dw); goto out; }
    /* clear what the picture won't cover */
    if (d.dy > by) lcd_fill(bx, by, bw, d.dy - by, bg);
    if (d.dy + d.dh < by + bh) lcd_fill(bx, d.dy + d.dh, bw, by + bh - d.dy - d.dh, bg);
    if (d.dx > bx) lcd_fill(bx, d.dy, d.dx - bx, d.dh, bg);
    if (d.dx + d.dw < bx + bw) lcd_fill(d.dx + d.dw, d.dy, bx + bw - d.dx - d.dw, d.dh, bg);
    r = jd_decomp(&jd, d_out, scale);
    flush_band(&d);
    ok = r == JDR_OK;
    if (!ok) ESP_LOGW(TAG, "%s: decomp %d", p, r);
out:
    if (d.f) fclose(d.f);
    free(pool); free(d.strip); free(d.band);
    return ok;
}

/* ---- the screen ----------------------------------------------------------- */

static void draw_strip(void)
{
    const int x = STRIP_X, w = LCD_W - STRIP_X;
    lcd_fill(x, 0, w, LCD_H, C_BG);
    char b[24];
    if (s_show < 0) {
        lcd_text(x + 4, 8, "REEL", C_AMBER, C_BG, 2);
        lcd_text(x + 4, 34, "EMPTY", C_GREY, C_BG, 1);
    } else {
        char label[17] = "?", at[10] = "", sz[16] = "";
        int w0 = 0, h0 = 0, bytes = 0;
        read_meta(s_num[s_show], label, &w0, &h0, at, &bytes);
        snprintf(b, sizeof b, "%d/%d", s_show + 1, s_n);
        lcd_text(x + 4, 6, b, C_AMBER, C_BG, s_n < 100 ? 2 : 1);
        lcd_text(x + 4, strlen(label) > 5 ? 34 : 30, label, C_WHITE, C_BG, strlen(label) > 5 ? 1 : 2);
        lcd_text(x + 4, 54, at, C_GREY, C_BG, 1);
        snprintf(sz, sizeof sz, "%dx%d", w0, h0);
        lcd_text(x + 4, 66, sz, C_GREY, C_BG, 1);
        snprintf(b, sizeof b, "%d KB", (bytes + 512) / 1024);
        lcd_text(x + 4, 78, b, C_GREY, C_BG, 1);
    }
    if (s_note1[0]) lcd_text(x + 4, 120, s_note1, s_note_c, C_BG, 1);
    if (s_note2[0]) lcd_text(x + 4, 132, s_note2, s_note_c, C_BG, 1);
}

void reel_strip_note(const char *a, const char *b, uint16_t c)
{
    xSemaphoreTakeRecursive(s_lock, portMAX_DELAY);
    snprintf(s_note1, sizeof s_note1, "%s", a ? a : "");
    snprintf(s_note2, sizeof s_note2, "%s", b ? b : "");
    s_note_c = c;
    lcd_fill(STRIP_X, 118, LCD_W - STRIP_X, 24, C_BG);
    if (s_note1[0]) lcd_text(STRIP_X + 4, 120, s_note1, c, C_BG, 1);
    if (s_note2[0]) lcd_text(STRIP_X + 4, 132, s_note2, c, C_BG, 1);
    xSemaphoreGiveRecursive(s_lock);
}

void reel_redraw(void)
{
    xSemaphoreTakeRecursive(s_lock, portMAX_DELAY);
    if (s_show < 0) {
        lcd_fill(0, 0, PIC_W, LCD_H, C_BLACK);
        lcd_text(40, 62, "NO SHOTS YET", C_DIM, C_BLACK, 2);
        lcd_text(22, 94, "HOLD 3 S OR DOUBLE-TAP", C_GREY, C_BLACK, 1);
    } else {
        char p[24];
        path(p, sizeof p, s_num[s_show], "JPG");
        if (!draw_jpeg(p, 0, 0, PIC_W, LCD_H, C_BLACK)) {
            lcd_fill(0, 0, PIC_W, LCD_H, C_BLACK);
            lcd_text(60, 80, "CAN'T SHOW", C_RED, C_BLACK, 2);
        }
    }
    draw_strip();
    xSemaphoreGiveRecursive(s_lock);
}

void reel_show(int i)
{
    if (i < 0 || i >= s_n) return;
    s_show = i;
    reel_redraw();
    announce();
}

void reel_step(int dir)
{
    if (!s_n) return;
    int i = s_show + dir;
    if (i < 0) i = 0;
    if (i >= s_n) i = s_n - 1;
    if (i != s_show) reel_show(i);
}

static void jpeg_size(const uint8_t *j, size_t n, int *w, int *h)
{
    *w = *h = 0;
    for (size_t i = 2; i + 9 < n; ) {           /* walk the markers to SOF0..SOF2 */
        if (j[i] != 0xFF) { i++; continue; }
        uint8_t m = j[i + 1];
        if (m >= 0xC0 && m <= 0xC2) { *h = (j[i + 5] << 8) | j[i + 6]; *w = (j[i + 7] << 8) | j[i + 8]; return; }
        if (m == 0xD8 || m == 0x01 || (m >= 0xD0 && m <= 0xD7)) { i += 2; continue; }
        i += 2 + ((j[i + 2] << 8) | j[i + 3]);
    }
}

int reel_add(const uint8_t *jpg, size_t n, const char *label)
{
    xSemaphoreTakeRecursive(s_lock, portMAX_DELAY);
    while (s_n && (free_bytes() < KEEP_FREE + n || s_n >= MAX_SHOTS)) drop_oldest();
    int num = s_n ? s_num[s_n - 1] + 1 : 1;
    if (num > 9999) num = 1;                 /* 8.3 names; wraps after 9999 shots */
    char p[24];
    path(p, sizeof p, num, "JPG");
    FILE *f = fopen(p, "wb");
    size_t put = f ? fwrite(jpg, 1, n, f) : 0;
    if (f) fclose(f);
    if (put != n) { remove(p); xSemaphoreGiveRecursive(s_lock); ESP_LOGW(TAG, "write %s failed", p); return -1; }
    int w, h; jpeg_size(jpg, n, &w, &h);
    time_t t = time(NULL); struct tm tm; localtime_r(&t, &tm);
    path(p, sizeof p, num, "TXT");
    f = fopen(p, "w");
    if (f) { fprintf(f, "%s %d %d %02d:%02d:%02d %u\n", label, w, h, tm.tm_hour, tm.tm_min, tm.tm_sec, (unsigned)n); fclose(f); }
    s_num[s_n++] = (uint16_t)num;
    s_show = s_n - 1;
    reel_redraw();
    announce();
    xSemaphoreGiveRecursive(s_lock);
    return num;
}

/* ---- words ------------------------------------------------------------- */

static void w_reel_q(void) { con_printf("reel %d free %llu showing %d\n", s_n, free_bytes() / 1024, s_show + 1); }
static void w_reel_ls(void)
{
    for (int i = 0; i < s_n; i++) {
        char label[17] = "?", at[10] = "?"; int w = 0, h = 0, b = 0;
        read_meta(s_num[i], label, &w, &h, at, &b);
        con_printf("%d %s %dx%d %s %d\n", i + 1, label, w, h, at, b);
    }
    con_printf("reel %d\n", s_n);
}
static void w_reel_show(void) { reel_show((int)forth_pop() - 1); }
static void w_show_q(void) { announce(); }
static void w_reel_wipe(void)
{
    xSemaphoreTakeRecursive(s_lock, portMAX_DELAY);
    while (s_n) drop_oldest();
    s_show = -1;
    reel_redraw();
    xSemaphoreGiveRecursive(s_lock);
    con_printf("reel 0\n");
    announce();
}

void reel_register_words(void)
{
    forth_register_word("reel?", w_reel_q);
    forth_register_word("reel-ls", w_reel_ls);
    forth_register_word("reel-show", w_reel_show);
    forth_register_word("reel-wipe", w_reel_wipe);
    forth_register_word("show?", w_show_q);
}
