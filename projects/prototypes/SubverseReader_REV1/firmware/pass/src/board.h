/*
 * board.h: the two passes this firmware builds for. PlatformIO picks one
 * with -DBOARD_SR15 or -DBOARD_SR16 (see platformio.ini).
 *
 * SR-15 Dial Pass: Elecrow CrowPanel ESP32 4.2in E-Paper HMI (DIE07300S),
 *   pins from Elecrow's Arduino examples and schematic. ESP32-S3, 400 x 300
 *   glass held portrait, microSD, thumb dial + MENU + EXIT, USB through a
 *   CH340 on UART0.
 *
 * SR-16 Setlist: Seeed XIAO (ESP32-C5 or ESP32-S3) on Seeed's ePaper Driver
 *   Board for XIAO V2, Elecrow's bare 3.7in 240 x 416 panel (UC8253). No card
 *   slot: packs live in the XIAO's flash. Three hand-wired keys, or none at
 *   all (SR-16N): then the phone app, the auto-turn timer and the XIAO's own
 *   BOOT button drive it. USB is the chip's own serial port.
 *
 *   Driver board: RST D0, CS D1, BUSY D2, DC D3, SCK D8, MOSI D10 (Seeed
 *   wiki). Seeed's older XIAO ePaper Breakout Board has BUSY on D5 instead;
 *   the panel driver finds which, so both boards work. The keys stay off D2
 *   and D5 for that reason: PREV D4, MENU D7, NEXT D9.
 *
 * Keys are active low.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef PASS_BOARD_H
#define PASS_BOARD_H
#include "sdkconfig.h"

#if defined(BOARD_SR15)
/* ------------------------------------------------------------------ SR-15 */
#define BOARD_NAME     "SR-15"
#define BOARD_TITLE    "SR-15 Dial Pass"
#define BOARD_HW       "Elecrow CrowPanel 4.2in"
#define BOARD_PROFILE  "sr15"
#define BOARD_CONSOLE_UART 1        /* CH340 on UART0 */
#define BOARD_STORE_SD     1
#define BOARD_KEYS_DEFAULT 5        /* dial (left, right, push) + MENU + EXIT */

#define EPD_NW       400            /* native: source pixels a row */
#define EPD_NH       300            /* native: gate rows */
#define EPD_CW       300            /* the canvas, as the reader holds the pass */
#define EPD_CH       400

/* SSD1683 or UC81xx panel on SPI2 */
#define PIN_EPD_SCK    12
#define PIN_EPD_MOSI   11
#define PIN_EPD_CS     45
#define PIN_EPD_DC     46
#define PIN_EPD_RST    47
#define PIN_EPD_BUSY   48
#define PIN_EPD_PWR     7     /* high: panel powered (switches its ground) */

/* microSD on SPI3 */
#define PIN_SD_SCK     39
#define PIN_SD_MOSI    40
#define PIN_SD_MISO    13
#define PIN_SD_CS      10
#define PIN_SD_PWR     42     /* high: card powered */

/* Controls: Elecrow's names, then what SR-15 calls them */
#define PIN_KEY_MENU    2     /* HOME */
#define PIN_KEY_EXIT    1     /* EXIT */
#define PIN_KEY_UP      4     /* dial turned left in portrait: previous (Elecrow's NEXT) */
#define PIN_KEY_DOWN    6     /* dial turned right in portrait: next (Elecrow's PRV) */
#define PIN_KEY_OK      5     /* dial push */
#define PIN_KEY_BOOT   (-1)
#define PIN_KEY_WAKE   (-1)

#define PIN_LED        41     /* the only LED; firmware-driven, not a charge light. On the */
#define PIN_PWR_EN     41     /* green-sticker revision Elecrow drives it high as "power enable" */

#elif defined(BOARD_SR16)
/* ------------------------------------------------------------------ SR-16 */
#define BOARD_NAME     "SR-16"
#define BOARD_TITLE    "SR-16 Setlist"
#define BOARD_PROFILE  "sr16"
#define BOARD_CONSOLE_UART 0        /* the chip's USB serial port */
#define BOARD_STORE_SD     0        /* packs in flash */
#define BOARD_KEYS_DEFAULT 0        /* no keys until `3 sr-keys!` says they are fitted */

#define EPD_NW       240
#define EPD_NH       416
#define EPD_CW       240
#define EPD_CH       416

#define PIN_KEY_MENU   (-1)
#define PIN_KEY_EXIT   (-1)
#define PIN_EPD_PWR    (-1)
#define PIN_LED        (-1)

#if CONFIG_IDF_TARGET_ESP32C5
/* XIAO ESP32-C5: D0 1, D1 0, D2 25, D3 7, D4 23, D5 24, D6 11, D7 12, D8 8, D9 9, D10 10 */
#define BOARD_HW       "XIAO ESP32-C5 + 3.7in"
#define PIN_EPD_RST     1     /* D0 */
#define PIN_EPD_CS      0     /* D1 */
#define PIN_EPD_BUSY   25     /* D2 on the driver board */
#define PIN_EPD_BUSY_ALT 24   /* D5 on the breakout board */
#define PIN_EPD_DC      7     /* D3 */
#define PIN_EPD_SCK     8     /* D8 */
#define PIN_EPD_MOSI   10     /* D10 */
#define PIN_KEY_UP     23     /* D4: PREV */
#define PIN_KEY_OK     12     /* D7: MENU */
#define PIN_KEY_DOWN    9     /* D9: NEXT */
#define PIN_KEY_BOOT   28     /* the XIAO's BOOT button: can't wake the C5 */
/* Only LP GPIO 0-6 wake a C5. The three keys also pull this line low
 * through a Schottky diode each (back pad MTDI). */
#define PIN_KEY_WAKE    3
#define PIN_BAT_ADC     6     /* the XIAO's own divider (100k/100k)... */
#define PIN_BAT_EN     26     /* ...behind a load switch, high to measure */
#elif CONFIG_IDF_TARGET_ESP32S3
/* XIAO ESP32-S3: D0 1, D1 2, D2 3, D3 4, D4 5, D5 6, D6 43, D7 44, D8 7, D9 8, D10 9 */
#define BOARD_HW       "XIAO ESP32-S3 + 3.7in"
#define PIN_EPD_RST     1
#define PIN_EPD_CS      2
#define PIN_EPD_BUSY    3
#define PIN_EPD_BUSY_ALT 6
#define PIN_EPD_DC      4
#define PIN_EPD_SCK     7
#define PIN_EPD_MOSI    9
#define PIN_KEY_UP      5     /* D4: PREV */
#define PIN_KEY_OK     44     /* D7: MENU (not an RTC pin: can't wake the S3) */
#define PIN_KEY_DOWN    8     /* D9: NEXT */
#define PIN_KEY_BOOT    0     /* the XIAO's BOOT button; wakes the pass */
#define PIN_KEY_WAKE   (-1)   /* PREV, NEXT and BOOT wake it themselves */
#define PIN_BAT_ADC    (-1)   /* the XIAO ESP32-S3 has no battery sense */
#define PIN_BAT_EN     (-1)
#else
#error "SR-16: no pin map for this chip"
#endif

#else
#error "define BOARD_SR15 or BOARD_SR16"
#endif

#define EPD_NSTRIDE  ((EPD_NW + 7) / 8)
#define EPD_CSTRIDE  ((EPD_CW + 7) / 8)
#define EPD_CLEN     (EPD_CSTRIDE * EPD_CH)

#endif
