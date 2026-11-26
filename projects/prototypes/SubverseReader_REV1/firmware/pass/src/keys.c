/*
 * keys.c: poll the controls every 10 ms, debounce, and queue a press on
 * release (or a long press once a key has been held for a second). A key that
 * is already down at boot (it woke the pass) is ignored until it is released.
 *
 * Waking: on the ESP32-S3 any RTC pin (GPIO 0-21) can wake the chip, so the
 * keys do it themselves and ext1 says which one. On the ESP32-C5 only LP GPIO
 * 0-6 can; SR-16's three keys share one such line (PIN_KEY_WAKE) through a
 * diode each, and which key it was is read off the pins at boot.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "esp_sleep.h"
#include "soc/soc_caps.h"
#include "board.h"
#include "keys.h"

#define NPINS (KEY_COUNT + 1)                 /* the five keys, then BOOT (reports as KEY_OK) */
static const int s_pin_all[NPINS] = { -1, PIN_KEY_UP, PIN_KEY_DOWN, PIN_KEY_OK, PIN_KEY_MENU, PIN_KEY_EXIT, PIN_KEY_BOOT };
static int s_pin[NPINS];                      /* -1: not fitted */
static const char *s_name[KEY_COUNT] = { "none", "up", "down", "ok", "menu", "exit" };
static QueueHandle_t s_q;
static volatile uint32_t s_down;
static int s_fitted, s_boot_held;

static int key_of(int i) { return i == KEY_COUNT ? KEY_OK : i; }
const char *keys_name(int k) { k &= 0x7F; return (k > 0 && k < KEY_COUNT) ? s_name[k] : "?"; }
uint32_t keys_down(void) { return s_down; }
int keys_fitted(void) { return s_fitted; }

static bool can_wake(int pin)
{
#if SOC_PM_SUPPORT_EXT1_WAKEUP
    return pin >= 0 && rtc_gpio_is_valid_gpio(pin);
#else
    return false;
#endif
}

void keys_arm_wake(void)
{
#if SOC_PM_SUPPORT_EXT1_WAKEUP
    uint64_t m = 0;
    for (int i = 1; i < NPINS; i++) if (can_wake(s_pin[i])) m |= 1ULL << s_pin[i];
#if PIN_KEY_WAKE >= 0
    if (s_fitted && can_wake(PIN_KEY_WAKE)) m |= 1ULL << PIN_KEY_WAKE;
#endif
    if (!m) return;
    for (int pin = 0; pin < 64; pin++) {
        if (!(m & (1ULL << pin))) continue;
        rtc_gpio_pullup_en(pin);
        rtc_gpio_pulldown_dis(pin);
    }
    esp_sleep_enable_ext1_wakeup_io(m, ESP_EXT1_WAKEUP_ANY_LOW);
#endif
}

int keys_woke(void)
{
#if SOC_PM_SUPPORT_EXT1_WAKEUP
    uint64_t m = esp_sleep_get_ext1_wakeup_status();
    for (int i = 1; i < NPINS; i++) if (s_pin[i] >= 0 && (m & (1ULL << s_pin[i]))) return key_of(i);
#endif
    return s_boot_held;                       /* the shared wake line: whichever key is still down */
}

static void keys_task(void *arg)
{
    int held[NPINS] = { 0 };
    bool ignore[NPINS] = { false };
    for (int i = 1; i < NPINS; i++) ignore[i] = s_pin[i] >= 0 && gpio_get_level(s_pin[i]) == 0;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
        uint32_t down = 0;
        for (int i = 1; i < NPINS; i++) {
            if (s_pin[i] < 0) continue;
            int k = key_of(i);
            bool d = gpio_get_level(s_pin[i]) == 0;
            if (d) {
                down |= 1u << k;
                if (ignore[i]) continue;
                if (++held[i] == 100) { int ev = k | KEY_LONG; xQueueSend(s_q, &ev, 0); }
            } else {
                if (!ignore[i] && held[i] >= 3 && held[i] < 100) { int ev = k; xQueueSend(s_q, &ev, 0); }
                held[i] = 0;
                ignore[i] = false;
            }
        }
        s_down = down;
    }
}

void keys_init(int fitted)
{
    s_fitted = fitted;
    uint64_t m = 0;
    for (int i = 1; i < NPINS; i++) {
        bool on = i == KEY_COUNT                       /* BOOT: always */
               || (fitted >= 5)
               || (fitted >= 3 && i <= KEY_OK);
        s_pin[i] = on ? s_pin_all[i] : -1;
        if (s_pin[i] >= 0) m |= 1ULL << s_pin[i];
    }
    if (m) {
        gpio_config_t c = { .pin_bit_mask = m, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE };
        gpio_config(&c);
    }
    for (int i = 1; i < NPINS && !s_boot_held; i++)
        if (s_pin[i] >= 0 && gpio_get_level(s_pin[i]) == 0) s_boot_held = key_of(i);
    s_q = xQueueCreate(16, sizeof(int));
    xTaskCreate(keys_task, "keys", 2560, NULL, 5, NULL);
}

void keys_inject(int ev) { xQueueSend(s_q, &ev, 0); }

int keys_get(int timeout_ms)
{
    int ev;
    if (xQueueReceive(s_q, &ev, timeout_ms < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms)) == pdTRUE) return ev;
    return KEY_NONE;
}
