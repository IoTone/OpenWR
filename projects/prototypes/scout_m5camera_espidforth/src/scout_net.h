/*
 * scout_net.h — Scout on the LAN: Wi-Fi station and a small web server
 * with the camera's JPEG frames. The glasses learn the URL over BLE
 * (url?), since Spectacles have no mDNS, and fetch frames by IP.
 *
 *   wifi!     ( ssid u pass u -- )  store credentials (NVS) and connect
 *   wifi-ps!  ( flag -- )           1 modem sleep (default, needed with BLE), 0 awake
 *   net?      ( -- )                net up <ip> rssi <dBm> | net down
 *   url?      ( -- )                url http://<ip>/ ws://<ip>/ws | url none
 *   web?      ( -- )                web jpg <n> ws <n> mjpeg <n> last <client ip> <ms ago>
 *
 * HTTP (port 80, CORS open):
 *   /           a test page: refreshes /frame.jpg in a loop and shows FPS
 *   /frame.jpg  one JPEG, fresh from the sensor
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

#endif
