/*
 * epd.c: the CrowPanel 4.2in 400x300 glass on ESP-IDF spi_master, behind a
 * portrait canvas. Elecrow has shipped it with two controllers:
 *
 *   SSD1683 (Solomon), early boards: BUSY high while working.
 *   UC81xx (UltraChip), boards with a green round sticker on the back:
 *     BUSY low while working, LUTs loaded from registers, 0x10 old / 0x13 new
 *     image, 0x17 0xA5 runs power-on, refresh, power-off in one go. Init,
 *     the GC (full) and DU (fast) LUTs are Elecrow's, from the green-sticker
 *     examples.
 *
 * epd_init() tells them apart by the BUSY level after a reset: an idle
 * UC81xx holds it high, an idle SSD1683 low. epd-ctl! can force one.
 *
 * SSD1683 notes:
 * Init and the full/partial update codes follow Elecrow's own example for
 * this board (EPD.cpp: 0x22 0xF7 full, 0xFF partial). Partial refreshes
 * need the controller's "previous" RAM (0x26) to hold what is on the glass,
 * so every partial writes the old picture to 0x26 and the new one to 0x24.
 * That costs one extra 15 KB transfer (~15 ms) and survives resets and
 * controller sleep, which matters because the pass deep-sleeps between pages.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs.h"

#include "pins.h"
#include "epd.h"

static const char *TAG = "epd";
extern const uint8_t font5x7[95][5];

static spi_device_handle_t s_spi;
static uint8_t s_canvas[EPD_CLEN];
static uint8_t s_native[EPD_NSTRIDE * EPD_NH];   /* 1 = white, the controller's format */
static uint8_t s_prev[EPD_NSTRIDE * EPD_NH];
static bool s_up, s_asleep, s_have_prev;
static int s_rot = 1;
static int s_ctl;          /* CTL_SSD or CTL_UC, decided at init */
static int s_ctl_force;    /* 0 auto, else CTL_* (NVS "ctl") */
enum { CTL_SSD = 1, CTL_UC = 2 };

/* ---------------------------------------------------------------- bus */

static void xfer(bool data, const uint8_t *buf, size_t n)
{
    gpio_set_level(PIN_EPD_DC, data);
    while (n) {
        size_t k = n > 4000 ? 4000 : n;
        spi_transaction_t t = { .length = k * 8, .tx_buffer = buf };
        spi_device_polling_transmit(s_spi, &t);
        buf += k; n -= k;
    }
}
static void cmd(uint8_t c) { xfer(false, &c, 1); }
static void dat(uint8_t d) { xfer(true, &d, 1); }

static void wait_busy(int max_ms)
{
    int busy = s_ctl == CTL_UC ? 0 : 1;
    for (int i = 0; i < max_ms / 5; i++) {
        if (gpio_get_level(PIN_EPD_BUSY) != busy) return;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    ESP_LOGW(TAG, "busy timeout after %d ms", max_ms);
}

static void window_full(void)
{
    cmd(0x44); dat(0x00); dat(EPD_NSTRIDE - 1);                       /* RAM x: bytes 0..49 */
    cmd(0x45); dat(0x00); dat(0x00); dat((EPD_NH - 1) & 0xFF); dat((EPD_NH - 1) >> 8);
    cmd(0x4E); dat(0x00);
    cmd(0x4F); dat(0x00); dat(0x00);
}

/* ---------------------------------------------------------------- UC81xx */

static const uint8_t lut_gc[5][7] = {          /* full refresh, R20..R24 (42 bytes each, rest 0) */
    { 0x01, 0x14, 0x0A, 0x14, 0x00, 0x01, 0x01 },
    { 0x01, 0x54, 0x0A, 0x94, 0x00, 0x01, 0x01 },
    { 0x01, 0x54, 0x0A, 0x94, 0x00, 0x01, 0x01 },
    { 0x01, 0x94, 0x0A, 0x54, 0x00, 0x01, 0x01 },
    { 0x01, 0x94, 0x0A, 0x54, 0x00, 0x01, 0x01 },
};
static const uint8_t lut_du[5][7] = {          /* fast black/white update, no flashing */
    { 0x01, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00 },
    { 0x01, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00 },
    { 0x01, 0x94, 0x00, 0x00, 0x00, 0x01, 0x00 },
    { 0x01, 0x54, 0x00, 0x00, 0x00, 0x01, 0x00 },
    { 0x01, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00 },
};

static void uc_lut(const uint8_t lut[5][7])
{
    uint8_t row[42];
    for (int r = 0; r < 5; r++) {
        memset(row, 0, sizeof row);
        memcpy(row, lut[r], 7);
        cmd(0x20 + r);
        xfer(true, row, sizeof row);
    }
}

static void uc_init(void)
{
    cmd(0x00); dat(0x3F); dat(0x4D);                               /* panel setting: LUT from registers */
    cmd(0x01); dat(0x03); dat(0x10); dat(0x3F); dat(0x3F); dat(0x03); /* power */
    cmd(0x06); dat(0x96); dat(0x96); dat(0x29);                    /* booster soft start */
    cmd(0x30); dat(0x09);                                          /* PLL */
    cmd(0x61); dat(0x01); dat(0x90); dat(0x01); dat(0x2C);         /* 400 x 300 */
    cmd(0x82); dat(0x05);                                          /* VCOM */
    cmd(0x50); dat(0x97);                                          /* border, data polarity */
    cmd(0x60); dat(0x22);                                          /* TCON */
    cmd(0xE3); dat(0x88);                                          /* power saving */
}

static void uc_show(bool full)
{
    cmd(0x50); dat(0xD7);
    cmd(0x10); xfer(true, s_prev, sizeof s_prev);                  /* what is on the glass */
    cmd(0x13); xfer(true, s_native, sizeof s_native);              /* what it should become */
    uc_lut(full ? lut_gc : lut_du);
    cmd(0x17); dat(0xA5);                                          /* power on, refresh, power off */
    vTaskDelay(pdMS_TO_TICKS(20));
    wait_busy(full ? 10000 : 5000);
}

/* ---------------------------------------------------------------- init */

static void controller_init(void)
{
    gpio_set_level(PIN_EPD_RST, 1); vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_EPD_RST, 0); vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_EPD_RST, 1); vTaskDelay(pdMS_TO_TICKS(100));
    if (!s_ctl) {
        s_ctl = s_ctl_force ? s_ctl_force : (gpio_get_level(PIN_EPD_BUSY) ? CTL_UC : CTL_SSD);
        ESP_LOGI(TAG, "controller %s (%s)", s_ctl == CTL_UC ? "UC81xx" : "SSD1683", s_ctl_force ? "forced" : "BUSY level");
    }
    if (s_ctl == CTL_UC) {
        wait_busy(2000);
        uc_init();
        s_asleep = false;
        return;
    }
    wait_busy(2000);
    cmd(0x12);                                     /* software reset */
    wait_busy(2000);
    cmd(0x21); dat(0x40); dat(0x00);               /* display update control */
    cmd(0x3C); dat(0x05);                          /* border waveform */
    cmd(0x18); dat(0x80);                          /* internal temperature sensor */
    cmd(0x11); dat(0x03);                          /* data entry: x+, y+ */
    window_full();
    wait_busy(2000);
    s_asleep = false;
}

