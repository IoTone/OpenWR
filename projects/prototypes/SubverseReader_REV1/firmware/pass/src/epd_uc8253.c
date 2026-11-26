/*
 * epd_uc8253.c (SR-16): Elecrow's bare 3.7in panel (DIE04037S, 240 x 416,
 * UltraChip UC8253; Good Display's GDEY037T03 is the same class) on a Seeed
 * XIAO ePaper board, through ESP-IDF spi_master.
 *
 * The controller keeps its waveforms in OTP, so there are no LUTs to load:
 *   0x00 0x1F 0x0D   panel setting (black/white, LUT from OTP)
 *   0x10 / 0x13      the picture on the glass / the picture it should become
 *   0xE0 0x02, 0xE5 t  pin the temperature the waveform is picked by: 0x5A is
 *                    a ~1 s full refresh, 0x6E a ~0.35 s quick one with no flash
 *   0x50             0x97 for a full refresh, 0xD7 for a quick one
 *   0x04, 0x12, 0x02 power on, refresh, power off
 *   0x07 0xA5        deep sleep; a reset wakes it
 * BUSY is low while it works. The sequence follows Good Display's sample code
 * as used by GxEPD2 (GxEPD2_370_GDEY037T03) and Elecrow's 3.7in examples.
 *
 * BUSY is on D2 on Seeed's ePaper Driver Board for XIAO V2 and on D5 on the
 * older XIAO ePaper Breakout Board. An idle controller holds BUSY high, so
 * after a reset the pin that reads high (against a pull-down) is the one.
 * With no panel plugged in neither does: the driver then skips the waits, so
 * a bare XIAO still runs the console and Bluetooth.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include "board.h"
#if defined(BOARD_SR16)
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "nvs.h"
#include "soc/soc_caps.h"

#include "board.h"
#include "epd.h"

static const char *TAG = "epd";

static spi_device_handle_t s_spi;
static uint8_t s_native[EPD_NSTRIDE * EPD_NH];   /* 1 = white, the controller's format */
static uint8_t s_prev[EPD_NSTRIDE * EPD_NH];
static bool s_up, s_asleep, s_have_prev;
static int s_rot;
static int s_busy = -1;                          /* the GPIO BUSY is on; -1: no panel answered */

static void xfer(bool data, const uint8_t *buf, size_t n)
{
    gpio_set_level(PIN_EPD_DC, data);
    while (n) {
        size_t k = n > 4000 ? 4000 : n;
        spi_transaction_t t = { .length = k * 8, .tx_buffer = buf };
        spi_device_polling_transmit(s_spi, &t);
        buf += k; n -= k;
    }
}
static void cmd(uint8_t c) { xfer(false, &c, 1); }
static void dat(uint8_t d) { xfer(true, &d, 1); }

/* Wait for BUSY to go high again. Without a panel, just give it the usual time. */
static int s_took[3];                            /* ms BUSY stayed low: power on, refresh, power off */
static int wait_idle(int max_ms, int usual_ms)
{
    if (s_busy < 0) { vTaskDelay(pdMS_TO_TICKS(usual_ms > 50 ? 50 : usual_ms)); return -1; }
    vTaskDelay(pdMS_TO_TICKS(2));
    for (int i = 0; i < max_ms / 5; i++) {
        if (gpio_get_level(s_busy)) return 2 + i * 5;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    ESP_LOGW(TAG, "busy timeout after %d ms", max_ms);
    return max_ms;
}

/* For the bench: which pin BUSY is on, what both candidates read now, and how
 * long the last refresh held BUSY low in each step. A controller that is
 * really working takes ~100 ms to power on and 0.3-1.5 s to refresh; 2 ms
 * means BUSY never moved (not this controller, or not wired where we think). */
void epd_diag(char *out, int n)
{
    snprintf(out, n, "busy pin GPIO%d, now D2(GPIO%d)=%d D5(GPIO%d)=%d, last refresh: on %d ms, draw %d ms, off %d ms",
             s_busy, PIN_EPD_BUSY, gpio_get_level(PIN_EPD_BUSY), PIN_EPD_BUSY_ALT, gpio_get_level(PIN_EPD_BUSY_ALT),
             s_took[0], s_took[1], s_took[2]);
}

static void panel_setting(void)
{
    cmd(0x00); dat(0x1F); dat(0x0D);
}

static void find_busy(void)
{
    static const int cand[2] = { PIN_EPD_BUSY, PIN_EPD_BUSY_ALT };
    for (int tries = 0; tries < 20; tries++) {            /* up to 200 ms for the controller to come up */
        for (int i = 0; i < 2; i++)
            if (gpio_get_level(cand[i])) { s_busy = cand[i]; return; }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    s_busy = -1;
}

static void controller_init(void)
{
    gpio_hold_dis(PIN_EPD_RST);
    gpio_hold_dis(PIN_EPD_CS);
    gpio_set_level(PIN_EPD_RST, 1); vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_EPD_RST, 0); vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_EPD_RST, 1); vTaskDelay(pdMS_TO_TICKS(20));
    int was = s_busy;
    find_busy();
    if (s_busy != was) {
        if (s_busy < 0) ESP_LOGW(TAG, "no panel answered on BUSY (GPIO%d or GPIO%d)", PIN_EPD_BUSY, PIN_EPD_BUSY_ALT);
        else ESP_LOGI(TAG, "UC8253, BUSY on GPIO%d%s", s_busy, s_busy == PIN_EPD_BUSY_ALT ? " (breakout board)" : "");
    }
    panel_setting();
    s_asleep = false;
}

