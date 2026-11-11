# Concept 6 "Rail": parts availability check

Checked 2026-11-10. Quick check, not deep research.

How to read the "how confirmed" column:

- **page**: read on the seller's own product page today.
- **snippet**: from a web search result summary, not the seller's page as read today. Treat price and stock as provisional.
- **unconfirmed**: not found or not readable.

Reading limits met today:

- **Switch Science** returned HTTP 429 (too many requests) on every direct page read, three attempts. All Switch Science data below is from search snippets.
- **SparkFun** pages show two stock labels at once: "In stock" near the price and "Backorder" in the purchase area. Both are recorded as shown; which is true is unconfirmed. Only the BMP581 page showed "In stock" alone.
- **DigiKey, Mouser, Amazon**: not read (blocked to automated reading, by instruction).
- **Sengoku**: not checked.

## Not available or problem items (summary)

| Item | Problem |
|---|---|
| Adafruit BMP390 (4816) | Out of stock |
| Adafruit BMP581 (6407) | Out of stock |
| Adafruit DPS310 (4494) | "No longer stocked"; Adafruit suggests SPA06-003 instead |
| Adafruit DIY Magnetic Connector, 4-pin straight and 6-pin at 0.1 in pitch | No such product found. 4-pin exists only right-angle (5358); 6-pin only straight at 2.2 mm pitch (5467) |
| Narrow LED strip at 3.3-4.2 V | Adafruit states 5 VDC only for strip 4368 and LEDs 4492; no statement on 3.3 V supply or 3.3 V data |
| SparkFun 18650 cell PRT-12895 | Page does not say it is protected |
| Protected 18650 cell with PSE mark at Akizuki | Akizuki search for "18650" returns only holders, no cells |
| ESP-NOW receive alongside BLE on ESP32-C3 | Supported only when Wi-Fi is in station mode (see part 9) |
| Small power banks at tens of mA | Shut off below roughly 30-90 mA (see part 10) |

## 1. Narrow addressable RGB LED strip

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| Ultra Skinny NeoPixel 1515 LED Strip 4mm wide - 0.5 meter long - 75 LEDs | Adafruit | 4368 | $24.95 | "33 in stock" | 4 mm wide; 150 LEDs/m; 0.5 m. "Operating voltage is 5VDC", "do not use higher than 6V". No statement on 3.3 V supply or 3.3 V data. Unprotected, delicate | https://www.adafruit.com/product/4368 | page |
| Same strip (Adafruit 4368) | Switch Science | 6008 | 5,225 yen incl. tax | 8 or 2 units shown in different snippets | 4 mm wide, 0.5 m, 75 LEDs, 5 VDC, IP20 | https://www.switch-science.com/products/6008 | snippet (page returned 429) |
| NeoPixel Addressable 1515 LEDs (1.5mm x 1.5mm) - 10 pack - SK6805-E-J | Adafruit | 4492 | $3.95 per 10 | "In stock" | Bare 1.5 x 1.5 mm LEDs. "Power the whole thing with 5VDC". No statement on 3.3 V | https://www.adafruit.com/product/4492 | page |
| Board with 10 x SK6805-2427 | Switch Science | 8028 | unconfirmed | unconfirmed | About 16 mA per LED at full white, per snippet | https://www.switch-science.com/catalog/8028/ | snippet |
| Cut pitch of strip 4368 | reseller listing | - | - | - | Cuttable every 6.66 mm (matches 150 LEDs/m) | - | snippet |
| Side-emitting NeoPixel strip, APA102-2020 micro strip, SK6805-EC15 | - | - | unconfirmed | unconfirmed | Not checked | - | unconfirmed |

Fit for Rail: at 150 LEDs/m (6.67 mm pitch), 120-160 mm is 18-24 pixels. For 12-20 pixels, cut 80-133 mm. Running from 3.3-4.2 V is not stated by Adafruit for either part; it needs a bench test or the SK6805 datasheet.

## 2. Magnetic pogo-pin connector pairs

