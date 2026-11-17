/*
 * hub_touch.c — see hub_touch.h. Report format as Zephyr's input_axs5106l:
 * write register 0x01, read 14 bytes; byte 1 low nibble = points, then
 * x (12 bits, big-endian) and y.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdlib.h>
#include <stdio.h>
#include "hub_touch.h"
#include "hub_lcd.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs.h"
#include "forth_core.h"
#include "console.h"

static const char *TAG = "touch";

#define PIN_SDA 18
#define PIN_SCL 19
#define PIN_RST 20
#define PIN_INT 21
#define ADDR    0x63
#define NAT_W   172            /* the panel's own frame: portrait */
#define NAT_H   320
#define SWIPE_PX   40
#define STILL_PX   18
#define TAP_MS     350
#define DOUBLE_MS  350

static i2c_master_dev_handle_t s_dev;
static touch_cb_t s_cb;
static int s_map = 1 | 4;      /* guess: swap, flip y; touch-map! fixes it */
static bool s_log;

/* raw (portrait) coordinates of the last read, for touch-cal */
static int s_rawx, s_rawy;
static int s_cal = -1;                       /* -1 off, else target 0..2 */
static int s_calx[3], s_caly[3];
static const int CAL_X[3] = { 24, LCD_W - 25, LCD_W - 25 }, CAL_Y[3] = { 24, 24, LCD_H - 25 };

static void save_map(void);

static void cal_target(int i)
{
    lcd_clear(C_BLACK);
    lcd_text(70, 74, "TAP THE CROSS", C_AMBER, C_BLACK, 2);
    lcd_fill(CAL_X[i] - 12, CAL_Y[i] - 1, 25, 3, C_WHITE);
    lcd_fill(CAL_X[i] - 1, CAL_Y[i] - 12, 3, 25, C_WHITE);
}

/* three taps: top-left, top-right, bottom-right. TL->TR is a move along the
 * screen's x: whichever raw axis changed most is x (no swap if it's raw x).
 * Then the signs give the flips. */
static void cal_done(void)
{
    int dxx = s_calx[1] - s_calx[0], dxy = s_caly[1] - s_caly[0];    /* along screen x */
    int dyx = s_calx[2] - s_calx[1], dyy = s_caly[2] - s_caly[1];    /* along screen y */
    int m = 0, sx, sy;
    if (abs(dxy) > abs(dxx)) { m |= 1; sx = dxy; sy = dyx; }
    else                     { sx = dxx; sy = dyy; }
    if (sx < 0) m |= 2;
    if (sy < 0) m |= 4;
    s_map = m;
    save_map();
    con_printf("touch-cal map %d (tl %d,%d tr %d,%d br %d,%d)\n", m, s_calx[0], s_caly[0], s_calx[1], s_caly[1], s_calx[2], s_caly[2]);
    lcd_clear(C_BLACK);
    char b[24]; snprintf(b, sizeof b, "TOUCH MAP %d", m);
    lcd_text(70, 74, b, C_GREEN, C_BLACK, 2);
    vTaskDelay(pdMS_TO_TICKS(1200));
    s_cal = -1;
    if (s_cb) { touch_ev_t e = { T_UP, 0, 0, 0, -1 }; s_cb(&e); }   /* ms -1: redraw please */
}

static bool read_point(int *x, int *y)
{
    uint8_t reg = 0x01, b[14];
    if (i2c_master_transmit(s_dev, &reg, 1, 20) != ESP_OK) return false;
    esp_rom_delay_us(50);
    if (i2c_master_receive(s_dev, b, sizeof b, 20) != ESP_OK) return false;
    if ((b[1] & 0x0F) == 0) return false;
    int nx = ((b[2] << 8) | b[3]) & 0x0FFF, ny = ((b[4] << 8) | b[5]) & 0x0FFF;
    s_rawx = nx; s_rawy = ny;
    int sx = nx, sy = ny, sw = NAT_W, sh = NAT_H;
    if (s_map & 1) { sx = ny; sy = nx; sw = NAT_H; sh = NAT_W; }
    if (s_map & 2) sx = sw - 1 - sx;
    if (s_map & 4) sy = sh - 1 - sy;
    *x = sx * LCD_W / sw; *y = sy * LCD_H / sh;
    return true;
}

static void emit(touch_kind_t k, int x, int y, int dir, int ms)
{
    touch_ev_t e = { k, x, y, dir, ms };
    if (s_log) {
        static const char *n[] = { "tap", "double", "swipe", "holding", "hold", "up" };
        if (k != T_HOLDING) con_printf("touch %s %d %d dir %d %d ms\n", n[k], x, y, dir, ms);
    }
    if (s_cb) s_cb(&e);
}

