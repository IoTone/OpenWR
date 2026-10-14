/*
 * main.c — maplet_xiao_eink
 *
 * The e-paper "skin": ESPIDFORTH with the console on USB and over BLE, the
 * stored-bundle and .mnpkg OTA machinery from goggl1, and a 212x104 e-paper
 * the phone pushes finished pictures to. Boots showing the last saved map.
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
#include "net_words.h"
#include "bundle.h"
#include "ble_console.h"
#include "ota_words.h"
#include "ota_mnpkg.h"
#include "console.h"
#include "pins.h"
#include "epd.h"
#include "driver/gpio.h"
#include "skin_words.h"
#include "cam_words.h"

#define CONSOLE_RX_BUF 256
#define CONSOLE_TX_BUF 256
#define FORTH_HEAP_SIZE (16 * 1024)

/* ----- Console: USB (when a host is attached) + BLE ----- */

static void console_install(void)
{
    usb_serial_jtag_driver_config_t cfg = { .tx_buffer_size = CONSOLE_TX_BUF, .rx_buffer_size = CONSOLE_RX_BUF };
    usb_serial_jtag_driver_install(&cfg);
}

static bool usb_listening(void) { return usb_serial_jtag_is_connected(); }

static void console_print(const char *s)
{
    if (!usb_listening()) return;
    usb_serial_jtag_write_bytes((const uint8_t *)s, strlen(s), pdMS_TO_TICKS(20));
}

static void console_printf(const char *fmt, ...)
{
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    console_print(buf);
}

static int console_getchar(void)
{
    int c = ble_console_getchar();
    if (c < 0) {
        uint8_t b;
        int n = usb_serial_jtag_read_bytes(&b, 1, pdMS_TO_TICKS(10));
        if (n <= 0) return -1;
        c = b;
    }
    if (bundle_console_char(c)) return -1;
    return c;
}

static void console_putchar(int c)
{
    uint8_t ch = (uint8_t)c;
    if (usb_listening()) usb_serial_jtag_write_bytes(&ch, 1, pdMS_TO_TICKS(2));
    ble_console_putchar(c);
}

void con_printf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    if (n < 0) return;
    if (n > (int)sizeof buf - 1) n = sizeof buf - 1;
    for (int i = 0; i < n; i++) {
        if (buf[i] == '\n') console_putchar('\r');
        console_putchar(buf[i]);
    }
}

/* ----- housekeeping: BLE flush/poll, hook for the bundle ----- */

/* Hold BOOT for a second to open the pairing window. The glass shows a
 * PAIR badge over the current picture while the window is open, and goes
 * back to the picture when a phone connects or the window closes. */
static uint8_t s_before_badge[EPD_CANVAS_MAX];
static bool s_badge;

static void pair_badge_show(void)
{
    memcpy(s_before_badge, epd_canvas(), EPD_CANVAS);
    int x = skin_safe_x() + skin_safe_w() - 40, y = 3;
    epd_box(x, y, 36, 15, 0);
    epd_frame(x, y, 36, 15);
    epd_frame(x + 1, y + 1, 34, 13);
    epd_str(x + 6, y + 4, "PAIR", 4, 1);
    epd_show();
    s_badge = true;
}

static void pair_badge_clear(void)
{
    memcpy(epd_canvas(), s_before_badge, EPD_CANVAS);
    epd_show();
    s_badge = false;
}

static void skin_task(void *arg)
{
    uint32_t t = 0;
    int held = 0;
    for (;;) {
        for (int i = 0; i < 20; i++) {
            vTaskDelay(pdMS_TO_TICKS(50));
            ble_console_flush();
            if (gpio_get_level(SKIN_PIN_BOOT) == 0) {
                if (++held == 20) {                               /* 1 s */
                    ble_pair_window(180);
                    console_print("ble: pairing window opened from BOOT\r\n");
                    if (ble_console_state() != 2) pair_badge_show();
                }
            } else {
                held = 0;
            }
        }
        t++;
        ble_console_poll();
        if (s_badge && ble_console_state() != 1) pair_badge_clear();   /* connected, or the window closed */
        if (net_just_connected()) clock_start_sntp();
        /* a bundle may define skin-tick for a once-a-minute job */
        if ((t % 60) == 0 && forth_word_exists("skin-tick")) forth_eval("skin-tick");
    }
}

void app_main(void)
{
    console_install();
    vTaskDelay(pdMS_TO_TICKS(500));

    console_print("\r\n\r\n============================================\r\n");
    console_printf("  maplet_xiao_eink (OpenWR) v%s\r\n", GOGGL1_VERSION);
    console_printf("  ESPIDFORTH v%s  build %s %s\r\n", ESPIDFORTH_VERSION_STRING, ESPIDFORTH_BUILD_DATE, ESPIDFORTH_BUILD_TIME);
    console_print("  " SKIN_BOARD_NAME " + 2.13\" e-paper 212x104\r\n");
    console_print("============================================\r\n");

    int rc = forth_init(FORTH_HEAP_SIZE);
    if (rc != 0) { console_printf("forth_init failed (rc=%d)\r\n", rc); return; }

    esp_err_t nerr = net_init();
    if (nerr != ESP_OK) console_printf("net_init failed: %s\r\n", esp_err_to_name(nerr));
    net_register_words();
    bundle_register_words();
    ota_register_words();
    if (ble_console_init("maplet") == ESP_OK) ble_console_register_words();

    esp_err_t derr = epd_init();
    skin_init();
    skin_register_words();
    cam_init_all();
    cam_register_words();
    if (derr == ESP_OK) {
        bool map = skin_show_boot();
        console_printf("e-paper up, showing %s.\r\n", map ? "the last saved map" : "the splash");
    } else {
        console_printf("e-paper init failed: %s (console still available)\r\n", esp_err_to_name(derr));
    }

    if (!bundle_apply_stored()) console_print("No bundle stored.\r\n");

    xTaskCreate(skin_task, "skin", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
    ota_mark_boot_ok();
    console_print("OTA release keys:\r\n");
    mnpkg_print_keys();

    /* Power-up is the pairing gesture: three minutes, like goggl1. */
    ble_pair_window(180);

    forth_repl(console_getchar, console_putchar);
    forth_deinit();
}
