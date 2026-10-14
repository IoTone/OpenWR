/*
 * ota_words.c — ota!, version, status.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
#include "esp_crt_bundle.h"
#include "esp_heap_caps.h"

#include "forth_core.h"
#include "console.h"
#include "net_words.h"
#include "epd.h"
#include "ble_console.h"
#include "bundle.h"
#include "ota_words.h"
#include "ota_mnpkg.h"

static const char *TAG = "ota";

/* ------------------------------------------------------------------ */
/* OTA                                                                 */
/* ------------------------------------------------------------------ */

static void ota_task(void *arg)
{
    char *url = (char *)arg;
    size_t ul = strlen(url);
    if (ul > 6 && strcmp(url + ul - 6, ".mnpkg") == 0) {
        mnpkg_apply_url(url, true);      /* prints its own verdict; reboots on success */
        free(url);
        vTaskDelete(NULL);
        return;
    }
    con_printf("ota: raw image (no package header); prefer a .mnpkg\n");
    esp_http_client_config_t http = {
        .url               = url,
        .timeout_ms        = 15000,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t cfg = { .http_config = &http };
    esp_https_ota_handle_t h = NULL;

    con_printf("ota: fetching %s\n", url);
    esp_err_t err = esp_https_ota_begin(&cfg, &h);
    if (err != ESP_OK) {
        con_printf("ota: begin failed: %s\n", esp_err_to_name(err));
        goto out;
    }
    int total = esp_https_ota_get_image_size(h), last_pct = -1;
    for (;;) {
        err = esp_https_ota_perform(h);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) break;
        int done = esp_https_ota_get_image_len_read(h);
        int pct = total > 0 ? done * 100 / total : -1;
        if (pct / 10 != last_pct / 10) { con_printf("ota: %d%% (%d bytes)\n", pct, done); last_pct = pct; }
    }
    if (err != ESP_OK) {
        con_printf("ota: download failed: %s\n", esp_err_to_name(err));
        esp_https_ota_abort(h);
        goto out;
    }
    if (!esp_https_ota_is_complete_data_received(h)) {
        con_printf("ota: incomplete image, aborting\n");
        esp_https_ota_abort(h);
        goto out;
    }
    err = esp_https_ota_finish(h);
    if (err != ESP_OK) {
        con_printf("ota: finish failed: %s\n", esp_err_to_name(err));
        goto out;
    }
    con_printf("ota: written and verified, rebooting into the new image\n");
    
    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
out:
    free(url);
    vTaskDelete(NULL);
}

/* ( url-addr u -- ) */
static void w_ota(void)
{
    int len = (int)forth_pop();
    const char *s = (const char *)forth_pop();
    if (!s || len <= 0 || len > 300) { con_printf("? ota! bad url\n"); return; }
    if (!net_ok()) { con_printf("ota: no network\n"); return; }
    char *url = malloc(len + 1);
    if (!url) return;
    memcpy(url, s, len); url[len] = '\0';
    if (xTaskCreate(ota_task, "ota", 8192, url, tskIDLE_PRIORITY + 2, NULL) != pdPASS) {
        con_printf("ota: could not start\n"); free(url);
    }
}

void ota_mark_boot_ok(void)
{
    const esp_partition_t *run = esp_ota_get_running_partition();
    esp_ota_img_states_t st;
    if (esp_ota_get_state_partition(run, &st) == ESP_OK && st == ESP_OTA_IMG_PENDING_VERIFY) {
        esp_ota_mark_app_valid_cancel_rollback();
        ESP_LOGI(TAG, "new image confirmed on %s", run->label);
    }
}

/* ------------------------------------------------------------------ */
/* version, status                                                     */
/* ------------------------------------------------------------------ */

static void w_ota_keys(void) { mnpkg_print_keys(); }

static void w_version(void)
{
    const esp_partition_t *run = esp_ota_get_running_partition();
    con_printf("maplet v%s on %s, built %s %s\n", GOGGL1_VERSION, run ? run->label : "?", __DATE__, __TIME__);
}

static void w_status(void)
{
    uint32_t a = net_ip4();
    int h = 0, m = 0; clock_local_hm(&h, &m);
    char bname[48]; bundle_name(bname, sizeof bname);
    extern int skin_last_slot(void);
    con_printf("maplet v%s net=%d ip=%u.%u.%u.%u clock=%d %02d:%02d map=%d epd=%s:%dx%d bundle=%s heap=%u ble=%d bledrop=%lu\n",
           GOGGL1_VERSION, net_ok() ? 1 : 0,
           (unsigned)(a & 0xFF), (unsigned)((a >> 8) & 0xFF), (unsigned)((a >> 16) & 0xFF), (unsigned)(a >> 24),
           clock_ok() ? 1 : 0, h, m, skin_last_slot(), epd_driver_name(epd_driver_current()), epd_w(), epd_h(), bname,
           (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL), ble_console_state(), (unsigned long)ble_console_dropped());
}

void ota_register_words(void)
{
    forth_register_word("ota!",    w_ota);
    forth_register_word("ota-keys", w_ota_keys);
    forth_register_word("version", w_version);
    forth_register_word("status",  w_status);
}
