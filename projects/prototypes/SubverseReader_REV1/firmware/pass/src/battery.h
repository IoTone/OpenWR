/*
 * battery.h: the cell and USB power.
 *
 * SR-15 (battery_crowpanel.c): the CrowPanel has no line to the cell, so two
 * optional dividers are read (SR-15 BOM line DIV):
 *   BAT (pad P5, or the cell's + lead) -- 100k -- IO3 (header pin 1) -- 100k -- GND (header pin 7)
 *   VBUS (pad P2)                      -- 100k -- IO9 (header pin 2) -- 100k -- GND   (optional)
 * Without the first divider IO3 reads near 0 V and the pass says the battery
 * isn't wired, rather than guessing.
 *
 * SR-16 (battery_xiao.c): the XIAO ESP32-C5 has its own divider on GPIO6,
 * switched on by GPIO26. It measures the XIAO's battery pads; a cell on the
 * driver board's JST may not reach them (unverified), and then this also
 * reads "not wired". The XIAO ESP32-S3 has no battery sense at all.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef PASS_BATTERY_H
#define PASS_BATTERY_H
#include <stdbool.h>

typedef struct {
    bool wired;       /* a plausible cell voltage is being read */
    int  mv;          /* cell voltage, mV (while charging this is the charge voltage) */
    int  pct;         /* 0..100 from a LiPo discharge curve; -1 if not wired */
    int  usb;         /* 1 USB power present, 0 absent, -1 unknown */
    bool charging;    /* USB present and the cell isn't full yet */
    bool full;        /* USB present and the cell is at 4.15 V or more */
} battery_t;

void battery_init(void);
battery_t battery_read(void);
const char *battery_text(const battery_t *b, char *buf, int len);   /* "87%", "87% charging", "not wired" */
int battery_probe(int gpio, bool pulldown);   /* bench, SR-15: mV on GPIO3, 8 or 9; -1 for others */
const char *battery_howto(void);              /* one line: what to wire if it reads "not wired" */
#endif
