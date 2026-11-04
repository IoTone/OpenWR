/*
 * display.c — the status screen. See display.h.
 *
 * Text is the classic 5x7 font at 2x: 12x16 px cells, 20 columns, 8 rows
 * across 240x135. Each row is drawn into a 240x16 line buffer and sent in one
 * SPI transfer, and only rows whose text changed are redrawn.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c_master.h"

#include "forth_core.h"
#include "console.h"
#include "net_words.h"
#include "ble_console.h"
#include "sony_http.h"
#include "display.h"

static const char *TAG = "disp";

#define W 240
#define H 135
#define ROWS 8
#define COLS 20
#define PIN_MOSI 15
#define PIN_SCLK 13
#define PIN_CS   5

/* RGB565, sent big-endian */
#define C_BG     0x0000
#define C_TITLE  0xFD20   /* amber */
#define C_TEXT   0xDEFB
#define C_OK     0x07E8
#define C_WAIT   0xFFE0
#define C_BAD    0xF8A2
#define C_DIM    0x8410

static const uint8_t FONT[95][5] = {
    {0x00,0x00,0x00,0x00,0x00},{0x00,0x00,0x5F,0x00,0x00},{0x00,0x07,0x00,0x07,0x00},{0x14,0x7F,0x14,0x7F,0x14},
    {0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},{0x36,0x49,0x56,0x20,0x50},{0x00,0x08,0x07,0x03,0x00},
    {0x00,0x1C,0x22,0x41,0x00},{0x00,0x41,0x22,0x1C,0x00},{0x2A,0x1C,0x7F,0x1C,0x2A},{0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x80,0x70,0x30,0x00},{0x08,0x08,0x08,0x08,0x08},{0x00,0x00,0x60,0x60,0x00},{0x20,0x10,0x08,0x04,0x02},
    {0x3E,0x51,0x49,0x45,0x3E},{0x00,0x42,0x7F,0x40,0x00},{0x72,0x49,0x49,0x49,0x46},{0x21,0x41,0x49,0x4D,0x33},
    {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x31},{0x41,0x21,0x11,0x09,0x07},
    {0x36,0x49,0x49,0x49,0x36},{0x46,0x49,0x49,0x29,0x1E},{0x00,0x00,0x14,0x00,0x00},{0x00,0x40,0x34,0x00,0x00},
    {0x00,0x08,0x14,0x22,0x41},{0x14,0x14,0x14,0x14,0x14},{0x00,0x41,0x22,0x14,0x08},{0x02,0x01,0x59,0x09,0x06},
    {0x3E,0x41,0x5D,0x59,0x4E},{0x7C,0x12,0x11,0x12,0x7C},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x41,0x3E},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},{0x3E,0x41,0x41,0x51,0x73},
    {0x7F,0x08,0x08,0x08,0x7F},{0x00,0x41,0x7F,0x41,0x00},{0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},
    {0x7F,0x40,0x40,0x40,0x40},{0x7F,0x02,0x1C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},{0x26,0x49,0x49,0x49,0x32},
    {0x03,0x01,0x7F,0x01,0x03},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x3F,0x40,0x38,0x40,0x3F},
    {0x63,0x14,0x08,0x14,0x63},{0x03,0x04,0x78,0x04,0x03},{0x61,0x59,0x49,0x4D,0x43},{0x00,0x7F,0x41,0x41,0x41},
    {0x02,0x04,0x08,0x10,0x20},{0x00,0x41,0x41,0x41,0x7F},{0x04,0x02,0x01,0x02,0x04},{0x40,0x40,0x40,0x40,0x40},
    {0x00,0x03,0x07,0x08,0x00},{0x20,0x54,0x54,0x78,0x40},{0x7F,0x28,0x44,0x44,0x38},{0x38,0x44,0x44,0x44,0x28},
    {0x38,0x44,0x44,0x28,0x7F},{0x38,0x54,0x54,0x54,0x18},{0x00,0x08,0x7E,0x09,0x02},{0x18,0xA4,0xA4,0x9C,0x78},
    {0x7F,0x08,0x04,0x04,0x78},{0x00,0x44,0x7D,0x40,0x00},{0x20,0x40,0x40,0x3D,0x00},{0x7F,0x10,0x28,0x44,0x00},
    {0x00,0x41,0x7F,0x40,0x00},{0x7C,0x04,0x78,0x04,0x78},{0x7C,0x08,0x04,0x04,0x78},{0x38,0x44,0x44,0x44,0x38},
    {0xFC,0x18,0x24,0x24,0x18},{0x18,0x24,0x24,0x18,0xFC},{0x7C,0x08,0x04,0x04,0x08},{0x48,0x54,0x54,0x54,0x24},
    {0x04,0x04,0x3F,0x44,0x24},{0x3C,0x40,0x40,0x20,0x7C},{0x1C,0x20,0x40,0x20,0x1C},{0x3C,0x40,0x30,0x40,0x3C},
    {0x44,0x28,0x10,0x28,0x44},{0x4C,0x90,0x90,0x90,0x7C},{0x44,0x64,0x54,0x4C,0x44},{0x00,0x08,0x36,0x41,0x00},
    {0x00,0x00,0x77,0x00,0x00},{0x00,0x41,0x36,0x08,0x00},{0x02,0x01,0x02,0x04,0x02},
};

