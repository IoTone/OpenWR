# Pod-0 parts: Japan purchase sources, stock, lead time, small-quantity price (checked 2026-11-10)

How to read this file:

- Every number was checked on 2026-11-10. "READ" means I read it on the seller's own page (or, for LCSC, the seller's own product-detail API). "SNIPPET" means it came from a search-result extract. "UNCONFIRMED" means I could not read it.
- DigiKey Japan prices are yen **excluding** 10% consumption tax (the page shows both; e.g. BMI270 ¥697.00 ex-tax, ¥766.70 incl.). Akizuki and Sengoku prices are yen **including** tax. LCSC prices are USD without tax or shipping.
- DigiKey pages show breaks at 1 and 10 only for most parts; the qty-5 price is the qty-1 price (inferred from the price table, not printed).
- Akizuki sells many chips in packs; pack size is given.
- What I could not read: mouser.jp (timeouts / "Access denied"), octopart.com and trustedparts.com (HTTP 403), chip1stop.com (403), marutsu.co.jp (bot wall), buydisplay.com (403), LCSC search pages (no data), AliExpress/Taobao/Amazon.co.jp (not readable), tele.soumu.go.jp search (error page), METI lithium FAQ page (403 / empty body), Switch Science search (HTTP 429 on retries; one results page was read). CoreStaff, RS Japan, Monotaro, Kyohritsu and Seeed were not checked.
- The machine-readable companion is `pod0_parts_japan.json` (71 rows) in the same folder.

## Which parts can a Japan buyer get from stock today, where, for how much at 1 / 5 / 10 pieces?

### Takeaway
Almost everything is in stock at DigiKey Japan and/or LCSC, and four items are on the shelf at Akizuki (WROOM-02 module, NTC, JST PH header, USB-C receptacles) plus a 500 mAh LiPo at Sengoku. Akizuki does not carry any of the BOM's ICs (BMI270, VEML7700, AP2112K, USBLC6, BQ24074, MAX17048 all returned zero hits), so a pure "Japanese shop" build is not possible; the practical basket is DigiKey Japan + Akizuki + Sengoku, or LCSC + Akizuki + Sengoku.

### Cited Findings

Prices are per piece at 1 / 5 / 10. All rows READ unless marked.

