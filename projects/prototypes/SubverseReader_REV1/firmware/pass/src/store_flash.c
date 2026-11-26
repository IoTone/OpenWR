/*
 * store_flash.c (SR-16): packs in the XIAO's own flash. The `packs`
 * partition (about 6 MB of the 8 MB chip) is a wear-levelled FAT volume
 * mounted at /fl; it is formatted the first time. At 10-30 KB a zine that is
 * a couple of hundred zines.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include "board.h"
#if defined(BOARD_SR16)
#include "esp_vfs_fat.h"
#include "wear_levelling.h"
#include "esp_log.h"
#include "store.h"

static const char *TAG = "store";
static wl_handle_t s_wl = WL_INVALID_HANDLE;

bool store_ok(void) { return s_wl != WL_INVALID_HANDLE; }

esp_err_t store_mount(void)
{
    if (store_ok()) return ESP_OK;
    esp_vfs_fat_mount_config_t mc = { .format_if_mount_failed = true, .max_files = 4, .allocation_unit_size = 4096 };
    esp_err_t e = esp_vfs_fat_spiflash_mount_rw_wl(STORE_ROOT, "packs", &mc, &s_wl);
    if (e != ESP_OK) { ESP_LOGE(TAG, "mount: %s", esp_err_to_name(e)); s_wl = WL_INVALID_HANDLE; }
    return e;
}

void store_unmount(void)
{
    if (!store_ok()) return;
    esp_vfs_fat_spiflash_unmount_rw_wl(STORE_ROOT, s_wl);
    s_wl = WL_INVALID_HANDLE;
}

bool store_space(uint32_t *total_kb, uint32_t *free_kb)
{
    uint64_t total, fr;
    if (!store_ok() || esp_vfs_fat_info(STORE_ROOT, &total, &fr) != ESP_OK) return false;
    *total_kb = (uint32_t)(total / 1024); *free_kb = (uint32_t)(fr / 1024);
    return true;
}
#endif