void epd_power(bool on) { (void)on; }            /* the XIAO boards don't switch the panel's supply */

esp_err_t epd_init(void)
{
    if (!s_up) {
        gpio_config_t out = { .pin_bit_mask = (1ULL << PIN_EPD_DC) | (1ULL << PIN_EPD_RST), .mode = GPIO_MODE_OUTPUT };
        gpio_config(&out);
        gpio_config_t in = { .pin_bit_mask = (1ULL << PIN_EPD_BUSY) | (1ULL << PIN_EPD_BUSY_ALT), .mode = GPIO_MODE_INPUT,
                             .pull_down_en = GPIO_PULLDOWN_ENABLE };
        gpio_config(&in);
        spi_bus_config_t bus = { .sclk_io_num = PIN_EPD_SCK, .mosi_io_num = PIN_EPD_MOSI, .miso_io_num = -1,
                                 .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 4096 };
        esp_err_t e = spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO);
        if (e != ESP_OK) return e;
        spi_device_interface_config_t dev = { .clock_speed_hz = 8000000, .mode = 0,
                                              .spics_io_num = PIN_EPD_CS, .queue_size = 1 };
        e = spi_bus_add_device(SPI2_HOST, &dev, &s_spi);
        if (e != ESP_OK) return e;
        nvs_handle_t h;
        if (nvs_open("epd", NVS_READONLY, &h) == ESP_OK) {
            int8_t r;
            if (nvs_get_i8(h, "rot", &r) == ESP_OK) s_rot = r & 3;
            nvs_close(h);
        }
        s_up = true;
    }
    controller_init();
    return ESP_OK;
}

const char *epd_ctl_name(void) { return s_busy < 0 ? "none" : "UC8253"; }
int  epd_busy_pin(void) { return s_busy; }
static int s_plain;                             /* 1: the OTP's own refresh for the real temperature, no pinned waveform */
void epd_set_ctl(int c) { s_plain = c == 1; s_have_prev = false; }
int  epd_rot(void) { return s_rot; }

