/*
 * main.c — ReelHub (reelhub_wavec6touch_espidforth).
 *
 * The LightScopeXR shot hub: shows the latest colour shot, swipes through
 * the reel, and fires every camera at once. Board: Waveshare
 * ESP32-C6-Touch-LCD-1.47. Console: the USB-C port (USB-serial-JTAG), as
 * goggl1, and the same prompt over BLE (Nordic UART, "ReelHub").
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "nvs_flash.h"
#include "driver/usb_serial_jtag.h"

#include "forth_core.h"
#include "forth_version.h"
#include "console.h"
#include "ble_console.h"
#include "hub_lcd.h"
#include "hub_touch.h"
#include "hub_reel.h"
#include "hub_net.h"
#include "hub_shoot.h"
#include "hub_web.h"

#define HUB_VERSION     "0.3.0"
#define FORTH_HEAP_SIZE (16 * 1024)

/* ----- console: USB-serial-JTAG, never blocking without a host (goggl1) ----- */

static void console_install(void)
{
    usb_serial_jtag_driver_config_t cfg = { .tx_buffer_size = 512, .rx_buffer_size = 256 };
    usb_serial_jtag_driver_install(&cfg);
}

static void console_print(const char *s)
{
    if (!usb_serial_jtag_is_connected()) return;
    usb_serial_jtag_write_bytes((const uint8_t *)s, strlen(s), pdMS_TO_TICKS(20));
}

static void console_printf(const char *fmt, ...)
{
    char buf[160];
    va_list a; va_start(a, fmt); vsnprintf(buf, sizeof buf, fmt, a); va_end(a);
    console_print(buf);
}

static int console_getchar(void)
{
    int c = ble_console_getchar();
    if (c >= 0) return c;
    uint8_t b;
    return usb_serial_jtag_read_bytes(&b, 1, pdMS_TO_TICKS(10)) > 0 ? b : -1;
}

static void console_putchar(int c)
{
    uint8_t ch = (uint8_t)c;
    if (usb_serial_jtag_is_connected()) usb_serial_jtag_write_bytes(&ch, 1, pdMS_TO_TICKS(2));
    ble_console_putchar(c);
}

void con_printf(const char *fmt, ...)
{
    char buf[256];
    va_list a; va_start(a, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, a);
    va_end(a);
    if (n < 0) return;
    if (n > (int)sizeof buf - 1) n = sizeof buf - 1;
    for (int i = 0; i < n; i++) {
        if (buf[i] == '\n') console_putchar('\r');
        console_putchar(buf[i]);
    }
}

static void w_version(void) { con_printf("reelhub v%s\n", HUB_VERSION); }
static void w_rot(void) { lcd_set_rot((int)forth_pop()); lcd_testcard(); }
static void w_card(void) { lcd_testcard(); }
static void w_redraw(void) { reel_redraw(); }

/* BLE flush and advertising whenever nobody is connected */
static void hub_task(void *arg)
{
    for (;;) {
        for (int i = 0; i < 20; i++) { vTaskDelay(pdMS_TO_TICKS(50)); ble_console_flush(); }
        ble_console_poll();
        if (ble_console_state() == 0) ble_pair_window(600);
    }
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    console_install();
    vTaskDelay(pdMS_TO_TICKS(300));
    console_print("\r\n\r\n============================================\r\n");
    console_printf("  ReelHub (OpenWR) v%s\r\n", HUB_VERSION);
    console_printf("  ESPIDFORTH v%s  build %s %s\r\n", ESPIDFORTH_VERSION_STRING, ESPIDFORTH_BUILD_DATE, ESPIDFORTH_BUILD_TIME);
    console_print("  Waveshare ESP32-C6-Touch-LCD-1.47\r\n");
    console_print("============================================\r\n");

    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { nvs_flash_erase(); nvs_flash_init(); }

    int rc = forth_init(FORTH_HEAP_SIZE);
    if (rc != 0) { console_printf("forth_init failed (rc=%d)\r\n", rc); return; }

    lcd_init();
    lcd_text(60, 70, "REELHUB", C_AMBER, C_BG, 4);
    reel_init();
    reel_register_words();
    hub_net_on_up(hub_web_start);
    hub_net_init();
    hub_net_register_words();
    if (ble_console_init("ReelHub") == ESP_OK) ble_console_register_words();
    xTaskCreate(hub_task, "hub", 3072, NULL, tskIDLE_PRIORITY + 1, NULL);
    ble_pair_window(600);
    reel_redraw();
    shoot_init();
    touch_register_words();
    shoot_register_words();
    forth_register_word("version", w_version);
    forth_register_word("rot!", w_rot);
    forth_register_word("card", w_card);
    forth_register_word("redraw", w_redraw);

    console_printf("reel %d shots, heap %u free, largest %u\r\n", reel_count(),
                   (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL), (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    console_print("Words: shoot cams? cams-find reel? reel-ls reel-show reel-wipe show? url?\r\n"
                  "       rot! card redraw touch-cal touch-map! touch-log  wifi! wifi? wifi-seen net? tz! mem?\r\n");
    forth_repl(console_getchar, console_putchar);
    forth_deinit();
}
