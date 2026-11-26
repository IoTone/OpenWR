/*
 * epd.h: the pass's glass behind a canvas the reader sees upright.
 *
 * Canvas: EPD_CW x EPD_CH (board.h), EPD_CSTRIDE bytes a row, MSB first,
 * 1 = black ink (the .svpk page format, so a page copies straight in).
 * epd_rot() picks how the canvas lands on the glass; it is kept in NVS so it
 * can be fixed from the console once (epd-rot!).
 *
 * Drivers: epd_crowpanel42.c (SR-15: SSD1683 or UC81xx, 400 x 300),
 * epd_uc8253.c (SR-16: 240 x 416). Drawing is in gfx.c.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef PASS_EPD_H
#define PASS_EPD_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "board.h"

esp_err_t epd_init(void);
uint8_t  *epd_canvas(void);
void epd_clear(void);
void epd_set_rot(int r);                  /* 0..3 */
int  epd_rot(void);

/* Push the canvas. full: flash-clean refresh (1.5-3 s). Otherwise a quick
 * refresh (~0.4 s) against the last picture shown. */
void epd_show(bool full);
/* After a reset the controller no longer knows what is on the glass; hand it
 * the picture that is there (1 = black, canvas layout) before a quick refresh. */
void epd_set_previous(const uint8_t *canvas);
void epd_sleep(void);                     /* controller deep sleep; epd_init wakes it */
void epd_hold_for_sleep(void);            /* keep the panel's lines steady through the chip's deep sleep */
void epd_power(bool on);
const char *epd_ctl_name(void);           /* "SSD1683", "UC81xx", "UC8253", or "none" */
void epd_set_ctl(int c);                  /* SR-15: 0 auto-detect, 1 SSD1683, 2 UC81xx (NVS) */
int  epd_busy_pin(void);                  /* the GPIO the driver reads BUSY on; -1 if no panel answered */

void epd_pixel(int x, int y, int on);
void epd_box(int x, int y, int w, int h, int on);
void epd_frame(int x, int y, int w, int h);
void epd_line(int x0, int y0, int x1, int y1, int on);   /* 2 px pen */
int  epd_str(int x, int y, const char *s, int scale);    /* 5x7 font; returns width */
int  epd_str_w(const char *s, int scale);
void epd_str_inv(int x, int y, const char *s, int scale); /* white letters: clears pixels */

#endif