void epd_set_rot(int r)
{
    s_rot = r & 3;
    s_have_prev = false;                 /* the old picture is in the old orientation */
    nvs_handle_t h;
    if (nvs_open("epd", NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_i8(h, "rot", (int8_t)s_rot);
        nvs_commit(h);
        nvs_close(h);
    }
}

/* Canvas and glass are both 240 x 416 portrait:
 *   rot 0: as is            rot 2: turned 180 degrees (the pass held the other way up)
 *   rot 1: mirrored left-right, rot 3: mirrored top-bottom, in case the glass scans the other way. */
static void to_native(const uint8_t *canvas, uint8_t *out)
{
    bool fx = s_rot == 1 || s_rot == 2, fy = s_rot == 2 || s_rot == 3;
    if (!fx) {
        for (int cy = 0; cy < EPD_CH; cy++) {
            const uint8_t *src = canvas + cy * EPD_CSTRIDE;
            uint8_t *dst = out + (fy ? EPD_NH - 1 - cy : cy) * EPD_NSTRIDE;
            for (int i = 0; i < EPD_NSTRIDE; i++) dst[i] = ~src[i];
        }
        return;
    }
    memset(out, 0xFF, EPD_NSTRIDE * EPD_NH);
    for (int cy = 0; cy < EPD_CH; cy++) {
        const uint8_t *src = canvas + cy * EPD_CSTRIDE;
        uint8_t *dst = out + (fy ? EPD_NH - 1 - cy : cy) * EPD_NSTRIDE;
        for (int cx = 0; cx < EPD_CW; cx++) {
            if (!(src[cx >> 3] & (0x80 >> (cx & 7)))) continue;
            int nx = EPD_NW - 1 - cx;
            dst[nx >> 3] &= ~(0x80 >> (nx & 7));
        }
    }
}

void epd_set_previous(const uint8_t *canvas)
{
    to_native(canvas, s_prev);
    s_have_prev = true;
}

/* Undo the pinned temperature the way Good Display's code does: a soft reset,
 * then the panel setting again. The picture memory survives it. */
static void soft_reset(void)
{
    cmd(0x00); dat(0x1E); dat(0x0D);
    vTaskDelay(pdMS_TO_TICKS(2));
    panel_setting();
}

/* The whole glass as a partial window: the quick waveform is only applied
 * properly inside one (as in GxEPD2's driver for this panel). */
static void window_all(void)
{
    cmd(0x90);
    dat(0); dat((EPD_NW - 1) | 7);
    dat(0); dat(0);
    dat((EPD_NH - 1) >> 8); dat((EPD_NH - 1) & 0xFF);
    dat(0x01);
}

void epd_show(bool full)
{
    if (!s_up) return;
    if (s_asleep || s_plain) controller_init();
    to_native(epd_canvas(), s_native);
    if (!s_have_prev) { memset(s_prev, 0xFF, sizeof s_prev); full = true; }
    if (full || s_plain) {
        cmd(0x10); xfer(true, s_prev, sizeof s_prev);
        cmd(0x13); xfer(true, s_native, sizeof s_native);
        if (!s_plain) { cmd(0xE0); dat(0x02); cmd(0xE5); dat(0x5A); }
        cmd(0x50); dat(0x97);
        cmd(0x04); s_took[0] = wait_idle(1000, 50);                 /* power on */
        cmd(0x12); vTaskDelay(pdMS_TO_TICKS(10));                   /* refresh */
        s_took[1] = wait_idle(s_plain ? 20000 : 8000, 1100);
        cmd(0x02); s_took[2] = wait_idle(1000, 50);                 /* power off */
        soft_reset();
    } else {
        cmd(0x91); window_all();
        cmd(0x10); xfer(true, s_prev, sizeof s_prev);               /* what is on the glass */
        cmd(0x92);
        cmd(0x91); window_all();
        cmd(0x13); xfer(true, s_native, sizeof s_native);           /* what it should become */
        cmd(0x92);
        cmd(0x91); window_all();
        cmd(0xE0); dat(0x02);
        cmd(0xE5); dat(0x6E);
        cmd(0x50); dat(0xD7);
        cmd(0x04); s_took[0] = wait_idle(1000, 50);
        cmd(0x12); vTaskDelay(pdMS_TO_TICKS(10));
        s_took[1] = wait_idle(4000, 350);
        cmd(0x02); s_took[2] = wait_idle(1000, 50);
        soft_reset();
        cmd(0x92);
    }
    memcpy(s_prev, s_native, sizeof s_prev);
    s_have_prev = true;
}

void epd_sleep(void)
{
    if (!s_up || s_asleep) return;
    cmd(0x07); dat(0xA5);
    vTaskDelay(pdMS_TO_TICKS(20));
    s_asleep = true;
}

/* The chip lets go of its pins in deep sleep. Hold reset and chip-select
 * high so the sleeping controller isn't reset or addressed by noise. */
void epd_hold_for_sleep(void)
{
    gpio_set_level(PIN_EPD_RST, 1);
    gpio_hold_en(PIN_EPD_RST);
    gpio_hold_en(PIN_EPD_CS);
#if SOC_GPIO_SUPPORT_HOLD_IO_IN_DSLP && !SOC_GPIO_SUPPORT_HOLD_SINGLE_IO_IN_DSLP
    gpio_deep_sleep_hold_en();
#endif
}
/* Bench probe: what BUSY does around a reset and after a few commands,
 * sampled every 100 us, once through the SPI driver and once bit-banged. */
#include "rom/ets_sys.h"
static int low_samples(int ms, int *first_low_us)
{
    int low = 0; *first_low_us = -1;
    for (int i = 0; i < ms * 10; i++) {
        if (!gpio_get_level(PIN_EPD_BUSY)) { low++; if (*first_low_us < 0) *first_low_us = i * 100; }
        ets_delay_us(100);
    }
    return low;
}
static void bb_byte(bool data, uint8_t b)
{
    gpio_set_level(PIN_EPD_DC, data);
    gpio_set_level(PIN_EPD_CS, 0);
    for (int i = 7; i >= 0; i--) {
        gpio_set_level(PIN_EPD_MOSI, (b >> i) & 1); ets_delay_us(2);
        gpio_set_level(PIN_EPD_SCK, 1); ets_delay_us(2);
        gpio_set_level(PIN_EPD_SCK, 0);
    }
    gpio_set_level(PIN_EPD_CS, 1); ets_delay_us(2);
}
void epd_probe(void (*say)(const char *fmt, ...))
{
    int f, n;
    gpio_set_pull_mode(PIN_EPD_BUSY, GPIO_PULLDOWN_ONLY);
    say("BUSY idle, pull-down: %d\n", gpio_get_level(PIN_EPD_BUSY));
    gpio_set_pull_mode(PIN_EPD_BUSY, GPIO_FLOATING); ets_delay_us(2000);
    say("BUSY idle, floating:  %d\n", gpio_get_level(PIN_EPD_BUSY));
    gpio_set_pull_mode(PIN_EPD_BUSY, GPIO_PULLUP_ONLY); ets_delay_us(2000);
    say("BUSY idle, pull-up:   %d\n", gpio_get_level(PIN_EPD_BUSY));
    gpio_set_pull_mode(PIN_EPD_BUSY, GPIO_PULLDOWN_ONLY);
    gpio_set_level(PIN_EPD_RST, 0); ets_delay_us(20000);
    say("RST low (pin reads back %d): BUSY %d\n", gpio_get_level(PIN_EPD_RST), gpio_get_level(PIN_EPD_BUSY));
    gpio_set_level(PIN_EPD_RST, 1);
    n = low_samples(200, &f); say("after RST high: BUSY low %d of 2000 samples, first at %d us\n", n, f);
    cmd(0x04); n = low_samples(300, &f); say("spi  0x04 power on: low %d/3000, first %d us\n", n, f);
    cmd(0x02); n = low_samples(300, &f); say("spi  0x02 power off: low %d/3000, first %d us\n", n, f);
    cmd(0x12); n = low_samples(300, &f); say("spi  0x12: low %d/3000, first %d us\n", n, f);
    /* the same by hand, in case the SPI host is not reaching the pins */
    spi_bus_remove_device(s_spi); spi_bus_free(SPI2_HOST);
    gpio_config_t out = { .pin_bit_mask = (1ULL << PIN_EPD_CS) | (1ULL << PIN_EPD_SCK) | (1ULL << PIN_EPD_MOSI) | (1ULL << PIN_EPD_DC) | (1ULL << PIN_EPD_RST),
                          .mode = GPIO_MODE_INPUT_OUTPUT };
    gpio_config(&out);
    gpio_set_level(PIN_EPD_CS, 1); gpio_set_level(PIN_EPD_SCK, 0);
    say("pins read back: CS %d SCK %d MOSI %d DC %d RST %d\n", gpio_get_level(PIN_EPD_CS), gpio_get_level(PIN_EPD_SCK),
        gpio_get_level(PIN_EPD_MOSI), gpio_get_level(PIN_EPD_DC), gpio_get_level(PIN_EPD_RST));
    gpio_set_level(PIN_EPD_RST, 0); ets_delay_us(20000); gpio_set_level(PIN_EPD_RST, 1); ets_delay_us(20000);
    bb_byte(false, 0x04); n = low_samples(300, &f); say("hand 0x04 power on: low %d/3000, first %d us\n", n, f);
    bb_byte(false, 0x02); n = low_samples(300, &f); say("hand 0x02 power off: low %d/3000, first %d us\n", n, f);
    bb_byte(false, 0x12); n = low_samples(500, &f); say("hand 0x12 (SSD16xx: soft reset): low %d/5000, first %d us\n", n, f);
    /* Can each panel line be driven both ways? (a pin stuck low sends all zeros) */
    static const struct { const char *n; int g; } dl[] = { {"RST D0",PIN_EPD_RST}, {"CS D1",PIN_EPD_CS}, {"DC D3",PIN_EPD_DC},
                                                             {"SCK D8",PIN_EPD_SCK}, {"D9",9}, {"MOSI D10",PIN_EPD_MOSI} };
    for (unsigned k = 0; k < sizeof dl / sizeof dl[0]; k++) {
        gpio_config_t io = { .pin_bit_mask = 1ULL << dl[k].g, .mode = GPIO_MODE_INPUT_OUTPUT };
        gpio_config(&io);
        gpio_set_level(dl[k].g, 1); ets_delay_us(500); int hi = gpio_get_level(dl[k].g);
        gpio_set_level(dl[k].g, 0); ets_delay_us(500); int lo = gpio_get_level(dl[k].g);
        say("drive %-8s GPIO%-2d: high reads %d, low reads %d%s\n", dl[k].n, dl[k].g, hi, lo, hi == 1 && lo == 0 ? "" : "  <-- STUCK");
    }
    gpio_set_level(PIN_EPD_RST, 1); gpio_set_level(PIN_EPD_CS, 1);
    /* Read back over the data line (these controllers turn it round after a
     * read command). The same bytes under a pull-up and a pull-down = the chip
     * is driving the line; FF then 00 = nobody answered. */
    static const uint8_t rd[] = { 0x70, 0x71, 0x40, 0x2F, 0x2E, 0x1B, 0x2D };
    for (unsigned c = 0; c < sizeof rd; c++) {
        uint8_t got[2][4];
        for (int pu = 0; pu < 2; pu++) {
            gpio_set_direction(PIN_EPD_MOSI, GPIO_MODE_INPUT_OUTPUT);
            gpio_set_level(PIN_EPD_DC, 0); gpio_set_level(PIN_EPD_CS, 0);
            for (int i = 7; i >= 0; i--) {
                gpio_set_level(PIN_EPD_MOSI, (rd[c] >> i) & 1); ets_delay_us(3);
                gpio_set_level(PIN_EPD_SCK, 1); ets_delay_us(3); gpio_set_level(PIN_EPD_SCK, 0);
            }
            gpio_set_direction(PIN_EPD_MOSI, GPIO_MODE_INPUT);
            gpio_set_pull_mode(PIN_EPD_MOSI, pu ? GPIO_PULLUP_ONLY : GPIO_PULLDOWN_ONLY);
            gpio_set_level(PIN_EPD_DC, 1); ets_delay_us(10);
            for (int b = 0; b < 4; b++) {
                uint8_t v = 0;
                for (int i = 7; i >= 0; i--) {
                    ets_delay_us(3); gpio_set_level(PIN_EPD_SCK, 1); ets_delay_us(2);
                    v |= gpio_get_level(PIN_EPD_MOSI) << i;
                    ets_delay_us(1); gpio_set_level(PIN_EPD_SCK, 0);
                }
                got[pu][b] = v;
            }
            gpio_set_level(PIN_EPD_CS, 1); ets_delay_us(20);
        }
        say("read 0x%02X: pull-down %02X %02X %02X %02X, pull-up %02X %02X %02X %02X\n", rd[c],
            got[0][0], got[0][1], got[0][2], got[0][3], got[1][0], got[1][1], got[1][2], got[1][3]);
    }
    /* Is the panel's supply behind a switch on one of the spare pins? Look at
     * each (what it reads under each weak pull), then try the panel with that
     * pin pulled each way. Weak pulls only: nothing here can fight a driver. */
    static const struct { const char *n; int g; } sp[] = { {"D4",23}, {"D5",24}, {"D6",11}, {"D7",12}, {"D9",9},
                                                             {"GPIO2",2}, {"GPIO3",3}, {"GPIO4",4}, {"GPIO5",5} };
    for (unsigned k = 0; k < sizeof sp / sizeof sp[0]; k++) {
        gpio_config_t in = { .pin_bit_mask = 1ULL << sp[k].g, .mode = GPIO_MODE_INPUT };
        gpio_config(&in);
        int lv[2], lows[2];
        for (int pu = 0; pu < 2; pu++) {
            gpio_set_pull_mode(sp[k].g, pu ? GPIO_PULLUP_ONLY : GPIO_PULLDOWN_ONLY);
            vTaskDelay(pdMS_TO_TICKS(60));
            lv[pu] = gpio_get_level(sp[k].g);
            gpio_set_level(PIN_EPD_RST, 0); ets_delay_us(20000); gpio_set_level(PIN_EPD_RST, 1); ets_delay_us(30000);
            bb_byte(false, 0x04); lows[pu] = low_samples(200, &f);
            bb_byte(false, 0x02); ets_delay_us(50000);
        }
        gpio_set_pull_mode(sp[k].g, GPIO_FLOATING);
        say("%-5s GPIO%-2d reads %d/%d under pull-down/up; panel BUSY lows after power-on: %d / %d\n",
            sp[k].n, sp[k].g, lv[0], lv[1], lows[0], lows[1]);
    }
    gpio_set_pull_mode(PIN_EPD_BUSY_ALT, GPIO_PULLDOWN_ONLY);
    say("BUSY(D2) now %d, D5 now %d. Restart the pass to use the panel again.\n", gpio_get_level(PIN_EPD_BUSY), gpio_get_level(PIN_EPD_BUSY_ALT));
}
#endif
