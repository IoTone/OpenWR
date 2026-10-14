/*
 * skin_words.c — the maplet Forth vocabulary: e-paper canvas, saved maps,
 * and the board words.
 *
 * E-paper (canvas 212x104, 1 = black; nothing reaches the panel until epd-show):
 *   epd-clear     ( -- )
 *   epd-cursor!   ( n -- )            byte cursor into the canvas
 *   epd-cursor@   ( -- n )
 *   epd-hex!      ( c-addr u -- )     append bytes written as hex at the cursor
 *   epd-pixel     ( x y -- )
 *   epd-line      ( x0 y0 x1 y1 -- )
 *   epd-box       ( x y w h -- )      filled, black
 *   epd-frame     ( x y w h -- )
 *   epd-str       ( c-addr u x y -- ) 5x7 text at the current scale
 *   epd-scale     ( n -- )            1..4
 *   epd-show      ( -- )              full refresh (~2 s) then panel sleep
 *   epd-save      ( n -- )            keep the canvas as map n (0..3) in flash
 *   epd-load      ( n -- flag )       canvas from map n
 *   epd-slot@     ( -- n )            last slot saved, -1 if none
 *   epd-splash    ( -- )              the brand splash
 *   epd-test      ( -- )              calibration pattern (then epd-show)
 *   epd-geom!     ( n -- )            0: native 104x212 (default)  1: native 122x250
 *   epd-cdi!      ( n -- )            VCOM/data interval byte (default 0x97 = 151)
 *   epd-old!      ( n -- )            old frame: 0 white (default), 1 inverse of new, 2 skip
 *   epd-lut!      ( n -- )            0: LUT from register (default)  1: LUT from OTP
 *   epd-spi!      ( hz -- )           SPI clock (default 4000000)
 *   epd-csb!      ( n -- )            1: chip select around every byte (vendor demo style)
 *   epd-fill      ( n -- )            0 white 1 black 2 h-stripes 3 v-stripes 4 checker 5 half
 *   epd-safe!     ( x w -- )          columns that render on this glass (NVS); epd-safe@ ( -- x w )
 *   epd-drivers   ( -- )              list the panel drivers, * on the active one
 *   epd-driver!   ( c-addr u -- )     choose a panel by name (NVS); s" ssd1680-290" epd-driver!
 *   epd-driver    ( -- )              print the active panel
 *   epd-size@     ( -- w h )          canvas size of the active panel
 *   epd-rot!      ( n -- )            0..3 orientation of the canvas on the glass (NVS)
 *
 * Board: ms us random reboot gpio@ button?
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "nvs.h"
#include "esp_partition.h"

#include "forth_core.h"
#include "console.h"
#include "net_words.h"
#include "pins.h"
#include "epd.h"
#include "skin_words.h"

static const char *TAG = "skin";
static int s_cursor;
static int s_scale = 1;
static int s_last_slot = -1;

#define POP() ((int)forth_pop())
#define SLOTS 4

/* ---- canvas words ------------------------------------------------- */

static void w_clear(void)   { epd_clear(); s_cursor = 0; }
static void w_cursor_store(void) { int n = POP(); s_cursor = n < 0 ? 0 : n > EPD_CANVAS ? EPD_CANVAS : n; }
static void w_cursor_fetch(void) { forth_push(s_cursor); }

static int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* ( c-addr u -- ) hex pairs; anything that is not hex is skipped */
static void w_hex_store(void)
{
    int n = POP(); const char *s = (const char *)forth_pop();
    if (!s || n <= 0) return;
    uint8_t *cv = epd_canvas();
    int hi = -1;
    for (int i = 0; i < n; i++) {
        int v = hexval(s[i]);
        if (v < 0) continue;
        if (hi < 0) { hi = v; continue; }
        if (s_cursor < EPD_CANVAS) cv[s_cursor++] = (uint8_t)((hi << 4) | v);
        hi = -1;
    }
}

