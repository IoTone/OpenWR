/*
 * hub_shoot.c — see hub_shoot.h.
 *
 * Licensed under the Apache License, Version 2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "nvs.h"
#include "esp_random.h"
#include "lwip/sockets.h"
#include "hub_shoot.h"
#include "hub_touch.h"
#include "hub_reel.h"
#include "hub_lcd.h"
#include "hub_net.h"
#include "forth_core.h"
#include "console.h"

static const char *TAG = "shoot";

#define BTN_X   (PIC_W / 2)
#define BTN_Y   (LCD_H / 2)
#define BTN_R   30
#define BTN_SHOW_MS 6000
#define BAR_X   (STRIP_X + 4)
#define BAR_Y   156
#define BAR_W   (LCD_W - STRIP_X - 8)
#define BAR_H   8
#define JPG_CAP (40 * 1024)          /* QVGA frames are ~12 KB; leaves room for the glasses' upload */

static QueueHandle_t s_q;            /* touch events, to the UI task */
static QueueHandle_t s_fire;         /* shoot requests, to the shoot task */
static bool     s_btn;               /* the shutter button is showing */
static int64_t  s_btn_at;
static int      s_shots;

/* the cameras on the LAN, found by broadcasting "who" (Scout firmware 0.8.0+) */
#define MAX_CAMS  6
#define HUB_PORT  5555
#define WHO_EVERY_MS 20000
#define SNAP_WAIT_MS 2000
#define SNAP_RESEND_MS 300
typedef struct { char name[17]; char url[40]; int64_t seen; int snapped; } cam_t;
static cam_t    s_cam[MAX_CAMS];
static int      s_ncam;
static int      s_sock = -1;
static SemaphoreHandle_t s_cam_lock;
static volatile int s_snap_id;

static void on_touch(const touch_ev_t *e) { xQueueSend(s_q, e, 0); }

static void button(bool on)
{
    s_btn = on;
    s_btn_at = esp_timer_get_time() / 1000;
    if (on) lcd_disc(BTN_X, BTN_Y, BTN_R, C_WHITE, 5, C_AMBER, C_BLACK);
    else reel_redraw();              /* the picture under it comes back */
}

static void bar(int ms)
{
    int w = ms >= HOLD_MS ? BAR_W : BAR_W * ms / HOLD_MS;
    lcd_fill(BAR_X, BAR_Y, w, BAR_H, C_AMBER);
    if (w < BAR_W) lcd_fill(BAR_X + w, BAR_Y, BAR_W - w, BAR_H, C_DIM);
}

static void ui_task(void *arg)
{
    touch_ev_t e;
    bool bar_on = false;
    for (;;) {
        if (!xQueueReceive(s_q, &e, pdMS_TO_TICKS(200))) {
            if (s_btn && esp_timer_get_time() / 1000 - s_btn_at > BTN_SHOW_MS) button(false);
            continue;
        }
        switch (e.kind) {
        case T_SWIPE:
            if (s_btn) s_btn = false;
            reel_step(-e.dir);                     /* finger left: the next (newer) one */
            break;
        case T_DOUBLE:
            if (e.x < PIC_W) button(!s_btn);
            break;
        case T_TAP:
            if (s_btn) {
                int dx = e.x - BTN_X, dy = e.y - BTN_Y;
                if (dx * dx + dy * dy <= (BTN_R + 12) * (BTN_R + 12)) { s_btn = false; shoot_fire("button"); }
            }
            break;
        case T_HOLDING:
            bar_on = true;
            bar(e.ms);
            break;
        case T_HOLD:
            bar(HOLD_MS);
            shoot_fire("hold");
            break;
        case T_UP:
            if (e.ms < 0) { reel_redraw(); break; }       /* after touch-cal */
            if (bar_on) { lcd_fill(BAR_X, BAR_Y, BAR_W, BAR_H, C_BG); bar_on = false; }
            break;
        }
    }
}

/* ---- the cameras --------------------------------------------------------- */

static void flash(void)
{
    lcd_backlight(50);
    lcd_clear(C_WHITE);
    vTaskDelay(pdMS_TO_TICKS(90));
    lcd_clear(C_BLACK);
    lcd_backlight(40);
}

static int64_t now_ms(void) { return esp_timer_get_time() / 1000; }

static void bcast(const char *msg)
{
    if (s_sock < 0) return;
    struct sockaddr_in to = { .sin_family = AF_INET, .sin_port = htons(HUB_PORT), .sin_addr.s_addr = htonl(INADDR_BROADCAST) };
    sendto(s_sock, msg, strlen(msg), 0, (struct sockaddr *)&to, sizeof to);
}

