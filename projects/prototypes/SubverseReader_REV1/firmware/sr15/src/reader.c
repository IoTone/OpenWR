/*
 * reader.c: the SR-15 reading loop. Two screens:
 *
 *   shelf  the packs in /sd/packs (or the card's root): dial right moves
 *          down the list, left up; push opens. EXIT shows the badge screen.
 *   page   dial right = next page, left = previous, push = next page,
 *          MENU = page number and title, EXIT = back to the shelf.
 *          Hold MENU: go to the first page. Hold EXIT: sleep now.
 *   any    hold MENU + EXIT: open a 3-minute Bluetooth sync window. A banner
 *          at the top shows SYNC while the phone may connect, PHONE while
 *          it is connected. The pass never sleeps while either is up.
 *
 * Page turns are partial refreshes; every Nth turn (and every shelf change
 * back to a page) is a full refresh to clear ghosting.
 *
 * After sleep_s seconds with no key and no console input the pass deep-
 * sleeps with the page still on the glass. Any control wakes it, and the key
 * that woke it is acted on, so a page turn from sleep is one press. Where we
 * were lives in RTC memory across deep sleep and in NVS across power-off.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "nvs.h"

#include "pins.h"
#include "epd.h"
#include "keys.h"
#include "sd.h"
#include "pack.h"
#include "reader.h"
#include "ble_console.h"

static const char *TAG = "reader";
const int g_canvas_w = EPD_CW, g_canvas_h = EPD_CH;

#define MAX_PACKS 40
#define SHELF_ROWS 13

typedef enum { SCR_SHELF = 0, SCR_PAGE, SCR_BADGE, SCR_TEST } screen_t;

/* Survives deep sleep (not power-off). */
static RTC_DATA_ATTR struct {
    uint32_t magic;
    screen_t screen;
    char path[96];
    uint32_t page;
    int cursor;
    int turns;          /* partial refreshes since the last full one */
    bool info;          /* the MENU overlay is up */
} s_rtc;
#define RTC_MAGIC 0x53523135   /* "SR15" */

static char s_paths[MAX_PACKS][96];
static int s_npacks;
static pack_t s_pack;
static bool s_pack_ok;
static SemaphoreHandle_t s_lock;
static volatile int64_t s_last_activity;
static int s_sleep_s = 120;
static int s_full_every = 8;
static char s_msg[96];
static int s_ble_shown;     /* the ble state the banner shows: 0 none, 1 sync, 2 phone */

void reader_activity(void) { s_last_activity = esp_timer_get_time(); }
int  reader_sleep_s(void) { return s_sleep_s; }
int  reader_full_every(void) { return s_full_every; }

static void nvs_put_i32(const char *k, int32_t v)
{
    nvs_handle_t h;
    if (nvs_open("reader", NVS_READWRITE, &h) == ESP_OK) { nvs_set_i32(h, k, v); nvs_commit(h); nvs_close(h); }
}

static void save_place(void)
{
    nvs_handle_t h;
    if (nvs_open("reader", NVS_READWRITE, &h) != ESP_OK) return;
    nvs_set_str(h, "path", s_rtc.path);
    nvs_set_u32(h, "page", s_rtc.page);
    nvs_set_i32(h, "screen", s_rtc.screen == SCR_PAGE ? SCR_PAGE : SCR_SHELF);
    nvs_commit(h);
    nvs_close(h);
}

static void load_settings(bool cold)
{
    nvs_handle_t h;
    if (nvs_open("reader", NVS_READONLY, &h) != ESP_OK) return;
    int32_t v;
    if (nvs_get_i32(h, "sleep", &v) == ESP_OK) s_sleep_s = v;
    if (nvs_get_i32(h, "full", &v) == ESP_OK && v > 0) s_full_every = v;
    if (cold) {
        size_t n = sizeof s_rtc.path;
        uint32_t pg;
        if (nvs_get_str(h, "path", s_rtc.path, &n) == ESP_OK && nvs_get_u32(h, "page", &pg) == ESP_OK) {
            s_rtc.page = pg;
            if (nvs_get_i32(h, "screen", &v) == ESP_OK) s_rtc.screen = v;
        }
    }
    nvs_close(h);
}

