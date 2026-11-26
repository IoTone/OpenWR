/*
 * battery_xiao.c (SR-16): see battery.h. On the XIAO ESP32-C5 the cell is
 * read through the board's own 100k/100k divider on GPIO6, powered through a
 * load switch on GPIO26 so it draws nothing between readings. "USB in" is
 * known only when a computer has opened the USB port, so a wall charger reads
 * as unknown and the charging mark is a best guess: USB seen and under 4.15 V.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include "board.h"
#if defined(BOARD_SR16)
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/usb_serial_jtag.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "board.h"
#include "battery.h"

#if PIN_BAT_ADC >= 0
static const char *TAG = "bat";
static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;
static adc_channel_t s_ch;
#endif

void battery_init(void)
{
#if PIN_BAT_ADC >= 0
    adc_unit_t unit;
    if (adc_oneshot_io_to_channel(PIN_BAT_ADC, &unit, &s_ch) != ESP_OK) return;
    adc_oneshot_unit_init_cfg_t u = { .unit_id = unit };
    if (adc_oneshot_new_unit(&u, &s_adc) != ESP_OK) { s_adc = NULL; return; }
    adc_oneshot_chan_cfg_t c = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    adc_oneshot_config_channel(s_adc, s_ch, &c);
    adc_cali_curve_fitting_config_t k = { .unit_id = unit, .chan = s_ch, .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    if (adc_cali_create_scheme_curve_fitting(&k, &s_cali) != ESP_OK) { s_cali = NULL; ESP_LOGW(TAG, "no ADC calibration"); }
    gpio_config_t en = { .pin_bit_mask = 1ULL << PIN_BAT_EN, .mode = GPIO_MODE_OUTPUT };
    gpio_config(&en);
    gpio_set_level(PIN_BAT_EN, 0);
#endif
}

#if PIN_BAT_ADC >= 0
static int pct_of(int mv)
{
    static const int v[] = { 3300, 3500, 3600, 3700, 3750, 3800, 3850, 3900, 4000, 4100, 4200 };
    static const int p[] = {    0,    5,   10,   20,   30,   40,   50,   60,   75,   90,  100 };
    if (mv <= v[0]) return 0;
    for (int i = 1; i < 11; i++)
        if (mv < v[i]) return p[i - 1] + (p[i] - p[i - 1]) * (mv - v[i - 1]) / (v[i] - v[i - 1]);
    return 100;
}
#endif

battery_t battery_read(void)
{
    battery_t b = { .pct = -1, .usb = usb_serial_jtag_is_connected() ? 1 : -1 };
#if PIN_BAT_ADC >= 0
    if (!s_adc) return b;
    gpio_set_level(PIN_BAT_EN, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    int sum = 0, n = 0;
    for (int i = 0; i < 16; i++) {
        int raw, mv;
        if (adc_oneshot_read(s_adc, s_ch, &raw) != ESP_OK) continue;
        if (s_cali && adc_cali_raw_to_voltage(s_cali, raw, &mv) == ESP_OK) sum += mv;
        else sum += raw * 3300 / 4095;
        n++;
    }
    gpio_set_level(PIN_BAT_EN, 0);
    b.mv = n ? 2 * sum / n : 0;
    b.wired = b.mv > 2500 && b.mv < 4600;
    if (b.wired) {
        b.pct = pct_of(b.mv);
        b.full = b.usb == 1 && b.mv >= 4150;
        b.charging = b.usb == 1 && !b.full;
    } else {
        b.mv = 0;
    }
#endif
    return b;
}

const char *battery_text(const battery_t *b, char *buf, int len)
{
    if (!b->wired) snprintf(buf, len, "battery: not wired");
    else snprintf(buf, len, "%d%%%s", b->pct, b->charging ? " charging" : b->full ? " full" : "");
    return buf;
}

int battery_probe(int gpio, bool pulldown) { (void)gpio; (void)pulldown; return -1; }

const char *battery_howto(void)
{
#if PIN_BAT_ADC >= 0
    return "The XIAO reads its own battery pads (GPIO6). If the cell is on the driver board's JST, "
           "bridge the driver board's BAT+ to the XIAO's BAT+ pad.";
#else
    return "The XIAO ESP32-S3 has no battery sense. The C5 has one.";
#endif
}
#endif