/* straight to one camera: the AP holds unicast for a dozing station, where
 * a broadcast can be missed */
static void ucast(const char *url, const char *msg)
{
    unsigned a, b, c, d;
    if (s_sock < 0 || sscanf(url, "http://%u.%u.%u.%u", &a, &b, &c, &d) != 4) return;
    struct sockaddr_in to = { .sin_family = AF_INET, .sin_port = htons(HUB_PORT),
                              .sin_addr.s_addr = htonl((a << 24) | (b << 16) | (c << 8) | d) };
    sendto(s_sock, msg, strlen(msg), 0, (struct sockaddr *)&to, sizeof to);
}

/* "cam <name> <url> <w> <h>" and "snapped <id> <bytes> <name> <url>" (a
 * snapped reply also introduces a camera the hub hadn't found yet) */
static void heard(const char *m, uint32_t from)
{
    char name[17], url[40];
    int id;
    unsigned bytes;
    xSemaphoreTake(s_cam_lock, portMAX_DELAY);
    bool snapped = sscanf(m, "snapped %d %u %16s %39s", &id, &bytes, name, url) == 4;
    if (snapped || sscanf(m, "cam %16s %39s", name, url) == 2) {
        int i = 0;
        while (i < s_ncam && strcmp(s_cam[i].name, name)) i++;
        if (i == s_ncam && s_ncam < MAX_CAMS) {
            s_ncam++;
            memset(&s_cam[i], 0, sizeof s_cam[i]);
            snprintf(s_cam[i].name, sizeof s_cam[i].name, "%s", name);
            con_printf("cam found %s %s\n", name, url);
        }
        if (i < s_ncam) {
            size_t l = strlen(url);
            if (l && url[l - 1] == '/') url[l - 1] = 0;
            if (strcmp(s_cam[i].url, url)) { snprintf(s_cam[i].url, sizeof s_cam[i].url, "%s", url); con_printf("cam %s at %s\n", name, url); }
            s_cam[i].seen = now_ms();
            if (snapped && id == s_snap_id) s_cam[i].snapped = id;
        }
    } else if (sscanf(m, "snapped %d", &id) == 1 && id == s_snap_id) {           /* 0.8.0 before names */
        char ip[16];
        snprintf(ip, sizeof ip, "%u.%u.%u.%u", (unsigned)(from & 255), (unsigned)(from >> 8 & 255), (unsigned)(from >> 16 & 255), (unsigned)(from >> 24));
        for (int i = 0; i < s_ncam; i++)
            if (strstr(s_cam[i].url, ip) && !s_cam[i].snapped) { s_cam[i].snapped = id; s_cam[i].seen = now_ms(); }
    }
    xSemaphoreGive(s_cam_lock);
}

static void udp_task(void *arg)
{
    char in[96];
    int64_t last_who = 0;
    for (;;) {
        if (s_sock < 0) {
            if (!hub_net_up()) { vTaskDelay(pdMS_TO_TICKS(1000)); continue; }
            int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            int yes = 1;
            setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &yes, sizeof yes);
            struct timeval tv = { .tv_sec = 0, .tv_usec = 200000 };
            setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
            struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = 0, .sin_addr.s_addr = htonl(INADDR_ANY) };
            if (fd < 0 || bind(fd, (struct sockaddr *)&a, sizeof a) != 0) { if (fd >= 0) close(fd); vTaskDelay(pdMS_TO_TICKS(2000)); continue; }
            s_sock = fd;
            last_who = 0;
        }
        if (!hub_net_up()) { close(s_sock); s_sock = -1; continue; }
        if (now_ms() - last_who > WHO_EVERY_MS) {
            bcast("who");
            xSemaphoreTake(s_cam_lock, portMAX_DELAY);
            for (int i = 0; i < s_ncam; i++) ucast(s_cam[i].url, "who");
            xSemaphoreGive(s_cam_lock);
            last_who = now_ms();
        }
        struct sockaddr_in from; socklen_t fl = sizeof from;
        int n = recvfrom(s_sock, in, sizeof in - 1, 0, (struct sockaddr *)&from, &fl);
        if (n > 0) { in[n] = 0; heard(in, from.sin_addr.s_addr); }
    }
}

static void upper(char *d, const char *s, size_t cap)
{
    size_t i = 0;
    for (; s[i] && i < cap - 1; i++) d[i] = (s[i] >= 'a' && s[i] <= 'z') ? s[i] - 32 : s[i];
    d[i] = 0;
}

/* every LAN camera at once: broadcast snap <id> (three times: UDP), wait for
 * the snapped replies, then fetch each camera's kept frame */
