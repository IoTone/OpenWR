/*
 * cam_words.c — Snapshot on the XIAO ESP32S3 Sense: the OV2640 camera, the
 * microSD roll, and the words that put a photo on the e-paper.
 *
 *   cam?        ( -- flag )             camera up
 *   cam-snap    ( -- flag )             grab a 320x240 grey frame into PSRAM
 *   epd-photo   ( x y w h -- )          the frame, fitted and dithered, into the canvas
 *   inset!      ( w h -- )              start a 1-bit inset (phone-rendered minimap)
 *   inset-hex!  ( c-addr u -- )         append inset bytes as hex (rows of (w+7)/8 bytes)
 *   inset-draw  ( x y -- )              the inset with a white margin and a frame
 *   snap        ( c-addr u -- )         photo + inset + caption inside the safe area, show, roll
 *   sd?         ( -- flag )             card mounted
 *   roll-ls     ( -- )                  list the roll
 *   roll-get    ( n -- )                print frame n as hex lines (for the phone)
 *   roll-save   ( -- n )                canvas + grey frame to the card
 *
 * Only built for the S3; on the C3 the words are absent.
 */
#include "sdkconfig.h"
#if CONFIG_IDF_TARGET_ESP32S3

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_camera.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdspi_host.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_rom_sys.h"
#include "hal/gpio_ll.h"

#include "forth_core.h"
#include "console.h"
#include "pins.h"
#include "epd.h"
#include "skin_words.h"
#include "cam_words.h"

static const char *TAG = "cam";
#define POP() ((int)forth_pop())

#define CAM_W 320
#define CAM_H 240
static uint8_t *s_frame;          /* PSRAM, CAM_W * CAM_H grey */
static bool s_cam_up, s_have_frame, s_sd_up;
static sdmmc_card_t *s_card;

/* the inset: a packed 1-bit bitmap the phone sends */
static uint8_t s_inset[64 * 16];   /* up to 128 wide x 64 tall */
static int s_inset_w, s_inset_h, s_inset_cur;

/* ---- camera -------------------------------------------------------- */

static esp_err_t cam_init(void)
{
    camera_config_t c = {
        .pin_pwdn = -1, .pin_reset = -1, .pin_xclk = 10, .pin_sccb_sda = 40, .pin_sccb_scl = 39,
        .pin_d7 = 48, .pin_d6 = 11, .pin_d5 = 12, .pin_d4 = 14, .pin_d3 = 16, .pin_d2 = 18, .pin_d1 = 17, .pin_d0 = 15,
        .pin_vsync = 38, .pin_href = 47, .pin_pclk = 13,
        .xclk_freq_hz = 20000000, .ledc_timer = LEDC_TIMER_0, .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_GRAYSCALE, .frame_size = FRAMESIZE_QVGA, .jpeg_quality = 12,
        .fb_count = 1, .fb_location = CAMERA_FB_IN_PSRAM, .grab_mode = CAMERA_GRAB_LATEST,
    };
    esp_err_t err = esp_camera_init(&c);
    if (err != ESP_OK) { ESP_LOGW(TAG, "camera init: %s", esp_err_to_name(err)); return err; }
    sensor_t *s = esp_camera_sensor_get();
    if (s) { s->set_vflip(s, 1); s->set_hmirror(s, 1); }   /* the Sense module is mounted upside down on the breakout */
    s_cam_up = true;
    return ESP_OK;
}

static void w_cam_q(void) { forth_push(s_cam_up ? -1 : 0); }

/* cam-scan ( -- ): run XCLK (a sensor only answers with a clock) and probe
 * the SCCB bus for anything, so "no module" and "wrong driver" look different. */
