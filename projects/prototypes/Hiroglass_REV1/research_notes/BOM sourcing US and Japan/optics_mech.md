# Optics and mechanical parts sourcing for the HUD prototype, US and Japan (checked 2026-11-10)

How to read confidence labels used below:

- **PAGE** = I read the number on the seller's own page on 2026-11-10.
- **SNIPPET** = the number came from a search-engine summary of the seller page (cached, may be stale).
- **UNCONFIRMED** = I could not read it; no number is given or the number is a secondary source.

Access limits on 2026-11-10: edmundoptics.com (US), kjmagnetics.com, monotaro.com and jp.misumi-ec.com returned HTTP 403 to my fetches; thorlabs.com / thorlabs.co.jp returned an empty JavaScript shell; jp.optosigma.com redirected to a static landing page; mcmaster.com and amazon.com / amazon.co.jp were not readable. So all Edmund US prices are SNIPPET, all Thorlabs prices are UNCONFIRMED, and most Japan mechanical prices are UNCONFIRMED. edmundoptics.jp, optosigma.com (US), aliexpress.com (US storefront), twowaymirrors.com, extech.com and prusa3d.com were readable.

## 1. What do #63-480, #43-736, #34-412 and the ND filter cost at Edmund US and Edmund Japan, are they in stock, and is there a minimum order or handling fee?

### Takeaway
All five Edmund parts are listed in stock in both regions and the US prices match the October BOM; the key correction is that the 25 mm 30R/70T float-glass plate is **#43-734 at $61.00 / ¥8,550**, not #34-412 ($222.00 / ¥31,100, a 3 mm fused-silica λ/10 plate). Edmund Japan prices are tax-exclusive, web-cart orders ship free in Japan, and I found no minimum order in either region.

### Cited Findings

Edmund Optics part table (US = SNIPPET, Japan = PAGE, all checked 2026-11-10):

| Ref | Stock no. | Description | US price 1-9 | US stock | Japan price (ex-tax) | Japan stock shown |
|---|---|---|---|---|---|---|
| LENS-A | #63-480 | PCX 15 x 20 mm, uncoated | $34.00 (10-24: $30.75) | 20+ | ¥4,800 (10-24: ¥4,350; 25-49: ¥3,850) | 20+ |
| LENS-B | #47-873 | PCX 15 x 22.5 mm, VIS 0° coated, N-BK7 | $48.50 | 20+ | ¥6,800 (10-24: ¥6,100; 25-49: ¥5,450) | not clearly shown |
| LENS-B alt | #23-083 | PCX 15 x 22.5 mm, MgF2, N-SK18, commercial grade, clearance | not found (C$29.93 on the Canada site) | 20+ (Canada/India pages) | ¥2,800 (single tier) | 20+ |
| BS-A | #43-736 | Plate BS 50R/50T, 25 mm dia | $61.00 (25-49: $41.25) | 20+ | ¥8,550 (10-24: ¥6,450; 25-49: ¥5,800) | 20+ |
| BS-B (correct cheap part) | #43-734 | Plate BS 30R/70T, 25 mm dia, float glass, 1.00 mm, 400-700 nm | $61.00 (10-24: $45.75) | not in snippet | ¥8,550 (10-24: ¥6,450; 25-49: ¥5,800) | 20+ |
| BS-B alt | #13-403 | Plate BS 30R/70T, 25 x 25 mm, AR-coated back, 3 mm float | $63.00 | 14 | ¥8,850 (10-24: ¥6,700; 25-49: ¥6,250) | 14 |
| BS-B alt | #47-240 | 25 mm dia 30R/70T VIS plate BS | $149.00 (6-25: $116.00) | 5 | not checked | not checked |
| BS-B (as listed in BOM) | #34-412 | TS λ/10 plate BS 30R/70T, 25 mm, fused silica, 3 mm, 30 arcmin wedge | $222.00 (qty 1-5) | 10 | ¥31,100 (1-5), ¥24,800 (6-49) | 7 |
| ND | #48-090 | Hoya absorptive ND, OD 1.0, 25 mm | $51.50 (qty 1-10) | 20+ | ¥7,250 (1-10), ¥6,450 (11-49) | 20+ |

