/*
 * goggle_words.c — Forth vocabulary for the goggl1 prototype.
 *
 * Thin wrappers: pop arguments, call the driver, push results. Everything
 * that decides *what* to draw lives in Forth (see the forth/ directory) so it can
 * be changed at the ok> prompt with the goggles on.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_random.h"

#include "forth_core.h"
#include "console.h"
#include "pins_beetle_esp32c3.h"
#include "ssd1309.h"
#include "goggle_words.h"

static const char *TAG = "goggle";
static bool s_display_ok = false;

/* ------------------------------------------------------------------ */
/* Bring-up                                                            */
/* ------------------------------------------------------------------ */

esp_err_t goggle_init(void)
{
    gpio_config_t led = {
        .pin_bit_mask = 1ULL << GOGGL_PIN_LED,
        .mode         = GPIO_MODE_OUTPUT,
    };
    gpio_config(&led);
    gpio_set_level(GOGGL_PIN_LED, 0);

    gpio_config_t btn = {
        .pin_bit_mask = 1ULL << GOGGL_PIN_BUTTON,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&btn);

    ssd1309_config_t cfg = {
        .spi_host = GOGGL_SPI_HOST,
        .pin_sclk = GOGGL_PIN_SCLK,
        .pin_mosi = GOGGL_PIN_MOSI,
        .pin_dc   = GOGGL_PIN_DC,
        .pin_rst  = GOGGL_PIN_RST,
        .pin_cs   = GOGGL_PIN_CS,
        .clock_hz = 4 * 1000 * 1000,
        .x_offset = 2,       /* U8g2 "NONAME2" — what the DFRobot demos select */
    };
    esp_err_t err = oled_init(&cfg);
    s_display_ok = (err == ESP_OK);
    if (!s_display_ok) ESP_LOGE(TAG, "display init failed: %s", esp_err_to_name(err));
    return err;
}

void goggle_splash(const char *line1, const char *line2)
{
    oled_clear();
    oled_set_color(OLED_COLOR_SET);
    oled_set_dir(0);
    oled_rframe(0, 0, SSD1309_WIDTH, 56, 4);
    oled_set_scale(2);
    oled_str(8, 8, line1, strlen(line1));
    oled_set_scale(1);
    oled_str(8, 30, line2, strlen(line2));
    oled_str(8, 42, "ok> on USB", 10);
    oled_show();
}

/*
 * Layout test pattern — the power-on screen.
 *
 * Built so that one photo of the glass answers the questions you have when
 * laying out a HUD on this panel:
 *
 *   - Which rows/columns are actually visible? A 1 px frame runs round the
 *     full 128x64 controller area, so a missing edge is a hidden edge.
 *   - Where is the 56-row transparent window? A dotted line at y=55 marks
 *     the edge of the top 56 rows, labelled "56"; if that line is the last
 *     thing visible, the window is rows 0..55.
 *   - Where is the centre? Crosshair at (64,32), with a circle of r=10 to
 *     show the aspect ratio is square.
 *   - Are x and y the way round you think? Column ticks along the top
 *     edge every 8 px (taller every 32) labelled 32/64/96, row ticks down
 *     the left edge labelled 16/32/48. "0" in the top-left corner, "127"
 *     and "63" at the far right / bottom edges.
 *   - Is the panel showing single pixels cleanly? An 8x8 checkerboard and
 *     a 2 px dot grid, both in small frames below the crosshair.
 */
void goggle_test_pattern(void)
{
    const int W = SSD1309_WIDTH, H = SSD1309_HEIGHT;
    oled_clear();
    oled_set_color(OLED_COLOR_SET);
    oled_set_dir(0);
    oled_set_scale(1);

    /* full-area frame */
    oled_frame(0, 0, W, H);

    /* column ticks along the top: every 8 px, longer every 32 */
    for (int x = 8; x < W; x += 8) oled_vline(x, 1, (x % 32 == 0) ? 4 : 2);
    /* row ticks down the left: every 8 px, longer every 16 */
    for (int y = 8; y < H; y += 8) oled_hline(1, y, (y % 16 == 0) ? 4 : 2);

    /* axis labels */
    oled_str(2, 2, "0", 1);
    oled_str(32 - 5, 6, "32", 2);
    oled_str(64 - 5, 6, "64", 2);
    oled_str(96 - 5, 6, "96", 2);
    oled_str(W - 19, 2, "127", 3);
    oled_str(6, 16 - 3, "16", 2);
    oled_str(6, 32 - 3, "32", 2);
    oled_str(6, 48 - 3, "48", 2);
    oled_str(W - 13, H - 9, "63", 2);

    /* transparent-window edge: dotted line on row 55, labelled */
    for (int x = 2; x < W - 2; x += 2) oled_pixel(x, 55);
    oled_str(W - 37, 47, "y56", 3);

    /* centre crosshair + circle */
    oled_hline(64 - 14, 32, 29);
    oled_vline(64, 32 - 14, 29);
    oled_circle(64, 32, 10);

    /* pixel-level checks */
    for (int j = 0; j < 8; j++)              /* 8x8 checkerboard, bottom-left */
        for (int i = 0; i < 8; i++)
            if ((i + j) & 1) oled_pixel(20 + i, 46 + j);
    oled_frame(19, 45, 10, 10);
    for (int j = 0; j < 8; j += 2)           /* 2 px dot grid, left of centre */
        for (int i = 0; i < 8; i += 2) oled_pixel(40 + i, 46 + j);
    oled_frame(39, 45, 10, 10);

    /* a diagonal on the right to check line rendering */
    oled_line(104, 14, 125, 44);

    oled_show();
}

