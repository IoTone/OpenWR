/*
 * pack.c: .svpk v0. A 256-byte header, a 12-byte entry a page, then pages as
 * 1-bit rows, MSB first, 1 = black: raw, PackBits, or DEFLATE (RFC 1951, no
 * zlib wrapper; about 40% smaller than PackBits on real zines). Nothing is held open
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

/* ---------------------------------------------------------------- inflate
 * A small RFC 1951 decoder. The whole page is decoded into one buffer, so
 * back-references read straight from the output: no window to keep. Codes
 * are decoded a bit at a time from canonical tables, which is slow per bit
 * and plenty for a 15 KB page. */
typedef struct { const uint8_t *in; int inlen, pos; uint32_t bits; int nbits; uint8_t *out; int outlen, o; } inf_t;
typedef struct { uint16_t count[16], symbol[288]; } huff_t;

static int getbits(inf_t *s, int n)
{
    while (s->nbits < n) {
        if (s->pos >= s->inlen) return -1;
        s->bits |= (uint32_t)s->in[s->pos++] << s->nbits;
        s->nbits += 8;
    }
    int v = s->bits & ((1u << n) - 1);
    s->bits >>= n; s->nbits -= n;
    return v;
}

static void huff_build(huff_t *h, const uint8_t *len, int n)
{
    uint16_t offs[16];
    memset(h->count, 0, sizeof h->count);
    for (int i = 0; i < n; i++) h->count[len[i]]++;
    h->count[0] = 0;
    offs[1] = 0;
    for (int l = 1; l < 15; l++) offs[l + 1] = offs[l] + h->count[l];
    for (int i = 0; i < n; i++) if (len[i]) h->symbol[offs[len[i]]++] = i;
}

static int huff_decode(inf_t *s, const huff_t *h)
{
    int code = 0, first = 0, index = 0;
    for (int l = 1; l <= 15; l++) {
        int b = getbits(s, 1);
        if (b < 0) return -1;
        code |= b;
        int count = h->count[l];
        if (code - count < first) return h->symbol[index + (code - first)];
        index += count; first += count;
        first <<= 1; code <<= 1;
    }
    return -1;
}

static bool inf_codes(inf_t *s, const huff_t *lc, const huff_t *dc)
{
    static const uint16_t lbase[29] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258 };
    static const uint8_t  lext[29]  = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0 };
    static const uint16_t dbase[30] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };
    static const uint8_t  dext[30]  = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
    for (;;) {
        int sym = huff_decode(s, lc);
        if (sym < 0) return false;
        if (sym < 256) { if (s->o >= s->outlen) return false; s->out[s->o++] = (uint8_t)sym; continue; }
        if (sym == 256) return true;
        sym -= 257;
        if (sym >= 29) return false;
        int e = getbits(s, lext[sym]);
        if (e < 0) return false;
        int len = lbase[sym] + e;
        int ds = huff_decode(s, dc);
        if (ds < 0 || ds >= 30) return false;
        e = getbits(s, dext[ds]);
        if (e < 0) return false;
        int dist = dbase[ds] + e;
        if (dist > s->o || s->o + len > s->outlen) return false;
        for (; len; len--, s->o++) s->out[s->o] = s->out[s->o - dist];
    }
}

/* Returns the bytes written, or -1 if the stream is damaged. Not re-entrant
 * (the tables are static): callers hold the reader lock. */
static int inflate_raw(const uint8_t *in, int inlen, uint8_t *out, int outlen)
{
    static huff_t lc, dc;
    static uint8_t lens[320];
    inf_t s = { .in = in, .inlen = inlen, .out = out, .outlen = outlen };
    int last;
    do {
        last = getbits(&s, 1);
        int type = getbits(&s, 2);
        if (last < 0 || type < 0) return -1;
        if (type == 0) {
            s.bits = 0; s.nbits = 0;
            if (s.pos + 4 > inlen) return -1;
            int len = in[s.pos] | in[s.pos + 1] << 8;
            if ((len ^ 0xFFFF) != (in[s.pos + 2] | in[s.pos + 3] << 8)) return -1;
            s.pos += 4;
            if (s.pos + len > inlen || s.o + len > outlen) return -1;
            memcpy(out + s.o, in + s.pos, len);
            s.pos += len; s.o += len;
        } else if (type == 1) {
            int i = 0;
            for (; i < 144; i++) lens[i] = 8;
            for (; i < 256; i++) lens[i] = 9;
            for (; i < 280; i++) lens[i] = 7;
            for (; i < 288; i++) lens[i] = 8;
            huff_build(&lc, lens, 288);
            for (i = 0; i < 30; i++) lens[i] = 5;
            huff_build(&dc, lens, 30);
            if (!inf_codes(&s, &lc, &dc)) return -1;
        } else if (type == 2) {
            static const uint8_t order[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
            int nlen = getbits(&s, 5), ndist = getbits(&s, 5), ncode = getbits(&s, 4);
            if (nlen < 0 || ndist < 0 || ncode < 0) return -1;
            nlen += 257; ndist += 1; ncode += 4;
            if (nlen > 286 || ndist > 30) return -1;
            memset(lens, 0, 19);
            for (int i = 0; i < ncode; i++) { int v = getbits(&s, 3); if (v < 0) return -1; lens[order[i]] = v; }
            huff_build(&lc, lens, 19);
            int i = 0;
            while (i < nlen + ndist) {
                int sym = huff_decode(&s, &lc), rep, v = 0;
                if (sym < 0) return -1;
                if (sym < 16) { lens[i++] = sym; continue; }
                if (sym == 16) { if (!i) return -1; v = lens[i - 1]; rep = getbits(&s, 2); if (rep < 0) return -1; rep += 3; }
                else if (sym == 17) { rep = getbits(&s, 3); if (rep < 0) return -1; rep += 3; }
                else { rep = getbits(&s, 7); if (rep < 0) return -1; rep += 11; }
                if (i + rep > nlen + ndist) return -1;
                while (rep--) lens[i++] = v;
            }
            static uint8_t dl[32];
            memcpy(dl, lens + nlen, ndist);
            huff_build(&lc, lens, nlen);
            huff_build(&dc, dl, ndist);
            if (!inf_codes(&s, &lc, &dc)) return -1;
        } else {
            return -1;
        }
    } while (!last);
    return s.o;
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
        memset(raw, 0, plen);
        if (comp == 1) unpackbits(blob, len, raw, plen);
        else if (comp == 2) ok = inflate_raw(blob, len, raw, plen) == plen;
        else if (comp == 0) memcpy(raw, blob, len < (uint32_t)plen ? len : (uint32_t)plen);
        else ok = false;
    }
    if (ok) {
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
