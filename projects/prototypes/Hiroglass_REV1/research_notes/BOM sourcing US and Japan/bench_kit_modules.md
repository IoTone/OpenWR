# Bench kit modules: where to buy in the US and in Japan (checked 2026-11-10)

All observations below were made on 2026-11-10. Each number is tagged:

- **READ** = read on the seller's own page (or, for Switch Science, the shop's own public catalogue feed `products.json`, which gives price and in/out status but no unit count).
- **SNIPPET** = from a search-result snippet only.
- **UNCONFIRMED** = could not be read; no number is given.

Sites that blocked automated reading: DigiKey (HTTP 403), Mouser (timeout), Octopart (403), Marutsu (403). Findchips returned "no results" for both `DFR0868` and `Adafruit 5405`. Amazon.co.jp, Taobao and LCSC were not checked. Per-row data is in `bench_kit_modules.json` (65 rows).

Three corrections to the BOM came out of this pass:

1. **Adafruit 1131 is not the switched cable.** 1131 is "JST-PH Battery Extension Cable - 500mm" ($1.95, no switch). The "JST 2-pin Extension Cable with On/Off Switch" at $2.95 is **Adafruit 3064**.
2. **Several Oct-2026 prices have moved**: BMI270 SEN-22397 is $14.95 (was $18.50), DFR0868 is $7.90 (was $6.90), USB cable 4474 is $4.95 (was $2.95), breadboard 64 is $4.95 (was $5.00).
3. **Adafruit QT Py ESP32-C3 (5405) is out of stock** at Adafruit and is not sold by Switch Science, so Kit B is only buildable today with the XIAO ESP32C3 fallback, in both regions.

---

## US: is every module in stock at Adafruit / SparkFun / DFRobot / DigiKey / Mouser today?

### Takeaway
No. Six Adafruit parts are out of stock with no restock date (5405, 4209, 4210, 3064, 758, 392), but every one has an in-stock substitute at Adafruit or SparkFun, so Kit A is buildable today from US stock except that the Beetle's US-origin source (DigiKey) could not be confirmed. Shipping fees are not published by either shop.

### Cited Findings

Adafruit (all READ on the product page; stock is shown as in/out only, never a count; the price is flat from 1 to 9 pieces, so the 5-piece unit price equals the 1-piece price):

