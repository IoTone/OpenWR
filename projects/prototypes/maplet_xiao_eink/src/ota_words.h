/*
 * ota_words.h — firmware update over HTTP(S) into the other OTA slot, and
 * the one-line `status` report. See docs/DESIGN-provisioning-ota.md §4, §5.
 *
 * Words:
 *   ota!      ( url u -- )   fetch a .mnpkg (or a raw image), verify, write, reboot into it
 *   ota-keys  ( -- )         print the compiled-in release keys
 *   version   ( -- )         print firmware version and running slot
 *   status    ( -- )         one line: net, geo, forecast age, theme, heap…
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef OTA_WORDS_H
#define OTA_WORDS_H

#define GOGGL1_VERSION "0.3.2"   /* skin1 firmware version (name kept for the shared OTA code) */

#ifdef __cplusplus
extern "C" {
#endif

/* Call once the boot has reached a trusted point (bundle applied, scheduler
 * running). Cancels the bootloader's pending rollback for this image. */
void ota_mark_boot_ok(void);

void ota_register_words(void);

#ifdef __cplusplus
}
#endif
#endif /* OTA_WORDS_H */
