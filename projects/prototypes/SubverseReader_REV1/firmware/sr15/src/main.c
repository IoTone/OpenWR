/*
 * main.c: SubverseReader SR-15 Dial Pass firmware, v0.
 *
 * Reads .svpk packs from the microSD card and pages through them with the
 * dial; deep-sleeps between uses. ESPIDFORTH runs on the USB-C console
 * (115200) for bring-up: type sr-info.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "forth_core.h"
#include "forth_version.h"
#include "pins.h"
#include "epd.h"
#include "keys.h"
#include "sd.h"
#include "reader.h"
#include "ble_console.h"
#include "battery.h"

#define SR15_VERSION "0.1.0"
void words_register(void);

/* The console is the USB-C serial port and Bluetooth (NUS) at once: input
 * from either, output to both. */
static void put(int c)
{
    uint8_t ch = (uint8_t)c;
    uart_write_bytes(UART_NUM_0, &ch, 1);
    ble_console_putchar(c);
}

static int get(void)
{
    int c = ble_console_getchar();
    if (c >= 0) { reader_activity(); return c; }
    uint8_t b;
    if (uart_read_bytes(UART_NUM_0, &b, 1, pdMS_TO_TICKS(10)) <= 0) return -1;
    reader_activity();
    return b;
}

/* Raw bytes for sd-recv, from whichever link has them; 0 on timeout. */
int con_read(uint8_t *buf, int n, int ms)
{
    int64_t until = esp_timer_get_time() + (int64_t)ms * 1000;
    do {
        int got = 0, c;
        while (got < n && (c = ble_console_getchar()) >= 0) buf[got++] = (uint8_t)c;
        if (got) return got;
        got = uart_read_bytes(UART_NUM_0, buf, n, pdMS_TO_TICKS(10));
        if (got > 0) return got;
    } while (esp_timer_get_time() < until);
    return 0;
}

static void ble_task(void *arg)
{
    for (int t = 0;; t++) {
        vTaskDelay(pdMS_TO_TICKS(50));
        ble_console_flush();
        if (t % 20 == 0) ble_console_poll();
    }
}

void con_printf(const char *fmt, ...)
{
    char buf[256];
    va_list a;
    va_start(a, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, a);
    va_end(a);
    if (n < 0) return;
    if (n > (int)sizeof buf - 1) n = sizeof buf - 1;
    for (int i = 0; i < n; i++) { if (buf[i] == '\n') put('\r'); put(buf[i]); }
}

void app_main(void)
{
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { nvs_flash_erase(); nvs_flash_init(); }
    uart_driver_install(UART_NUM_0, 16384, 1024, 0, NULL, 0);   /* big RX: sd-recv streams packs while FAT writes stall */

    gpio_config_t led = { .pin_bit_mask = 1ULL << PIN_LED, .mode = GPIO_MODE_OUTPUT };
    gpio_config(&led);
    bool woke = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1;
    if (!woke) {
        con_printf("\n\n============================================\n");
        con_printf("  SubverseReader SR-15 Dial Pass v%s\n", SR15_VERSION);
        con_printf("  ESPIDFORTH v%s, Elecrow CrowPanel 4.2in\n", ESPIDFORTH_VERSION_STRING);
        con_printf("============================================\n");
    }

    battery_init();
    keys_init();                     /* first, so the key that woke us is seen as held */
    if (epd_init() != ESP_OK) con_printf("e-paper init failed\n");
    if (sd_mount() != ESP_OK) con_printf("no microSD card\n");
    reader_start();

    if (forth_init(16 * 1024) != 0) { con_printf("forth_init failed\n"); return; }
    words_register();
    if (ble_console_init("SR-15") == ESP_OK) {
        ble_console_register_words();
        xTaskCreate(ble_task, "blecon", 3072, NULL, 3, NULL);
        if (!woke) ble_pair_window(180);        /* power-up opens a sync window, like goggl1 */
    }
    if (!woke) con_printf("Type sr-info. The pass sleeps after %d s idle (0 sr-sleep! to stay awake).\n", reader_sleep_s());
    forth_repl(get, put);
}
