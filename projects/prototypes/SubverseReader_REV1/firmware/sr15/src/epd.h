/*
 * epd.h: the SSD1683 4.2in 400x300 panel behind a portrait canvas.
 *
 * Canvas: EPD_CW x EPD_CH (300 x 400 portrait), EPD_CSTRIDE bytes a row,
 * MSB first, 1 = black ink (the .svpk page format, so a page copies straight
 * in). epd_rot() picks how the canvas lands on the landscape glass; it is
 * kept in NVS so it can be fixed from the console once (epd-rot!).
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef SR15_EPD_H
#define SR15_EPD_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#define EPD_NW       400                  /* native: source pixels a row */
#define EPD_NH       300                  /* native: gate rows */
#define EPD_NSTRIDE  (EPD_NW / 8)
#define EPD_CW       300                  /* portrait canvas */
#define EPD_CH       400
#define EPD_CSTRIDE  ((EPD_CW + 7) / 8)
#define EPD_CLEN     (EPD_CSTRIDE * EPD_CH)

esp_err_t epd_init(void);
uint8_t  *epd_canvas(void);
void epd_clear(void);
void epd_set_rot(int r);                  /* 0..3; 1 and 3 are portrait */
int  epd_rot(void);

/* Push the canvas. full: flash-clean refresh (~3 s). Otherwise a partial
 * refresh (~0.4 s) against the last picture shown. */
void epd_show(bool full);
/* After a reset the controller no longer knows what is on the glass; hand it
 * the picture that is there (1 = black, canvas layout) before a partial. */
void epd_set_previous(const uint8_t *canvas);
void epd_sleep(void);                     /* controller deep sleep; epd_init wakes it */
void epd_power(bool on);
const char *epd_ctl_name(void);           /* "SSD1683" or "UC81xx" */
void epd_set_ctl(int c);                  /* 0 auto-detect, 1 SSD1683, 2 UC81xx (NVS) */

void epd_pixel(int x, int y, int on);
void epd_box(int x, int y, int w, int h, int on);
void epd_frame(int x, int y, int w, int h);
int  epd_str(int x, int y, const char *s, int scale);   /* 5x7 font; returns width */
int  epd_str_w(const char *s, int scale);

#endif
