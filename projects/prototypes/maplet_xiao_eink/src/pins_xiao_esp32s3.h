/*
 * pins_xiao_esp32s3.h — Seeed XIAO ESP32S3 (Sense) on the XIAO ePaper Breakout Board.
 *
 * Breakout silk → XIAO silk → S3 GPIO (wiki.seeedstudio.com/xiao_esp32s3_getting_started):
 *   RST   D0   GPIO1
 *   CS    D1   GPIO2
 *   DC    D3   GPIO4
 *   BUSY  D5   GPIO6   (input; high = idle)
 *   SCK   D8   GPIO7
 *   MOSI  D10  GPIO9
 * BOOT button GPIO0. User LED GPIO21 (active low) — on the Sense that pin is also
 * the microSD chip select, so the LED is not used. Camera: GPIO10-18, 38-40, 47, 48.
 * USB is GPIO19/20: never reconfigure them.
 */

#ifndef PINS_XIAO_ESP32S3_H
#define PINS_XIAO_ESP32S3_H

#define SKIN_SPI_HOST   SPI2_HOST
#define SKIN_PIN_RST    1
#define SKIN_PIN_CS     2
#define SKIN_PIN_DC     4
#define SKIN_PIN_BUSY   6
#define SKIN_PIN_SCK    7
#define SKIN_PIN_MOSI   9
#define SKIN_PIN_BOOT   0
#define SKIN_PIN_SD_CS  21
#define SKIN_USB_PIN_A  19
#define SKIN_USB_PIN_B  20
#define SKIN_GPIO_MAX   48
#define SKIN_BOARD_NAME "XIAO ESP32S3 Sense"

#endif /* PINS_XIAO_ESP32S3_H */
