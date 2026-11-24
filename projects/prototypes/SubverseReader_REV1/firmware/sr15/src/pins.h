/*
 * pins.h: Elecrow CrowPanel ESP32 4.2in E-Paper HMI (DIE07300S), from Elecrow's
 * Arduino examples and schematic (Elecrow-RD/CrowPanel-ESP32-4.2-E-paper-HMI-
 * Display-with-400-300). Same on the "green circular sticker" board revision.
 *
 * Keys are active low. Held portrait for SR-15, the dial sits on the bottom
 * edge with MENU and EXIT either side of it.
 *
 * Licensed under the Apache License, Version 2.0
 */
#ifndef SR15_PINS_H
#define SR15_PINS_H

/* SSD1683 panel, 400 x 300, on SPI2 */
#define PIN_EPD_SCK    12
#define PIN_EPD_MOSI   11
#define PIN_EPD_CS     45
#define PIN_EPD_DC     46
#define PIN_EPD_RST    47
#define PIN_EPD_BUSY   48     /* high while the controller works */
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

#define PIN_LED        41     /* the only LED; firmware-driven, not a charge light. On the */
#define PIN_PWR_EN     41     /* green-sticker revision Elecrow drives it high as "power enable" */

#endif
