/*
 * main.c: SubverseReader pass firmware, for SR-15 Dial Pass and SR-16
 * Setlist (board.h says which).
 *
 * Reads .svpk packs from the store (microSD card or flash) and pages through
 * them; deep-sleeps between uses. ESPIDFORTH runs on the USB-C console and
 * over Bluetooth for bring-up and for the phone app: type sr-info.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "forth_core.h"
#include "forth_version.h"
#include "board.h"
#include "console.h"
#include "epd.h"
#include "keys.h"
#include "store.h"
#include "reader.h"
#include "ble_console.h"
#include "battery.h"

#define PASS_VERSION "0.2.0"
void words_register(void);

static void put(int c) { con_put(c); }
static int  get(void)  { return con_get(); }

static void ble_task(void *arg)
{
    for (int t = 0;; t++) {
        vTaskDelay(pdMS_TO_TICKS(50));
        ble_console_flush();
        if (t % 20 == 0) ble_console_poll();
    }
}

void app_main(void)
{
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) { nvs_flash_erase(); nvs_flash_init(); }
    reader_keys_init();              /* first, so the key that woke us is seen as held */
    con_init();

#if PIN_LED >= 0
    gpio_config_t led = { .pin_bit_mask = 1ULL << PIN_LED, .mode = GPIO_MODE_OUTPUT };
    gpio_config(&led);
#endif
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    bool timer = cause == ESP_SLEEP_WAKEUP_TIMER;
    bool woke = timer || cause == ESP_SLEEP_WAKEUP_EXT1;
    if (!woke) {
        con_printf("\n\n============================================\n");
        con_printf("  SubverseReader %s v%s\n", BOARD_TITLE, PASS_VERSION);
        con_printf("  ESPIDFORTH v%s, %s\n", ESPIDFORTH_VERSION_STRING, BOARD_HW);
        con_printf("============================================\n");
    }

    battery_init();
    if (epd_init() != ESP_OK) con_printf("e-paper init failed\n");
    if (store_mount() != ESP_OK) con_printf("no " STORE_KIND "\n");
    reader_start();

    if (forth_init(16 * 1024) != 0) { con_printf("forth_init failed\n"); return; }
    words_register();
    if (ble_console_init(BOARD_NAME) == ESP_OK) {
        ble_console_register_words();
        xTaskCreate(ble_task, "blecon", 3072, NULL, 3, NULL);
        /* Power-up opens a sync window, like goggl1. A timer wake is a
         * check-in: a few seconds for a waiting phone to connect. */
        if (!woke) ble_pair_window(180);
        else if (timer && reader_sync_s() > 0) ble_pair_window(5);
    }
    if (!woke) con_printf("Type sr-info. The pass sleeps after %d s idle (0 sr-sleep! to stay awake).\n", reader_sleep_s());
    forth_repl(get, put);
}
