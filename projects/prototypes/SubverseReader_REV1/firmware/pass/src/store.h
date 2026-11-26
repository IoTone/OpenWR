/*
 * store.h: where the packs live. SR-15: the microSD card (store_sd.c).
 * SR-16: a FAT partition in the XIAO's flash (store_flash.c).
 * Licensed under the Apache License, Version 2.0
 */
#ifndef PASS_STORE_H
#define PASS_STORE_H
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "board.h"

#if BOARD_STORE_SD
#define STORE_ROOT  "/sd"
#define STORE_KIND  "card"
#else
#define STORE_ROOT  "/fl"
#define STORE_KIND  "flash"
#endif
#define STORE_PACKS STORE_ROOT "/packs"

esp_err_t store_mount(void);
void store_unmount(void);            /* before sleep: and power the card off */
bool store_ok(void);
bool store_space(uint32_t *total_kb, uint32_t *free_kb);
#endif