static int shoot_lan(uint8_t *buf)
{
    if (!hub_net_up() || s_sock < 0) { con_printf("shot lan err no wifi\n"); return 0; }
    int id = (int)(esp_random() & 0x3FFFFFFF);
    xSemaphoreTake(s_cam_lock, portMAX_DELAY);
    s_snap_id = id;
    for (int i = 0; i < s_ncam; i++) s_cam[i].snapped = 0;
    int want = s_ncam;
    xSemaphoreGive(s_cam_lock);
    char m[24]; snprintf(m, sizeof m, "snap %d", id);
    int64_t t0 = now_ms(), sent = 0;
    while (now_ms() - t0 < SNAP_WAIT_MS) {
        int got = 0;
        bool resend = now_ms() - sent >= SNAP_RESEND_MS;
        xSemaphoreTake(s_cam_lock, portMAX_DELAY);
        for (int i = 0; i < s_ncam; i++) {
            if (s_cam[i].snapped == id) got++;
            else if (resend) ucast(s_cam[i].url, m);      /* every camera, the same instant */
        }
        want = s_ncam;
        xSemaphoreGive(s_cam_lock);
        if (resend) { bcast(m); sent = now_ms(); }        /* and anyone not found yet */
        if (want && got == want) break;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    con_printf("snap %d: answered in %d ms\n", id, (int)(now_ms() - t0));
    int ok = 0;
    for (int i = 0; i < want; i++) {
        cam_t c;
        xSemaphoreTake(s_cam_lock, portMAX_DELAY);
        c = s_cam[i];
        xSemaphoreGive(s_cam_lock);
        char label[17]; upper(label, c.name, sizeof label);
        if (c.snapped != id) { con_printf("shot %s err no snap\n", c.name); reel_strip_note(label, "NO ANSWER", C_RED); continue; }
        char url[56]; snprintf(url, sizeof url, "%s/snap.jpg", c.url);
        int64_t t1 = now_ms();
        int n = hub_http_get(url, buf, JPG_CAP, 4000);
        if (n <= 0) { con_printf("shot %s err %d\n", c.name, n); reel_strip_note(label, "NO PICTURE", C_RED); continue; }
        int num = reel_add(buf, n, label);
        con_printf("shot %s %d B %d ms #%d\n", c.name, n, (int)(now_ms() - t1), num);
        ok += num > 0;
    }
    if (!want) con_printf("shot lan err no cameras found\n");
    return ok;
}

static void shoot_task(void *arg)
{
    char why[12];
    for (;;) {
        xQueueReceive(s_fire, why, portMAX_DELAY);
        int id = ++s_shots;
        con_printf("fire %d\n", id);                 /* to USB and BLE: the glasses shoot too */
        flash();
        reel_redraw();
        reel_strip_note("SHOOTING", "", C_AMBER);
        uint8_t *buf = malloc(JPG_CAP);
        int got = 0;
        if (!buf) con_printf("shot err memory\n");
        else {
            got += shoot_lan(buf);
            free(buf);
        }
        reel_strip_note(got ? "" : "NO CAMERA", got ? "" : "ANSWERED", got ? C_AMBER : C_RED);
        ESP_LOGI(TAG, "fire %d (%s): %d pictures", id, why, got);
    }
}

void shoot_fire(const char *why)
{
    char w[12];
    snprintf(w, sizeof w, "%s", why);
    xQueueSend(s_fire, w, 0);
}

void shoot_init(void)
{
    s_cam_lock = xSemaphoreCreateMutex();
    xTaskCreate(udp_task, "hubudp", 3072, NULL, 4, NULL);
    s_q = xQueueCreate(16, sizeof(touch_ev_t));
    s_fire = xQueueCreate(2, 12);
    xTaskCreate(ui_task, "ui", 4096, NULL, 4, NULL);
    xTaskCreate(shoot_task, "shoot", 6144, NULL, 3, NULL);
    touch_init(on_touch);
}

/* ---- words --------------------------------------------------------------- */

static void w_shoot(void) { shoot_fire("word"); }
static void w_cams_q(void)
{
    xSemaphoreTake(s_cam_lock, portMAX_DELAY);
    for (int i = 0; i < s_ncam; i++)
        con_printf("cam %s %s seen %lld s ago\n", s_cam[i].name, s_cam[i].url, (now_ms() - s_cam[i].seen) / 1000);
    int n = s_ncam;
    xSemaphoreGive(s_cam_lock);
    con_printf("cam glasses ble fire line\ncams %d\n", n);
}
static void w_cams_find(void) { bcast("who"); con_printf("who sent\n"); }

void shoot_register_words(void)
{
    forth_register_word("shoot", w_shoot);
    forth_register_word("cams?", w_cams_q);
    forth_register_word("cams-find", w_cams_find);
}
