/*
 * pins_beetle_esp32c3.h — DFRobot Beetle ESP32-C3 (DFR0868) pin map for the
 * goggl1 prototype.
 *
 * The Fermion SSD1309 transparent OLED (DFR0934) plugs into the Beetle's
 * 18-pin GDI ribbon connector. The OLED only uses the SPI subset of that
 * connector: SCLK, MOSI, DC, RST, CS. Everything else on the ribbon (MISO,
 * backlight, touch CS, SD CS, I2C) is unconnected on the OLED side.
 *
 * Source: https://wiki.dfrobot.com/dfr0868/ — "GDI Display Interface" table.
 *
 *   FPC pin   GPIO   use here
 *   SCLK      4      SPI clock
 *   MOSI      6      SPI data to display
 *   MISO      5      (unused — the OLED has no data-out line)
 *   LCD_DC    1      data / command select
 *   LCD_RST   2      reset, active low
 *   LCD_CS    7      chip select, active low
 *   LCD_BL    10     (unused by the OLED; shared with the onboard LED)
 *   TCS       3      (unused)
 *   SD_CS     0      (unused)
 *
 * Cable orientation: the FPC's metal contacts face the white part of the
 * GDI connector. Reversed insertion is harmless, the display just stays
 * blank — so if nothing comes up, flip the ribbon first.
 *
 * GPIO10 is the onboard LED *and* LCD_BL on the ribbon. The OLED does not
 * use BL, so the LED is free for status.
 * GPIO9 is the BOOT button (pulled up, low when pressed). Holding it at
 * reset enters the ROM bootloader, otherwise it is an ordinary input.
 */

#ifndef PINS_BEETLE_ESP32C3_H
#define PINS_BEETLE_ESP32C3_H

#define GOGGL_SPI_HOST   SPI2_HOST

#define GOGGL_PIN_SCLK   4
#define GOGGL_PIN_MOSI   6
#define GOGGL_PIN_DC     1
#define GOGGL_PIN_RST    2
#define GOGGL_PIN_CS     7

#define GOGGL_PIN_LED    10   /* onboard LED, active high */
#define GOGGL_PIN_BUTTON 9    /* BOOT button, active low */

#endif /* PINS_BEETLE_ESP32C3_H */
