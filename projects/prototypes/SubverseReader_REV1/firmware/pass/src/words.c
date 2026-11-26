/*
 * words.c: Forth words for bring-up and for the phone app, on the USB-C
 * console and over Bluetooth.
 *
 *   sr-info        ( -- )      where the reader is, the packs on the card
 *   sr-test        ( -- )      orientation screen: TOP at the top, arrow at the dial
 *   epd-rot!       ( n -- )    0..3, kept in NVS; redraws the test screen
 *   epd-rot        ( -- n )
 *   sr-shelf       ( -- )      back to the shelf (rescans the card)
 *   sr-open        ( i -- )    open pack i (sr-info lists them)
 *   sr-page        ( n -- )    go to page n (1-based)
 *   sr-next sr-prev ( -- )
 *   sr-badge       ( -- )
 *   sr-full        ( -- )      redraw with a full refresh
 *   sr-sleep!      ( s -- )    idle seconds before deep sleep, 0 = never (NVS)
 *   sr-full-every! ( n -- )    full refresh every n page turns (NVS)
 *   sr-auto!       ( s -- )    auto-turn: a page every s seconds (5 or more), 0 = off (NVS)
 *   sr-sync!       ( s -- )    timed Bluetooth check-in every s seconds of sleep, 0 = off (NVS)
 *   sr-keys!       ( n -- )    SR-16: 3 = the three keys are fitted, 0 = none; restarts the pass
 *   sr-sleep       ( -- )      sleep now
 *   led            ( f -- )
 *   keys@          ( -- mask ) bit 1 up/PREV, 2 down/NEXT, 3 ok (dial push, MENU on SR-16, BOOT), 4 menu, 5 exit
 *   epd-ctl!       ( n -- )    panel controller: 0 auto-detect, 1 SSD1683, 2 UC81xx (NVS)
 *   sr-bat         ( -- )      battery: voltage, %, USB, charging (needs the IO3 divider)
 *   sr-df          ( -- )      store: total and free KB
 *   sd-recv        ( -- )      loader: receive one file into the store's /packs (tools/sr_push.py, the app)
 *   sd-rm          ( i -- )    delete pack i
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_rom_crc.h"
#include "store.h"
#include "battery.h"
#include "forth_core.h"
#include "console.h"
#include "board.h"
#include "epd.h"
#include "keys.h"
#include "reader.h"

static void w_info(void)   { reader_lock(); reader_info(con_printf); reader_unlock(); }
static void w_test(void)   { reader_lock(); reader_cmd_test(); reader_unlock(); }
static void w_rot_store(void) { int r = (int)forth_pop(); reader_lock(); epd_set_rot(r); reader_cmd_test(); reader_unlock(); }
static void w_ctl(void)    { int c = (int)forth_pop(); reader_lock(); epd_set_ctl(c); reader_cmd_test(); reader_unlock(); }
static void w_bat(void)
{
    battery_t b = battery_read();
    char t[40];
    con_printf("%s  cell %d mV  usb %s\n", battery_text(&b, t, sizeof t), b.mv,
               b.usb == 1 ? "in" : "not seen");
    if (!b.wired) con_printf("%s\n", battery_howto());
}
static void w_adc(void)   { int g = (int)forth_pop(); forth_push(battery_probe(g, false)); }
static void w_adcpd(void) { int g = (int)forth_pop(); forth_push(battery_probe(g, true)); }
static void w_rot(void)    { forth_push(epd_rot()); }
static void w_shelf(void)  { reader_lock(); reader_cmd_shelf(); reader_unlock(); }
static void w_open(void)   { int i = (int)forth_pop(); reader_lock(); reader_cmd_open(i); reader_unlock(); }
static void w_page(void)   { int n = (int)forth_pop(); reader_lock(); reader_cmd_page(n); reader_unlock(); }
static void w_where(void)  { reader_lock(); reader_cmd_where(); reader_unlock(); }
static void w_next(void)   { reader_lock(); reader_cmd_turn(+1); reader_unlock(); }
static void w_prev(void)   { reader_lock(); reader_cmd_turn(-1); reader_unlock(); }
static void w_badge(void)  { reader_lock(); reader_cmd_badge(); reader_unlock(); }
static void w_full(void)   { reader_lock(); reader_cmd_redraw(true); reader_unlock(); }
static void w_sleep_store(void) { reader_set_sleep((int)forth_pop()); }
static void w_full_every(void)  { reader_set_full_every((int)forth_pop()); }
static void w_sleep(void)  { reader_lock(); reader_sleep_now(); }
static void w_led(void)
{
    int on = forth_pop() != 0;
#if PIN_LED >= 0
    gpio_set_level(PIN_LED, on);
#else
    (void)on;
#endif
}
static void w_auto(void)   { int s = (int)forth_pop(); reader_lock(); reader_set_auto(s); reader_unlock(); }
static void w_sync(void)   { reader_set_sync((int)forth_pop()); }
static void w_keys_store(void)
{
    int n = (int)forth_pop();
#if defined(BOARD_SR16)
    if (n != 0 && n != 3) { con_printf("? 0 (no keys) or 3\n"); return; }
    reader_set_keys(n);
    con_printf("keys %d: restarting\n", n);
    vTaskDelay(pdMS_TO_TICKS(200));
    esp_restart();
#else
    (void)n;
    con_printf("this pass's controls are fixed\n");
#endif
}
#if defined(BOARD_SR16)
void epd_diag(char *out, int n);
void epd_probe(void (*say)(const char *fmt, ...));
static void w_epd_probe(void) { reader_lock(); epd_probe((void (*)(const char *, ...))con_printf); reader_unlock(); }
static void w_epd_diag(void) { char t[160]; epd_diag(t, sizeof t); con_printf("%s\n", t); }
#endif
static void w_df(void)
{
    uint32_t total, fr;
    if (store_space(&total, &fr)) con_printf(STORE_KIND " %lu KB, %lu KB free\n", (unsigned long)total, (unsigned long)fr);
    else con_printf("no " STORE_KIND "\n");
}
static void w_keys(void)   { forth_push(keys_down()); }

/* sd-recv: the host sends "name size crc32hex\n" then size raw bytes. The
 * file lands as <store>/packs/name.part and is renamed once its CRC matches, so
 * a broken transfer never leaves a half pack on the shelf. Replies with one
 * line: "OK name size" or "ERR why". */
