/*
 * wx_sched.h — the refresh scheduler and the use-case hook (design §5.1).
 *
 * One task. Geolocation once per network join, forecast every 15 min with
 * backoff on failure, SNTP hourly, `hud-tick` every 60 s and after any
 * change. BOOT button short press cycles the theme.
 *
 * Words:
 *   theme!   ( n -- )   0 strip, 1 glance, 2 ribbon; persisted
 *   theme@   ( -- n )
 *   hud-now  ( -- )     run hud-tick immediately (debug)
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

#ifdef __cplusplus
}
#endif
#endif /* WX_SCHED_H */
