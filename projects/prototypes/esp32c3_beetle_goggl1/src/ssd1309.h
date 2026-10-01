/*
 * ssd1309.h — a small SSD1309 128x64 driver for ESP-IDF with a 1 KB
 * framebuffer and the handful of primitives the DFRobot U8g2 demos use.
 *
 * Coordinates: (0,0) is top-left, x to the right, y down. Text is anchored
 * at its top-left corner (U8g2's setFontPosTop behaviour), because that is
 * what every DFRobot demo for this module sets.
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef SSD1309_H
#define SSD1309_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1309_WIDTH   128
#define SSD1309_HEIGHT  64

/* Draw colour, same numbering as U8g2's setDrawColor */
enum {
    OLED_COLOR_CLEAR = 0,
    OLED_COLOR_SET   = 1,
    OLED_COLOR_XOR   = 2,
};

typedef struct {
    int spi_host;
    int pin_sclk, pin_mosi, pin_dc, pin_rst, pin_cs;
    int clock_hz;       /* 0 = 4 MHz, the figure U8g2 uses for this part */
    int x_offset;       /* column offset; 2 for the DFRobot "NONAME2" module */
} ssd1309_config_t;

/* ---- device ---- */
esp_err_t oled_init(const ssd1309_config_t *cfg);
void oled_show(void);                 /* push the framebuffer to the panel */
void oled_power(bool on);             /* 0xAF / 0xAE */
void oled_contrast(uint8_t level);    /* 0..255 */
void oled_invert(bool inverted);      /* 0xA7 / 0xA6 */
void oled_flip(bool flipped);         /* rotate 180° via segment/scan remap */

/* ---- framebuffer state ---- */
void oled_clear(void);
void oled_set_color(int color);       /* OLED_COLOR_* */
int  oled_get_color(void);
uint8_t *oled_framebuffer(void);      /* 128*64/8 bytes, page-major */

/* ---- primitives ---- */
void oled_pixel(int x, int y);
int  oled_get_pixel(int x, int y);
void oled_hline(int x, int y, int w);
void oled_vline(int x, int y, int h);
void oled_line(int x0, int y0, int x1, int y1);
void oled_frame(int x, int y, int w, int h);
void oled_box(int x, int y, int w, int h);
void oled_rframe(int x, int y, int w, int h, int r);
void oled_rbox(int x, int y, int w, int h, int r);
void oled_circle(int cx, int cy, int r);
void oled_disc(int cx, int cy, int r);
void oled_triangle(int x0, int y0, int x1, int y1, int x2, int y2);

/* XBM bitmap: rows padded to whole bytes, LSB is the leftmost pixel.
 * Transparent mode draws only the 1 bits; solid mode also clears the 0s. */
void oled_xbm(int x, int y, int w, int h, const uint8_t *bits, bool transparent);

/* ---- text (built-in 5x7 font, 6 px advance, scalable, 4 directions) ---- */
void oled_set_scale(int scale);       /* 1..4, both axes */
void oled_set_scale_xy(int sx, int sy); /* independent, e.g. 1 wide x 2 tall */
void oled_set_dir(int dir);           /* 0=→ 1=↓ 2=← 3=↑ like setFontDirection */
int  oled_glyph(int x, int y, char c);                 /* returns advance */
int  oled_str(int x, int y, const char *s, int len);   /* returns width drawn */
int  oled_str_width(int len);

#ifdef __cplusplus
}
#endif
#endif /* SSD1309_H */