/* ------------------------------------------------------------------ */
/* Framebuffer words                                                   */
/* ------------------------------------------------------------------ */

#define POP() ((int)forth_pop())

static void w_oled_clear(void)  { oled_clear(); }
static void w_oled_show(void)   { if (s_display_ok) oled_show(); }
static void w_oled_color(void)  { oled_set_color(POP()); }

static void w_oled_pixel(void)  { int y = POP(), x = POP(); oled_pixel(x, y); }
static void w_oled_pixel_at(void) { int y = POP(), x = POP(); forth_push(oled_get_pixel(x, y)); }

static void w_oled_line(void)
{
    int y1 = POP(), x1 = POP(), y0 = POP(), x0 = POP();
    oled_line(x0, y0, x1, y1);
}
static void w_oled_hline(void)  { int w = POP(), y = POP(), x = POP(); oled_hline(x, y, w); }
static void w_oled_vline(void)  { int h = POP(), y = POP(), x = POP(); oled_vline(x, y, h); }

static void w_oled_frame(void)
{
    int h = POP(), w = POP(), y = POP(), x = POP();
    oled_frame(x, y, w, h);
}
static void w_oled_box(void)
{
    int h = POP(), w = POP(), y = POP(), x = POP();
    oled_box(x, y, w, h);
}
static void w_oled_rframe(void)
{
    int r = POP(), h = POP(), w = POP(), y = POP(), x = POP();
    oled_rframe(x, y, w, h, r);
}
static void w_oled_rbox(void)
{
    int r = POP(), h = POP(), w = POP(), y = POP(), x = POP();
    oled_rbox(x, y, w, h, r);
}
static void w_oled_circle(void) { int r = POP(), y = POP(), x = POP(); oled_circle(x, y, r); }
static void w_oled_disc(void)   { int r = POP(), y = POP(), x = POP(); oled_disc(x, y, r); }

static void w_oled_triangle(void)
{
    int y2 = POP(), x2 = POP(), y1 = POP(), x1 = POP(), y0 = POP(), x0 = POP();
    oled_triangle(x0, y0, x1, y1, x2, y2);
}

static void xbm_common(bool transparent)
{
    int y = POP(), x = POP(), h = POP(), w = POP();
    const uint8_t *bits = (const uint8_t *)forth_pop();
    if (bits) oled_xbm(x, y, w, h, bits, transparent);
}
static void w_oled_xbm(void)       { xbm_common(true); }
static void w_oled_xbm_solid(void) { xbm_common(false); }

/* ------------------------------------------------------------------ */
/* Text                                                                */
/* ------------------------------------------------------------------ */

static void w_oled_str(void)
{
    int y = POP(), x = POP(), len = POP();
    const char *s = (const char *)forth_pop();
    if (s && len > 0) oled_str(x, y, s, len);
}
/* ( n x y -- )  draw a signed integer, left-aligned at x,y */
static void w_oled_num(void)
{
    int y = POP(), x = POP(), n = POP();
    char b[16]; int len = snprintf(b, sizeof b, "%d", n);
    oled_str(x, y, b, len);
}
/* ( n x y -- )  two digits, zero padded: hours, minutes */
static void w_oled_num02(void)
{
    int y = POP(), x = POP(), n = POP();
    char b[16]; int len = snprintf(b, sizeof b, "%02d", n);
    oled_str(x, y, b, len);
}
/* ( n x y -- )  draw n right-aligned so its last digit ends at x */
static void w_oled_numr(void)
{
    int y = POP(), x = POP(), n = POP();
    char b[16]; int len = snprintf(b, sizeof b, "%d", n);
    oled_str(x - oled_str_width(len), y, b, len);
}
static void w_oled_glyph(void)  { int y = POP(), x = POP(), c = POP(); oled_glyph(x, y, (char)c); }
static void w_oled_scale(void)  { oled_set_scale(POP()); }
static void w_oled_scale_xy(void) { int sy = POP(), sx = POP(); oled_set_scale_xy(sx, sy); }
static void w_oled_dir(void)    { oled_set_dir(POP()); }
static void w_oled_strw(void)   { forth_push(oled_str_width(POP())); }

/* ------------------------------------------------------------------ */
/* Panel                                                               */
/* ------------------------------------------------------------------ */

