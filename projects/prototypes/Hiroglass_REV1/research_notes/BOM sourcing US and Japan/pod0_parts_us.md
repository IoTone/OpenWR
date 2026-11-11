# Pod-0 parts: US sources, stock, lead times and small-quantity prices (checked 2026-11-10)

How to read these notes:

- Every number below was READ on a page on 2026-11-10 unless it is marked "snippet" or "unconfirmed".
- DigiKey: `digikey.com` product detail pages still return HTTP 403 to automated fetches. The same product pages on DigiKey's Mexico storefront (`digikey.com.mx`, English, "All prices are in USD") are readable and carry the same product ids, the same in-stock figure and the same USD price table. The qty-1 price matched the `digikey.com` search card on all 8 parts where both were read (BMI270 4.23, USBLC6-2SC6 0.54, ESP32-C3-MINI-1-N4 3.26, BQ24074RGTR 2.43, FDN338P 0.73, FDN335N 0.76, PCM12SMTR 1.28, USB4105-GF-A 0.80). DigiKey figures below are therefore "page" confidence for stock and list price, but any US tariff surcharge added at US checkout was not visible and is unconfirmed.
- Mouser (`mouser.com`, `.ca`, `.sg`, `.be`), Newark, Arrow, Future and Verical timed out; Octopart, TrustedParts, OEMsecrets, Digipart, SnapEDA, TTI, RS Americas and Jameco returned 403; Findchips returned only non-US brokers or nothing. **No Mouser, Newark or Arrow stock or price was read. For most parts DigiKey is the only confirmed US source in these notes.**
- "price_5": DigiKey usually has no break at 5 pieces (breaks are 1 / 10 / 25). Where there is none, the 5-piece unit price equals the 1-piece price.
- Machine-readable rows: `pod0_parts_us.json` (61 rows, same folder).

## Which parts are in stock today at a US distributor, with how many, and what do 1 / 5 / 10 pieces cost?

### Takeaway
Every Pod-0 line except three discretes (Q1 FDN338P, Q2 FDN335N, D10/D11 PMEG2010AEH) is in DigiKey stock today in cut tape with no minimum, and those three have in-stock DigiKey substitutes. The October worry part J2 (Amphenol 12401610E4#2A) now shows 79,753 in stock; the OLED bare panel is not at DigiKey but Orient Display's own Seattle store lists 169.

### Cited Findings
Prices are USD per piece at 1 / 5 / 10; "lead" is the manufacturer standard lead time shown on the page.

Core ICs and module
- U1 ESP32-C3-MINI-1-N4: 1,231 in stock, 3.26 / 3.26 / 2.819, status "Not For New Designs" — [DigiKey](https://www.digikey.com.mx/en/products/detail/espressif-systems/ESP32-C3-MINI-1-N4/13877574)
- U1 ESP32-C3-MINI-1-N4X (active successor): 1,518 in stock, 3.26 / 3.26 / 2.819, lead 18 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/espressif-systems/ESP32-C3-MINI-1-N4X/27525554)
- U1 alternates on the DigiKey listing: 1U-N4X 7,498 at 3.38 (Active); 1U-H4X 2,516 at 3.50 (Active); 1U-N4 1,441 at 3.38 ("Discontinued at DigiKey"); N4-A 109 at 3.91; H4X 0 stock, reel-only 3,250 pcs; H4 0 stock "Discontinued at DigiKey"; 1U-H4 0 stock "Obsolete" — [DigiKey listing](https://www.digikey.com.mx/en/products/filter/rf-transceiver-modules-and-modems/872?keywords=ESP32-C3-MINI-1)
- U2 BMI270: 32,243 in stock, 4.23 / 3.788 / 3.624, Active, lead 16 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/bosch-sensortec/BMI270/9974486)
- U3 VEML7700-TT: 14,393 in stock, 1.44 / 1.272 / 1.209, Active, lead 29 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=VEML7700-TT)
- U4 AP2112K-3.3TRG1: 406,515 in stock, 0.28 / 0.28 / 0.191, Active, lead 24 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=AP2112K-3.3TRG1)
- U5 USBLC6-2SC6: 27,230 in stock, 0.54 / 0.54 / 0.329, Active, lead 25 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/stmicroelectronics/USBLC6-2SC6/1040559)
- U5 second source: ST eStore shows "In stock" (no quantity), 0.54 for 1-9 and 0.24 for 10-99, with a "Shipping Delays" banner; ship-from location not stated — [ST eStore](https://estore.st.com/en/usblc6-2sc6-cpn.html)
- U10 BQ24074RGTR: 1,901 in stock (+148 Marketplace), 2.43 / 2.43 / 1.807, Active, lead 26 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/texas-instruments/BQ24074RGTR/2047269)
- U10 BQ24074RGTT: 8,549 in stock, 3.19 / 3.19 / 2.388, Active, lead 26 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/texas-instruments/BQ24074RGTT/1957746)
- U10 at TI: lifecycle Active for both; the fetched page text showed "out of stock" with inventory and price behind login, so TI-store stock is unconfirmed — [TI](https://www.ti.com/product/BQ24074/part-details/BQ24074RGTR)
- U11 MAX17048G+T10: 21,841 in stock, 4.61 / 4.61 / 3.499, Active, lead 18 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/analog-devices-inc-maxim-integrated/MAX17048G-T10/3758921)

