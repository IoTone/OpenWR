/*
 * console.h — printf for words and modules whose output must reach every
 * console (USB and BLE). ESP_LOG* stays on USB only, on purpose.
 */
#ifndef CONSOLE_H
#define CONSOLE_H
#ifdef __cplusplus
extern "C" {
#endif
void con_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#ifdef __cplusplus
}
#endif
#endif