Pitch is from the page text. No page states current per pin or dimensions.

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| DIY Magnetic Connector - Right Angle Four Contact Pins | Adafruit | 5358 | $6.50 | "In stock" | 0.1 in pitch; current per pin not stated | https://www.adafruit.com/product/5358 | page |
| DIY Magnetic Connector - Right Angle Five Contact Pins | Adafruit | 5359 | $6.95 | "In stock" | 0.1 in pitch; current per pin not stated | https://www.adafruit.com/product/5359 | page |
| DIY Magnetic Connector - Straight Angle Five Contact Pins | Adafruit | 5413 | $6.95 | "In stock" | 0.1 in pitch; current per pin not stated | https://www.adafruit.com/product/5413 | page |
| DIY Magnetic Connector - Straight 6 Contact Pins - 2.2mm Pitch | Adafruit | 5467 | $7.95 | "In stock" | 2.2 mm pitch; current per pin not stated | https://www.adafruit.com/product/5467 | page |
| DIY Magnetic Connector - Right Angle Three Contact Pins | Adafruit | 5360 | $5.95 | unconfirmed | 0.1 in pitch | https://www.adafruit.com/product/5360 | page (listed as related item on 5358) |
| Straight 7 / 8 / 9 Contact Pins - 2.2mm Pitch | Adafruit | 5468 / 5469 / 5470 | unconfirmed | unconfirmed | 2.2 mm pitch | - | page (listed as related items on 5467) |
| Magnetic USB Charging Cable for 4 Pin 0.1" Magnetic Connector, 60cm | Adafruit | 5412 | unconfirmed | unconfirmed | Mates with 4-pin | - | page (listed as related item on 5413) |
| 4-pin straight, 6-pin right-angle, 6-pin at 0.1 in | Adafruit | - | - | not found | No such product seen | - | unconfirmed |
| SparkFun equivalent | SparkFun | - | - | - | Not checked | - | unconfirmed |
| Switch Science / Akizuki equivalent | - | - | - | - | Switch Science search returned 429; Akizuki not checked | - | unconfirmed |

## 3. Real-time clock breakout

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| SparkFun Real Time Clock Module - RV-8803 (Qwiic) | SparkFun | BOB-16281 | $20.57 | "In stock" and "Backorder" both shown | Includes 1x 12 mm CR1225 coin cell; no supercap mentioned | https://www.sparkfun.com/products/16281 | page |
| Adafruit DS3231 Precision RTC - STEMMA QT | Adafruit | 5188 | $13.95 | "In stock" | Coin cell holder on board; CR1220 not included (+$0.95); no supercap | https://www.adafruit.com/product/5188 | page |
| Adafruit PCF8523 Real Time Clock Breakout Board - STEMMA QT / Qwiic | Adafruit | 5189 | $4.95 | "In stock" | CR1220 not included (+$0.95); "Will keep time for 5 years or more" | https://www.adafruit.com/product/5189 | page |
| Adafruit RV-3028 STEMMA QT | Adafruit | unconfirmed | unconfirmed | unconfirmed | Not checked | - | unconfirmed |
| Japan stock of any of these | - | - | - | - | Not checked | - | unconfirmed |

## 4. Barometric pressure + temperature breakout

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| Adafruit BMP390 - Precision Barometric Pressure and Altimeter (STEMMA QT / Qwiic) | Adafruit | 4816 | $10.95 | "Out of stock" | Relative accuracy +/-3 Pa (about +/-0.25 m); I2C or SPI; 3 V or 5 V logic | https://www.adafruit.com/product/4816 | page |
| Adafruit DPS310 Precision Barometric Pressure / Altitude Sensor | Adafruit | 4494 | $6.95 | "No longer stocked" (SPA06-003 suggested) | 25.5 x 17.7 x 4.6 mm; 300-1200 hPa; +/-0.002 hPa precision | https://www.adafruit.com/product/4494 | page |
| Adafruit BMP581 I2C or SPI Temperature and Pressure Sensor - STEMMA QT | Adafruit | 6407 | $9.95 | "Out of stock" | Dimensions not stated | https://www.adafruit.com/product/6407 | page |
| Adafruit BMP580 I2C or SPI Temperature and Pressure Sensor - STEMMA QT | Adafruit | 6411 | $7.95 | unconfirmed | Firmware compatible with BMP581, lower accuracy | https://www.adafruit.com/product/6411 | snippet |
| Adafruit BMP580 (same board) | Switch Science | 10948 | 1,639 yen incl. tax | out of stock when captured | - | https://www.switch-science.com/products/10948 | snippet |
| SparkFun Pressure Sensor - BMP581 (Qwiic) | SparkFun | SEN-20170 | $22.95 | "In stock" | 1 x 1 in board; 1.71-3.6 V; 30-125 kPa; +/-0.5 hPa absolute; up to 240 Hz; 2x Qwiic | https://www.sparkfun.com/products/20170 | page |
| Adafruit SPA06-003, SparkFun LPS28 | - | unconfirmed | unconfirmed | unconfirmed | Not checked | - | unconfirmed |