static void w_pixel(void) { int y = POP(), x = POP(); epd_pixel(x, y, 1); }
static void w_line(void)  { int y1 = POP(), x1 = POP(), y0 = POP(), x0 = POP(); epd_line(x0, y0, x1, y1); }
static void w_box(void)   { int h = POP(), w = POP(), y = POP(), x = POP(); epd_box(x, y, w, h, 1); }
static void w_frame(void) { int h = POP(), w = POP(), y = POP(), x = POP(); epd_frame(x, y, w, h); }
static void w_scale(void) { int n = POP(); s_scale = n < 1 ? 1 : n > 4 ? 4 : n; }
static void w_str(void)
{
    int y = POP(), x = POP(), len = POP(); const char *s = (const char *)forth_pop();
    if (s && len > 0) epd_str(x, y, s, len, s_scale);
}

static void w_show(void)
{
    /* A hex upload that stopped short is a frame for another panel size
     * (an older phone app, say). Refuse it rather than show a scramble. */
    if (s_cursor > 0 && s_cursor < EPD_CANVAS) {
        con_printf("epd-show: refused, got %d of %d bytes for %dx%d (is the app current?)\n", s_cursor, EPD_CANVAS, EPD_W, EPD_H);
        s_cursor = 0;
        return;
    }
    s_cursor = 0;
    int64_t t0 = esp_timer_get_time();
    epd_show();
    con_printf("epd: shown in %lld ms\n", (long long)((esp_timer_get_time() - t0) / 1000));
}

/* ---- saved maps ---------------------------------------------------- */
/* Four slots. On the S3 they live in the 'maps' data partition (8 KB a
 * slot, a small header then the canvas), so a 296x128 frame fits; on the
 * C3 they stay NVS blobs. Each slot records the canvas size it was drawn
 * for, and a slot from a different panel is refused rather than shown. */

typedef struct { uint32_t magic; uint16_t w, h; uint32_t len; } slot_hdr_t;
#define SLOT_MAGIC 0x4D504C54u   /* "MPLT" */
#define SLOT_SPAN  8192

static const esp_partition_t *maps_part(void)
{
    static const esp_partition_t *p; static bool looked;
    if (!looked) { p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "maps"); looked = true; }
    return p;
}

static void slot_key(int n, char *k) { snprintf(k, 16, "map%d", n); }

static bool slot_write(int n)
{
    const esp_partition_t *p = maps_part();
    if (p) {
        slot_hdr_t h = { SLOT_MAGIC, (uint16_t)EPD_W, (uint16_t)EPD_H, (uint32_t)EPD_CANVAS };
        size_t off = (size_t)n * SLOT_SPAN;
        if (esp_partition_erase_range(p, off, SLOT_SPAN) != ESP_OK) return false;
        if (esp_partition_write(p, off, &h, sizeof h) != ESP_OK) return false;
        return esp_partition_write(p, off + sizeof h, epd_canvas(), EPD_CANVAS) == ESP_OK;
    }
    char k[16]; slot_key(n, k);
    kv_set_blob(k, epd_canvas(), EPD_CANVAS);
    return true;
}

static bool slot_read(int n)
{
    if (n < 0 || n >= SLOTS) return false;
    const esp_partition_t *p = maps_part();
    if (p) {
        slot_hdr_t h;
        size_t off = (size_t)n * SLOT_SPAN;
        if (esp_partition_read(p, off, &h, sizeof h) != ESP_OK) return false;
        if (h.magic != SLOT_MAGIC || h.w != EPD_W || h.h != EPD_H || h.len != (uint32_t)EPD_CANVAS) return false;
        return esp_partition_read(p, off + sizeof h, epd_canvas(), EPD_CANVAS) == ESP_OK;
    }
    char k[16]; slot_key(n, k);
    return kv_get_blob(k, epd_canvas(), EPD_CANVAS);    /* fails on a size mismatch */
}

static void w_save(void)
{
    int n = POP();
    if (n < 0 || n >= SLOTS) { con_printf("? slot 0..%d\n", SLOTS - 1); return; }
    if (!slot_write(n)) { con_printf("epd: could not save map %d\n", n); return; }
    kv_set_i32("mapslot", n);
    s_last_slot = n;
    con_printf("epd: saved map %d\n", n);
}

static void w_load(void) { int n = POP(); forth_push(slot_read(n) ? -1 : 0); }

static void w_slot_fetch(void) { forth_push(s_last_slot); }
int skin_last_slot(void) { return s_last_slot; }

/* ---- splash --------------------------------------------------------- */

static int s_safe_x = 0, s_safe_w = 0;   /* columns that render on this glass; w 0 = whole panel */

