/*
 * ble_console.c — NUS console on NimBLE, peripheral only, one connection.
 *
 * Licensed under the Apache License, Version 2.0
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#include "forth_core.h"
#include "console.h"
#include "ble_console.h"

static const char *TAG = "ble";

/* Nordic UART Service, 128-bit UUIDs in little-endian byte order */
static const ble_uuid128_t UUID_NUS = BLE_UUID128_INIT(
    0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5,0x01,0x00,0x40,0x6e);
static const ble_uuid128_t UUID_RX = BLE_UUID128_INIT(
    0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5,0x02,0x00,0x40,0x6e);
static const ble_uuid128_t UUID_TX = BLE_UUID128_INIT(
    0x9e,0xca,0xdc,0x24,0x0e,0xe5,0xa9,0xe0,0x93,0xf3,0xa3,0xb5,0x03,0x00,0x40,0x6e);

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */

#define RX_RING   4096
#define TX_CHUNK  180

static uint8_t  s_own_addr_type;
static uint16_t s_tx_handle;
static uint16_t s_conn = BLE_HS_CONN_HANDLE_NONE;
static bool     s_subscribed;
static bool     s_synced;
static bool     s_advertising;
static uint16_t s_mtu = 23;
static int64_t  s_window_until_us;       /* 0 = no window */
static char     s_name[20] = "goggl1";

/* RX ring: written from the NimBLE host task, read by the REPL task.
 * Single producer / single consumer, so plain volatile indices suffice. */
static uint8_t  s_rx[RX_RING];
static volatile uint16_t s_rx_head, s_rx_tail;

/* TX line buffer, guarded: the REPL task writes, the scheduler flushes. */
static uint8_t  s_tx[TX_CHUNK];
static uint16_t s_tx_len;
static SemaphoreHandle_t s_tx_lock;

/* ------------------------------------------------------------------ */
/* Advertising                                                         */
/* ------------------------------------------------------------------ */

static int gap_event(struct ble_gap_event *ev, void *arg);

static void adv_start(void)
{
    if (!s_synced || s_advertising || s_conn != BLE_HS_CONN_HANDLE_NONE) return;

    struct ble_hs_adv_fields adv = { 0 };
    adv.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    adv.name = (uint8_t *)s_name;
    adv.name_len = strlen(s_name);
    adv.name_is_complete = 1;
    adv.tx_pwr_lvl_is_present = 1;
    adv.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    int rc = ble_gap_adv_set_fields(&adv);
    if (rc) { ESP_LOGE(TAG, "adv fields rc=%d", rc); return; }

    struct ble_hs_adv_fields rsp = { 0 };
    rsp.uuids128 = (ble_uuid128_t *)&UUID_NUS;
    rsp.num_uuids128 = 1;
    rsp.uuids128_is_complete = 1;
    ble_gap_adv_rsp_set_fields(&rsp);

    struct ble_gap_adv_params p = { 0 };
    p.conn_mode = BLE_GAP_CONN_MODE_UND;
    p.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(s_own_addr_type, NULL, BLE_HS_FOREVER, &p, gap_event, NULL);
    if (rc) { ESP_LOGE(TAG, "adv start rc=%d", rc); return; }
    s_advertising = true;
    ESP_LOGI(TAG, "advertising as %s", s_name);
}

static void adv_stop(void)
{
    if (!s_advertising) return;
    ble_gap_adv_stop();
    s_advertising = false;
    ESP_LOGI(TAG, "advertising stopped");
}

static bool window_open(void)
{
    return s_window_until_us && esp_timer_get_time() < s_window_until_us;
}

/* ------------------------------------------------------------------ */
/* GAP / GATT                                                          */
/* ------------------------------------------------------------------ */

static int gap_event(struct ble_gap_event *ev, void *arg)
{
    switch (ev->type) {
    case BLE_GAP_EVENT_CONNECT:
        s_advertising = false;
        if (ev->connect.status == 0) {
            s_conn = ev->connect.conn_handle;
            ESP_LOGI(TAG, "connected");
        } else if (window_open()) {
            adv_start();
        }
        return 0;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI(TAG, "disconnected (reason %d)", ev->disconnect.reason);
        s_conn = BLE_HS_CONN_HANDLE_NONE;
        s_subscribed = false;
        s_mtu = 23;
        /* A phone that was connected gets three minutes to come back (it
         * slept, walked out of range, the app was restarted) without anyone
         * touching the goggle. The window is still a window: it closes. */
        s_window_until_us = esp_timer_get_time() + 180LL * 1000000LL;
        adv_start();
        return 0;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        s_advertising = false;
        return 0;
    case BLE_GAP_EVENT_CONN_UPDATE:
        ESP_LOGI(TAG, "connection update (status %d)", ev->conn_update.status);
        return 0;
    case BLE_GAP_EVENT_MTU:
        s_mtu = ev->mtu.value;
        return 0;
    case BLE_GAP_EVENT_SUBSCRIBE:
        if (ev->subscribe.attr_handle == s_tx_handle)
            s_subscribed = ev->subscribe.cur_notify;
        return 0;
    default:
        return 0;
    }
}

