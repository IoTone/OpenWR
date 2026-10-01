/*
 * wx_words.c — geolocation, forecast fetch/parse/cache, accessors, icons.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "esp_log.h"
#include "cJSON.h"

#include "forth_core.h"
#include "console.h"
#include "net_words.h"
#include "ssd1309.h"
#include "wx_words.h"

static const char *TAG = "wx";

static wx_t  s_wx;
static geo_t s_geo;
static int   s_imperial;     /* 0 metric, 1 imperial */

#define HTTP_BUF 4096

/* ------------------------------------------------------------------ */
/* Persistence                                                         */
/* ------------------------------------------------------------------ */

void wx_init(void)
{
    if (!kv_get_blob("geo", &s_geo, sizeof s_geo)) memset(&s_geo, 0, sizeof s_geo);
    if (!kv_get_blob("wx", &s_wx, sizeof s_wx))    memset(&s_wx, 0, sizeof s_wx);
    int32_t u = 0;
    if (kv_get_i32("units", &u)) s_imperial = u ? 1 : 0;
    if (s_wx.valid) clock_set_offset(s_wx.utc_offset_s);
    ESP_LOGI(TAG, "cache: geo %s (%s), forecast %s, units %s",
             s_geo.valid ? s_geo.city : "none", s_geo.manual ? "manual" : "ip",
             s_wx.valid ? "present" : "none", s_imperial ? "imperial" : "metric");
}

bool geo_valid(void) { return s_geo.valid; }
const char *geo_city_str(void) { return s_geo.city; }
int  units_imperial(void) { return s_imperial; }
bool wx_valid(void)  { return s_wx.valid; }

int wx_age_s(void)
{
    if (!s_wx.valid || !s_wx.fetched_at || !clock_synced()) return -1;
    return (int)(clock_unix() - s_wx.fetched_at);
}

