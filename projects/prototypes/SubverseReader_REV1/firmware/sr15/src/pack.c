/*
 * pack.c: .svpk v0. A 256-byte header, a 12-byte entry a page, then pages as
 * 1-bit rows, MSB first, 1 = black, raw or PackBits. Nothing is held open
 * between pages: each turn opens the file, seeks, reads one page and closes,
 * so pulling the card or sleeping never leaves a stale handle.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "esp_log.h"
#include "pack.h"

static const char *TAG = "pack";

static uint16_t u16(const uint8_t *b) { return b[0] | b[1] << 8; }
static uint32_t u32(const uint8_t *b) { return b[0] | b[1] << 8 | b[2] << 16 | (uint32_t)b[3] << 24; }

static void str_field(char *dst, const uint8_t *src, int n)
{
    memcpy(dst, src, n);
    dst[n - 1] = 0;
}

bool pack_open(pack_t *p, const char *path, char *err, int errlen)
{
    memset(p, 0, sizeof *p);
    snprintf(p->path, sizeof p->path, "%s", path);
    FILE *f = fopen(path, "rb");
    if (!f) { snprintf(err, errlen, "can't open"); return false; }
    uint8_t h[256];
    size_t n = fread(h, 1, sizeof h, f);
    fclose(f);
    if (n != sizeof h || memcmp(h, "SVPK", 4)) { snprintf(err, errlen, "not a pack"); return false; }
    if (u16(h + 4) != 0) { snprintf(err, errlen, "pack v%u, this firmware reads v0", u16(h + 4)); return false; }
    p->w = u16(h + 8); p->h = u16(h + 10); p->bpp = h[12];
    p->pages = u32(h + 16); p->table_off = u32(h + 20);
    str_field(p->title, h + 60, 64);
    str_field(p->author, h + 124, 64);
    if (p->bpp != 1) { snprintf(err, errlen, "%u-bit pages", p->bpp); return false; }
    if (!p->title[0]) {
        const char *b = strrchr(path, '/');
        snprintf(p->title, sizeof p->title, "%s", b ? b + 1 : path);
    }
    return true;
}

static int unpackbits(const uint8_t *in, int inlen, uint8_t *out, int outlen)
{
    int i = 0, o = 0;
    while (i < inlen && o < outlen) {
        int c = in[i++];
        if (c < 128) {
            int k = c + 1;
            if (i + k > inlen) k = inlen - i;
            if (o + k > outlen) k = outlen - o;
            memcpy(out + o, in + i, k); i += c + 1; o += k;
        } else if (c > 128) {
            int k = 257 - c;
            if (i >= inlen) break;
            if (o + k > outlen) k = outlen - o;
            memset(out + o, in[i++], k); o += k;
        }
    }
    return o;
}

/* The page is p->w x p->h; the canvas is out_w x out_h (stride from out_w).
 * A page of another size is centred and clipped, so a pack built for SR-16
 * still shows on SR-15, just not edge to edge. */
bool pack_page(const pack_t *p, uint32_t n, uint8_t *out, int outlen)
{
    extern const int g_canvas_w, g_canvas_h;
    if (n >= p->pages) return false;
    FILE *f = fopen(p->path, "rb");
    if (!f) return false;
    uint8_t e[12];
    bool ok = fseek(f, p->table_off + 12 * n, SEEK_SET) == 0 && fread(e, 1, 12, f) == 12;
    uint32_t off = u32(e), len = u32(e + 4);
    uint8_t comp = e[8];
    int pstride = (p->w + 7) / 8, plen = pstride * p->h;
    uint8_t *blob = ok ? malloc(len) : NULL, *raw = malloc(plen);
    ok = ok && blob && raw && fseek(f, off, SEEK_SET) == 0 && fread(blob, 1, len, f) == len;
    fclose(f);
    if (ok) {
        if (comp == 1) unpackbits(blob, len, raw, plen);
        else memcpy(raw, blob, len < (uint32_t)plen ? len : (uint32_t)plen);
        int cw = g_canvas_w, ch = g_canvas_h, cstride = (cw + 7) / 8;
        memset(out, 0, outlen);
        if (p->w == cw && p->h == ch && plen <= outlen) {
            memcpy(out, raw, plen);
        } else {
            int dx = (cw - p->w) / 2, dy = (ch - p->h) / 2;
            for (int y = 0; y < p->h; y++) {
                int cy = y + dy;
                if (cy < 0 || cy >= ch) continue;
                for (int x = 0; x < p->w; x++) {
                    int cx = x + dx;
                    if (cx < 0 || cx >= cw) continue;
                    if (raw[y * pstride + (x >> 3)] & (0x80 >> (x & 7))) out[cy * cstride + (cx >> 3)] |= 0x80 >> (cx & 7);
                }
            }
        }
    } else {
        ESP_LOGW(TAG, "%s: page %lu unreadable", p->path, (unsigned long)n);
    }
    free(blob); free(raw);
    return ok;
}

static int cmp(const void *a, const void *b) { return strcasecmp(a, b); }

int pack_scan(const char *dir, char (*paths)[96], int max)
{
    DIR *d = opendir(dir);
    if (!d) return 0;
    int n = 0;
    struct dirent *de;
    while ((de = readdir(d)) && n < max) {
        const char *dot = strrchr(de->d_name, '.');
        if (de->d_name[0] == '.' || !dot || strcasecmp(dot, ".svpk")) continue;
        snprintf(paths[n++], 96, "%.30s/%.64s", dir, de->d_name);
    }
    closedir(d);
    qsort(paths, n, 96, cmp);
    return n;
}
