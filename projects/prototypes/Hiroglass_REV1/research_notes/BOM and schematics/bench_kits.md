# Pod-0 bench kits (DigiKey / Mouser only)

Research date: 2026-10-14. USD, qty-1 prices.

**Verification status.** DigiKey returned HTTP 403 and Mouser timed out on direct fetches, so no distributor page was read live today. Every price and stock figure below comes from a search-indexed snapshot of the DigiKey or Mouser page, or from the Adafruit product page, which shows DigiKey pricing. The Status column says which. Lines marked **UNVERIFIED** are part numbers or prices from memory or naming pattern, with no distributor hit found. Check every line in the cart before ordering.

Pod-0 background: `reports/Open hardware XR wearable approaches.md`, section "REV1 sequencing: Pod-0 first". goggl1 pin map: `projects/prototypes/esp32c3_beetle_goggl1/src/pins_beetle_esp32c3.h`.

## Shared design choices

- **OLED.** Adafruit 938: 1.3" 128x64 SSD1306, STEMMA QT, onboard 3.3 V regulator and level shifting. Default interface is I2C, at 0x3D by default (UNVERIFIED; 0x3C with the jumper). Switching to SPI means cutting two jumpers on the back and soldering the 8-pin header. Alternate: Adafruit 326, the 0.96" version of the same board (DigiKey 1528-1220-ND ~$17.50 / Mouser 485-326 ~$17.42, aggregator snapshot).
- **IMU.** SparkFun SEN-22397, a Qwiic BMI270, 1"x1", 0x68 by default (0x69 with the ADR jumper). Qwiic boards are **3.3 V only**, with no regulator. I found no Adafruit BMI270 breakout at DigiKey or Mouser. Alternates are Adafruit LSM6DSOX 4438 or ISM330DHCX 4502 (not priced).
- **ALS.** Adafruit 4162 VEML7700, STEMMA QT, 0x10 fixed (it cannot be changed).
- **I2C bus.** One bus, daisy-chained: OLED (in Option B) -> BMI270 -> VEML7700. Three pull-up sets in parallel (Adafruit ~10k x2, SparkFun 2.2k) is fine at 100-400 kHz. You can cut SparkFun's I2C jumper if edges look soft.
- **Puck.** Adafruit 4755 (bq24074) has USB-C input, a JST-PH BATT port and a JST-PH LOAD port, plus an 11-pin header. **Default charge current is 1 A. A 500 mAh cell must be set to 500 mA with the solder jumper (one blob).** The cold-charge NTC needs a 10k NTC soldered in (optional for the bench). Do not use the DC/solar input, because OUT tracks the input and could exceed the MCU's VIN rating.
- **Power to the MCU.** 4755 LOAD/OUT goes through the slide switch to the MCU **5V/VIN pin**, never to 3V3 and never to the Beetle's BAT pin. OUT is about VBUS (5 V) with USB in and about VBAT (3.0-4.2 V) on battery. The board's 3.3 V LDO drops out below about 3.4 V, which is acceptable for the bench. Do not also hang a LiPo on the Beetle's own BAT pin (it has an onboard TP4057 charger).
- **ESP32-C3 pins to avoid.** GPIO2, 8 and 9 are strapping pins (9 is BOOT). GPIO18/19 are USB D-/D+ (USB-CDC console). GPIO20/21 are UART0 RX/TX (free when the console is on USB, but the ROM prints on 21 at boot).

## Option A: "Reuse goggl1" (Beetle ESP32-C3), SPI OLED

Minimal soldering: header strips on the Beetle (if not pre-fitted, UNVERIFIED) and on the OLED, cut two OLED jumpers, and one charge-current jumper on the 4755.