/* ---------------------------------------------------------------- drawing */

static void trunc_copy(char *dst, const char *src, int maxch)
{
    int n = (int)strlen(src);
    if (n <= maxch) { strcpy(dst, src); return; }
    memcpy(dst, src, maxch - 2); dst[maxch - 2] = '.'; dst[maxch - 1] = '.'; dst[maxch] = 0;
}

static void draw_shelf(void)
{
    epd_clear();
    epd_box(0, 0, EPD_CW, 30, 1);
    /* the band is black; write the name in white by clearing pixels */
    {
        uint8_t *c = epd_canvas();
        static uint8_t tmp[EPD_CLEN];
        memcpy(tmp, c, EPD_CLEN);
        memset(c, 0, EPD_CLEN);
        epd_str(10, 8, "SUBVERSE  ALL ACCESS", 2);
        for (int i = 0; i < EPD_CLEN; i++) c[i] = tmp[i] & ~c[i];
    }
    if (!sd_ok()) {
        epd_str(10, 60, "No microSD card.", 2);
        epd_str(10, 90, "Put .svpk packs in /packs", 1);
        epd_str(10, 102, "on a FAT32 card and restart.", 1);
    } else if (s_npacks == 0) {
        epd_str(10, 60, "No packs on the card.", 2);
        epd_str(10, 90, "Build them with tools/svpk.py", 1);
        epd_str(10, 102, "and copy them to /packs.", 1);
    } else {
        int top = s_rtc.cursor - s_rtc.cursor % SHELF_ROWS;
        for (int r = 0; r < SHELF_ROWS && top + r < s_npacks; r++) {
            int i = top + r, y = 40 + r * 26;
            pack_t p;
            char err[40], line[40];
            if (pack_open(&p, s_paths[i], err, sizeof err)) {
                trunc_copy(line, p.title, 22);
            } else {
                const char *b = strrchr(s_paths[i], '/');
                snprintf(line, sizeof line, "%.14s (%.20s)", b ? b + 1 : s_paths[i], err);
            }
            if (i == s_rtc.cursor) epd_frame(4, y - 4, EPD_CW - 8, 24);
            epd_str(12, y + 2, line, 2);
            if (pack_open(&p, s_paths[i], err, sizeof err)) {
                char pp[12];
                snprintf(pp, sizeof pp, "%lu", (unsigned long)p.pages);
                epd_str(EPD_CW - 12 - epd_str_w(pp, 1), y + 6, pp, 1);
            }
        }
        if (s_npacks > SHELF_ROWS) {
            char m[24];
            snprintf(m, sizeof m, "%d/%d", s_rtc.cursor + 1, s_npacks);
            epd_str(EPD_CW - 10 - epd_str_w(m, 1), EPD_CH - 30, m, 1);
        }
    }
    if (s_msg[0]) epd_str(10, EPD_CH - 30, s_msg, 1);
    epd_box(0, EPD_CH - 16, EPD_CW, 1, 1);
    epd_str(6, EPD_CH - 11, "dial: choose  push: open  EXIT: badge", 1);
}

static bool draw_page(void)
{
    if (!s_pack_ok) return false;
    if (!pack_page(&s_pack, s_rtc.page, epd_canvas(), EPD_CLEN)) {
        epd_clear();
        epd_str(10, 60, "Can't read this page.", 2);
        return false;
    }
    if (s_rtc.info) {
        char a[40], b[24];
        trunc_copy(a, s_pack.title, 39);
        snprintf(b, sizeof b, "page %lu of %lu", (unsigned long)s_rtc.page + 1, (unsigned long)s_pack.pages);
        epd_box(0, EPD_CH - 40, EPD_CW, 40, 0);
        epd_box(0, EPD_CH - 40, EPD_CW, 2, 1);
        epd_str(8, EPD_CH - 32, a, 1);
        epd_str(8, EPD_CH - 18, b, 2);
    }
    return true;
}