## 5. Magnetometer breakout

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| Adafruit Triple-axis Magnetometer - MMC5603 - STEMMA QT / Qwiic | Adafruit | 5579 | $5.95 | "In stock" | +/-30 G; 2 mG RMS noise; 1 degree heading; up to 1 kHz; I2C 0x30; board size not stated | https://www.adafruit.com/product/5579 | page |
| Adafruit Triple-axis Magnetometer - LIS3MDL - STEMMA QT / Qwiic | Adafruit | 4479 | $12.50 | "In stock" | 25.7 x 17.8 x 4.6 mm; 1.5 g; +/-4 to +/-16 G; up to 1000 Hz | https://www.adafruit.com/product/4479 | page |
| SparkFun Micro Magnetometer - MMC5983MA (Qwiic) | SparkFun | SEN-19921 | $18.50 | "In stock" and "Backorder" both shown | 19.05 x 7.62 mm (overview text says 24.65 x 7.62 mm); 2.8-3.6 V; +/-8 G; 0.4 mG noise; +/-0.5 degree heading; 1000 Hz | https://www.sparkfun.com/products/19921 | page |
| Japan stock of any of these | - | - | - | - | Not checked | - | unconfirmed |

## 6. GNSS breakout, 10 Hz or faster

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| Adafruit Mini GPS PA1010D - UART and I2C - STEMMA QT | Adafruit | 4415 | $29.95 | "In stock" | "Up to 10 Hz updates"; 25.5 x 25.4 x 8.2 mm; 5.0 g; antenna on board | https://www.adafruit.com/product/4415 | page |
| SparkFun GNSS Receiver Breakout - MAX-M10S (Qwiic) | SparkFun | GPS-18037 | $45.95 | "In stock" and "Backorder" both shown | 10 Hz max; 1.74 x 1.20 in with SMA connector (needs external antenna); mass not stated | https://www.sparkfun.com/products/18037 | page |
| SparkFun GPS Breakout - Chip Antenna, SAM-M10Q (Qwiic) | SparkFun | GPS-21834 | $51.50 | "In stock" and "Backorder" both shown | "Max 5Hz (Four GNSS)", "Max 18Hz (One GNSS)"; size and mass not stated | https://www.sparkfun.com/products/21834 | page |
| Japan stock of any of these | - | - | - | - | Not checked | - | unconfirmed |

## 7. 18650 holder and protected cell

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| Lithium Ion Cylindrical Battery - 3.7v 2200mAh | Adafruit | 1781 | $9.95 | "11 in stock" | Protected (over-voltage, under-voltage, over-current); 2-pin JST-PH lead fitted; 69 x 18 mm; 46 g. Has a wired lead, so not a loose cell for a holder | https://www.adafruit.com/product/1781 | page |
| Lithium Ion Battery - 18650 Cell (2600mAh) | SparkFun | PRT-12895 | $9.83 | "In stock" and "Backorder" both shown | 65 x 18.2 mm; protection circuit not mentioned | https://www.sparkfun.com/products/12895 | page |
| Battery Holder - 1x18650 (wire leads) | SparkFun | PRT-12899 | $1.25 | unconfirmed | Open holder, wire leads | https://www.sparkfun.com/products/12899 | snippet |
| Single 18650 holder | Adafruit | - | - | not found | No Adafruit single 18650 holder found in search | - | unconfirmed |
| 電池ボックス 18650x1本 リード線付 (BHC-18650-1A) | Akizuki | 108407 | 110 yen incl. tax | In stock, 1,174 | Holder, lead wires | https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=18650 | page (search results) |
| 電池ボックス 18650x1本 表面実装用 (BHC-18650-3-SMT) | Akizuki | 129374 | 160 yen incl. tax | In stock, 851 | Holder, surface mount | same | page (search results) |
| 電池ボックス 18650x2本 リード線付 (BHC-18650-2A150MM) | Akizuki | 129373 | 160 yen incl. tax | In stock, 764 | Holder, 2 cells | same | page (search results) |
| Protected 18650 cell with PSE mark | Akizuki | - | - | not listed | Search for "18650" returned only the 3 holders above | same | page (search results) |
| Protected 18650 cell with PSE mark | Sengoku | - | - | - | Not checked | - | unconfirmed |

