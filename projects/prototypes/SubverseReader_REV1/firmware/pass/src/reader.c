/*
 * reader.c: the reading loop. Screens:
 *
 *   shelf  the packs in the store's /packs folder
 *   page   one page of the open pack
 *   badge  ALL ACCESS, and a test screen for sr-test
 *
 * Controls depend on what the pass has (keys_fitted()):
 *
 *   5, SR-15  shelf: dial right/left moves, push opens, EXIT badge, MENU rescans.
 *             page: dial right or push = next, left = previous, MENU = page
 *             number and title, EXIT = shelf. Hold MENU: first page. Hold
 *             EXIT: sleep now. Hold MENU + EXIT: Bluetooth sync window.
 *   3, SR-16  shelf: PREV/NEXT move, MENU opens, hold MENU badge.
 *             page: NEXT/PREV turn, MENU = page number, hold MENU = shelf,
 *             hold PREV = first page. Hold PREV + NEXT: sync window.
 *   0, SR-16N no keys. The phone app turns pages and picks the zine, and the
 *             XIAO's BOOT button is one key: page: press = next page (round
 *             to the first after the last), hold = next zine. shelf: press =
 *             move down, hold = open. Every hold also opens a sync window.
 *
 * Auto-turn (sr-auto!): on a page, turn every N seconds, round and round:
 * a slideshow, or a setlist that advances by itself.
 *
 * Bluetooth. The sync window says SYNC (window open) or PHONE CONNECTED for
 * 7 s at the top, then shrinks to a corner icon: outlined while the window
 * is open, solid while a phone is connected. The pass never sleeps while
 * either is up. A pass with no keys can't be asked to open a window, so it
 * checks in by itself (sr-sync!): every N seconds of sleep it wakes for
 * SYNC_WINDOW_S seconds and advertises without touching the glass; a phone
 * that is waiting for it connects then.
 *
 * Page turns are quick refreshes; every Nth turn (and every shelf change
 * back to a page) is a full refresh to clear ghosting.
 *
 * After sleep_s seconds with no key and no console input the pass deep-
 * sleeps with the page still on the glass. Any control that can wakes it,
 * and the key that woke it is acted on, so a page turn from sleep is one
 * press. Where we were lives in RTC memory across deep sleep and in NVS
 * across power-off.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#include "board.h"
#include "epd.h"
#include "keys.h"
#include "store.h"
#include "pack.h"
#include "reader.h"
#include "ble_console.h"
#include "battery.h"

static const char *TAG = "reader";
const int g_canvas_w = EPD_CW, g_canvas_h = EPD_CH;

#define MAX_PACKS 100
#define SHELF_ROWS ((EPD_CH - 62) / 26)
#define SHELF_CHARS ((EPD_CW - 36) / 12)       /* title letters a shelf row holds at scale 2 */
#define SYNC_WINDOW_S 5                        /* how long a timed check-in advertises */

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
    int32_t auto_left;  /* seconds to the next auto-turn when the timer wakes us; -1: auto-turn off */
} s_rtc;
#define RTC_MAGIC 0x50415332   /* "PAS2" */

static char s_paths[MAX_PACKS][96];
static int s_npacks;
static pack_t s_pack;
static bool s_pack_ok;
static SemaphoreHandle_t s_lock;
static volatile int64_t s_last_activity;
static int s_sleep_s = 120;
static int s_full_every = 8;
static int s_auto_s;                 /* auto-turn: seconds a page, 0 = off */
static int s_sync_s = BOARD_KEYS_DEFAULT ? 0 : 300;   /* timed Bluetooth check-in while asleep, 0 = never */
static int s_keys = BOARD_KEYS_DEFAULT;
static int64_t s_auto_due;
static int64_t s_info_until;        /* a timed "where am I" bar: taken down when this passes */
static bool s_quiet;                 /* a timed check-in is advertising: say nothing on the glass */
static volatile bool s_brief;        /* woken by the timer: back to sleep as soon as the check-in is over */
static char s_msg[96];
static int s_ble_shown;     /* the ble state on the glass: 0 none, 1 sync window open, 2 phone connected */
static int64_t s_banner_until;   /* the words show until then; after that only a corner icon */
static bool s_banner_up;         /* the glass shows the words right now */
#define BANNER_US (7 * 1000000LL)
static void draw_battery(int x, int y);
#define line epd_line