static void draw_badge(void)
{
    epd_clear();
    epd_box(0, 0, EPD_CW, EPD_CH, 1);
    uint8_t *c = epd_canvas();
    static uint8_t tmp[EPD_CLEN];
    memcpy(tmp, c, EPD_CLEN);
    memset(c, 0, EPD_CLEN);
    epd_str((EPD_CW - epd_str_w("ALL", 6)) / 2, 90, "ALL", 6);
    epd_str((EPD_CW - epd_str_w("ACCESS", 6)) / 2, 150, "ACCESS", 6);
    epd_str((EPD_CW - epd_str_w("SUBVERSE", 3)) / 2, 260, "SUBVERSE", 3);
    for (int i = 0; i < EPD_CLEN; i++) c[i] = tmp[i] & ~c[i];
}

/* Orientation check: the top says TOP, an arrow points at the dial. */
static void draw_test(void)
{
    char r[24];
    epd_clear();
    epd_frame(0, 0, EPD_CW, EPD_CH);
    epd_frame(3, 3, EPD_CW - 6, EPD_CH - 6);
    epd_str((EPD_CW - epd_str_w("TOP", 4)) / 2, 20, "TOP", 4);
    epd_str(12, EPD_CH / 2 - 10, "L", 3);
    epd_str(EPD_CW - 30, EPD_CH / 2 - 10, "R", 3);
    snprintf(r, sizeof r, "epd-rot = %d", epd_rot());
    epd_str((EPD_CW - epd_str_w(r, 2)) / 2, 150, r, 2);
    epd_str(20, 190, "Upright, dial at the", 2);
    epd_str(20, 210, "bottom? Then this one.", 2);
    epd_str(20, 240, "If not: n epd-rot!", 2);
    epd_str(20, 260, "with n = 0, 1, 2 or 3", 2);
    for (int i = 0; i < 30; i++) epd_box(EPD_CW / 2 - i, EPD_CH - 40 - 30 + i, 2 * i + 1, 1, 1);
    epd_str((EPD_CW - epd_str_w("DIAL", 2)) / 2, EPD_CH - 30, "DIAL", 2);
}

static void draw_ble_banner(void)
{
    if (!s_ble_shown) return;
    const char *t = s_ble_shown == 2 ? "PHONE CONNECTED" : "SYNC: open the app";
    int w = epd_str_w(t, 2) + 16;
    int x = (EPD_CW - w) / 2;
    epd_box(x, 36, w, 26, 1);
    uint8_t *c = epd_canvas();
    static uint8_t tmp[EPD_CLEN];
    memcpy(tmp, c, EPD_CLEN);
    memset(c, 0, EPD_CLEN);
    epd_str(x + 8, 42, t, 2);
    for (int i = 0; i < EPD_CLEN; i++) c[i] = tmp[i] & ~c[i];
}

static void redraw(void)
{
    switch (s_rtc.screen) {
    case SCR_SHELF: draw_shelf(); break;
    case SCR_PAGE:  if (!draw_page()) { s_rtc.screen = SCR_SHELF; draw_shelf(); } break;
    case SCR_BADGE: draw_badge(); break;
    case SCR_TEST:  draw_test(); break;
    }
    draw_ble_banner();
}

static void show(bool full)
{
    redraw();
    if (!full && ++s_rtc.turns >= s_full_every) full = true;
    if (full) s_rtc.turns = 0;
    epd_show(full);
}

/* ---------------------------------------------------------------- actions */

static void rescan(void)
{
    if (!sd_ok()) sd_mount();
    s_npacks = sd_ok() ? pack_scan("/sd/packs", s_paths, MAX_PACKS) : 0;
    if (sd_ok() && s_npacks == 0) s_npacks = pack_scan("/sd", s_paths, MAX_PACKS);
    if (s_rtc.cursor >= s_npacks) s_rtc.cursor = s_npacks ? s_npacks - 1 : 0;
}