void skin_splash(void)
{
    int x0 = s_safe_x, w = s_safe_w;
    epd_clear();
    epd_frame(x0, 0, w, EPD_H);
    epd_frame(x0 + 2, 2, w - 4, EPD_H - 4);
    /* the switchback mark: two strokes and a dot, top left */
    for (int t = 0; t < 4; t++) {
        epd_line(x0 + 12 + t, 70, x0 + 46 + t, 46);
        epd_line(x0 + 46 + t, 46, x0 + 18 + t, 32);
        epd_line(x0 + 18 + t, 32, x0 + 44 + t, 16);
    }
    epd_box(x0 + 42, 42, 8, 8, 1);
    int big = (w >= 200) ? 3 : 2;
    epd_str(x0 + 60, 16, "maplet", 6, big);
    epd_str(x0 + 60, 16 + 8 * big + 6, "OpenWR", 6, 1);
    epd_str(x0 + 12, 86, "pair from Switchback", 20, 1);
}
static void w_splash(void) { skin_splash(); }
int skin_safe_x(void); int skin_safe_w(void);
static void w_test(void)   { epd_test_pattern_in(skin_safe_x(), skin_safe_w()); }
static void w_geom(void)   { epd_knob(0, POP()); }
static void w_cdi(void)    { epd_knob(1, POP()); }
static void w_old(void)    { epd_knob(2, POP()); }
static void w_lut(void)    { epd_knob(3, POP()); }
static void w_spi(void)    { epd_knob(4, POP()); }
static void w_csb(void)    { epd_knob(5, POP()); }
static void w_fill(void)   { epd_fill(POP()); }

/* Safe area: the columns of the glass that still render (this bench panel
 * has dead gate zones at both ends). Stored in NVS; the splash, the test
 * pattern and the phone's renderer keep inside it. */
static void safe_load(void)
{
    int32_t x, w;
    if (kv_get_i32("safex", &x) && kv_get_i32("safew", &w) && x >= 0 && w >= 0) { s_safe_x = x; s_safe_w = w; }
}
static void w_safe_store(void)
{
    int w = POP(), x = POP();
    if (x < 0 || w <= 8 || x + w > EPD_W) { con_printf("epd-safe!: need 0 <= x, x+w <= %d\n", EPD_W); return; }
    s_safe_x = x; s_safe_w = w;
    kv_set_i32("safex", x); kv_set_i32("safew", w);
    con_printf("epd: safe area x=%d w=%d\n", x, w);
}
int skin_safe_x(void) { return (s_safe_w > 0 && s_safe_x + s_safe_w <= EPD_W) ? s_safe_x : 0; }
int skin_safe_w(void) { return (s_safe_w > 0 && s_safe_x + s_safe_w <= EPD_W) ? s_safe_w : EPD_W; }
static void w_safe_fetch(void) { forth_push(skin_safe_x()); forth_push(skin_safe_w()); }

/* ---- panel drivers ---------------------------------------------------- */
/* epd-driver! picks the panel by name, remembers it, resets the safe area
 * to the whole new panel, and leaves a blank canvas: follow it with
 * epd-test epd-show. Saved maps drawn for another size are not shown. */

static void w_drivers(void)
{
    for (int i = 0; i < epd_driver_count(); i++)
        con_printf("%c %-16s %s\n", i == epd_driver_current() ? '*' : ' ', epd_driver_name(i), epd_driver_desc(i));
}

static void w_driver_store(void)
{
    int n = POP(); const char *name = (const char *)forth_pop();
    int i = name ? epd_driver_find(name, n) : -1;
    if (i < 0) { con_printf("epd-driver!: unknown; epd-drivers lists them\n"); return; }
    epd_driver_set(i);
    kv_set_str("epddrv", epd_driver_name(i));
    s_safe_x = 0; s_safe_w = 0;
    kv_set_i32("safex", 0); kv_set_i32("safew", 0);
    con_printf("epd: driver %s, %dx%d; safe area reset to the whole panel\n", epd_driver_name(i), EPD_W, EPD_H);
}

static void w_driver_print(void) { con_printf("%s\n", epd_driver_name(epd_driver_current())); }
static void w_size_fetch(void)   { forth_push(EPD_W); forth_push(EPD_H); }
static void w_rot_store(void)
{
    int r = POP() & 3;
    epd_set_rot(r);
    kv_set_i32("epdrot", r);
    con_printf("epd: rotation %d\n", r);
}