void reader_activity(void) { s_last_activity = esp_timer_get_time(); s_brief = false; }
int  reader_sleep_s(void) { return s_sleep_s; }
int  reader_full_every(void) { return s_full_every; }
int  reader_sync_s(void) { return s_sync_s; }
int  reader_keys(void) { return s_keys; }

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
    if (nvs_get_i32(h, "auto", &v) == ESP_OK && v >= 0) s_auto_s = v;
    if (nvs_get_i32(h, "sync", &v) == ESP_OK && v >= 0) s_sync_s = v;
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

/* White letters on a black band. */
static void band(int y, int h, int x, int ty, const char *s, int scale)
{
    epd_box(0, y, EPD_CW, h, 1);
    epd_str_inv(x, ty, s, scale);
}

static const char *hint_text(void)
{
    if (s_keys >= 5) return "dial: choose  push: open  EXIT: badge";
    if (s_keys == 3) return "< > choose   MENU: open";
    return "BOOT: next   hold: open";
}

static void draw_shelf(void)
{
    epd_clear();
    band(0, 30, 10, 8, EPD_CW >= 300 ? "SUBVERSE  ALL ACCESS" : "SUBVERSE READER", 2);
    if (!store_ok()) {
#if BOARD_STORE_SD
        epd_str(10, 60, "No microSD card.", 2);
        epd_str(10, 90, "Put .svpk packs in /packs", 1);
        epd_str(10, 102, "on a FAT32 card and restart.", 1);
#else
        epd_str(10, 60, "Storage error.", 2);
        epd_str(10, 90, "The flash store didn't mount.", 1);
#endif
    } else if (s_npacks == 0) {
#if BOARD_STORE_SD
        epd_str(10, 60, "No packs on the card.", 2);
        epd_str(10, 90, "Build them with tools/svpk.py", 1);
        epd_str(10, 102, "and copy them to /packs.", 1);
#else
        epd_str(10, 60, "No zines yet.", 2);
        epd_str(10, 90, "Send some from the phone app:", 1);
        epd_str(10, 102, "open it and pick this pass.", 1);
#endif
    } else {
        int top = s_rtc.cursor - s_rtc.cursor % SHELF_ROWS;
        for (int r = 0; r < SHELF_ROWS && top + r < s_npacks; r++) {
            int i = top + r, y = 40 + r * 26;
            pack_t p;
            char err[40], line[64];
            bool ok = pack_open(&p, s_paths[i], err, sizeof err);
            if (ok) {
                trunc_copy(line, p.title, SHELF_CHARS);
            } else {
                const char *b = strrchr(s_paths[i], '/');
                snprintf(line, sizeof line, "%.14s (%.20s)", b ? b + 1 : s_paths[i], err);
                line[SHELF_CHARS] = 0;
            }
            if (i == s_rtc.cursor) epd_frame(4, y - 4, EPD_CW - 8, 24);
            epd_str(12, y + 2, line, 2);
            if (ok) {
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
    epd_str(6, EPD_CH - 11, hint_text(), 1);
    draw_battery(EPD_CW - 28, EPD_CH - 13);
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
        trunc_copy(a, s_pack.title, (EPD_CW - 16) / 6 < 39 ? (EPD_CW - 16) / 6 : 39);
        snprintf(b, sizeof b, "page %lu of %lu", (unsigned long)s_rtc.page + 1, (unsigned long)s_pack.pages);
        epd_box(0, EPD_CH - 40, EPD_CW, 40, 0);
        epd_box(0, EPD_CH - 40, EPD_CW, 2, 1);
        epd_str(8, EPD_CH - 32, a, 1);
        draw_battery(EPD_CW - 30, EPD_CH - 22);
        epd_str(8, EPD_CH - 18, b, 2);
    }
    return true;
}

static void draw_badge(void)
{
    epd_clear();
    epd_box(0, 0, EPD_CW, EPD_CH, 1);
    epd_str_inv((EPD_CW - epd_str_w("ALL", 6)) / 2, 90, "ALL", 6);
    epd_str_inv((EPD_CW - epd_str_w("ACCESS", 6)) / 2, 150, "ACCESS", 6);
    epd_str_inv((EPD_CW - epd_str_w("SUBVERSE", 3)) / 2, 260, "SUBVERSE", 3);
}

/* Orientation check: the top says TOP, an arrow points at the bottom edge
 * (the dial on SR-15). */
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
    epd_str((EPD_CW - epd_str_w(r, 2)) / 2, 100, r, 2);
#if defined(BOARD_SR15)
    static const char *msg[] = { "Upright, dial at the", "bottom? Then this one.", "If not: n epd-rot!", "with n = 0, 1, 2 or 3" };
    const char *foot = "DIAL";
#else
    static const char *msg[] = { "Reads upright,", "not mirrored?", "Then this one.", "If not: n epd-rot!", "n = 0, 1, 2 or 3", "(2 = upside down)" };
    const char *foot = "BOTTOM";
#endif
    for (unsigned i = 0; i < sizeof msg / sizeof msg[0]; i++)
        epd_str((EPD_CW - epd_str_w(msg[i], 2)) / 2, 226 + i * 18, msg[i], 2);
    for (int i = 0; i < 30; i++) epd_box(EPD_CW / 2 - i, EPD_CH - 40 - 30 + i, 2 * i + 1, 1, 1);
    epd_str((EPD_CW - epd_str_w(foot, 2)) / 2, EPD_CH - 30, foot, 2);
}

/* The Bluetooth rune in a 24 px square, top right. Solid: a phone is
 * connected. Outlined: the sync window is open, nobody connected yet. */
static void draw_ble_icon(void)
{
    const int s = 24, x = EPD_CW - s - 4, y = 4, solid = s_ble_shown == 2;
    epd_box(x - 2, y - 2, s + 4, s + 4, 0);           /* white margin so it reads over a page */
    if (solid) epd_box(x, y, s, s, 1);
    else { epd_frame(x, y, s, s); epd_frame(x + 1, y + 1, s - 2, s - 2); }
    int ink = solid ? 0 : 1, cx = x + 12, t = y + 4, b = y + 20;
    line(cx, t, cx, b, ink);                          /* the stem */
    line(cx, t, cx + 5, t + 4, ink);                  /* upper arrow */
    line(cx + 5, t + 4, cx - 5, t + 12, ink);
    line(cx, b, cx + 5, b - 4, ink);                  /* lower arrow */
    line(cx + 5, b - 4, cx - 5, t + 4, ink);
}

/* A small battery at (x, y), 22 x 11: fill by charge, a bolt while charging,
 * a question mark when the divider isn't fitted. */
static char s_bat_shown[24];
static void draw_battery(int x, int y)
{
    battery_t b = battery_read();
    battery_text(&b, s_bat_shown, sizeof s_bat_shown);
    epd_box(x - 26, y - 2, 52, 15, 0);
    epd_frame(x, y, 20, 11);
    epd_box(x + 20, y + 3, 2, 5, 1);
    if (!b.wired) { epd_str(x + 7, y + 2, "?", 1); return; }
    epd_box(x + 2, y + 2, 16 * b.pct / 100, 7, 1);
    if (b.charging) {                                   /* bolt, white through the fill */
        line(x + 11, y + 1, x + 8, y + 6, 0); line(x + 8, y + 6, x + 12, y + 5, 0); line(x + 12, y + 5, x + 9, y + 10, 0);
    }
    char t[8];
    snprintf(t, sizeof t, "%d", b.pct);
    epd_str(x - 3 - epd_str_w(t, 1), y + 2, t, 1);
}

static void draw_ble_banner(void)
{
    s_banner_up = false;
    if (!s_ble_shown) return;
    if (esp_timer_get_time() >= s_banner_until) { draw_ble_icon(); return; }
    s_banner_up = true;
    const char *t = s_ble_shown == 2 ? "PHONE CONNECTED" : "SYNC: open the app";
    int w = epd_str_w(t, 2) + 16;
    int x = (EPD_CW - w) / 2;
    epd_box(x, 36, w, 26, 1);
    epd_str_inv(x + 8, 42, t, 2);
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
    if (!store_ok()) store_mount();
    s_npacks = store_ok() ? pack_scan(STORE_PACKS, s_paths, MAX_PACKS) : 0;
    if (store_ok() && s_npacks == 0) s_npacks = pack_scan(STORE_ROOT, s_paths, MAX_PACKS);
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

static void auto_restart(void) { s_auto_due = esp_timer_get_time() + (int64_t)s_auto_s * 1000000; }

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
    auto_restart();
}

/* wrap: past the last page comes the first (auto-turn, the one-button pass). */
static void turn(int d, bool wrap)
{
    if (!s_pack_ok || !s_pack.pages) return;
    int64_t p = (int64_t)s_rtc.page + d, n = s_pack.pages;
    if (wrap) p = ((p % n) + n) % n;
    if (p < 0 || p >= n) return;                       /* stays put at either end */
    auto_restart();
    if ((uint32_t)p == s_rtc.page) return;             /* a one-page zine */
    s_rtc.page = (uint32_t)p;
    s_rtc.info = false;
    show(false);
    save_place();
}

static void sync_window(void)
{
    ble_pair_window(180);
    s_quiet = false;
    s_ble_shown = ble_console_state();
    s_banner_until = esp_timer_get_time() + BANNER_US;
    show(false);
}

static void first_page(void) { s_rtc.page = 0; s_rtc.info = false; show(true); save_place(); auto_restart(); }

/* SR-15: dial, MENU, EXIT */
static void handle5(int k, bool lng)
{
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
        if (k == KEY_DOWN || k == KEY_OK) turn(+1, false);
        else if (k == KEY_UP) turn(-1, false);
        else if (k == KEY_MENU && lng) first_page();
        else if (k == KEY_MENU) { s_rtc.info = !s_rtc.info; show(false); }
        else if (k == KEY_EXIT) go_shelf();
        break;
    default:
        go_shelf();
        break;
    }
}

