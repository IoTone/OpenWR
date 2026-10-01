/*
 * bundle.h — the hosted use case as a stored Forth bundle (design §2, phase 3).
 *
 * A bundle is Forth text. It is captured over the console between the lines
 * `bundle-begin` and `bundle-end`, applied with forth_eval_rollback (all or
 * nothing), stored in NVS on success, and re-applied at every boot.
 *
 * The first lines may carry a manifest as comments:
 *   \ bundle: weather-hud 0.3
 *   \ requires: wx-temp oled-str theme@
 * `requires:` names words that must exist before the bundle is applied.
 *
 * Words:
 *   bundle-begin   ( -- )  capture every following line until `bundle-end`
 *   bundle-apply   ( -- )  re-apply the stored bundle now
 *   bundle-forget  ( -- )  erase the stored bundle (takes effect next boot)
 *   bundle-info    ( -- )  print name, size, and state
 *
 * Licensed under the Apache License, Version 2.0
 */

#ifndef BUNDLE_H
#define BUNDLE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Console hook: call with every received character BEFORE handing it to
 * the REPL. Returns true if the character was consumed by capture mode. */
bool bundle_console_char(int c);

/* Apply the bundle stored in NVS, if any. Returns true if one was applied. */
bool bundle_apply_stored(void);

void bundle_register_words(void);

#ifdef __cplusplus
}
#endif
#endif /* BUNDLE_H */