static void driver_load(void)
{
    char name[24] = "";
    if (kv_get_str("epddrv", name, sizeof name) && name[0]) {
        int i = epd_driver_find(name, (int)strlen(name));
        if (i >= 0) epd_driver_set(i);
    }
    int32_t r = 0;
    if (kv_get_i32("epdrot", &r)) epd_set_rot((int)r);
    ESP_LOGI(TAG, "panel %s %dx%d rot %d", epd_driver_name(epd_driver_current()), EPD_W, EPD_H, epd_rot());
}

/* ---- board ---------------------------------------------------------- */

static void w_ms(void)     { int n = POP(); if (n > 0) vTaskDelay(pdMS_TO_TICKS(n)); }
static void w_us(void)     { forth_push((intptr_t)(uint32_t)esp_timer_get_time()); }
static void w_random(void) { int n = POP(); forth_push(n > 0 ? (intptr_t)(esp_random() % (uint32_t)n) : 0); }
static void w_reboot(void) { con_printf("rebooting\n"); vTaskDelay(pdMS_TO_TICKS(100)); esp_restart(); }
static void w_button(void) { forth_push(gpio_get_level(SKIN_PIN_BOOT) == 0 ? -1 : 0); }
static void w_gpio_fetch(void)
{
    int n = POP();
    if (n < 0 || n > SKIN_GPIO_MAX || n == SKIN_USB_PIN_A || n == SKIN_USB_PIN_B || n == SKIN_PIN_SCK || n == SKIN_PIN_MOSI
        || n == SKIN_PIN_CS || n == SKIN_PIN_DC || n == SKIN_PIN_RST) { forth_push(-1); return; }
    gpio_config_t io = { .pin_bit_mask = 1ULL << n, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE };
    gpio_config(&io);
    forth_push(gpio_get_level(n));
}

/* ---- init --------------------------------------------------------- */

void skin_init(void)
{
    gpio_config_t btn = { .pin_bit_mask = 1ULL << SKIN_PIN_BOOT, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE };
    gpio_config(&btn);
    driver_load();
    safe_load();
    int32_t slot = -1;
    if (kv_get_i32("mapslot", &slot)) s_last_slot = (int)slot;
    ESP_LOGI(TAG, "last map slot %d", s_last_slot);
}

/* The boot picture: the last saved map, else the splash. */
bool skin_show_boot(void)
{
    if (s_last_slot >= 0 && slot_read(s_last_slot)) { epd_show(); return true; }
    skin_splash();
    epd_show();
    return false;
}

void skin_register_words(void)
{
    static const struct { const char *n; forth_word_fn f; } w[] = {
        { "epd-clear", w_clear }, { "epd-cursor!", w_cursor_store }, { "epd-cursor@", w_cursor_fetch },
        { "epd-hex!", w_hex_store }, { "epd-pixel", w_pixel }, { "epd-line", w_line },
        { "epd-box", w_box }, { "epd-frame", w_frame }, { "epd-str", w_str }, { "epd-scale", w_scale },
        { "epd-show", w_show }, { "epd-save", w_save }, { "epd-load", w_load }, { "epd-slot@", w_slot_fetch },
        { "epd-splash", w_splash }, { "epd-test", w_test },
        { "epd-geom!", w_geom }, { "epd-cdi!", w_cdi }, { "epd-old!", w_old }, { "epd-lut!", w_lut }, { "epd-spi!", w_spi }, { "epd-csb!", w_csb }, { "epd-fill", w_fill }, { "epd-safe!", w_safe_store }, { "epd-safe@", w_safe_fetch },
        { "epd-drivers", w_drivers }, { "epd-driver!", w_driver_store }, { "epd-driver", w_driver_print }, { "epd-size@", w_size_fetch }, { "epd-rot!", w_rot_store },
        { "ms", w_ms }, { "us", w_us }, { "random", w_random }, { "reboot", w_reboot },
        { "button?", w_button }, { "gpio@", w_gpio_fetch },
    };
    for (size_t i = 0; i < sizeof w / sizeof w[0]; i++) forth_register_word(w[i].n, w[i].f);
}