typedef enum { BL_OFF, BL_AXP, BL_GPIO } bl_t;

static spi_device_handle_t s_spi;
static int       s_dc = 23;
static bl_t      s_bl = BL_OFF;
static uint16_t *s_line;                  /* W x 16, DMA-capable */
static char      s_row[ROWS][COLS + 1];
static uint16_t  s_col[ROWS];
static char      s_msg[COLS + 1] = "hold A: find camera";

/* The alpha: right of rows 1-4, which narrow to 15 columns to make room */
#define LOGO_X 184
#define LOGO_Y 20
#define LOGO_W 56
#define LOGO_H 64
#define NARROW 15
static uint8_t   s_mask[LOGO_H][LOGO_W];  /* coverage 0..255, antialiased */
static int       s_logo = -1;             /* last drawn level; -1 = never */

/* ---- power: AXP192 (StickC Plus) or a GPIO backlight ------------------ */

static bool axp_init(void)
{
    i2c_master_bus_config_t bc = {
        .i2c_port = I2C_NUM_0, .sda_io_num = 21, .scl_io_num = 22,
        .clk_source = I2C_CLK_SRC_DEFAULT, .glitch_ignore_cnt = 7, .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    if (i2c_new_master_bus(&bc, &bus) != ESP_OK) return false;
    if (i2c_master_probe(bus, 0x34, 50) != ESP_OK) { i2c_del_master_bus(bus); return false; }
    i2c_device_config_t dc = { .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = 0x34, .scl_speed_hz = 400000 };
    i2c_master_dev_handle_t axp;
    if (i2c_master_bus_add_device(bus, &dc, &axp) != ESP_OK) return false;
    uint8_t reg = 0x12, v = 0;
    i2c_master_transmit_receive(axp, &reg, 1, &v, 1, 50);
    /* as M5's AXP192::begin: LDO2 (backlight) and LDO3 (panel) at 3.0 V,
     * then DCDC1, LDO2, LDO3 and EXTEN on */
    const uint8_t w1[] = { 0x28, 0xCC };
    const uint8_t w2[] = { 0x12, (uint8_t)(v | 0x4D) };
    i2c_master_transmit(axp, w1, 2, 50);
    i2c_master_transmit(axp, w2, 2, 50);
    ESP_LOGI(TAG, "AXP192: power control was 0x%02x, now 0x%02x", v, w2[1]);
    return true;
}

/* ---- ST7789 ------------------------------------------------------------ */

static void lcd_send(bool data, const void *buf, size_t len)
{
    if (!len) return;
    gpio_set_level(s_dc, data);
    spi_transaction_t t = { .length = len * 8, .tx_buffer = buf };
    spi_device_polling_transmit(s_spi, &t);
}

static void lcd_cmd(uint8_t c, const uint8_t *d, size_t n)
{
    lcd_send(false, &c, 1);
    if (n) lcd_send(true, d, n);
}

/* landscape: 240 wide, panel offsets 40 / 53 */
static void lcd_window(int x0, int y0, int x1, int y1)
{
    x0 += 40; x1 += 40; y0 += 53; y1 += 53;
    uint8_t ca[] = { x0 >> 8, x0 & 255, x1 >> 8, x1 & 255 };
    uint8_t ra[] = { y0 >> 8, y0 & 255, y1 >> 8, y1 & 255 };
    lcd_cmd(0x2A, ca, 4);
    lcd_cmd(0x2B, ra, 4);
    lcd_cmd(0x2C, NULL, 0);
}

static void lcd_fill_rows(int y, int h, uint16_t c)
{
    uint16_t be = (uint16_t)(c >> 8 | c << 8);
    for (int i = 0; i < W * 16; i++) s_line[i] = be;
    for (int yy = y; yy < y + h; yy += 16) {
        int n = y + h - yy < 16 ? y + h - yy : 16;
        lcd_window(0, yy, W - 1, yy + n - 1);
        lcd_send(true, s_line, W * n * 2);
    }
}

static int row_cols(int r) { return r >= 1 && r <= 4 ? NARROW : COLS; }

static void draw_row(int r, const char *s, uint16_t fg, uint16_t bg)
{
    uint16_t f = (uint16_t)(fg >> 8 | fg << 8), b = (uint16_t)(bg >> 8 | bg << 8);
    int cols = row_cols(r), pw = cols == COLS ? W : cols * 12;    /* narrow rows stop short of the logo */
    for (int i = 0; i < W * 16; i++) s_line[i] = b;
    for (int c = 0; c < cols && s[c]; c++) {
        int ch = (unsigned char)s[c];
        if (ch < 32 || ch > 126) ch = '?';
        const uint8_t *g = FONT[ch - 32];
        for (int col = 0; col < 5; col++)
            for (int bit = 0; bit < 8; bit++) {
                if (!(g[col] >> bit & 1)) continue;
                int x = c * 12 + 1 + col * 2, y = bit * 2;
                s_line[y * pw + x] = f; s_line[y * pw + x + 1] = f;
                s_line[(y + 1) * pw + x] = f; s_line[(y + 1) * pw + x + 1] = f;
            }
    }
    lcd_window(0, r * 16 + 4, pw - 1, r * 16 + 4 + 15);   /* 4 px top margin: 8 rows of 16 in 135 */
    lcd_send(true, s_line, pw * 16 * 2);
}

/* ---- the alpha logo ------------------------------------------------------
 * A round bowl open on the right, whose two ends cross into tails: the
 * fish-shaped lowercase alpha. Built once as a coverage mask from distances
 * to the arc and the two strokes. */

static float seg_dist(float px, float py, float ax, float ay, float bx, float by)
{
    float dx = bx - ax, dy = by - ay;
    float t = ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy);
    t = t < 0 ? 0 : t > 1 ? 1 : t;
    float ex = ax + t * dx - px, ey = ay + t * dy - py;
    return sqrtf(ex * ex + ey * ey);
}

