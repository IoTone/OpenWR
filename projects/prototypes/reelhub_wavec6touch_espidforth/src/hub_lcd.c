/*
 * hub_lcd.c — see hub_lcd.h. Bring-up from MagNET_OTA_WaveC6LED's
 * panel_st7789.c (touch-board branch): JD9853 via Espressif's driver, RGB
 * order, inversion on, 34-column gap; here turned on its side.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <string.h>
#include <stdio.h>
#include "hub_lcd.h"
#include "esp_lcd_jd9853.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "nvs.h"

static const char *TAG = "lcd";

#define PIN_MOSI  2
#define PIN_SCLK  1
#define PIN_CS   14
#define PIN_DC   15
#define PIN_RST  22
#define PIN_BL   23
#define LCD_HOST SPI2_HOST
#define LCD_HZ   (40 * 1000 * 1000)
#define GAP      34          /* 172 visible of the controller's 240 */
#define BAND     16
#define BL_MAX   50

static esp_lcd_panel_handle_t s_panel;
static SemaphoreHandle_t      s_done, s_lock;
static int                    s_rot;
static bool                   s_ready;

uint16_t lcd_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t v = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return (uint16_t)((v >> 8) | (v << 8));
}

static bool on_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *e, void *ctx)
{
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(s_done, &woken);
    return woken == pdTRUE;
}

/* the four ways to lay 320 along the glass: swap always, then mirror x/y */
static void apply_rot(void)
{
    static const bool mx[4] = { true, false, false, true }, my[4] = { false, true, false, true };
    esp_lcd_panel_swap_xy(s_panel, true);
    esp_lcd_panel_mirror(s_panel, mx[s_rot], my[s_rot]);
    esp_lcd_panel_set_gap(s_panel, 0, GAP);
}

int lcd_rot(void) { return s_rot; }

void lcd_set_rot(int r)
{
    s_rot = r & 3;
    nvs_handle_t h;
    if (nvs_open("hub", NVS_READWRITE, &h) == ESP_OK) { nvs_set_i32(h, "rot", s_rot); nvs_commit(h); nvs_close(h); }
    if (s_ready) apply_rot();
}

void lcd_backlight(int pct)
{
    if (pct < 0) pct = 0;
    if (pct > BL_MAX) pct = BL_MAX;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (uint32_t)(pct * 1023 / 100));
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

esp_err_t lcd_init(void)
{
    s_done = xSemaphoreCreateBinary();
    s_lock = xSemaphoreCreateMutex();
    nvs_handle_t h; int32_t r = 0;
    if (nvs_open("hub", NVS_READONLY, &h) == ESP_OK) { nvs_get_i32(h, "rot", &r); nvs_close(h); }
    s_rot = r & 3;

    spi_bus_config_t bus = {
        .sclk_io_num = PIN_SCLK, .mosi_io_num = PIN_MOSI, .miso_io_num = -1,
        .quadwp_io_num = -1, .quadhd_io_num = -1,
        .max_transfer_sz = LCD_W * BAND * 2,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &bus, SPI_DMA_CH_AUTO));
    esp_lcd_panel_io_handle_t io = NULL;
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = PIN_DC, .cs_gpio_num = PIN_CS, .pclk_hz = LCD_HZ,
        .lcd_cmd_bits = 8, .lcd_param_bits = 8, .spi_mode = 0, .trans_queue_depth = 4,
        .on_color_trans_done = on_done,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_cfg, &io));
    esp_lcd_panel_dev_config_t pc = { .reset_gpio_num = PIN_RST, .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB, .bits_per_pixel = 16 };
    ESP_ERROR_CHECK(esp_lcd_new_panel_jd9853(io, &pc, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(s_panel, true));
    apply_rot();
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));

    ledc_timer_config_t t = { .speed_mode = LEDC_LOW_SPEED_MODE, .duty_resolution = LEDC_TIMER_10_BIT,
                              .timer_num = LEDC_TIMER_0, .freq_hz = 5000, .clk_cfg = LEDC_AUTO_CLK };
    ledc_timer_config(&t);
    ledc_channel_config_t c = { .gpio_num = PIN_BL, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_0,
                                .timer_sel = LEDC_TIMER_0, .duty = 0, .hpoint = 0 };
    ledc_channel_config(&c);
    s_ready = true;
    lcd_clear(C_BG);
    lcd_backlight(40);
    ESP_LOGI(TAG, "JD9853 %dx%d, rot %d", LCD_W, LCD_H, s_rot);
    return ESP_OK;
}