/* SR-16: PREV (up), MENU (ok), NEXT (down) */
static void handle3(int k, bool lng)
{
    switch (s_rtc.screen) {
    case SCR_SHELF:
        if (k == KEY_UP && s_rtc.cursor > 0) { s_rtc.cursor--; show(false); }
        else if (k == KEY_DOWN && s_rtc.cursor + 1 < s_npacks) { s_rtc.cursor++; show(false); }
        else if (k == KEY_OK && lng) { s_rtc.screen = SCR_BADGE; show(true); }
        else if (k == KEY_OK) open_cursor();
        break;
    case SCR_PAGE:
        if (k == KEY_UP && lng) first_page();
        else if (k == KEY_DOWN) turn(+1, false);
        else if (k == KEY_UP) turn(-1, false);
        else if (k == KEY_OK && lng) go_shelf();
        else if (k == KEY_OK) { s_rtc.info = !s_rtc.info; show(false); }
        break;
    default:
        go_shelf();
        break;
    }
}

/* SR-16N: only the XIAO's BOOT button (ok) */
static void handle1(int k, bool lng)
{
    if (k != KEY_OK) return;
    switch (s_rtc.screen) {
    case SCR_SHELF:
        if (lng) { ble_pair_window(180); s_quiet = false; open_cursor(); }
        else if (s_npacks) { s_rtc.cursor = (s_rtc.cursor + 1) % s_npacks; show(false); }
        break;
    case SCR_PAGE:
        if (!lng) { turn(+1, true); break; }
        ble_pair_window(180); s_quiet = false;
        rescan();
        if (s_npacks) {                                 /* the next zine, from its first page */
            int cur = 0;
            for (int i = 0; i < s_npacks; i++) if (!strcmp(s_paths[i], s_rtc.path)) cur = i;
            s_rtc.cursor = (cur + 1) % s_npacks;
            s_rtc.path[0] = 0;
            open_cursor();
        }
        break;
    default:
        go_shelf();
        break;
    }
}

