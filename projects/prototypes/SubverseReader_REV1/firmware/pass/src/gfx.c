/*
 * gfx.c: the canvas and the little drawing the shelf and overlays need:
 * boxes, lines and a 5x7 font. Pages don't come through here; a pack's page
 * is copied into the canvas whole.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <string.h>
#include <stdlib.h>
#include "epd.h"

extern const uint8_t font5x7[95][5];
static uint8_t s_canvas[EPD_CLEN];

uint8_t *epd_canvas(void) { return s_canvas; }
void epd_clear(void) { memset(s_canvas, 0, sizeof s_canvas); }

void epd_pixel(int x, int y, int on)
{
    if (x < 0 || y < 0 || x >= EPD_CW || y >= EPD_CH) return;
    uint8_t *b = &s_canvas[y * EPD_CSTRIDE + (x >> 3)];
    uint8_t m = 0x80 >> (x & 7);
    if (on) *b |= m; else *b &= ~m;
}

void epd_box(int x, int y, int w, int h, int on)
{
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++) epd_pixel(i, j, on);
}

void epd_frame(int x, int y, int w, int h)
{
    epd_box(x, y, w, 1, 1); epd_box(x, y + h - 1, w, 1, 1);
    epd_box(x, y, 1, h, 1); epd_box(x + w - 1, y, 1, h, 1);
}

void epd_line(int x0, int y0, int x1, int y1, int on)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, e = dx + dy;
    for (;;) {
        epd_box(x0 - 1, y0 - 1, 2, 2, on);
        if (x0 == x1 && y0 == y1) return;
        int e2 = 2 * e;
        if (e2 >= dy) { e += dy; x0 += sx; }
        if (e2 <= dx) { e += dx; y0 += sy; }
    }
}

int epd_str_w(const char *s, int scale) { return (int)strlen(s) * 6 * scale; }

static int str(int x, int y, const char *s, int scale, int on)
{
    int x0 = x;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        const uint8_t *g = (c >= 0x20 && c <= 0x7E) ? font5x7[c - 0x20] : font5x7['?' - 0x20];
        for (int col = 0; col < 5; col++)
            for (int row = 0; row < 7; row++)
                if (g[col] & (1 << row)) epd_box(x + col * scale, y + row * scale, scale, scale, on);
        x += 6 * scale;
    }
    return x - x0;
}

int  epd_str(int x, int y, const char *s, int scale) { return str(x, y, s, scale, 1); }
void epd_str_inv(int x, int y, const char *s, int scale) { str(x, y, s, scale, 0); }