static int rx_access(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op != BLE_GATT_ACCESS_OP_WRITE_CHR) return 0;
    struct os_mbuf *om = ctxt->om;
    while (om) {
        for (int i = 0; i < om->om_len; i++) {
            uint16_t next = (uint16_t)((s_rx_head + 1) % RX_RING);
            if (next == s_rx_tail) { ESP_LOGW(TAG, "rx overflow"); return 0; }
            s_rx[s_rx_head] = om->om_data[i];
            s_rx_head = next;
        }
        om = SLIST_NEXT(om, om_next);
    }
    return 0;
}

static int tx_access(uint16_t conn, uint16_t attr, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    return 0;   /* notify only */
}

static const struct ble_gatt_svc_def s_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &UUID_NUS.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            { .uuid = &UUID_RX.u, .access_cb = rx_access,
              .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP },
            { .uuid = &UUID_TX.u, .access_cb = tx_access, .val_handle = &s_tx_handle,
              .flags = BLE_GATT_CHR_F_NOTIFY },
            { 0 }
        },
    },
    { 0 }
};

static void on_sync(void)
{
    ble_hs_util_ensure_addr(0);
    ble_hs_id_infer_auto(0, &s_own_addr_type);
    s_synced = true;
    ESP_LOGI(TAG, "host synced");
    if (window_open()) adv_start();
}

static void on_reset(int reason) { ESP_LOGW(TAG, "host reset %d", reason); s_synced = false; }

static void host_task(void *arg)
{
    nimble_port_run();
    nimble_port_freertos_deinit();
}

esp_err_t ble_console_init(const char *name)
{
    if (name) { strncpy(s_name, name, sizeof s_name - 1); }
    s_tx_lock = xSemaphoreCreateMutex();

    esp_err_t err = nimble_port_init();
    if (err != ESP_OK) { ESP_LOGE(TAG, "nimble_port_init: %s", esp_err_to_name(err)); return err; }

    ble_hs_cfg.sync_cb = on_sync;
    ble_hs_cfg.reset_cb = on_reset;
    ble_svc_gap_init();
    ble_svc_gatt_init();
    int rc = ble_gatts_count_cfg(s_svcs);
    if (rc == 0) rc = ble_gatts_add_svcs(s_svcs);
    if (rc) { ESP_LOGE(TAG, "gatt svcs rc=%d", rc); return ESP_FAIL; }
    ble_svc_gap_device_name_set(s_name);
    nimble_port_freertos_init(host_task);
    return ESP_OK;
}

/* ------------------------------------------------------------------ */
/* Console side                                                        */
/* ------------------------------------------------------------------ */

int ble_console_getchar(void)
{
    if (s_rx_tail == s_rx_head) return -1;
    uint8_t c = s_rx[s_rx_tail];
    s_rx_tail = (uint16_t)((s_rx_tail + 1) % RX_RING);
    return c;
}

static uint32_t s_tx_dropped;

/* Notifications fail with ENOMEM when the host's mbuf pool is momentarily
 * empty (a wx-dump is a dozen chunks in a few ms). Dropping the chunk loses
 * the ok> prompt and the phone waits out its timeout, so wait and retry. */
static void tx_send_locked(void)
{
    if (s_tx_len == 0) return;
    if (s_conn != BLE_HS_CONN_HANDLE_NONE && s_subscribed) {
        uint16_t chunk = s_mtu > 3 ? s_mtu - 3 : 20;
        if (chunk > TX_CHUNK) chunk = TX_CHUNK;
        for (uint16_t off = 0; off < s_tx_len; off += chunk) {
            uint16_t n = s_tx_len - off < chunk ? s_tx_len - off : chunk;
            int rc = -1;
            for (int tries = 0; tries < 40 && rc != 0; tries++) {
                struct os_mbuf *om = ble_hs_mbuf_from_flat(s_tx + off, n);
                rc = om ? ble_gatts_notify_custom(s_conn, s_tx_handle, om) : BLE_HS_ENOMEM;
                if (rc != 0) vTaskDelay(pdMS_TO_TICKS(5));
            }
            if (rc != 0) { s_tx_dropped++; ESP_LOGW(TAG, "notify dropped rc=%d (total %lu)", rc, (unsigned long)s_tx_dropped); }
        }
    }
    s_tx_len = 0;
}

