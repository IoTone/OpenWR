/*
 * ota_mnpkg.c — .mnpkg download, verify, stage.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "mbedtls/sha256.h"
#include "monocypher-ed25519.h"

#include "console.h"
#include "net_words.h"
#include "ota_words.h"
#include "mnpkg_keys.h"
#include "ota_mnpkg.h"

static const char *TAG = "mnpkg";

#define HDR_LEN     256
#define SIGNED_LEN  192
#define SIG_P256    1
#define SIG_ED25519 2
#define FMT_ESP_IDF 1
#define FLAG_REQUIRES_REPARTITION 0x01
#define FLAG_CLEARS_BUNDLES       0x02

typedef struct {
    uint8_t  sig_alg, image_format, flags;
    uint16_t chip, board;
    uint8_t  version[8], min_running[8];
    uint32_t image_len;
    uint8_t  image_sha256[32];
    char     variant[17], fw_version[33];
    uint8_t  key_id[8];
    uint8_t  signature[64];
} mnpkg_hdr_t;

static const struct {
    uint8_t alg, len;
    uint8_t id[8];
    uint8_t pub[65];
} s_keys[] = {
    { MN_PKG_KEY0_ALG, MN_PKG_KEY0_LEN, MN_PKG_KEY0_ID, MN_PKG_KEY0_PUB },
#ifdef MN_PKG_KEY1_ALG
    { MN_PKG_KEY1_ALG, MN_PKG_KEY1_LEN, MN_PKG_KEY1_ID, MN_PKG_KEY1_PUB },
#endif
};

const char *mnpkg_err_name(int code)
{
    switch (code) {
        case MNPKG_OK: return "OK";
        case MNPKG_E_FORMAT: return "E_PKG_FORMAT";
        case MNPKG_E_KEY: return "E_PKG_KEY";
        case MNPKG_E_SIG: return "E_PKG_SIG";
        case MNPKG_E_TARGET: return "E_PKG_TARGET";
        case MNPKG_E_REPARTITION: return "E_PKG_REPARTITION";
        case MNPKG_E_SIZE: return "E_PKG_SIZE";
        case MNPKG_E_SHA: return "E_PKG_SHA";
        case MNPKG_E_IMAGE: return "E_PKG_IMAGE";
        case MNPKG_E_MIN: return "E_PKG_MIN";
        case MNPKG_E_NET: return "E_NET";
        case MNPKG_E_FLASH: return "E_FLASH";
        case MNPKG_E_HTTP: return "E_HTTP";
        default: return "E_?";
    }
}

static uint16_t be16(const uint8_t *p) { return (uint16_t)(p[0] << 8 | p[1]); }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }

/* version bytes: major u8, minor u8, revision u16, build u32, all big-endian */
static void fmt_ver(const uint8_t *v, char *out, size_t cap)
{
    snprintf(out, cap, "%u.%u.%u+%lu", v[0], v[1], be16(v + 2), (unsigned long)be32(v + 4));
}

static int cmp_ver(const uint8_t *a, const uint8_t *b)
{
    for (int i = 0; i < 8; i++) if (a[i] != b[i]) return a[i] < b[i] ? -1 : 1;
    return 0;
}

static void running_version(uint8_t out[8])
{
    unsigned maj = 0, min = 0, rev = 0;
    sscanf(GOGGL1_VERSION, "%u.%u.%u", &maj, &min, &rev);
    memset(out, 0, 8);
    out[0] = (uint8_t)maj; out[1] = (uint8_t)min; out[2] = (uint8_t)(rev >> 8); out[3] = (uint8_t)rev;
}

