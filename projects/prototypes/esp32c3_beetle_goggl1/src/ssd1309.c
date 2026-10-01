/*
 * ssd1309.c — SSD1309 over 4-wire SPI on ESP-IDF, plus a tiny monochrome
 * graphics library.
 *
 * The init sequence is the one U8g2 sends for its SSD1309_128X64_NONAME
 * constructors (u8x8_d_ssd1309.c), which is what the DFRobot demos use:
 *
 *   U8G2_SSD1309_128X64_NONAME2_1_4W_HW_SPI u8g2(U8G2_R0, cs, dc, reset);
 *
 * The "NONAME2" variant differs from NONAME0 only in a 2-column offset
 * (default_x_offset = 2), which is why there is an x_offset in the config.
 * Get that wrong and the image is shifted two pixels with a wrapped stripe.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ssd1309.h"

static const char *TAG = "ssd1309";

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */

static spi_device_handle_t s_spi = NULL;
static ssd1309_config_t    s_cfg;
static uint8_t  s_fb[SSD1309_WIDTH * SSD1309_HEIGHT / 8];
static int      s_color = OLED_COLOR_SET;
static int      s_scale_x = 1;
static int      s_scale_y = 1;
static int      s_dir   = 0;

/* ------------------------------------------------------------------ */
/* Bus                                                                 */
/* ------------------------------------------------------------------ */

static void spi_write(bool is_data, const uint8_t *buf, size_t len)
{
    if (!s_spi || len == 0) return;
    gpio_set_level(s_cfg.pin_dc, is_data ? 1 : 0);
    spi_transaction_t t = {
        .length    = len * 8,
        .tx_buffer = buf,
    };
    /* Polling keeps the DC line stable for the whole transfer without a
     * pre-transfer callback; at 4 MHz a full frame is ~2 ms. */
    esp_err_t err = spi_device_polling_transmit(s_spi, &t);
    if (err != ESP_OK) ESP_LOGE(TAG, "spi: %s", esp_err_to_name(err));
}

static void cmd(uint8_t c)              { spi_write(false, &c, 1); }
static void cmd2(uint8_t c, uint8_t a)  { uint8_t b[2] = { c, a }; spi_write(false, b, 2); }

