/*
 * epd.h — 2.13" flexible monochrome e-paper (212x104, UC8151-class) on
 * ESP-IDF, with a landscape 1-bit canvas and a 5x7 font.
 *
 * Canvas: 212 wide x 104 tall, 27 bytes per row, MSB first, 1 = black.
 * The panel is natively 104 x 212 (13 bytes per row, 1 = white); epd_show()
 * rotates and inverts on the way out. Full refresh only, about 2 s.
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef EPD_DRIVER_H
#define EPD_DRIVER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EPD_W        212
#define EPD_H        104
#define EPD_STRIDE   27                      /* bytes per canvas row */
#define EPD_CANVAS   (EPD_STRIDE * EPD_H)    /* 2808 */
#define EPD_NATIVE   (13 * 212)              /* 2756, what the panel takes */

esp_err_t epd_init(void);
void epd_show(void);                 /* push the canvas, refresh, sleep the panel */
void epd_clear(void);                /* canvas to white */
void epd_test_pattern(void);
void epd_test_pattern_in(int x0, int w);
void epd_fill(int mode);             /* 0 white 1 black 2 h-stripes 3 v-stripes 4 checker 5 half */
void epd_knob(int which, int v);     /* bench knobs: 0 geom, 1 cdi, 2 old-frame mode, 3 lut source, 4 spi hz */         /* calibration pattern into the canvas */
uint8_t *epd_canvas(void);           /* EPD_CANVAS bytes */

void epd_pixel(int x, int y, int on);
int  epd_get(int x, int y);
void epd_line(int x0, int y0, int x1, int y1);
void epd_box(int x, int y, int w, int h, int on);
void epd_frame(int x, int y, int w, int h);
int  epd_str(int x, int y, const char *s, int len, int scale);   /* returns width */

#ifdef __cplusplus
}
#endif
#endif /* EPD_DRIVER_H */