/* Checks 1..5 need only the header. */
static int check_header(const uint8_t *h, mnpkg_hdr_t *d)
{
    if (memcmp(h, "MNPK", 4) != 0 || h[4] != 1 || be16(h + 6) != HDR_LEN) return MNPKG_E_FORMAT;
    d->sig_alg = h[5];
    if (d->sig_alg != SIG_P256 && d->sig_alg != SIG_ED25519) return MNPKG_E_FORMAT;
    d->chip = be16(h + 8); d->board = be16(h + 10);
    d->image_format = h[12]; d->flags = h[13];
    if (d->flags & ~(FLAG_REQUIRES_REPARTITION | FLAG_CLEARS_BUNDLES)) return MNPKG_E_FORMAT;
    if (h[14] || h[15]) return MNPKG_E_FORMAT;
    for (int i = 132; i < 192; i++) if (h[i]) return MNPKG_E_FORMAT;
    memcpy(d->version, h + 16, 8);
    memcpy(d->min_running, h + 24, 8);
    d->image_len = be32(h + 32);
    memcpy(d->image_sha256, h + 36, 32);
    memcpy(d->variant, h + 76, 16); d->variant[16] = '\0';
    memcpy(d->fw_version, h + 92, 32); d->fw_version[32] = '\0';
    memcpy(d->key_id, h + 124, 8);
    memcpy(d->signature, h + 192, 64);

    /* 2: key */
    int k = -1;
    for (size_t i = 0; i < sizeof s_keys / sizeof s_keys[0]; i++)
        if (memcmp(s_keys[i].id, d->key_id, 8) == 0 && s_keys[i].alg == d->sig_alg) k = (int)i;
    if (k < 0) return MNPKG_E_KEY;

    /* 3: signature over bytes 0..191 */
    if (d->sig_alg == SIG_ED25519) {
        if (crypto_ed25519_check(d->signature, s_keys[k].pub, h, SIGNED_LEN) != 0) return MNPKG_E_SIG;
    } else {
        /* P-256 is SHOULD in the spec; this firmware carries Ed25519 keys only. */
        return MNPKG_E_KEY;
    }

    /* 4: target */
    if (d->chip != MNPKG_CHIP_ESP32C3) return MNPKG_E_TARGET;
    if (d->board != 0 && d->board != MNPKG_BOARD_BEETLE_ESP32C3) return MNPKG_E_TARGET;
    if (d->image_format != FMT_ESP_IDF) return MNPKG_E_TARGET;

    /* 5: repartition */
    if (d->flags & FLAG_REQUIRES_REPARTITION) return MNPKG_E_REPARTITION;
    return MNPKG_OK;
}

/* ------------------------------------------------------------------ */

typedef struct {
    uint8_t hdr[HDR_LEN];
    size_t  hdr_got;
    bool    hdr_ok;
    int     err;
    mnpkg_hdr_t d;
    const esp_partition_t *part;
    esp_ota_handle_t ota;
    bool    ota_open;
    mbedtls_sha256_context sha;
    uint32_t written;
    int     last_pct;
} dl_t;

static esp_err_t on_data(esp_http_client_event_t *e)
{
    if (e->event_id != HTTP_EVENT_ON_DATA) return ESP_OK;
    dl_t *s = (dl_t *)e->user_data;
    if (s->err) return ESP_FAIL;
    const uint8_t *p = (const uint8_t *)e->data;
    size_t n = (size_t)e->data_len;

    /* collect the header first */
    if (s->hdr_got < HDR_LEN) {
        size_t take = HDR_LEN - s->hdr_got < n ? HDR_LEN - s->hdr_got : n;
        memcpy(s->hdr + s->hdr_got, p, take);
        s->hdr_got += take; p += take; n -= take;
        if (s->hdr_got < HDR_LEN) return ESP_OK;

        s->err = check_header(s->hdr, &s->d);
        if (s->err) return ESP_FAIL;
        /* 6: size */
        s->part = esp_ota_get_next_update_partition(NULL);
        if (!s->part || s->d.image_len == 0 || s->d.image_len > s->part->size) { s->err = MNPKG_E_SIZE; return ESP_FAIL; }
        if (esp_ota_begin(s->part, s->d.image_len, &s->ota) != ESP_OK) { s->err = MNPKG_E_FLASH; return ESP_FAIL; }
        s->ota_open = true;
        s->hdr_ok = true;
        char v[32]; fmt_ver(s->d.version, v, sizeof v);
        con_printf("mnpkg: %s %s (%s) %lu bytes, key %02x%02x..: header ok, writing %s\n",
                   s->d.fw_version, v, s->d.variant, (unsigned long)s->d.image_len,
                   s->d.key_id[0], s->d.key_id[1], s->part->label);
    }
    if (n == 0) return ESP_OK;
    if (s->written + n > s->d.image_len) { s->err = MNPKG_E_SIZE; return ESP_FAIL; }
    if (esp_ota_write(s->ota, p, n) != ESP_OK) { s->err = MNPKG_E_FLASH; return ESP_FAIL; }
    mbedtls_sha256_update(&s->sha, p, n);
    s->written += n;
    int pct = (int)((uint64_t)s->written * 100 / s->d.image_len);
    if (pct / 10 != s->last_pct / 10) { con_printf("mnpkg: %d%%\n", pct); s->last_pct = pct; }
    return ESP_OK;
}

