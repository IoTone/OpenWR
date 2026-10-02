/*
 * epd.c — UC8151-class e-paper driver (the raw sequence from the ESP_Skin
 * prototype, ported to ESP-IDF spi_master) plus a landscape canvas.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "pins.h"
#include "epd.h"

static const char *TAG = "epd";

/* Bench knobs (epd-geom! epd-cdi! epd-spi! epd-old! epd-lut!) so a panel
 * can be characterised from the console without reflashing. */
static int s_geom = 0;        /* 0: native 104x212 (13 B/row); 1: native 122x250 (16 B/row) */
static int s_cdi  = 0x97;     /* VCOM and data interval byte */
static int s_old  = 0;        /* 0x10 "old" frame: 0 all white, 1 inverse of new, 2 not written */
static int s_lutm = 0;        /* 0 LUT from register (GxEPD2), 1 LUT from OTP (vendor, PSR 0x1f) */
static int s_hz   = 4000000;
static int s_csb  = 0;        /* 1: assert CS around every byte, as the vendor demo does */
static spi_device_handle_t s_spi;
static uint8_t s_canvas[EPD_CANVAS];
static bool s_up;

/* ---- bus ---------------------------------------------------------- */

static void xfer(bool data, const uint8_t *buf, size_t n)
{
    gpio_set_level(SKIN_PIN_DC, data ? 1 : 0);
    if (s_csb) {
        for (size_t i = 0; i < n; i++) {
            spi_transaction_t t = { .length = 8, .tx_buffer = buf + i };
            spi_device_polling_transmit(s_spi, &t);
        }
        return;
    }
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

/* Init as GxEPD2_213_flex does it (the combination the ESP_Skin prototype
 * proved on this glass): power setting, boost, panel setting 0xbf 0x0d
 * (LUT from register), PLL, 104x212, VCOM DC, CDI 0x97, then the five
 * full-refresh LUTs and power on. The vendor's short sequence (PSR 0x1f,
 * LUT from OTP) left the picture short of the glass with comb artifacts. */
static const uint8_t lut_20_vcomDC[] = {
    0x00, 0x08, 0x00, 0x00, 0x00, 0x02, 0x60, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t lut_21_ww[] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01, 0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t lut_22_bw[] = {
    0x40, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x40, 0x14, 0x00, 0x00, 0x00, 0x01, 0xA0, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t lut_23_wb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01, 0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const uint8_t lut_24_bb[] = {
    0x80, 0x08, 0x00, 0x00, 0x00, 0x02, 0x90, 0x28, 0x28, 0x00, 0x00, 0x01,
    0x80, 0x14, 0x00, 0x00, 0x00, 0x01, 0x50, 0x12, 0x12, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static void lut(uint8_t c, const uint8_t *t, size_t n) { cmd(c); xfer(true, t, n); }

static void panel_init(void)
{
    panel_reset();
    cmd(0x01); dat(0x03); dat(0x00); dat(0x2b); dat(0x2b); dat(0x03);   /* power setting */
    cmd(0x06); dat(0x17); dat(0x17); dat(0x17);                         /* boost soft start */
    int nw = s_geom ? 122 : EPD_H, nh = s_geom ? 250 : EPD_W;           /* native width (sources) x height (gates) */
    if (s_lutm == 0) { cmd(0x00); dat(0xbf); dat(0x0d); }               /* panel setting: LUT from register */
    else             { cmd(0x00); dat(0x1f); }                          /* vendor: LUT from OTP */
    cmd(0x30); dat(0x3a);                                               /* PLL 100 Hz */
    cmd(0x61); dat((uint8_t)nw); dat((uint8_t)(nh >> 8)); dat((uint8_t)(nh & 0xff));
    cmd(0x82); dat(0x08);                                               /* VCOM DC */
    cmd(0x50); dat((uint8_t)s_cdi);                                     /* VCOM and data interval */
    if (s_lutm == 0) {
        lut(0x20, lut_20_vcomDC, sizeof lut_20_vcomDC);
        lut(0x21, lut_21_ww, sizeof lut_21_ww);
        lut(0x22, lut_22_bw, sizeof lut_22_bw);
        lut(0x23, lut_23_wb, sizeof lut_23_wb);
        lut(0x24, lut_24_bb, sizeof lut_24_bb);
    }
    cmd(0x04); wait_idle(3000);                                         /* power on */
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
    spi_device_interface_config_t dev = { .clock_speed_hz = s_hz, .mode = 0, .spics_io_num = SKIN_PIN_CS, .queue_size = 2 };
    err = spi_bus_add_device(SKIN_SPI_HOST, &dev, &s_spi);
    if (err != ESP_OK) return err;
    s_up = true;
    epd_clear();
    ESP_LOGI(TAG, "up: 212x104, rst=%d cs=%d dc=%d busy=%d", SKIN_PIN_RST, SKIN_PIN_CS, SKIN_PIN_DC, SKIN_PIN_BUSY);
    return ESP_OK;
}

/* Rotate the landscape canvas (212x104, 1 = black) into the panel's native
 * frame (sources across, gates down, 1 = white). Canvas (x, y) lands at
 * native column (103 - y), native row x. In geometry 1 the native frame is
 * 122 x 250 (16 bytes per row) and the canvas sits at its origin. */
static int native_stride(void) { return s_geom ? 16 : 13; }
static int native_rows(void)   { return s_geom ? 250 : EPD_W; }
static int native_len(void)    { return native_stride() * native_rows(); }

static void to_native(uint8_t *out)
{
    int st = native_stride();
    memset(out, 0xFF, native_len());
    for (int y = 0; y < EPD_H; y++) {
        for (int x = 0; x < EPD_W; x++) {
            if (!(s_canvas[y * EPD_STRIDE + (x >> 3)] & (0x80 >> (x & 7)))) continue;
            int nc = 103 - y, nr = x;
            out[nr * st + (nc >> 3)] &= (uint8_t)~(0x80 >> (nc & 7));
        }
    }
}

static void send_frame(uint8_t c, const uint8_t *buf, int len, int invert)
{
    cmd(c);
    uint8_t tmp[256];
    for (int i = 0; i < len; i += 256) {
        int n = (len - i) < 256 ? (len - i) : 256;
        if (buf) { for (int k = 0; k < n; k++) tmp[k] = invert ? (uint8_t)~buf[i + k] : buf[i + k]; }
        else memset(tmp, 0xFF, n);
        xfer(true, tmp, n);
    }
}

void epd_show(void)
{
    if (!s_up) return;
    int len = native_len();
    uint8_t *native = malloc(len);
    if (!native) return;
    to_native(native);
    ESP_LOGI(TAG, "show: geom=%d (%dx%d) cdi=0x%02x old=%d lut=%s spi=%d csb=%d", s_geom, s_geom ? 122 : 104,
             native_rows(), s_cdi, s_old, s_lutm ? "otp" : "reg", s_hz, s_csb);
    panel_init();
    if (s_old == 0)      send_frame(0x10, NULL, len, 0);       /* old frame: all white */
    else if (s_old == 1) send_frame(0x10, native, len, 1);     /* old frame: inverse, every pixel transitions */
    send_frame(0x13, native, len, 0);                          /* new frame */
    cmd(0x12); vTaskDelay(pdMS_TO_TICKS(10));                  /* refresh */
    wait_idle(6000);
    panel_sleep();
    free(native);
}

void epd_knob(int which, int v)
{
    switch (which) {
    case 0: s_geom = v ? 1 : 0; break;
    case 1: s_cdi = v & 0xff; break;
    case 2: s_old = v; break;
    case 3: s_lutm = v ? 1 : 0; break;
    case 5: s_csb = v ? 1 : 0; break;
    case 4:
        if (v >= 100000 && v <= 20000000 && s_up) {
            s_hz = v;
            spi_bus_remove_device(s_spi);
            spi_device_interface_config_t dev = { .clock_speed_hz = s_hz, .mode = 0, .spics_io_num = SKIN_PIN_CS, .queue_size = 2 };
            spi_bus_add_device(SKIN_SPI_HOST, &dev, &s_spi);
        }
        break;
    }
}

/* ---- canvas ------------------------------------------------------- */

/* Calibration pattern: a 1 px border on the very edge, 10 px corner Ls,
 * ticks every 8 px (longer every 32), checkerboards at 1/2/4/8 px pitch,
 * a solid bar and a 50 % hatch, a centre crosshair and the size in text.
 * If the border is not on the glass edge, the resolution or rotation is
 * wrong; if the 1 px checker is grey mush, the LUT or data path is. */
/* Fill patterns for electrical checks: 0 white, 1 black, 2 alternate canvas
 * rows (= alternate source lines), 3 alternate canvas columns (= alternate
 * gate lines), 4 single-pixel checker, 5 left half black. */
void epd_fill(int mode)
{
    epd_clear();
    for (int y = 0; y < EPD_H; y++)
        for (int x = 0; x < EPD_W; x++) {
            int on = 0;
            switch (mode) {
            case 1: on = 1; break;
            case 2: on = y & 1; break;
            case 3: on = x & 1; break;
            case 4: on = (x + y) & 1; break;
            case 5: on = x < EPD_W / 2; break;
            }
            if (on) epd_pixel(x, y, 1);
        }
}

void epd_test_pattern_in(int X0, int W0);
void epd_test_pattern(void) { epd_test_pattern_in(0, EPD_W); }

/* the same pattern squeezed into columns X0 .. X0+W0 */
void epd_test_pattern_in(int X0, int W0)
{
    int X1 = X0 + W0 - 1;
    epd_clear();
    epd_frame(X0, 0, W0, EPD_H);
    const int cx[4] = { X0 + 2, X1 - 2, X0 + 2, X1 - 2 }, cy[4] = { 2, 2, EPD_H - 3, EPD_H - 3 };
    const int sx[4] = { 1, -1, 1, -1 },  sy[4] = { 1, 1, -1, -1 };
    for (int c = 0; c < 4; c++) {
        epd_line(cx[c], cy[c], cx[c] + 10 * sx[c], cy[c]);
        epd_line(cx[c], cy[c], cx[c], cy[c] + 10 * sy[c]);
    }
    for (int x = X0; x <= X1; x += 8) { int l = ((x - X0) % 32) ? 4 : 7; epd_line(x, 1, x, l); epd_line(x, EPD_H - 2, x, EPD_H - 1 - l); }
    for (int y = 0; y < EPD_H; y += 8) { int l = (y % 32) ? 4 : 7; epd_line(X0 + 1, y, X0 + l, y); epd_line(X1 - 1, y, X1 - l, y); }
    const int pitch[4] = { 1, 2, 4, 8 };
    int box = (W0 - 28) / 4 - 6; if (box > 40) box = 40; if (box < 12) box = 12;
    int step = (W0 - 28) / 4;
    for (int k = 0; k < 4; k++) {
        int x0 = X0 + 14 + k * step, y0 = 14;
        epd_frame(x0 - 1, y0 - 1, box + 2, box + 2);
        for (int y = 0; y < box; y++)
            for (int x = 0; x < box; x++)
                if (((x / pitch[k]) + (y / pitch[k])) % 2 == 0) epd_pixel(x0 + x, y0 + y, 1);
        char t[4] = { (char)('0' + pitch[k]), 'P', 'X', 0 };
        epd_str(x0, y0 + box + 3, t, 3, 1);
    }
    int by = 14 + box + 14;
    epd_box(X0 + 14, by, W0 / 2 - 20, 10, 1);
    for (int y = by; y < by + 10; y++) for (int x = X0 + W0 / 2 + 2; x < X1 - 12; x++) if (((x + y) & 1) == 0) epd_pixel(x, y, 1);
    int mx = X0 + W0 / 2;
    epd_line(mx - 10, 52, mx + 10, 52); epd_line(mx, 42, mx, 62);
    char lbl[32]; int n = snprintf(lbl, sizeof lbl, "TEST %dX%d @%d", W0, EPD_H, X0);
    epd_str(X0 + 12, EPD_H - 20, lbl, n, 1);
    epd_str(X0 + 12, EPD_H - 11, "X", 1, 1); epd_str(X1 - 16, EPD_H - 11, "X", 1, 1);
}

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
