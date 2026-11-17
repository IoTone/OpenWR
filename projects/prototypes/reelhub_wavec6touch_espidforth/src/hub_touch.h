/*
 * hub_touch.h — the AXS5106L (I2C 0x63, SDA 18, SCL 19, RST 20, INT 21)
 * turned into gestures in screen coordinates (320x172, as hub_lcd).
 *
 *   TAP      a short touch that didn't move
 *   DOUBLE   a second tap within 350 ms of the first, near it
 *   SWIPE    moved > 40 px, mostly sideways: dir -1 left, +1 right
 *   HOLDING  still down and still: ms so far, every ~100 ms after 400 ms
 *   HOLD     still down and still for 3 s (once per touch)
 *   UP       the touch ended (after HOLDING, so a progress bar can go)
 *
 * The panel reports in its own portrait frame; `n touch-map!` (bits: 1 swap,
 * 2 flip x, 4 flip y; saved) maps it onto the screen. `touch-cal` finds it:
 * tap the three crosses it shows. `touch-log` prints every gesture.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef HUB_TOUCH_H
#define HUB_TOUCH_H
#include "esp_err.h"

typedef enum { T_TAP, T_DOUBLE, T_SWIPE, T_HOLDING, T_HOLD, T_UP } touch_kind_t;
typedef struct { touch_kind_t kind; int x, y, dir, ms; } touch_ev_t;
typedef void (*touch_cb_t)(const touch_ev_t *e);

#define HOLD_MS 3000

esp_err_t touch_init(touch_cb_t cb);
void touch_register_words(void);

#endif
