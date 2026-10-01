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
#include "console.h"
#include "net_words.h"
#include "wx_words.h"
#include "pins_beetle_esp32c3.h"
#include "ble_console.h"
#include "wx_sched.h"

static const char *TAG = "sched";

#define FORECAST_PERIOD_S   (15 * 60)
#define FORECAST_RETRY_S    (2 * 60)
#define SNTP_PERIOD_S       (60 * 60)
#define TICK_PERIOD_S       60
#define ANIM_PERIOD_MS      200     /* hud-anim at 5 fps, if the bundle defines it */
#define THEMES              3

static int  s_theme;
static volatile bool s_tick_now;

/* loop state, exposed by sched-info for the bench */
static volatile uint32_t s_t, s_next_wx;
static volatile int s_wx_failures;
static volatile bool s_geo_pending;

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
    bool     have_anim = false;

    for (;;) {
        have_anim = forth_word_exists("hud-anim");   /* once a second is plenty */
        /* 50 ms button poll, 20 per second */
        for (int i = 0; i < 20; i++) {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (have_anim && (i % (ANIM_PERIOD_MS / 50)) == 0) forth_eval("hud-anim");
            ble_console_flush();
            if (s_tick_now) { s_tick_now = false; hud_tick(); }
        }
        t++;
        s_t = t; s_next_wx = next_wx; s_wx_failures = wx_failures; s_geo_pending = geo_pending;
        ble_console_poll();

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

/* The BOOT button gets its own task so a forecast fetch (seconds of TLS)
 * can never swallow a press. Sampled every 20 ms with a debounce; a short
 * press (60 ms .. 800 ms) cycles the theme on release; holding for 1 s
 * opens the pairing window at once, while the thumb is still down, so the
 * B appears as feedback. */
static void button_task(void *arg)
{
    int held = 0;
    bool fired = false;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(20));
        bool down = gpio_get_level(GOGGL_PIN_BUTTON) == 0;
        if (down) {
            held++;
            if (held == 50 && !fired) {          /* 1 s */
                fired = true;
                ble_pair_window(180);
                s_tick_now = true;
            }
        } else {
            if (held >= 3 && held < 40 && !fired) set_theme(s_theme + 1);   /* 60 .. 800 ms */
            held = 0;
            fired = false;
        }
    }
}

void wx_sched_start(void)
{
    int32_t th = 0;
    if (kv_get_i32("theme", &th)) s_theme = ((th % THEMES) + THEMES) % THEMES;
    xTaskCreate(sched_task, "wx_sched", 6144, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(button_task, "button", 2048, NULL, tskIDLE_PRIORITY + 2, NULL);
    ESP_LOGI(TAG, "started, theme %d", s_theme);
}

/* ------------------------------------------------------------------ */

int wx_sched_theme(void) { return s_theme; }

static void w_theme_store(void) { set_theme((int)forth_pop()); }
static void w_theme_fetch(void) { forth_push(s_theme); }
/* Deferred to the scheduler task on purpose: evaluating hud-tick inline
 * would re-enter the interpreter from inside the line being interpreted,
 * and the engine's input cursor is global, so the rest of that line would
 * be lost. The tick lands within 50 ms. */
static void w_hud_now(void)     { s_tick_now = true; }
static void w_sched_info(void)
{
    con_printf("sched: t=%lu next_wx=%lu (in %ld s) geo_pending=%d wx_failures=%d net=%d geo=%d\n",
               (unsigned long)s_t, (unsigned long)s_next_wx, (long)((int32_t)s_next_wx - (int32_t)s_t),
               s_geo_pending, s_wx_failures, net_ok(), geo_valid());
}

void wx_sched_register_words(void)
{
    forth_register_word("theme!",  w_theme_store);
    forth_register_word("theme@",  w_theme_fetch);
    forth_register_word("hud-now", w_hud_now);
    forth_register_word("sched-info", w_sched_info);
}
