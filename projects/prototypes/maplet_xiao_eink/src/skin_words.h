/*
 * skin_words.h — skin1 vocabulary (see skin_words.c for the word list).
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef SKIN_WORDS_H
#define SKIN_WORDS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void skin_init(void);
void skin_register_words(void);
void skin_splash(void);           /* draw the splash onto the canvas */
bool skin_show_boot(void);        /* show last saved map or the splash; true if a map */
int  skin_safe_x(void);           /* columns of the glass that render (epd-safe!) */
int  skin_safe_w(void);

#ifdef __cplusplus
}
#endif
#endif