static void handle(int ev)
{
    int k = ev & 0x7F;
    bool lng = ev & KEY_LONG;
    ESP_LOGI(TAG, "key %s%s", keys_name(k), lng ? " (long)" : "");
    uint32_t chord = s_keys >= 5 ? (1u << KEY_MENU) | (1u << KEY_EXIT)
                   : s_keys == 3 ? (1u << KEY_UP) | (1u << KEY_DOWN) : 0;
    if (lng && chord && (chord & (1u << k)) && (keys_down() & chord) == chord) { sync_window(); return; }
    if (s_keys >= 5) handle5(k, lng);
    else if (s_keys == 3) handle3(k, lng);
    else handle1(k, lng);
}

/* ---------------------------------------------------------------- sleep */

static bool auto_on(void) { return s_auto_s > 0 && s_rtc.screen == SCR_PAGE && s_pack_ok && s_pack.pages > 1; }

void reader_sleep_now(void)
{
    save_place();
    /* The timer wakes the pass for the next auto-turn or the next check-in,
     * whichever comes first. */
    int32_t t = 0, left = 0;
    if (auto_on()) {
        left = (int32_t)((s_auto_due - esp_timer_get_time()) / 1000000);
        if (left < 1) left = 1;
        t = left;
    }
    if (s_sync_s > 0 && (!t || s_sync_s < t)) t = s_sync_s;
    s_rtc.auto_left = left ? left - t : -1;
    ESP_LOGI(TAG, "sleeping%s", t ? "; the timer wakes the pass" : "; any control wakes the pass");
    epd_sleep();
    epd_hold_for_sleep();
    epd_power(false);
    store_unmount();
#if PIN_LED >= 0
    gpio_set_level(PIN_LED, 0);
#endif
    keys_arm_wake();
    if (t) esp_sleep_enable_timer_wakeup((uint64_t)t * 1000000);
    vTaskDelay(pdMS_TO_TICKS(50));
    esp_deep_sleep_start();
}