| # | Item | Mfr / MPN | DigiKey PN | Mouser PN | $ qty1 | Stock (snapshot) | Qty/kit | Status | URL |
|---|---|---|---|---|---|---|---|---|---|
| 1 | MCU | DFRobot DFR0868 Beetle ESP32-C3 | 1738-DFR0868-ND | not found | ~$6.90 (US price not seen; CA$12.55, EUR6.82) | 55-140 on regional DK sites | 1 (0 if reusing goggl1 boards) | DK regional snapshots | https://www.digikey.com/en/products/detail/dfrobot/DFR0868/16678683 |
| 2 | OLED 1.3" SSD1306 | Adafruit 938 | 1528-1512-ND | not found | $19.95 | 183-362 | 1 | DK snapshot | https://www.digikey.com/en/products/detail/adafruit-industries-llc/938/5774238 |
| 3 | IMU BMI270 Qwiic | SparkFun SEN-22397 | 1568-22397-ND | 474-SEN-22397 | $18.50 | DK 33; Mouser 113 (+208 due 10/16/2026) | 1 | DK+Mouser snapshot | https://www.digikey.com/en/products/detail/sparkfun-electronics/22397/21703804 |
| 4 | ALS VEML7700 | Adafruit 4162 | 1528-2891-ND | not found | $4.95 | DK 125 | 1 | DK snapshot | https://www.digikey.com/en/products/detail/adafruit-industries-llc/4162/9997696 |
| 5 | Charger bq24074 | Adafruit 4755 | 1528-4755-ND | 485-4755 (no price; OEM/EMS only) | $14.95 | DK not shown; Adafruit in stock | 1 | DK snapshot | https://www.digikey.com/en/products/detail/adafruit-industries-llc/4755/13231325 |
| 6 | LiPo 3.7 V 500 mAh JST-PH | Adafruit 1578 | 1528-xxxx (PN not seen) | 485-1578 | $7.95 | DK ~570 (aggregator) | 1 | aggregator | https://www.mouser.com/ProductDetail/485-1578 |
| 7 | QT to male-header cable 150 mm | Adafruit 4209 | 1528-4209-ND (UNVERIFIED) | 485-4209 (UNVERIFIED) | $0.95 | ? | 1 | Adafruit price | https://www.adafruit.com/product/4209 |
| 8 | QT-QT cable 50 mm | Adafruit 4399 | 1528-4399-ND | 485-4399 (UNVERIFIED) | ~$0.95 | ? | 1 | DK listing exists | https://www.digikey.com/en/products/detail/adafruit-industries-llc/4399/10824268 |
| 9 | JST-PH 2-pin extension with on/off switch | Adafruit 1131 | UNVERIFIED | UNVERIFIED | ~$2.95 | ? | 1 | UNVERIFIED | https://www.adafruit.com/product/1131 |
| 10 | JST-PH 2-pin to male header 200 mm | Adafruit 3814 | UNVERIFIED | UNVERIFIED | ~$0.75 | ? | 1 | UNVERIFIED | https://www.adafruit.com/product/3814 |
| 11 | Half-size breadboard | Adafruit 64 | UNVERIFIED | UNVERIFIED | ~$5.00 | ? | 1 | UNVERIFIED | https://www.adafruit.com/product/64 |
| 12 | Jumpers M/M 6" x20 | Adafruit 1957/758 | UNVERIFIED | UNVERIFIED | ~$3.95 | ? | 1 | UNVERIFIED | https://www.adafruit.com/product/758 |
| 13 | USB-A to USB-C cable | Adafruit 4474 | UNVERIFIED | UNVERIFIED | ~$2.95 | ? | 2 | UNVERIFIED | https://www.adafruit.com/product/4474 |
| 14 | 0.1" male header strip (team pack) | Adafruit 392 | UNVERIFIED | UNVERIFIED | ~$4.95 | ? | 1 per team | UNVERIFIED | https://www.adafruit.com/product/392 |

Alternative switch: Adafruit 805 breadboard SPDT (~$0.95-1.25, UNVERIFIED at DK), placed inline between OUT and VIN on the breadboard.

**Cost.** Per kit about **$93.65**. Three kits are $280.95, plus a $4.95 header pack, for **about $285.90**. That drops by about $20.70 if the three goggl1 Beetles are reused.

### Option A pin labels

- **Beetle DFR0868.** Power: `VIN` (5 V in), `3V3` (out), `GND`, `BAT`. GPIOs 0-10, 20, 21 are silkscreened as numbers (exact glyphs not verified). The Arduino variant defaults are SDA=8, SCL=9, SCK=4, MOSI=6, MISO=5, SS=7, LED=10, TX=21, RX=20. The 18-pin GDI connector is unused here.
- **Adafruit 938, 8-pin header.** `GND, Vin, 3v3, CS, Rst, DC, Clk, Data` (order and case from memory; verify against the silkscreen). Plus 2x STEMMA QT.
- **SparkFun 22397.** Qwiic x2, plus PTH `3V3, GND, SDA, SCL, INT1, INT2` (UNVERIFIED). Jumpers: ADR, I2C pull-ups, LED.
- **Adafruit 4162.** `VIN, 3Vo, GND, SCL, SDA`, plus 2x STEMMA QT.
- **Adafruit 4755.** USB-C, `BATT` JST-PH, `LOAD` JST-PH, and an 11-pin header. Expected labels are `IN/VIN, OUT, BAT, GND, CHG, PGOOD, CE, EN1, EN2, TS` (UNVERIFIED; check the learn guide pinout page).

### Option A wiring