void ble_console_putchar(int c)
{
    if (s_conn == BLE_HS_CONN_HANDLE_NONE) return;      /* nobody listening */
    if (!s_tx_lock || xSemaphoreTake(s_tx_lock, pdMS_TO_TICKS(500)) != pdTRUE) { s_tx_dropped++; return; }
    s_tx[s_tx_len++] = (uint8_t)c;
    if (s_tx_len >= TX_CHUNK || c == '\n') tx_send_locked();
    xSemaphoreGive(s_tx_lock);
}

void ble_console_flush(void)
{
    if (!s_tx_lock || s_tx_len == 0) return;
    if (xSemaphoreTake(s_tx_lock, pdMS_TO_TICKS(20)) != pdTRUE) return;
    tx_send_locked();
    xSemaphoreGive(s_tx_lock);
}

void ble_console_poll(void)
{
    if (s_advertising && !window_open()) {
        adv_stop();
        s_window_until_us = 0;
    }
}

/* ------------------------------------------------------------------ */
/* Pairing window                                                      */
/* ------------------------------------------------------------------ */

void ble_pair_window(int seconds)
{
    s_window_until_us = esp_timer_get_time() + (int64_t)seconds * 1000000LL;
    con_printf("ble: pairing window open for %d s, advertising as %s\n", seconds, s_name);
    adv_start();
}

void ble_pair_close(void)
{
    s_window_until_us = 0;
    adv_stop();
    if (s_conn != BLE_HS_CONN_HANDLE_NONE)
        ble_gap_terminate(s_conn, BLE_ERR_REM_USER_CONN_TERM);
}

uint32_t ble_console_dropped(void) { return s_tx_dropped; }

int ble_console_state(void)
{
    if (s_conn != BLE_HS_CONN_HANDLE_NONE) return 2;
    return s_advertising ? 1 : 0;
}

/* ------------------------------------------------------------------ */
/* Words                                                               */
/* ------------------------------------------------------------------ */

static void w_pair(void)    { ble_pair_window(180); }
static void w_unpair(void)  { ble_pair_close(); }
static void w_ble_on(void)  { forth_push(ble_console_state() ? -1 : 0); }

/* ble-conn ( -- ) the link as the central set it: interval, latency,
 * supervision timeout, MTU */
static void w_ble_conn(void)
{
    struct ble_gap_conn_desc d;
    if (s_conn == BLE_HS_CONN_HANDLE_NONE || ble_gap_conn_find(s_conn, &d) != 0) { con_printf("ble: no link\n"); return; }
    int us = d.conn_itvl * 1250;
    con_printf("ble: interval %d.%02d ms, latency %d, timeout %d ms, mtu %d\n",
               us / 1000, (us % 1000) / 10, d.conn_latency, d.supervision_timeout * 10, s_mtu);
}

/* ble-conn! ( min max -- ) ask the central for a connection interval, in
 * 1.25 ms units (6 = 7.5 ms); it may refuse or pick something in between. A
 * short interval moves more notifications a second; ask for it around a
 * transfer and give it back after. */
static void w_ble_conn_store(void)
{
    int max = (int)forth_pop(), min = (int)forth_pop();
    if (s_conn == BLE_HS_CONN_HANDLE_NONE) { con_printf("ble: no link\n"); return; }
    if (min < 6 || max < min || max > 3200) { con_printf("? ble-conn! bad args\n"); return; }
    struct ble_gap_upd_params p = { .itvl_min = min, .itvl_max = max, .latency = 0, .supervision_timeout = 400 };
    int rc = ble_gap_update_params(s_conn, &p);
    if (rc != 0) con_printf("ble: update refused here (rc %d)\n", rc);
}

void ble_console_register_words(void)
{
    forth_register_word("pair",    w_pair);
    forth_register_word("unpair",  w_unpair);
    forth_register_word("ble-on?", w_ble_on);
    forth_register_word("ble-conn", w_ble_conn);
    forth_register_word("ble-conn!", w_ble_conn_store);
}