Discretes
- D1/D12/D13 LTST-C191KGKT (green): 2,228,010 in stock, 0.15 / 0.15 / 0.104 — [DigiKey](https://www.digikey.com.mx/en/products/detail/liteon/LTST-C191KGKT/386835)
- D1/D12/D13 LTST-C191KRKT (red): 1,334,425 in stock, 0.15 / 0.15 / 0.104 — [DigiKey](https://www.digikey.com.mx/en/products/detail/liteon/LTST-C191KRKT/386837)
- RT1 NCP15XH103F03RC: 122,042 in stock, 0.12 / 0.10 / 0.093, status "Not For New Designs" — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=NCP15XH103F03RC)
- Q1, Q2, D10/D11: zero stock, see the next question.

Connectors, cable, switches
- J1 FH12-30S-0.5SH(55): 4,414 in stock, 2.64 / 2.64 / 2.244, Active, lead 16 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=FH12-30S-0.5SH(55))
- J2 12401610E4#2A: 79,753 in stock, 1.43 / 1.43 / 1.217, Active, lead 16 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=12401610E4%232A)
- J10 USB4105-GF-A: 263,569 in stock, 0.80 / 0.80 / 0.676, Active, lead 7 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/gct/USB4105-GF-A/11198441)
- J3/J12 SM10B-SRSS-TB: 27,096 in stock, 0.93 / 0.93 / 0.788, Active, lead 16 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=SM10B-SRSS-TB)
- J11 S2B-PH-SM4-TB: 133,904 in stock, 0.49 / 0.49 / 0.417, Active, lead 16 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=S2B-PH-SM4-TB)
- CBL A10SR10SR30K152B (6 in, "Socket to Socket"): 106 in stock, 2.46 / 2.46 / 2.09, lead 8 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/jst-sales-america-inc/A10SR10SR30K152B/6708510)
- CBL A10SR10SR30K152A (6 in, "Socket to Socket, Reversed"): 5,019 in stock, 2.46 / 2.46 / 2.09 — [DigiKey](https://www.digikey.com.mx/en/products/detail/jst-sales-america-inc/A10SR10SR30K152A/6708483)
- CBL other lengths (qty-1 price, stock): 2 in 51A 531 at 1.90; 4 in 102A 117 at 2.33; 10 in 254A 613 at 3.17; 12 in 305B 580 at 3.28; 12 in 305A 465 at 3.57; 102B, 203A, 203B, 254B are 0 stock — [DigiKey listing](https://www.digikey.com.mx/en/products/result?keywords=A10SR10SR30K)
- SW1/SW2 PTS810SJM250SMTR LFS (160 gf): 69,043 in stock, 0.58 / 0.58 / 0.501, Active, lead 16 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/c-k/PTS810SJM250SMTR-LFS/4176610)
- SW10 PCM12SMTR: 2,726 in stock, 1.28 / 1.28 / 1.087, status "Obsolete" — [DigiKey](https://www.digikey.com.mx/en/products/detail/c-k/PCM12SMTR/1640112)

Display
- DS1 AOM12864A0-0.96WW direct from Orient Display: 169 in stock, 4.53 each, no quantity breaks shown, ships from Seattle, free US ground shipping over $50; page names the controller SSD1315 and does not state the FPC pin count — [Orient Display](https://orientdisplay.com/products/0-96-oled-128x64-monochrome-parallel-spi-iic-interface/)
- DS1 AOM12864A0-0.96WW at DigiKey: 0 in stock, 11.57 / 11.57 / 8.931, lead 8 weeks, controller listed as SSD1306, interface I2C/Parallel/SPI — [DigiKey](https://www.digikey.com.mx/en/products/detail/orient-display/AOM12864A0-0-96WW/22531883)
- DS1 AOM12864A0-0.96WW-ANO ("Arduino" SPI-only variant, 27.3 x 27.8 x 5.0 mm): 140 at DigiKey at 12.13 / 12.13 / 9.379; 211 at Orient Display at 8.01 (1-24) — [DigiKey](https://www.digikey.com.mx/en/products/detail/orient-display/AOM12864A0-0-96WW-ANO/22531926); [Orient Display](https://orientdisplay.com/products/0-96-arduino-oled-displays-128-x64-monochrome-white-4-wire-spi-interface/)
- DS1 Crystalfontz CFAL12864N-A-B4 (0.96 in, SSD1306): "0 Units In-Stock", 7.51 / 7.51 / 6.89, "solderdown foldable tab" tail (not ZIF) — [Crystalfontz](https://www.crystalfontz.com/product/cfal12864nab4)
- DS1 Midas MDOB128064V2V-WI: 326 on DigiKey Marketplace, ships from Midas in about 3 days, first price break is 2 pcs at 12.30, 10 at 11.36, I2C, SSD1306BZ — [DigiKey](https://www.digikey.com.mx/en/products/detail/midas-displays/MDOB128064V2V-WI/20841734)
- DS1 Focus LCDs O12864A-GLB-YW3 (blue, SSD1306): 61 on DigiKey Marketplace, 18.27 / 18.27 / 17.56 — [DigiKey](https://www.digikey.com.mx/en/products/detail/focus-lcds/O12864A-GLB-YW3/28872197)
- DS1 Newhaven: DigiKey search for "NHD-0.96-12864" returned no results — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=NHD-0.96-12864)

Battery
- BT1 Adafruit 1578 (500 mAh, JST-PH, protection): "In stock" (no count), 7.95 for 1-9, 7.16 for 10-99; page says "the batteries we sell do not have thermistors built-in" — [Adafruit](https://www.adafruit.com/product/1578)
- BT1 alternate SparkFun PRT-13851 (400 mAh, JST-PH, protection): in stock (no count), 7.98, 7.58 at 25+ — [SparkFun](https://www.sparkfun.com/lithium-ion-battery-400mah.html)
- BT1 alternate USE-702528-500PCBJST (500 mAh, 30 x 25.5 x 7.1 mm): 49 on DigiKey Marketplace, 5.49 / 5.49 / 5.19, page showed a separate $245.00 flat-rate shipping fee (Mexico storefront) — [DigiKey](https://www.digikey.com.mx/en/products/detail/us-electronics-inc/USE-702528-500PCBJST/15781479)

Passives (commodity, all in stock; price at qty 10 is per piece)
- CL05B104KO5NNNC: 10,000 in stock, 0.10 at 1, 0.021 at 10, lead 61 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=CL05B104KO5NNNC)
- GRM21BR71C475KE51L: 224,106 in stock, 0.13 at 1, 0.072 at 10 — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=GRM21BR71C475KE51L)
- RC0402FR-07620KL: 46,090 in stock, 0.11 at 1, 0.024 at 10 — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=RC0402FR-07620KL)
- RC0402FR-071K78L: 8,078 in stock, 0.10 at 1, 0.023 at 10 — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=RC0402FR-071K78L)
- RC0402FR-073K09L: 103,245 in stock, 0.11 at 1, 0.024 at 10 — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=RC0402FR-073K09L)

### Inferences
- The whole electronic BOM except the display and battery can be bought in one DigiKey order today, provided the three out-of-stock discretes are swapped (next question).
- The display should be bought direct from Orient Display (4.53 vs 11.57 at DigiKey, and DigiKey has none).
- For 5 units plus spares, buying 10 of each IC lands on the 10-piece break and costs little more than 5-6 pieces at the 1-piece price (e.g. BQ24074RGTR: 6 x 2.43 = 14.58 vs 10 x 1.807 = 18.07).

### Gaps
- No Mouser, Newark, Arrow, Future or TI-store stock/price could be read (timeouts, 403, login). A "second confirmed US source" exists only for USBLC6-2SC6 (ST eStore), the OLED and the battery.
- The 30-pin 0.5 mm FPC pinout of AOM12864A0-0.96WW was not confirmed on any page read; the Orient Display page says SSD1315 while DigiKey says SSD1306. Read the datasheet before ordering.
- Winstar WEO012864D, DisplayModule (HTTP 429) and BuyDisplay (HTTP 403; ships from China, non-US) were not read. Amazon US was not checked.
- VEML7700-TT, AP2112K, FH12, SM10B, S2B, NTC and passive pages were reached by keyword search, so their DigiKey product ids were not captured; the JSON uses the MPN search URL for these.
- Unit counts at Adafruit and SparkFun are not shown on their pages.

## Which parts are NOT buildable from US stock right now, and what is the best in-stock substitute for each?

### Takeaway
Three BOM parts have zero DigiKey stock and very long factory lead times: FDN338P (57 weeks), FDN335N (70 weeks) and PMEG2010AEH (30 weeks, next delivery October 2027). In-stock SOT-23 / SOD-123F substitutes exist for all three; four more parts are flagged for lifecycle (U1 N4 NRND, RT1 NRND, SW10 obsolete) or pinout risk (DS1).

### Cited Findings
- Q1 FDN338P: 0 in stock, lead 57 weeks, 0.73 / 0.73 / 0.452 — [DigiKey](https://www.digikey.com.mx/en/products/detail/onsemi/FDN338P/458848)
- Q1 substitute DMG2305UX-7 (Diodes Inc, P-ch 20 V, 4.2 A, 52 mOhm @ 4.5 V, Vgs(th) 0.9 V, Vgs max +-8 V, SOT-23-3): 74,154 in stock, 0.38 / 0.38 / 0.232, lead 40 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=DMG2305UX-7)
- Q1 other candidates are also out: FDN340P 0 stock, lead 67 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/onsemi/FDN340P/965304); SI2301CDS-T1-GE3 0 stock, 3,000 expected 07-May-2027, lead 55 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=SI2301CDS-T1-GE3)
- Q2 FDN335N: 0 in stock, lead 70 weeks, 0.76 / 0.76 / 0.468 — [DigiKey](https://www.digikey.com.mx/en/products/detail/onsemi/FDN335N/965301)
- Q2 substitute FDN337N (onsemi SuperSOT-3, N-ch 30 V, 2.2 A, 65 mOhm @ 4.5 V, Vgs(th) max 1 V, Vgs max +-8 V): 222,614 in stock, 0.77 / 0.77 / 0.479, lead 17 weeks — [DigiKey](https://www.digikey.com.mx/en/products/detail/onsemi/FDN337N/458847)
- Q2 second substitute SI2302CDS-T1-GE3 (N-ch 20 V, 2.6 A, 57 mOhm @ 4.5 V): 185,491 in stock, 0.82 / 0.82 / 0.507 — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=SI2302CDS-T1-GE3)
- Q2 DMG2302UK-7: only 13 in stock — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=DMG2302UK-7)
- D10/D11 PMEG2010AEH,115: 0 in stock, "3,000 expected in stock on 04-Oct-2027", lead 30 weeks, 0.37 / 0.37 / 0.23 — [DigiKey](https://www.digikey.com.mx/en/products/detail/nexperia-usa-inc/PMEG2010AEH-115/1157670)
- D10/D11 substitute MBR120VLSFT1G (onsemi, 20 V 1 A Schottky, package/case SOD-123F, supplier package SOD-123FL): 105,142 in stock, 0.61 / 0.61 / 0.373, lead 41 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=MBR120VLSFT1G)
- D10/D11 same-family options: PMEG2010AEH-QX 0 stock; PMEG2010EH-QX (SOD-123F) listed at 2,990 in stock, 0.41 at 1; PMEG2010ER,115 141,302 in stock but SOD-123W package — [DigiKey listing](https://www.digikey.com.mx/en/products/filter/single-diodes/280?keywords=PMEG2010)
- D10/D11 PMEG2010EH,115: 0 in stock, 3,000 expected 25-Oct-2027, Vf max 500 mV @ 1 A — [DigiKey](https://www.digikey.com.mx/en/products/detail/nexperia-usa-inc/PMEG2010EH-115/1157675)
- D10/D11 PMEG2010EH-QX detail page: lead 26 weeks, Vf max 500 mV @ 1 A, 0.41 / 0.41 / 0.251; the detail page did not show a clear in-stock figure for this part — [DigiKey](https://www.digikey.com.mx/en/products/detail/nexperia-usa-inc/PMEG2010EH-QX/21413746)
- U4 alternate TLV75733PDBVR: 0 in stock, 3,000 expected 14-Dec-2026, lead 26 weeks (the primary AP2112K is in stock, so no action) — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=TLV75733PDBVR)
- U1 ESP32-C3-MINI-1-N4 is "Not For New Designs"; N4X is Active at the same price — [DigiKey N4](https://www.digikey.com.mx/en/products/detail/espressif-systems/ESP32-C3-MINI-1-N4/13877574); [DigiKey N4X](https://www.digikey.com.mx/en/products/detail/espressif-systems/ESP32-C3-MINI-1-N4X/27525554)
- U1: a Mouser listing (search snippet, not read on page) marks the N4 NRND and says the N4X's ESP32-C3FH4X chip needs chip revision v1.1 or above — [Mouser](https://www.mouser.sg/ProductDetail/356-ESP32-C3-MINI1N4)
- RT1 NCP15XH103F03RC is "Not For New Designs"; DigiKey's "MFR Recommended" substitute NCU15XH103F60RC (10k 1 %, B25/50 3380K 1 %, 0402) has 39,686 in stock at 0.10 / 0.068 / 0.065, lead 17 weeks — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=NCU15XH103F60RC)
- SW10 PCM12SMTR is "Obsolete": "This product is no longer manufactured and will no longer be stocked once stock is depleted"; DigiKey "Similar" substitutes are Nidec CUS-12TB (74,403 at 0.86) and Same Sky SLW-662712-15A-SMT-TR (5,400 at 0.50) — [DigiKey](https://www.digikey.com.mx/en/products/detail/c-k/PCM12SMTR/1640112)
- SW10: C&K PCM12SMTBR is Active but has 0 DigiKey stock, only a 3,500-piece Marketplace lot at 0.65 — [DigiKey listing](https://www.digikey.com.mx/en/products/filter/slide-switches/213?keywords=PCM12)
- U11 MAX17048G+U: DigiKey search returns no results; MAX17048G+ and MAX17048G+TG4K are 0 stock, "discontinued at DigiKey" — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=MAX17048G)
- J2 alternates in stock if ever needed (24-position, qty-1 price): Amphenol 12401598E4#2A 55,770 at 1.80; GCT USB4056-03-A 51,944 at 0.93; GCT USB4115-03-C 50,026 at 1.04; Amphenol 12402032E202A 17,796 at 1.10; GCT USB4080-03-A 22,842 at 1.11; JAE DX07S024JA1R1300 4,743 at 2.14 — [DigiKey listing](https://www.digikey.com.mx/en/products/filter/usb-dvi-hdmi-connectors/312?keywords=USB-C%20receptacle%2024%20position%20USB%203.2%20surface%20mount)
- BT1 with NTC lead: DigiKey's rechargeable-battery search for 500 mAh LiPo returned only two parts, neither mentioning a thermistor — [DigiKey listing](https://www.digikey.com.mx/en/products/filter/batteries-rechargeable-secondary/91?keywords=lithium%20polymer%20500mAh)

### Inferences
- Recommended swaps for this build: Q1 -> DMG2305UX-7, Q2 -> FDN337N, D10/D11 -> MBR120VLSFT1G, U1 -> ESP32-C3-MINI-1-N4X (or buy the remaining N4), RT1 -> keep NCP15 (122k in stock) or NCU15XH103F60RC.
- The three MOSFET/diode substitutes share the SOT-23-3 / SOD-123F outline but pin order, gate threshold in the actual circuit, forward voltage and pad fit were not checked against datasheets here; that check is needed before ordering.
- MBR120VLSFT1G's forward voltage was not read; PMEG2010AEH is the low-Vf grade, so power-path drop may differ.
- SW10 is fine for 1-5 units from the remaining 2,726, but a later revision should move to an active slide switch; the DigiKey "Similar" parts are not confirmed footprint-compatible.
- Since no thermistor-equipped 500 mAh cell was found, the board NTC (RT1) placed against the cell remains the practical route for the BQ24074 TS pin.
- Single confirmed source (DigiKey only): everything except USBLC6-2SC6, the OLED and the battery. This reflects what could be read, not necessarily real-world availability at Mouser/Newark.

### Gaps
- No Espressif PCN or product-status page was read; the N4 NRND status rests on the DigiKey page and a Mouser snippet.
- onsemi, Nexperia and Murata manufacturer lifecycle pages were not read (onsemi page exceeded the fetch size limit); lifecycle statuses here are DigiKey's.
- Whether Mouser or Newark hold FDN338P, FDN335N or PMEG2010AEH stock is unknown.
- H4 (105 C) module in the PCB-antenna version: no US stock found (H4X is reel-only 3,250 MOQ with 0 stock).

## What are DigiKey's and Mouser's current US shipping fees and free-shipping thresholds, and typical delivery days?

### Takeaway
DigiKey US shipping starts at $4.99 (USPS Ground Advantage) and $8.49 for FedEx/UPS Ground, with no free-shipping threshold listed on its rate page. Mouser's US terms could not be read on mouser.com and remain unconfirmed.

### Cited Findings
- DigiKey US rates per shipment: USPS Ground Advantage $4.99; FedEx Ground $8.49; UPS Ground $8.49; FedEx Economy / UPS 2nd Day Air $13.99; USPS Priority Mail $13.99; FedEx Saver 3-day $16.99; UPS 3-Day $17.99; overnight PM $26.99; overnight AM $29.99 — [DigiKey](https://www.digikey.com/en/help-support/delivery-information/delivery-time-and-cost)
- DigiKey: no free-shipping threshold is listed on that page; Marketplace items ship from the vendor and may carry separate shipping charges, "Average Time to Ship 1-3 Days" for Marketplace products — [DigiKey](https://www.digikey.com/en/help-support/delivery-information/delivery-time-and-cost)
- DigiKey: "shipping rates starting as low as $4.99" for US customers — [DigiKey](https://www.digikey.com/en/help-support/delivery-information)
- DigiKey Digi-Reel orders add a $7.00 reeling fee (not needed for cut tape) — [DigiKey](https://www.digikey.com.mx/en/products/detail/bosch-sensortec/BMI270/9974486)
- Mouser (search snippet only, non-US Mouser pages): "Free shipping on most orders over $90 (USD)" appears on Mouser's UK pressroom page, while other Mouser country sites show $60; these look like international-order thresholds — [Mouser UK](https://www.mouser.co.uk/pressroom)
- Mouser (search snippet only): "2-day shipping as low as $9.99 every day for UPS and FedEx orders up to 10 pounds" — [Mouser](https://www.mouser.sg/best-2-day-shipping/)
- Adafruit delivery times: USPS Ground Advantage 2-5 business days (orders under $200), UPS Ground 3-5 days; orders usually ship within 1-2 business days; no general free-shipping threshold stated — [Adafruit](https://www.adafruit.com/shipping)
- Crystalfontz: "$15 Flat Rate Ground on orders over $100 | FREE Ground Shipping on orders over $300! U.S. Only" — [Crystalfontz](https://www.crystalfontz.com/product/cfal12864nab4)
- Orient Display: free ground shipping on US orders over $50 (excluding AK and HI), ships from Seattle — [Orient Display](https://orientdisplay.com/products/0-96-oled-128x64-monochrome-parallel-spi-iic-interface/)

### Inferences
- A one-box DigiKey order for this BOM costs $4.99-$8.49 to ship; with Adafruit (battery) and Orient Display (OLED) that makes three shipments.
- A LiPo cell ships ground-only in practice, so the battery sets the slowest delivery (Adafruit ground 2-5 business days after 1-2 days handling).

### Gaps
- Mouser US domestic rates, any US free-shipping threshold and delivery days: unconfirmed (mouser.com help pages timed out; the snippets above come from non-US Mouser sites).
- DigiKey transit days for ground and order cut-off time were not on the pages read.
- Adafruit and SparkFun shipping prices were not read.
- US tariff surcharges at DigiKey checkout were not visible.

## Do any of these parts carry a minimum order quantity or reel-only packaging that hurts a 1-5 unit run?

### Takeaway
No BOM part needs a reel or a minimum above 1 at DigiKey: every in-stock line is sold in cut tape (or bulk for the cable) from 1 piece. The only MOQ traps are among alternates and Marketplace listings.

### Cited Findings
- All DigiKey lines read (ICs, discretes, connectors, switches, passives) list a "Cut Tape (CT)" price table starting at quantity 1; e.g. BMI270 breaks at 1/5/10/25, with tape-and-reel only from 5,000 — [DigiKey](https://www.digikey.com.mx/en/products/detail/bosch-sensortec/BMI270/9974486)
- ESP32-C3-MINI-1-H4X is reel-only: single price break at 3,250 pcs, 0 in stock — [DigiKey listing](https://www.digikey.com.mx/en/products/filter/rf-transceiver-modules-and-modems/872?keywords=ESP32-C3-MINI-1)
- BQ24074RGTR: cut tape from 1; the 148 Marketplace pieces are a bulk lot priced at 148 pcs; full reel is 3,000 (RGTT reel is 250) — [DigiKey listing](https://www.digikey.com.mx/en/products/result?keywords=BQ24074RGT)
- JST cable assemblies are "Bulk" with a price break at 1 — [DigiKey](https://www.digikey.com.mx/en/products/detail/jst-sales-america-inc/A10SR10SR30K152B/6708510)
- Midas MDOB128064V2V-WI (Marketplace): lowest price break is 2 pieces — [DigiKey](https://www.digikey.com.mx/en/products/detail/midas-displays/MDOB128064V2V-WI/20841734)
- PCM12SMTBR (active PCM12 variant): only a 3,500-piece Marketplace lot — [DigiKey listing](https://www.digikey.com.mx/en/products/filter/slide-switches/213?keywords=PCM12)
- USE-702528-500PCBJST battery (Marketplace): separate $245.00 flat-rate shipping fee shown — [DigiKey](https://www.digikey.com.mx/en/products/detail/us-electronics-inc/USE-702528-500PCBJST/15781479)
- Passives drop steeply at 10: e.g. RC0402FR-07620KL is 0.11 at 1 and 0.024 at 10 — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=RC0402FR-07620KL)
- MBR120VLSFT1G has a maximum purchase limit of 27,000 per 30 days (irrelevant at this scale) — [DigiKey](https://www.digikey.com.mx/en/products/result?keywords=MBR120VLSFT1G)

### Inferences
- Order passives and cheap discretes in 10s or more: 10 resistors cost about the same as 2.
- Avoid DigiKey Marketplace lines for this run (separate shipment, separate shipping fee, MOQs); the battery Marketplace listing in particular is uneconomic if the $245 fee also applies to US orders.
- Thin stock to watch: A10SR10SR30K152B cable (106), DMG2302UK-7 (13), ESP32-C3-MINI-1-N4 (1,231, NRND), PCM12SMTR (2,726, obsolete), Orient Display OLED (169, single source).

### Gaps
- The cable's A vs B suffix: DigiKey labels 152B "Socket to Socket" and 152A "Socket to Socket, Reversed"; which pin-1 mapping Pod-0 needs was not checked against the schematic.
- Whether the $245 Marketplace battery shipping fee applies to US addresses is unconfirmed (read on the Mexico storefront).
- Mouser / Newark minimums were not readable.