**U1 ESP32-C3 module**
- ESP32-C3-MINI-1-N4, DigiKey Japan: stock 1,231; ¥537 / ¥537 / ¥464.70 ex-tax; page flags it as not recommended for new designs — [DigiKey](https://www.digikey.jp/ja/products/detail/espressif-systems/ESP32-C3-MINI-1-N4/13877574)
- ESP32-C3-MINI-1-N4X (active successor), DigiKey Japan: stock 1,518; ¥537 / ¥537 / ¥464.70 ex-tax — [DigiKey](https://www.digikey.jp/ja/products/detail/espressif-systems/ESP32-C3-MINI-1-N4X/27525554)
- ESP32-C3-MINI-1-N4, LCSC C2838502: stock 0; listed $3.0672 @1, $2.7002 @10; LCSC's page text says NRND and points to -H4X — [LCSC](https://www.lcsc.com/product-detail/rf-modules_espressif-systems-esp32-c3-mini-1-n4_C2838502.html)
- ESP32-C3-MINI-1-H4, LCSC C2934569: stock 5,289; $3.0971 @1, $2.7651 @10 — [LCSC](https://www.lcsc.com/product-detail/C2934569.html)
- ESP32-C3-MINI-1-H4X, DigiKey Japan: stock 0, special-order, only a 3,250-piece price shown, 4-week maker lead time — [DigiKey search](https://www.digikey.jp/ja/products/result?keywords=ESP32-C3-MINI-1-H4X)
- ESP32-C3-MINI-1U-N4, DigiKey Japan: stock 1,441; ¥557 / ¥557 / ¥481.50; page says DigiKey no longer carries it and accepts no backorders — [DigiKey](https://www.digikey.jp/ja/products/detail/espressif-systems/ESP32-C3-MINI-1U-N4/15222550)
- ESP32-C3-WROOM-02-N4, Akizuki 販売コード 117493: stock 1,735; ¥440 incl. tax, sold singly — [Akizuki](https://akizukidenshi.com/catalog/g/g117493/)
- ESP32-C3-WROOM-02-N4, DigiKey Japan: stock 0, 650 expected 2026-11-16; ¥557 / ¥557 / ¥481.50 — [DigiKey](https://www.digikey.jp/ja/products/detail/espressif-systems/ESP32-C3-WROOM-02-N4/14553031)
- ESP32-C3-WROOM-02-N4, LCSC C2934560: stock 10,694; $2.9037 @1, $2.5089 @10 — [LCSC](https://www.lcsc.com/product-detail/C2934560.html)
- Akizuki has no ESP32-C3-MINI-1 (search "ESP32-C3" returns only XIAO ESP32C3 at ¥1,080, stock 1,460, and the WROOM-02-N4) — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=ESP32-C3)

**U1 Japan radio certification (技適)**
- Espressif's certificate "Certificate of Radio Equipment in JAPAN No: 201-210888 / 00", type designation ESP32-C3-MINI-1, issued by Telefication (CAB ID 201), Apeldoorn, 01 December 2021; required marking "R 201-210888"; PCB antenna, max gain 3.96 dBi at 2.4 GHz. READ from the PDF text — [Espressif TELEC PDF](https://www.espressif.com/sites/default/files/certificates/ESP32-C3-MINI-1%20TELEC%20Certification.pdf)
- Espressif's certificates page lists "ESP32-C3-MINI-1 TELEC Certification" (page date 2025.12.18) — [Espressif certificates](https://www.espressif.com/en/support/documents/certificates?keys=ESP32-C3-MINI-1)
- ESP32-C3-MINI-1U has its own certificate: No. 201-230074 / 00, marking "R 201-230074", type designation ESP32-C3-MINI-1U (issuer shown as Kiwa Nederland B.V.). READ from the PDF text — [Espressif TELEC PDF, MINI-1U](https://www.espressif.com/sites/default/files/certificates/ESP32-C3-MINI-1U%20TELEC%20Certification.pdf)

**U2 BMI270**
- DigiKey Japan: stock 32,243; ¥697 / ¥697 / ¥597.40 ex-tax (¥766.70 incl. at qty 1) — [DigiKey](https://www.digikey.jp/ja/products/detail/bosch-sensortec/BMI270/9974486)
- LCSC C2836813: stock 9,254; $2.58 @1, $2.3174 @10 — [LCSC](https://www.lcsc.com/product-detail/C2836813.html)
- Akizuki: zero hits for BMI270; its only bare 6-axis IMU is LSM6DSV16XTR (130032, ¥770, 31 left, "low stock") — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=6%E8%BB%B8)

**U3 VEML7700-TT**
- DigiKey Japan (751-VEML7700-TTCT-ND): stock 14,393; ¥237 / ¥209.60 / ¥199.30 ex-tax (this part does have a qty-5 break) — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=VEML7700-TT)
- LCSC C504893 is VEML7700-**TR**, not -TT: stock 1,988; $1.1435 @1, $0.9483 @10 — [LCSC](https://www.lcsc.com/product-detail/C504893.html)

**U4 LDO**
- AP2112K-3.3TRG1, DigiKey Japan: stock 406,515; ¥46 / ¥46 / ¥31.50 — [DigiKey](https://www.digikey.jp/ja/products/detail/diodes-incorporated/AP2112K-3-3TRG1/4470746)
- AP2112K-3.3TRG1, LCSC C51118: stock 8,805; MOQ 5 at $0.1683 each — [LCSC](https://www.lcsc.com/product-detail/C51118.html)
- TLV75733PDBVR, DigiKey Japan: stock 0, 3,000 expected 2026-12-14; ¥56 @1, ¥38.90 @10 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=TLV75733PDBVR)
- TLV75733PDBVR, LCSC C485517: stock 24,055; MOQ 5 at $0.2098 — [LCSC](https://www.lcsc.com/product-detail/C485517.html)
- Akizuki has no AP2112K; nearest shelf part is LM8805SF5-3.3 (3.3 V 600 mA, 5 for ¥200, 1,364 packs) — [Akizuki](https://akizukidenshi.com/catalog/g/g117321/)

**U5 USBLC6-2SC6**
- DigiKey Japan: stock 27,230; ¥89 / ¥89 / ¥54.20 — [DigiKey](https://www.digikey.jp/ja/products/detail/stmicroelectronics/USBLC6-2SC6/1040559)
- LCSC C7519: stock 28,055; MOQ 5 at $0.1639 — [LCSC](https://www.lcsc.com/product-detail/C7519.html)

**U10 BQ24074RGTR**
- DigiKey Japan: stock 1,901; ¥401 / ¥401 / ¥297.90 — [DigiKey](https://www.digikey.jp/ja/products/detail/texas-instruments/BQ24074RGTR/2047269)
- LCSC C54313: stock 4,337; $2.1391 @1, $1.8583 @10 — [LCSC](https://www.lcsc.com/product-detail/C54313.html)

**U11 MAX17048G+T10**
- DigiKey Japan: stock 21,841; ¥760 / ¥760 / ¥576.80 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=MAX17048G%2BT10)
- LCSC C2682616: stock 20,977; $1.9804 @1, $1.7244 @10 — [LCSC](https://www.lcsc.com/product-detail/C2682616.html)

**Q1 FDN338P / Q2 FDN335N / D10, D11 PMEG2010AEH** — see the "not obtainable" section; DigiKey Japan has zero stock of all three.
- FDN338P (onsemi), LCSC C24855: stock 6,980; MOQ 5 at $0.2454 — [LCSC](https://www.lcsc.com/product-detail/C24855.html)

**LEDs**
- LTST-C191KGKT, DigiKey Japan: stock 2,228,010; ¥25 / ¥25 / ¥17.10 — [DigiKey](https://www.digikey.jp/ja/products/detail/liteon/LTST-C191KGKT/386835)
- LTST-C191KRKT, DigiKey Japan: stock 1,334,425; ¥25 / ¥25 / ¥17.10 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=LTST-C191KRKT)
- LCSC: LTST-C191KGKT C125098 stock 137,450, MOQ 50 at $0.0208; LTST-C191KRKT C125099 stock 390,280, MOQ 20 at $0.0194 — [LCSC KGKT](https://www.lcsc.com/product-detail/C125098.html), [LCSC KRKT](https://www.lcsc.com/product-detail/C125099.html)
- Akizuki shelf 1608 LEDs (not the Lite-On MPN): green OSG50603C1E 10 for ¥100 (1,723 packs), red OSR50603C1E 10 for ¥100 (673 packs) — [Akizuki green](https://akizukidenshi.com/catalog/g/g108275/), [Akizuki red](https://akizukidenshi.com/catalog/g/g108274/)

**RT1 NCP15XH103F03RC**
- Akizuki 130324, exact MPN: 20 for ¥130 incl. tax, 136 packs — [Akizuki](https://akizukidenshi.com/catalog/g/g130324/)
- DigiKey Japan: stock 122,042; ¥20 / ¥20 / ¥15.30 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=NCP15XH103F03RC)
- LCSC C77131: stock 299,800; MOQ 50 at $0.0166 — [LCSC](https://www.lcsc.com/product-detail/C77131.html)

**J1 FH12-30S-0.5SH(55)**
- DigiKey Japan (HFJ130CT-ND): stock 4,414; ¥435 / ¥435 / ¥369.90 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=FH12-30S-0.5SH%2855%29)
- LCSC C506793: stock 1,951; $0.6985 @1, $0.5785 @10 — [LCSC](https://www.lcsc.com/product-detail/C506793.html)
- Akizuki: zero hits for FH12 — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=FH12)

**J2 USB-C 24-pin**
- Amphenol 12401610E4#2A, DigiKey Japan: stock 79,753; ¥236 / ¥236 / ¥200.60 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=12401610E4%232A)
- Akizuki CX70M-24P1 (113235), 24-pin mid-mount, depth 8.35 mm: ¥160 incl. tax, stock 848 — [Akizuki](https://akizukidenshi.com/catalog/g/g113235/)

**J10 USB-C 16-pin**
- GCT USB4105-GF-A, DigiKey Japan: stock 263,569; ¥132 / ¥132 / ¥111.40 — [DigiKey](https://www.digikey.jp/ja/products/detail/gct/USB4105-GF-A/11198441)
- USB4105-GF-A, LCSC C3020560: stock 15,717; $1.0307 @1, $0.8358 @10 — [LCSC](https://www.lcsc.com/product-detail/C3020560.html)
- Korean Hroparts TYPE-C-31-M-12, LCSC C165948: stock 271,035; MOQ 5 at $0.1887 — [LCSC](https://www.lcsc.com/product-detail/C165948.html)
- Akizuki 5077CR-16-SMC2-BK-TR (114356): ¥110 incl. tax, 209 left, marked low stock — [Akizuki](https://akizukidenshi.com/catalog/g/g114356/)

**J3, J12 SM10B-SRSS-TB(LF)(SN)**
- DigiKey Japan: stock 27,096; ¥153 / ¥153 / ¥129.90 (¥168.30 incl. tax @1) — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=SM10B-SRSS-TB)
- LCSC C160409: stock 75,862; $0.4353 @1, $0.3499 @10 — [LCSC](https://www.lcsc.com/product-detail/C160409.html)

**CBL JST SH 10-pin cable, both ends**
- JST A10SR10SR30K152A (10-position, 1.0 mm, socket-to-socket, "reversed", 152 mm), DigiKey Japan 455-3727-ND: stock 5,019; ¥405 / ¥405 / ¥344.50 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=A10SR10SR30K152A)
- Same family on that page: 51 mm stock 531 ¥313; 102 mm stock 117 ¥384; 203 mm stock 0; 254 mm stock 613 ¥523; 305 mm stock 465 ¥588 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=A10SR10SR30K152A)
- Akizuki has only 4P/6P/7P corded connectors, no 10-pin SH cable — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=SH%E3%82%B3%E3%83%8D%E3%82%AF%E3%82%BF)

**J11 S2B-PH-SM4-TB(LF)(SN)**
- Akizuki 112810, exact MPN: ¥40 incl. tax, stock 1,000 — [Akizuki](https://akizukidenshi.com/catalog/g/g112810/)
- DigiKey Japan: stock 133,904; ¥81 / ¥81 / ¥68.70 — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=S2B-PH-SM4-TB)
- LCSC C295747: stock 2,775; MOQ 5 at $0.2284 — [LCSC](https://www.lcsc.com/product-detail/C295747.html)

**SW10 PCM12SMTR**
- DigiKey Japan: stock 2,726; ¥211 / ¥211 / ¥179.20; page marks the part discontinued (no longer manufactured, stock not replenished) — [DigiKey](https://www.digikey.jp/ja/products/detail/c-k/PCM12SMTR/1640112)
- LCSC C221841: stock 4,263; $1.2754 @1, $1.0617 @10 — [LCSC](https://www.lcsc.com/product-detail/C221841.html)
- Akizuki Alps Alpine SSAJ120100 (113989), SMT slide SPDT side-actuated: ¥80 incl. tax, stock 2,230 — [Akizuki](https://akizukidenshi.com/catalog/g/g113989/)

**BT1 LiPo 500 mAh**
- Sengoku Densho, Data Power Technology DTP652533(PHR): 3.7 V 500 mAh, 35 × 25.5 × 6.7 mm, built-in protection circuit, PHR-2 connector with pin 1 red (+), ¥1,580 incl. tax, minimum 1; the page lists Akihabara and Osaka stores but no stock quantity; PSE is not mentioned on the page — [Sengoku](https://www.sengoku.co.jp/mod/sgk_cart/detail.php?code=EEHD-67JP)
- Sengoku's list also has DTP502535(PHR) 400 mAh ¥1,210, DTP443442(PHR) 640 mAh ¥1,650, DTP902040(PHR) 700 mAh ¥1,600; two other DTP items are labelled "PSE対象外" (outside PSE scope) — [Sengoku search](https://www.sengoku.co.jp/mod/sgk_cart/search.php?multi=%E3%83%AA%E3%83%81%E3%82%A6%E3%83%A0%E3%82%A4%E3%82%AA%E3%83%B3%E3%83%9D%E3%83%AA%E3%83%9E%E3%83%BC)
- Switch Science: its bare cells (110, 400, 850, 1800 mAh) all carry "販売終了" (discontinued) in the title — [Switch Science 400 mAh](https://www.switch-science.com/products/3118)
- Akizuki: no LiPo cells (searches for リチウムイオンポリマー / リチウムポリマー / LiPo return only charger ICs and boards) — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=%E3%83%AA%E3%83%81%E3%82%A6%E3%83%A0%E3%83%9D%E3%83%AA%E3%83%9E%E3%83%BC)
- digikey.jp: no result for LP503035 — [DigiKey search](https://www.digikey.jp/ja/products/result?keywords=LP503035)

**Passives**
- Odd-value 1% 0402 resistor example, Yageo RC0402FR-07620KL (620 kΩ): DigiKey Japan stock 46,090; ¥18 @1, ¥4.00 @10, ¥1.88 @100 ex-tax — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=RC0402FR-07620KL)
- Akizuki 1005 (0402) Murata MLCC packs, all in stock: 0.1 µF 50 V X7R 25 for ¥100 (113377); 1 µF 25 V X5R 40 for ¥100 (116074); 4.7 µF 10 V 20 for ¥100 (113700); 10 µF 6.3 V 20 for ¥100 (131755) — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=%E3%83%81%E3%83%83%E3%83%97%E7%A9%8D%E5%B1%A4%E3%82%BB%E3%83%A9%E3%83%9F%E3%83%83%E3%82%AF%E3%82%B3%E3%83%B3%E3%83%87%E3%83%B3%E3%82%B5%E3%83%BC+1005)
- Akizuki returns zero hits for "チップ抵抗 1005" and for "620kΩ" — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=620k%CE%A9)

### Inferences
- Cheapest all-in-stock basket by line count: DigiKey Japan covers every IC, connector, LED, NTC, cable and resistor in one order (well over the ¥6,000 free-shipping line for 5 units), except Q1, Q2, D10/D11, the OLED panel and the battery.
- LCSC is far cheaper per piece on connectors (FH12-30S $0.70 vs ¥435; SM10B $0.44 vs ¥153; MAX17048 $1.98 vs ¥760) but adds international shipping, so it only wins if the whole basket goes there.
- The MINI-1 footprint can be populated today from DigiKey Japan with either -N4 (NRND) or -N4X (active). For new work the N4X/H4X line is the one with a future.
- For Akizuki pack items, the per-piece cost in the JSON is pack price divided by pieces needed; price_1 is the cost of one pack.

### Gaps
- No stock or price could be read from mouser.jp, Chip One Stop, Marutsu, CoreStaff, RS Japan, Monotaro, Amazon.co.jp, AliExpress or Taobao. Mouser Japan very likely stocks most of these parts but that is unconfirmed here.
- LCSC part numbers for FDN335N, PMEG2010AEH, VEML7700-TT and ESP32-C3-MINI-1U were not found.
- Japan MIC database (tele.soumu.go.jp) lookup of 201-210888 failed (error page); the certificate number comes from Espressif's own PDF only.
- Whether the ESP32-C3-MINI-1-N4X / -H4X revisions are covered by certificate 201-210888 (type designation "ESP32-C3-MINI-1") was not confirmed on a document.
- ESP32-C3-WROOM-02 TELEC number not found (the guessed certificate URL did not resolve).
- 1.78 kΩ and 3.09 kΩ were not individually looked up; only 620 kΩ was.
- Sengoku does not show a stock count for the battery.

## digikey.jp and mouser.jp: shipping fee, free-shipping threshold, delivery days, tax/duty at checkout

### Takeaway
DigiKey Japan: free shipping at ¥6,000 of goods or more, ¥2,000 below that, delivery usually 3–4 days, prices shown ex-tax with tax-inclusive price alongside. Mouser Japan: only search snippets were readable — free shipping on most orders over ¥6,000, DDP with consumption tax added at order time.

### Cited Findings
- DigiKey: "orders shipped to Japan with merchandise of ¥6,000 or more ship free; under ¥6,000 a ¥2,000 shipping fee applies" and "usually delivered within 3–4 days depending on location". READ on a product page — [DigiKey BMI270 page](https://www.digikey.jp/ja/products/detail/bosch-sensortec/BMI270/9974486)
- DigiKey product pages show the unit price ex-tax and incl. tax side by side (BMI270: 単価（消費税抜き）¥697.00, 単価（消費税込み）¥766.70). READ — [DigiKey BMI270 page](https://www.digikey.jp/ja/products/detail/bosch-sensortec/BMI270/9974486)
- DigiKey terms: DDP — duties and fees borne by DigiKey, qualified-invoice compatible; marketplace items ship CPT — duty and import consumption tax paid by the customer on delivery. SNIPPET — [DigiKey Japan](https://www.digikey.jp/ja/maker/profiles/39c3850bc1eb4b75b5df02a57d70c517)
- DigiKey Digi-Reel adds a ¥800 reel fee per line (cut tape has none). READ — [DigiKey FH12 page](https://www.digikey.jp/ja/products/result?keywords=FH12-30S-0.5SH%2855%29)
- Mouser Japan: "free shipping on most orders over ¥6,000 (JPY)"; Incoterms DDP with consumption tax added at order; "All prices include duty and customs fees"; USD-priced orders instead pay duty, customs fees and consumption tax on delivery. SNIPPET — [Mouser Japan](https://www.mouser.jp/allianceelectronics/)

### Inferences
- With either distributor a 1–5 unit Pod-0 basket clears ¥6,000, so shipping is free and no customs payment is collected at the door (DDP), provided no DigiKey "marketplace" lines are in the cart.
- Some clone parts on digikey.jp (Slkor, UMW FDN338P) are marketplace-type listings; those may ship CPT and separately.

### Gaps
- Mouser's shipping fee below ¥6,000 and its delivery days to Japan: unconfirmed (mouser.jp timed out or denied access on every attempt).
- DigiKey's dedicated shipping help page URL returned 404; the figures above come from text embedded in product pages.
- Whether the ¥6,000 threshold is judged ex-tax or incl. tax is not stated in what I read.

## LCSC to Japan: shipping options, fee and days; which MPNs LCSC stocks

### Takeaway
LCSC stocks nearly the whole BOM with C-numbers confirmed below; its Japan shipping fee and transit time could not be read (unconfirmed). Not in stock at LCSC: ESP32-C3-MINI-1-N4 and PTS810.

### Cited Findings
LCSC stock and price READ from LCSC's product-detail data on 2026-11-10 (USD):

| Ref | MPN | LCSC # | Stock | MOQ | Price |
|---|---|---|---|---|---|
| U1 | ESP32-C3-MINI-1-N4 | C2838502 | 0 | 1 | 3.0672 @1, 2.7002 @10 |
| U1 alt | ESP32-C3-MINI-1-H4 | C2934569 | 5,289 | 1 | 3.0971 @1, 2.7651 @10 |
| U1 alt | ESP32-C3-WROOM-02-N4 | C2934560 | 10,694 | 1 | 2.9037 @1, 2.5089 @10 |
| U2 | BMI270 | C2836813 | 9,254 | 1 | 2.58 @1, 2.3174 @10 |
| U3 | VEML7700-TR (not -TT) | C504893 | 1,988 | 1 | 1.1435 @1, 0.9483 @10 |
| U4 | AP2112K-3.3TRG1 | C51118 | 8,805 | 5 | 0.1683 @5 |
| U4 alt | TLV75733PDBVR | C485517 | 24,055 | 5 | 0.2098 @5 |
| U5 | USBLC6-2SC6 | C7519 | 28,055 | 5 | 0.1639 @5 |
| U10 | BQ24074RGTR | C54313 | 4,337 | 1 | 2.1391 @1, 1.8583 @10 |
| U11 | MAX17048G+T10 | C2682616 | 20,977 | 1 | 1.9804 @1, 1.7244 @10 |
| Q1 | FDN338P (onsemi) | C24855 | 6,980 | 5 | 0.2454 @5 |
| Q1 clone | FDN338P (UMW) | C347511 | 9,700 | 20 | 0.0375 @20 |
| D | LTST-C191KGKT | C125098 | 137,450 | 50 | 0.0208 @50 |
| D | LTST-C191KRKT | C125099 | 390,280 | 20 | 0.0194 @20 |
| RT1 | NCP15XH103F03RC | C77131 | 299,800 | 50 | 0.0166 @50 |
| J1 | FH12-30S-0.5SH(55) | C506793 | 1,951 | 1 | 0.6985 @1, 0.5785 @10 |
| J10 | USB4105-GF-A | C3020560 | 15,717 | 1 | 1.0307 @1, 0.8358 @10 |
| J10 alt | TYPE-C-31-M-12 | C165948 | 271,035 | 5 | 0.1887 @5 |
| J3, J12 | SM10B-SRSS-TB(LF)(SN) | C160409 | 75,862 | 1 | 0.4353 @1, 0.3499 @10 |
| J11 | S2B-PH-SM4-TB(LF)(SN) | C295747 | 2,775 | 5 | 0.2284 @5 |
| SW1, SW2 | PTS810SJM250SMTRLFS | C116501 | 0 | 1 | 0.6595 @1, 0.5339 @10 |
| SW10 | PCM12SMTR | C221841 | 4,263 | 1 | 1.2754 @1, 1.0617 @10 |

- Source for every row: LCSC product pages, e.g. — [C2836813](https://www.lcsc.com/product-detail/C2836813.html), [C506793](https://www.lcsc.com/product-detail/C506793.html), [C160409](https://www.lcsc.com/product-detail/C160409.html), [C24855](https://www.lcsc.com/product-detail/C24855.html), [C116501](https://www.lcsc.com/product-detail/C116501.html) (pattern `https://www.lcsc.com/product-detail/<C-number>.html`)
- LCSC lists its carriers as DHL, UPS, FedEx, SF Express, Belgium Post, Global Direct Standard Line, Registered Mail and 4PX, "subject to the country and regions"; the page gives no Japan fee or transit time. READ — [LCSC shipping FAQ](https://www.lcsc.com/faqs/shipping-delivering/shipping-methods)

### Inferences
- A single LCSC order can cover U2, U4, U5, U10, U11, Q1, LEDs, RT1, J1, J10, J3/J12, J11 and SW10 at a parts cost of roughly a quarter to a half of DigiKey Japan's, before shipping.
- LCSC's MOQ-5/20/50 lines are harmless for a 1–5 unit build (cents per line).

### Gaps
- LCSC shipping fee and days to Japan: UNCONFIRMED. LCSC's rate is only shown in the cart; no Japan-specific page was found. Whether OCS / Sagawa / Yamato options are offered to Japan was not confirmed.
- LCSC C-numbers not found for FDN335N, PMEG2010AEH, VEML7700-TT, ESP32-C3-MINI-1U, a 24-pin USB-C receptacle, a JST SH 10-pin cable, a bare OLED panel.
- No JST SH 10-pin cable or OLED panel was confirmed at LCSC, AliExpress or Taobao.

## Which parts are NOT obtainable in Japan within 2 weeks, and what substitutes are?

### Takeaway
Six items need attention: Q1 FDN338P, Q2 FDN335N and D10/D11 PMEG2010AEH are at zero stock at DigiKey Japan with long or unknown lead times; SW1/SW2 PTS810 is at zero or unknown stock at both DigiKey and LCSC; the OLED panel DS1 is at zero stock at DigiKey (8 weeks) and has an unresolved connector-pitch question; SW10 PCM12SMTR is discontinued (still in stock). Akizuki shelf parts cover the MOSFETs, Schottky and switches the same day.

### Cited Findings
- FDN338P (onsemi), DigiKey Japan: stock 0, restock undetermined, maker lead time 57 weeks; ¥120 @1, ¥74.50 @10 — [DigiKey](https://www.digikey.jp/ja/products/detail/onsemi/FDN338P/458848)
  - In stock instead: LCSC C24855 genuine onsemi, 6,980 pcs, $0.2454 @5 — [LCSC](https://www.lcsc.com/product-detail/C24855.html)
  - Akizuki shelf substitute: IRLML6402TRPBF P-ch 20 V 3.7 A SOT-23, 10 for ¥330 incl. tax, 884 packs — [Akizuki](https://akizukidenshi.com/catalog/g/g102553/)
- FDN335N (onsemi), DigiKey Japan: stock 0, restock date undetermined, backorder only; ¥125 @1, ¥77.10 @10 — [DigiKey](https://www.digikey.jp/ja/products/detail/onsemi/FDN335N/965301)
  - Akizuki shelf substitutes: Toshiba SSM3K345R N-ch 20 V 4 A, 5 for ¥100, 2,786 packs — [Akizuki](https://akizukidenshi.com/catalog/g/g117477/); MTN2306AN3 N-ch 30 V 5 A, 10 for ¥120, 1,830 packs — [Akizuki](https://akizukidenshi.com/catalog/g/g117513/)
- PMEG2010AEH,115 (Nexperia), DigiKey Japan: stock 0, 3,000 expected 2027-10-04; ¥61 @1, ¥37.90 @10 — [DigiKey](https://www.digikey.jp/ja/products/detail/nexperia-usa-inc/PMEG2010AEH-115/1157670)
  - Akizuki shelf substitutes: Rohm RB160M-30TR 30 V 1 A, 10 for ¥150, 376 packs — [Akizuki](https://akizukidenshi.com/catalog/g/g101398/); SS14 40 V 1 A, 20 for ¥150, 1,960 packs — [Akizuki search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=%E3%82%B7%E3%83%A7%E3%83%83%E3%83%88%E3%82%AD%E3%83%BC+1A+%E8%A1%A8%E9%9D%A2%E5%AE%9F%E8%A3%85)
- PTS810 SJM 250 SMTR LFS: the C&K DigiKey page shows prices (¥96 @1, ¥82.60 @10) but no stock figure and a 16-week maker lead time — [DigiKey C&K](https://www.digikey.jp/ja/products/detail/c-k/PTS810SJM250SMTR-LFS/4176610); the second DigiKey listing shows stock 0, ship date undetermined, no backorders — [DigiKey WEC](https://www.digikey.jp/ja/products/detail/wec/PTS810SJM250SMTRLFS/27533075); LCSC C116501 stock 0 — [LCSC](https://www.lcsc.com/product-detail/C116501.html)
  - Akizuki shelf substitute: Alps Alpine SKRPACE010 (2.55 N), 5 for ¥100, 659 packs; SKRPABE010 (1.57 N), 5 for ¥120, 98 packs — [Akizuki](https://akizukidenshi.com/catalog/g/g106185/)
- PCM12SMTR: DigiKey marks it discontinued, 2,726 left — [DigiKey](https://www.digikey.jp/ja/products/detail/c-k/PCM12SMTR/1640112); Akizuki Alps Alpine SSAJ120100 ¥80, 2,230 in stock — [Akizuki](https://akizukidenshi.com/catalog/g/g113989/)
- OLED AOM12864A0-0.96WW (Orient Display), DigiKey Japan: stock 0, maker lead time 8 weeks; ¥1,907 @1, ¥1,472.10 @10; controller SSD1306, I2C/parallel/SPI; connection type and FPC pitch not listed — [DigiKey](https://www.digikey.jp/ja/products/detail/orient-display/AOM12864A0-0-96WW/22531883)
- aitendo UG-2864 (Univision UG-2864HLBEG01 blue / KSWEG01 white / KMBEG01 two-colour): bare 0.96" 128×64 SSD1306 panel, outline 26.7 × 19.26 × 1.45 mm, I2C/SPI/8-bit parallel, connection "0.7 mm pitch 30P surface mount" — [aitendo](https://www.aitendo.com/product/6279)
- Akizuki's 0.96" OLED (112031, ¥580, 333 left, "low stock, next arrival mid-November") is an I2C module on a PCB (27 × 24.7 mm, SSD1315), not a bare panel — [Akizuki](https://akizukidenshi.com/catalog/g/g112031/)
- TLV75733PDBVR (U4 alternate) is at 0 at DigiKey Japan until 2026-12-14; the primary AP2112K is in deep stock, so no action — [DigiKey](https://www.digikey.jp/ja/products/result?keywords=TLV75733PDBVR)
- ESP32-C3-MINI-1-N4 is NRND and at 0 at LCSC; DigiKey Japan still has 1,231 plus 1,518 of the N4X — [DigiKey N4X](https://www.digikey.jp/ja/products/detail/espressif-systems/ESP32-C3-MINI-1-N4X/27525554)

### Inferences
- DS1 pinout / connector risk (important): the aitendo page shows that the classic Univision-type 0.96" 30-pin SSD1306 bare panel has a **0.7 mm pitch solder-down** tail, while Pod-0's J1 is a **0.5 mm pitch ZIF** (FH12-30S-0.5SH). From general knowledge, most "0.96 OLED 30pin SSD1306" bare panels sold on AliExpress/Taobao are that 0.7 mm solder type; 0.5 mm plug-in variants exist but must be picked deliberately. I could not confirm on any seller page which type AOM12864A0-0.96WW, Winstar WEO012864 or buydisplay ER-OLED0.96-1 use, so the "30-pin pinout matches the standard panel" question is NOT confirmed. Check the Orient Display datasheet for FPC pitch before ordering either the panel or J1.
- The Akizuki substitutes for Q1/Q2/D10/D11 are electrically in the same class (20–30 V, 1 A+ SOT-23 / SOD-123-size), but SSM3K345R is SOT-23F and RB160M is Rohm PMDU; pad fit, gate threshold and forward voltage should be checked against the schematic (the BQ24074 power-path circuit is sensitive to Schottky drop).
- SKRPACE010 is the same 4.2 × 3.2 × 2.5 mm size class as PTS810; land pattern equality is likely but not verified here.
- Buying genuine FDN338P from LCSC keeps Q1 unchanged; only Q2 and the diodes then need a decision.

### Gaps
- No in-stock source confirmed for a bare 0.5 mm-pitch 30-pin 0.96" SSD1306 panel deliverable to Japan (buydisplay returned 403; AliExpress/Taobao not readable; aitendo price and stock not readable).
- Mouser Japan / Chip One Stop stock of FDN338P, FDN335N, PMEG2010AEH and PTS810 unknown; one of them may have stock.
- Winstar WEO012864 availability in Japan not checked beyond a search hit for Winstar modules at Global Denshi.

## Japan import rules for a 1–5 unit hobby/prototype buy (duty/tax threshold, 2026 change, PSE for LiPo)

### Takeaway
Today, a parcel whose total customs value is ¥10,000 or less is exempt from both customs duty and consumption tax; the FY2026 tax reform decided to end the consumption-tax side of that exemption, with reports giving 1 April 2028 as the start, so it does not affect a late-2026 buy. For the battery, PSE applies to lithium-ion cells of 400 Wh/L or more per cell; a 500 mAh pouch cell of the sizes found here computes to roughly 310–375 Wh/L, which is below that line.

### Cited Findings
- Japan Customs answer 1006: imports whose total customs value (課税価格) is ¥10,000 or less are exempt from customs duty and consumption tax; other internal taxes (liquor, tobacco) are not exempt; some goods are excluded (leather bags, knitted clothing, leather shoes etc.). READ — [Japan Customs 1006](https://www.customs.go.jp/tetsuzuki/c-answer/imtsukan/1006_jr.htm)
- Same page: the ¥10,000 test is per declaration; goods on one invoice split into several declarations are summed; for mail it is per package, but parcels from the same sender to the same addressee posted at the same time (e.g. split for weight) are summed. READ — [Japan Customs 1006](https://www.customs.go.jp/tetsuzuki/c-answer/imtsukan/1006_jr.htm)
- FY2026 tax reform outline (customs part), "response to rapidly increasing small-value imports": abolish the special rule that sets the customs value of personal-use goods at 60% of the overseas retail price; no effective date on that page. READ — [Ministry of Finance FY2026 outline](https://www.mof.go.jp/tax_policy/tax_reform/outline/fy2026/08taikou_08.htm)
- FY2026 tax reform: sellers of goods covered by the small-value import exemption become liable for consumption tax, and platform taxation is created. SNIPPET — [Ministry of Finance FY2026 explanation](https://www.mof.go.jp/tax_policy/tax_reform/outline/fy2026/explanation/PDF/p0823-0860.pdf)
- A 10 April 2026 article says the consumption-tax exemption for imports of ¥10,000 or less is to be abolished from 1 April 2028 following the December 2025 ruling-party outline, with a "特定少額資産販売事業者登録制度" registration system; abolition of the customs-duty exemption "is not decided at this time". READ (secondary source) — [Sogyo Techo](https://sogyotecho.jp/1man-menzei-haishi/)
- METI FAQ: lithium-ion batteries are in PSE scope when the volumetric energy density per single cell is 400 Wh/L or more; calculation method is in the notice "電気用品の範囲等の解釈について". SNIPPET — [METI lithium FAQ](https://www.meti.go.jp/policy/consumer/seian/denan/lithium_faq.html)
- Excluded from scope: batteries for automobiles, motorised bicycles, medical equipment and industrial equipment, and those fixed in equipment by soldering or other joining so they cannot easily be removed. SNIPPET — [METI 2008 outline](https://www.meti.go.jp/policy/consumer/seian/denan/hourei/kisoku/080514_gaiyou.pdf)
- PSE is not a government certification: the manufacturer or **importer** notifies METI, confirms conformity to the technical standard, inspects and keeps records for 3 years, and then applies the round PSE mark ("specified electrical appliances other than…" category). SNIPPET — [METI PSE overview](https://www.meti.go.jp/policy/consumer/seian/denan/file/PSE_gaiyo.pdf)
- Sengoku labels some Data Power cells "PSE対象外" (outside PSE scope); the 500 mAh DTP652533 page does not mention PSE. READ — [Sengoku search](https://www.sengoku.co.jp/mod/sgk_cart/search.php?multi=%E3%83%AA%E3%83%81%E3%82%A6%E3%83%A0%E3%82%A4%E3%82%AA%E3%83%B3%E3%83%9D%E3%83%AA%E3%83%9E%E3%83%BC)

### Inferences
- Energy density arithmetic (mine, using outer dimensions; METI's official method may differ): 500 mAh × 3.7 V = 1.85 Wh. DTP652533 at 35 × 25.5 × 6.7 mm = 5.98 cm³ gives about 309 Wh/L. An Adafruit-1578-size cell at 29 × 36 × 4.75 mm = 4.96 cm³ gives about 373 Wh/L. Both are under 400 Wh/L, so such cells would sit outside PSE scope and legitimately carry no PSE mark; that is consistent with Sengoku's "PSE対象外" labels. A denser 500 mAh cell (under about 4.6 cm³) would cross the line.
- A PSE-marked 500 mAh bare cell is therefore not something to expect; the realistic Japan answer is a below-threshold cell with protection circuit from a domestic shop.
- For a DigiKey Japan or Mouser Japan order the ¥10,000 rule is irrelevant: tax is charged at checkout under DDP. It matters for LCSC/AliExpress parcels: a 1–5 unit LCSC basket (roughly USD 30–60 of parts) may land under ¥10,000 customs value and arrive tax-free; above it, 10% consumption tax (and courier advance-payment fees) is collected on delivery. Splitting one order into several parcels to stay under the line does not work for same-time shipments (customs sums them).
- A team buying for a project is arguably not "personal use", so the 60% valuation rule should not be relied on, and it is slated for abolition anyway.
- Importing loose LiPo cells from China: beyond tax, the binding constraints are carrier dangerous-goods rules (lithium cells shipped alone) and, if the cell is 400 Wh/L or more, the importer's own PSE obligations. I found no primary source today for the carrier rules.

### Gaps
- Effective date of the consumption-tax change (1 April 2028) rests on a secondary article and a search summary; the MoF explanation PDF was not opened. The effective date of the 60%-rule abolition was not found.
- METI's FAQ page itself could not be opened (HTTP 403 / empty body); PSE statements are from search extracts of METI pages. Whether importing a ≥400 Wh/L cell purely for one's own use (no sale) triggers the importer obligations was not confirmed.
- No primary source read on restrictions for shipping loose LiPo cells from China to Japan (airline/courier dangerous-goods rules, whether LCSC/AliExpress sellers will ship them to Japan).
- Amazon.co.jp battery listings could not be read; PSE marking on those is unknown.
- Akizuki, Sengoku and Switch Science domestic shipping fees were not read.
