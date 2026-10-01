/*
 * ota_mnpkg.h — apply a MagNET OTA package (.mnpkg, wire format 1) fetched
 * over HTTP(S). See ProjectMagNET docs/OTA-PACKAGE.md rev 1.4 and
 * docs/DESIGN-provisioning-ota.md §4 here.
 *
 * A package is a 256-byte signed header followed by the ESP-IDF app image.
 * The nine checks run in the spec's order; a refusal names the failing one.
 * The image goes to the other OTA slot and the chip reboots into it; the
 * bootloader rolls back unless the new image confirms itself.
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef OTA_MNPKG_H
#define OTA_MNPKG_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MNPKG_CHIP_ESP32C3        0x0005
#define MNPKG_BOARD_BEETLE_ESP32C3 0x0503   /* proposed registry row */

/* Spec §4 outcomes, 0 = all checks passed and the image is staged + bootable */
enum {
    MNPKG_OK = 0,
    MNPKG_E_FORMAT = 1, MNPKG_E_KEY, MNPKG_E_SIG, MNPKG_E_TARGET, MNPKG_E_REPARTITION,
    MNPKG_E_SIZE, MNPKG_E_SHA, MNPKG_E_IMAGE, MNPKG_E_MIN,
    MNPKG_E_NET = 20, MNPKG_E_FLASH, MNPKG_E_HTTP,
};

const char *mnpkg_err_name(int code);

/* Blocking: download, verify, write, mark bootable. Reboots on success unless
 * reboot is false. Progress is printed to the console. */
int mnpkg_apply_url(const char *url, bool reboot);

/* Print the compiled-in release key store. */
void mnpkg_print_keys(void);

#ifdef __cplusplus
}
#endif
#endif /* OTA_MNPKG_H */
