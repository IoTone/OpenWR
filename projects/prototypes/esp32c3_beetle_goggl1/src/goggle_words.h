/*
 * goggle_words.h — ESPIDFORTH vocabulary for the goggl1 prototype:
 * DFRobot Beetle ESP32-C3 + Fermion SSD1309 transparent OLED.
 *
 * Usage from app_main(), after forth_init() and before forth_repl():
 *
 *     forth_init(heap_size);
 *     goggle_init();               // SPI + display bring-up, LED, button
 *     goggle_register_words();     // adds the words below
 *     forth_repl(getchar_fn, putchar_fn);
 *
 * Display — framebuffer, nothing reaches the glass until oled-show:
 *   oled-clear     ( -- )                 clear the framebuffer
 *   oled-show      ( -- )                 push framebuffer to the panel
 *   oled-color     ( n -- )               0 clear, 1 set, 2 xor (U8g2 setDrawColor)
 *   oled-pixel     ( x y -- )
 *   oled-line      ( x0 y0 x1 y1 -- )
 *   oled-hline     ( x y w -- )
 *   oled-vline     ( x y h -- )
 *   oled-frame     ( x y w h -- )         hollow rectangle (drawFrame)
 *   oled-box       ( x y w h -- )         filled rectangle (drawBox)
 *   oled-rframe    ( x y w h r -- )       rounded hollow rectangle (drawRFrame)
 *   oled-rbox      ( x y w h r -- )       rounded filled rectangle (drawRBox)
 *   oled-circle    ( x y r -- )           drawCircle
 *   oled-disc      ( x y r -- )           drawDisc
 *   oled-triangle  ( x0 y0 x1 y1 x2 y2 -- ) filled (drawTriangle)
 *   oled-xbm       ( addr w h x y -- )    XBM bitmap from memory, 1-bits only
 *   oled-xbm-solid ( addr w h x y -- )    XBM bitmap, 0-bits clear too
 *   oled-pixel@    ( x y -- 0|1 )         read a framebuffer pixel
 *
 * Text — built-in 5x7 font, 6 px advance, top-left anchored:
 *   oled-str       ( c-addr u x y -- )    draw a string (drawStr)
 *   oled-glyph     ( c x y -- )           draw one character
 *   oled-scale     ( n -- )               text scale 1..4, both axes
 *   oled-scale-xy  ( sx sy -- )           independent x / y scale, 1..4 each
 *   oled-dir       ( n -- )               0 → 1 ↓ 2 ← 3 ↑ (setFontDirection)
 *   oled-strw      ( u -- w )             pixel width of u chars at current scale
 *
 * Panel control:
 *   oled-on        ( flag -- )            display on/off (setPowerSave inverse)
 *   oled-contrast  ( n -- )               0..255
 *   oled-invert    ( flag -- )
 *   oled-flip      ( flag -- )            rotate 180°
 *   oled-test      ( -- )                 draw the layout test pattern (the power-on screen)
 *   oled-splash    ( -- )                 draw the goggl1 splash
 *   oled-width     ( -- 128 )
 *   oled-height    ( -- 64 )
 *
 * Board:
 *   led            ( flag -- )            onboard LED (GPIO10)
 *   button?        ( -- flag )            BOOT button pressed (GPIO9, active low)
 *
 * Timing and maths for the demos:
 *   ms             ( n -- )               delay
 *   us             ( -- t )               microseconds since boot (wraps ~71 min)
 *   sin1k          ( deg -- n )           sin(deg) * 1000, integer degrees
 *   cos1k          ( deg -- n )           cos(deg) * 1000
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef GOGGLE_WORDS_H
#define GOGGLE_WORDS_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bring up the SPI bus, the display, the LED and the button.
 * The REPL is still usable if this fails; the oled-* words just do nothing. */
esp_err_t goggle_init(void);

/* Register the vocabulary. Call after forth_init(). */
void goggle_register_words(void);

/* The goggl1 splash: two text lines in a rounded frame. */
void goggle_splash(const char *line1, const char *line2);

/* The layout test pattern drawn at power-on: frame, ticks, labels, centre
 * crosshair, the 56-row window edge, and pixel-level checks. */
void goggle_test_pattern(void);

#ifdef __cplusplus
}
#endif
#endif /* GOGGLE_WORDS_H */