static void w_cam_scan(void)
{
    if (s_cam_up) { con_printf("cam-scan: camera already up\n"); return; }
    /* Presence check: the Sense board pulls SIOD/SIOC and SD CS up; with the
     * chip's own pull-downs on, a 1 means something external is there. */
    {
        const int pins[3] = { 40, 39, 21 };
        const char *names[3] = { "SIOD40", "SIOC39", "SDCS21" };
        for (int i = 0; i < 3; i++) {
            gpio_config_t io = { .pin_bit_mask = 1ULL << pins[i], .mode = GPIO_MODE_INPUT, .pull_down_en = GPIO_PULLDOWN_ENABLE };
            gpio_config(&io);
            vTaskDelay(pdMS_TO_TICKS(2));
            con_printf("cam-scan: %s reads %d with pull-down (1 = board present)\n", names[i], gpio_get_level(pins[i]));
            gpio_config_t off = { .pin_bit_mask = 1ULL << pins[i], .mode = GPIO_MODE_INPUT };
            gpio_config(&off);
        }
    }
    ledc_timer_config_t tc = { .speed_mode = LEDC_LOW_SPEED_MODE, .duty_resolution = LEDC_TIMER_1_BIT, .timer_num = LEDC_TIMER_1,
                               .freq_hz = 20000000, .clk_cfg = LEDC_AUTO_CLK };
    ledc_timer_config(&tc);
    ledc_channel_config_t cc = { .gpio_num = 10, .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_1, .timer_sel = LEDC_TIMER_1, .duty = 1, .hpoint = 0 };
    ledc_channel_config(&cc);
    vTaskDelay(pdMS_TO_TICKS(20));
    {   /* read the clock pin back while it runs: a toggling XCLK samples as a mix of 0s and 1s */
        gpio_ll_input_enable(&GPIO, 10);   /* read back without re-routing the LEDC signal */
        int ones = 0;
        for (int i = 0; i < 200; i++) { ones += gpio_get_level(10); esp_rom_delay_us(7); }
        con_printf("cam-scan: XCLK10 samples high %d/200 (%s)\n", ones, (ones > 20 && ones < 180) ? "toggling" : "STUCK");
    }
    /* the camera library links the legacy I2C driver; use the same one */
    i2c_config_t ic = { .mode = I2C_MODE_MASTER, .sda_io_num = 40, .scl_io_num = 39,
                        .sda_pullup_en = GPIO_PULLUP_ENABLE, .scl_pullup_en = GPIO_PULLUP_ENABLE, .master.clk_speed = 100000 };
    if (i2c_param_config(I2C_NUM_1, &ic) != ESP_OK || i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0) != ESP_OK) {
        con_printf("cam-scan: no i2c bus\n"); return;
    }
    int found = 0;
    for (int a = 0x08; a < 0x78; a++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (a << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_err_t r = i2c_master_cmd_begin(I2C_NUM_1, cmd, pdMS_TO_TICKS(20));
        i2c_cmd_link_delete(cmd);
        if (r == ESP_OK) { con_printf("cam-scan: device at 0x%02x\n", a); found++; }
    }
    i2c_driver_delete(I2C_NUM_1);
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    con_printf("cam-scan: %d device(s) on SDA40/SCL39 (OV2640 answers at 0x30)\n", found);
}

static void w_cam_snap(void)
{
    if (!s_cam_up || !s_frame) { forth_push(0); return; }
    camera_fb_t *fb = NULL;
    for (int i = 0; i < 3; i++) {               /* the first frame after sleep is dark; take the third */
        if (fb) esp_camera_fb_return(fb);
        fb = esp_camera_fb_get();
        if (!fb) break;
    }
    if (!fb || fb->width != CAM_W || fb->height != CAM_H || fb->len < CAM_W * CAM_H) {
        if (fb) esp_camera_fb_return(fb);
        con_printf("cam-snap: no frame\n");
        forth_push(0);
        return;
    }
    memcpy(s_frame, fb->buf, CAM_W * CAM_H);
    esp_camera_fb_return(fb);
    s_have_frame = true;
    forth_push(-1);
}

/* Fit the frame into w x h (crop to aspect, centre at 35 % from the top),
 * stretch the contrast between the 2 % and 98 % percentiles, lift the
 * midtones (gamma 0.55: e-paper has no dark grey), then a 4x4 Bayer
 * ordered dither, which reads as texture rather than noise on this glass. */