int mnpkg_apply_url(const char *url, bool reboot)
{
    if (!net_ok()) return MNPKG_E_NET;
    dl_t *s = calloc(1, sizeof *s);
    if (!s) return MNPKG_E_FLASH;
    s->last_pct = -1;
    mbedtls_sha256_init(&s->sha);
    mbedtls_sha256_starts(&s->sha, 0);

    esp_http_client_config_t cfg = {
        .url = url, .event_handler = on_data, .user_data = s,
        .timeout_ms = 15000, .crt_bundle_attach = esp_crt_bundle_attach,
        .buffer_size = 2048, .keep_alive_enable = true,
    };
    con_printf("mnpkg: fetching %s\n", url);
    esp_http_client_handle_t c = esp_http_client_init(&cfg);
    int rc = MNPKG_OK;
    if (!c) { rc = MNPKG_E_HTTP; goto out; }
    esp_err_t err = esp_http_client_perform(c);
    int status = esp_http_client_get_status_code(c);
    esp_http_client_cleanup(c);
    if (s->err) { rc = s->err; goto out; }
    if (err != ESP_OK || status != 200) { ESP_LOGE(TAG, "http %s / %d", esp_err_to_name(err), status); rc = MNPKG_E_HTTP; goto out; }
    if (!s->hdr_ok) { rc = MNPKG_E_FORMAT; goto out; }
    if (s->written != s->d.image_len) { rc = MNPKG_E_SIZE; goto out; }

    /* 7: SHA-256 of the payload as written — then read back from flash too */
    uint8_t sum[32];
    mbedtls_sha256_finish(&s->sha, sum);
    if (memcmp(sum, s->d.image_sha256, 32) != 0) { rc = MNPKG_E_SHA; goto out; }
    {
        mbedtls_sha256_context rb; mbedtls_sha256_init(&rb); mbedtls_sha256_starts(&rb, 0);
        uint8_t *buf = malloc(2048);
        if (!buf) { rc = MNPKG_E_FLASH; goto out; }
        for (uint32_t off = 0; off < s->d.image_len; off += 2048) {
            uint32_t n = s->d.image_len - off < 2048 ? s->d.image_len - off : 2048;
            if (esp_partition_read(s->part, off, buf, n) != ESP_OK) { free(buf); rc = MNPKG_E_FLASH; goto out; }
            mbedtls_sha256_update(&rb, buf, n);
        }
        free(buf);
        mbedtls_sha256_finish(&rb, sum);
        mbedtls_sha256_free(&rb);
        if (memcmp(sum, s->d.image_sha256, 32) != 0) { rc = MNPKG_E_SHA; goto out; }
    }

    /* 8: the platform's own image check */
    err = esp_ota_end(s->ota);
    s->ota_open = false;
    if (err != ESP_OK) { ESP_LOGE(TAG, "esp_ota_end: %s", esp_err_to_name(err)); rc = MNPKG_E_IMAGE; goto out; }

    /* 9: running version >= min_running */
    uint8_t run[8]; running_version(run);
    static const uint8_t zero[8] = { 0 };
    if (memcmp(s->d.min_running, zero, 8) != 0 && cmp_ver(run, s->d.min_running) < 0) { rc = MNPKG_E_MIN; goto out; }

    if (esp_ota_set_boot_partition(s->part) != ESP_OK) { rc = MNPKG_E_FLASH; goto out; }
    {
        char v[32]; fmt_ver(s->d.version, v, sizeof v);
        con_printf("mnpkg: staged %s %s on %s, all 9 checks passed%s\n", s->d.fw_version, v, s->part->label,
                   (s->d.flags & FLAG_CLEARS_BUNDLES) ? " (CLEARS_BUNDLES)" : "");
    }
    if (reboot) {
        con_printf("mnpkg: rebooting into it\n");
        vTaskDelay(pdMS_TO_TICKS(300));
        esp_restart();
    }
out:
    if (s->ota_open) esp_ota_abort(s->ota);
    mbedtls_sha256_free(&s->sha);
    if (rc != MNPKG_OK) con_printf("mnpkg: refused, check %d %s\n", rc, mnpkg_err_name(rc));
    free(s);
    return rc;
}

void mnpkg_print_keys(void)
{
    for (size_t i = 0; i < sizeof s_keys / sizeof s_keys[0]; i++) {
        con_printf("key%u %s id=", (unsigned)i, s_keys[i].alg == SIG_ED25519 ? "ed25519" : "p256");
        for (int j = 0; j < 8; j++) con_printf("%02x", s_keys[i].id[j]);
        con_printf("%s\n", (s_keys[i].id[0] == 0xf0 && s_keys[i].id[1] == 0x84) ? " (DEV, bench only)" : "");
    }
}