static void logo_build(void)
{
    const float cx = 22, cy = 32, r = 16, half = 3.2f, open = 50.0f * (float)M_PI / 180.0f;
    const float jx = cx + r * cosf(open), jy1 = cy - r * sinf(open), jy2 = cy + r * sinf(open);
    for (int y = 0; y < LOGO_H; y++)
        for (int x = 0; x < LOGO_W; x++) {
            float px = x + 0.5f, py = y + 0.5f, dx = px - cx, dy = py - cy;
            float a = atan2f(-dy, dx), d;
            if (fabsf(a) > open) d = fabsf(sqrtf(dx * dx + dy * dy) - r);     /* on the bowl's side */
            else {                                                             /* nearest arc end */
                float d1 = hypotf(px - jx, py - jy1), d2 = hypotf(px - jx, py - jy2);
                d = d1 < d2 ? d1 : d2;
            }
            float s1 = seg_dist(px, py, jx, jy1, 52, 56), s2 = seg_dist(px, py, jx, jy2, 52, 8);
            if (s1 < d) d = s1;
            if (s2 < d) d = s2;
            float cov = half + 0.5f - d;
            s_mask[y][x] = cov <= 0 ? 0 : cov >= 1 ? 255 : (uint8_t)(cov * 255);
        }
}