static void w_oled_on(void)       { if (s_display_ok) oled_power(POP() != 0); else POP(); }
static void w_oled_contrast(void) { int n = POP(); if (s_display_ok) oled_contrast((uint8_t)(n & 0xFF)); }
static void w_oled_invert(void)   { if (s_display_ok) oled_invert(POP() != 0); else POP(); }
static void w_oled_flip(void)     { if (s_display_ok) oled_flip(POP() != 0); else POP(); }
static void w_oled_test(void)     { if (s_display_ok) goggle_test_pattern(); }
static void w_oled_splash(void)   { if (s_display_ok) goggle_splash("goggl1", "ESPIDFORTH"); }
static void w_oled_width(void)    { forth_push(SSD1309_WIDTH); }
static void w_oled_height(void)   { forth_push(SSD1309_HEIGHT); }

/* ------------------------------------------------------------------ */
/* Board                                                               */
/* ------------------------------------------------------------------ */

static void w_led(void)     { gpio_set_level(GOGGL_PIN_LED, POP() != 0); }
static void w_button(void)  { forth_push(gpio_get_level(GOGGL_PIN_BUTTON) == 0 ? -1 : 0); }
/* ( n -- r )  0 <= r < n, hardware RNG */
static void w_random(void)  { int n = POP(); forth_push(n > 0 ? (intptr_t)(esp_random() % (uint32_t)n) : 0); }
/* ( n -- level )  read any GPIO as an input with pull-up; -1 for a bad pin */
static void w_gpio_fetch(void)
{
    int n = POP();
    if (n < 0 || n > 21) { forth_push(-1); return; }
    gpio_config_t io = { .pin_bit_mask = 1ULL << n, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE };
    if (n != GOGGL_PIN_SCLK && n != GOGGL_PIN_MOSI && n != GOGGL_PIN_DC && n != GOGGL_PIN_RST && n != GOGGL_PIN_CS && n != GOGGL_PIN_LED)
        gpio_config(&io);
    forth_push(gpio_get_level(n));
}
static void w_reboot(void)  { con_printf("rebooting\n");  vTaskDelay(pdMS_TO_TICKS(100)); esp_restart(); }

/* ------------------------------------------------------------------ */
/* Timing and maths                                                    */
/* ------------------------------------------------------------------ */

static void w_ms(void)
{
    int n = POP();
    if (n > 0) vTaskDelay(pdMS_TO_TICKS(n));
}
static void w_us(void) { forth_push((intptr_t)(uint32_t)esp_timer_get_time()); }

/* Integer-degree sine table would be smaller, but the C3 has no FPU and the
 * demos only call these a handful of times per frame, so libm is fine. */
static void w_sin1k(void)
{
    int deg = POP();
    forth_push((intptr_t)lroundf(sinf((float)deg * (float)M_PI / 180.0f) * 1000.0f));
}
static void w_cos1k(void)
{
    int deg = POP();
    forth_push((intptr_t)lroundf(cosf((float)deg * (float)M_PI / 180.0f) * 1000.0f));
}

/* ------------------------------------------------------------------ */
/* Registration                                                        */
/* ------------------------------------------------------------------ */

void goggle_register_words(void)
{
    static const struct { const char *name; forth_word_fn fn; } words[] = {
        { "oled-clear",     w_oled_clear },
        { "oled-show",      w_oled_show },
        { "oled-color",     w_oled_color },
        { "oled-pixel",     w_oled_pixel },
        { "oled-pixel@",    w_oled_pixel_at },
        { "oled-line",      w_oled_line },
        { "oled-hline",     w_oled_hline },
        { "oled-vline",     w_oled_vline },
        { "oled-frame",     w_oled_frame },
        { "oled-box",       w_oled_box },
        { "oled-rframe",    w_oled_rframe },
        { "oled-rbox",      w_oled_rbox },
        { "oled-circle",    w_oled_circle },
        { "oled-disc",      w_oled_disc },
        { "oled-triangle",  w_oled_triangle },
        { "oled-xbm",       w_oled_xbm },
        { "oled-xbm-solid", w_oled_xbm_solid },

        { "oled-str",       w_oled_str },
        { "oled-glyph",     w_oled_glyph },
        { "oled-num",       w_oled_num },
        { "oled-num02",     w_oled_num02 },
        { "oled-numr",      w_oled_numr },
        { "oled-scale",     w_oled_scale },
        { "oled-scale-xy",  w_oled_scale_xy },
        { "oled-dir",       w_oled_dir },
        { "oled-strw",      w_oled_strw },

        { "oled-on",        w_oled_on },
        { "oled-contrast",  w_oled_contrast },
        { "oled-invert",    w_oled_invert },
        { "oled-flip",      w_oled_flip },
        { "oled-test",      w_oled_test },
        { "oled-splash",    w_oled_splash },
        { "oled-width",     w_oled_width },
        { "oled-height",    w_oled_height },

        { "led",            w_led },
        { "button?",        w_button },
        { "reboot",         w_reboot },
        { "random",         w_random },
        { "gpio@",          w_gpio_fetch },

        { "ms",             w_ms },
        { "us",             w_us },
        { "sin1k",          w_sin1k },
        { "cos1k",          w_cos1k },
    };
    for (size_t i = 0; i < sizeof(words) / sizeof(words[0]); i++) {
        forth_register_word(words[i].name, words[i].fn);
    }
    ESP_LOGI(TAG, "registered %u words", (unsigned)(sizeof(words) / sizeof(words[0])));
}
