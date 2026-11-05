/*
 * scout_wdog.h — Scout watches itself and reboots when a radio or the
 * camera has died.
 *
 * ESP32-CAM boards are flaky: on 2026-11-04 Scout dropped off both BLE and
 * Wi-Fi twice while unattended and needed a power-cycle. Nobody is at the
 * board when the glasses lose it, so it recovers on its own:
 *
 *   - the task watchdog (15 s, panic -> reboot) covers the housekeeping
 *     task and this one: a hang anywhere in them reboots;
 *   - BLE: not connected and not advertising for 60 s (the housekeeping
 *     task re-opens the window every second, so that means NimBLE is gone);
 *   - Wi-Fi: configured but without an IP for 180 s; up but the web server
 *     not running for 60 s (it is retried every 10 s first);
 *   - camera: 3 failed grabs in a row (LAN frames or shots), or no camera at
 *     boot (rebooted at most 3 boots running, then left down);
 *   - internal RAM under 6 KB for 20 s.
 *
 * The reason survives the reboot in RTC memory, so the next boot prints it.
 *
 *   boot?   ( -- )     boot <reset reason> why <watchdog reason> boots <n> up <s>
 *   wdog!   ( flag -- ) 0 turns the reboots off until the next boot (debugging)
 *   wdog?   ( -- )     wdog on|off ble <s> wifi <s> web <s> cam <fails> ram <free>
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef SCOUT_WDOG_H
#define SCOUT_WDOG_H

#include <stdbool.h>

void scout_wdog_init(void);              /* after the camera, Wi-Fi and BLE are started */
void scout_wdog_cam(bool ok);            /* every camera grab: did it give a frame? */
void scout_wdog_register_words(void);

#endif
