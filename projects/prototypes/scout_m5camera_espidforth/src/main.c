/*
 * main.c — Scout on an M5Camera (PSRAM) Model B
 *
 * A second camera for LightScopeXR, speaking the same line contract as
 * AlphaFriend straight over BLE (Nordic UART): caps? cam? shoot last lv
 * roll? roll-th roll-b64. The console is on UART0 (CP2104) and BLE, the
 * same ok> prompt either way.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_system.h"

#include "forth_core.h"
#include "forth_version.h"
#include "console.h"
#include "nvs_flash.h"
#include "ble_console.h"
#include "scout_cam.h"
#include "scout_net.h"

#define SCOUT_VERSION   "0.5.0"   /* the lens's RollView uses roll-b64 from 0.3.2 */
#define CONSOLE_UART    UART_NUM_0
#define CONSOLE_BAUD    115200
#define CONSOLE_RX_BUF  256
#define CONSOLE_TX_BUF  512
#define FORTH_HEAP_SIZE (16 * 1024)


/* ----- Console: UART0 + BLE ----- */

static void console_install(void)
{
    const uart_config_t cfg = {
        .baud_rate  = CONSOLE_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(CONSOLE_UART, CONSOLE_RX_BUF, CONSOLE_TX_BUF, 0, NULL, 0);
    uart_param_config(CONSOLE_UART, &cfg);
}

static void console_print(const char *s) { uart_write_bytes(CONSOLE_UART, s, strlen(s)); }

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
    if (c >= 0) return c;
    uint8_t b;
    int n = uart_read_bytes(CONSOLE_UART, &b, 1, pdMS_TO_TICKS(10));
    return n > 0 ? b : -1;
}

static void console_putchar(int c)
{
    char ch = (char)c;
    uart_write_bytes(CONSOLE_UART, &ch, 1);
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

static void w_version(void) { con_printf("scout v%s\n", SCOUT_VERSION); }

/* ----- housekeeping: BLE flush, keep advertising for the glasses ----- */

static void scout_task(void *arg)
{
    for (;;) {
        for (int i = 0; i < 20; i++) {
            vTaskDelay(pdMS_TO_TICKS(50));
            ble_console_flush();
        }
        ble_console_poll();
        /* The glasses can't bond and have no pairing gesture, so Scout
         * advertises whenever nobody is connected. */
        if (ble_console_state() == 0) ble_pair_window(600);
    }
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);          /* compiled at ERROR, see sdkconfig.defaults */
    console_install();
    vTaskDelay(pdMS_TO_TICKS(300));
    console_print("\r\n\r\n============================================\r\n");
    console_printf("  Scout (OpenWR) v%s\r\n", SCOUT_VERSION);
    console_printf("  ESPIDFORTH v%s  build %s %s\r\n", ESPIDFORTH_VERSION_STRING, ESPIDFORTH_BUILD_DATE, ESPIDFORTH_BUILD_TIME);
    console_print("  M5Camera Model B: OV2640, roll on flash\r\n");
    console_print("============================================\r\n");

    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { nvs_flash_erase(); nvs_flash_init(); }

    int rc = forth_init(FORTH_HEAP_SIZE);
    if (rc != 0) { console_printf("forth_init failed (rc=%d)\r\n", rc); return; }

    scout_cam_init();
    scout_register_words();
    scout_net_init();
    scout_net_register_words();
    forth_register_word("version", w_version);
    if (ble_console_init("Scout") == ESP_OK) ble_console_register_words();

    xTaskCreate(scout_task, "scout", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
    ble_pair_window(600);

    console_printf("camera %s, roll %d frames\r\n", scout_cam_up() ? "up" : "DOWN", scout_roll_count());
    console_print("Words: caps? cam? shoot last lv sty! fr! sty? fr? flip! roll? roll-ls roll-b64 roll-th\r\n       wifi! wifi-ps! net? url? web?\r\n");
    forth_repl(console_getchar, console_putchar);
    forth_deinit();
}
