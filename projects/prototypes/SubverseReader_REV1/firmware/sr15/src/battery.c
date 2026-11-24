/*
 * battery.c: see battery.h. Both dividers halve the voltage (100k/100k), so a
 * 4.2 V cell reads 2.1 V and 5 V USB 2.5 V on the ADC at 12 dB attenuation.
 * The 4054 charger's own status pin isn't wired on this board, so "charging"
 * is inferred: USB present and the cell below 4.15 V.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "battery.h"

static const char *TAG = "bat";
#define CH_BAT  ADC_CHANNEL_2      /* GPIO3 */
#define CH_USB  ADC_CHANNEL_8      /* GPIO9 */

static adc_oneshot_unit_handle_t s_adc;
static adc_cali_handle_t s_cali;

void battery_init(void)
{
    adc_oneshot_unit_init_cfg_t u = { .unit_id = ADC_UNIT_1 };
    if (adc_oneshot_new_unit(&u, &s_adc) != ESP_OK) { s_adc = NULL; return; }
    adc_oneshot_chan_cfg_t c = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    adc_oneshot_config_channel(s_adc, CH_BAT, &c);
    adc_oneshot_config_channel(s_adc, CH_USB, &c);
    adc_cali_curve_fitting_config_t k = { .unit_id = ADC_UNIT_1, .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    if (adc_cali_create_scheme_curve_fitting(&k, &s_cali) != ESP_OK) { s_cali = NULL; ESP_LOGW(TAG, "no ADC calibration"); }
}

static int read_mv(adc_channel_t ch)
{
    if (!s_adc) return 0;
    int sum = 0, n = 0;
    for (int i = 0; i < 16; i++) {
        int raw, mv;
        if (adc_oneshot_read(s_adc, ch, &raw) != ESP_OK) continue;
        if (s_cali && adc_cali_raw_to_voltage(s_cali, raw, &mv) == ESP_OK) sum += mv;
        else sum += raw * 3100 / 4095;
        n++;
    }
    return n ? sum / n : 0;
}

/* Resting LiPo voltage to charge, coarse but honest to a few percent. */
static int pct_of(int mv)
{
    static const int v[] = { 3300, 3500, 3600, 3700, 3750, 3800, 3850, 3900, 4000, 4100, 4200 };
    static const int p[] = {    0,    5,   10,   20,   30,   40,   50,   60,   75,   90,  100 };
    if (mv <= v[0]) return 0;
    for (int i = 1; i < 11; i++)
        if (mv < v[i]) return p[i - 1] + (p[i] - p[i - 1]) * (mv - v[i - 1]) / (v[i] - v[i - 1]);
    return 100;
}

/* Is anything driving the pin? With the internal pull-down (~45k) on, an
 * open pin on this board reads 0.3-0.4 V (measured, IO3/8/9); a 100k/100k
 * divider on a 3.3 V+ cell reads 0.78 V or more, on 5 V USB about 1.2 V. */
static bool driven(int gpio, adc_channel_t ch)
{
    rtc_gpio_pulldown_en(gpio);              /* an ADC pin sits in the RTC mux */
    vTaskDelay(pdMS_TO_TICKS(10));
    int mv = read_mv(ch);
    rtc_gpio_pulldown_dis(gpio);
    vTaskDelay(pdMS_TO_TICKS(5));
    return mv > 650;
}

battery_t battery_read(void)
{
    battery_t b = { .pct = -1, .usb = -1 };
    bool bat = driven(3, CH_BAT);
    b.mv = bat ? read_mv(CH_BAT) * 2 : 0;
    b.wired = bat && b.mv > 2500 && b.mv < 4600;
    /* USB: a divider on a live VBUS holds IO9 up; with USB out the divider
     * pulls it to 0, which looks the same as no divider: so only "in" is sure. */
    if (driven(9, CH_USB) && read_mv(CH_USB) * 2 > 4000) b.usb = 1;
    if (b.wired) {
        b.pct = pct_of(b.mv);
        b.full = b.usb == 1 && b.mv >= 4150;
        b.charging = b.usb == 1 && !b.full;
    }
    return b;
}

const char *battery_text(const battery_t *b, char *buf, int len)
{
    if (!b->wired) snprintf(buf, len, "battery: not wired");
    else snprintf(buf, len, "%d%%%s", b->pct, b->charging ? " charging" : b->full ? " full" : "");
    return buf;
}

/* Bench probe: mV on a spare ADC1 pin (GPIO3, 8 or 9), optionally with the pull-down on. */
int battery_probe(int gpio, bool pulldown)
{
    if (!(gpio == 3 || gpio == 8 || gpio == 9) || !s_adc) return -1;   /* the spare pins only: 10 is the card's CS */
    adc_channel_t ch = (adc_channel_t)(gpio - 1);
    adc_oneshot_chan_cfg_t c = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };
    adc_oneshot_config_channel(s_adc, ch, &c);
    if (pulldown) rtc_gpio_pulldown_en(gpio);
    vTaskDelay(pdMS_TO_TICKS(10));
    int mv = read_mv(ch);
    if (pulldown) rtc_gpio_pulldown_dis(gpio);
    return mv;
}