esp_err_t oled_init(const ssd1309_config_t *cfg)
{
    s_cfg = *cfg;
    if (s_cfg.clock_hz <= 0) s_cfg.clock_hz = 4 * 1000 * 1000;

    gpio_config_t io = {
        .pin_bit_mask = (1ULL << s_cfg.pin_dc) | (1ULL << s_cfg.pin_rst),
        .mode         = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io);

    spi_bus_config_t bus = {
        .mosi_io_num     = s_cfg.pin_mosi,
        .miso_io_num     = -1,
        .sclk_io_num     = s_cfg.pin_sclk,
        .quadwp_io_num   = -1,
        .quadhd_io_num   = -1,
        .max_transfer_sz = sizeof(s_fb),
    };
    esp_err_t err = spi_bus_initialize(s_cfg.spi_host, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "spi_bus_initialize: %s", esp_err_to_name(err));
        return err;
    }

    spi_device_interface_config_t dev = {
        .clock_speed_hz = s_cfg.clock_hz,
        .mode           = 0,                 /* CPOL=0 CPHA=0, per U8g2 */
        .spics_io_num   = s_cfg.pin_cs,
        .queue_size     = 2,
    };
    err = spi_bus_add_device(s_cfg.spi_host, &dev, &s_spi);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "spi_bus_add_device: %s", esp_err_to_name(err));
        return err;
    }

    /* Reset pulse. U8g2 uses 100 ms each side for this part; "far east OLEDs
     * need much longer setup time" is the upstream comment. */
    gpio_set_level(s_cfg.pin_rst, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(s_cfg.pin_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    /* u8x8_d_ssd1309_128x64_noname_init_seq, in order */
    cmd(0xAE);          /* display off */
    cmd2(0xD5, 0xA0);   /* clock divide ratio / oscillator frequency */
    cmd(0x40);          /* display start line 0 */
    cmd2(0x20, 0x02);   /* page addressing mode */
    cmd(0xA1);          /* segment remap */
    cmd(0xC8);          /* COM scan direction: reverse */
    cmd2(0xDA, 0x12);   /* COM pins hardware config */
    cmd2(0x81, 0x6F);   /* contrast */
    cmd2(0xD9, 0xD3);   /* pre-charge period */
    cmd2(0xDB, 0x20);   /* VCOMH deselect level */
    cmd(0x2E);          /* deactivate scroll */
    cmd(0xA4);          /* output follows RAM */
    cmd(0xA6);          /* normal (non-inverted) */

    oled_clear();
    oled_show();        /* blank RAM before switching on, no power-up garbage */
    cmd(0xAF);          /* display on */

    ESP_LOGI(TAG, "up: sclk=%d mosi=%d dc=%d rst=%d cs=%d @ %d Hz, x_offset=%d",
             s_cfg.pin_sclk, s_cfg.pin_mosi, s_cfg.pin_dc, s_cfg.pin_rst,
             s_cfg.pin_cs, s_cfg.clock_hz, s_cfg.x_offset);
    return ESP_OK;
}

void oled_show(void)
{
    const int col = s_cfg.x_offset;
    for (int page = 0; page < SSD1309_HEIGHT / 8; page++) {
        uint8_t set_addr[3] = {
            (uint8_t)(0xB0 | page),
            (uint8_t)(0x10 | (col >> 4)),
            (uint8_t)(0x00 | (col & 0x0F)),
        };
        spi_write(false, set_addr, sizeof(set_addr));
        spi_write(true, &s_fb[page * SSD1309_WIDTH], SSD1309_WIDTH);
    }
}

void oled_power(bool on)             { cmd(on ? 0xAF : 0xAE); }
void oled_contrast(uint8_t level)    { cmd2(0x81, level); }
void oled_invert(bool inverted)      { cmd(inverted ? 0xA7 : 0xA6); }

void oled_flip(bool flipped)
{
    /* u8x8_d_ssd1309_128x64_flip1_seq / flip0_seq. The NONAME2 module keeps
     * the same 2-column offset in both orientations. */
    if (flipped) { cmd(0xA0); cmd(0xC0); }
    else         { cmd(0xA1); cmd(0xC8); }
}

/* ------------------------------------------------------------------ */
/* Framebuffer                                                         */
/* ------------------------------------------------------------------ */

void oled_clear(void)                 { memset(s_fb, 0, sizeof(s_fb)); }
void oled_set_color(int color)        { s_color = color; }
int  oled_get_color(void)             { return s_color; }
uint8_t *oled_framebuffer(void)       { return s_fb; }

static inline void plot(int x, int y, int color)
{
    if ((unsigned)x >= SSD1309_WIDTH || (unsigned)y >= SSD1309_HEIGHT) return;
    uint8_t *p = &s_fb[(y >> 3) * SSD1309_WIDTH + x];
    uint8_t  m = (uint8_t)(1u << (y & 7));
    switch (color) {
        case OLED_COLOR_CLEAR: *p &= (uint8_t)~m; break;
        case OLED_COLOR_XOR:   *p ^= m;           break;
        default:               *p |= m;           break;
    }
}

void oled_pixel(int x, int y) { plot(x, y, s_color); }

int oled_get_pixel(int x, int y)
{
    if ((unsigned)x >= SSD1309_WIDTH || (unsigned)y >= SSD1309_HEIGHT) return 0;
    return (s_fb[(y >> 3) * SSD1309_WIDTH + x] >> (y & 7)) & 1;
}

void oled_hline(int x, int y, int w) { for (int i = 0; i < w; i++) plot(x + i, y, s_color); }
void oled_vline(int x, int y, int h) { for (int i = 0; i < h; i++) plot(x, y + i, s_color); }

void oled_line(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    for (;;) {
        plot(x0, y0, s_color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void oled_frame(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) return;
    oled_hline(x, y, w);
    oled_hline(x, y + h - 1, w);
    oled_vline(x, y, h);
    oled_vline(x + w - 1, y, h);
}

void oled_box(int x, int y, int w, int h)
{
    for (int j = 0; j < h; j++) oled_hline(x, y + j, w);
}

/* Rounded corners follow U8g2's drawRFrame/drawRBox construction: a frame
 * with the corners replaced by circle quadrants of radius r. */
static void circle_quadrants(int cx, int cy, int r, int quads, bool fill)
{
    /* quads bitmask: 1=upper-right 2=upper-left 4=lower-left 8=lower-right */
    int x = 0, y = r, f = 1 - r, ddx = 1, ddy = -2 * r;
    while (x <= y) {
        if (fill) {
            if (quads & 1) { oled_vline(cx + x, cy - y, y + 1); oled_vline(cx + y, cy - x, x + 1); }
            if (quads & 2) { oled_vline(cx - x, cy - y, y + 1); oled_vline(cx - y, cy - x, x + 1); }
            if (quads & 4) { oled_vline(cx - x, cy, y + 1);     oled_vline(cx - y, cy, x + 1); }
            if (quads & 8) { oled_vline(cx + x, cy, y + 1);     oled_vline(cx + y, cy, x + 1); }
        } else {
            if (quads & 1) { plot(cx + x, cy - y, s_color); plot(cx + y, cy - x, s_color); }
            if (quads & 2) { plot(cx - x, cy - y, s_color); plot(cx - y, cy - x, s_color); }
            if (quads & 4) { plot(cx - x, cy + y, s_color); plot(cx - y, cy + x, s_color); }
            if (quads & 8) { plot(cx + x, cy + y, s_color); plot(cx + y, cy + x, s_color); }
        }
        if (f >= 0) { y--; ddy += 2; f += ddy; }
        x++; ddx += 2; f += ddx;
    }
}

void oled_circle(int cx, int cy, int r) { circle_quadrants(cx, cy, r, 0xF, false); }
void oled_disc(int cx, int cy, int r)   { circle_quadrants(cx, cy, r, 0xF, true); }

void oled_rframe(int x, int y, int w, int h, int r)
{
    if (w <= 0 || h <= 0) return;
    int xl = x + r, yu = y + r;
    int xr = x + w - r - 1, yl = y + h - r - 1;
    circle_quadrants(xl, yu, r, 2, false);
    circle_quadrants(xr, yu, r, 1, false);
    circle_quadrants(xl, yl, r, 4, false);
    circle_quadrants(xr, yl, r, 8, false);
    int ww = w - 2 * r - 2, hh = h - 2 * r - 2;
    if (ww > 0) { oled_hline(xl + 1, y, ww); oled_hline(xl + 1, y + h - 1, ww); }
    if (hh > 0) { oled_vline(x, yu + 1, hh); oled_vline(x + w - 1, yu + 1, hh); }
}

void oled_rbox(int x, int y, int w, int h, int r)
{
    if (w <= 0 || h <= 0) return;
    int xl = x + r, yu = y + r;
    int xr = x + w - r - 1, yl = y + h - r - 1;
    circle_quadrants(xl, yu, r, 2, true);
    circle_quadrants(xr, yu, r, 1, true);
    circle_quadrants(xl, yl, r, 4, true);
    circle_quadrants(xr, yl, r, 8, true);
    int ww = w - 2 * r - 2, hh = h - 2 * r - 2;
    if (ww > 0) { oled_box(xl + 1, y, ww, r + 1); oled_box(xl + 1, yl, ww, r + 1); }
    if (hh > 0) oled_box(x, yu + 1, w, hh);
}

/* Filled triangle by horizontal scanline, like U8g2's drawTriangle. */
static void swap_pt(int *ax, int *ay, int *bx, int *by)
{
    int t = *ax; *ax = *bx; *bx = t;
    t = *ay; *ay = *by; *by = t;
}

void oled_triangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
    if (y0 > y1) swap_pt(&x0, &y0, &x1, &y1);
    if (y1 > y2) swap_pt(&x1, &y1, &x2, &y2);
    if (y0 > y1) swap_pt(&x0, &y0, &x1, &y1);

    if (y0 == y2) {   /* degenerate: all on one line */
        int a = x0, b = x0;
        if (x1 < a) a = x1;
        if (x1 > b) b = x1;
        if (x2 < a) a = x2;
        if (x2 > b) b = x2;
        oled_hline(a, y0, b - a + 1);
        return;
    }
    for (int y = y0; y <= y2; y++) {
        /* long edge x0..x2 */
        int xa = x0 + (int)((long)(x2 - x0) * (y - y0) / (y2 - y0));
        int xb;
        if (y < y1 || y1 == y2) {
            if (y1 == y0) xb = x1;
            else xb = x0 + (int)((long)(x1 - x0) * (y - y0) / (y1 - y0));
        } else {
            if (y2 == y1) xb = x2;
            else xb = x1 + (int)((long)(x2 - x1) * (y - y1) / (y2 - y1));
        }
        if (xa > xb) { int t = xa; xa = xb; xb = t; }
        oled_hline(xa, y, xb - xa + 1);
    }
}

void oled_xbm(int x, int y, int w, int h, const uint8_t *bits, bool transparent)
{
    int stride = (w + 7) / 8;
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            int on = (bits[j * stride + (i >> 3)] >> (i & 7)) & 1;
            if (on) plot(x + i, y + j, s_color);
            else if (!transparent) plot(x + i, y + j, OLED_COLOR_CLEAR);
        }
    }
}

