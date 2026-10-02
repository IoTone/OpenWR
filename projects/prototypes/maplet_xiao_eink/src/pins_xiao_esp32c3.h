/*
 * pins_xiao_esp32c3.h — Seeed XIAO ESP32C3 on the XIAO ePaper Breakout Board.
 *
 * Breakout (wiki.seeedstudio.com/XIAO-eInk-Expansion-Board) → XIAO silk → C3 GPIO:
 *   RST   D0   GPIO2
 *   CS    D1   GPIO3
 *   DC    D3   GPIO5
 *   BUSY  D5   GPIO7   (input; high = idle)
 *   SCK   D8   GPIO8
 *   MOSI  D10  GPIO10
 * The XIAO ESP32C3 has no user LED and no user button; BOOT is GPIO9.
 */

#ifndef PINS_XIAO_ESP32C3_H
#define PINS_XIAO_ESP32C3_H

#define SKIN_SPI_HOST   SPI2_HOST
#define SKIN_PIN_RST    2
#define SKIN_PIN_CS     3
#define SKIN_PIN_DC     5
#define SKIN_PIN_BUSY   7
#define SKIN_PIN_SCK    8
#define SKIN_PIN_MOSI   10
#define SKIN_PIN_MISO   -1
#define SKIN_PIN_BOOT   9
#define SKIN_USB_PIN_A  18
#define SKIN_USB_PIN_B  19
#define SKIN_GPIO_MAX   21
#define SKIN_BOARD_NAME "XIAO ESP32C3"

#endif /* PINS_XIAO_ESP32C3_H */
