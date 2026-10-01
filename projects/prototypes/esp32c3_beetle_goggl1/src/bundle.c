/*
 * bundle.c — capture, verify, apply, and persist a Forth use-case bundle.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "esp_log.h"
#include "nvs.h"

#include "forth_core.h"
#include "net_words.h"
#include "bundle.h"

static const char *TAG = "bundle";

#define BUNDLE_MAX   (12 * 1024)       /* NVS has 24 KB; leave room for the rest */
#define NVS_KEY      "bundle"
#define END_MARK     "bundle-end"

/* ------------------------------------------------------------------ */
/* Manifest                                                            */
/* ------------------------------------------------------------------ */

/* Copy the value after "\ <key>:" on any of the first few lines. */
static bool manifest_field(const char *text, const char *key, char *out, size_t cap)
{
    char pat[32];
    snprintf(pat, sizeof pat, "\\ %s:", key);
    const char *p = text;
    for (int line = 0; p && *p && line < 8; line++) {
        while (*p == ' ') p++;
        if (!strncmp(p, pat, strlen(pat))) {
            p += strlen(pat);
            while (*p == ' ') p++;
            size_t n = strcspn(p, "\r\n");
            if (n >= cap) n = cap - 1;
            memcpy(out, p, n); out[n] = '\0';
            return true;
        }
        p = strchr(p, '\n');
        if (p) p++;
    }
    return false;
}

/* Every word listed in `requires:` must already exist. */
static bool requirements_met(const char *text, char *missing, size_t cap)
{
    char req[200];
    if (!manifest_field(text, "requires", req, sizeof req)) return true;
    char *save = NULL;
    for (char *w = strtok_r(req, " \t", &save); w; w = strtok_r(NULL, " \t", &save)) {
        if (!forth_word_exists(w)) { strncpy(missing, w, cap - 1); missing[cap - 1] = '\0'; return false; }
    }
    return true;
}

/* ------------------------------------------------------------------ */
/* Apply                                                               */
/* ------------------------------------------------------------------ */

static bool apply_text(const char *text, size_t len, const char *what)
{
    char missing[64] = "";
    if (!requirements_met(text, missing, sizeof missing)) {
        printf("bundle: %s needs word '%s' which this firmware lacks\n", what, missing);
        return false;
    }
    char fail[160] = "";
    int rc = forth_eval_rollback(text, len, fail, sizeof fail);
    if (rc == 0) {
        char name[48] = "(unnamed)";
        manifest_field(text, "bundle", name, sizeof name);
        printf("bundle: %s applied: %s, %u bytes\n", what, name, (unsigned)len);
        return true;
    }
    if (rc == 1) printf("bundle: %s rejected, rolled back. Failing line:\n  %s\n", what, fail);
    else         printf("bundle: %s could not be evaluated (rc=%d)\n", what, rc);
    return false;
}

bool bundle_apply_stored(void)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READONLY, &h) != ESP_OK) return false;
    size_t len = 0;
    if (nvs_get_blob(h, NVS_KEY, NULL, &len) != ESP_OK || len == 0 || len > BUNDLE_MAX) {
        nvs_close(h);
        return false;
    }
    char *text = malloc(len + 1);
    if (!text) { nvs_close(h); return false; }
    nvs_get_blob(h, NVS_KEY, text, &len);
    nvs_close(h);
    text[len] = '\0';
    bool ok = apply_text(text, len, "stored");
    free(text);
    return ok;
}

static void store(const char *text, size_t len)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READWRITE, &h) != ESP_OK) return;
    esp_err_t err = nvs_set_blob(h, NVS_KEY, text, len);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) printf("bundle: stored copy failed: %s\n", esp_err_to_name(err));
    else               printf("bundle: stored, will re-apply at boot\n");
}

/* ------------------------------------------------------------------ */
/* Capture                                                             */
/* ------------------------------------------------------------------ */

static char  *s_buf;
static size_t s_len;
static size_t s_line_start;
static int    s_lines;
static bool   s_overflow;

static void capture_begin(void)
{
    if (s_buf) return;
    s_buf = malloc(BUNDLE_MAX + 1);
    if (!s_buf) { printf("bundle: no memory for capture\n"); return; }
    s_len = 0; s_line_start = 0; s_lines = 0; s_overflow = false;
    printf("bundle: capturing until a line reading '%s'\n", END_MARK);
}

static void capture_finish(void)
{
    char *text = s_buf;
    size_t len = s_len;
    s_buf = NULL;
    if (s_overflow) {
        printf("bundle: too large (limit %u bytes), discarded\n", (unsigned)BUNDLE_MAX);
    } else {
        text[len] = '\0';
        printf("bundle: %d lines, %u bytes\n", s_lines, (unsigned)len);
        if (apply_text(text, len, "new")) store(text, len);
    }
    free(text);
    printf("ok> ");
    fflush(stdout);
}

bool bundle_console_char(int c)
{
    if (!s_buf) return false;
    if (c == '\n') return true;                      /* \r is the line end */
    if (c == '\r') {
        /* is this line the end marker? */
        size_t n = s_len - s_line_start;
        if (n == strlen(END_MARK) && !strncmp(s_buf + s_line_start, END_MARK, n)) {
            s_len = s_line_start;                    /* drop the marker itself */
            capture_finish();
            return true;
        }
        if (s_len < BUNDLE_MAX) s_buf[s_len++] = '\n';
        s_line_start = s_len;
        s_lines++;
        if ((s_lines % 10) == 0) { putchar('.'); fflush(stdout); }
        return true;
    }
    if (s_len < BUNDLE_MAX) s_buf[s_len++] = (char)c;
    else s_overflow = true;
    return true;
}

/* ------------------------------------------------------------------ */
/* Words                                                               */
/* ------------------------------------------------------------------ */

static void w_bundle_begin(void)  { capture_begin(); }
static void w_bundle_apply(void)  { if (!bundle_apply_stored()) printf("bundle: nothing stored\n"); }

static void w_bundle_forget(void)
{
    nvs_handle_t h;
    if (nvs_open(GOGGL_NVS_NS, NVS_READWRITE, &h) == ESP_OK) {
        nvs_erase_key(h, NVS_KEY); nvs_commit(h); nvs_close(h);
    }
    printf("bundle: erased; the current definitions stay until reboot\n");
}

static void w_bundle_info(void)
{
    nvs_handle_t h;
    size_t len = 0;
    if (nvs_open(GOGGL_NVS_NS, NVS_READONLY, &h) == ESP_OK) {
        nvs_get_blob(h, NVS_KEY, NULL, &len);
        if (len && len <= BUNDLE_MAX) {
            char *text = malloc(len + 1);
            if (text) {
                nvs_get_blob(h, NVS_KEY, text, &len); text[len] = '\0';
                char name[48] = "(unnamed)", req[200] = "";
                manifest_field(text, "bundle", name, sizeof name);
                manifest_field(text, "requires", req, sizeof req);
                printf("bundle: %s, %u bytes stored\n  requires: %s\n", name, (unsigned)len, req);
                free(text);
            }
        } else printf("bundle: nothing stored\n");
        nvs_close(h);
    }
    printf("  hud-tick %s defined\n", forth_word_exists("hud-tick") ? "is" : "is not");
}

void bundle_register_words(void)
{
    forth_register_word("bundle-begin",  w_bundle_begin);
    forth_register_word("bundle-apply",  w_bundle_apply);
    forth_register_word("bundle-forget", w_bundle_forget);
    forth_register_word("bundle-info",   w_bundle_info);
    (void)TAG;
}
