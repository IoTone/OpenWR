/*
 * keys.c: poll the five controls every 10 ms, debounce, and queue a press on
 * release (or a long press once a key has been held for a second). A key that
 * is already down at boot (it woke the pass) is ignored until it is released.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "pins.h"
#include "keys.h"

static const int s_pin[KEY_COUNT] = { -1, PIN_KEY_UP, PIN_KEY_DOWN, PIN_KEY_OK, PIN_KEY_MENU, PIN_KEY_EXIT };
static const char *s_name[KEY_COUNT] = { "none", "up", "down", "ok", "menu", "exit" };
static QueueHandle_t s_q;
static volatile uint32_t s_down;

const char *keys_name(int k) { k &= 0x7F; return (k > 0 && k < KEY_COUNT) ? s_name[k] : "?"; }
uint32_t keys_down(void) { return s_down; }

uint64_t keys_wake_mask(void)
{
    uint64_t m = 0;
    for (int k = 1; k < KEY_COUNT; k++) m |= 1ULL << s_pin[k];
    return m;
}

int keys_from_wake_mask(uint64_t m)
{
    for (int k = 1; k < KEY_COUNT; k++) if (m & (1ULL << s_pin[k])) return k;
    return KEY_NONE;
}

static void keys_task(void *arg)
{
    int held[KEY_COUNT] = { 0 };
    bool ignore[KEY_COUNT] = { false };
    for (int k = 1; k < KEY_COUNT; k++) ignore[k] = gpio_get_level(s_pin[k]) == 0;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(10));
        uint32_t down = 0;
        for (int k = 1; k < KEY_COUNT; k++) {
            bool d = gpio_get_level(s_pin[k]) == 0;
            if (d) {
                down |= 1u << k;
                if (ignore[k]) continue;
                if (++held[k] == 100) { int ev = k | KEY_LONG; xQueueSend(s_q, &ev, 0); }
            } else {
                if (!ignore[k] && held[k] >= 3 && held[k] < 100) { int ev = k; xQueueSend(s_q, &ev, 0); }
                held[k] = 0;
                ignore[k] = false;
            }
        }
        s_down = down;
    }
}

void keys_init(void)
{
    uint64_t m = keys_wake_mask();
    gpio_config_t c = { .pin_bit_mask = m, .mode = GPIO_MODE_INPUT, .pull_up_en = GPIO_PULLUP_ENABLE };
    gpio_config(&c);
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
