/*
 * wx_sched.h — the refresh scheduler and the use-case hook (design §5.1).
 *
 * One task. Geolocation once per network join, forecast every 15 min with
 * backoff on failure, SNTP hourly, `hud-tick` every 60 s and after any
 * change, `hud-anim` every 200 ms when the bundle defines it. BOOT button
 * short press cycles the theme, long press opens the BLE pairing window.
 *
 * Words:
 *   theme!   ( n -- )   0 strip, 1 glance, 2 ribbon; persisted
 *   theme@   ( -- n )
 *   hud-now  ( -- )     request a hud-tick from the scheduler (within 50 ms)
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef WX_SCHED_H
#define WX_SCHED_H

#ifdef __cplusplus
extern "C" {
#endif

void wx_sched_start(void);      /* call after forth_init() and all registrations */
void wx_sched_register_words(void);
int  wx_sched_theme(void);

#ifdef __cplusplus
}
#endif
#endif /* WX_SCHED_H */
