# Sourceable displays and support parts for a low-cost monocular HUD (Hiroglass REV1)

Research date: 2026-10-13. Prices are USD. Many figures come from search-indexed distributor snapshots of uncertain age (some aggregator snapshots are years old), so each one is marked with how it was observed. Only two pages were fetched live today: DigiKey Orient Display AOM12864A0-0.96WW and DigiKey Winstar WEO012864DWPP3N00006. Check everything else on the live page before ordering.

Not repeated here (covered by earlier research): Sony ECX, SeeYA 0.49", TI DLPDLCR2000EVM, Himax LCoS, and JBD microLED.

## Sourcing table (summary)

| Part | Role | Distributor(s) seen | Qty 1 | Qty 10/100 | Stock seen | Interface | Brightness | Alternates | Observed |
|---|---|---|---|---|---|---|---|---|---|
| Orient Display AOM12864A0-0.96WW | 0.96" 128x64 PMOLED, SSD1306 | DigiKey | $11.57 | $8.931 / $7.389 | 0 (8-wk lead; 1 unit est. 08-Dec-2026) | I2C/SPI/parallel | not listed | Winstar WEO012864D, Raystar REX012864 (same SSD1306 128x64 class) | live fetch 2026-10-13 |
| Winstar WEO012864DWPP3N00006 | 0.96" 128x64 PMOLED white, SSD1306BZ | DigiKey Marketplace | $5.24 | no breaks listed | 4 | I2C/SPI | not listed | above | live fetch 2026-10-13 |
| Raystar REX012864MXPP3N00001 | 0.96" 128x64 PMOLED yellow/blue, SSD1306 | TME (no DigiKey hit) | $6.89 | $4.72 (10), $4.43 (25) | 144 | SSD1306 | n/a | above | search snapshot |
| DFRobot DFR0413 | 0.96" SSD1306 breakout | DigiKey | $23.90 | n/a | not stocked, 9-wk | | | | search snapshot |
| Newhaven RGB OLED glass (0.95"/1.27"/1.5") | colour PMOLED (SSD1331/SSD1351 class) | Mouser | n/a | n/a | n/a | SPI | 90-100 cd/m² | | Mouser listing |
| Bosch BMI270 | 6-axis IMU | DigiKey, Mouser, LCSC | $4.23 | ~$2.83 @1k | ~50,000 (DigiKey) | I2C/SPI | | ICM-42688-P, LSM6DSOX | snapshot, possibly stale |
| TDK ICM-42688-P | 6-axis IMU | DigiKey | n/a | ~$2.65 @5k reel | 0, 45-wk lead | I2C/SPI | | BMI270 | snapshot, conflicting |
| Vishay VEML7700-TR / -TT | ambient light sensor | DigiKey, Mouser | $1.44 (TR), $1.53 (TT DK), $1.58 (TT Mouser) | ~$0.81 @11.5k | 7,570 (DK TR); 4,895 (Mouser TT) | I2C | | OPT3001 (not verified) | snapshot |
| TI BQ24074RGTT | 1-cell Li-ion charger, power path | DigiKey | $3.19 | $1.9684 @100 | 9,247 cut tape | standalone | | BQ25180, MAX77654, nPM1300 | Findchips snapshot |
| Nordic nPM1300 | PMIC: charger + bucks + LDOs + fuel gauge | Mouser, LCSC, RS, Arrow/Verical/Future | $2.70 (Mouser), ~$2.69 (LCSC) | $1.53-1.58 @reel | Mouser 0 (10,479 on order, ETA 8/14/2026); LCSC 835 | I2C | | MAX77654 | snapshot |
| ADI MAX77654 | PMIC: SIMO + charger | ADI direct, DigiKey | $6.96 (old DK snapshot) | $4.518 @100 (old); ADI 1k $2.84-3.15 | some SKUs in stock at ADI | I2C | | nPM1300 | ADI page + stale snapshot |
| Espressif ESP32-C3-MINI-1-N4 | BLE5/Wi-Fi certified module | DigiKey, Mouser | $3.26 | $2.819 (10), $2.4502 (100) | 2,779 | | | -H4 variant $3.28 (Mouser) | snapshot |
| Raytac MDBT50Q-1MV2 / -P1MV2 | nRF52840 certified module | DigiKey (marketplace, $25 flat ship), Adafruit | $6.15 / $6.00 | n/a | 50 / 82 | | | u-blox, Ezurio (not priced) | snapshot |
| Raytac AN54LQ-15 / AN54LV-15 | nRF54L15 certified module | DigiKey | ~$4.10 / ~$4.30 | n/a | active | | | Ezurio BL54L15 (0-7,700 by SKU) | snapshot |
| Nordic nRF54L15-CAAA-R7 | bare SoC | DigiKey | $4.15 | n/a | 561 | | | | ~700-day-old highlight page |

## Small passive-matrix and AMOLED OLED panels and modules

### Takeaway
Monochrome 0.96" 128x64 SSD1306 PMOLEDs come from several vendors (Orient Display, Winstar, Raystar, plus breakouts) at about $5-12 for one unit and $7.39 at 100 on DigiKey. They use the same controller and resolution, so they can stand in for each other at the firmware level, but no one stocks them in depth. Pin-for-pin FPC compatibility is not verified. I found no luminance figures for the mono parts. Newhaven's colour PMOLEDs on Mouser are rated only 90-100 cd/m², far too dim for snow without a lot of optical help. I found no distributor source for the RM67162 AMOLED panel itself.

### Cited Findings
- DigiKey Orient Display AOM12864A0-0.96WW (SSD1306, 0.96", 128x64, I2C/Parallel/SPI; 23.74 x 12.86 mm active area): $11.57 (1), $8.931 (10), $7.389 (100). 0 in stock, 8-week factory lead time, 1 unit estimated 08-Dec-2026. No luminance or temperature range listed. Fetched 2026-10-13. [DigiKey](https://www.digikey.com/en/products/detail/orient-display/AOM12864A0-0-96WW/22531883). A search snapshot earlier the same day showed $11.13 (1), $6.25 (1,000) and 30 in stock, so stock changes quickly. [DigiKey azcus mirror](https://azcus.digikey.com/en/products/detail/orient-display/AOM12864A0-0-96WW/22531883)
- DigiKey Winstar WEO012864DWPP3N00006 (passive white OLED, SSD1306BZ, I2C/SPI, 0.96" 128x64): $5.24 at qty 1, 4 in stock. It is a DigiKey Marketplace item with a purchase limit per 30 days, ships from a tray of 1,000, and lists no luminance or temperature. Fetched 2026-10-13. [DigiKey](https://www.digikey.com/en/products/detail/winstar-display/WEO012864DWPP3N00006/20533252)
- Winstar makes several 0.96" variants with different controllers: WEO012864D (SSD1306), WEO012864C (SH1106), WEO012864AD (ST7315), WEO012864V (SSD1315). [Winstar WEO012864D](https://www.winstar.com.tw/products/oled-module/graphic-oled-display/0_96-oled.html); [Winstar WEO012864C](https://www.winstar.com.tw/products/oled-module/graphic-oled-display/weo012864c.html); [Winstar SSD1315](https://www.winstar.com.tw/products/oled-module/graphic-oled-display/ssd1315-oled.html)
- Raystar REX012864MXPP3N00001 (SSD1306, yellow/blue) at TME: $6.89 (1), $4.72 (10), $4.43 (25), 144 in stock. Raystar REX012864DBPP3N00F00 (SSD1306BZ) is out of stock. The search found no DigiKey listing for REX012864. [TME](https://www.tme.com/us/en-us/details/rex012864mxpp3n01/graphical-oled-displays/raystar-optronics/rex012864mxpp3n00001/)
- DFRobot DFR0413 SSD1306 0.96" board on DigiKey: $23.90, not stocked, 9-week lead. [DigiKey](https://www.digikey.com/en/products/detail/dfrobot/DFR0413/7087175)
- Newhaven full-colour OLED glass on Mouser: 1.5" 128x128 rated 90 cd/m², 0.95" 96x64 and 1.27" 128x96 rated 100 cd/m². [Mouser Newhaven full colour OLED](https://www.mouser.hk/newhaven-full-color-oled-displays)
- The LILYGO T-Display-S3 AMOLED uses a 1.91" 536x240 RM67162 panel over QSPI. LILYGO's driver code sets brightness as an 8-bit value in register 0x51 and gives no nit rating. [espboards.dev](https://www.espboards.dev/esp32/lilygo-t-display-s3-amoled/); [LILYGO rm67162.cpp](https://github.com/Xinyuan-LilyGO/T-Display-S3-AMOLED/blob/main/examples/factory/rm67162.cpp)
- Mouser carries Microtips OLED and AMOLED modules, a possible extra AMOLED source that was not priced. [Mouser Microtips](https://www.mouser.com/en/new/microtips/microtips-oled-modules/)

### Inferences
- The SSD1306 0.96" 128x64 class is the safest multi-source display. Orient, Winstar and Raystar all ship SSD1306 glass, and goggl1 firmware already speaks it. Whether the FPCs share a pinout needs a datasheet comparison. Plan the carrier PCB for one footprint, or make it swappable.
- Typical PMOLED luminance (about 100-150 cd/m² for mono parts, by analogy with Newhaven's 90-100 cd/m² RGB ratings) is low for snow use. Outdoor readability would have to come from the optics (an occluded look-around eyepiece or a high-reflectivity combiner) rather than from the panel.

### Gaps
- No cd/m² figures for Orient, Winstar or Raystar mono 0.96" parts. Their datasheets were not fetched.
- I found no DigiKey or Mouser listing or price for SH1106 1.3", SSD1309 1.54", SSD1327 or SSD1351 modules from Midas, Visionox, Univision or Densitron, or for Adafruit and Waveshare breakouts (not searched within budget).
- I found no distributor listing for a bare RM67162 AMOLED panel, and no nit rating for it.

## Small high-brightness TFT/IPS (ST7789/GC9A01/ST7735, 0.85"-1.3")

### Takeaway
I found no >800-nit TFT in the 0.85"-1.5" range on DigiKey or Mouser. Common 1.3" 240x240 ST7789 IPS modules publish no nit rating. The smallest 1000-nit sunlight-readable IPS parts I found are 2.4" (Orient Display catalogue) and 3.4" (GL Electronics), both too big for a monocular eyepiece without magnifying optics.

### Cited Findings
- Common 1.3" 240x240 ST7789 (or ST7789V3) IPS SPI modules at hobby retailers list no brightness. [universal-solder](https://www.universal-solder.ca/product/1-3-inch-display-tft-ips-with-st7789-240x240-pixel-spi/); [ShillehTek](https://shillehtek.com/products/tft-lcd-1-3-240x240-st7789-esp32-arduino)
- Orient Display's high-brightness range is 1000-nit IPS in 2.4", 2.8", 3.5", 4.3", 7" and 10.1" sizes. The 7" and 10.1" parts operate from -20 to 70 °C. [Orient Display high brightness](https://orientdisplay.com/tft-displays/tft-panels/high-brightness/); [Orient 10.1"](https://www.orientdisplay.com/products/afy1280800a0-10-1inth/)
- A 3.4" 480x480 SPI/RGB IPS module is rated 1000 cd/m², -20 to 70 °C. [GL Electronics](https://lcdscreenmfg.com/product/3-4inch-high-brightness-1000nits-tft-lcd-display-480480-spi-rgb-ips-lcd-module/)
- Orient Display says a typical TFT is about 250-450 nits and that 800-1000 nits is the usual sunlight-readable target. [Orient knowledge base](https://orientdisplay.com/knowledge-base/tft-basics/sunlight-readable-tft-lcd/)

### Inferences
- For a combiner design, a bright TFT has an advantage over a PMOLED because its LED backlight can be overdriven or replaced. That is custom work, though, and not an off-the-shelf part.

### Gaps
- I did not find DigiKey or Mouser prices for small ST7789, GC9A01 or ST7735 bare panels, for example from Newhaven, Adafruit or Waveshare.
- I found no small (<1.5") high-brightness TFT at a major distributor. Absence from search results is not proof that none exists.

## Micro-OLED or microdisplay stocked at Mouser or DigiKey

### Takeaway
I found no micro-OLED microdisplay (Kopin, eMagin/Samsung, Microoled, Lumicore) stocked at DigiKey or Mouser. The "microOLED" items DigiKey carries are 4D Systems smart PMOLED modules, not near-eye microdisplays. Retail 0.39" micro-OLEDs still come only from Tindie and DisplayModule-type sellers.

### Cited Findings
- DigiKey's "microOLED Smart Display Modules" highlight is a 4D Systems PMOLED product, not a microdisplay. [DigiKey 4D Systems](https://www.digikey.com/en/product-highlight/4/4d-systems/microoled-smart-display-modules)
- 0.39" 1024x768 micro-OLED, MIPI, 600 nits, on Tindie. [Tindie](https://www.tindie.com/products/oled-modules/micro-ar-display-039-inch-1024x768-oled-display/)
- DisplayModule 0.39" 1920x1080 micro-OLED, MIPI plus I2C, $139.00, -10 to 80 °C. [DisplayModule](https://www.displaymodule.com/products/0-39-inch-micro-oled-display-1920x1080-with-mipi-i2c)
- Samsung Display bought eMagin in October 2023 for about $218M, so eMagin parts now go through Samsung. [Wikipedia eMagin](https://en.wikipedia.org/wiki/EMagin)
- A search for Kopin distributor stock returned only stock-quote pages. Kopin's microdisplay technologies include AMLCD, FLCoS, MicroLED and OLED. [Coinbase KOPN profile](https://www.coinbase.com/stock/KOPN)

### Inferences
- The project's "avoid single-supplier parts" rule effectively rules out micro-OLEDs for REV1. PMOLED or TFT plus optics is the multi-source path.

### Gaps
- I did not check the DigiKey or Mouser parametric filters directly for Kopin or Microoled listings. Search alone cannot prove absence.

## Support parts (IMU, ALS, charger/PMIC, BLE modules, RP2350, nRF54L15)

### Takeaway
Most support parts are well multi-sourced and cheap. BMI270 (about 50k at DigiKey, also Mouser and LCSC), VEML7700 (thousands at DigiKey and Mouser), BQ24074 and ESP32-C3-MINI-1 are all deep-stock. ICM-42688-P showed 0 stock with a long lead time, so it should be the alternate rather than the primary. nPM1300 is stocked at LCSC and RS, with Mouser on back order. nRF52840 and nRF54L15 modules from Raytac are on DigiKey, as marketplace items with a $25 flat shipping fee for MDBT50Q.

### Cited Findings
- BMI270: DigiKey $4.23 (1), ~$2.83 (1,000), ~50,000 in stock (page possibly stale). Bosch lists DigiKey, Mouser and LCSC as stockists and the part as in production. [DigiKey](https://www.digikey.com/en/products/detail/bosch-sensortec/BMI270/9974486); [Bosch](https://www.bosch-sensortec.com/en/products/motion-sensors/imus/bmi270)
- ICM-42688-P: DigiKey 0 in stock, 45-week standard lead, ~$2.65 at a 5k reel. Another snapshot gives a 2-week lead, so the sources conflict. The QCIOT-ICM42688P eval board is $55.51 with 48 in stock. [DigiKey ICM-42688-P](https://www.digikey.com/en/products/detail/tdk-invensense/icm-42688-p/10824934); [DigiKey QCIOT](https://www.digikey.com/en/products/detail/tdk-invensense/QCIOT-ICM42688P/25676496)
- SlimeVR keeps a comparison of IMU drift and quality across BMI270, ICM-42688 and LSM6DSx parts. [SlimeVR IMU comparison](https://docs.slimevr.dev/diy/imu-comparison.html)
- VEML7700-TR: DigiKey $1.44 (1), ~$0.81 (11.5k), ~7,570 in stock. VEML7700-TT: DigiKey $1.53 (12,247 in stock, via TrustedParts) and Mouser $1.58 (4,895 in stock). Adafruit sells a STEMMA QT breakout. [DigiKey TR](https://www.digikey.com/en/products/detail/vishay-semiconductor-opto-division/VEML7700-TR/5820243); [Mouser TT](https://www.mouser.com/en/ProductDetail/Vishay-Semiconductors/VEML7700-TT?qs=BcfjnG7NVaXdL6DJFdWbdw%3D%3D); [Adafruit 4162](https://www.adafruit.com/product/4162)
- BQ24074RGTT: $3.19 (1), $1.9684 (100), 9,247 in cut tape at DigiKey per Findchips. BQ24074EVM $70.60. [Findchips](https://www.findchips.com/search/bq24074); [DigiKey BQ24074](https://www.digikey.com/en/products/base-product/texas-instruments/296/BQ24074/65988)
- nPM1300-QEAA-R7 at Mouser: 0 in stock, 10,479 on order, ETA 8/14/2026, 20-week lead time, $2.70 falling to $1.53 at reel. nPM1300-CAAA-R7 (WLCSP) at LCSC: 835 in stock, ~$2.69 falling to $1.58. Octopart shows Verical, Arrow and Future quotes at ~$1.62. RS UK shows 1,440 in stock on one listing. [Mouser](https://www.mouser.com/ProductDetail/Nordic-Semiconductor/nPM1300-QEAA-R7?qs=3pZoU%2F6IRTj0hxAp51B2zA%3D%3D); [LCSC](https://www.lcsc.com/product-detail/C25346894.html); [Octopart](https://octopart.com/part/nordic-semiconductor/NPM1300-QEAA-R); [RS](https://uk.rs-online.com/web/p/battery-management/2796397)
- MAX77654: ADI 1k pricing $2.84-3.15, mixed SKU stock. An old DigiKey snapshot shows $6.96 (1) and $4.518 (100). [ADI](https://www.analog.com/en/products/max77654.html)
- ESP32-C3-MINI-1-N4 at DigiKey: $3.26 (1), $2.819 (10), $2.4502 (100), 2,779 in stock. -N4-A: $1.99 at a 650-unit reel. -H4 at Mouser: $3.28 (1), ~$2.84 (10). -H4 at DigiKey: 0 available. [DigiKey N4](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-C3-MINI-1-N4/13877574); [Mouser H4](https://www.mouser.com/ProductDetail/Espressif-Systems/ESP32-C3-MINI-1-H4?qs=stqOd1AaK78JkwNX13Q9hQ%3D%3D)
- Raytac MDBT50Q-1MV2 (nRF52840): DigiKey $6.15, 50 in stock, marketplace with a $25 flat shipping fee. MDBT50Q-P1MV2: $6.00, 82 in stock. Adafruit sells MDBT50Q-1MV2 at $12.95 (out of stock). [DigiKey 1MV2](https://www.digikey.com/en/products/detail/raytac/MDBT50Q-1MV2/13677591); [DigiKey P1MV2](https://www.digikey.com/en/products/detail/raytac/MDBT50Q-P1MV2/13677599); [Adafruit 4078](https://www.adafruit.com/product/4078)
- nRF54L15: Raytac AN54LQ-15 ~$4.10 and AN54LV-15 ~$4.30 at DigiKey. AN54LQ-DB-15 eval board $16.84. Ezurio BL54L15 stock runs from 0 to ~7,700 depending on SKU. Bare nRF54L15-CAAA-R7: $4.15, 561 in stock, and NRF54L15-DK $39.00, both from an old DigiKey highlight page. [DigiKey AN54LQ-15](https://www.digikey.com/en/products/detail/raytac/AN54LQ-15/25855461); [DigiKey nRF54L15 highlight](https://www.digikey.com/en/product-highlight/n/nordic-semi/nrf54l15-multiprotocol-soc); [Ezurio BL54L15](https://www.ezurio.com/product/bl54l15-series-bluetooth-le-80215-4-nfc)

### Inferences
- Suggested primary and alternate pairs: IMU BMI270 / LSM6DSOX (keep ICM-42688-P off the critical path). ALS VEML7700 / OPT3001. Charger BQ24074 (simple, with TS/NTC pin) / nPM1300 or MAX77654 (integrated PMIC). Radio ESP32-C3-MINI-1 (keeps goggl1 firmware) / Raytac nRF54L15 module.
- For cold-charge cutoff, BQ24074 uses a TS pin for the NTC, and nPM1300 and MAX77654 have JEITA/NTC inputs. Confirm each against its datasheet; none were fetched here.

### Gaps
- No price or stock found for LSM6DSOX, OPT3001, BQ25180, RP2350, u-blox NINA/ANNA modules, or ESP32-S3-MINI-1.
- NTC and JEITA cold-charge behaviour is not verified from datasheets in this pass.

## Rough BOM, minimal HUD headpiece plus puck (qty 1 / qty 100)

### Takeaway
Using the sourced unit prices above, the core electronics (display, MCU/radio, IMU, ALS, charger) come to about $25 at qty 1 and $17 at qty 100. Passives, PCBs, battery, connectors, optics and enclosure, none of them priced here, will dominate. A sub-$300 kit looks very feasible on the electronics side.

### Cited Findings
- Sourced component prices are the ones cited in the sections above. The arithmetic below is an inference.

### Inferences
Headpiece: display, ALS, IMU, and an FPC/cable to the puck. Puck: MCU/BLE, charger, battery, button. A single-MCU split, with the MCU in the headpiece and only the battery in the puck, also works.

| Item | Qty 1 | Qty 100 (per unit) |
|---|---|---|
| 0.96" SSD1306 PMOLED (Orient AOM12864A0) | $11.57 | $7.39 |
| ESP32-C3-MINI-1-N4 | $3.26 | $2.45 |
| BMI270 | $4.23 | ~$3 (assumed between the $4.23 and $2.83 breaks) |
| VEML7700 | $1.44 | ~$1.0 (assumed) |
| BQ24074 | $3.19 | $1.97 |
| **Core silicon subtotal** | **~$23.7** | **~$15.8** |
| Unpriced (estimate only): 2 PCBs, passives, USB-C, LiPo 500-1000 mAh, NTC, cable, combiner/lens optics, enclosure, clip | likely $40-120 at qty 1 | |

- Optics and mechanics, not silicon, will set where the cost lands against the $300 target.

### Gaps
- Battery, optics (combiner, lens, prism), PCB fab and enclosure costs were not researched in this pass.
