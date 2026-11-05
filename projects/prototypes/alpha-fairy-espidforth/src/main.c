/*
 * main.c — alpha-fairy-espidforth on an M5StickC Plus SE (ESP32-PICO-D4)
 *
 * AlphaFriend: Alpha Fairy's Sony camera code ported onto ESPIDFORTH. The
 * console is on the USB-UART bridge (UART0) and over BLE (Nordic UART), the
 * same ok> prompt either way; Wi-Fi joins the camera's own access point.
 *
 *   s" DIRECT-xxxx:ILCE-7RM3" s" password" wifi!     once; kept in NVS
 *   cam-join  cam-api  56 f!  s" 1/250" ss!  shoot   L0 from the console
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_timer.h"

#include "forth_core.h"
#include "forth_version.h"
#include "console.h"
#include "net_words.h"
#include "ble_console.h"
#include "sony_http.h"
#include "postview.h"
#include "display.h"

#define AF_VERSION      "0.3.0"
#define CONSOLE_UART    UART_NUM_0
#define CONSOLE_BAUD    115200
#define CONSOLE_RX_BUF  256
#define CONSOLE_TX_BUF  512
#define FORTH_HEAP_SIZE (16 * 1024)

/* StickC Plus: the big front button (A) is GPIO37, active low */
#define PIN_BTN_A       GPIO_NUM_37
/* The Plus holds its own power on GPIO4 when running from the battery */
#define PIN_HOLD        GPIO_NUM_4

/* ----- Console: UART0 + BLE ----- */

static void console_install(void)
{
    const uart_config_t cfg = {
        .baud_rate  = CONSOLE_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(CONSOLE_UART, CONSOLE_RX_BUF, CONSOLE_TX_BUF, 0, NULL, 0);
    uart_param_config(CONSOLE_UART, &cfg);
}

static void console_print(const char *s) { uart_write_bytes(CONSOLE_UART, s, strlen(s)); }

static void console_printf(const char *fmt, ...)
{
    char buf[160];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    console_print(buf);
}

static int console_getchar(void)
{
    int c = ble_console_getchar();
    if (c >= 0) return c;
    uint8_t b;
    int n = uart_read_bytes(CONSOLE_UART, &b, 1, pdMS_TO_TICKS(10));
    return n > 0 ? b : -1;
}

static void console_putchar(int c)
{
    char ch = (char)c;
    uart_write_bytes(CONSOLE_UART, &ch, 1);
    ble_console_putchar(c);
}

void con_printf(const char *fmt, ...)
{
    char buf[256];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, args);
    va_end(args);
    if (n < 0) return;
    if (n > (int)sizeof buf - 1) n = sizeof buf - 1;
    for (int i = 0; i < n; i++) {
        if (buf[i] == '\n') console_putchar('\r');
        console_putchar(buf[i]);
    }
}

/* ----- BLE on/off (saved; applies at boot), reboot ----- */

static bool s_ble = true;

static void w_ble_store(void)
{
    int32_t on = forth_pop() ? 1 : 0;
    kv_set_i32("ble", on);
    con_printf("ble %s after reboot\n", on ? "on" : "off");
}

static void w_reboot(void) { esp_restart(); }

/* ----- housekeeping: BLE flush, keep advertising for the glasses ----- */

static void friend_task(void *arg)
{
    int held = 0;
    for (uint32_t t = 0;; t++) {
        for (int i = 0; i < 20; i++) {
            vTaskDelay(pdMS_TO_TICKS(50));
            if (s_ble) ble_console_flush();
            if (i & 1) display_pulse((uint32_t)(esp_timer_get_time() / 1000));
            if (gpio_get_level(PIN_BTN_A) == 0) {
                if (++held == 20) forth_eval("cam-join");     /* hold A 1 s: find the camera again */
            } else {
                held = 0;
            }
        }
        /* The glasses can't bond and have no pairing gesture, so AlphaFriend
         * advertises whenever nobody is connected. */
        if (s_ble) {
            ble_console_poll();
            if (ble_console_state() == 0) ble_pair_window(600);
        }
        display_tick();
        shc_tick();
        if (forth_word_exists("friend-tick")) forth_eval("friend-tick");
    }
}

void app_main(void)
{
    gpio_set_direction(PIN_HOLD, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_HOLD, 1);
    gpio_set_direction(PIN_BTN_A, GPIO_MODE_INPUT);

    console_install();
    vTaskDelay(pdMS_TO_TICKS(300));
    console_print("\r\n\r\n============================================\r\n");
    console_printf("  alpha-fairy-espidforth (OpenWR) v%s\r\n", AF_VERSION);
    console_printf("  ESPIDFORTH v%s  build %s %s\r\n", ESPIDFORTH_VERSION_STRING, ESPIDFORTH_BUILD_DATE, ESPIDFORTH_BUILD_TIME);
    console_print("  AlphaFriend on M5StickC Plus SE; Sony camera code from Alpha Fairy (MIT)\r\n");
    console_print("============================================\r\n");

    bool screen = display_init();                  /* before Wi-Fi: the AXP192 powers the panel */
    if (!screen) console_print("display: not found\r\n");

    int rc = forth_init(FORTH_HEAP_SIZE);
    if (rc != 0) { console_printf("forth_init failed (rc=%d)\r\n", rc); return; }

    esp_err_t nerr = net_init();
    if (nerr != ESP_OK) console_printf("net_init failed: %s\r\n", esp_err_to_name(nerr));
    net_register_words();
    sony_register_words();
    postview_register_words();
    forth_register_word("ble!", w_ble_store);
    forth_register_word("reboot", w_reboot);
    display_register_words();
    int32_t ble = 1;
    kv_get_i32("ble", &ble);
    s_ble = ble != 0;
    if (s_ble) s_ble = ble_console_init("AlphaFriend") == ESP_OK;
    if (s_ble) ble_console_register_words();
    else console_print("ble: off (1 ble! reboot to turn it back on)\r\n");

    xTaskCreate(friend_task, "friend", 4096, NULL, tskIDLE_PRIORITY + 1, NULL);
    if (s_ble) ble_pair_window(600);

    console_print("Words: cam-join cam? cam-api cam-evt shoot af af- rec rec- f! ss! iso! ev!\r\n");
    forth_repl(console_getchar, console_putchar);
    forth_deinit();
}