static void touch_task(void *arg)
{
    bool down = false, moved = false, held = false, holding = false;
    int x0 = 0, y0 = 0, x = 0, y = 0, lastx = 0, lasty = 0;
    int64_t t0 = 0, last_tap = 0, last_hold_ev = 0;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(15));
        int nx, ny;
        bool on = read_point(&nx, &ny);
        int64_t now = esp_timer_get_time() / 1000;
        if (on) {
            if (!down) { down = true; moved = held = holding = false; x0 = x = nx; y0 = y = ny; t0 = now; continue; }
            x = nx; y = ny;
            if (s_cal >= 0) continue;
            if (abs(x - x0) > STILL_PX || abs(y - y0) > STILL_PX) moved = true;
            int ms = (int)(now - t0);
            if (!moved && !held && ms >= 400 && now - last_hold_ev >= 100) {
                holding = true; last_hold_ev = now;
                emit(T_HOLDING, x0, y0, 0, ms);
            }
            if (!moved && !held && ms >= HOLD_MS) { held = true; emit(T_HOLD, x0, y0, 0, ms); }
            continue;
        }
        if (!down) continue;
        down = false;                         /* released */
        if (s_cal >= 0) {
            s_calx[s_cal] = s_rawx; s_caly[s_cal] = s_rawy;
            if (s_log) con_printf("touch-cal %d raw %d %d\n", s_cal, s_rawx, s_rawy);
            if (++s_cal == 3) cal_done(); else cal_target(s_cal);
            continue;
        }
        int ms = (int)(now - t0), dx = x - x0, dy = y - y0;
        if (holding || held) emit(T_UP, x0, y0, 0, ms);
        if (held) continue;
        if (abs(dx) > SWIPE_PX && abs(dx) > 2 * abs(dy)) { emit(T_SWIPE, x0, y0, dx > 0 ? 1 : -1, ms); continue; }
        if (moved || ms > TAP_MS) continue;
        if (now - last_tap < DOUBLE_MS && abs(x0 - lastx) < 40 && abs(y0 - lasty) < 40) {
            last_tap = 0; emit(T_DOUBLE, x0, y0, 0, ms);
        } else {
            last_tap = now; lastx = x0; lasty = y0; emit(T_TAP, x0, y0, 0, ms);
        }
    }
}

esp_err_t touch_init(touch_cb_t cb)
{
    s_cb = cb;
    nvs_handle_t h; int32_t m;
    if (nvs_open("hub", NVS_READONLY, &h) == ESP_OK) { if (nvs_get_i32(h, "tmap", &m) == ESP_OK) s_map = m & 7; nvs_close(h); }

    gpio_reset_pin(PIN_RST);
    gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(300));

    i2c_master_bus_config_t bc = { .i2c_port = I2C_NUM_0, .sda_io_num = PIN_SDA, .scl_io_num = PIN_SCL,
                                   .clk_source = I2C_CLK_SRC_DEFAULT, .glitch_ignore_cnt = 7,
                                   .flags.enable_internal_pullup = true };
    i2c_master_bus_handle_t bus;
    esp_err_t e = i2c_new_master_bus(&bc, &bus);
    if (e != ESP_OK) { ESP_LOGW(TAG, "i2c bus: %s", esp_err_to_name(e)); return e; }
    e = i2c_master_probe(bus, ADDR, 50);
    if (e != ESP_OK) { ESP_LOGW(TAG, "no AXS5106L at 0x%02x: %s", ADDR, esp_err_to_name(e)); return e; }
    i2c_device_config_t dc = { .dev_addr_length = I2C_ADDR_BIT_LEN_7, .device_address = ADDR, .scl_speed_hz = 400000 };
    i2c_master_bus_add_device(bus, &dc, &s_dev);
    xTaskCreate(touch_task, "touch", 3072, NULL, 5, NULL);
    ESP_LOGI(TAG, "AXS5106L up, map %d", s_map);
    return ESP_OK;
}

static void save_map(void)
{
    nvs_handle_t h;
    if (nvs_open("hub", NVS_READWRITE, &h) == ESP_OK) { nvs_set_i32(h, "tmap", s_map); nvs_commit(h); nvs_close(h); }
}

static void w_touch_map(void)
{
    s_map = (int)forth_pop() & 7;
    save_map();
    con_printf("touch-map %d\n", s_map);
}
static void w_touch_cal(void) { s_cal = 0; cal_target(0); con_printf("touch-cal: tap the three crosses\n"); }
static void w_touch_log(void) { s_log = !s_log; con_printf("touch-log %s\n", s_log ? "on" : "off"); }

void touch_register_words(void)
{
    forth_register_word("touch-map!", w_touch_map);
    forth_register_word("touch-log", w_touch_log);
    forth_register_word("touch-cal", w_touch_cal);
}