- #63-480 Japan: "TS 平凸レンズ 15 x 20", ¥4,800 at 1-9, "20+ 個 在庫有" (PAGE) — [edmundoptics.jp search 63-480](https://www.edmundoptics.jp/search/?criteria=63-480)
- #63-480 US: $34.00 at 1-9, $30.75 at 10-24, 20+ in stock (SNIPPET; the result's URL slug named a different lens, so confirm on the live page) — [edmundoptics.com](https://www.edmundoptics.com/search/?criteria=63-480)
- #43-736 Japan: "プレート型B/S 50R/50T 25mm", ¥8,550 / ¥6,450 / ¥5,800, 20+ (PAGE) — [edmundoptics.jp search 43-736](https://www.edmundoptics.jp/search/?criteria=43-736)
- #43-736 US: $61.00 at 1-9, down to $41.25 at 25-49, 20+ in stock (SNIPPET) — [edmundoptics.com #43-736](https://www.edmundoptics.com/p/25mm-diameter-50r50t-plate-beamsplitter/5240/)
- #34-412 is verified as a real number: "TS λ/10 プレート型B/S 25mm 30R/70T", 25.00 mm dia, 3.00 mm thick, fused silica (Corning 7980), 30/70 ±5% at 550 nm, ¥31,100 at 1-5, 7 in stock (PAGE) — [edmundoptics.jp search 34-412](https://www.edmundoptics.jp/search/?criteria=34-412)
- #34-412 US: $222.00 at 1-5, 10 in stock (SNIPPET) — [edmundoptics.com #34-412](https://www.edmundoptics.com/p/25mm-diameter-30r-70t-vis-lambda-10-plate-beamsplitter/3783/)
- #43-734 Japan: "プレート型B/S 30R/70T 25mm", 25.00 mm ±0.38, 1.00 mm, float glass, 400-700 nm, dielectric coating on surface 1, surface 2 uncoated, flatness 4-6λ, ¥8,550 at 1-9, 20+ (PAGE) — [edmundoptics.jp search 43-734](https://www.edmundoptics.jp/search/?criteria=43-734)
- #43-734 US: $61.00 at 1-9, $45.75 at 10-24 (SNIPPET) — [edmundoptics.com #43-734](https://www.edmundoptics.com/p/25mm-diameter-30r70t-plate-beamsplitter/5238/)
- #13-403 Japan: 25 x 25 mm, 3.00 mm float glass, 30/70, ¥8,850 at 1-9, 14 in stock (PAGE) — [edmundoptics.jp search 13-403](https://www.edmundoptics.jp/search/?criteria=13-403); US $63.00 at 1-9, 14 in stock (SNIPPET) — [edmundoptics.com #13-403](https://www.edmundoptics.com/p/25-x-25mm-30r70t-ar-coated-plate-beamsplitter/45378/)
- #47-240 US: 25 mm dia 30R/70T VIS plate, $149.00 at 1-5, $116.00 at 6-25, 5 in stock (SNIPPET) — [edmundoptics.com #47-240](https://www.edmundoptics.com/p/25mm-dia-30r70t-vis-plate-beamsplitter/7262/)
- LENS-B exact stock number is #47-873: "TS 平凸レンズ 15 x 22.5 VIS 0°", dia 15.00 +0/-0.025, EFL 22.50 at 587.6 nm, Ravg ≤0.4% 425-675 nm, ¥6,800 at 1-9 (PAGE; stock count not clearly rendered) — [edmundoptics.jp search 47-873](https://www.edmundoptics.jp/search/?criteria=47-873)
- #47-873 US: $48.50 at 1-9, 20+ in stock, N-BK7 (SNIPPET). This is $7.50 above the $41.00 in the October BOM — [edmundoptics.com family 8041](https://www.edmundoptics.com/p/150mm-dia-x-225mm-fl-vis-0deg-coated-plano-convex-lens/8041/)
- #23-083 Japan: "平凸レンズ 15 X 22.5 MGF2 (商用グレード)", MgF2 Ravg ≤1.75% 400-700 nm, EFL 22.50 ±1%, ¥2,800 single tier, 20+; clearance item, no returns except manufacturing defects (PAGE) — [edmundoptics.jp #23-083](https://www.edmundoptics.jp/p/15mm-dia-x-225mm-fl-commercial-grade-plano-convex-lens-1336272b/696/)
- #23-083 other regions: C$29.93 Canada, ₹2,119 India, clearance, 20+ in stock; no US-dollar price found (SNIPPET) — [edmundoptics.com #23-083](https://www.edmundoptics.com/p/15mm-dia-x-225mm-fl-commercial-grade-plano-convex-lens-1336272b/696/)
- #48-090 Japan: "Hoya 吸収型 NDフィルター OD1.0 25mm", OD 1.0 ±0.05, 25.00 mm, ¥7,250 at 1-10, ¥6,450 at 11-49, 20+ (PAGE) — [edmundoptics.jp search 48-090](https://www.edmundoptics.jp/search/?criteria=48-090)
- #48-090 US: $51.50 at 1-10, 20+ in stock (SNIPPET). Other OD 1.0 25 mm options: #29-001 Wratten 2 at $91.50, #63-410 non-reflective at $165.00 (SNIPPET) — [edmundoptics.com ND OD 1.0 25 mm](https://www.edmundoptics.com/p/10-od-25mm-dia-absorptive-nd-filter/8291/)
- Edmund Japan prices exclude consumption tax ("消費税を含まない外税表示") (PAGE) — [edmundoptics.jp customer FAQ](https://www.edmundoptics.jp/contact-support/customer-service-frequently-asked-questions/)
- Edmund Japan shipping: orders through the website cart carry no domestic shipping charge; FAX/e-mail orders under ¥50,000 pay a flat ¥1,000 (ex-tax). No minimum order amount is stated (PAGE) — [edmundoptics.jp customer FAQ](https://www.edmundoptics.jp/contact-support/customer-service-frequently-asked-questions/)
- Edmund Japan only serves customers resident in Japan with a Japanese delivery address (PAGE) — [edmundoptics.jp customer FAQ](https://www.edmundoptics.jp/contact-support/customer-service-frequently-asked-questions/)
- Edmund US terms: the only fee surfaced is a $25.00 processing fee folded into shipping/handling for quotes paid by bank wire; no order-value minimum surfaced (SNIPPET, truncated) — [edmundoptics.com terms and conditions](https://www.edmundoptics.com/terms-and-conditions/)

### Inferences
- At 1 to 5 pieces every Edmund part stays in its first price tier, so price_5 equals price_1 for all of them (the first break is at 6 or 10 pieces).
- The Japan/US ratio is about ¥140 per dollar on these parts (¥4,800/$34.00 = 141; ¥8,550/$61.00 = 140; ¥7,250/$51.50 = 141; ¥31,100/$222 = 140), before Japan's 10% consumption tax.
- Replacing #34-412 with #43-734 saves $161 / ¥22,550 per piece and gives the ~1 mm float plate the design asked for; #34-412 is 3 mm thick, which would also add a more displaced ghost reflection.
- Paying by card through the web cart avoids both the US wire fee and the Japan FAX/e-mail shipping fee.

### Gaps
- Edmund US pages were not readable (HTTP 403), so US prices and stock are search-snippet values; the US shipping charge for a small order and any US small-order fee are unconfirmed.
- US price and stock for #23-083 (clearance 15 x 22.5 MgF2 lens) not found.
- Japan stock for #47-873 was not legible on the page.

## 2. Thorlabs equivalents (12.7 mm lenses f = 20-25 mm; 25 mm plate beamsplitters), price and same-day stock in the US and Japan

### Takeaway
I could not read a single current Thorlabs price or stock status in either region (the site returns an empty shell to my fetcher), so every Thorlabs row is UNCONFIRMED. The best second catalog source I could actually read is OptoSigma US, which sells the exact 15 x 20 mm lens for $14.43 uncoated, but with a contradictory "In stock / Delivery in 6-12 weeks" status.

### Cited Findings
- Thorlabs EBP1 is the "Economy 30:70 Beamsplitter, Ø1", AOI 45°", 25.4 mm, 1 mm thick, T 70% / R 30% ±10% over 450-650 nm; an undated Thorlabs catalog page lists it at $28.30 (SNIPPET, old catalog, not a current price) — [Thorlabs catalog page 833](https://www.thorlabs.com/catalogpages/833.pdf); [GoPhotonics EBP1](https://www.gophotonics.com/products/beam-splitters/thorlabs-inc/37-46-ebp1)
- The same catalog snippet shows "¥ 270.30" for EBP1; the column header is RMB, so that is Chinese yuan, not Japanese yen (SNIPPET) — [Thorlabs catalog page 833](https://www.thorlabs.com/catalogpages/833.pdf)
- Thorlabs BSW10R is a 25 x 36 mm 50:50 UVFS plate beamsplitter; BSS10R is described as the rectangular 30:70 UVFS plate; no price in results (SNIPPET, third-party catalog) — [3DOptix BSW10R](https://www.3doptix.com/catalog/optics/beam-splitter/thorlabs/BSW10R); [3DOptix BSS10R](https://3doptix.com/catalog/optics/beam-splitter/thorlabs/BSS10R)
- LA1074 is a Ø1/2" N-BK7 plano-convex lens with f = 20.0 mm (seen for the -780 variant); no price for LA1074-A (SNIPPET) — [GoPhotonics LA1074-780](https://gophotonics.com/products/optical-lenses/thorlabs-inc/33-46-la1074-780)
- OptoSigma US, exact 15 mm x 20 mm plano-convex: "Reasonable Plano Convex Lens BK7 15mm Diameter 20mm Focal Length Uncoated", SKU S-SLB-15-20P, $14.43, page shows "In stock" and also "Delivery in: 6-12 weeks" (PAGE) — [optosigma.com S-SLB-15-20P](https://www.optosigma.com/us_en/optics/lenses/spherical-lenses/reasonable-plano-convex-lens-bk7-15mm-diameter-20mm-focal-length-uncoated.html)
- OptoSigma US, same size other grades: "Reasonable" 400-700 nm coated $30.05; standard grade uncoated $27.64; standard 400-700 nm $58.00; fused silica $112.00 (PAGE, search results list; SKUs and stock not shown) — [optosigma.com search](https://www.optosigma.com/us_en/catalogsearch/result/?q=plano+convex+15mm+diameter+20mm+focal+length)
- OptoSigma (Sigma Koki) SLB-15-20P is a 15.0 mm dia, 20.0 mm focal length N-BK7 uncoated plano-convex lens (SNIPPET) — [3DOptix SLB-15-20P](https://www.3doptix.com/catalog/optics/lens/optosigma/SLB-15-20P)
- OptoSigma US dielectric plate beamsplitters 400-700 nm are $295.00 each; PSM25-25.4C03-10-550 (25.4 mm, 3 mm, R:T 1:3) shows "In stock", PSM33-25.4C03-10-550 (R:T 1:2) shows 6-12 weeks (PAGE) — [optosigma.com dielectric plate beamsplitters](https://www.optosigma.com/us_en/optics/beamsplitters/plate-beamsplitters/dielectric-plate-beamsplitters.html)

### Inferences
- If the old $28.30 EBP1 price is roughly still valid, EBP1 would be the cheapest catalog 30:70 plate in the US (about half of Edmund #43-734), but it must be checked on thorlabs.com before it goes in a BOM.
- OptoSigma plate beamsplitters at $295 are not competitive with Edmund's $61 float plates for this build; OptoSigma is only interesting for the lens.
- A 12.7 mm Thorlabs lens is a mechanical change (lens seat 15 mm to 12.7 mm) and reduces the eyebox; with Edmund and OptoSigma both offering exact 15 mm parts there is no need to switch.

### Gaps
- Current price and same-day stock for Thorlabs LA1074-A, LA1540-A, LA1951-A, EBS1, EBP1, BSW10R, BSS10R in the US: UNCONFIRMED.
- Thorlabs Japan (thorlabs.co.jp) yen prices and stock: UNCONFIRMED, not readable.
- Sigma Koki Japan (jp.optosigma.com) yen price and stock for SLB-15-20P and plate half mirrors: UNCONFIRMED, site redirected to a static page. Tokai Optical and Chuo Precision were not checked.
- Misumi and MonotaRO as resellers of Sigma Koki optics: pages returned HTTP 403; UNCONFIRMED.

## 3. Cheapest workable beamsplitter in each region (teleprompter glass, acrylic two-way mirror, half-mirror film)

### Takeaway
US-only: a $9.95 teleprompter-glass sample (4 x 4 in, 2 mm, 30R/70T or 40R/60T) from Two Way Mirrors, cut down by hand, is the cheapest readable option. Japan-deliverable: AliExpress 25 mm class plates at about $12 to $28 are the cheapest readable option; I found no priced Japanese half-mirror acrylic source.

### Cited Findings
- Two Way Mirrors (US) sells teleprompter-mirror samples at $9.95 each: 6 x 6 in in 3 mm or 4 mm, and 4 x 4 in in 2 mm, in 30R/70T or 40R/60T; thicknesses offered are 2, 3 and 4 mm; cutting tolerance ±1/16 in; "Ships right away" (PAGE) — [twowaymirrors.com teleprompter mirror](https://www.twowaymirrors.com/teleprompter-mirror/)
- Two Way Mirrors custom-size price is calculator/quote based and was not shown on the page; 50R/50T appears only in the calculator dropdown (PAGE) — [twowaymirrors.com teleprompter mirror](https://www.twowaymirrors.com/teleprompter-mirror/)
- Full-size replacement teleprompter glass is far more expensive: Ikan 70/30 glass $175.00, LCD4Video 7 x 9 in 70/30 glass $59.95 (SNIPPET) — [SCMS Ikan PT2500-G](https://www.scmsinc.com/ikan-pt2500-g-mirror-glass-for-pt-elite-ipad-and-pt2500-teleprompter.html); [ProCam LCD4Video glass](https://procam.com/a/llms/products/lcd4video-teleprompter-glass-mirror-replacement)
- AliExpress (US storefront view, USD): "50T/50R 70T/30R square beam splitter plate, 25/30 mm, 1.0 mm thick, AR coated" $12.29 (28 sold); "1:1 50R/50T optical glass plate beamsplitter" $12.72 (40 sold); "50/50 and 80/20 45° plane beam splitter 36 x 25 x 1.1 mm" $18.05 to $27.99 (the $27.99 listing has free shipping, 39 sold); "25 x 36 mm visible 400-700 nm half beamsplitter plate" $38.13 (PAGE, search results; shipping not shown for most) — [aliexpress.com beam splitter plate 25mm](https://www.aliexpress.com/w/wholesale-beam-splitter-plate-25mm.html)
- Surplus Shed search for "beamsplitter" returned only cube beamsplitters in the part of the page I could read (10 mm $18.95, 15 mm $21.00, 20 mm $25.00, 30 mm $39.00, 40 mm $59.50); no plate beamsplitters seen (PAGE, first half of results only) — [surplusshed.com search](https://www.surplusshed.com/search.php?search_query=beamsplitter)
- Japan: a search for cut-to-size half-mirror acrylic found only clear acrylic sheet (MonotaRO 2 mm clear, ¥1,538 incl. tax for 545 x 650 mm) and optical-grade half mirrors (Misumi dielectric cube half mirror CSMH series from about ¥16,700 ex-tax); no priced half-mirror acrylic listing surfaced (SNIPPET) — [monotaro.com acrylic sheet](https://www.monotaro.com/p/6408/0443/); [misumi half mirror](https://jp.misumi-ec.com/vona2/detail/223302756874/)

### Inferences
- A 4 x 4 in (101.6 mm) sample holds a 4 x 4 grid of 25 mm squares on paper, so one $9.95 sample can cover all five units with spares if hand-scoring 2 mm glass goes well. That is my arithmetic, not a vendor claim.
- Teleprompter glass is 2 mm or thicker, twice the 1 mm of the Edmund float plates, so the back-surface ghost image is displaced about twice as far; acceptable for a first brightness test, worse for text legibility.
- The AliExpress "70T/30R" 1.0 mm plate at $12.29 is the closest cheap match to BS-B (30R/70T, 1 mm); the 80/20 listings are the closest to a 20R/80T combiner, but the listings do not say which number is reflection.
- For Japan the cheapest low-risk path is still Edmund Japan #43-734 at ¥8,550 (in stock, known coating), with AliExpress as the low-cost experiment.

### Gaps
- No readable price for acrylic two-way mirror sheet or half-mirror window film in either region (Amazon pages not readable).
- Japan-side half-mirror suppliers (はざいや, アクリ屋, Kenis, Akizuki, Sengoku) returned nothing priced; UNCONFIRMED.
- Anchor Optics was not checked. Taobao and Alibaba were not readable; no CNY prices.
- AliExpress shipping cost and delivery days were not shown on the search pages; the 1-3 week figure in the brief is not confirmed by a page I read.

## 4. Lead time risk: made-to-order optics and overseas stock

### Takeaway
Edmund Japan's "in stock" counts include overseas warehouses, so plan on 5-7 business days rather than 1-2 for any Edmund part in Japan. The real schedule risks are OptoSigma US (6-12 weeks shown on the cheap lens and on most plate beamsplitters) and China-direct parcels into the US after the October 2026 customs changes.

### Cited Findings
- Edmund Japan standard delivery: about 1-2 business days after order when stock is in Japan, about 5-7 business days when stock is overseas; longer in busy periods or with customs delays (PAGE) — [edmundoptics.jp customer FAQ](https://www.edmundoptics.jp/contact-support/customer-service-frequently-asked-questions/)
- Every Edmund Japan product page I read carries the note "在庫数は海外倉庫分の在庫も含んでおり、日本国内の在庫を示すものではありません" (stock count includes overseas warehouses and does not indicate stock in Japan) (PAGE) — [edmundoptics.jp search 63-480](https://www.edmundoptics.jp/search/?criteria=63-480)
- Edmund low-count parts: #34-412 shows 7 (Japan page) / 10 (US snippet); #47-240 shows 5 (US snippet); #13-403 shows 14 in both — [edmundoptics.jp search 34-412](https://www.edmundoptics.jp/search/?criteria=34-412); [edmundoptics.jp search 13-403](https://www.edmundoptics.jp/search/?criteria=13-403)
- Edmund India lists #43-736 with a "10 to 12 days" lead time, an example of how regional sites draw on remote stock (SNIPPET) — [edmundoptics.in plate beamsplitters](https://www.edmundoptics.in/f/PdfExport/12424)
- OptoSigma US S-SLB-15-20P: "In stock" and "Delivery in: 6-12 weeks" on the same page (PAGE) — [optosigma.com S-SLB-15-20P](https://www.optosigma.com/us_en/optics/lenses/spherical-lenses/reasonable-plano-convex-lens-bk7-15mm-diameter-20mm-focal-length-uncoated.html)
- OptoSigma US plate beamsplitters: one variant in stock, one at 2 weeks, four at 6-12 weeks (PAGE) — [optosigma.com dielectric plate beamsplitters](https://www.optosigma.com/us_en/optics/beamsplitters/plate-beamsplitters/dielectric-plate-beamsplitters.html)
- Prusa heat-set inserts M2 short, 100 pcs: in stock, "Preparation time: 4-7 business days" before shipping (PAGE) — [prusa3d.com M2 short inserts](https://www.prusa3d.com/product/heat-set-inserts-m2-short-100-pcs/)
- JLC3DP build time: resin 48-72 hours, nylon 48-72 hours, ABS 96 hours, plus 1-3 business days for small-volume orders (PAGE) — [jlc3dp.com build time](https://jlc3dp.com/help/article/1219-What-is-the-build-time-of-the-3D-printing-parts)

### Inferences
- For Japan, assume every Edmund line ships from overseas stock: 5-7 business days is about 7-10 calendar days. The four 20+ parts are unlikely to slip beyond that; #34-412 (7 left) is the only Edmund part where 5 pieces could exhaust visible stock.
- For the US, Edmund's 20+ parts are low risk, but I could not read Edmund's US ship-time statement, so "same-day shipping" is not confirmed here.
- Treat the OptoSigma US $14.43 lens as a 6-12 week item until a cart check proves otherwise; it should not gate the build.

### Gaps
- Edmund US ship time and carrier cost: not readable.
- Which Edmund warehouse (US, Singapore, Korea) holds each part: not shown on any page I read.
- AliExpress delivery days to the US and to Japan: not shown on the pages I read.

## 5. Import situation: US de minimis for China-origin parcels in November 2026, and Japan's consumption tax and 10,000-yen exemption

### Takeaway
US: the $800 de minimis exemption is still suspended for China (and for every other country), so a ~$50 AliExpress optics order is dutiable; the flat per-parcel fees are gone and ordinary tariffs now apply, but the exact rate for optics and the handling of postal parcels after 22 October 2026 are not settled by any official page I could read. Japan: imports with a customs value of ¥10,000 or less are exempt from duty and consumption tax, and that rule is still in force.

### Cited Findings
- De minimis treatment was suspended indefinitely for products of the PRC and Hong Kong effective May 2025 and for all other countries effective August 2025; Congress repealed Section 321 for all commercial shipments effective July 1, 2027 (P.L. 119-21); "Low-value imports are now subject to applicable U.S. tariffs" (primary source, CRS In Focus IF12891, updated August 7, 2026, read as PDF) — [CRS IF12891](https://www.everycrsreport.com/files/2026-08-07_IF12891_bc938dadca910fad3d3a455956da173e6b4aa610.pdf)
- "In July 2026, CBP launched an entry process for postal shipments valued at $800 or less, and in September 2026 is to pilot a voluntary electronic process for postal shipments valued at $2,500 or less" (CRS, same document) — [CRS IF12891](https://www.everycrsreport.com/files/2026-08-07_IF12891_bc938dadca910fad3d3a455956da173e6b4aa610.pdf)
- CBP says it collected over $1 billion in duties on over 246 million low-cost shipments between May and December 2025 (CRS, same document) — [CRS IF12891](https://www.everycrsreport.com/files/2026-08-07_IF12891_bc938dadca910fad3d3a455956da173e6b4aa610.pdf)
- History of the flat fee: in May 2025 China postal parcels under $800 faced a 54% tariff or a $100 per-item fee (dated May 15, 2025; superseded) — [FreightWaves](https://www.freightwaves.com/news/house-considers-universal-de-minimis-ban-as-fees-on-china-parcels-ease)
- IEEPA-based country duties stopped on February 24, 2026; a 10% Section 122 surcharge applied to postal items from February 24 and expired July 24, 2026; from July 24, 2026 "standard duties (MFN, Section 232, Section 301, etc.) now apply to postal shipments for the first time", and postal shipments under $2,500 require prepaid duties; a Section 301 forced-labor tariff of 12.5% is listed for China (secondary source, tariff-software vendor) — [Zonos US tariff updates](https://zonos.com/us-tariff-updates)
- A trade blog (published October 3, updated October 23, 2026) says no flat fee or surcharge applies now: a China postal parcel pays the Column 1 rate plus any Section 301 rate plus a 0.3464% merchandise processing fee; it cites EO 14388 (91 FR 9433), Proclamation 11012 (91 FR 9339), and CBP interim final rules 91 FR 37801 and 91 FR 38007 of June 24, 2026 (secondary source, single blog) — [Landed Math](https://landedmath.com/blog/us-de-minimis-postal-route-closes-october-2026/)
- The same blog says that from October 22, 2026 postal informal entry closes for goods subject to Chapter 98/99 duties (which includes most Section 301 China goods), leaving formal entry or bonded "entry type 13", with entry preparation costs it estimates at $20-$60 per shipment; it had not confirmed how CBP is enforcing this (secondary source, single blog, not corroborated) — [Landed Math](https://landedmath.com/blog/us-de-minimis-postal-route-closes-october-2026/)
- AliExpress duty collection for US buyers varies by seller and shipping method: some listings show "incl. import charges" at checkout, others ship without duty prepaid and the carrier bills duty plus a brokerage fee (secondary sources, commercial blogs) — [TariffsTool AliExpress guide](https://www.tariffstool.com/import-tax/aliexpress)
- Japan: imported goods whose total customs value is ¥10,000 or less are exempt from customs duty and consumption tax; goods on one invoice or mailed in split parcels are added together; some goods (examples given: handbags, knitted clothing, ski equipment) are excluded (primary source, PAGE) — [Japan Customs answer 1006](https://www.customs.go.jp/tetsuzuki/c-answer/imtsukan/1006_jr.htm)
- Japan: for goods bought at retail for the importer's personal use, the customs value is the overseas retail price x 0.6, which makes the effective exemption ceiling ¥16,666; a reform of the low-value exemption was decided in the FY2026 tax reform, with private media reporting an April 1, 2028 start (SNIPPET of a Ministry of Finance document and trade press) — [MOF FY2026 tax reform explanation](https://www.mof.go.jp/tax_policy/tax_reform/outline/fy2026/explanation/PDF/p0823-0860.pdf); [sogyotecho.jp](https://sogyotecho.jp/1man-menzei-haishi/)
- Japan: when the total of duty and consumption tax on a postal item is ¥10,000 or less, Japan Post delivers it and collects the tax on delivery (PAGE) — [Japan Customs personal import guide](https://www.customs.go.jp/tsukan/kojinyunyu.htm)

### Inferences
- US, ~$50 AliExpress optics order: expect duty to be charged, most likely prepaid at checkout. Using Zonos's own worked example of about 37.5% combined duty on a $50 China product, the duty would be roughly $19; this is an illustrative figure from a secondary source for a different product, not a rate for lenses or beamsplitters. The optics-specific HTS rate (heading 9001/9002) and whether Section 301 applies to it were not verified.
- US worst case: if the seller does not prepay and the parcel needs a broker entry, a $20-$60 entry fee could exceed the value of a $12 beamsplitter. This is another reason to keep the US build on US-stocked parts.
- Because the US-only case excludes China sellers anyway, the de minimis question only matters if the US site wants to use the cheap AliExpress beamsplitters as an experiment.
- Japan: a single AliExpress optics order of one to five beamsplitters ($12 to $62) is well under the ¥10,000 customs-value threshold at any plausible exchange rate if the 0.6 rule applies, and likely under it even without, so no duty or consumption tax is expected. If the purchase is made by a company rather than for personal use, the 0.6 rule does not apply and the plain ¥10,000 threshold governs.
- Japan: keep AliExpress orders split by need, not artificially; customs adds up parcels from one invoice.

### Gaps
- No official CBP page (CSMS message or Federal Register text) was read directly; the post-July and post-October 2026 postal rules rest on one vendor page and one blog, which broadly agree with each other and with CRS on the suspension but are not primary sources.
- The actual duty rate for HTS 9001/9002 optics of Chinese origin in November 2026 (MFN plus Section 301 list coverage) is unconfirmed.
- One search result said the China reciprocal-tariff arrangement was "extended through November 10, 2026", i.e. today; I could not confirm what happens after that date.
- A search result claimed a new per-parcel postal handling fee "no later than November 1, 2026"; I found no confirmation.
- The Japan Customs page for the 0.6 personal-use rule was not read directly (the page I fetched did not contain it).

## 6. Mechanical parts, lux meter and 3D printing: best and second source per region

### Takeaway
US mechanical parts are all cheap and available from US sellers (K&J, Zoro, Amazon), with the only notable cost being brand-name M2 inserts at about $36 per 100. For Japan I could not read MonotaRO, Misumi, Akizuki or Amazon.co.jp prices for the small hardware, so the Japan mechanical lines are largely unconfirmed; AliExpress covers magnets and a lux meter cheaply.

### Cited Findings

US, mechanical and tools:

- MAG: K&J Magnetics D41-N52 (1/4 in dia disc, N52) $0.27 each at 1-49 on one cached page, $0.32 on another; D42-N52 $0.52 each ($0.50 at 50-99); D43-N52 $0.67 at 1-24 (SNIPPET; thicknesses not shown in the snippets, and no 1/4 x 1/10 in part was identified) — [kjmagnetics.com D41-N52](https://kjmagnetics.com/d41-n52-neodymium-disc-magnet); [kjmagnetics.com D42-N52](https://kjmagnetics.com/d42-n52-neodymium-disc-magnet?cat=168)
- SCR: Zoro M2-0.40 x 6 mm Phillips pan machine screw, 316 stainless, 100 pk $8.99; 18-8 stainless Phillips 100 pk $9.69; 18-8 slotted pan 100 pk $2.69 (SNIPPET) — [zoro.com ZSCRW-429](https://zoro.com/usa-industrials-m2-040-x-6-mm-phillips-pan-machine-screw-passivated-316-stainless-steel-100-pk-zscrw-429/i/G201010850); [zoro.com slotted pan](https://zoro.com/zoro-select-m2-040-x-6-mm-slotted-pan-machine-screw-plain-18-8-stainless-steel-100-pk-m511400200006/i/G1912407)
- STO: Amazon MECCANIXITY M2 male brass hex standoff kit, 20 sets, $8.19 (price dated 2026-04-23); Walmart 100 pcs M2 8+4 mm male-female brass standoffs $9.29 (SNIPPET) — [specpicks listing](https://specpicks.com/product/B0FP8MKB8L); [walmart.com M2 standoff](https://www.walmart.com/c/kp/m2-standoff)
- INS: McMaster 94180A307 (tapered brass heat-set insert, M2 x 0.4, 2.9 mm installed length, 100 pk) is resold by Zoro at $36.35 per 100 in one snippet and $39.19 in another; no McMaster price read (SNIPPET, two conflicting values) — [zoro.com 94180A307](https://zoro.com/bsc-preferred-tapered-heat-set-inserts-for-plastic-brass-m2-x-040-mm-thread-size-2900-mm-installed-lngth-100pk-94180a307/i/G411638875)
- INS alternative: Prusa "Heat-set inserts M2 short, 100 pcs" $11.99, in stock, preparation 4-7 business days, ship-from not stated (PAGE) — [prusa3d.com](https://www.prusa3d.com/product/heat-set-inserts-m2-short-100-pcs/)
- INS alternative: CNC Kitchen M2 standard (M2 x 3.0), 100 pcs $11.52 ex-VAT at 3DJake; Ruthex RX-M2x4 is sold in 70-piece packs ($8.79 ex-VAT at 3DJake), not 100 (SNIPPET; 3DJake is an EU seller) — [3djake.com CNC Kitchen M2](https://3djake.com/cnc-kitchen/threaded-inserts-m2-standard); [3djake.com Ruthex M2](https://3djake.com/ruthex/threaded-insert-m2-70-pieces)
- LUX: the Extech LT45 is not a ~$110 meter: Transcat $318.99, Zoro $314.99, ValueTronics $284.23 (SNIPPET) — [transcat.com LT45](https://www.transcat.com/brand/extech-store/light-meters/extech-lt45)
- LUX: the ~$110-$155 Extech is the LT40 (LED light meter, up to 400,000 lux, ±3%): $155.00 on extech.com (PAGE); a search snippet of the same page showed "$144.00 $109.00"; Instrumart $127.99 with 3 in stock; Transcat $144.00 (SNIPPET) — [extech.com LT40](https://www.extech.com/products/lt40/); [instrumart.com LT40](https://www.instrumart.com/products/39554)
- LUX cheapest credible in US: Dr.meter LX1330B (0-200,000 lux) $40.99 at one reseller, $59.95 with free shipping on eBay (SNIPPET) — [ebay.com LX1330B](https://www.ebay.com/p/2238960293)
- Filament: a 4-pack of 1 kg Elegoo Rapid PETG was $39.91 on Amazon in January 2026 (about $10/kg), with commenters saying $50+ is the normal 4-pack price (SNIPPET, 10 months old) — [slickdeals.net](https://slickdeals.net/f/19139599-39-91-4-count-of-1kg-elegoo-rapid-1-75mm-petg-3d-printer-filament-at-amazon)
- Print service: JLC3DP advertises "From $0.30 with no minimum order" and cut selected material prices by up to 70% effective July 24, 2026 (SNIPPET); build 48-72 h for resin or nylon plus 1-3 business days for small orders (PAGE). JLC3DP is a China-based service, so it counts for the Japan case but not as a US seller — [jlc3dp.com pricing update](https://jlc3dp.com/news/materials-finishing-pricing-update-july2026); [jlc3dp.com build time](https://jlc3dp.com/help/article/1219-What-is-the-build-time-of-the-3D-printing-parts)

Japan-deliverable, mechanical and tools:

- MAG (China direct): AliExpress "10 or 50 pcs/lot Disc N52 NdFeB 3x3 to 6x3" from $1.09 ($0.11/pc), 600+ sold; "N52 5x4/6x3/5x3/5x2 Round Mini Magnet" from $1.09, 1,000+ sold; no 6 x 2.5 mm listing (PAGE, US storefront view; "from" prices for the smallest option) — [aliexpress.com N52 6x3](https://www.aliexpress.com/w/wholesale-neodymium-magnet-6x3mm-N52.html)
- MAG (Japan): a 6 x 3 mm round neodymium magnet "ネオジウム磁石 マル 6×3mm 51-402" is listed at ¥679 incl. tax per bag at an office-supply mail-order site; pieces per bag not shown (SNIPPET) — [kaunet.com](https://www.kaunet.com/rakuraku/variation/00150470/)
- MAG (Japan): Akizuki's catalog search for ネオジム磁石 returned no loose magnets (only a speaker) (PAGE) — [akizukidenshi.com search](https://akizukidenshi.com/catalog/goods/search.aspx?search=x&keyword=%E3%83%8D%E3%82%AA%E3%82%B8%E3%83%A0%E7%A3%81%E7%9F%B3)
- LUX (Japan catalog): Sanwa LX20 ¥22,185 ex-tax / ¥24,404 incl.; LX-204 (AS ONE 6-6131-12) ¥22,631 ex-tax / ¥24,894 incl.; TLX-204 ¥20,167 ex-tax / ¥22,184 incl. (SNIPPET, Misumi) — [misumi LX20](https://jp.misumi-ec.com/vona2/detail/223000668399/); [misumi TLX-204](https://jp.misumi-ec.com/vona2/detail/223304212784/)
- LUX (Japan budget): Shinwa 78747 separate-sensor lux meter (0-40,000 lux, 0.1 lux resolution) about ¥8,626 incl. tax at one industrial mail-order listing, ¥9,727 at another (SNIPPET, seller not clearly identified) — [misumi Shinwa 78747](https://jp.misumi-ec.com/vona2/detail/223007959434/)
- LUX (China direct): UNI-T UT383 / UT383BT $19.39 to $35.19; UT383S $29.69 to $45.75; one UT383 listing at $23.79 has 390 sold; several listings show free shipping (PAGE, US storefront view) — [aliexpress.com UT383](https://www.aliexpress.com/w/wholesale-UT383-lux-meter.html)
- Print service (Japan): DMM.make offers a no-registration price simulator at quote.make.dmm.com (information dated July 2024); nylon is PA2200 on EOS SLS machines; no current price read (SNIPPET) — [ipros DMM.make listing](https://mono.ipros.com/product/detail/2000562039)

### Inferences
- The BOM's "Extech LT45 class, ~$110" mixes two models: LT45 is ~$315; the LT40 is the $109-$155 meter. For a relative brightness test (with and without the ND filter) a $20-$60 meter (UT383, LX1330B) is sufficient; absolute accuracy matters little for a ratio.
- US insert cost is dominated by brand: $36-$39 per 100 for the McMaster-equivalent part vs about $12 for Prusa or CNC Kitchen packs. For 5 units any 100-pack is plenty.
- One 100-pack each of screws and inserts covers all five units; magnets (50) and standoffs (30) are the only mechanical parts whose cost scales from 1 to 5 units.

### Gaps
- K&J Magnetics pages were not readable (403); the exact K&J part for 1/4 x 1/10 in N52 and its price are unconfirmed; $0.35 each in the BOM is consistent with the snippet range ($0.27-$0.67) but not verified.
- McMaster-Carr price for 94180A307: not read.
- Japan prices for M2 x 6 pan-head screws, M2 standoffs and M2 heat-set inserts (MonotaRO, Misumi, Amazon.co.jp, Akizuki, Sengoku): UNCONFIRMED, none readable.
- Amazon.com and Amazon.co.jp current prices for magnets, screws, inserts, lux meters and filament: UNCONFIRMED.
- PETG/PLA 1 kg filament price in Japan: UNCONFIRMED. US price is a 10-month-old deal post.
- Print-service quotes for ~5 small parts (Craftcloud, Xometry, JLC3DP to US or Japan, DMM.make): none obtained; all need an STL upload to quote. JLC3DP shipping cost and days to the US and Japan: UNCONFIRMED.
- Custom, HIOKI and Tasco lux meters: not found.

## 7. Total optics + mechanical cost for 1 unit and 5 units per region, catalog tier and cheapest workable tier, with gating item and delivery estimate

### Takeaway
Catalog tier from Edmund costs about $315 for one fully optioned unit in the US ($256 optics + $59 hardware) and ¥35,950 ex-tax (¥39,545 incl. tax) for the optics in Japan; the cheapest workable tier is about $150 in the US and roughly ¥10,050 + $12 in Japan for optics. Nothing in the Edmund catalog tier is a long-lead item; the gating risks are the Japan hardware lines I could not price and, for cheap tiers, hand-cut teleprompter glass (US) or AliExpress transit (Japan).

### Cited Findings
All prices below are the ones cited in sections 1, 3 and 6; the sums are my arithmetic. Excluded everywhere: sales tax, shipping, lux meter, OLED, electronics, filament and printing.

"Full option set" = one of each optic the BOM lists (LENS-A, LENS-B, BS-A, BS-B, ND). The ND filter is a test tool, so it is bought once, not per unit.

US-only sellers, catalog tier (Edmund US, SNIPPET prices; BS-B as #43-734):

| Item | 1 unit | 5 units |
|---|---|---|
| LENS-A #63-480 $34.00 | $34.00 | $170.00 |
| LENS-B #47-873 $48.50 | $48.50 | $242.50 |
| BS-A #43-736 $61.00 | $61.00 | $305.00 |
| BS-B #43-734 $61.00 | $61.00 | $305.00 |
| ND #48-090 $51.50 (once) | $51.50 | $51.50 |
| Optics subtotal | $256.00 | $1,074.00 |
| MAG K&J D42-N52, 10 per unit ($0.52; $0.50 at 50) | $5.20 | $25.00 |
| SCR Zoro 316 SS 100 pk | $8.99 | $8.99 |
| STO 20-set kit $8.19 (2 kits for 30) | $8.19 | $16.38 |
| INS Zoro 94180A307 100 pk | $36.35 | $36.35 |
| Mechanical subtotal | $58.73 | $86.72 |
| **Total** | **$314.73** | **$1,160.72** |

- With #34-412 ($222.00) in place of #43-734, add $161.00 per unit: $475.73 for 1 unit, $1,965.72 for 5 — [edmundoptics.com #34-412](https://www.edmundoptics.com/p/25mm-diameter-30r-70t-vis-lambda-10-plate-beamsplitter/3783/)
- A lean unit with one lens and one combiner (LENS-B + #43-734) is $109.50 of optics per unit — [edmundoptics.com #43-734](https://www.edmundoptics.com/p/25mm-diameter-30r70t-plate-beamsplitter/5238/)

US-only sellers, cheapest workable tier:

| Item | 1 unit | 5 units |
|---|---|---|
| Lens: Edmund #63-480 $34.00 | $34.00 | $170.00 |
| Combiner: Two Way Mirrors 4 x 4 in 2 mm 30R/70T sample $9.95, hand cut | $9.95 | $9.95 |
| ND #48-090 (once) | $51.50 | $51.50 |
| MAG | $5.20 | $25.00 |
| SCR Zoro slotted 18-8 100 pk $2.69 | $2.69 | $2.69 |
| STO | $8.19 | $16.38 |
| INS Zoro 94180A307 | $36.35 | $36.35 |
| **Total** | **$147.88** | **$311.87** |

- If the OptoSigma US lens S-SLB-15-20P ($14.43) is really deliverable, the lens line drops by $19.57 per unit, but the page shows a 6-12 week delivery — [optosigma.com S-SLB-15-20P](https://www.optosigma.com/us_en/optics/lenses/spherical-lenses/reasonable-plano-convex-lens-bk7-15mm-diameter-20mm-focal-length-uncoated.html)

Japan-deliverable, catalog tier (Edmund Japan, PAGE prices, ex-tax; optics only):

| Item | 1 unit | 5 units |
|---|---|---|
| LENS-A #63-480 ¥4,800 | ¥4,800 | ¥24,000 |
| LENS-B #47-873 ¥6,800 | ¥6,800 | ¥34,000 |
| BS-A #43-736 ¥8,550 | ¥8,550 | ¥42,750 |
| BS-B #43-734 ¥8,550 | ¥8,550 | ¥42,750 |
| ND #48-090 ¥7,250 (once) | ¥7,250 | ¥7,250 |
| **Optics total, ex-tax** | **¥35,950** | **¥150,750** |
| Optics total incl. 10% consumption tax | ¥39,545 | ¥165,825 |

- With #34-412 (¥31,100) in place of #43-734, add ¥22,550 ex-tax per unit — [edmundoptics.jp search 34-412](https://www.edmundoptics.jp/search/?criteria=34-412)
- Web-cart orders ship free within Japan; delivery 1-2 business days from Japan stock, 5-7 business days from overseas stock — [edmundoptics.jp customer FAQ](https://www.edmundoptics.jp/contact-support/customer-service-frequently-asked-questions/)

Japan-deliverable, cheapest workable tier (optics only):

| Item | 1 unit | 5 units |
|---|---|---|
| Lens: Edmund Japan #23-083 clearance 15 x 22.5 MgF2 ¥2,800 ex-tax | ¥2,800 | ¥14,000 |
| Combiner: AliExpress 25/30 mm 1.0 mm 50/50 or 70T/30R plate $12.29 | $12.29 | $61.45 |
| ND #48-090 ¥7,250 ex-tax (once) | ¥7,250 | ¥7,250 |
| **Optics total** | **¥10,050 ex-tax + $12.29** | **¥21,250 ex-tax + $61.45** |

- Sources for the three lines — [edmundoptics.jp #23-083](https://www.edmundoptics.jp/p/15mm-dia-x-225mm-fl-commercial-grade-plano-convex-lens-1336272b/696/); [aliexpress.com beam splitter plate 25mm](https://www.aliexpress.com/w/wholesale-beam-splitter-plate-25mm.html); [edmundoptics.jp search 48-090](https://www.edmundoptics.jp/search/?criteria=48-090)

### Inferences
- US catalog tier: gating item is none of the optics (all 20+ in stock per snippets); practical delivery is driven by Edmund's US ship time, which I could not read. Assume about a week door to door until confirmed.
- US cheapest tier: gating item is the hand-cut teleprompter glass (skill and breakage risk, not lead time) and, if chosen, the 6-12 week OptoSigma lens.
- Japan catalog tier: gating item is Edmund overseas stock, 5-7 business days (about 7-10 calendar days). The whole optics set can come from one free-shipping web order.
- Japan cheapest tier: gating item is the AliExpress beamsplitter transit time (not confirmed on a page; the brief's 1-3 weeks is the working assumption). The clearance lens #23-083 is non-returnable and could sell out, though 20+ were shown.
- Japan hardware (magnets, screws, standoffs, inserts) is likely a few thousand yen in total from MonotaRO or Amazon.co.jp, but I have no readable prices, so I have left it out of the Japan totals rather than estimate.
- Recommendation implied by the numbers: buy #43-734 instead of #34-412 in both regions; in Japan consider #23-083 at ¥2,800 as the LENS-B stand-in (same 15 x 22.5 mm prescription class, MgF2 instead of VIS 0° coating, N-SK18 instead of N-BK7).

### Gaps
- Japan mechanical subtotal: not computed (no readable prices).
- Shipping and sales tax for all US lines: not included, not readable.
- Thorlabs-based totals: not computed (no readable prices).
- The US cheapest tier assumes one 4 x 4 in sample yields at least five usable 25 mm pieces; untested.
