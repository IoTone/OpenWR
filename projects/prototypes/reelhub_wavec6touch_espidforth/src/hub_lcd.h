/*
 * hub_lcd.h — the 1.47" JD9853 on its side: 320x172, landscape.
 *
 * No framebuffer (172*320*2 = 110 KB of a 512 KB part that also holds Wi-Fi
 * and BLE): everything draws through small band buffers blitted straight to
 * the panel. Pixels are RGB565, byte-swapped for the panel (lcd_rgb()).
 *
 * The board can sit either way round; `rot!` picks one of four
 * (swap + mirror) settings and saves it, and the touch layer follows it.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef HUB_LCD_H
#define HUB_LCD_H
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#define LCD_W 320
#define LCD_H 172

uint16_t lcd_rgb(uint8_t r, uint8_t g, uint8_t b);
#define C_BLACK  lcd_rgb(0, 0, 0)
#define C_BG     lcd_rgb(0x08, 0x08, 0x0C)
#define C_AMBER  lcd_rgb(0xF2, 0xA3, 0x3A)
#define C_DIM    lcd_rgb(0x46, 0x2E, 0x10)
#define C_WHITE  lcd_rgb(0xFF, 0xFF, 0xFF)
#define C_GREY   lcd_rgb(0x60, 0x60, 0x68)
#define C_RED    lcd_rgb(0xFF, 0x4D, 0x5A)
#define C_GREEN  lcd_rgb(0x5A, 0xFF, 0x8A)

esp_err_t lcd_init(void);
int  lcd_rot(void);                 /* 0..3 */
void lcd_set_rot(int r);            /* applies and saves */
void lcd_blit(int x, int y, int w, int h, const uint16_t *px);   /* waits for the DMA */
void lcd_fill(int x, int y, int w, int h, uint16_t c);
void lcd_clear(uint16_t c);
void lcd_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale);
int  lcd_text_w(const char *s, int scale);
void lcd_disc(int cx, int cy, int r, uint16_t c, int ring, uint16_t ring_c, uint16_t bg);
void lcd_backlight(int pct);        /* clamped to 50 % (Waveshare: more burns the panel) */
void lcd_testcard(void);

#endif
