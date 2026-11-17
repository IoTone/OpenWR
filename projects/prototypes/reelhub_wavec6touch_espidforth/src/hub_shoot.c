/*
 * hub_shoot.c — see hub_shoot.h.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs.h"
#include "hub_shoot.h"
#include "hub_touch.h"
#include "hub_reel.h"
#include "hub_lcd.h"
#include "hub_net.h"
#include "forth_core.h"
#include "console.h"

static const char *TAG = "shoot";

#define BTN_X   (PIC_W / 2)
#define BTN_Y   (LCD_H / 2)
#define BTN_R   30
#define BTN_SHOW_MS 6000
#define BAR_X   (STRIP_X + 4)
#define BAR_Y   156
#define BAR_W   (LCD_W - STRIP_X - 8)
#define BAR_H   8
#define JPG_CAP (96 * 1024)

static QueueHandle_t s_q;            /* touch events, to the UI task */
static QueueHandle_t s_fire;         /* shoot requests, to the shoot task */
static bool     s_btn;               /* the shutter button is showing */
static int64_t  s_btn_at;
static int      s_shots;
static char     s_scout[64];

static void on_touch(const touch_ev_t *e) { xQueueSend(s_q, e, 0); }

static void button(bool on)
{
    s_btn = on;
    s_btn_at = esp_timer_get_time() / 1000;
    if (on) lcd_disc(BTN_X, BTN_Y, BTN_R, C_WHITE, 5, C_AMBER, C_BLACK);
    else reel_redraw();              /* the picture under it comes back */
}

static void bar(int ms)
{
    int w = ms >= HOLD_MS ? BAR_W : BAR_W * ms / HOLD_MS;
    lcd_fill(BAR_X, BAR_Y, w, BAR_H, C_AMBER);
    if (w < BAR_W) lcd_fill(BAR_X + w, BAR_Y, BAR_W - w, BAR_H, C_DIM);
}

static void ui_task(void *arg)
{
    touch_ev_t e;
    bool bar_on = false;
    for (;;) {
        if (!xQueueReceive(s_q, &e, pdMS_TO_TICKS(200))) {
            if (s_btn && esp_timer_get_time() / 1000 - s_btn_at > BTN_SHOW_MS) button(false);
            continue;
        }
        switch (e.kind) {
        case T_SWIPE:
            if (s_btn) s_btn = false;
            reel_step(-e.dir);                     /* finger left: the next (newer) one */
            break;
        case T_DOUBLE:
            if (e.x < PIC_W) button(!s_btn);
            break;
        case T_TAP:
            if (s_btn) {
                int dx = e.x - BTN_X, dy = e.y - BTN_Y;
                if (dx * dx + dy * dy <= (BTN_R + 12) * (BTN_R + 12)) { s_btn = false; shoot_fire("button"); }
            }
            break;
        case T_HOLDING:
            bar_on = true;
            bar(e.ms);
            break;
        case T_HOLD:
            bar(HOLD_MS);
            shoot_fire("hold");
            break;
        case T_UP:
            if (e.ms < 0) { reel_redraw(); break; }       /* after touch-cal */
            if (bar_on) { lcd_fill(BAR_X, BAR_Y, BAR_W, BAR_H, C_BG); bar_on = false; }
            break;
        }
    }
}

/* ---- the cameras --------------------------------------------------------- */

static void flash(void)
{
    lcd_backlight(50);
    lcd_clear(C_WHITE);
    vTaskDelay(pdMS_TO_TICKS(90));
    lcd_clear(C_BLACK);
    lcd_backlight(40);
}

static bool get_scout(uint8_t *buf)
{
    if (!s_scout[0]) return false;
    char url[80];
    snprintf(url, sizeof url, "%s/frame.jpg", s_scout);
    int64_t t0 = esp_timer_get_time();
    int n = hub_http_get(url, buf, JPG_CAP, 4000);
    int ms = (int)((esp_timer_get_time() - t0) / 1000);
    if (n <= 0) { con_printf("shot scout err %d\n", n); reel_strip_note("SCOUT", "NO PICTURE", C_RED); return false; }
    int num = reel_add(buf, n, "SCOUT");
    con_printf("shot scout %d B %d ms #%d\n", n, ms, num);
    return num > 0;
}

static void shoot_task(void *arg)
{
    char why[12];
    for (;;) {
        xQueueReceive(s_fire, why, portMAX_DELAY);
        int id = ++s_shots;
        con_printf("fire %d\n", id);                 /* to USB and BLE: the glasses shoot too */
        flash();
        reel_redraw();
        reel_strip_note("SHOOTING", "", C_AMBER);
        uint8_t *buf = malloc(JPG_CAP);
        int got = 0;
        if (!buf) con_printf("shot err memory\n");
        else {
            got += get_scout(buf);
            free(buf);
        }
        reel_strip_note(got ? "" : "NO CAMERA", got ? "" : "ANSWERED", got ? C_AMBER : C_RED);
        ESP_LOGI(TAG, "fire %d (%s): %d pictures", id, why, got);
    }
}

void shoot_fire(const char *why)
{
    char w[12];
    snprintf(w, sizeof w, "%s", why);
    xQueueSend(s_fire, w, 0);
}

void shoot_init(void)
{
    nvs_handle_t h; size_t n = sizeof s_scout;
    if (nvs_open("hub", NVS_READONLY, &h) == ESP_OK) { nvs_get_str(h, "scout", s_scout, &n); nvs_close(h); }
    s_q = xQueueCreate(16, sizeof(touch_ev_t));
    s_fire = xQueueCreate(2, 12);
    xTaskCreate(ui_task, "ui", 4096, NULL, 4, NULL);
    xTaskCreate(shoot_task, "shoot", 6144, NULL, 3, NULL);
    touch_init(on_touch);
}

/* ---- words --------------------------------------------------------------- */

static void w_shoot(void) { shoot_fire("word"); }
static void w_cams_q(void)
{
    con_printf("cam scout http %s\n", s_scout[0] ? s_scout : "none (scout-url!)");
    con_printf("cam specs ble %s\n", "fire line");
}
static void w_scout_url(void)
{
    int l = (int)forth_pop(); const char *s = (const char *)forth_pop();
    if (!s || l < 0 || l >= (int)sizeof s_scout) { con_printf("? scout-url! bad args\n"); return; }
    memcpy(s_scout, s, l); s_scout[l] = 0;
    while (l && s_scout[l - 1] == '/') s_scout[--l] = 0;
    nvs_handle_t h;
    if (nvs_open("hub", NVS_READWRITE, &h) == ESP_OK) { nvs_set_str(h, "scout", s_scout); nvs_commit(h); nvs_close(h); }
    con_printf("scout %s\n", s_scout);
}

void shoot_register_words(void)
{
    forth_register_word("shoot", w_shoot);
    forth_register_word("cams?", w_cams_q);
    forth_register_word("scout-url!", w_scout_url);
}
