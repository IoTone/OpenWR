/*
 * wx_words.h — geolocation and forecast for the goggl1 weather HUD
 * (design §3, §4, §6).
 *
 * Location: ipinfo.io over HTTPS, ip-api.com over HTTP as fallback.
 * Forecast: Open-Meteo, 9 hourly slots from the current hour.
 *
 * Words:
 *   geo-ok? geo-lat geo-lon geo-city geo! geo-fetch
 *   wx-ok? wx-age wx-hour0 wx-temp wx-feels wx-pop wx-precip wx-snow
 *   wx-wind wx-gust wx-wdir wx-code wx-cloud wx-uv wx-day? wx-fetch
 *   units!  units@
 *   wx-icon wx-icon2 wx-arrow
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef WX_WORDS_H
#define WX_WORDS_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WX_HOURS 9

typedef struct {
    int16_t  temp10;
    int16_t  feels10;
    uint8_t  pop;
    uint16_t precip10;
    uint16_t snow10;
    uint8_t  wind;
    uint8_t  gust;
    uint16_t wdir;
    uint8_t  code;
    uint8_t  cloud;
    uint8_t  uv10;
    uint8_t  is_day;
} wx_hour_t;

typedef struct {
    wx_hour_t h[WX_HOURS];
    int32_t   utc_offset_s;
    uint32_t  fetched_at;
    uint16_t  hour0;
    uint8_t   valid;
} wx_t;

typedef struct {
    int32_t lat4, lon4;      /* degrees x 10000 */
    char    city[24];
    char    country[3];
    uint8_t valid;
    uint8_t manual;          /* set by geo! — never overwritten by lookup */
} geo_t;

void wx_init(void);          /* load cached geo/forecast/units from NVS */
bool geo_fetch(void);        /* blocking; returns true on success */
bool wx_fetch(void);         /* blocking; needs a valid geo */
bool geo_valid(void);
const char *geo_city_str(void);
int  units_imperial(void);
bool wx_valid(void);
int  wx_age_s(void);         /* -1 if never fetched */

/* Parse an Open-Meteo JSON body into the live forecast (used by wx_fetch and
 * by the test word against a stored fixture). */
bool wx_parse(const char *json);

void wx_register_words(void);

#ifdef __cplusplus
}
#endif
#endif /* WX_WORDS_H */
