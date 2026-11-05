/*
 * scout_wdog.c — see scout_wdog.h.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_task_wdt.h"
#include "esp_heap_caps.h"
#include "esp_attr.h"

#include "forth_core.h"
#include "console.h"
#include "ble_console.h"
#include "scout_cam.h"
#include "scout_net.h"
#include "scout_wdog.h"

static const char *TAG = "wdog";

#define BLE_DEAD_S      60
#define WIFI_DEAD_S     180
#define WEB_RETRY_S     10
#define WEB_DEAD_S      60
#define CAM_FAILS       3
#define CAM_BOOTS_MAX   3
#define RAM_LOW         6000
#define RAM_LOW_S       20

/* kept across a software reboot; garbage after power-on (checked by magic) */
#define MAGIC 0x5c0d0a7d
RTC_NOINIT_ATTR static uint32_t s_magic;
RTC_NOINIT_ATTR static char     s_why[40];
RTC_NOINIT_ATTR static uint32_t s_boots;       /* reboots by this watchdog since power-on */
RTC_NOINIT_ATTR static uint32_t s_cam_boots;   /* boots in a row with no camera */

static bool         s_on = true;
static volatile int s_cam_fails;
static int          s_ble_dead, s_wifi_dead, s_web_dead, s_ram_low;

static const char *reason_name(esp_reset_reason_t r)
{
    switch (r) {
    case ESP_RST_POWERON:   return "power-on";
    case ESP_RST_EXT:       return "external";
    case ESP_RST_SW:        return "software";
    case ESP_RST_PANIC:     return "panic";
    case ESP_RST_INT_WDT:   return "int-wdt";
    case ESP_RST_TASK_WDT:  return "task-wdt";
    case ESP_RST_WDT:       return "wdt";
    case ESP_RST_DEEPSLEEP: return "deep-sleep";
    case ESP_RST_BROWNOUT:  return "brownout";
    case ESP_RST_SDIO:      return "sdio";
    default:                return "unknown";
    }
}

static void reboot(const char *why)
{
    if (!s_on) { ESP_LOGW(TAG, "would reboot (%s), but wdog is off", why); return; }
    strncpy(s_why, why, sizeof s_why - 1);
    s_why[sizeof s_why - 1] = 0;
    s_magic = MAGIC;
    s_boots++;
    con_printf("wdog: rebooting: %s\n", why);
    ESP_LOGE(TAG, "rebooting: %s", why);
    vTaskDelay(pdMS_TO_TICKS(200));             /* let the line out */
    esp_restart();
}

void scout_wdog_cam(bool ok)
{
    if (ok) s_cam_fails = 0;
    else s_cam_fails++;
}

static void wdog_task(void *arg)
{
    esp_task_wdt_add(NULL);
    int tick = 0;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_task_wdt_reset();
        tick++;

        /* the window is re-opened every second while nobody is connected */
        s_ble_dead = ble_console_state() == 0 ? s_ble_dead + 1 : 0;
        if (s_ble_dead >= BLE_DEAD_S) reboot("ble: not advertising");

        if (scout_net_configured() && !scout_net_up()) s_wifi_dead++;
        else s_wifi_dead = 0;
        if (s_wifi_dead >= WIFI_DEAD_S) reboot("wifi: no ip");

        if (scout_net_up() && !scout_net_web_up()) {
            s_web_dead++;
            if (s_web_dead % WEB_RETRY_S == 0) scout_net_web_retry();
        } else {
            s_web_dead = 0;
        }
        if (s_web_dead >= WEB_DEAD_S) reboot("web: server down");

        if (s_cam_fails >= CAM_FAILS) reboot("camera: grabs failing");

        s_ram_low = heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < RAM_LOW ? s_ram_low + 1 : 0;
        if (s_ram_low >= RAM_LOW_S) reboot("ram: internal low");

        /* a camera that came up stays counted as fine */
        if (tick == 5 && scout_cam_up()) s_cam_boots = 0;
    }
}

void scout_wdog_init(void)
{
    esp_reset_reason_t r = esp_reset_reason();
    if (r == ESP_RST_POWERON || r == ESP_RST_BROWNOUT || s_magic != MAGIC) {
        s_why[0] = 0;
        s_boots = 0;
        s_cam_boots = 0;
        s_magic = MAGIC;
    }
    con_printf("boot %s%s%s, watchdog reboots %u\n", reason_name(r), s_why[0] ? ", last: " : "", s_why, (unsigned)s_boots);

    if (!scout_cam_up()) {
        if (s_cam_boots < CAM_BOOTS_MAX) {
            s_cam_boots++;
            reboot("camera: no camera at boot");
        }
        con_printf("wdog: no camera after %d boots, leaving it down\n", CAM_BOOTS_MAX);
    }
    xTaskCreate(wdog_task, "wdog", 3072, NULL, tskIDLE_PRIORITY + 1, NULL);
}

/* ---- words -------------------------------------------------------------- */

static void w_boot_q(void)
{
    con_printf("boot %s why %s boots %u up %lld\n", reason_name(esp_reset_reason()),
               s_why[0] ? s_why : "-", (unsigned)s_boots, esp_timer_get_time() / 1000000LL);
}

static void w_wdog_store(void)
{
    s_on = forth_pop() != 0;
    con_printf("wdog %s\n", s_on ? "on" : "off (until reboot)");
}

static void w_wdog_q(void)
{
    con_printf("wdog %s ble %d wifi %d web %d cam %d ram %u\n", s_on ? "on" : "off",
               s_ble_dead, s_wifi_dead, s_web_dead, s_cam_fails,
               (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
}

void scout_wdog_register_words(void)
{
    forth_register_word("boot?", w_boot_q);
    forth_register_word("wdog!", w_wdog_store);
    forth_register_word("wdog?", w_wdog_q);
}
