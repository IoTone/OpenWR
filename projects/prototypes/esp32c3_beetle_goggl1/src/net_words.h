/*
 * net_words.h — WiFi station, SNTP clock, NVS key/value, HTTP GET, and the
 * Forth words over them, for the goggl1 weather HUD (design §3.3, §5, §6).
 *
 * Words:
 *   net-ok?    ( -- flag )            associated and has an IP
 *   wifi!      ( ssid u pass u -- )   store credentials in NVS and reconnect
 *   net-ip     ( -- a b c d )
 *   clock-ok?  ( -- flag )            SNTP synced and a UTC offset is known
 *   clock-hm   ( -- h m )             local time
 *   clock-s    ( -- unix )
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef NET_WORDS_H
#define NET_WORDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* NVS namespace shared by every goggl1 word that persists something. */
#define GOGGL_NVS_NS "goggl1"

esp_err_t net_init(void);                 /* nvs + netif + wifi; connects if creds stored */
bool      net_ok(void);
bool      net_just_connected(void);       /* true once per (re)association; clears on read */
void      net_set_credentials(const char *ssid, const char *pass);

/* Blocking GET. Returns body length, or -1. Body is NUL-terminated. */
int net_http_get(const char *url, char *buf, size_t cap);

/* Clock */
void     clock_start_sntp(void);
bool     clock_synced(void);
void     clock_set_offset(int32_t utc_offset_s);  /* from the forecast response */
bool     clock_ok(void);
uint32_t clock_unix(void);
void     clock_local_hm(int *h, int *m);

/* NVS helpers used by the geo/wx modules too */
bool kv_get_str(const char *key, char *out, size_t cap);
void kv_set_str(const char *key, const char *val);
bool kv_get_i32(const char *key, int32_t *out);
void kv_set_i32(const char *key, int32_t val);
bool kv_get_blob(const char *key, void *out, size_t len);
void kv_set_blob(const char *key, const void *data, size_t len);

void net_register_words(void);

#ifdef __cplusplus
}
#endif
#endif /* NET_WORDS_H */
