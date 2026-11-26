/* console.h: the pass's console, on USB and Bluetooth at once (console.c). Apache-2.0 */
#ifndef PASS_CONSOLE_H
#define PASS_CONSOLE_H
#include <stdint.h>
void con_init(void);
void con_put(int c);                         /* to USB and Bluetooth */
int  con_get(void);                          /* from either; -1 if nothing within ~10 ms */
int  con_read(uint8_t *buf, int n, int ms);  /* raw bytes from USB or Bluetooth; 0 on timeout */
void con_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#endif