/* ------------------------------------------------------------------ */
/* Text                                                                */
/* ------------------------------------------------------------------ */

/* Classic 5x7 ASCII font, columns left to right, bit 0 = top row.
 * Covers 0x20..0x7E. Anything else draws as a box. */
static const uint8_t font5x7[95][5] = {
    {0x00,0x00,0x00,0x00,0x00}, /* space */
    {0x00,0x00,0x5F,0x00,0x00}, /* ! */
    {0x00,0x07,0x00,0x07,0x00}, /* " */
    {0x14,0x7F,0x14,0x7F,0x14}, /* # */
    {0x24,0x2A,0x7F,0x2A,0x12}, /* $ */
    {0x23,0x13,0x08,0x64,0x62}, /* % */
    {0x36,0x49,0x55,0x22,0x50}, /* & */
    {0x00,0x05,0x03,0x00,0x00}, /* ' */
    {0x00,0x1C,0x22,0x41,0x00}, /* ( */
    {0x00,0x41,0x22,0x1C,0x00}, /* ) */
    {0x14,0x08,0x3E,0x08,0x14}, /* * */
    {0x08,0x08,0x3E,0x08,0x08}, /* + */
    {0x00,0x50,0x30,0x00,0x00}, /* , */
    {0x08,0x08,0x08,0x08,0x08}, /* - */
    {0x00,0x60,0x60,0x00,0x00}, /* . */
    {0x20,0x10,0x08,0x04,0x02}, /* / */
    {0x3E,0x51,0x49,0x45,0x3E}, /* 0 */
    {0x00,0x42,0x7F,0x40,0x00}, /* 1 */
    {0x42,0x61,0x51,0x49,0x46}, /* 2 */
    {0x21,0x41,0x45,0x4B,0x31}, /* 3 */
    {0x18,0x14,0x12,0x7F,0x10}, /* 4 */
    {0x27,0x45,0x45,0x45,0x39}, /* 5 */
    {0x3C,0x4A,0x49,0x49,0x30}, /* 6 */
    {0x01,0x71,0x09,0x05,0x03}, /* 7 */
    {0x36,0x49,0x49,0x49,0x36}, /* 8 */
    {0x06,0x49,0x49,0x29,0x1E}, /* 9 */
    {0x00,0x36,0x36,0x00,0x00}, /* : */
    {0x00,0x56,0x36,0x00,0x00}, /* ; */
    {0x08,0x14,0x22,0x41,0x00}, /* < */
    {0x14,0x14,0x14,0x14,0x14}, /* = */
    {0x00,0x41,0x22,0x14,0x08}, /* > */
    {0x02,0x01,0x51,0x09,0x06}, /* ? */
    {0x32,0x49,0x79,0x41,0x3E}, /* @ */
    {0x7E,0x11,0x11,0x11,0x7E}, /* A */
    {0x7F,0x49,0x49,0x49,0x36}, /* B */
    {0x3E,0x41,0x41,0x41,0x22}, /* C */
    {0x7F,0x41,0x41,0x22,0x1C}, /* D */
    {0x7F,0x49,0x49,0x49,0x41}, /* E */
    {0x7F,0x09,0x09,0x01,0x01}, /* F */
    {0x3E,0x41,0x41,0x51,0x32}, /* G */
    {0x7F,0x08,0x08,0x08,0x7F}, /* H */
    {0x00,0x41,0x7F,0x41,0x00}, /* I */
    {0x20,0x40,0x41,0x3F,0x01}, /* J */
    {0x7F,0x08,0x14,0x22,0x41}, /* K */
    {0x7F,0x40,0x40,0x40,0x40}, /* L */
    {0x7F,0x02,0x04,0x02,0x7F}, /* M */
    {0x7F,0x04,0x08,0x10,0x7F}, /* N */
    {0x3E,0x41,0x41,0x41,0x3E}, /* O */
    {0x7F,0x09,0x09,0x09,0x06}, /* P */
    {0x3E,0x41,0x51,0x21,0x5E}, /* Q */
    {0x7F,0x09,0x19,0x29,0x46}, /* R */
    {0x46,0x49,0x49,0x49,0x31}, /* S */
    {0x01,0x01,0x7F,0x01,0x01}, /* T */
    {0x3F,0x40,0x40,0x40,0x3F}, /* U */
    {0x1F,0x20,0x40,0x20,0x1F}, /* V */
    {0x7F,0x20,0x18,0x20,0x7F}, /* W */
    {0x63,0x14,0x08,0x14,0x63}, /* X */
    {0x03,0x04,0x78,0x04,0x03}, /* Y */
    {0x61,0x51,0x49,0x45,0x43}, /* Z */
    {0x00,0x00,0x7F,0x41,0x41}, /* [ */
    {0x02,0x04,0x08,0x10,0x20}, /* \ */
    {0x41,0x41,0x7F,0x00,0x00}, /* ] */
    {0x04,0x02,0x01,0x02,0x04}, /* ^ */
    {0x40,0x40,0x40,0x40,0x40}, /* _ */
    {0x00,0x01,0x02,0x04,0x00}, /* ` */
    {0x20,0x54,0x54,0x54,0x78}, /* a */
    {0x7F,0x48,0x44,0x44,0x38}, /* b */
    {0x38,0x44,0x44,0x44,0x20}, /* c */
    {0x38,0x44,0x44,0x48,0x7F}, /* d */
    {0x38,0x54,0x54,0x54,0x18}, /* e */
    {0x08,0x7E,0x09,0x01,0x02}, /* f */
    {0x08,0x14,0x54,0x54,0x3C}, /* g */
    {0x7F,0x08,0x04,0x04,0x78}, /* h */
    {0x00,0x44,0x7D,0x40,0x00}, /* i */
    {0x20,0x40,0x44,0x3D,0x00}, /* j */
    {0x00,0x7F,0x10,0x28,0x44}, /* k */
    {0x00,0x41,0x7F,0x40,0x00}, /* l */
    {0x7C,0x04,0x18,0x04,0x78}, /* m */
    {0x7C,0x08,0x04,0x04,0x78}, /* n */
    {0x38,0x44,0x44,0x44,0x38}, /* o */
    {0x7C,0x14,0x14,0x14,0x08}, /* p */
    {0x08,0x14,0x14,0x18,0x7C}, /* q */
    {0x7C,0x08,0x04,0x04,0x08}, /* r */
    {0x48,0x54,0x54,0x54,0x20}, /* s */
    {0x04,0x3F,0x44,0x40,0x20}, /* t */
    {0x3C,0x40,0x40,0x20,0x7C}, /* u */
    {0x1C,0x20,0x40,0x20,0x1C}, /* v */
    {0x3C,0x40,0x30,0x40,0x3C}, /* w */
    {0x44,0x28,0x10,0x28,0x44}, /* x */
    {0x0C,0x50,0x50,0x50,0x3C}, /* y */
    {0x44,0x64,0x54,0x4C,0x44}, /* z */
    {0x00,0x08,0x36,0x41,0x00}, /* { */
    {0x00,0x00,0x7F,0x00,0x00}, /* | */
    {0x00,0x41,0x36,0x08,0x00}, /* } */
    {0x08,0x04,0x08,0x10,0x08}, /* ~ */
};

