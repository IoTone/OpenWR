/*
 * wx_sched.c — refresh scheduler, button, theme, and the Forth hook.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "forth_core.h"
#include "net_words.h"
#include "wx_words.h"
#include "pins_beetle_esp32c3.h"
#include "wx_sched.h"

static const char *TAG = "sched";

#define FORECAST_PERIOD_S   (15 * 60)
#define FORECAST_RETRY_S    (2 * 60)
#define SNTP_PERIOD_S       (60 * 60)
#define TICK_PERIOD_S       60
#define THEMES              3

static int  s_theme;
static volatile bool s_tick_now;

/* ------------------------------------------------------------------ */

static void hud_tick(void)
{
    if (forth_word_exists("hud-tick")) forth_eval("hud-tick");
}

static void set_theme(int n)
{
    s_theme = ((n % THEMES) + THEMES) % THEMES;
    kv_set_i32("theme", s_theme);
    s_tick_now = true;
}

static void sched_task(void *arg)
{
    uint32_t t = 0;                     /* seconds since task start */
    uint32_t next_wx = 0, next_sntp = 0, next_tick = 0;
    int      wx_failures = 0;
    bool     geo_pending = false;
    int      btn_held = 0;

    for (;;) {
        /* 50 ms button poll, 20 per second */
        for (int i = 0; i < 20; i++) {
            vTaskDelay(pdMS_TO_TICKS(50));
            bool down = gpio_get_level(GOGGL_PIN_BUTTON) == 0;
            if (down) btn_held++;
            else if (btn_held) {
                if (btn_held >= 2 && btn_held < 20) set_theme(s_theme + 1);   /* 100 ms .. 1 s */
                btn_held = 0;
            }
            if (s_tick_now) { s_tick_now = false; hud_tick(); }
        }
        t++;

        if (net_just_connected()) {
            clock_start_sntp();
            geo_pending = true;
            next_wx = t;                /* fetch as soon as geo is in */
            next_sntp = t + SNTP_PERIOD_S;
        }
        if (!net_ok()) {
            if (t >= next_tick) { hud_tick(); next_tick = t + TICK_PERIOD_S; }
            continue;
        }

        if (geo_pending && clock_synced()) {
            geo_pending = false;
            if (!geo_fetch()) geo_pending = true;   /* try again next second */
        }

        if (!geo_pending && geo_valid() && t >= next_wx) {
            if (wx_fetch()) {
                wx_failures = 0;
                next_wx = t + FORECAST_PERIOD_S;
            } else {
                wx_failures++;
                next_wx = t + (wx_failures < 3 ? FORECAST_RETRY_S : FORECAST_PERIOD_S);
            }
            hud_tick();
            next_tick = t + TICK_PERIOD_S;
        }

        if (t >= next_sntp) { next_sntp = t + SNTP_PERIOD_S; }   /* esp_netif_sntp resyncs itself */

        if (t >= next_tick) { hud_tick(); next_tick = t + TICK_PERIOD_S; }
    }
}

void wx_sched_start(void)
{
    int32_t th = 0;
    if (kv_get_i32("theme", &th)) s_theme = ((th % THEMES) + THEMES) % THEMES;
    xTaskCreate(sched_task, "wx_sched", 6144, NULL, tskIDLE_PRIORITY + 1, NULL);
    ESP_LOGI(TAG, "started, theme %d", s_theme);
}

/* ------------------------------------------------------------------ */

static void w_theme_store(void) { set_theme((int)forth_pop()); }
static void w_theme_fetch(void) { forth_push(s_theme); }
/* Deferred to the scheduler task on purpose: evaluating hud-tick inline
 * would re-enter the interpreter from inside the line being interpreted,
 * and the engine's input cursor is global, so the rest of that line would
 * be lost. The tick lands within 50 ms. */
static void w_hud_now(void)     { s_tick_now = true; }

void wx_sched_register_words(void)
{
    forth_register_word("theme!",  w_theme_store);
    forth_register_word("theme@",  w_theme_fetch);
    forth_register_word("hud-now", w_hud_now);
}
