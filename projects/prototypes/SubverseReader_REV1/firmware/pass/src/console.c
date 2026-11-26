/*
 * console.c: one console on two links. Input comes from the USB port or
 * Bluetooth (NUS), output goes to both.
 *
 * SR-15's USB-C goes through a CH340 to UART0. SR-16's is the chip's own
 * USB serial port; writes to it are skipped when no computer is listening,
 * because they would otherwise block the prompt for good.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdarg.h>
#include "freertos/FreeRTOS.h"
#include "esp_timer.h"
#include "board.h"
#include "console.h"
#include "ble_console.h"
#include "reader.h"

#if BOARD_CONSOLE_UART
#include "driver/uart.h"
/* big RX: sd-recv streams packs while FAT writes stall */
void con_init(void) { uart_driver_install(UART_NUM_0, 16384, 1024, 0, NULL, 0); }
static void usb_put(uint8_t ch) { uart_write_bytes(UART_NUM_0, &ch, 1); }
static int usb_read(uint8_t *buf, int n, int ms) { return uart_read_bytes(UART_NUM_0, buf, n, pdMS_TO_TICKS(ms)); }
#else
#include "driver/usb_serial_jtag.h"
void con_init(void)
{
    usb_serial_jtag_driver_config_t cfg = { .tx_buffer_size = 1024, .rx_buffer_size = 16384 };
    usb_serial_jtag_driver_install(&cfg);
}
static void usb_put(uint8_t ch) { if (usb_serial_jtag_is_connected()) usb_serial_jtag_write_bytes(&ch, 1, pdMS_TO_TICKS(2)); }
static int usb_read(uint8_t *buf, int n, int ms) { return usb_serial_jtag_read_bytes(buf, n, pdMS_TO_TICKS(ms)); }
#endif

void con_put(int c)
{
    usb_put((uint8_t)c);
    ble_console_putchar(c);
}

int con_get(void)
{
    int c = ble_console_getchar();
    if (c >= 0) { reader_activity(); return c; }
    uint8_t b;
    if (usb_read(&b, 1, 10) <= 0) return -1;
    reader_activity();
    return b;
}

int con_read(uint8_t *buf, int n, int ms)
{
    int64_t until = esp_timer_get_time() + (int64_t)ms * 1000;
    do {
        int got = 0, c;
        while (got < n && (c = ble_console_getchar()) >= 0) buf[got++] = (uint8_t)c;
        if (got) return got;
        got = usb_read(buf, n, 10);
        if (got > 0) return got;
    } while (esp_timer_get_time() < until);
    return 0;
}

void con_printf(const char *fmt, ...)
{
    char buf[256];
    va_list a;
    va_start(a, fmt);
    int n = vsnprintf(buf, sizeof buf, fmt, a);
    va_end(a);
    if (n < 0) return;
    if (n > (int)sizeof buf - 1) n = sizeof buf - 1;
    for (int i = 0; i < n; i++) { if (buf[i] == '\n') con_put('\r'); con_put(buf[i]); }
}