void lcd_blit(int x, int y, int w, int h, const uint16_t *px)
{
    if (!s_ready || w <= 0 || h <= 0) return;
    xSemaphoreTake(s_lock, portMAX_DELAY);
    esp_lcd_panel_draw_bitmap(s_panel, x, y, x + w, y + h, px);
    xSemaphoreTake(s_done, pdMS_TO_TICKS(200));
    xSemaphoreGive(s_lock);
}

static uint16_t s_band[LCD_W * BAND];

void lcd_fill(int x, int y, int w, int h, uint16_t c)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LCD_W) w = LCD_W - x;
    if (y + h > LCD_H) h = LCD_H - y;
    if (w <= 0 || h <= 0) return;
    for (int i = 0; i < w * BAND; i++) s_band[i] = c;
    for (int yy = y; yy < y + h; yy += BAND) {
        int rows = y + h - yy < BAND ? y + h - yy : BAND;
        lcd_blit(x, yy, w, rows, s_band);
    }
}

void lcd_clear(uint16_t c) { lcd_fill(0, 0, LCD_W, LCD_H, c); }

extern const uint8_t font5x7[59][5];

int lcd_text_w(const char *s, int scale) { return (int)strlen(s) * 6 * scale; }

void lcd_text(int x, int y, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    static uint16_t g[6 * 8 * 4 * 8 * 4 / 4];       /* one glyph up to scale 4: 24x32 */
    if (scale < 1) scale = 1;
    if (scale > 4) scale = 4;
    const int gw = 6 * scale, gh = 8 * scale;
    for (const char *p = s; *p; p++, x += gw) {
        if (x + gw > LCD_W) break;
        char ch = *p;
        if (ch >= 'a' && ch <= 'z') ch -= 32;
        int idx = (ch < 32 || ch > 90) ? 0 : ch - 32;
        for (int i = 0; i < gw * gh; i++) g[i] = bg;
        for (int col = 0; col < 5; col++) {
            uint8_t bits = font5x7[idx][col];
            for (int row = 0; row < 7; row++) {
                if (!(bits & (1 << row))) continue;
                for (int sy = 0; sy < scale; sy++)
                    for (int sx = 0; sx < scale; sx++)
                        g[(row * scale + sy) * gw + col * scale + sx] = fg;
            }
        }
        lcd_blit(x, y, gw, gh, g);
    }
}

/* a filled disc with a ring, drawn in bands over bg */
void lcd_disc(int cx, int cy, int r, uint16_t c, int ring, uint16_t ring_c, uint16_t bg)
{
    int x0 = cx - r, w = 2 * r + 1;
    if (x0 < 0 || x0 + w > LCD_W) return;
    int r2 = r * r, ri = r - ring, ri2 = ri * ri;
    for (int y0 = cy - r; y0 <= cy + r; y0 += BAND) {
        int rows = cy + r + 1 - y0 < BAND ? cy + r + 1 - y0 : BAND;
        for (int j = 0; j < rows; j++) {
            int dy = y0 + j - cy;
            for (int i = 0; i < w; i++) {
                int dx = i - r, d = dx * dx + dy * dy;
                s_band[j * w + i] = d > r2 ? bg : d > ri2 ? ring_c : c;
            }
        }
        if (y0 >= 0 && y0 + rows <= LCD_H) lcd_blit(x0, y0, w, rows, s_band);
    }
}

/* corners R top-left, G top-right, B bottom-left, W bottom-right, "TOP" and
 * an F, so one look settles rotation and mirroring */
void lcd_testcard(void)
{
    const int M = 22;
    lcd_clear(C_BLACK);
    lcd_fill(0, 0, LCD_W, 1, C_WHITE); lcd_fill(0, LCD_H - 1, LCD_W, 1, C_WHITE);
    lcd_fill(0, 0, 1, LCD_H, C_WHITE); lcd_fill(LCD_W - 1, 0, 1, LCD_H, C_WHITE);
    lcd_fill(2, 2, M, M, lcd_rgb(255, 0, 0));
    lcd_fill(LCD_W - M - 2, 2, M, M, lcd_rgb(0, 255, 0));
    lcd_fill(2, LCD_H - M - 2, M, M, lcd_rgb(0, 0, 255));
    lcd_fill(LCD_W - M - 2, LCD_H - M - 2, M, M, C_WHITE);
    lcd_text(34, 6, "TOP", C_WHITE, C_BLACK, 2);
    lcd_text(140, 50, "F", C_AMBER, C_BLACK, 4);
    char b[16]; snprintf(b, sizeof b, "ROT %d", s_rot);
    lcd_text(120, 120, b, C_WHITE, C_BLACK, 3);
}