static void photo_into(int X, int Y, int W, int H)
{
    static const uint8_t bayer[4][4] = { {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5} };
    float sx = (float)CAM_W / W, sy = (float)CAM_H / H;
    float sc = sx < sy ? sx : sy;                 /* cover: scale by the smaller ratio */
    int cw = (int)(W * sc), ch = (int)(H * sc);
    int ox = (CAM_W - cw) / 2, oy = (int)((CAM_H - ch) * 0.35f);
    uint32_t hist[256] = { 0 };
    for (int y = oy; y < oy + ch; y += 2)
        for (int x = ox; x < ox + cw; x += 2) hist[s_frame[y * CAM_W + x]]++;
    uint32_t total = 0; for (int i = 0; i < 256; i++) total += hist[i];
    int lo = 0, hi = 255; uint32_t acc = 0;
    for (int i = 0; i < 256; i++) { acc += hist[i]; if (acc >= total * 2 / 100) { lo = i; break; } }
    acc = 0;
    for (int i = 255; i >= 0; i--) { acc += hist[i]; if (acc >= total * 2 / 100) { hi = i; break; } }
    if (hi <= lo + 8) { lo = 0; hi = 255; }
    static uint8_t lut[256];
    for (int i = 0; i < 256; i++) {
        float v = (float)(i - lo) / (float)(hi - lo);
        if (v < 0) v = 0;
        if (v > 1) v = 1;
        lut[i] = (uint8_t)(255.0f * powf(v, 0.55f));
    }
    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            /* box average of the source region */
            int x0 = ox + (int)(x * sc), x1 = ox + (int)((x + 1) * sc), y0 = oy + (int)(y * sc), y1 = oy + (int)((y + 1) * sc);
            if (x1 <= x0) x1 = x0 + 1;
            if (y1 <= y0) y1 = y0 + 1;
            uint32_t sum = 0, n = 0;
            for (int yy = y0; yy < y1 && yy < CAM_H; yy++)
                for (int xx = x0; xx < x1 && xx < CAM_W; xx++) { sum += s_frame[yy * CAM_W + xx]; n++; }
            int v = lut[n ? sum / n : 0];
            int th = (bayer[y & 3][x & 3] + 1) * 16 - 8;
            epd_pixel(X + x, Y + y, v < th ? 1 : 0);
        }
    }
}

static void w_epd_photo(void)
{
    int h = POP(), w = POP(), y = POP(), x = POP();
    if (!s_have_frame) { con_printf("epd-photo: no frame (cam-snap first)\n"); return; }
    if (w < 8 || h < 8) return;
    photo_into(x, y, w, h);
}

/* ---- inset ----------------------------------------------------------- */

static void w_inset_store(void)
{
    int h = POP(), w = POP();
    if (w < 8 || h < 8 || w > 128 || h > 64) { con_printf("inset!: up to 128x64\n"); s_inset_w = 0; return; }
    s_inset_w = w; s_inset_h = h; s_inset_cur = 0;
    memset(s_inset, 0, sizeof s_inset);
}

static int hexval(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static void w_inset_hex(void)
{
    int n = POP(); const char *s = (const char *)forth_pop();
    if (!s || n <= 0 || !s_inset_w) return;
    int stride = (s_inset_w + 7) / 8, max = stride * s_inset_h, hi = -1;
    for (int i = 0; i < n; i++) {
        int v = hexval(s[i]);
        if (v < 0) continue;
        if (hi < 0) { hi = v; continue; }
        if (s_inset_cur < max) s_inset[s_inset_cur++] = (uint8_t)((hi << 4) | v);
        hi = -1;
    }
}

static void inset_draw(int x, int y)
{
    if (!s_inset_w) return;
    int stride = (s_inset_w + 7) / 8;
    epd_box(x - 3, y - 3, s_inset_w + 6, s_inset_h + 6, 0);
    epd_frame(x - 3, y - 3, s_inset_w + 6, s_inset_h + 6);
    for (int j = 0; j < s_inset_h; j++)
        for (int i = 0; i < s_inset_w; i++)
            if (s_inset[j * stride + (i >> 3)] & (0x80 >> (i & 7))) epd_pixel(x + i, y + j, 1);
}

static void w_inset_draw(void) { int y = POP(), x = POP(); inset_draw(x, y); }

/* ---- SD card roll ------------------------------------------------------ */

static esp_err_t sd_mount(void)
{
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = SKIN_SPI_HOST;
    host.max_freq_khz = 10000;                     /* the e-paper shares this bus */
    sdspi_device_config_t slot = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot.gpio_cs = SKIN_PIN_SD_CS;
    slot.host_id = SKIN_SPI_HOST;
    esp_vfs_fat_sdmmc_mount_config_t mc = { .format_if_mount_failed = false, .max_files = 4, .allocation_unit_size = 16 * 1024 };
    esp_err_t err = esp_vfs_fat_sdspi_mount("/sd", &host, &slot, &mc, &s_card);
    if (err != ESP_OK) { ESP_LOGW(TAG, "no SD card (%s)", esp_err_to_name(err)); return err; }
    mkdir("/sd/roll", 0775);
    s_sd_up = true;
    ESP_LOGI(TAG, "SD: %s, %llu MB", s_card->cid.name, ((uint64_t)s_card->csd.capacity * s_card->csd.sector_size) >> 20);
    return ESP_OK;
}

static void w_sd_q(void) { forth_push(s_sd_up ? -1 : 0); }

static int roll_next(void)
{
    int n = 0;
    DIR *d = opendir("/sd/roll");
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        int k = atoi(e->d_name);
        if (k >= n) n = k + 1;
    }
    closedir(d);
    return n;
}