void epd_power(bool on)
{
    gpio_set_level(PIN_PWR_EN, on);                /* green-sticker boards: power enable (and the LED) */
    gpio_set_level(PIN_EPD_PWR, on);
    if (on) vTaskDelay(pdMS_TO_TICKS(10));
}

esp_err_t epd_init(void)
{
    if (!s_up) {
        gpio_config_t out = { .pin_bit_mask = (1ULL << PIN_EPD_DC) | (1ULL << PIN_EPD_RST) | (1ULL << PIN_EPD_PWR) | (1ULL << PIN_PWR_EN),
                              .mode = GPIO_MODE_OUTPUT };
        gpio_config(&out);
        gpio_config_t in = { .pin_bit_mask = 1ULL << PIN_EPD_BUSY, .mode = GPIO_MODE_INPUT };
        gpio_config(&in);
        spi_bus_config_t bus = { .sclk_io_num = PIN_EPD_SCK, .mosi_io_num = PIN_EPD_MOSI, .miso_io_num = -1,
                                 .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 4096 };
        esp_err_t e = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
        if (e != ESP_OK) return e;
        spi_device_interface_config_t dev = { .clock_speed_hz = 8000000, .mode = 0,
                                              .spics_io_num = PIN_EPD_CS, .queue_size = 1 };
        e = spi_bus_add_device(SPI2_HOST, &dev, &s_spi);
        if (e != ESP_OK) return e;
        nvs_handle_t h;
        if (nvs_open("epd", NVS_READONLY, &h) == ESP_OK) {
            int8_t r;
            if (nvs_get_i8(h, "rot", &r) == ESP_OK) s_rot = r & 3;
            if (nvs_get_i8(h, "ctl", &r) == ESP_OK) s_ctl_force = r;
            nvs_close(h);
        }
        s_up = true;
    }
    epd_power(true);
    controller_init();
    return ESP_OK;
}

/* ---------------------------------------------------------------- frames */

uint8_t *epd_canvas(void) { return s_canvas; }
const char *epd_ctl_name(void) { return s_ctl == CTL_UC ? "UC81xx" : s_ctl == CTL_SSD ? "SSD1683" : "?"; }

void epd_set_ctl(int c)
{
    s_ctl_force = (c == CTL_SSD || c == CTL_UC) ? c : 0;
    s_ctl = 0;
    s_have_prev = false;
    nvs_handle_t h;
    if (nvs_open("epd", NVS_READWRITE, &h) == ESP_OK) { nvs_set_i8(h, "ctl", s_ctl_force); nvs_commit(h); nvs_close(h); }
    controller_init();
}
void epd_clear(void) { memset(s_canvas, 0, sizeof s_canvas); }
int  epd_rot(void) { return s_rot; }

