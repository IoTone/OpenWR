/*
 * keys.h: the pass's controls as debounced events.
 *
 * SR-15 has five (dial left/right/push, MENU, EXIT). SR-16 has three keys
 * (PREV = KEY_UP, NEXT = KEY_DOWN, MENU = KEY_OK), or none; either way the
 * XIAO's own BOOT button is one more KEY_OK.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef PASS_KEYS_H
#define PASS_KEYS_H
#include <stdint.h>
#include <stdbool.h>

typedef enum { KEY_NONE = 0, KEY_UP, KEY_DOWN, KEY_OK, KEY_MENU, KEY_EXIT, KEY_COUNT } key_t_;
#define KEY_LONG 0x80          /* or'd in: held for a second */

void keys_init(int fitted);    /* how many keys are wired: 5, 3, or 0 (BOOT only) */
int  keys_fitted(void);
int  keys_get(int timeout_ms); /* next event (key | KEY_LONG), or KEY_NONE on timeout */
void keys_inject(int ev);      /* e.g. the key that woke the pass */
uint32_t keys_down(void);      /* bit k set while key k is held */
void keys_arm_wake(void);      /* before deep sleep: any fitted control that can, wakes the pass */
int  keys_woke(void);          /* after a key wake: which key, or KEY_NONE if it can't be told */
const char *keys_name(int k);
#endif