/* ---------------------------------------------------------------- task */

static void reader_task(void *arg)
{
    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    bool timer = cause == ESP_SLEEP_WAKEUP_TIMER;
    bool woke = (cause == ESP_SLEEP_WAKEUP_EXT1 || timer) && s_rtc.magic == RTC_MAGIC;
    if (!woke) {
        memset(&s_rtc, 0, sizeof s_rtc);
        s_rtc.magic = RTC_MAGIC;
        s_rtc.auto_left = -1;
        load_settings(true);
        timer = false;
    } else {
        load_settings(false);
    }
    xSemaphoreTake(s_lock, portMAX_DELAY);
    rescan();
    if (s_rtc.screen == SCR_PAGE && !open_path(s_rtc.path)) s_rtc.screen = SCR_SHELF;
    reader_activity();
    auto_restart();
    if (woke) {
        /* The glass still shows what we drew before sleeping: tell the
         * controller, so the first turn can be a quick refresh. */
        redraw();
        epd_set_previous(epd_canvas());
        if (timer) {
            if (auto_on() && s_rtc.auto_left == 0) turn(+1, true);
            else if (s_rtc.auto_left > 0) s_auto_due = esp_timer_get_time() + (int64_t)s_rtc.auto_left * 1000000;
            s_quiet = true;          /* main opens the check-in window; nothing is drawn for it */
            s_brief = true;
        } else {
            int k = keys_woke();
            if (k) keys_inject(k);
        }
    } else {
        show(true);
    }
    xSemaphoreGive(s_lock);
    for (;;) {
        int ev = keys_get(500);
        if (ev) {
            reader_activity();
            xSemaphoreTake(s_lock, portMAX_DELAY);
            handle(ev);
            xSemaphoreGive(s_lock);
        }
        int bs = ble_console_state();
        if (bs == 2) { s_quiet = false; s_brief = false; }
        int vis = (s_quiet && bs == 1) ? 0 : bs;
        if (vis != s_ble_shown) {
            xSemaphoreTake(s_lock, portMAX_DELAY);
            bool was_connected = s_ble_shown == 2;
            s_ble_shown = vis;
            /* words for news (window opened, phone joined); a phone dropping back
             * to an open window only swaps the icon */
            if (!(was_connected && vis == 1) && vis) s_banner_until = esp_timer_get_time() + BANNER_US;
            show(false);
            if (vis == 0 && s_rtc.screen == SCR_SHELF) go_shelf();   /* packs may have arrived */
            xSemaphoreGive(s_lock);
        }
        if (s_banner_up && esp_timer_get_time() >= s_banner_until) {   /* words done: down to the icon */
            xSemaphoreTake(s_lock, portMAX_DELAY);
            show(false);
            xSemaphoreGive(s_lock);
        }
        if (s_info_until && esp_timer_get_time() >= s_info_until) {    /* the page-number bar has had its time */
            xSemaphoreTake(s_lock, portMAX_DELAY);
            s_info_until = 0;
            if (s_rtc.screen == SCR_PAGE && s_rtc.info) { s_rtc.info = false; show(false); }
            xSemaphoreGive(s_lock);
        }
        if (auto_on() && esp_timer_get_time() >= s_auto_due) {
            xSemaphoreTake(s_lock, portMAX_DELAY);
            turn(+1, true);
            xSemaphoreGive(s_lock);
        }
        static int64_t next_bat;
        if (s_rtc.screen == SCR_SHELF && !s_brief && esp_timer_get_time() > next_bat) {
            next_bat = esp_timer_get_time() + 30 * 1000000LL;
            battery_t b = battery_read();
            char t[24];
            if (strcmp(battery_text(&b, t, sizeof t), s_bat_shown)) {
                xSemaphoreTake(s_lock, portMAX_DELAY);
                show(false);
                xSemaphoreGive(s_lock);
            }
        }
        if (bs) s_last_activity = esp_timer_get_time();
        /* Woken by the timer and nobody came: sleep again as soon as the
         * check-in window has shut, not after the full idle time. */
        int64_t idle_us = s_brief ? 300000 : (int64_t)s_sleep_s * 1000000;
        if (s_sleep_s > 0 && keys_down() == 0 && esp_timer_get_time() - s_last_activity > idle_us) {
            xSemaphoreTake(s_lock, portMAX_DELAY);
            reader_sleep_now();
        }
    }
}

