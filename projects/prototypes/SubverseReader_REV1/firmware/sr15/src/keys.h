/*
 * keys.h: the CrowPanel's five controls as debounced events.
 * Licensed under the Apache License, Version 2.0
 */
#ifndef SR15_KEYS_H
#define SR15_KEYS_H
#include <stdint.h>

typedef enum { KEY_NONE = 0, KEY_UP, KEY_DOWN, KEY_OK, KEY_MENU, KEY_EXIT, KEY_COUNT } key_t_;
#define KEY_LONG 0x80          /* or'd in: held for a second */

void keys_init(void);
int  keys_get(int timeout_ms); /* next event (key | KEY_LONG), or KEY_NONE on timeout */
void keys_inject(int ev);      /* e.g. the key that woke the pass */
uint32_t keys_down(void);      /* bit k set while key k is held */
uint64_t keys_wake_mask(void); /* the GPIOs that wake the pass from deep sleep */
int  keys_from_wake_mask(uint64_t m);
const char *keys_name(int k);
#endif
