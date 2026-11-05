/*
 * thumb_jpg.h — a colour JPEG from RGB565 pixels, for RealReel thumbnails.
 * jpge.cpp / jpge.h are Rich Geldreich's public-domain encoder as carried
 * in espressif/esp32-camera (Apache-2.0); unchanged.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef THUMB_JPG_H
#define THUMB_JPG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Encode w x h RGB565 (native order) at quality 1..100 into out (cap bytes).
 * Returns the JPEG size, or 0 if it failed or did not fit. */
int thumb_jpeg(const uint16_t *rgb565, int w, int h, int quality, uint8_t *out, int cap);

/* Streaming: begin, then h RGB888 lines in order, then end (returns the
 * JPEG size, 0 on failure; frees the encoder). */
void *thumb_stream_begin(int w, int h, int quality, uint8_t *out, int cap);
bool  thumb_stream_line(void *enc, const uint8_t *rgb888);
int   thumb_stream_end(void *enc);

#ifdef __cplusplus
}
#endif
#endif