| From | To | Note |
|---|---|---|
| Beetle GPIO4 (SCK) | 938 `Clk` | goggl1 SCLK |
| Beetle GPIO6 (MOSI) | 938 `Data` | goggl1 MOSI |
| Beetle GPIO7 (SS) | 938 `CS` | goggl1 CS |
| Beetle GPIO1 | 938 `DC` | goggl1 DC |
| Beetle GPIO2 | 938 `Rst` | goggl1 RST. GPIO2 is a strapping pin and must be high at boot. If boot misbehaves, move RST to GPIO0 or leave `Rst` open (the board has auto-reset, UNVERIFIED) |
| Beetle 3V3 | 938 `Vin` | |
| Beetle GND | 938 `GND` | |
| Beetle GPIO5 | QT cable 4209 blue (SDA) -> SparkFun 22397 Qwiic | GDI MISO and TCS are unused, so GPIO5/3 avoid strapping 8/9 |
| Beetle GPIO3 | QT cable 4209 yellow (SCL) | |
| Beetle 3V3 | QT cable 4209 red (3.3 V) | Qwiic is 3.3 V only |
| Beetle GND | QT cable 4209 black | |
| SparkFun 22397 Qwiic #2 | 4162 STEMMA QT (cable 4399) | BMI270 0x68, VEML7700 0x10 |
| LiPo 1578 | 4755 `BATT` | check polarity (Adafruit cell into Adafruit board is OK) |
| 4755 `LOAD` | 1131 switch ext -> 3814 lead red | |
| 3814 red | Beetle `VIN` | not `BAT` and not `3V3` |
| 3814 black | Beetle `GND` | |
| USB-C | 4755 USB-C | charging. The Beetle's own USB-C is for flashing and console; unplug the puck lead when flashing, or accept that VBUS backfeeds through the switch |

Firmware change: none for the display pins (goggl1 SPI map is unchanged; the SSD1306 init differs from SSD1309 slightly). Add the I2C bus on SDA=5, SCL=3.

## Option B: "XIAO / Qwiic" (Adafruit QT Py ESP32-C3), all-I2C, zero solder for the bench

All signals run over STEMMA QT, with the OLED in its default I2C mode. On the bench, power comes from the QT Py's USB-C. Running from the puck needs one soldered header (QT Py `5V`/`GND`) or the charger's 11-pin header. That is the only soldering, plus the 500 mA charge jumper.

| # | Item | Mfr / MPN | DigiKey PN | Mouser PN | $ qty1 | Stock (snapshot) | Qty/kit | Status | URL |
|---|---|---|---|---|---|---|---|---|---|
| 1 | MCU | Adafruit 5405 QT Py ESP32-C3 | 1528-5405-ND (UNVERIFIED) | not found | $9.95 (DK price shown on Adafruit page) | Adafruit page showed **out of stock**; most retailers sold out | 1 | Adafruit page | https://www.adafruit.com/product/5405 |
| 1-alt | MCU | Seeed 113991054 XIAO ESP32C3 | 113991054 (DK PN not seen) | 713-113991054 (UNVERIFIED) | $4.99 | DK ~2.3k, Mouser ~1.3-1.9k (aggregator) | - | aggregator | https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html |
| 2 | OLED 1.3" SSD1306 | Adafruit 938 | 1528-1512-ND | - | $19.95 | 183-362 | 1 | DK snapshot | as Option A |
| 3 | IMU BMI270 | SparkFun SEN-22397 | 1568-22397-ND | 474-SEN-22397 | $18.50 | DK 33 / Mouser 113 | 1 | snapshot | as Option A |
| 4 | ALS | Adafruit 4162 | 1528-2891-ND | - | $4.95 | DK 125 | 1 | snapshot | as Option A |
| 5 | Charger | Adafruit 4755 | 1528-4755-ND | 485-4755 | $14.95 | - | 1 | snapshot | as Option A |
| 6 | LiPo 500 mAh | Adafruit 1578 | - | 485-1578 | $7.95 | ~570 | 1 | aggregator | as Option A |
| 7 | QT-QT cable 100 mm | Adafruit 4210 | UNVERIFIED (DK 4528 is a 100 mm QT cable, 476 in stock) | UNVERIFIED | $0.95 | - | 3 | Adafruit price | https://www.adafruit.com/product/4210 |
| 8 | Switch extension | Adafruit 1131 | UNVERIFIED | UNVERIFIED | ~$2.95 | - | 1 | UNVERIFIED | |
| 9 | JST-PH to male header | Adafruit 3814 | UNVERIFIED | UNVERIFIED | ~$0.75 | - | 1 | UNVERIFIED | |
| 10 | Half breadboard | Adafruit 64 | UNVERIFIED | | ~$5.00 | | 1 | UNVERIFIED | |
| 11 | Jumpers | Adafruit 758 | UNVERIFIED | | ~$3.95 | | 1 | UNVERIFIED | |
| 12 | USB-A to C cable | Adafruit 4474 | UNVERIFIED | | ~$2.95 | | 2 | UNVERIFIED | |
| 13 | Header strip (team) | Adafruit 392 | UNVERIFIED | | ~$4.95 | | 1/team | UNVERIFIED | |

