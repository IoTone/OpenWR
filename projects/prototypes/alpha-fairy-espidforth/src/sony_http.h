/*
 * sony_http.h — Sony's Camera Remote API (JSON-RPC over HTTP) for bodies in
 * "Ctrl w/ Smartphone", with AlphaFriend joined to the camera's own AP.
 *
 * Ported from Alpha Fairy's SonyHttpCamera library (Frank Zhao, MIT; see
 * third_party/alpha-fairy/LICENSE): the SSDP search, the dd.xml parse, the
 * init sequence, the JSON key scanner and the command formats. The Arduino
 * state machine became plain blocking calls, because each Forth word waits
 * for its own reply anyway.
 *
 * Words (sony_register_words):
 *   cam-join  ( -- )          find the camera on the AP, startRecMode, first getEvent
 *   cam?      ( -- )          cam <off|join|api|ready|err> <model> <rssi>
 *   cam-api   ( -- )          api <method> ... (getAvailableApiList, the L0 go/no-go)
 *   cam-rpc   ( addr u -- )   call a no-argument method, print the raw reply
 *   cam-evt   ( -- )          getEvent; evt <key> <value> per known key
 *   cam-dbg   ( flag -- )     print every request and reply
 *   shoot     ( -- )          actTakePicture -> ok / err <code>
 *   af  af-   ( -- )          actHalfPressShutter / cancelHalfPressShutter
 *   rec rec-  ( -- )          startMovieRec / stopMovieRec
 *   f!        ( x10 -- )      setFNumber: 56 f! is f/5.6, 110 f! is f/11
 *   ss!       ( addr u -- )   setShutterSpeed: s" 1/250" ss!  s" 2s" ss! is 2"
 *   iso!      ( n -- )        setIsoSpeedRate; 0 is AUTO
 *   ev!       ( thirds -- )   setExposureCompensation: -3 ev! is -1.0 EV
 *
 * Licensed under the Apache License, Version 2.0 (the ported parts: MIT)
 */

#ifndef SONY_HTTP_H
#define SONY_HTTP_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { SHC_OFF, SHC_JOIN, SHC_API, SHC_READY, SHC_ERR } shc_state_t;

shc_state_t shc_state(void);
const char *shc_state_name(void);
const char *shc_model(void);              /* friendlyName from dd.xml, "-" until known */
const char *shc_postview(void);           /* URL of the last postview, "" if none */
const char *shc_liveview(void);           /* liveview URL from dd.xml, "" if none */

bool shc_connect(void);
void shc_tick(void);
int  shc_shoot(bool verbose);            /* half-press, shoot, liveview again; 0 or an error code */
int  shc_shots(void);                    /* shots taken since boot (console or button) */
void shc_lock(void);                     /* one talker to the camera at a time */
void shc_unlock(void);                      /* each second: rejoin a lost camera every 10 s */                   /* SSDP / dd.xml / startRecMode / getEvent */

/* One JSON-RPC call to the camera service. params is the inside of the
 * "params" array, e.g. "" or "\"5.6\"". Returns the Sony error code (0 = ok,
 * -1 = no reply). The reply body stays in shc_reply() until the next call. */
int shc_call(const char *method, const char *params, const char *version);
const char *shc_reply(void);

/* Alpha Fairy's JSON scanner: finds "key": <value> in data without a parse
 * tree. The value lands in tgt without its quotes or brackets. */
bool shc_scan_json(const char *data, int len, const char *key, int *start, int *end, char *tgt, int tgtlen);

void sony_register_words(void);

#ifdef __cplusplus
}
#endif
#endif /* SONY_HTTP_H */
