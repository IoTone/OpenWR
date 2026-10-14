/*
 * epd.h — e-paper panels behind one canvas. A driver table (epd.c) holds
 * each panel's size and its init / write / refresh / sleep; the active
 * driver is chosen at run time (epd-driver!) and remembered in NVS.
 *
 * Canvas: landscape, epd_w() x epd_h(), epd_stride() bytes a row, MSB
 * first, 1 = black. epd_show() rotates and inverts into the panel's native
 * frame. Full refresh only.
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

/* The biggest panel in the table sizes the static buffers. */
#define EPD_W_MAX      296
#define EPD_H_MAX      128
#define EPD_CANVAS_MAX (((EPD_W_MAX + 7) / 8) * EPD_H_MAX)   /* 4736 */

/* The active panel; these read like the old constants but follow the driver. */
int epd_w(void);
int epd_h(void);
int epd_stride(void);
int epd_canvas_len(void);
#define EPD_W        epd_w()
#define EPD_H        epd_h()
#define EPD_STRIDE   epd_stride()
#define EPD_CANVAS   epd_canvas_len()

/* Driver table */
int  epd_driver_count(void);
int  epd_driver_current(void);
const char *epd_driver_name(int i);
const char *epd_driver_desc(int i);
int  epd_driver_find(const char *name, int len);   /* index or -1 */
void epd_driver_set(int i);                        /* switch now; clears the canvas */
void epd_set_rot(int r);                           /* 0..3, see to_native() */
int  epd_rot(void);

esp_err_t epd_init(void);
void epd_show(void);                 /* push the canvas, refresh, sleep the panel */
void epd_clear(void);                /* canvas to white */
void epd_test_pattern(void);
void epd_test_pattern_in(int x0, int w);
void epd_fill(int mode);             /* 0 white 1 black 2 h-stripes 3 v-stripes 4 checker 5 half */
void epd_knob(int which, int v);     /* bench knobs: 0 geom, 1 cdi, 2 old-frame mode, 3 lut source (UC8151); 4 spi hz, 5 cs per byte */
uint8_t *epd_canvas(void);           /* EPD_CANVAS bytes (EPD_CANVAS_MAX allocated) */

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
