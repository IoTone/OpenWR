/*
 * epd.c — UC8151-class e-paper driver (the raw sequence from the ESP_Skin
 * prototype, ported to ESP-IDF spi_master) plus a landscape canvas.
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

#include "pins_xiao_esp32c3.h"
#include "epd.h"

static const char *TAG = "epd";
static spi_device_handle_t s_spi;
static uint8_t s_canvas[EPD_CANVAS];
static bool s_up;

/* ---- bus ---------------------------------------------------------- */

static void xfer(bool data, const uint8_t *buf, size_t n)
{
    if (!s_spi || n == 0) return;
    gpio_set_level(SKIN_PIN_DC, data ? 1 : 0);
    spi_transaction_t t = { .length = n * 8, .tx_buffer = buf };
    spi_device_polling_transmit(s_spi, &t);
}
static void cmd(uint8_t c)            { xfer(false, &c, 1); }
static void dat(uint8_t d)            { xfer(true, &d, 1); }

/* BUSY is low while the controller works; wait for idle, bounded. */
static void wait_idle(int max_ms)
{
    for (int i = 0; i < max_ms / 10; i++) {
        if (gpio_get_level(SKIN_PIN_BUSY) == 1) return;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGW(TAG, "busy timeout after %d ms", max_ms);
}

static void panel_reset(void)
{
    for (int i = 0; i < 3; i++) {          /* three pulses, as the vendor code does */
        gpio_set_level(SKIN_PIN_RST, 0); vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(SKIN_PIN_RST, 1); vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/* The vendor init: power on, panel setting (LUT from OTP, B/W), 104x212,
 * VCOM and data interval 0x97. */
static void panel_init(void)
{
    panel_reset();
    cmd(0x04); wait_idle(3000);
    cmd(0x00); dat(0x1f);
    cmd(0x61); dat(0x68); dat(0x00); dat(0xd4);
    cmd(0x50); dat(0x97);
}

static void panel_sleep(void)
{
    cmd(0x02); wait_idle(3000);           /* power off */
    vTaskDelay(pdMS_TO_TICKS(1000));      /* the vendor code insists */
    cmd(0x07); dat(0xA5);                 /* deep sleep */
}

esp_err_t epd_init(void)
{
    gpio_config_t out = { .pin_bit_mask = (1ULL << SKIN_PIN_RST) | (1ULL << SKIN_PIN_DC), .mode = GPIO_MODE_OUTPUT };
    gpio_config(&out);
    gpio_config_t in = { .pin_bit_mask = 1ULL << SKIN_PIN_BUSY, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE };
    gpio_config(&in);
    gpio_set_level(SKIN_PIN_RST, 1);

    spi_bus_config_t bus = { .mosi_io_num = SKIN_PIN_MOSI, .miso_io_num = -1, .sclk_io_num = SKIN_PIN_SCK,
                             .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 4096 };
    esp_err_t err = spi_bus_initialize(SKIN_SPI_HOST, &bus, SPI_DMA_CH_AUTO);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) return err;
    spi_device_interface_config_t dev = { .clock_speed_hz = 4 * 1000 * 1000, .mode = 0, .spics_io_num = SKIN_PIN_CS, .queue_size = 2 };
    err = spi_bus_add_device(SKIN_SPI_HOST, &dev, &s_spi);
    if (err != ESP_OK) return err;
    s_up = true;
    epd_clear();
    ESP_LOGI(TAG, "up: 212x104, rst=%d cs=%d dc=%d busy=%d", SKIN_PIN_RST, SKIN_PIN_CS, SKIN_PIN_DC, SKIN_PIN_BUSY);
    return ESP_OK;
}

/* Rotate the landscape canvas (212x104, 1 = black) into the panel's native
 * frame (104 wide x 212 tall, 13 bytes per row, 1 = white). Canvas (x, y)
 * lands at native column (103 - y), native row x. */
static void to_native(uint8_t *out)
{
    memset(out, 0xFF, EPD_NATIVE);
    for (int y = 0; y < EPD_H; y++) {
        for (int x = 0; x < EPD_W; x++) {
            if (!(s_canvas[y * EPD_STRIDE + (x >> 3)] & (0x80 >> (x & 7)))) continue;
            int nc = 103 - y, nr = x;
            out[nr * 13 + (nc >> 3)] &= (uint8_t)~(0x80 >> (nc & 7));
        }
    }
}

void epd_show(void)
{
    if (!s_up) return;
    uint8_t *native = malloc(EPD_NATIVE);
    if (!native) return;
    to_native(native);
    panel_init();
    cmd(0x10);                                     /* "old" frame: all white */
    for (int i = 0; i < EPD_NATIVE; i += 256) {
        uint8_t ff[256]; memset(ff, 0xFF, sizeof ff);
        xfer(true, ff, (EPD_NATIVE - i) < 256 ? (size_t)(EPD_NATIVE - i) : 256);
    }
    cmd(0x13);                                     /* new frame */
    for (int i = 0; i < EPD_NATIVE; i += 256)
        xfer(true, native + i, (EPD_NATIVE - i) < 256 ? (size_t)(EPD_NATIVE - i) : 256);
    cmd(0x12); vTaskDelay(pdMS_TO_TICKS(10));      /* refresh */
    wait_idle(6000);
    panel_sleep();
    free(native);
}

/* ---- canvas ------------------------------------------------------- */

void epd_clear(void)          { memset(s_canvas, 0, sizeof s_canvas); }
uint8_t *epd_canvas(void)     { return s_canvas; }

void epd_pixel(int x, int y, int on)
{
    if ((unsigned)x >= EPD_W || (unsigned)y >= EPD_H) return;
    uint8_t *p = &s_canvas[y * EPD_STRIDE + (x >> 3)];
    uint8_t m = (uint8_t)(0x80 >> (x & 7));
    if (on) *p |= m; else *p &= (uint8_t)~m;
}

int epd_get(int x, int y)
{
    if ((unsigned)x >= EPD_W || (unsigned)y >= EPD_H) return 0;
    return (s_canvas[y * EPD_STRIDE + (x >> 3)] >> (7 - (x & 7))) & 1;
}

void epd_line(int x0, int y0, int x1, int y1)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1, err = dx + dy;
    for (;;) {
        epd_pixel(x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void epd_box(int x, int y, int w, int h, int on)
{
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) epd_pixel(x + i, y + j, on);
}

void epd_frame(int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) return;
    for (int i = 0; i < w; i++) { epd_pixel(x + i, y, 1); epd_pixel(x + i, y + h - 1, 1); }
    for (int j = 0; j < h; j++) { epd_pixel(x, y + j, 1); epd_pixel(x + w - 1, y + j, 1); }
}

/* Classic 5x7 ASCII font, columns left to right, bit 0 = top row. */
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

int epd_str(int x, int y, const char *s, int len, int scale)
{
    if (scale < 1) scale = 1;
    int adv = 0;
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        const uint8_t *g = (c >= 0x20 && c <= 0x7E) ? font5x7[c - 0x20] : font5x7[0];
        for (int col = 0; col < 5; col++)
            for (int row = 0; row < 7; row++)
                if ((g[col] >> row) & 1)
                    epd_box(x + adv + col * scale, y + row * scale, scale, scale, 1);
        adv += 6 * scale;
    }
    return adv;
}