static void set_units_from_country(const char *cc)
{
    int32_t stored;
    if (kv_get_i32("units", &stored)) return;          /* user chose; keep it */
    s_imperial = (!strcmp(cc, "US") || !strcmp(cc, "LR") || !strcmp(cc, "MM")) ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Geolocation                                                         */
/* ------------------------------------------------------------------ */

static bool geo_from_ipinfo(const char *body)
{
    cJSON *j = cJSON_Parse(body);
    if (!j) return false;
    bool ok = false;
    cJSON *loc = cJSON_GetObjectItem(j, "loc");
    cJSON *city = cJSON_GetObjectItem(j, "city");
    cJSON *cc = cJSON_GetObjectItem(j, "country");
    if (cJSON_IsString(loc)) {
        double la = 0, lo = 0;
        if (sscanf(loc->valuestring, "%lf,%lf", &la, &lo) == 2) {
            s_geo.lat4 = (int32_t)lround(la * 10000);
            s_geo.lon4 = (int32_t)lround(lo * 10000);
            ok = true;
        }
    }
    if (ok) {
        strncpy(s_geo.city, cJSON_IsString(city) ? city->valuestring : "?", sizeof s_geo.city - 1);
        strncpy(s_geo.country, cJSON_IsString(cc) ? cc->valuestring : "", sizeof s_geo.country - 1);
    }
    cJSON_Delete(j);
    return ok;
}

static bool geo_from_ipapi(const char *body)
{
    cJSON *j = cJSON_Parse(body);
    if (!j) return false;
    bool ok = false;
    cJSON *la = cJSON_GetObjectItem(j, "lat");
    cJSON *lo = cJSON_GetObjectItem(j, "lon");
    cJSON *city = cJSON_GetObjectItem(j, "city");
    cJSON *cc = cJSON_GetObjectItem(j, "countryCode");
    if (cJSON_IsNumber(la) && cJSON_IsNumber(lo)) {
        s_geo.lat4 = (int32_t)lround(la->valuedouble * 10000);
        s_geo.lon4 = (int32_t)lround(lo->valuedouble * 10000);
        strncpy(s_geo.city, cJSON_IsString(city) ? city->valuestring : "?", sizeof s_geo.city - 1);
        strncpy(s_geo.country, cJSON_IsString(cc) ? cc->valuestring : "", sizeof s_geo.country - 1);
        ok = true;
    }
    cJSON_Delete(j);
    return ok;
}

bool geo_fetch(void)
{
    if (s_geo.manual) return true;
    char *buf = malloc(HTTP_BUF);
    if (!buf) return false;
    bool ok = false;

    if (net_http_get("https://ipinfo.io/json", buf, HTTP_BUF) > 0 && geo_from_ipinfo(buf)) {
        ok = true;
        ESP_LOGI(TAG, "geo via ipinfo.io");
    } else if (net_http_get("http://ip-api.com/json/?fields=status,countryCode,city,lat,lon", buf, HTTP_BUF) > 0
               && geo_from_ipapi(buf)) {
        ok = true;
        ESP_LOGI(TAG, "geo via ip-api.com (fallback, plaintext)");
    }
    free(buf);
    if (ok) {
        s_geo.valid = 1;
        set_units_from_country(s_geo.country);
        kv_set_blob("geo", &s_geo, sizeof s_geo);
        ESP_LOGI(TAG, "geo: %s %s %ld,%ld", s_geo.city, s_geo.country, (long)s_geo.lat4, (long)s_geo.lon4);
    } else {
        ESP_LOGE(TAG, "geo: both lookups failed");
    }
    return ok;
}

/* ------------------------------------------------------------------ */
/* Forecast                                                            */
/* ------------------------------------------------------------------ */

static int num_at(cJSON *arr, int i, double scale)
{
    cJSON *v = arr ? cJSON_GetArrayItem(arr, i) : NULL;
    if (!cJSON_IsNumber(v)) return 0;
    return (int)lround(v->valuedouble * scale);
}

bool wx_parse(const char *json)
{
    cJSON *j = cJSON_Parse(json);
    if (!j) { ESP_LOGE(TAG, "json parse failed"); return false; }
    cJSON *hourly = cJSON_GetObjectItem(j, "hourly");
    cJSON *off = cJSON_GetObjectItem(j, "utc_offset_seconds");
    if (!hourly || !cJSON_IsNumber(off)) { cJSON_Delete(j); return false; }

    cJSON *t   = cJSON_GetObjectItem(hourly, "time");
    cJSON *te  = cJSON_GetObjectItem(hourly, "temperature_2m");
    cJSON *fe  = cJSON_GetObjectItem(hourly, "apparent_temperature");
    cJSON *po  = cJSON_GetObjectItem(hourly, "precipitation_probability");
    cJSON *pr  = cJSON_GetObjectItem(hourly, "precipitation");
    cJSON *sn  = cJSON_GetObjectItem(hourly, "snowfall");
    cJSON *wi  = cJSON_GetObjectItem(hourly, "wind_speed_10m");
    cJSON *gu  = cJSON_GetObjectItem(hourly, "wind_gusts_10m");
    cJSON *wd  = cJSON_GetObjectItem(hourly, "wind_direction_10m");
    cJSON *co  = cJSON_GetObjectItem(hourly, "weather_code");
    cJSON *cl  = cJSON_GetObjectItem(hourly, "cloud_cover");
    cJSON *uv  = cJSON_GetObjectItem(hourly, "uv_index");
    cJSON *dy  = cJSON_GetObjectItem(hourly, "is_day");
    if (!t || cJSON_GetArraySize(t) < WX_HOURS || !te) { cJSON_Delete(j); return false; }

    wx_t w = { 0 };
    for (int i = 0; i < WX_HOURS; i++) {
        wx_hour_t *h = &w.h[i];
        h->temp10   = (int16_t)num_at(te, i, 10);
        h->feels10  = (int16_t)num_at(fe, i, 10);
        h->pop      = (uint8_t)num_at(po, i, 1);
        h->precip10 = (uint16_t)num_at(pr, i, 10);
        h->snow10   = (uint16_t)num_at(sn, i, 10);
        h->wind     = (uint8_t)num_at(wi, i, 1);
        h->gust     = (uint8_t)num_at(gu, i, 1);
        h->wdir     = (uint16_t)num_at(wd, i, 1);
        h->code     = (uint8_t)num_at(co, i, 1);
        h->cloud    = (uint8_t)num_at(cl, i, 1);
        h->uv10     = (uint8_t)num_at(uv, i, 10);
        h->is_day   = (uint8_t)num_at(dy, i, 1);
    }
    /* "2026-10-01T14:00" -> 14 */
    cJSON *t0 = cJSON_GetArrayItem(t, 0);
    if (cJSON_IsString(t0) && strlen(t0->valuestring) >= 13)
        w.hour0 = (uint16_t)atoi(t0->valuestring + 11);
    w.utc_offset_s = (int32_t)off->valuedouble;
    w.fetched_at   = clock_synced() ? clock_unix() : 0;
    w.valid        = 1;
    cJSON_Delete(j);

    s_wx = w;
    clock_set_offset(s_wx.utc_offset_s);
    return true;
}

bool wx_fetch(void)
{
    if (!s_geo.valid) return false;
    char *buf = malloc(HTTP_BUF);
    if (!buf) return false;
    /* Coordinates rounded to 2 decimals (~1 km) before they leave the device. */
    int lat2 = (int)lround(s_geo.lat4 / 100.0), lon2 = (int)lround(s_geo.lon4 / 100.0);
    char la[16], lo[16], url[420];
    snprintf(la, sizeof la, "%s%d.%02d", lat2 < 0 ? "-" : "", abs(lat2) / 100, abs(lat2) % 100);
    snprintf(lo, sizeof lo, "%s%d.%02d", lon2 < 0 ? "-" : "", abs(lon2) / 100, abs(lon2) % 100);
    snprintf(url, sizeof url,
        "https://api.open-meteo.com/v1/forecast?latitude=%s&longitude=%s"
        "&hourly=temperature_2m,apparent_temperature,precipitation_probability,precipitation,"
        "snowfall,wind_speed_10m,wind_gusts_10m,wind_direction_10m,weather_code,cloud_cover,"
        "uv_index,is_day&forecast_hours=9&timezone=auto&wind_speed_unit=kmh", la, lo);

    int n = net_http_get(url, buf, HTTP_BUF);
    bool ok = n > 0 && wx_parse(buf);
    free(buf);
    if (ok) {
        kv_set_blob("wx", &s_wx, sizeof s_wx);
        ESP_LOGI(TAG, "forecast ok: hour0=%u t0=%d.%d pop0=%u", s_wx.hour0,
                 s_wx.h[0].temp10 / 10, abs(s_wx.h[0].temp10 % 10), s_wx.h[0].pop);
    } else {
        ESP_LOGE(TAG, "forecast fetch failed (%d bytes)", n);
    }
    return ok;
}

/* ------------------------------------------------------------------ */
/* Fixture                                                             */
/* ------------------------------------------------------------------ */

/* The design fixture (docs/DESIGN-weather-hud.md §7): an autumn afternoon,
 * rain arriving at 17:00. Shaped exactly like an Open-Meteo response so the
 * same parser runs. Loaded by wx-test; lets every theme be drawn offline. */
static const char FIXTURE_JSON[] =
"{\"utc_offset_seconds\":-25200,\"hourly\":{"
"\"time\":[\"2026-10-01T14:00\",\"2026-10-01T15:00\",\"2026-10-01T16:00\",\"2026-10-01T17:00\","
"\"2026-10-01T18:00\",\"2026-10-01T19:00\",\"2026-10-01T20:00\",\"2026-10-01T21:00\",\"2026-10-01T22:00\"],"
"\"temperature_2m\":[15.8,16.1,16.3,16.0,15.2,14.1,13.3,12.8,12.4],"
"\"apparent_temperature\":[15.0,15.4,15.5,15.0,13.9,12.5,11.6,11.0,10.7],"
"\"precipitation_probability\":[5,10,20,55,80,85,70,40,20],"
"\"precipitation\":[0,0,0,0.4,1.6,2.2,1.2,0.5,0.1],"
"\"snowfall\":[0,0,0,0,0,0,0,0,0],"
"\"wind_speed_10m\":[12,14,15,19,26,31,29,22,18],"
"\"wind_gusts_10m\":[20,22,26,33,45,52,48,38,30],"
"\"wind_direction_10m\":[200,210,220,230,240,250,250,245,240],"
"\"weather_code\":[1,2,3,3,61,63,61,51,2],"
"\"cloud_cover\":[20,40,80,95,100,100,100,90,50],"
"\"uv_index\":[3.1,2.4,1.5,0.6,0.1,0,0,0,0],"
"\"is_day\":[1,1,1,1,1,0,0,0,0]}}";

/* ------------------------------------------------------------------ */
/* Icons                                                               */
/* ------------------------------------------------------------------ */

/* 8x8 XBM glyphs (LSB = leftmost pixel), one per condition class. */
enum { IC_SUN, IC_PART, IC_CLOUD, IC_RAIN, IC_DRIZZLE, IC_SNOW, IC_STORM, IC_FOG, IC_COUNT };

static const uint8_t ICON8[IC_COUNT][8] = {
    /* sun     */ {0x08,0x49,0x22,0x1C,0x55,0x1C,0x22,0x49},
    /* part    */ {0x04,0x25,0x0E,0xED,0x94,0x84,0xFC,0x00},
    /* cloud   */ {0x00,0x1C,0x22,0x41,0x81,0x81,0x7E,0x00},
    /* rain    */ {0x1C,0x22,0x81,0x7E,0x00,0x92,0x49,0x00},
    /* drizzle */ {0x1C,0x22,0x81,0x7E,0x00,0x24,0x00,0x12},
    /* snow    */ {0x1C,0x22,0x81,0x7E,0x00,0x55,0xAA,0x55},
    /* storm   */ {0x1C,0x22,0x81,0x7E,0x18,0x0C,0x18,0x04},
    /* fog     */ {0x00,0x3F,0x00,0xFC,0x00,0x3F,0x00,0xFC},
};

static int icon_for(int code)
{
    if (code <= 1) return IC_SUN;
    if (code == 2) return IC_PART;
    if (code == 3) return IC_CLOUD;
    if (code == 45 || code == 48) return IC_FOG;
    if (code >= 51 && code <= 57) return IC_DRIZZLE;
    if ((code >= 61 && code <= 67) || (code >= 80 && code <= 82)) return IC_RAIN;
    if ((code >= 71 && code <= 77) || code == 85 || code == 86) return IC_SNOW;
    if (code >= 95) return IC_STORM;
    return IC_CLOUD;
}

static void draw_icon(int code, int x, int y, int scale)
{
    const uint8_t *g = ICON8[icon_for(code)];
    for (int j = 0; j < 8; j++)
        for (int i = 0; i < 8; i++)
            if ((g[j] >> i) & 1) {
                if (scale == 1) oled_pixel(x + i, y + j);
                else oled_box(x + i * scale, y + j * scale, scale, scale);
            }
}

/* ------------------------------------------------------------------ */
/* Forth words                                                         */
/* ------------------------------------------------------------------ */

#define POP() ((int)forth_pop())

static const wx_hour_t *slot(void)
{
    int i = POP();
    if (i < 0) i = 0;
    if (i >= WX_HOURS) i = WX_HOURS - 1;
    return &s_wx.h[i];
}

static int to_units_temp(int c10)  { return s_imperial ? c10 * 9 / 5 + 320 : c10; }
static int to_units_speed(int kmh) { return s_imperial ? (kmh * 621 + 500) / 1000 : kmh; }

static void w_geo_ok(void)   { forth_push(s_geo.valid ? -1 : 0); }
static void w_geo_lat(void)  { forth_push(s_geo.lat4); }
static void w_geo_lon(void)  { forth_push(s_geo.lon4); }
static void w_geo_city(void) { forth_push((intptr_t)s_geo.city); forth_push((intptr_t)strlen(s_geo.city)); }
static void w_geo_fetch(void){ forth_push(geo_fetch() ? -1 : 0); }
static void w_geo_store(void)
{
    int lon4 = POP(), lat4 = POP();
    s_geo.lat4 = lat4; s_geo.lon4 = lon4;
    s_geo.valid = 1; s_geo.manual = 1;
    strncpy(s_geo.city, "manual", sizeof s_geo.city - 1);
    kv_set_blob("geo", &s_geo, sizeof s_geo);
}

static void w_wx_ok(void)    { forth_push(s_wx.valid ? -1 : 0); }
static void w_wx_age(void)   { forth_push(wx_age_s()); }
static void w_wx_hour0(void) { forth_push(s_wx.hour0); }
static void w_wx_temp(void)  { forth_push(to_units_temp(slot()->temp10)); }
static void w_wx_feels(void) { forth_push(to_units_temp(slot()->feels10)); }
static void w_wx_pop(void)   { forth_push(slot()->pop); }
static void w_wx_precip(void){ forth_push(slot()->precip10); }
static void w_wx_snow(void)  { forth_push(slot()->snow10); }
static void w_wx_wind(void)  { forth_push(to_units_speed(slot()->wind)); }
static void w_wx_gust(void)  { forth_push(to_units_speed(slot()->gust)); }
static void w_wx_wdir(void)  { forth_push(slot()->wdir); }
static void w_wx_code(void)  { forth_push(slot()->code); }
static void w_wx_cloud(void) { forth_push(slot()->cloud); }
static void w_wx_uv(void)    { forth_push(slot()->uv10); }
static void w_wx_day(void)   { forth_push(slot()->is_day ? -1 : 0); }
static void w_wx_fetch(void) { forth_push(wx_fetch() ? -1 : 0); }

/* ( -- ) nine lines, one per slot, for the phone app:
 *   hour temp10 feels10 pop precip10 snow10 wind gust wdir code day
 * temperatures and speeds in the current units, like the wx-* words */
static void w_wx_dump(void)
{
    if (!s_wx.valid) { con_printf("no forecast\n"); return; }
    for (int i = 0; i < WX_HOURS; i++) {
        const wx_hour_t *h = &s_wx.h[i];
        con_printf("%d %d %d %u %u %u %d %d %u %u %u\n",
                   (s_wx.hour0 + i) % 24, to_units_temp(h->temp10), to_units_temp(h->feels10),
                   h->pop, h->precip10, h->snow10, to_units_speed(h->wind), to_units_speed(h->gust),
                   h->wdir, h->code, h->is_day);
    }
}

/* ( -- flag ) load the fixture; also a parser self-test */
static void w_wx_test(void)
{
    bool ok = wx_parse(FIXTURE_JSON);
    if (ok) {
        ok = s_wx.hour0 == 14 && s_wx.h[0].temp10 == 158 && s_wx.h[5].pop == 85
          && s_wx.h[5].precip10 == 22 && s_wx.h[8].code == 2 && s_wx.utc_offset_s == -25200;
        if (!s_geo.valid) { strncpy(s_geo.city, "Fixture", sizeof s_geo.city - 1); s_geo.valid = 1; }
        s_wx.fetched_at = clock_synced() ? clock_unix() : 0;
    }
    con_printf("wx-test: %s\n", ok ? "PASS (fixture loaded)" : "FAIL");
    forth_push(ok ? -1 : 0);
}

static void w_units_store(void) { s_imperial = POP() ? 1 : 0; kv_set_i32("units", s_imperial); }
static void w_units_fetch(void) { forth_push(s_imperial); }

static void w_wx_icon(void)  { int y = POP(), x = POP(), c = POP(); draw_icon(c, x, y, 1); }
static void w_wx_icon2(void) { int y = POP(), x = POP(), c = POP(); draw_icon(c, x, y, 2); }

/* ( deg x y -- ) 7 px tick from (x,y) toward where the wind blows TO. */
static void w_wx_arrow(void)
{
    int y = POP(), x = POP(), deg = POP();
    float a = (float)(deg + 180) * (float)M_PI / 180.0f;
    int dx = (int)lroundf(3.0f * sinf(a)), dy = (int)lroundf(-3.0f * cosf(a));
    oled_line(x, y, x + dx, y + dy);
}

void wx_register_words(void)
{
    static const struct { const char *n; forth_word_fn f; } w[] = {
        { "geo-ok?",   w_geo_ok },   { "geo-lat",  w_geo_lat },  { "geo-lon", w_geo_lon },
        { "geo-city",  w_geo_city }, { "geo!",     w_geo_store }, { "geo-fetch", w_geo_fetch },
        { "wx-ok?",    w_wx_ok },    { "wx-age",   w_wx_age },   { "wx-hour0", w_wx_hour0 },
        { "wx-temp",   w_wx_temp },  { "wx-feels", w_wx_feels }, { "wx-pop", w_wx_pop },
        { "wx-precip", w_wx_precip },{ "wx-snow",  w_wx_snow },  { "wx-wind", w_wx_wind },
        { "wx-gust",   w_wx_gust },  { "wx-wdir",  w_wx_wdir },  { "wx-code", w_wx_code },
        { "wx-cloud",  w_wx_cloud }, { "wx-uv",    w_wx_uv },    { "wx-day?", w_wx_day },
        { "wx-fetch",  w_wx_fetch }, { "wx-test",  w_wx_test },  { "wx-dump", w_wx_dump },
        { "units!",   w_units_store }, { "units@", w_units_fetch },
        { "wx-icon",   w_wx_icon },  { "wx-icon2", w_wx_icon2 }, { "wx-arrow", w_wx_arrow },
    };
    for (size_t i = 0; i < sizeof w / sizeof w[0]; i++) forth_register_word(w[i].n, w[i].f);
}