static bool open_path(const char *path)
{
    char err[48];
    s_pack_ok = pack_open(&s_pack, path, err, sizeof err);
    if (!s_pack_ok) { snprintf(s_msg, sizeof s_msg, "%.40s", err); return false; }
    snprintf(s_rtc.path, sizeof s_rtc.path, "%s", path);
    return true;
}

static void go_shelf(void)
{
    rescan();
    for (int i = 0; i < s_npacks; i++) if (!strcmp(s_paths[i], s_rtc.path)) s_rtc.cursor = i;
    s_rtc.screen = SCR_SHELF;
    s_rtc.info = false;
    show(true);
}

static void open_cursor(void)
{
    if (s_rtc.cursor >= s_npacks) return;
    bool same = !strcmp(s_paths[s_rtc.cursor], s_rtc.path);
    if (!open_path(s_paths[s_rtc.cursor])) { show(false); return; }
    if (!same) s_rtc.page = 0;
    if (s_rtc.page >= s_pack.pages) s_rtc.page = 0;
    s_rtc.screen = SCR_PAGE;
    s_rtc.info = false;
    s_msg[0] = 0;
    show(true);
    save_place();
}

static void turn(int d)
{
    int64_t p = (int64_t)s_rtc.page + d;
    if (p < 0 || p >= (int64_t)s_pack.pages) return;   /* stays put at either end */
    s_rtc.page = (uint32_t)p;
    s_rtc.info = false;
    show(false);
    save_place();
}

static void handle(int ev)
{
    int k = ev & 0x7F;
    bool lng = ev & KEY_LONG;
    ESP_LOGI(TAG, "key %s%s", keys_name(k), lng ? " (long)" : "");
    uint32_t both = (1u << KEY_MENU) | (1u << KEY_EXIT);
    if (lng && (k == KEY_MENU || k == KEY_EXIT) && (keys_down() & both) == both) {
        ble_pair_window(180);
        s_ble_shown = ble_console_state();
        show(false);
        return;
    }
    if (lng && k == KEY_EXIT) { reader_sleep_now(); return; }
    switch (s_rtc.screen) {
    case SCR_SHELF:
        if (k == KEY_UP && s_rtc.cursor > 0) { s_rtc.cursor--; show(false); }
        else if (k == KEY_DOWN && s_rtc.cursor + 1 < s_npacks) { s_rtc.cursor++; show(false); }
        else if (k == KEY_OK) open_cursor();
        else if (k == KEY_EXIT) { s_rtc.screen = SCR_BADGE; show(true); }
        else if (k == KEY_MENU) go_shelf();             /* rescan the card */
        break;
    case SCR_PAGE:
        if (k == KEY_DOWN || k == KEY_OK) turn(+1);
        else if (k == KEY_UP) turn(-1);
        else if (k == KEY_MENU && lng) { s_rtc.page = 0; s_rtc.info = false; show(true); save_place(); }
        else if (k == KEY_MENU) { s_rtc.info = !s_rtc.info; show(false); }
        else if (k == KEY_EXIT) go_shelf();
        break;
    case SCR_BADGE:
    case SCR_TEST:
        go_shelf();
        break;
    }
}

/* ---------------------------------------------------------------- sleep */

void reader_sleep_now(void)
{
    ESP_LOGI(TAG, "sleeping; any control wakes the pass");
    save_place();
    epd_sleep();
    epd_power(false);
    sd_unmount();
    gpio_set_level(PIN_LED, 0);
    uint64_t m = keys_wake_mask();
    for (int pin = 0; pin < 64; pin++) {
        if (!(m & (1ULL << pin))) continue;
        rtc_gpio_pullup_en(pin);
        rtc_gpio_pulldown_dis(pin);
    }
    esp_sleep_enable_ext1_wakeup_io(m, ESP_EXT1_WAKEUP_ANY_LOW);
    vTaskDelay(pdMS_TO_TICKS(50));
    esp_deep_sleep_start();
}

/* ---------------------------------------------------------------- task */