static int roll_save(void)
{
    if (!s_sd_up) return -1;
    int n = roll_next();
    char path[48];
    snprintf(path, sizeof path, "/sd/roll/%04d.bin", n);
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    fwrite(epd_canvas(), 1, EPD_CANVAS, f);
    fclose(f);
    if (s_have_frame) {
        snprintf(path, sizeof path, "/sd/roll/%04d.pgm", n);
        f = fopen(path, "wb");
        if (f) { fprintf(f, "P5\n%d %d\n255\n", CAM_W, CAM_H); fwrite(s_frame, 1, CAM_W * CAM_H, f); fclose(f); }
    }
    return n;
}

static void w_roll_save(void) { forth_push(roll_save()); }

static void w_roll_ls(void)
{
    DIR *d = opendir("/sd/roll");
    if (!d) { con_printf("roll: no card\n"); return; }
    struct dirent *e; int n = 0;
    while ((e = readdir(d)) != NULL) {
        if (strcasestr(e->d_name, ".bin")) { con_printf("%s\n", e->d_name); n++; }
    }
    closedir(d);
    con_printf("roll: %d frames\n", n);
}

static void w_roll_get(void)
{
    int n = POP();
    char path[48];
    snprintf(path, sizeof path, "/sd/roll/%04d.bin", n);
    FILE *f = fopen(path, "rb");
    if (!f) { con_printf("roll-get: no frame %d\n", n); return; }
    uint8_t buf[110];
    size_t k;
    while ((k = fread(buf, 1, sizeof buf, f)) > 0) {
        char line[224];
        for (size_t i = 0; i < k; i++) sprintf(line + i * 2, "%02x", buf[i]);
        con_printf("%s\n", line);
    }
    fclose(f);
}

/* ---- the Snapshot ------------------------------------------------------ */

/* photo across the safe area, the inset top-right, a caption band at the
 * bottom; then show and keep. ( c-addr u -- ) */
static void w_snap(void)
{
    int n = POP(); const char *cap = (const char *)forth_pop();
    int x0 = skin_safe_x(), w = skin_safe_w();
    if (!s_have_frame) {
        forth_push(0); w_cam_snap();
        if (POP() == 0) { con_printf("snap: no camera frame\n"); return; }
    }
    epd_clear();
    photo_into(x0, 0, w, EPD_H);
    if (s_inset_w) inset_draw(x0 + w - s_inset_w - 4, 4);
    if (cap && n > 0) {
        char t[40]; int m = n < 39 ? n : 39;
        memcpy(t, cap, m); t[m] = 0;
        epd_box(x0, EPD_H - 11, w, 11, 0);          /* white band, black text, then invert the band */
        epd_str(x0 + 3, EPD_H - 9, t, m, 1);
        for (int y = EPD_H - 11; y < EPD_H; y++)
            for (int x = x0; x < x0 + w; x++) epd_pixel(x, y, !epd_get(x, y));
    }
    epd_show();
    int r = roll_save();
    if (r >= 0) con_printf("snap: roll %d\n", r); else con_printf("snap: shown (no card)\n");
    s_have_frame = false;
}

/* ---- init -------------------------------------------------------------- */

void cam_init_all(void)
{
    s_frame = heap_caps_malloc(CAM_W * CAM_H, MALLOC_CAP_SPIRAM);
    if (!s_frame) ESP_LOGW(TAG, "no PSRAM for the frame");
    cam_init();
    sd_mount();
}

void cam_register_words(void)
{
    static const struct { const char *name; void (*fn)(void); } words[] = {
        { "cam?", w_cam_q }, { "cam-scan", w_cam_scan }, { "cam-snap", w_cam_snap }, { "epd-photo", w_epd_photo },
        { "inset!", w_inset_store }, { "inset-hex!", w_inset_hex }, { "inset-draw", w_inset_draw },
        { "snap", w_snap }, { "sd?", w_sd_q }, { "roll-ls", w_roll_ls }, { "roll-get", w_roll_get }, { "roll-save", w_roll_save },
    };
    for (size_t i = 0; i < sizeof words / sizeof words[0]; i++) forth_register_word(words[i].name, words[i].fn);
}

#else
void cam_init_all(void) {}
void cam_register_words(void) {}
#endif
