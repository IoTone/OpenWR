/*
 * sd.c: the CrowPanel's microSD on SPI3 (its own bus, not the panel's), with
 * the card's power switch on IO42 so it goes dark in sleep.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_log.h"
#include "pins.h"
#include "sd.h"

static const char *TAG = "sd";
static sdmmc_card_t *s_card;
static bool s_bus;

bool sd_ok(void) { return s_card != NULL; }

esp_err_t sd_mount(void)
{
    if (s_card) return ESP_OK;
    gpio_config_t p = { .pin_bit_mask = 1ULL << PIN_SD_PWR, .mode = GPIO_MODE_OUTPUT };
    gpio_config(&p);
    gpio_set_level(PIN_SD_PWR, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SPI3_HOST;
    host.max_freq_khz = 20000;
    if (!s_bus) {
        spi_bus_config_t bus = { .mosi_io_num = PIN_SD_MOSI, .miso_io_num = PIN_SD_MISO, .sclk_io_num = PIN_SD_SCK,
                                 .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 4096 };
        esp_err_t e = spi_bus_initialize(SPI3_HOST, &bus, SPI_DMA_CH_AUTO);
        if (e != ESP_OK) { ESP_LOGE(TAG, "bus: %s", esp_err_to_name(e)); return e; }
        s_bus = true;
    }
    sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot.gpio_cs = PIN_SD_CS;
    slot.host_id = SPI3_HOST;
    esp_vfs_fat_sdmmc_mount_config_t mc = { .format_if_mount_failed = false, .max_files = 4, .allocation_unit_size = 16 * 1024 };
    esp_err_t e = esp_vfs_fat_sdspi_mount("/sd", &host, &slot, &mc, &s_card);
    if (e != ESP_OK) { ESP_LOGW(TAG, "mount: %s", esp_err_to_name(e)); s_card = NULL; return e; }
    ESP_LOGI(TAG, "card %s, %llu MB", s_card->cid.name, ((uint64_t)s_card->csd.capacity) * s_card->csd.sector_size / (1024 * 1024));
    return ESP_OK;
}

void sd_unmount(void)
{
    if (s_card) esp_vfs_fat_sdcard_unmount("/sd", s_card);
    s_card = NULL;
    gpio_set_level(PIN_SD_PWR, 0);
}