static void reader_task(void *arg)
{
    bool woke = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1;
    if (!woke || s_rtc.magic != RTC_MAGIC) {
        memset(&s_rtc, 0, sizeof s_rtc);
        s_rtc.magic = RTC_MAGIC;
        load_settings(true);
        woke = false;
    } else {
        load_settings(false);
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    rescan();
    if (s_rtc.screen == SCR_PAGE && !open_path(s_rtc.path)) s_rtc.screen = SCR_SHELF;
    if (woke) {
        /* The glass still shows what we drew before sleeping: tell the
         * controller, so the first turn can be a quick partial refresh. */
        redraw();
        epd_set_previous(epd_canvas());
        int k = keys_from_wake_mask(esp_sleep_get_ext1_wakeup_status());
        if (k) keys_inject(k);
    } else {
        show(true);
    }
    xSemaphoreGive(s_lock);
    reader_activity();
    for (;;) {
        int ev = keys_get(500);
        if (ev) {
            reader_activity();
            xSemaphoreTake(s_lock, portMAX_DELAY);
            handle(ev);
            xSemaphoreGive(s_lock);
        }
        int bs = ble_console_state();
        if (bs != s_ble_shown) {
            xSemaphoreTake(s_lock, portMAX_DELAY);
            bool was_connected = s_ble_shown == 2;
            s_ble_shown = bs;
            if (!(was_connected && bs == 1)) show(false);   /* not between a drop and a quick reconnect */
            if (bs == 0 && s_rtc.screen == SCR_SHELF) go_shelf();   /* packs may have arrived */
            xSemaphoreGive(s_lock);
        }
        if (bs) reader_activity();
        if (s_sleep_s > 0 && keys_down() == 0 &&
            esp_timer_get_time() - s_last_activity > (int64_t)s_sleep_s * 1000000) {
            xSemaphoreTake(s_lock, portMAX_DELAY);
            reader_sleep_now();
        }
    }
}

void reader_start(void)
{
    s_lock = xSemaphoreCreateMutex();
    xTaskCreate(reader_task, "reader", 8192, NULL, 4, NULL);
}

/* ---------------------------------------------------------------- console */

void reader_lock(void)   { xSemaphoreTake(s_lock, portMAX_DELAY); reader_activity(); }
void reader_unlock(void) { xSemaphoreGive(s_lock); }

void reader_cmd_shelf(void) { go_shelf(); }
void reader_cmd_open(int i) { if (i >= 0 && i < s_npacks) { s_rtc.cursor = i; open_cursor(); } }
void reader_cmd_page(int n) { if (s_rtc.screen == SCR_PAGE) turn(n - 1 - (int)s_rtc.page); }
void reader_cmd_turn(int d) { if (s_rtc.screen == SCR_PAGE) turn(d); }
void reader_cmd_test(void)  { s_rtc.screen = SCR_TEST; show(true); }
void reader_cmd_badge(void) { s_rtc.screen = SCR_BADGE; show(true); }
void reader_cmd_redraw(bool full) { show(full); }
void reader_set_sleep(int s) { s_sleep_s = s < 0 ? 0 : s; nvs_put_i32("sleep", s_sleep_s); }
void reader_set_full_every(int n) { if (n > 0) { s_full_every = n; nvs_put_i32("full", n); } }

bool pack_path(int i, char *out, int len)
{
    if (i < 0 || i >= s_npacks) return false;
    snprintf(out, len, "%s", s_paths[i]);
    return true;
}

void reader_info(void (*out)(const char *fmt, ...))
{
    static const char *scr[] = { "shelf", "page", "badge", "test" };
    out("screen %s, card %s, %d packs, panel %s, rot %d, sleep after %d s, full refresh every %d\n",
        scr[s_rtc.screen], sd_ok() ? "mounted" : "none", s_npacks, epd_ctl_name(), epd_rot(), s_sleep_s, s_full_every);
    if (s_pack_ok)
        out("open: %s  '%s' by %s, page %lu of %lu\n", s_pack.path, s_pack.title, s_pack.author,
            (unsigned long)s_rtc.page + 1, (unsigned long)s_pack.pages);
    for (int i = 0; i < s_npacks; i++) out("  %2d  %s\n", i, s_paths[i]);
}
