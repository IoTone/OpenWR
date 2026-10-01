/*
 * main.c — esp32c3_beetle_goggl1
 *
 * ESPIDFORTH's src/main.c with the goggle vocabulary added between
 * forth_init() and forth_repl(). Boots the display, draws a splash, then
 * hands the USB console to the Forth REPL.
 *
 * Board: DFRobot Beetle ESP32-C3 (DFR0868). Console is USB-serial-JTAG.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "driver/usb_serial_jtag.h"

#include "forth_core.h"
#include "forth_version.h"
#include "goggle_words.h"
#include "net_words.h"
#include "wx_words.h"
#include "wx_sched.h"
#include "bundle.h"

#define CONSOLE_RX_BUF 256
#define CONSOLE_TX_BUF 256

/* Forth dictionary heap. With WiFi + TLS on the C3 (no PSRAM) a TLS handshake
 * needs ~40 KB transiently and free heap is ~85 KB before WiFi even starts,
 * so the dictionary gets 32 KB. The three themes use under 1 KB of it;
 * compiled code lives in the engine's own static array. */
#define FORTH_HEAP_SIZE (32 * 1024)

/* ----- Console: raw byte I/O over USB-serial-JTAG ----- */

static void console_install(void)
{
    usb_serial_jtag_driver_config_t cfg = {
        .tx_buffer_size = CONSOLE_TX_BUF,
        .rx_buffer_size = CONSOLE_RX_BUF,
    };
    usb_serial_jtag_driver_install(&cfg);
}

static void console_print(const char *s)
{
    usb_serial_jtag_write_bytes((const uint8_t *)s, strlen(s), pdMS_TO_TICKS(500));
}

static void console_printf(const char *fmt, ...)
{
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    console_print(buf);
}

static int console_getchar(void)
{
    uint8_t c;
    int n = usb_serial_jtag_read_bytes(&c, 1, pdMS_TO_TICKS(10));
    if (n <= 0) return -1;
    if (bundle_console_char(c)) return -1;   /* swallowed by bundle capture */
    return c;
}

static void console_putchar(int c)
{
    uint8_t ch = (uint8_t)c;
    usb_serial_jtag_write_bytes(&ch, 1, pdMS_TO_TICKS(100));
}

void app_main(void)
{
    console_install();
    vTaskDelay(pdMS_TO_TICKS(500));   /* let the USB host enumerate */

    console_print("\r\n\r\n");
    console_print("============================================\r\n");
    console_print("  esp32c3_beetle_goggl1 (OpenWR)\r\n");
    console_printf("  ESPIDFORTH v%s  build %s %s\r\n",
                   ESPIDFORTH_VERSION_STRING, ESPIDFORTH_BUILD_DATE, ESPIDFORTH_BUILD_TIME);
    console_print("  Beetle ESP32-C3 + SSD1309 transparent OLED\r\n");
    console_print("============================================\r\n");

    console_printf("Free heap before init: %lu bytes\r\n",
                   (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    int rc = forth_init(FORTH_HEAP_SIZE);
    if (rc != 0) {
        console_printf("forth_init failed (rc=%d)\r\n", rc);
        return;
    }

    esp_err_t derr = goggle_init();
    goggle_register_words();

    esp_err_t nerr = net_init();
    if (nerr != ESP_OK) console_printf("net_init failed: %s\r\n", esp_err_to_name(nerr));
    net_register_words();
    wx_init();
    wx_register_words();
    wx_sched_register_words();
    bundle_register_words();
    if (derr == ESP_OK) {
        goggle_test_pattern();   /* power-on layout pattern; oled-splash for the logo */
        console_print("Display up, showing the layout test pattern.\r\n");
        console_print("Try: oled-clear s\" hi\" 0 0 oled-str oled-show\r\n");
    } else {
        console_printf("Display init failed: %s (REPL still available)\r\n", esp_err_to_name(derr));
    }

    console_printf("Free heap after init:  %lu bytes\r\n",
                   (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    /* The hosted use case: re-apply the stored Forth bundle, if there is one.
     * It ends with hud-tick, so the cached forecast is on the glass before
     * the network is even up. */
    if (!bundle_apply_stored()) console_print("No bundle stored; test pattern stays up.\r\n");

    wx_sched_start();   /* geolocation, forecast, clock, button, hud-tick */

    forth_repl(console_getchar, console_putchar);   /* blocks until 'bye' */
    forth_deinit();
}
