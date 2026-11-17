/*
 * hub_net.h — ReelHub on Wi-Fi: saved networks with fallback (as Scout
 * 0.7.0), the clock (SNTP), and HTTP GET for the cameras on the LAN.
 *
 *   wifi!       ( ssid u pass u -- )  save a network (up to 5, NVS) and join it now
 *   wifi?       ( -- )                wifi <n> <ssid> <rssi|-> [on] ..., wifi saved <n>
 *   wifi-seen   ( -- )                the last scan: seen <ssid> <rssi> ch <n> [open]
 *   wifi-forget ( n -- )              forget saved network n
 *   wifi-first  ( n -- )              make saved network n the first choice
 *   wifi-ps!    ( flag -- )           1 modem sleep (default, needed with BLE), 0 awake
 *   net?        ( -- )                net up <ip> rssi <dBm> <ssid> | net down
 *   tz!         ( addr u -- )         time zone, POSIX TZ string; saved (default UTC0)
 *   mem?        ( -- )                internal heap free / largest block
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef HUB_NET_H
#define HUB_NET_H
#include <stdbool.h>
#include <stdint.h>

void hub_net_init(void);
void hub_net_register_words(void);
bool hub_net_up(void);
bool hub_net_configured(void);
void hub_net_on_up(void (*fn)(void));      /* called on every new IP */
/* bytes read, or < 0: -1 no Wi-Fi, -4 bigger than cap, -<status> HTTP error */
int  hub_http_get(const char *url, uint8_t *buf, int cap, int timeout_ms);

#endif