/* level 0..255 of the colour (r, g, b 0..255) over black */
static void logo_draw(int R, int G, int B, int level)
{
    for (int y = 0; y < LOGO_H; y++)
        for (int x = 0; x < LOGO_W; x++) {
            int k = s_mask[y][x] * level / 255;
            uint16_t c = (uint16_t)(((R * k / 255) >> 3) << 11 | ((G * k / 255) >> 2) << 5 | ((B * k / 255) >> 3));
            s_line[y * LOGO_W + x] = (uint16_t)(c >> 8 | c << 8);
        }
    lcd_window(LOGO_X, LOGO_Y, LOGO_X + LOGO_W - 1, LOGO_Y + LOGO_H - 1);
    lcd_send(true, s_line, LOGO_W * LOGO_H * 2);
}

/* red while the camera is not linked; orange, breathing every 2 s, when it is */
void display_pulse(uint32_t ms)
{
    if (s_bl == BL_OFF || !s_line) return;
    if (shc_state() != SHC_READY) {
        if (s_logo != 1000) { logo_draw(255, 24, 16, 255); s_logo = 1000; }
        return;
    }
    float ph = (float)(ms % 2000) / 2000.0f;
    int level = 90 + (int)(165.0f * (0.5f - 0.5f * cosf(2.0f * (float)M_PI * ph)));
    if (abs(level - s_logo) < 6) return;
    logo_draw(255, 130, 0, level);
    s_logo = level;
}

bool display_init(void)
{
    if (axp_init()) {
        s_bl = BL_AXP;
        s_dc = 23;                                   /* StickC Plus: DC 23, RST 18 */
        gpio_set_direction(18, GPIO_MODE_OUTPUT);
        gpio_set_level(18, 0); vTaskDelay(pdMS_TO_TICKS(10)); gpio_set_level(18, 1);
    } else {
        s_bl = BL_GPIO;
        s_dc = 14;                                   /* Plus2 layout: DC 14, RST 12, backlight 27 */
        gpio_set_direction(12, GPIO_MODE_OUTPUT);
        gpio_set_level(12, 0); vTaskDelay(pdMS_TO_TICKS(10)); gpio_set_level(12, 1);
    }
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_direction(s_dc, GPIO_MODE_OUTPUT);

    spi_bus_config_t bus = { .mosi_io_num = PIN_MOSI, .miso_io_num = -1, .sclk_io_num = PIN_SCLK,
                             .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = W * 16 * 2 + 16 };
    if (spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO) != ESP_OK) { s_bl = BL_OFF; return false; }
    spi_device_interface_config_t dev = { .clock_speed_hz = 26 * 1000 * 1000, .mode = 0, .spics_io_num = PIN_CS, .queue_size = 1 };
    if (spi_bus_add_device(SPI2_HOST, &dev, &s_spi) != ESP_OK) { s_bl = BL_OFF; return false; }
    s_line = heap_caps_malloc(W * 16 * 2, MALLOC_CAP_DMA);
    if (!s_line) { s_bl = BL_OFF; return false; }

    lcd_cmd(0x01, NULL, 0); vTaskDelay(pdMS_TO_TICKS(150));      /* software reset */
    lcd_cmd(0x11, NULL, 0); vTaskDelay(pdMS_TO_TICKS(120));      /* sleep out */
    const uint8_t colmod = 0x55, madctl = 0x60;                  /* RGB565; landscape (MX | MV) */
    lcd_cmd(0x3A, &colmod, 1);
    lcd_cmd(0x36, &madctl, 1);
    lcd_cmd(0x21, NULL, 0);                                      /* the panel wants inversion on */
    lcd_cmd(0x13, NULL, 0);
    lcd_fill_rows(0, H, C_BG);
    lcd_cmd(0x29, NULL, 0);                                      /* display on */

    if (s_bl == BL_GPIO) { gpio_set_direction(27, GPIO_MODE_OUTPUT); gpio_set_level(27, 1); }
    memset(s_row, 0xFF, sizeof s_row);                           /* everything differs: full first draw */
    logo_build();
    ESP_LOGI(TAG, "screen up (%s)", s_bl == BL_AXP ? "AXP192" : "GPIO27 backlight");
    display_tick();
    display_pulse(0);
    return true;
}

