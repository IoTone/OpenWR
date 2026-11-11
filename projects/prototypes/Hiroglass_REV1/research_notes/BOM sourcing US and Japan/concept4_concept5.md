# Concept 4 (micro-OLED driver experiment) and Concept 5 (Linux puck + USB-C display glasses): sourcing in the US and Japan

All observations checked 2026-11-10. Confidence tags used below:
**[page]** = read on the seller's own page or its store JSON today; **[agg]** = read on an aggregator (findchips), not the seller; **[snippet]** = search-result snippet only, possibly stale; **[unconfirmed]** = not obtained.
Yen prices are tax-included unless stated. Dollar prices exclude sales tax and shipping.
DigiKey, Mouser, RS Japan, Tindie store index, Crowd Supply, Octopart, oemsecrets, Micro Center and the Monado GitLab all refused automated fetches today (403 / timeout / bot wall); Amazon, AliExpress, Taobao, eBay, Mercari and Yahoo Auctions were not readable. Rows that depend on them are marked unconfirmed. The per-row data is in `concept4_concept5.json` (80 rows).

## Verdicts

### Takeaway
| Build | US-only sellers | Japan-deliverable sellers |
|---|---|---|
| Concept 4 | **Blocked** under the strict rule. Gating item: PANEL. No seller shipping an ECX336-class panel from US stock was found. Second issue: iCEBreaker is sold out at 1BitSquared (substitute exists). | **Buildable with substitutions.** Panel from Hong Kong/China sellers at $219-$275 with HDMI board (not $99.50); FPGA = Tang Nano 9K/20K from Akizuki instead of iCEBreaker; LVL and LDO sources unconfirmed. |
| Concept 5 | **Buildable now** with XREAL Air 2 ($199, the only current glasses in Monado's list). The HDMI-to-USB-C adapter is the weak item (VITURE cable backordered 1 week, cross-brand use not stated). | **Buildable with substitutions.** Pi 5 8 GB in stock at Akizuki and Switch Science; glasses = XREAL 1S or One Pro (XREAL One and Air 2 are no longer sold standalone in the JP shop); BNO085 = Switch Science Conta board; no HDMI-to-USB-C adapter source confirmed. |

### Cited Findings
See the sections below; each verdict rests on the rows cited there.

### Inferences
- If "US-only" is relaxed to "US buyer importing from Hong Kong/China", Concept 4 in the US becomes buildable with substitutions, with the same panel sources as Japan.
- Concept 5's real technical gate is not stock but driver support: Monado lists only XREAL Air / Air 2 / Air 2 Pro. In Japan those are exactly the models that are no longer sold new by XREAL, so the Japan build either uses a used Air 2 class unit (price unconfirmed) or uses a One-family unit through the community XRLinuxDriver rather than Monado's own driver.

### Gaps
- DigiKey/Mouser stock and price for the two TI parts in either region.
- Any used-market sold prices (eBay, Mercari, Yahoo Auctions).

## Concept 4: which micro-OLED panels can be bought at 1-5 pieces, and from whom

### Takeaway
The Sony ECX336 0.23 inch 640x400 panel is buyable in single units today from three China/Hong Kong-based sellers, but at $219-$397 with an HDMI board, not the $99.50 in the October BOM. No US-stocked seller and no Japanese small-quantity Sony channel was found.

### Cited Findings
- Tindie, Display Components (Kowloon, Hong Kong), "0.23 inch Micro OLED 640(RGB)x400 with HDMI Controller Board": **$275.00**, 800 cd/m2, display + HDMI board, prism optional (prism 800 in stock; display stock not shown; shipping table blank in the fetch) [page] — [Tindie](https://www.tindie.com/products/oled-modules/microdisplay-023-inch-micro-oled-display-640x400/)
- Same seller, kit with optics and transfer cable: **$294.00**; "51 pin, 15.6 x 13.6 mm", no part number given [page] — [Tindie](https://www.tindie.com/products/oled-module/oled-microdisplay-023-640x400-ar-vr-display)
- Same seller, "3000 nits" kit with HDMI board and prism: **$397.00**; the spec table on the same page says 800 cd/m2, which conflicts with the title [page] — [Tindie](https://www.tindie.com/products/oled-modules/micro-023-inch-oled-dispaly-for-ar-vr-devices/)
- Tindie, Hicenda (Shenzhen), 0.23 inch 640x400 800 cd/m2 with HDMI driver board: **$219.00**, ships from China [page] — [Tindie](https://www.tindie.com/products/hicenda/microdisplay-023-micro-oled-display-640400)
- Hicenda own store, 4000 nit 0.23 inch module: **$389.00**, SKU HTECX336CN, available [page, store JSON] — [Hicenda](https://www.hicendadisplay.com/products/0-23-inch-640x400-oled-microdisplay-module-4000nits-60hz-for-ar-vr)
- Hicenda own store, "0.23 inch 640x400 optical module for helmet-mounted": **$184.00**, SKU HTECX336CN-2, available. This is the "$184 standard version"; it is an optics module listing [page, store JSON] — [Hicenda](https://www.hicendadisplay.com/products/0-23-inch-micro-oled-display-640x400-optical-module-for-helmet-mounted)
- DisplayModule, "0.23 inch OLEDoS Sony Micro Display 640x400 1600 nits": panel only **$139**, HDMI adapter only **$119**, panel + adapter **$239**; SKU DM-OLED023-683 / DM-ADTTR-048; inventory field 60 (the same 60 appears on its other listings, so treat as a placeholder) [page, store JSON] — [DisplayModule](https://www.displaymodule.com/products/0-23-inch-micro-oled-display-640x400)
- DisplayModule, 2400 nit 0.23 inch version: **$599** [page, store JSON] — [DisplayModule](https://www.displaymodule.com/products/0-23-inch-micro-oled-display)
- DisplayModule, non-Sony 0.23 inch 640x400 AMOLED microdisplay, MIPI DSI + I2C: **$99**, panel only [page, store JSON] — [DisplayModule](https://www.displaymodule.com/products/dmrxo0023wgna-0-23-inch-amoled-microdisplay-640x400-3255ppi)
- Other sizes buyable singly at DisplayModule (store search, all "available"): 0.39 inch 1024x768 $99-$169; 0.39 inch 1920x1080 MIPI $139; 0.49 inch 1920x1080 MIPI $149-$299; 0.72 inch 1920x1200 MIPI $179; 1.03 inch 2560x2560 $299; HDMI micro driver boards $199 [page, store search JSON] — [DisplayModule](https://www.displaymodule.com/)
- Hicenda other sizes: 0.39 inch 1024x768 500 nit with HDMI board $189; 0.71 inch 1920x1080 LVDS with HDMI board $310; 0.5 inch 1280x960 with optics $375 [page, store search JSON] — [Hicenda](https://www.hicendadisplay.com/)
- Sony's ECX336C through distribution is quote-only: the FRAMOS page shows no price, MOQ, stock or datasheet and redirects purchases to sister company Restar Framos [page] — [FRAMOS](https://framos.com/products/microdisplays/ecx336c-23552/)
- A search for Japanese small-quantity channels (Chip One Stop, Macnica, Marubun, Restar) returned no listing for ECX336/ECX335/ECX343; Macnica's European page lists other Sony models (ECX334, ECX337, ECX339, ECX342) [snippet] — [Macnica EU](https://www.macnica.com/eu/atd-europe/en/products-support/suppliers/sony/sony-oled-microdisplay/)
- DisplayModule search snippet: "Factory bulk lead time: 3 weeks", and the HDMI adapter is three parts (driver board, FPC cable, display connect board) [snippet] — [DisplayModule](https://displaymodule.com/products/0-23-inch-micro-oled-display)

### Inferences
- The $99.50 Tindie figure in the BOM could not be reproduced; the closest live prices are $219 (Hicenda on Tindie) and $239 (DisplayModule panel + adapter). Budget $220-$280 per panel with HDMI board.
- Hicenda's SKU "HTECX336CN" indicates the Sony ECX336CN; Display Components' "51 pin" matches the ECX336 pin count in the datasheet below. Neither seller states the Sony part number in the listing text.
- For Japan, all of these count as China/Hong Kong-origin sellers; delivery to Japan is plausible but was not confirmed on any page.

### Gaps
- Whether DisplayModule ships from the US (not established; ship-from origin unconfirmed), which decides the US-only verdict.
- AliExpress, Taobao, Panelook listings: not readable.
- BOE, SeeYA, Kopin, eMagin, Visionox small-quantity availability: not researched beyond the unbranded DisplayModule/Hicenda listings.
- Real lead time and stock for every panel seller; none publishes a credible count.

## Concept 4: is a full datasheet with pinout and init sequence public?

### Takeaway
Partly. A 36-page Sony ECX336BF datasheet (tentative, Ver. 0.3E) is openly downloadable with pinout, register map and power sequence, but it withholds the timing-register values ("setting value separately submitted"), so a complete init sequence is not in the public document.

### Cited Findings
- DisplayModule hosts the ECX336BF datasheet PDF without login: 36 pages, "Ver.0.3E Tentative", 51-pin assignment table (VDD1 1.8 V, VDD2 10 V, VCATH, MCLK, XHD, XVD, XCLR, DATA0-23, XSCK/XCS/SI/SO), parallel RGB 24-bit / YCbCr input, serial register map (section 10.1.1), register descriptions, power on/off sequence (section 9). No confidentiality marking found in the text [page, PDF text extracted locally] — [ECX336BF datasheet](https://cdn.shopify.com/s/files/1/0264/7629/files/DM-OLED023-683datasheet.pdf)
- Power-on sequence as written: XCLR low and 1.8 V on; 1 ms after 1.8 V is up set XCLR high (panel enters power-save); turn on 10 V; write all registers and clear PS over serial [page] — [ECX336BF datasheet](https://cdn.shopify.com/s/files/1/0264/7629/files/DM-OLED023-683datasheet.pdf)
- About 35 timing registers (H_ACT_U, V_ACT_U, DE_U, WSST1_U, WSEN1_U, VCK_U, SIGSEL*, SEL*, AZEN* ...) are described only as "Timing setting register (setting value separately submitted)" [page] — [ECX336BF datasheet](https://cdn.shopify.com/s/files/1/0264/7629/files/DM-OLED023-683datasheet.pdf)
- Brilliant Labs' open-source Monocle firmware documents a driver for the sister panel ECX335AF: `ecx335af_prepare()`, `ecx335af_init()` (configures each setting over SPI), `ecx335af_set_luminance()`; it says no public ECX335AF datasheet exists and cites the ECX336CN datasheet section 10.8 for luminance values [page] — [Monocle docs](https://monocle-docs.readthedocs.io/en/latest/firmware_driver_api/ecx335af.html)
- A search for an open-source ECX336 driver repository found none [snippet] — [search via Monocle docs source](https://github.com/Itsbrilliantlabs/monocle-micropython/blob/main/docs/firmware_driver_api/ecx335af.rst)

### Inferences
- The panel needs a 10 V rail and a cathode supply in addition to 1.8 V; the BOM's 1.8 V LDO alone is not sufficient for a bare panel. The sellers' HDMI boards provide these rails.
- The practical open route to the missing timing values is to read the register writes from the Monocle firmware source (ECX335AF, same family) or to capture the SPI traffic of a purchased HDMI driver board. Neither was verified here.
- The public document is the BF revision; modules on sale appear to be CN. Register compatibility between BF and CN is not established.

### Gaps
- Whether the Monocle source actually contains the full register table and whether it applies to ECX336: the code itself was not opened.
- Legal status of the hosted Sony datasheet (a reseller copy, not a Sony publication).

## Concept 4: MCU, level translator, breakout, LDO

### Takeaway
The Pico 2 is plentiful in both regions. The two TI parts and the TSSOP-24 breakout could not be confirmed at an authorised distributor because DigiKey and Mouser blocked reads.

### Cited Findings
- Pico 2 SC1631, PiShop.us: $5.00, in stock, no limit stated [page] — [PiShop.us](https://www.pishop.us/product/raspberry-pi-pico-2/)
- Pico 2 SC1631, Newark: 888 in stock, $3.52 [agg] — [findchips](https://www.findchips.com/search/SC1631)
- Pico 2, Akizuki 129604: Y990 tax incl., 1,462 orderable online [page] — [Akizuki](https://akizukidenshi.com/catalog/g/g129604/)
- Pico 2, Switch Science 9809: Y1,100, available [page, store JSON] — [Switch Science](https://www.switch-science.com/products/9809)
- SN74AXC8T245PWR on TI.com: "Out of stock", price hidden behind login [page] — [TI](https://www.ti.com/product/SN74AXC8T245/part-details/SN74AXC8T245PWR)
- SN74AXC8T245PWR on findchips: only brokers shown (Win Source 19,850 at $0.556-$0.714; Maritex 173 at $0.635-$1.254) [agg] — [findchips](https://www.findchips.com/search/SN74AXC8T245PWR)
- TLV75718PDBVR on findchips: Bristol Electronics 214 ($0.21-$0.60), TME 97 ($0.171-$0.64), Win Source 24,490; LCSC lists a clone "TLV75718PDBVR-MS" (MSKSEMI) 2,955 at $0.09-$0.18 [agg] — [findchips](https://www.findchips.com/search/TLV75718PDBVR)
- Adafruit 1207 is a SOIC-16/TSSOP-16 breakout 3-pack at $3.95, 81 in stock; it is not 24-pin [page] — [Adafruit](https://www.adafruit.com/product/1207)
- Akizuki search for "TSSOP 24" returns no 24-pin board; nearest is AE-HTSSOP28-DIP, Y80, 2,728 in stock [page] — [Akizuki](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=TSSOP+24)

### Inferences
- These three items are commodity and unlikely to gate either build, but no authorised-distributor stock number was read today, so they are rated "watch", not "ok".

### Gaps
- DigiKey / Mouser / DigiKey JP / Mouser JP stock and 1-piece and 5-piece prices for both TI parts.
- A confirmed TSSOP-24 (0.65 mm) to DIP adapter in either region.

## Concept 4: FPGA board

### Takeaway
The iCEBreaker is sold out at its maker. The nearest in-stock substitutes are the Lattice iCE40UP5K-B-EVN in the US (Newark, 18 pcs) and the Tang Nano 9K / 20K in Japan (Akizuki, 101 / 52 pcs).

### Cited Findings
- 1BitSquared iCEBreaker V1.1b: $79.95, sold out (store JSON available=false, inventory 0); iCEBreaker Bitsy $42 also sold out [page] — [1BitSquared](https://1bitsquared.com/products/icebreaker)
- Mouser ICEBREAKER-V1_1: $80.00, 118 in stock, 134 expected 19/06/2026 — undated cached snippet, the expected date has passed; live page timed out [snippet] — [Mouser](https://www.mouser.com/c/?m=1BitSquared)
- Lattice ICE40UP5K-B-EVN, Newark: 18 in stock, $65.62 [agg] — [findchips](https://www.findchips.com/search/ICE40UP5K-B-EVN)
- Akizuki Tang Nano 9K (117448): Y3,780 tax incl. (raised from Y2,980 on 2026-06-19), 101 online, 8 Akihabara, 3 Yashio [page] — [Akizuki](https://akizukidenshi.com/catalog/g/g117448/)
- Akizuki Tang Nano 20K (130974): Y6,280, 52 online; Tang Nano 1K Y1,730 (74); Tang Nano 4K camera set Y2,980 (82) [page] — [Akizuki](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=Tang+Nano)
- Switch Science: no current Tang Nano listing (search returns only discontinued Sipeed items); Fomu PVT1 (iCE40UP5K) Y5,692, available [page, store JSON] — [Switch Science](https://www.switch-science.com/products/6380)

### Inferences
- Tang Nano boards use Gowin FPGAs, so moving from iCEBreaker changes the toolchain (open-source Gowin flow via Apicula exists) and the pin plan, but both have enough I/O for 24-bit RGB plus sync.
- Fomu shares the iCEBreaker's FPGA but exposes only four pads, so it cannot drive the panel.

### Gaps
- Crowd Supply, DigiKey stock for iCEBreaker; iCESugar, OrangeCrab, ULX3S availability; Tang Nano at US sellers; Marutsu and Amazon.co.jp listings. None read.

## Concept 5: Raspberry Pi 5 price, stock, limits and giteki

### Takeaway
The Pi 5 8 GB is in stock today in both regions: $175 at PiShop.us and CanaKit, Y35,200 at Akizuki (25 pcs) and Y36,300 at Switch Science (20 pcs). The 16 GB went to $385 on 2 November 2026. The Pi 5 carries Japanese construction-design certification (giteki).

### Cited Findings
- Official: from 2 November 2026 the 16 GB Pi 5 rises $80 to $385; 16 GB Compute Module 5 variants rise $40 [snippet of official post] — [Raspberry Pi news](https://raspberrypi.com/news/price-rises-for-some-16gb-raspberry-pi-products/)
- The raspberrypi.com Pi 5 product page still read "16GB ... $305" when fetched today, and shows no price for other sizes [page] — [Raspberry Pi 5 product page](https://www.raspberrypi.com/products/raspberry-pi-5/)
- 8 GB list price is $175 since the April 2026 round (+$50); earlier rounds December 2025 and February 2026 [snippet, press] — [TechRadar](https://www.techradar.com/pro/raspberry-pi-5-price-hike-means-the-16gb-model-now-costs-three-times-what-it-did-at-launch-and-of-course-the-global-ram-shortage-is-to-blame)
- PiShop.us: 4 GB $110, 8 GB $175, 16 GB $385, all "in stock", no purchase limit stated [page] — [4 GB](https://www.pishop.us/product/raspberry-pi-5-4gb/), [8 GB](https://www.pishop.us/product/raspberry-pi-5-8gb/), [16 GB](https://www.pishop.us/product/raspberry-pi-5-16gb/)
- CanaKit 8 GB: $175.00, in stock, no limit stated [page] — [CanaKit](https://www.canakit.com/raspberry-pi-5-8gb.html)
- SparkFun 8 GB: $180.00, in stock [page] — [SparkFun](https://www.sparkfun.com/raspberry-pi-5-8gb.html)
- Adafruit: 8 GB $200.00, 4 GB $130.00, 16 GB $385.00; in stock; **max 2 per customer** [page] — [Adafruit 5813](https://www.adafruit.com/product/5813), [Adafruit 5812](https://www.adafruit.com/product/5812)
- Akizuki: 4 GB Y22,330 (10, delivery date being confirmed), 8 GB Y35,200 (25), 16 GB Y60,390 (302), 2 GB Y15,180 (no restock date), 8 GB starter kit Y40,500 (9) [page] — [Akizuki](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=Raspberry+Pi+5)
- Switch Science: 8 GB Y36,300 tax incl., stock 20; 4 GB Y22,990 sold out; 16 GB Y78,980 available. The page carries a price-surge notice dated 1 April 2026 and says prices are reviewed periodically [page] — [8 GB](https://www.switch-science.com/products/9250), [16 GB](https://www.switch-science.com/products/10055)
- Switch Science states the Pi 5 8 GB "has obtained construction design certification (giteki) and displays the giteki mark" inside the box and on the board [page] — [Switch Science](https://www.switch-science.com/products/9250)
- Raspberry Pi publishes a Japan approvals document for the Pi 5 (RP-005374-CF) [snippet] — [Raspberry Pi PIP](https://pip.raspberrypi.com/categories/894-approvals/documents/RP-005374-CF/rpi-5-Japan.pdf)
- RS Japan snippet of Y17,409 ex tax for the 8 GB is stale (refers to April/May 2026 arrivals); the page returned 403 today [snippet] — [RS Japan](https://jp.rs-online.com/web/p/raspberry-pi/0219255)

### Inferences
- Five 8 GB boards: PiShop.us or CanaKit in one order (no stated limit); Adafruit cannot supply five. In Japan, Akizuki's 25 or Switch Science's 20 cover five.
- Akizuki's 16 GB at Y60,390 against Switch Science's Y78,980 suggests Akizuki has not yet applied the November increase.
- The BOM's $175 for the 8 GB is correct as of today.

### Gaps
- Micro Center (403), DigiKey, KSY (page rendered empty), Marutsu, Sengoku, Amazon.co.jp prices.
- The giteki certificate number itself was not read.
- Explicit per-customer limits at PiShop.us, CanaKit, Akizuki, Switch Science (none shown on the pages).

## Concept 5: alternate SBCs with USB-C DisplayPort alt mode

### Takeaway
ROCK 5B and 5B+ have DP alt mode on USB-C, which would remove the HDMI adapter, but every ROCK 5B/5B+ variant is unavailable at Arace today and no other in-stock source was confirmed in either region.

### Cited Findings
- Arace ROCK 5B+: all eight RAM/eMMC variants available=false; listed prices are inconsistent (4 GB $75, 8 GB $175, 16 GB $135, 24 GB $175, 32 GB $215) [page, store JSON] — [Arace](https://arace.tech/products/radxa-rock-5b-plus)
- Arace ROCK 5B: 4 GB $79.90, 8 GB $99.90, 16 GB $139.90, all unavailable; ROCK 5T and ROCK 5 ITX also unavailable; only ROCK 5C ($34.90 up) in stock [page, store JSON] — [Arace](https://arace.tech/products/radxa-rock-5b)
- ROCK 5B+ USB-C is "USB 3.1 Gen 1 Type-C with DisplayPort alt mode"; it adds on-board Wi-Fi 6 / BT 5.2 (RTL8852BE), on-board eMMC and a second M.2 M-key [snippet] — [CNX Software](https://www.cnx-software.com/2024/07/27/radxa-rock-5b-plus-sbc-lpddr5-memory-emmc-flash-wifi-6-two-m-2-m-key-sockets-4g-lte-5g/)
- Original ROCK 5B: USB-C DisplayPort alt mode up to 8Kp30 [snippet] — [CNX Software](https://www.cnx-software.com/2022/01/09/rock5-model-b-rk3588-single-board-computer/)
- Orange Pi 5 Plus: USB 3.1 Type-C with DisplayPort 1.4 alt mode [snippet] — [CNX Software](https://www.cnx-software.com/2023/05/10/orange-pi-5-plus-sbc-switches-to-rockchip-rk3588-soc-brings-dual-hdmi-2-1-dual-2-5gbe-m-2-pcie-sockets/)
- ameriDroid (US) search shows no ROCK 5B/5B+, Orange Pi 5 or Khadas Edge2 board; ROCK 5 Model A $154.95 is listed [page, store search JSON] — [ameriDroid](https://ameridroid.com/)
- Switch Science search for "ROCK 5B" returns no Radxa board [page] — [Switch Science](https://www.switch-science.com/)

### Inferences
- Giteki: the original ROCK 5B has no on-board radio (wireless is an M.2 E-key add-in), so the bare board needs no Japanese radio certification; any module added must itself be certified. The ROCK 5B+ does have an on-board radio, and no Japanese certification for it was found, so it should be treated as not certified until shown otherwise.
- RK3588 boards share the 2026 memory-price pressure; the launch prices ($119 for 5B+ 16 GB) no longer apply.

### Gaps
- Amazon.com, Amazon.co.jp, AliExpress, AllNet stock and price for any RK3588 board: not readable.
- Khadas Edge2 DP alt mode and price: not found.
- Radxa approved distributor for Japan: not found.
- Whether DP alt mode works on these boards under a mainline kernel with the glasses' EDID: not researched.

## Concept 5: PSU, cooler, cameras, IMU, cables, adapter

### Takeaway
Every support part is in stock in both regions, with two price corrections against the BOM (BNO085 is $29.50, Camera Module 3 is $29.25). The HDMI-to-USB-C DisplayPort adapter is the only item without a clean source.

### Cited Findings
- 27 W PSU: PiShop.us $12.95 in stock [page] — [PiShop.us](https://www.pishop.us/product/raspberry-pi-27w-usb-c-power-supply-white-us/); Akizuki Y2,750 (black 545, white 292) [page] — [Akizuki](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=Raspberry+Pi+5); Switch Science Y2,860 black available, white sold out [page, store JSON] — [Switch Science](https://www.switch-science.com/products/10259)
- Active Cooler: PiShop.us page returned $12.95, in stock — the same figure as the PSU and far above the $5 BOM price, so verify by eye [page] — [PiShop.us](https://www.pishop.us/product/raspberry-pi-active-cooler/); Switch Science Y1,100 available [page, store JSON] — [Switch Science](https://www.switch-science.com/products/9253); Akizuki Y1,100 (raised from Y990 on 2026-07-24), online restock undetermined [page] — [Akizuki](https://akizukidenshi.com/catalog/g/g129327/)
- Camera Module 3: PiShop.us $29.25 in stock [page] — [PiShop.us](https://www.pishop.us/product/raspberry-pi-camera-module-3/); Akizuki SC1223 Y5,940 (112), NoIR Y5,980 (84) [page] — [Akizuki](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=Raspberry+Pi+5); Switch Science standard V3 marked discontinued, Wide Y7,920 available [page] — [Switch Science](https://www.switch-science.com/products/8702)
- QT Py RP2040: Adafruit $9.95, 94 in stock ($8.96 at 10+) [page] — [Adafruit 4900](https://www.adafruit.com/product/4900); Switch Science Y2,090 available [page, store JSON] — [Switch Science](https://www.switch-science.com/products/7211)
- BNO085: Adafruit 4754 **$29.50**, in stock ($26.55 at 10+), QT cable not included [page] — [Adafruit 4754](https://www.adafruit.com/product/4754); Switch Science Conta BNO085 module Y4,950 available (different board) [page, store JSON] — [Switch Science](https://www.switch-science.com/products/9372)
- HDMI A to micro D cable 2 m, Akizuki: Y350, 814 in stock; Pi 5 camera cables 200 mm Y230 (257) [page] — [Akizuki](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=Raspberry+Pi+5)
- VITURE HDMI to USB-C Adapter Cable: **$29**, backordered, "shipped separately in 1 week"; "compatible with all VITURE glasses" [page] — [VITURE](https://www.viture.com/product/viture-hdmi-to-usb-c-adapter-cable)
- XREAL Hub: $39.99 (US) / Y5,980 (JP), unavailable in both shops; Hub bundles in the JP shop are also unavailable. No HDMI adapter appears in either XREAL shop catalog [page, store JSON] — [XREAL US](https://us.shop.xreal.com/products/xreal-hub), [XREAL JP](https://jp.shop.xreal.com/)
- RayNeo sells "Universal Hub for Legacy Devices: Air 3s + HDMI Adapter" bundle at $329, available (Air 3s alone $269) [page, store search JSON] — [RayNeo](https://www.rayneo.com/products/universal-hub-for-legacy-devices-air-3s-hdmi-adapter)

### Inferences
- The BOM's $69 adapter line can drop to $29 if the VITURE cable works with the chosen glasses; that is unverified for non-VITURE glasses. The XREAL Hub is a charging/passthrough hub and does not convert HDMI, so it is not a substitute.
- The RayNeo bundle prices its HDMI adapter at an effective $60.

### Gaps
- 64 GB microSD and STEMMA QT cable prices (not looked up; commodity).
- Any HDMI-to-USB-C DP adapter source deliverable to Japan.
- Second US source for cooler and camera (Adafruit/CanaKit pages not read).

## Concept 5: display glasses — price, used price, open driver support

### Takeaway
New US prices today: XREAL Air 2 $199, RayNeo Air 3s $269, VITURE Pro 2 $299, Rokid Max 2 $299, XREAL One $399, XREAL 1S $449, XREAL One Pro $599. Monado itself lists only XREAL Air, Air 2 and Air 2 Pro (3DoF); the One family and other brands depend on the community XRLinuxDriver. Used-market prices could not be obtained.

### Cited Findings
- XREAL US shop: Air 2 $199 (grey available, red not), Air 2 Pro $249 unavailable, Air 2 Pro certified refurbished $199 available, One $399, 1S $449, One Pro $599 (M and L), 1 Pro+ $599, Eye $99, Beam Pro $199-$299 [page, store JSON] — [XREAL US shop](https://us.shop.xreal.com/products/xreal-air-2)
- XREAL JP shop: 1S Y67,980, One Pro Y84,980, 1 Pro+ Y84,980, Eye Y13,980, Beam Pro Y29,680-Y43,180; XREAL One and Air 2 are not listed standalone; "One + Hub" and "Beam Pro + Air 2 Pro" bundles are unavailable [page, store JSON] — [XREAL JP shop](https://jp.shop.xreal.com/products/xreal-1s)
- Japanese list prices: One Y62,980, One Pro Y84,980 (tax incl.); XREAL One described as discontinued and replaced by 1S [snippet] — [Techno Edge](https://www.techno-edge.net/article/2026/07/29/5345.html)
- VITURE home page: Pro 2 $299, Luma $399, Luma Pro $499, Luma Ultra $599, Beast $549 [page] — [VITURE](https://www.viture.com/)
- RayNeo Air 3s $269 available; Air 4 Pro $299 [page, store JSON] — [RayNeo](https://www.rayneo.com/products/rayneo-air-3s-xr-glasses)
- Rokid Max 2 $299, available [page, store JSON] — [Rokid](https://global.rokid.com/products/rokid-max-2-ar-glasses)
- Monado supported hardware lists "Xreal Air | 3DoF | Linux", "Xreal Air 2 | 3DoF | Linux", "Xreal Air 2 Pro | 3DoF | Linux"; no Rokid, VITURE or RayNeo [page] — [Monado](https://monado.freedesktop.org/)
- Monado's changelog and driver header add the Air 2 Ultra USB ID [snippet] — [Monado xreal_air header](https://monado.pages.freedesktop.org/monado/xreal__air__interface_8h_source.html)
- XRLinuxDriver table: VITURE One/One Lite/Pro XR/Luma/Luma Pro/Luma Ultra/Pro 2 supported, closed-source official SDK; RayNeo Air 2/2s/3s/Pro supported, closed-source SDK (GT moving to open source); Rokid Max and Air supported, closed-source SDK, **Max 2 not listed**; XREAL Air/Air 2/Air 2 Pro/Air 2 Ultra supported via unofficial open-source SDK, "exhibits drift"; XREAL One/One Pro/1S supported "see notes before use" (disable stabilizer/anchor, latest firmware) [page] — [XRLinuxDriver](https://www.github.com/wheaney/XRLinuxDriver)
- Used and refurbished, Japan, official XREAL reuse sales (all ended): XREAL One Y40,980 and Air 2 Y15,980 (March 2026); One Y35,980 (April 2026); One Pro Y54,980, 300 units (July-August 2026) [snippet] — [Techno Edge April](https://www.techno-edge.net/article/2026/04/24/5022.html), [Mogura](https://www.moguravr.com/xreal-certified-reuse-sale-202607/)
- Used, US: Swappa average used XREAL One Pro $551 [snippet] — [Swappa](https://swappa.com/sell/type/gaming-vr)

### Inferences
- For an open-hardware Monado/OpenXR build, the XREAL Air 2 is the best fit: cheapest, in stock in the US, in Monado's own list, and its IMU is read over USB HID by an open driver. Its weakness is drift.
- VITURE, RayNeo and Rokid IMU access on Linux goes through vendor closed-source SDKs, which conflicts with the open-hardware goal even though the glasses work.
- The Swappa One Pro figure ($551 used against $599 new) suggests little used discount in the US; the Japanese official refurbished prices (roughly 55-65% of new) are a better guide to second-hand value there.

### Gaps
- eBay sold prices, Mercari and Yahoo Auctions prices for every model: not obtained.
- Japan prices for VITURE, RayNeo, Rokid; Amazon.co.jp availability of XREAL Air 2.
- Whether the XREAL One family exposes raw IMU over USB without the X1 chip's on-glasses processing: only the XRLinuxDriver note was read.

## Concept 5: cost of one complete set

### Takeaway
Excluding microSD, QT cable and (in Japan) the HDMI adapter, which were not priced: about **$527** in the US with a new XREAL Air 2, and about **Y126,300** in Japan with a new XREAL 1S. A used-glasses total could only be estimated for Japan from past refurbished prices.

### Cited Findings
- US core parts, all [page] rows above: Pi 5 8 GB $175.00 + PSU $12.95 + cooler $12.95 (verify) + VITURE HDMI cable $29.00 + QT Py $9.95 + BNO085 $29.50 + 2 cameras $58.50 = **$327.85** — sources as cited in the sections above, e.g. [PiShop.us](https://www.pishop.us/product/raspberry-pi-5-8gb/), [Adafruit](https://www.adafruit.com/product/4754)
- US with glasses new: + XREAL Air 2 $199 = **$526.85**; + XREAL One $399 = **$726.85**; + XREAL One Pro $599 = **$926.85** — [XREAL US shop](https://us.shop.xreal.com/products/xreal-one)
- US with glasses refurbished: + XREAL Air 2 Pro certified refurbished $199 = **$526.85** — [XREAL US shop](https://us.shop.xreal.com/products/xreal-air-2-pro-refurbished)
- Japan core parts, all [page] rows above: Pi 5 8 GB Y35,200 + PSU Y2,750 + cooler Y1,100 + HDMI cable Y350 + QT Py Y2,090 + Conta BNO085 Y4,950 + 2 cameras Y11,880 = **Y58,320** (no HDMI-to-USB-C adapter included) — [Akizuki](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=Raspberry+Pi+5), [Switch Science](https://www.switch-science.com/products/9372)
- Japan with glasses new: + XREAL 1S Y67,980 = **Y126,300**; + XREAL One Pro Y84,980 = **Y143,300** — [XREAL JP shop](https://jp.shop.xreal.com/products/xreal-one-pro)

### Inferences
- Japan with refurbished glasses, if a reuse sale recurs: Y58,320 + Y35,980 to Y40,980 (XREAL One) = roughly Y94,300 to Y99,300; with an Air 2 at the March price of Y15,980, roughly Y74,300. These are past prices, not available today.
- Five sets cost five times the single-set figure; no quantity break below 10 was seen on any item.

### Gaps
- True used-market totals for either region.
- Shipping, US sales tax, and the three unpriced small items.