Note: a protected cell is longer than a bare one (Adafruit 1781 is 69 mm against 65 mm). Check holder length before pairing. PSE-marked protected 18650 cells do appear on Japanese marketplaces (snippet only, sellers not verified).

## 8. Test-host goggles

| Item | Seller | Product number | Price | Stock as shown | Key specs | URL | How confirmed |
|---|---|---|---|---|---|---|---|
| 100% Accuri 2 Moto/MTB goggle | 100percent.com | - | "From $49.95" | unconfirmed | Strap width not found | https://100percent.com/collections/spring-2026-goggles | snippet |
| 100% Accuri 2 | Helmet House (distributor) | - | MSRP $45.00-60.00 | unconfirmed | - | https://helmethouse.com/100/accuri-2-goggles.html | snippet |
| 100% Strata 2 Moto/MTB goggle | 100percent.com | - | "From $29.95" | unconfirmed | Strap width not found | https://100percent.com/collections/spring-2026-goggles | snippet |
| 100% Strata 2 | Helmet House (distributor) | - | MSRP $25.00-35.00 | unconfirmed | - | https://helmethouse.com/100/strata-2-goggles.html | snippet |
| Smith Frontier snow goggle | smithoptics.com | - | $27 sale / $45 regular on one page; $33 sale / $55 regular on another | Marked "Past Season"; one colour sold out | Strap width not found | https://www.smithoptics.com/en-us/products/frontier | snippet |

No page stated a strap width in mm. Measure on the bought goggle.

## 9. ESP-NOW and Bluetooth LE together on ESP32-C3

Source: Espressif ESP-IDF "RF Coexistence" guide for ESP32-C3, stable version. https://docs.espressif.com/projects/esp-idf/en/stable/esp32c3/api-guides/coexist.html (how confirmed: page)

The page has no prose sentence about ESP-NOW with BLE. The answer is in the table "Supported Features of Wi-Fi and BLE Coexistence", which has these ESP-NOW rows:

- "ESP-NOW RX S S S"
- "TX Y Y Y"

Legend, quoted:

- "Y: supported and the performance is stable"
- "C1: supported but the performance is unstable"
- "S: supported and the performance is stable in STA mode, otherwise not supported"

So: yes. ESP-NOW transmit is supported and stable alongside BLE. ESP-NOW receive is supported and stable only with Wi-Fi in station mode. The radio is shared by time-division, so both lose some airtime.

Limits of this reading: the BLE column headers (which BLE states the three columns stand for) were not captured. Search snippets show four columns in v5.3/v5.5 and no ESP-NOW rows in the v5.2 guide, so check the page for the IDF version in use.

## 10. Power banks switching off at low load

| Statement | Source | Threshold | URL | How confirmed |
|---|---|---|---|---|
| Battery "requires connected devices to draw at least 30 - 90mA to continue operation"; Trickle Charging Mode "automatically turns off after 2 hours if no interaction occurs" | Anker support, "What is Trickle Charging Mode?" | 30-90 mA | https://service.anker.com/ca/article-description/What-is-Trickle-Charging-Mode | snippet (page body did not load for reading) |
| Power bank assumes nothing is plugged in and shuts off when current drops "typically below 30-90mA" | Ugreen power bank FAQ | 30-90 mA | https://uk.ugreen.com/blogs/power-bank/power-bank-faqs-complete-troubleshooting-guide | snippet |
| Low-Current Charging Mode is for devices drawing "60mA or less" | JAR Systems knowledge base | 60 mA | https://www.jar-systems.com/knowledge-base/how-does-the-low-current-charging-mode-on-my-active-charge-power-banks-work | snippet |
| Voltaic FAQ | - | - | - | unconfirmed (not found) |

So: yes, a Rail load of tens of mA is in the shut-off zone for ordinary power banks. Options are a bank with a low-current mode (Anker's times out after 2 hours), a keep-alive pulsed load, or a bare cell with its own protection.
