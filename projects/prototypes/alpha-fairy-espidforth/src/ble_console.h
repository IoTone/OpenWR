/*
 * ble_console.h — the ok> prompt over Bluetooth LE (Nordic UART Service).
 * See docs/DESIGN-provisioning-ota.md.
 *
 * Bytes written to NUS RX feed the same console input the USB port uses;
 * everything the console prints is also sent as NUS TX notifications.
 * The radio advertises only inside a pairing window opened by a long
 * press of BOOT or the `pair` word.
 *
 * Words:
 *   pair      ( -- )        open a 3-minute pairing window
 *   unpair    ( -- )        close it and drop any connection
 *   ble-on?   ( -- flag )   advertising or connected
 *   ble-conn  ( -- )        print interval, latency, timeout, MTU
 *   ble-conn! ( min max -- ) ask for a connection interval (1.25 ms units)
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef BLE_CONSOLE_H
#define BLE_CONSOLE_H

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ble_console_init(const char *name);

/* Console integration */
int  ble_console_getchar(void);          /* -1 if nothing pending */
void ble_console_putchar(int c);         /* buffered; flushed on '\n' or full */
void ble_console_flush(void);            /* call periodically (50 ms is fine) */
void ble_console_poll(void);             /* call every second: window expiry */

/* Pairing window */
void ble_pair_window(int seconds);
void ble_pair_close(void);
int  ble_console_state(void);            /* 0 off, 1 advertising, 2 connected */
uint32_t ble_console_dropped(void);      /* notifications lost since boot */

void ble_console_register_words(void);

#ifdef __cplusplus
}
#endif
#endif /* BLE_CONSOLE_H */
