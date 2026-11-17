/*
 * scout_net.h — Scout on the LAN: Wi-Fi station and a small web server
 * with the camera's JPEG frames. The glasses learn the URL over BLE
 * (url?), since Spectacles have no mDNS, and fetch frames by IP.
 *
 *   wifi!     ( ssid u pass u -- )  save a network (up to 5, NVS) and join it now
 *   wifi?     ( -- )                wifi <n> <ssid> <rssi|-> [on] ..., wifi saved <n>
 *   wifi-seen ( -- )                the last scan: seen <ssid> <rssi> ch <n> [open]
 *   wifi-forget ( n -- )            forget saved network n
 *   wifi-first  ( n -- )            make saved network n the first choice
 *             At boot and whenever it loses Wi-Fi, Scout scans and joins the
 *             first saved network it can see, falling back down the list.
 *   wifi-ps!  ( flag -- )           1 modem sleep (default, needed with BLE), 0 awake
 *   net?      ( -- )                net up <ip> rssi <dBm> <ssid> | net down
 *   url?      ( -- )                url http://<ip>/ ws://<ip>/ws | url none
 *   web?      ( -- )                web jpg <n> ws <n> mjpeg <n> last <client ip> <ms ago>
 *   name!     ( addr u -- )         this camera's name (BLE, hostname, ReelHub); saved, next boot
 *   name?     ( -- )                name <name>
 *
 * ReelHub, UDP port 5555 (broadcast): "who" -> "cam <name> http://<ip>/ <w> <h>";
 * "snap <id>" -> grab and keep a frame, "snapped <id> <bytes> <name> <url>"; then GET /snap.jpg.
 *
 * HTTP (port 80, CORS open):
 *   /           a test page: refreshes /frame.jpg in a loop and shows FPS
 *   /frame.jpg  one JPEG, fresh from the sensor
 *   /snap.jpg   the frame the last "snap" kept (header X-Snap: <id>)
 *   /stream     multipart MJPEG
 *   /ws         WebSocket: any message in, one binary JPEG out
 *   /info       {"w":320,"h":240,"jpg":n,"ws":n,"mjpeg":n}
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef SCOUT_NET_H
#define SCOUT_NET_H

#include <stdbool.h>

void scout_net_init(void);          /* after nvs_flash_init and the camera */
void scout_net_register_words(void);
bool scout_net_up(void);
bool scout_net_configured(void);    /* has credentials, so it should be up */
bool scout_net_web_up(void);
const char *scout_name(void);     /* "Scout" unless name! (or the build) says otherwise */
void scout_net_web_retry(void);

#endif
