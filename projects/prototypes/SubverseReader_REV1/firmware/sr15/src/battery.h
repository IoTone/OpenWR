/*
 * battery.h: the cell and USB power, read through two optional dividers the
 * CrowPanel doesn't have from the factory (SR-15 BOM line DIV):
 *
 *   BAT (pad P5, or the cell's + lead) -- 100k -- IO3 (header pin 1) -- 100k -- GND (header pin 7)
 *   VBUS (pad P2)                      -- 100k -- IO9 (header pin 2) -- 100k -- GND   (optional)
 *
 * Without the first divider IO3 reads near 0 V and the pass says the battery
 * isn't wired, rather than guessing. Licensed under the Apache License, Version 2.0
 */
#ifndef SR15_BATTERY_H
#define SR15_BATTERY_H
#include <stdbool.h>

typedef struct {
    bool wired;       /* the IO3 divider is fitted (reads a plausible cell voltage) */
    int  mv;          /* cell voltage, mV (while charging this is the charge voltage) */
    int  pct;         /* 0..100 from a LiPo discharge curve; -1 if not wired */
    int  usb;         /* 1 USB power present, 0 absent, -1 unknown (no IO9 divider) */
    bool charging;    /* USB present and the cell isn't full yet */
    bool full;        /* USB present and the cell is at 4.15 V or more */
} battery_t;

void battery_init(void);
battery_t battery_read(void);
const char *battery_text(const battery_t *b, char *buf, int len);   /* "87%", "87% charging", "not wired" */
int battery_probe(int gpio, bool pulldown);   /* bench: mV on GPIO3, 8 or 9; -1 for others */
#endif