void epd_set_rot(int r)
{
    s_rot = r & 3;
    s_have_prev = false;                 /* the old picture is in the old orientation */
    nvs_handle_t h;
    if (nvs_open("epd", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_i8(h, "rot", (int8_t)s_rot);
        nvs_commit(h);
        nvs_close(h);
    }
}

/* Canvas (cx, cy), 300 x 400 portrait, onto the 400 x 300 glass:
 *   rot 0: nx = cy,        ny = cx
 *   rot 1: nx = cy,        ny = 299 - cx
 *   rot 2: nx = 399 - cy,  ny = 299 - cx
 *   rot 3: nx = 399 - cy,  ny = cx
 * One of 1 or 3 is upright with the dial at the bottom; 0 and 2 are their
 * mirror images, there in case the glass scans the other way. */
static void to_native(const uint8_t *canvas, uint8_t *out)
{
    memset(out, 0xFF, EPD_NSTRIDE * EPD_NH);
    for (int cy = 0; cy < EPD_CH; cy++) {
        const uint8_t *row = canvas + cy * EPD_CSTRIDE;
        int nx = (s_rot == 0 || s_rot == 1) ? cy : EPD_NW - 1 - cy;
        uint8_t mask = 0x80 >> (nx & 7);
        int nbyte = nx >> 3;
        for (int cx = 0; cx < EPD_CW; cx++) {
            if (!(row[cx >> 3] & (0x80 >> (cx & 7)))) continue;
            int ny = (s_rot == 0 || s_rot == 3) ? cx : EPD_NH - 1 - cx;
            out[ny * EPD_NSTRIDE + nbyte] &= ~mask;
        }
    }
}

void epd_set_previous(const uint8_t *canvas)
{
    to_native(canvas, s_prev);
    s_have_prev = true;
}

void epd_show(bool full)
{
    if (!s_up) return;
    if (s_asleep) controller_init();
    to_native(s_canvas, s_native);
    if (s_ctl == CTL_UC) {
        if (!s_have_prev) memset(s_prev, 0xFF, sizeof s_prev);
        uc_show(full || !s_have_prev);
    } else if (full || !s_have_prev) {
        window_full(); cmd(0x24); xfer(true, s_native, sizeof s_native);
        window_full(); cmd(0x26); xfer(true, s_native, sizeof s_native);
        cmd(0x21); dat(0x40); dat(0x00);
        cmd(0x3C); dat(0x05);
        cmd(0x22); dat(0xF7);
        cmd(0x20);
        vTaskDelay(pdMS_TO_TICKS(10));
        wait_busy(10000);
    } else {
        window_full(); cmd(0x26); xfer(true, s_prev, sizeof s_prev);
        window_full(); cmd(0x24); xfer(true, s_native, sizeof s_native);
        cmd(0x3C); dat(0x80);
        cmd(0x21); dat(0x00); dat(0x00);
        cmd(0x22); dat(0xFF);
        cmd(0x20);
        vTaskDelay(pdMS_TO_TICKS(5));
        wait_busy(5000);
    }
    memcpy(s_prev, s_native, sizeof s_prev);
    s_have_prev = true;
}

void epd_sleep(void)
{
    if (!s_up || s_asleep) return;
    if (s_ctl == CTL_UC) { cmd(0x07); dat(0xA5); }  /* deep sleep; a reset wakes it */
    else { cmd(0x10); dat(0x01); }                 /* deep sleep 1: the next init resets it */
    vTaskDelay(pdMS_TO_TICKS(20));
    s_asleep = true;
}

/* ---------------------------------------------------------------- drawing */

void epd_pixel(int x, int y, int on)
{
    if (x < 0 || y < 0 || x >= EPD_CW || y >= EPD_CH) return;
    uint8_t *b = &s_canvas[y * EPD_CSTRIDE + (x >> 3)];
    uint8_t m = 0x80 >> (x & 7);
    if (on) *b |= m; else *b &= ~m;
}

void epd_box(int x, int y, int w, int h, int on)
{
    for (int j = y; j < y + h; j++)
        for (int i = x; i < x + w; i++) epd_pixel(i, j, on);
}

void epd_frame(int x, int y, int w, int h)
{
    epd_box(x, y, w, 1, 1); epd_box(x, y + h - 1, w, 1, 1);
    epd_box(x, y, 1, h, 1); epd_box(x + w - 1, y, 1, h, 1);
}

int epd_str_w(const char *s, int scale) { return (int)strlen(s) * 6 * scale; }

int epd_str(int x, int y, const char *s, int scale)
{
    int x0 = x;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        const uint8_t *g = (c >= 0x20 && c <= 0x7E) ? font5x7[c - 0x20] : font5x7['?' - 0x20];
        for (int col = 0; col < 5; col++)
            for (int row = 0; row < 7; row++)
                if (g[col] & (1 << row)) epd_box(x + col * scale, y + row * scale, scale, scale, 1);
        x += 6 * scale;
    }
    return x - x0;
}