#define GLYPH_W 5
#define GLYPH_H 7
#define GLYPH_ADV 6

static int clamp_scale(int s) { return s < 1 ? 1 : s > 4 ? 4 : s; }
void oled_set_scale(int scale)         { s_scale_x = s_scale_y = clamp_scale(scale); }
void oled_set_scale_xy(int sx, int sy) { s_scale_x = clamp_scale(sx); s_scale_y = clamp_scale(sy); }
void oled_set_dir(int dir)     { s_dir = dir & 3; }

/* Map a glyph-local offset (gx right, gy down) into screen space for the
 * current direction, matching U8g2's setFontDirection rotation sense:
 * 0 = left-to-right, 1 = top-to-bottom, 2 = right-to-left, 3 = bottom-to-top. */
static inline void dir_map(int gx, int gy, int *dx, int *dy)
{
    switch (s_dir) {
        default: *dx =  gx; *dy =  gy; break;
        case 1:  *dx = -gy; *dy =  gx; break;
        case 2:  *dx = -gx; *dy = -gy; break;
        case 3:  *dx =  gy; *dy = -gx; break;
    }
}

int oled_glyph(int x, int y, char c)
{
    const uint8_t *g;
    uint8_t boxg[5] = {0x7F,0x41,0x41,0x41,0x7F};
    if (c >= 0x20 && c <= 0x7E) g = font5x7[(int)c - 0x20];
    else g = boxg;

    for (int col = 0; col < GLYPH_W; col++) {
        uint8_t bits = g[col];
        for (int row = 0; row < GLYPH_H; row++) {
            if (!((bits >> row) & 1)) continue;
            for (int sx = 0; sx < s_scale_x; sx++) {
                for (int sy = 0; sy < s_scale_y; sy++) {
                    int dx, dy;
                    dir_map(col * s_scale_x + sx, row * s_scale_y + sy, &dx, &dy);
                    plot(x + dx, y + dy, s_color);
                }
            }
        }
    }
    return GLYPH_ADV * s_scale_x;
}

int oled_str(int x, int y, const char *s, int len)
{
    int adv_total = 0;
    for (int i = 0; i < len; i++) {
        int adv = oled_glyph(x, y, s[i]);
        int dx, dy;
        dir_map(adv, 0, &dx, &dy);
        x += dx; y += dy;
        adv_total += adv;
    }
    return adv_total;
}

int oled_str_width(int len) { return len * GLYPH_ADV * s_scale_x; }