**Cost.** Per kit about **$97.65**. Three kits are $292.95, plus the $4.95 header pack, for **about $297.90**. With the XIAO fallback it is about $93.60 per kit, after adding a 4209 QT-to-header cable and soldering the XIAO headers.

The SparkFun Qwiic Pocket ESP32-C3 (DEV-22925) has an onboard Qwiic port, a LiPo charger and a JST, but it was not priced or stocked here.

### Option B pin labels

- **QT Py ESP32-C3.** One side: `A0 A1 A2 A3 SDA SCL TX`. Other side: `5V GND 3V MO MI SCK RX`. GPIO map from the Arduino variant: A0=4, A1=3, A2=1, A3=0, SDA=5, SCL=6 (STEMMA QT shares these), TX=21, RX=20, SCK=10, MO=7, MI=8 (strapping), NeoPixel=2 (strapping), button=9. Silkscreen glyphs are from memory.
- **XIAO ESP32C3** (fallback). `D0..D10, 5V, GND, 3V3`. D0=GPIO2, D1=3, D2=4, D3=5, D4/SDA=6, D5/SCL=7, D6/TX=21, D7/RX=20, D8/SCK=8, D9/MISO=9, D10/MOSI=10. From memory, UNVERIFIED. Note that SCK, MISO and D0 sit on strapping pins. BAT+/- pads are on the underside.

### Option B wiring

| From | To | Note |
|---|---|---|
| QT Py STEMMA QT | 938 STEMMA QT #1 | GPIO5 SDA / GPIO6 SCL, 3.3 V. OLED 0x3D (or 0x3C) |
| 938 STEMMA QT #2 | SparkFun 22397 Qwiic | BMI270 0x68 |
| 22397 Qwiic #2 | 4162 STEMMA QT | VEML7700 0x10 |
| QT Py USB-C | laptop | bench power plus console |
| LiPo 1578 | 4755 `BATT` | set charge jumper to 500 mA |
| 4755 `LOAD` -> 1131 switch -> 3814 red/black | QT Py `5V` / `GND` (soldered header) | puck mode. Unplug USB from the QT Py when on the puck, or accept the backfeed |

Optional SPI OLED on the QT Py (cut the 938 jumpers and solder its header): `SCK`(GPIO10) -> `Clk`, `MO`(GPIO7) -> `Data`, `A0`(GPIO4) -> `CS`, `A1`(GPIO3) -> `DC`, `A2`(GPIO1) -> `Rst`, `3V` -> `Vin`, `GND` -> `GND`. Sensors stay on STEMMA QT.

## Sources

- DigiKey DFR0868 (regional snapshots): https://www.digikey.ca/en/products/detail/dfrobot/DFR0868/16678683
- DFRobot wiki DFR0868: https://wiki.dfrobot.com/dfr0868/
- Arduino-ESP32 variants: https://github.com/espressif/arduino-esp32/tree/master/variants/dfrobot_beetle_esp32c3 and .../adafruit_qtpy_esp32c3
- DigiKey SparkFun 22397: https://www.digikey.com/en/products/detail/sparkfun-electronics/22397/21703804
- DigiKey Adafruit 4162 (US mirror): https://azcus.digikey.com/en/products/detail/adafruit-industries-llc/4162/9997696
- DigiKey Adafruit 938 (US mirror): https://azcus.digikey.com/en/products/detail/adafruit-industries-llc/938/5774238
- DigiKey Adafruit 4755: https://www.digikey.com/en/products/detail/adafruit-industries-llc/4755/13231325
- Adafruit 938: https://www.adafruit.com/product/938 ; Adafruit 4755: https://www.adafruit.com/product/4755 ; Adafruit 5405: https://www.adafruit.com/product/5405
- Adafruit 326 aggregator: https://www.electronicsdatasheets.com/manufacturers/adafruit/parts/326
- Adafruit 1578 at Mouser: https://www.mouser.com/ProductDetail/485-1578
- DigiKey 4399: https://www.digikey.com/es/products/detail/adafruit-industries-llc/4399/10824268
- XIAO ESP32C3 aggregator: https://www.flux.ai/jecstronic/113991054~lch.md