| Ref | Part | Price 1-9 | Price 10+ | Status |
|---|---|---|---|---|
| OLED | 938, 1.3" 128x64 | $19.95 | $17.96 | In stock — [Adafruit 938](https://www.adafruit.com/product/938) |
| OLED alt | 326, 0.96" 128x64 | $17.50 | $15.75 | In stock — [Adafruit 326](https://www.adafruit.com/product/326) |
| ALS | 4162, VEML7700 | $4.95 | $4.46 | In stock — [Adafruit 4162](https://www.adafruit.com/product/4162) |
| QTH | 4209, JST SH to male header 150 mm | $0.95 | $0.86 | **Out of stock** — [Adafruit 4209](https://www.adafruit.com/product/4209) |
| QT | 4399, 50 mm | $0.95 | $0.86 | In stock — [Adafruit 4399](https://www.adafruit.com/product/4399) |
| QT | 4210, 100 mm | $0.95 | $0.86 | **Out of stock** — [Adafruit 4210](https://www.adafruit.com/product/4210) |
| PUCK | 4755, bq24074 | $14.95 | $13.46 | In stock — [Adafruit 4755](https://www.adafruit.com/product/4755) |
| BAT | 1578, 500 mAh | $7.95 | $7.16 | In stock — [Adafruit 1578](https://www.adafruit.com/product/1578) |
| SW | 3064, switched JST-PH extension | $2.95 | $2.66 | **Out of stock** — [Adafruit 3064](https://www.adafruit.com/product/3064) |
| SW (as listed) | 1131, plain 500 mm extension, no switch | $1.95 | $1.76 | In stock — [Adafruit 1131](https://www.adafruit.com/product/1131) |
| LEAD | 3814 | $0.75 | $0.68 | In stock — [Adafruit 3814](https://www.adafruit.com/product/3814) |
| USB | 4474 | $4.95 | $4.46 | In stock — [Adafruit 4474](https://www.adafruit.com/product/4474) |
| BB | 64 | $4.95 | $4.46 | In stock — [Adafruit 64](https://www.adafruit.com/product/64) |
| JMP | 758 | $3.95 | $3.56 | **Out of stock** — [Adafruit 758](https://www.adafruit.com/product/758) |
| HDR | 392 | $4.95 | $4.46 | **Out of stock** — [Adafruit 392](https://www.adafruit.com/product/392) |
| MCU (Kit B) | 5405, QT Py ESP32-C3 | $9.95 | $8.96 | **Out of stock** — [Adafruit 5405](https://www.adafruit.com/product/5405) |

- Out-of-stock Adafruit pages offer only a "Notify Me" sign-up; none gives a restock date (READ) — [Adafruit 5405](https://www.adafruit.com/product/5405)
- QT Py ESP32-C3: DigiKey part 1528-5405-ND shows an 18-week manufacturer standard lead time; Pimoroni, The Pi Hut, Little Bird and Micro Center show sold out (SNIPPET) — [DigiKey UK listing](https://www.digikey.co.uk/en/products/detail/adafruit-industries-llc/5405/21283808)

SparkFun (all READ; "In stock" with no count):

- IMU SEN-22397: $14.95; $14.20 at 10+, $13.46 at 25+; in stock — [SparkFun SEN-22397](https://www.sparkfun.com/sparkfun-6dof-imu-breakout-bmi270-qwiic.html)
- Substitute for 4209: PRT-17912 Flexible Qwiic Cable - Breadboard Jumper (4-pin), $1.95, in stock — [SparkFun PRT-17912](https://www.sparkfun.com/flexible-qwiic-cable-breadboard-jumper-4-pin.html)
- Substitute for 758: PRT-11026 Jumper Wires 7" M/M 30-pack, $3.50, in stock — [SparkFun PRT-11026](https://www.sparkfun.com/jumper-wires-standard-7-m-m-30-awg-30-pack.html)
- Substitute for 392: PRT-00116 straight 40-pin header, $1.95, in stock — [SparkFun PRT-00116](https://www.sparkfun.com/break-away-headers-straight.html)
- Second source for the battery: PRT-13851 400 mAh JST-PH, $7.98, in stock; cannot ship to Alaska/Hawaii, by express, or internationally — [SparkFun PRT-13851](https://www.sparkfun.com/lithium-ion-battery-400mah.html)

MCU boards:

- DFRobot DFR0868: $7.90 (1), $7.80 (3+), $7.70 (5+), $7.30 (10+); "Add to Cart" shown, no stock figure, no ships-from warehouse stated; page lists CE and RoHS only (READ) — [DFRobot DFR0868](https://www.dfrobot.com/product-2566.html)
- DigiKey lists DFR0868 as "BEETLE ESP32-C3 VERSION 2" (SNIPPET; stock and price UNCONFIRMED, page returned 403) — [DigiKey DFR0868](https://www.digikey.com/en/products/detail/dfrobot/DFR0868/16678683)
- Seeed XIAO ESP32C3 (113991054): $4.99, "In stock", warehouse options United States / China / Germany, no per-warehouse count (READ) — [Seeed XIAO ESP32C3](https://www.seeedstudio.com/Seeed-XIAO-ESP32C3-p-5431.html)

Shipping:

- Adafruit: orders usually ship within 1-2 business days; USPS Ground Advantage (orders under $200 only) 2-5 business days; Priority Mail (under $200) 3-5 business days; UPS Ground 3-5 business days. The page lists no fee amounts and no free-shipping threshold (READ) — [Adafruit shipping](https://www.adafruit.com/shipping)
- Adafruit: "Orders containing batteries may have limited shipping options at checkout" (READ) — [Adafruit shipping](https://www.adafruit.com/shipping)
- SparkFun: in-stock orders ship in 1-3 business days; USPS Ground Advantage 2-5 business days, UPS Ground 1-5, FedEx Ground 2-5; free UPS Ground at a $100+ subtotal, logged in, contiguous 48 states; rates are not listed. Orders with lithium batteries are excluded from same-day shipping (READ) — [SparkFun shipping](https://www.sparkfun.com/shipping)

### Inferences
- A single-kit order splits into about $62 at Adafruit and about $22 at SparkFun, so the SparkFun part does not reach the $100 free-shipping threshold; a five-kit order (about $112 at SparkFun) does.
- A five-kit Adafruit order (about $312) is over $200, so the USPS options drop out and it ships UPS.
- DFRobot is a China-based seller and its page names no US warehouse, so buying the Beetle from dfrobot.com probably does not meet the "US seller shipping from the US" rule; DigiKey is the likely compliant source but is unverified.
- With the QT Py out of stock and on an 18-week lead at DigiKey, the XIAO ESP32C3 is the practical Kit B MCU in the US.

### Gaps
- DigiKey and Mouser stock and price for every part: both sites blocked reading. Nothing from them is confirmed.
- Adafruit and SparkFun shipping fee for a ~$100 order: neither shop publishes a rate table; the fee appears only at checkout.
- Restock dates for the six out-of-stock Adafruit parts.
- Whether Seeed's US warehouse actually holds XIAO ESP32C3 stock, and its shipping fee.
- A SparkFun 100 mm Qwiic cable as the second source for Adafruit 4210 was not checked.

---

## Japan: which exact modules can be bought from a Japanese shop today, and which must be imported or substituted?

### Takeaway
Switch Science stocks the exact Beetle, OLED 938, BMI270 SEN-22397, VEML7700 4162, the 100 mm STEMMA QT cable and the switched JST-PH cable (3064) today; the bq24074 charger, the QT Py ESP32-C3 and any bare LiPo cell are not available from the Japanese shops checked. Akizuki fills in the XIAO ESP32C3, breadboard, jumpers and headers.

### Cited Findings

Switch Science (prices are tax included; READ from the shop's catalogue feed on 2026-11-10 unless noted; in/out status only):

| Ref | Item (maker code) | Yen, tax incl. | Status |
|---|---|---|---|
| MCU | DFRobot Beetle ESP32-C3 (DFR0868) | 1,268 (sale; list 1,425) | In stock, **9 units** (READ on product page) — [SS 10088](https://www.switch-science.com/products/10088) |
| OLED | Adafruit 938 | 4,180 | In stock — [SS 6105](https://www.switch-science.com/products/6105) |
| OLED alt | Adafruit 326 | 3,685 | In stock — [SS 2729](https://www.switch-science.com/products/2729) |
| OLED sub | SparkFun Qwiic OLED 1.3" (LCD-23453), SH1106 | 4,114 | In stock — [SS 9405](https://www.switch-science.com/products/9405) |
| OLED sub | Pololu 1.3" OLED (3761), SH1106, SPI | 3,643 | In stock — [SS 9638](https://www.switch-science.com/products/9638) |
| IMU | SparkFun SEN-22397 | 3,817 | In stock — [SS 9087](https://www.switch-science.com/products/9087) |
| IMU sub | SparkFun Qwiic Micro BMI270 | 3,630 | In stock — [SS 9086](https://www.switch-science.com/products/9086) |
| ALS | Adafruit 4162 | 1,210 | In stock — [SS 8182](https://www.switch-science.com/products/8182) |
| ALS sub | SparkFun Qwiic VEML7700 | 1,529 | In stock — [SS 10600](https://www.switch-science.com/products/10600) |
| QTH | Adafruit 4209 | 242 | **Out of stock** — [SS 5890](https://www.switch-science.com/products/5890) |
| QTH sub | SparkFun Qwiic to 4-pin female, 150 mm | 407 | In stock — [SS 6895](https://www.switch-science.com/products/6895) |
| QT | Adafruit 4210, 100 mm | 209 | In stock — [SS 11398](https://www.switch-science.com/products/11398) |
| QT | Adafruit 4399, 50 mm | 209 | **Out of stock** — [SS 11397](https://www.switch-science.com/products/11397) |
| QT sub | SparkFun Qwiic 50 mm | 308 | In stock — [SS 6896](https://www.switch-science.com/products/6896) |
| PUCK sub | Adafruit bq25185 USB/DC/solar charger (6091) | 1,485 | In stock — [SS 10066](https://www.switch-science.com/products/10066) |
| PUCK sub | Adafruit USB-C LiPo charger (4410) | 1,265 | In stock — [SS 6006](https://www.switch-science.com/products/6006) |
| SW | Adafruit 3064, switched JST-PH extension | 726 | In stock — [SS 2737](https://www.switch-science.com/products/2737) |
| LEAD sub | JST PH 2-pin plug with red/black wires | 264 | In stock — [SS 2215](https://www.switch-science.com/products/2215) |
| USB sub | Adafruit USB-A to right-angle USB-C, 1 m | 968 | **Out of stock** — [SS 7424](https://www.switch-science.com/products/7424) |
| MCU (Kit B) | Seeed XIAO ESP32C3 | 1,122 | **Out of stock** — [SS 8348](https://www.switch-science.com/products/8348) |
| MCU sub | M5Stamp C3 Mate (K056) / C3U Mate (K122) | 1,386 each | In stock — [SS 7474](https://www.switch-science.com/products/7474), [SS 7894](https://www.switch-science.com/products/7894) |
| MCU sub | M5Stamp C3 5-pack / C3U 5-pack | 6,622 / 6,325 | In stock — [SS 7473](https://www.switch-science.com/products/7473), [SS 7893](https://www.switch-science.com/products/7893) |

- Not in the Switch Science catalogue at all (10,519 products scanned by maker code): Adafruit 4755 (bq24074), 5405 (QT Py ESP32-C3), 1578 (battery), 3814, 4474, 64, 758, 392 (READ) — [Switch Science](https://www.switch-science.com/)
- Switch Science shipping: orders of 8,000 yen or more ship free by Yamato Takkyubin; below that, 200 yen (Nekopos items, free from 3,000 yen) or 650 yen (Takkyubin items). Orders paid by 14:00 on a business day ship that evening; arrival is expected next day in Kanto and two or more days elsewhere; orders containing lithium-ion batteries can take a few days longer in some regions. Ships from Tokyo and Kanagawa (READ) — [Switch Science shipping policy](https://www.switch-science.com/policies/shipping-policy)

Akizuki Denshi (all READ; prices tax included; stock = units purchasable online):

- Seeed XIAO ESP32C3: 1,080 yen (982 before tax), 1,460 units, no quantity break — [Akizuki 117454](https://akizukidenshi.com/catalog/g/g117454/)
- Breadboard EIC-801: 450 yen, 2,311 units — [Akizuki 100315](https://akizukidenshi.com/catalog/g/g100315/)
- Breadboard jumper wire kit, 14 lengths x 5: 650 yen, 940 sets — [Akizuki 102315](https://akizukidenshi.com/catalog/g/g102315/)
- Pin header 1x40: 35 yen, 8,000 units — [Akizuki 100167](https://akizukidenshi.com/catalog/g/g100167/)
- 0.96" 128x64 white OLED, controller SSD1315, I2C: 580 yen, 333 units, marked low stock with restock mid-November — [Akizuki 112031](https://akizukidenshi.com/catalog/g/g112031/)
- Akizuki searches for "BMI270", "VEML", a 1.3" OLED and lithium-polymer cells returned no products — [Akizuki BMI270 search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=BMI270)

Other Japanese shops:

- Sengoku Densho: a "リチウムポリマー" search returns chargers and boards, no cells; it resells Switch Science / SparkFun / Adafruit items at tax-included prices — [Sengoku search](https://www.sengoku.co.jp/mod/sgk_cart/search.php?multi=%E3%83%AA%E3%83%81%E3%82%A6%E3%83%A0%E3%83%9D%E3%83%AA%E3%83%9E%E3%83%BC)
- Kyohritsu Eleshop: the same search returned no products — [Eleshop search](https://eleshop.jp/shop/goods/search.aspx?search=x&keyword=%E3%83%AA%E3%83%81%E3%82%A6%E3%83%A0%E3%83%9D%E3%83%AA%E3%83%9E%E3%83%BC)

China-sourced generics (AliExpress search pages READ, but on the **US storefront in USD**; many prices are new-user promotions, so treat them as a floor, not a quote):

- 1.3" SH1106 OLED, 4-pin/7-pin: $1.04 to $3.33 — [AliExpress search](https://www.aliexpress.com/w/wholesale-1.3-inch-oled-sh1106-7pin-spi.html)
- BMI270 module: generic $3.88 to $4.69; Qwiic-style clones $17.72 to $19.92 — [AliExpress search](https://www.aliexpress.com/w/wholesale-BMI270-module.html)
- VEML7700 module: only $1.09 promotional prices shown — [AliExpress search](https://www.aliexpress.com/w/wholesale-VEML7700-module.html)
- ESP32-C3 SuperMini: $1.09 (promo) to $3.33 — [AliExpress search](https://www.aliexpress.com/w/wholesale-esp32-c3-supermini.html)
- TP4056 USB-C charger with protection: $1.09 to $1.79 — [AliExpress search](https://www.aliexpress.com/w/wholesale-TP4056-type-c-charger-module-protection.html)
- 503035 500 mAh LiPo with JST PH 2.0: $1.08 to $10.45, typical non-promo $4.02 to $7.37 — [AliExpress search](https://www.aliexpress.com/w/wholesale-503035-500mah-lipo-PH2.0.html)
- Several of these listings show "Fastest delivery in 7 days" (US storefront) — [AliExpress search](https://www.aliexpress.com/w/wholesale-esp32-c3-supermini.html)

### Inferences
- The exact charger (bq24074) has no Japanese source found; the closest in-stock functional substitute is Adafruit's bq25185 USB/DC/solar board at Switch Science. It is a different TI chip, so it is a role substitute, not a same-chip one.
- The male-header Qwiic jumper is out at Switch Science in both Adafruit and SparkFun versions; the in-stock female version needs a pin header to reach a breadboard.
- Switch Science items tagged as on sale can be cheaper on the page than in the feed (the Beetle is 1,268 on the page against 1,425 in the feed), so feed prices are an upper bound.

### Gaps
- Akizuki's shipping fee, free-shipping threshold and delivery days: the guide pages returned 404 and no source was found. UNCONFIRMED.
- Marutsu (blocked) and Amazon.co.jp (not checked): no prices or stock.
- AliExpress prices and delivery days **to Japan** in yen: not obtained. Taobao, LCSC, and Seeed/DFRobot shipping fees to Japan: not checked.
- A Japan source for a plain USB-A to USB-C cable and for male/male jumper wires was not priced.
- Unit counts at Switch Science (other than the Beetle's 9), so whether 5 of each are on the shelf is unconfirmed.

---

## Japan radio law: which MCU boards carry 技適 certification?

### Takeaway
The best 技適-certified ESP32-C3 board a Japan builder can buy from stock is the **Seeed XIAO ESP32C3 at Akizuki Denshi**: 1,080 yen tax included, 1,460 in stock, with certificate number 204-B00704 printed on the page. The Beetle DFR0868 from Switch Science is the best choice for Kit A (seller states the mark is confirmed, 9 in stock); the QT Py ESP32-C3 and generic SuperMini boards have no evidence of certification.

### Cited Findings

| Board | Evidence | Verdict |
|---|---|---|
| Seeed XIAO ESP32C3 | Akizuki page lists "工事設計認証(技適)番号：204-B00704" (READ) — [Akizuki 117454](https://akizukidenshi.com/catalog/g/g117454/). Akizuki also hosts a Phoenix Testlab Japan certificate for the XIAO ESP32C3 (SNIPPET) — [certificate PDF](https://akizukidenshi.com/goodsaffix/SKU113991054TC.pdf) | Certified, number cited |
| DFRobot Beetle ESP32-C3 (DFR0868) | Switch Science page states "※ 技適マーク確認済みの製品です。" No certificate number is given (READ) — [SS 10088](https://www.switch-science.com/products/10088). DFRobot's own page lists only CE and RoHS (READ) — [DFRobot DFR0868](https://www.dfrobot.com/product-2566.html) | Seller statement only |
| M5Stamp C3 / C3U | Switch Science pages state the 技適 mark is printed on the enclosed sticker ("本製品の技適マークは添付のステッカーに印字されています") (READ) — [SS 7474](https://www.switch-science.com/products/7474), [SS 7894](https://www.switch-science.com/products/7894). M5Stack's own doc page shows no number (READ) — [M5Stack Stamp-C3 docs](https://docs.m5stack.com/en/core/stamp_c3) | Seller statement only; keep the sticker |
| Adafruit QT Py ESP32-C3 (5405) | Adafruit page lists RoHS only; no FCC, TELEC or Japan mention (READ) — [Adafruit 5405](https://www.adafruit.com/product/5405). Not carried by Switch Science | No evidence of 技適 |
| Espressif ESP32-C3-DevKitM-1 | No certificate found. DigiKey shows the board as obsolete (SNIPPET) — [DigiKey ESP32-C3-DEVKITM-1](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-C3-DEVKITM-1/13684315). Not listed at Akizuki or Switch Science | Unconfirmed |
| Generic ESP32-C3 SuperMini | No certificate or seller statement found in any search — [AliExpress search](https://www.aliexpress.com/w/wholesale-esp32-c3-supermini.html) | No evidence of 技適 |

- For reference, the Espressif module ESP32-C3-WROOM-02-N4 sold by Akizuki carries 工事設計認証 201-220555 (440 yen, 1,735 in stock) (READ) — [Akizuki 117493](https://akizukidenshi.com/catalog/g/g117493/)

### Inferences
- The XIAO ESP32C3 has the same footprint as the QT Py, so it is the natural Kit B board for Japan; it lacks the STEMMA QT connector, so the I2C chain needs the Qwiic-to-header jumper.
- A Beetle imported direct from DFRobot is not covered by Switch Science's "mark confirmed" statement; buy it from Switch Science if the mark matters.
- Boards built on a bare ESP32-C3 chip with their own antenna (QT Py, SuperMini) cannot inherit an Espressif module certificate, which is consistent with no certificate being found. This is background knowledge, not something read on a page this session.

### Gaps
- The MIC database (tele.soumu.go.jp) was not queried, so 204-B00704 and 201-220555 are as stated by Akizuki, not cross-checked against the register.
- No certificate number was found for the Beetle DFR0868 or the M5Stamp C3 / C3U.
- ESP32-C3-DevKitM-1: no Japanese stock or certificate located.

---

## Is the 1.3 inch OLED SSD1306 or SH1106, and does it matter for a driver?

### Takeaway
The Adafruit 938 is SSD1306; nearly every other 1.3 inch module on offer, including the SparkFun and Pololu boards stocked in Japan and all generic AliExpress listings, is SH1106, so a substitute changes the driver.

### Cited Findings
- Adafruit 938: the page names the driver chip as SSD1306, I2C or SPI (READ) — [Adafruit 938](https://www.adafruit.com/product/938)
- Switch Science's listing for Adafruit 938 also names SSD1306 (READ) — [SS 6105](https://www.switch-science.com/products/6105)
- SparkFun Qwiic OLED 1.3" (LCD-23453): the listing links the SH1106 datasheet (READ) — [SS 9405](https://www.switch-science.com/products/9405)
- Pololu 1.3" OLED (3761): controller SH1106, 4-wire SPI (READ) — [SS 9638](https://www.switch-science.com/products/9638)
- M5Stack 1.3" OLED unit (U119): SH1107, I2C, 2,068 yen at Switch Science / 2,200 yen at Akizuki (READ) — [SS 7233](https://www.switch-science.com/products/7233)
- Every 1.3" listing on the AliExpress search page is titled SH1106 (READ) — [AliExpress search](https://www.aliexpress.com/w/wholesale-1.3-inch-oled-sh1106-7pin-spi.html)
- Akizuki's 0.96" OLED uses SSD1315 (READ) — [Akizuki 112031](https://akizukidenshi.com/catalog/g/g112031/)

### Inferences
- It matters. From background knowledge (not read on a page this session): SH1106 has a 132-column RAM, so a 128-column panel sits at a 2-pixel offset, and it supports page addressing only, so an SSD1306 driver that streams the whole frame in one write shows shifted or garbled output. Firmware should select the controller at build time, or use a library that supports both (for example U8g2).
- To keep one firmware image across both sites, buy the Adafruit 938 in both regions; it is in stock at Adafruit and at Switch Science.

### Gaps
- No generic module was physically identified; listing titles are the only evidence for the SH1106 claim on clones.

---

## Shipping LiPo cells: can Adafruit/SparkFun ship to Japan, and what cells do Japanese shops stock?

### Takeaway
Plan on not getting cells from the US: SparkFun's battery page says it cannot ship internationally, and Adafruit publishes no policy that confirms battery shipment to Japan. No 400-600 mAh JST-PH cell was found in stock at any Japanese shop checked, which makes the battery the open item for the Japan build.

### Cited Findings
- SparkFun PRT-13851 (400 mAh): cannot ship to Alaska or Hawaii, by express, or internationally; the page calls this temporary (READ) — [SparkFun PRT-13851](https://www.sparkfun.com/lithium-ion-battery-400mah.html)
- SparkFun treats lithium batteries as dangerous goods needing extra packing and paperwork (READ) — [SparkFun shipping](https://www.sparkfun.com/shipping)
- Adafruit ships to Japan by DHL; the page says air mail "may not be available for packages containing lithium batteries" and gives no battery rule for international orders (READ) — [Adafruit shipping locations](https://www.adafruit.com/shipping_locations)
- Switch Science: the 900 mAh cell (3,520 yen) is out of stock; the 400 mAh DTP502535 with JST-PH (1,540 yen) and the 110 mAh cell are discontinued (READ) — [SS 2073](https://www.switch-science.com/products/2073), [SS 3118](https://www.switch-science.com/products/3118)
- Switch Science's 400 mAh page states the cell is outside the Electrical Appliance and Material Safety Law because its energy density is 400 Wh/L or less (SNIPPET) — [SS 3118](https://www.switch-science.com/products/3118)
- MonotaRO lists a DATA POWER 3.7 V 500 mAh cell as out of stock (SNIPPET) — [MonotaRO](https://www.monotaro.com/g/07050362)
- Akizuki, Sengoku and Kyohritsu searches returned no LiPo cells (READ) — [Akizuki battery category](https://akizukidenshi.com/catalog/r/rbatt/)

### Inferences
- The Japan site most likely has to import a generic 503035 cell from China or find one on Amazon.co.jp. Generic cells often have JST-PH polarity reversed relative to Adafruit boards, so check polarity before connecting (background knowledge).
- Small cells at or under 400 Wh/L fall outside PSE, so a "PSE-marked" 500 mAh cell may not exist as such; what matters is the energy-density figure for the specific cell.

### Gaps
- Whether Adafruit's checkout offers any method for a LiPo to Japan: unconfirmed without a real cart.
- No in-stock, PSE-marked or PSE-exempt 400-600 mAh JST-PH cell with a Japanese seller was found. Amazon.co.jp and Marutsu remain unchecked.
- Price, delivery days and battery-shipping terms for AliExpress cells to Japan.

---

## Total cost of one kit and of five kits in each region, and the gating item

### Takeaway
US Kit A comes to about $92.55 in parts for one and about $461.75 for five, before shipping, using four in-stock substitutes. Japan Kit A comes to about 15,009 yen for one and about 75,045 yen for five, tax included, **excluding the battery and USB cable**, which have no confirmed Japanese source. The gating item is the LiPo cell in Japan, and the Beetle's US-origin source in the US.

### Cited Findings
All prices used are the READ values cited in the sections above; the sums are mine.

US Kit A, one kit (USD, tax excluded):

| Ref | Source used | $ |
|---|---|---|
| MCU | DFR0868, dfrobot.com | 7.90 |
| OLED | Adafruit 938 | 19.95 |
| IMU | SparkFun SEN-22397 | 14.95 |
| ALS | Adafruit 4162 | 4.95 |
| QTH | SparkFun PRT-17912 (4209 out) | 1.95 |
| QT | 2 x Adafruit 4399 (4210 out) | 1.90 |
| PUCK | Adafruit 4755 | 14.95 |
| BAT | Adafruit 1578 | 7.95 |
| SW | Adafruit 1131, no switch (3064 out) | 1.95 |
| LEAD | Adafruit 3814 | 0.75 |
| USB | Adafruit 4474 | 4.95 |
| BB | Adafruit 64 | 4.95 |
| JMP | SparkFun PRT-11026 (758 out) | 3.50 |
| HDR | SparkFun PRT-00116 (392 out) | 1.95 |
| **Total** | | **92.55** |

- Five kits: 5 x 92.55 = 462.75, less the DFR0868 5+ break ($7.70) = **$461.75**. No other part has a price break below 10 pieces.
- Kit B (US): QT Py is out of stock; with the XIAO ESP32C3 at $4.99 the kit is **$89.64** for one and **$448.20** for five. If the QT Py returns at $9.95 the kit is $94.60.
- Order split for one kit: Adafruit about $62.30, SparkFun about $22.35, MCU $7.90.

Japan Kit A, one kit (JPY, tax included):

| Ref | Source used | Yen |
|---|---|---|
| MCU | DFR0868, Switch Science | 1,268 |
| OLED | Adafruit 938, Switch Science | 4,180 |
| IMU | SEN-22397, Switch Science | 3,817 |
| ALS | Adafruit 4162, Switch Science | 1,210 |
| QTH | SparkFun Qwiic to female, Switch Science (4209 out) | 407 |
| QT | Adafruit 4210 + SparkFun 50 mm, Switch Science | 517 |
| PUCK | Adafruit bq25185, Switch Science (4755 not sold) | 1,485 |
| BAT | no confirmed source | unconfirmed |
| SW | Adafruit 3064, Switch Science | 726 |
| LEAD | JST PH wire, Switch Science | 264 |
| USB | no confirmed source | unconfirmed |
| BB | EIC-801, Akizuki | 450 |
| JMP | jumper kit, Akizuki | 650 |
| HDR | 1x40 header, Akizuki | 35 |
| **Total excluding BAT and USB** | | **15,009** |

- Five kits: 5 x 15,009 = **75,045 yen**, excluding battery and USB cable; no quantity breaks were seen.
- Kit B (Japan): swap the Beetle for the Akizuki XIAO ESP32C3 at 1,080 yen: **14,821 yen** for one, **74,105 yen** for five, same exclusions.
- The Switch Science part of one kit is 13,874 yen, above the 8,000 yen free-shipping line — [Switch Science shipping policy](https://www.switch-science.com/policies/shipping-policy)

### Inferences
- **US gating item:** nothing in the kit is unobtainable, but the Beetle is the slowest if it must come from DFRobot in China; the fix is a confirmed DigiKey order or switching to the XIAO from Seeed's US warehouse. For the rest, expect roughly 3 to 8 business days (1-3 days processing plus 2-5 days ground).
- **Japan gating item:** the LiPo cell. Everything from Switch Science can arrive in 1 to 2 days after payment; an imported cell is likely a week or more, and that estimate is not confirmed for Japan.
- The US total is understated by shipping on two or three orders and by sales tax; the Japan total is understated by the battery, the USB cable and Akizuki's shipping fee.
- The Japan kit costs noticeably more than the US kit for the same Adafruit/SparkFun parts (for example 4,180 yen against $19.95 for the OLED).

### Gaps
- Shipping cost in both regions (Adafruit, SparkFun, DFRobot, Akizuki): unconfirmed, so no shipped total can be given.
- Battery and USB cable price for Japan.
- Whether Switch Science holds five of each item.
- A firm delivery estimate for any China-origin item to Japan.
