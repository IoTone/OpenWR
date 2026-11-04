/*
 * display.h — AlphaFriend's status screen on the StickC's 135x240 ST7789,
 * drawn in landscape: Wi-Fi, camera, glasses link, and a message line, with
 * an alpha logo at the right: red while the camera is not linked, orange and
 * breathing while it is.
 *
 * The StickC Plus powers the panel and its backlight from an AXP192 (I2C
 * 0x34, SDA 21 / SCL 22): nothing lights until LDO2/LDO3 are switched on.
 * A board without the AXP192 (Plus2 layout) gets its backlight on GPIO27.
 *
 *   msg!   ( addr u -- )   show a line of text at the bottom of the screen
 *   disp?  ( -- )          disp <axp192|gpio27|off>
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include <stdint.h>

bool display_init(void);
void display_pulse(uint32_t ms);         /* the alpha logo; call every ~100 ms */
void display_tick(void);                 /* redraw what changed; call about once a second */
void display_msg(const char *s);
void display_register_words(void);

#endif