static int con_line(char *buf, int max, int ms)
{
    int n = 0;
    while (n < max - 1) {
        uint8_t c;
        if (con_read(&c, 1, ms) <= 0) return -1;
        if (c == '\r') continue;
        if (c == '\n') break;
        buf[n++] = c;
    }
    buf[n] = 0;
    return n;
}

static void w_recv(void)
{
    char line[128], name[64], path[96], part[104];
    unsigned long size, crc;
    reader_lock();
    if (!store_ok() && store_mount() != ESP_OK) { con_printf("\nERR no " STORE_KIND "\n"); reader_unlock(); return; }
    mkdir(STORE_PACKS, 0777);
    con_printf("\nREADY\n");
    if (con_line(line, sizeof line, 5000) < 0 || sscanf(line, "%63s %lu %lx", name, &size, &crc) != 3
        || strchr(name, '/') || size == 0 || size > 64UL * 1024 * 1024) {
        con_printf("ERR bad header\n"); reader_unlock(); return;
    }
    uint32_t total, fr;
    if (store_space(&total, &fr) && size / 1024 + 8 > fr) { con_printf("ERR full: %lu KB free\n", (unsigned long)fr); reader_unlock(); return; }
    snprintf(path, sizeof path, STORE_PACKS "/%.60s", name);
    snprintf(part, sizeof part, "%s.part", path);
    FILE *f = fopen(part, "wb");
    if (!f) { con_printf("ERR can't write %s\n", part); reader_unlock(); return; }
    static uint8_t buf[4096];
    uint32_t c = 0;
    unsigned long got = 0;
    while (got < size) {
        int want = size - got > sizeof buf ? sizeof buf : (int)(size - got);
        int n = con_read(buf, want, 5000);
        if (n <= 0) break;
        c = esp_rom_crc32_le(c, buf, n);
        if (fwrite(buf, 1, n, f) != (size_t)n) break;
        got += n;
        reader_activity();
    }
    fclose(f);
    if (got != size) { unlink(part); con_printf("ERR got %lu of %lu bytes\n", got, size); reader_unlock(); return; }
    if (c != crc) { unlink(part); con_printf("ERR crc %08lx, expected %08lx\n", (unsigned long)c, crc); reader_unlock(); return; }
    unlink(path);
    rename(part, path);
    con_printf("OK %s %lu\n", name, size);
    reader_unlock();
}

static void w_rm(void)
{
    int i = (int)forth_pop();
    char p[96];
    if (pack_path(i, p, sizeof p) && unlink(p) == 0) con_printf("removed %s\n", p);
    else con_printf("no pack %d\n", i);
}

void words_register(void)
{
    static const struct { const char *name; forth_word_fn fn; } w[] = {
        { "sr-info", w_info }, { "sr-test", w_test }, { "epd-rot!", w_rot_store }, { "epd-rot", w_rot },
        { "sr-shelf", w_shelf }, { "sr-open", w_open }, { "sr-page", w_page }, { "sr-next", w_next },
        { "sr-prev", w_prev }, { "sr-badge", w_badge }, { "sr-full", w_full }, { "sr-sleep!", w_sleep_store },
        { "sr-full-every!", w_full_every }, { "sr-sleep", w_sleep }, { "led", w_led }, { "keys@", w_keys },
        { "sd-recv", w_recv }, { "epd-ctl!", w_ctl }, { "sr-bat", w_bat }, { "adc@", w_adc }, { "adc-pd@", w_adcpd }, { "sd-rm", w_rm },
        { "sr-auto!", w_auto }, { "sr-sync!", w_sync }, { "sr-keys!", w_keys_store }, { "sr-df", w_df }, { "sr-where", w_where },
#if defined(BOARD_SR16)
        { "epd-diag", w_epd_diag }, { "epd-probe", w_epd_probe },
#endif
    };
    for (unsigned i = 0; i < sizeof w / sizeof w[0]; i++) forth_register_word(w[i].name, w[i].fn);
}
