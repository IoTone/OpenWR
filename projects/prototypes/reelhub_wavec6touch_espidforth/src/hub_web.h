/*
 * hub_web.h — the reel over HTTP (port 80, CORS open), for the glasses.
 *
 *   GET  /reel.txt          one line per shot, oldest first:
 *                           <file> <camera> <w> <h> <hh:mm:ss> <bytes>
 *                           then: showing <file>|none
 *   GET  /reel/<file>.jpg   that shot (e.g. /reel/0012.jpg)
 *   POST /reel/add?label=SPECS
 *                           a JPEG (raw, or base64 text) into the reel; it
 *                           shows at once. Answers "added <file>".
 *
 * The glasses learn the address over BLE (url?), as with Scout.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef HUB_WEB_H
#define HUB_WEB_H
void hub_web_start(void);     /* on every new IP; once up, stays up */
#endif
