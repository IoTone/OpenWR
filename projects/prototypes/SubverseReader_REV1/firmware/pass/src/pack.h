/*
 * pack.h: read .svpk packs (format v0, see tools/svpk.py) straight off the card.
 * Licensed under the Apache License, Version 2.0
 */
#ifndef SR15_PACK_H
#define SR15_PACK_H
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char path[96];
    char title[64];
    char author[64];
    uint16_t w, h;
    uint8_t bpp;
    uint32_t pages;
    uint32_t table_off;
} pack_t;

/* Fill p from the file's header. false (with a reason in err) if it isn't a pack this pass can show. */
bool pack_open(pack_t *p, const char *path, char *err, int errlen);
/* Decode page n (0-based) into a canvas-format buffer (1 = black). */
bool pack_page(const pack_t *p, uint32_t n, uint8_t *out, int outlen);
/* The packs under dir, sorted by file name: up to max paths of 96 bytes. */
int pack_scan(const char *dir, char (*paths)[96], int max);
#endif
