/*
 * hub_reel.h — the shots, newest last, on a FAT partition in flash
 * (/reel/0001.JPG + 0001.TXT, about 5.9 MB: a few hundred medium JPEGs),
 * and the screen that shows one of them.
 *
 * Screen (320x172): the picture fitted into the left 258x172, a strip on
 * the right with its number, camera, time and size. The oldest shots go
 * when the partition is nearly full.
 *
 *   reel?       ( -- )      reel <count> free <KB> showing <n>
 *   reel-ls     ( -- )      one line per shot: <n> <label> <w>x<h> <time> <bytes>
 *   n reel-show ( n -- )    show shot n (1 = oldest)
 *   reel-wipe   ( -- )      delete every shot
 *   show?       ( -- )      show <file> <camera> <i>/<count> | show none 0/0
 *
 * Whenever the shot on screen changes (a swipe, a new shot, reel-show) the
 * hub prints that show line on every console, unasked: the glasses follow it.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef HUB_REEL_H
#define HUB_REEL_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "esp_err.h"

#define PIC_W   258                  /* the picture box; the strip is the rest */
#define STRIP_X PIC_W

esp_err_t reel_init(void);
int  reel_count(void);
int  reel_showing(void);             /* index into the reel, 0 = oldest; -1 none */
/* save a JPEG and show it; label like "SCOUT", "A7", "SPECS" */
int  reel_add(const uint8_t *jpg, size_t n, const char *label);
void reel_show(int i);
void reel_step(int dir);             /* -1 older, +1 newer */
void reel_redraw(void);
void reel_strip_note(const char *line1, const char *line2, uint16_t colour);   /* status lines in the strip */
bool reel_meta(int num, char *label, int *w, int *h, char *at, int *bytes);
int  reel_num_at(int i);             /* file number of shot i, -1 if none */
void reel_file(int num, char *out, size_t cap);
void reel_register_words(void);

#endif