static void put(int r, uint16_t col, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
static void put(int r, uint16_t col, const char *fmt, ...)
{
    char s[COLS + 8];
    va_list a; va_start(a, fmt); vsnprintf(s, sizeof s, fmt, a); va_end(a);
    int cols = row_cols(r);
    s[cols] = 0;
    for (int i = (int)strlen(s); i < cols; i++) s[i] = ' ';     /* pad: overwrite the old text */
    s[cols] = 0;
    if (s_col[r] == col && strcmp(s_row[r], s) == 0) return;
    strcpy(s_row[r], s);
    s_col[r] = col;
    draw_row(r, s, col, r == 0 ? 0x2104 : C_BG);
}

void display_tick(void)
{
    if (s_bl == BL_OFF || !s_line) return;
    put(0, C_TITLE, "AlphaFriend");

    char ssid[33] = "";
    kv_get_str("ssid", ssid, sizeof ssid);
    if (net_ok()) {
        uint32_t a = net_ip4();
        put(1, C_OK, "wifi %d dBm", net_rssi());
        put(2, C_DIM, "%u.%u.%u.%u", (unsigned)(a & 255), (unsigned)(a >> 8 & 255), (unsigned)(a >> 16 & 255), (unsigned)(a >> 24));
    } else {
        put(1, ssid[0] ? C_WAIT : C_BAD, ssid[0] ? "wifi looking" : "wifi not set");
        put(2, C_DIM, "%s", strncmp(ssid, "DIRECT-", 7) == 0 ? ssid + 7 : ssid);
    }

    shc_state_t st = shc_state();
    put(3, st == SHC_READY ? C_OK : st == SHC_ERR ? C_BAD : st == SHC_OFF ? C_DIM : C_WAIT, "cam %s", shc_state_name());
    put(4, C_TEXT, "%s", strcmp(shc_model(), "-") ? shc_model() : "");

    int b = ble_console_state();
    put(5, b == 2 ? C_OK : b == 1 ? C_WAIT : C_DIM, "%s", b == 2 ? "glasses linked" : b == 1 ? "BLE waiting" : "BLE off");
    put(6, C_DIM, " ");
    put(7, C_TEXT, "%s", s_msg);
}

void display_msg(const char *s)
{
    strncpy(s_msg, s, COLS);
    s_msg[COLS] = 0;
}

static void w_msg(void)
{
    int n = (int)forth_pop(); const char *p = (const char *)forth_pop();
    char s[COLS + 1] = "";
    if (p && n > 0) { if (n > COLS) n = COLS; memcpy(s, p, n); }
    display_msg(s);
}

static void w_disp_q(void)
{
    con_printf("disp %s\n", s_bl == BL_AXP ? "axp192" : s_bl == BL_GPIO ? "gpio27" : "off");
}

void display_register_words(void)
{
    forth_register_word("msg!", w_msg);
    forth_register_word("disp?", w_disp_q);
}