/* First thing at boot, so the key that woke the pass is still seen held. */
void reader_keys_init(void)
{
    nvs_handle_t h;
    if (nvs_open("reader", NVS_READONLY, &h) == ESP_OK) {
        int32_t v;
        if (nvs_get_i32(h, "keys", &v) == ESP_OK && (v == 0 || v == 3 || v == 5)) s_keys = v;
        nvs_close(h);
    }
#if defined(BOARD_SR15)
    s_keys = 5;
#endif
    keys_init(s_keys);
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
void reader_cmd_page(int n) { if (s_rtc.screen == SCR_PAGE) turn(n - 1 - (int)s_rtc.page, false); }
void reader_cmd_turn(int d) { if (s_rtc.screen == SCR_PAGE) turn(d, s_keys == 0); }
void reader_cmd_test(void)  { s_rtc.screen = SCR_TEST; show(true); }
void reader_cmd_badge(void) { s_rtc.screen = SCR_BADGE; show(true); }
void reader_cmd_redraw(bool full) { show(full); }
/* Show the title and "page N of M" over the page for a few seconds (a pass
 * with no keys has no MENU to ask with). Caller holds the lock. */
void reader_cmd_where(void)
{
    if (s_rtc.screen != SCR_PAGE || !s_pack_ok) return;
    s_rtc.info = true;
    s_info_until = esp_timer_get_time() + 5 * 1000000LL;
    reader_activity();
    show(false);
}

void reader_set_sleep(int s) { s_sleep_s = s < 0 ? 0 : s; nvs_put_i32("sleep", s_sleep_s); }
void reader_set_full_every(int n) { if (n > 0) { s_full_every = n; nvs_put_i32("full", n); } }
void reader_set_auto(int s) { s_auto_s = s <= 0 ? 0 : s < 5 ? 5 : s; nvs_put_i32("auto", s_auto_s); auto_restart(); }
void reader_set_sync(int s) { s_sync_s = s <= 0 ? 0 : s < 15 ? 15 : s; nvs_put_i32("sync", s_sync_s); }
void reader_set_keys(int n)
{
    if (n != 0 && n != 3) return;
    nvs_put_i32("keys", n);         /* read at the next start: the pins are set up once */
}

bool pack_path(int i, char *out, int len)
{
    if (i < 0 || i >= s_npacks) return false;
    snprintf(out, len, "%s", s_paths[i]);
    return true;
}

void reader_info(void (*out)(const char *fmt, ...))
{
    static const char *scr[] = { "shelf", "page", "badge", "test" };
    uint32_t total = 0, fr = 0;
    battery_t b = battery_read();
    char bt[24];
    store_space(&total, &fr);
    out("screen %s, " STORE_KIND " %s, %d packs, panel %s, rot %d, sleep after %d s, full refresh every %d\n",
        scr[s_rtc.screen], store_ok() ? "mounted" : "none", s_npacks, epd_ctl_name(), epd_rot(), s_sleep_s, s_full_every);
    /* one line the phone app reads: keep the words and their order */
    out("pass " BOARD_PROFILE " %dx%d keys %d auto %d sync %d store %lu/%lu KB bat %s\n",
        EPD_CW, EPD_CH, s_keys, s_auto_s, s_sync_s, (unsigned long)total, (unsigned long)fr, battery_text(&b, bt, sizeof bt));
    if (s_pack_ok)
        out("open: %s  '%s' by %s, page %lu of %lu\n", s_pack.path, s_pack.title, s_pack.author,
            (unsigned long)s_rtc.page + 1, (unsigned long)s_pack.pages);
    for (int i = 0; i < s_npacks; i++) out("  %2d  %s\n", i, s_paths[i]);
}
