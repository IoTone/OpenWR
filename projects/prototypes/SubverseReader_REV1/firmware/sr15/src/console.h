/* console.h: printf to the USB-C console (UART0 via the CH340). Apache-2.0 */
#ifndef SR15_CONSOLE_H
#define SR15_CONSOLE_H
#include <stdint.h>
int con_read(uint8_t *buf, int n, int ms);   /* raw bytes from USB or Bluetooth; 0 on timeout */
void con_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#endif
