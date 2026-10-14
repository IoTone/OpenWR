# Optics, mechanics and experimental-branch BOMs (Pod-0, Concepts 4 and 5)

Compiled 2026-10-14 for a team of 3 (quantities for 3 builders unless noted).

## How to read these tables

- **Live stock could not be checked today.** DigiKey product pages return HTTP 403 to our fetcher, and Mouser product and search pages time out or hit a bot wall. Thorlabs pages render client-side and came back empty. Every price and stock figure here is therefore from search-index snapshots or the vendor's indexed page, **with no confirmed date**. Re-check each line in a browser before ordering.
- Status tags:
  - **[V]** The part number and price were seen in an indexed vendor page during this session.
  - **[P]** The part number was seen, but the price or stock came from a stale or regional snapshot.
  - **[U]** Unverified: the part number or price comes from memory or inference.
- "Not at DK/M" marks lines with no DigiKey or Mouser source.

## Do DigiKey or Mouser sell imaging optics?

- **DigiKey: partly.** DigiKey has an "Optics - Lenses" category (filter 112) and a "Laser Optics" category. It stocks **Excelitas (LINOS) N-BK7 plano-convex singlets**, for example G312330000 (DigiKey 1601-G312330000-ND, 350-2000 nm, 12.5 mm optic, 25 mm focal length per the listing summary, 21-week lead time). Other LINOS PCX lenses run about $31-52 with single-digit stock. The search turned up no Edmund Optics or Thorlabs product at DigiKey, and **no plate beamsplitters**. [P]
- **Mouser: no.** Mouser's optics are LED secondary optics (LEDiL, Carclo, LedLink). These are TIR collimators and are not usable as an eyepiece or combiner. No Thorlabs, Edmund or beamsplitter listing was found. [P]
- **Conclusion:** buy the optics from Edmund Optics or Thorlabs directly, or from AliExpress for the cheap tier. DigiKey can supply only a slow-lead LINOS lens.
  - [DigiKey lenses category](https://www.digikey.com/en/products/filter/optics-lenses/112)
  - [DigiKey G312330000](https://www.digikey.com/en/products/detail/excelitas-technologies/G312330000/16641416)
  - [Mouser Carclo](https://www.mouser.com/en/manufacturer/carclo-optics/)

## Part 1: Pod-0 optics and mechanics

| # | Item | Mfr / vendor | MPN / SKU | DK PN | Mouser PN | Qty | Unit (qty 1) | Stock seen | Status | URL |
|---|---|---|---|---|---|---|---|---|---|---|
| A1 | Cart. A lens: PCX 15 mm dia x 20 mm FL, uncoated N-BK7 | Edmund Optics | #63-480 | Not at DK/M | Not at DK/M | 3 | $34.00 ($30.75 @10) | n/a | P | [EO](https://www.edmundoptics.com/p/150mm-dia-x-200mm-fl-uncoated-plano-convex-lens/18092) |
| A1-alt | Cart. A lens, AR coated: PCX 15 x 22.5 mm FL, VIS 0° | Edmund Optics | page 8041 (stock # not captured) | Not at DK/M | Not at DK/M | 3 | $41.00 | n/a | P | [EO](https://www.edmundoptics.com/p/15.0mm-dia.-x-22.5mm-fl-vis-0deg-coated-plano-convex-lens/8041/) |
| A1-cheap | Cart. A lens, budget: 15 x 22.5 mm FL commercial grade, MgF2 coated (clearance) | Edmund Optics | page 696 | Not at DK/M | Not at DK/M | 3 | $18.20 | clearance | P | [EO](https://www.edmundoptics.com/p/15mm-dia-x-225mm-fl-commercial-grade-plano-convex-lens-1336272b/696/) |
| A1-DK | Cart. A lens, DigiKey-sourced: LINOS PCX N-BK7, 12.5 mm optic, 25 mm FL | Excelitas | G312330000 | 1601-G312330000-ND | none | 3 | about $31 (class price) | 21-week lead | P | [DK](https://www.digikey.com/en/products/detail/excelitas-technologies/G312330000/16641416) |
| A1-TL | Cart. A lens: Ø1/2" N-BK7 PCX, f = 20 mm, ARC 350-700 nm | Thorlabs | LA1074-A | Not at DK/M | Not at DK/M | 3 | about $35-40 | n/a | U | thorlabs.com |
| A2 | Beamsplitter, 30R/70T VIS λ/10, 25 mm round (the ideal split) | Edmund Optics | #34-412 | Not at DK/M | Not at DK/M | 3 | $222 ($177 @6-49) | 10 in stock | P | [EO](https://www.edmundoptics.com/p/25mm-diameter-30r-70t-vis-lambda-10-plate-beamsplitter/3783/) |
| A2-alt | Beamsplitter, 50R/50T float glass, 1 mm, 25 mm | Edmund Optics | #43-736 | Not at DK/M | Not at DK/M | 3 | $61 | n/a | P (from earlier notes) | [EO](https://www.edmundoptics.com/p/25mm-diameter-50r50t-plate-beamsplitter/5240/) |
| A2-wedge | Beamsplitter, 50R/50T VIS **wedged**, 25 mm (no ghost image) | Edmund Optics | page 3784 (stock # not captured) | Not at DK/M | Not at DK/M | 1-3 | price not captured | n/a | U | [EO EU](https://www.edmundoptics.eu/p/25mm-diameter-50r-50t-vis-wedged-plate-beamsplitter/3784) |
| A2-TL | Beamsplitter, Ø1" 30:70 (R:T) plate, 5 mm UVFS, 400-700 nm | Thorlabs | BSS10 | Not at DK/M | Not at DK/M | 3 | about $150 | n/a | U (R:T naming conflict with BST10; check) | [3DOptix](https://www.3doptix.com/catalog/optics/beam-splitter/thorlabs/BSS10) |
| A-cheap | **AliExpress-class kit:** PMMA aspheric lens 18-20 mm, f ≈ 20 mm, plus a 20-25 mm 30/70 or 50/50 glass or acrylic beamsplitter | AliExpress | various | Not at DK/M | Not at DK/M | 3 sets | about $5-15 per set | n/a | U (grey market, no specs) | aliexpress.com |
| B1 | Cart. B eyepiece: Ø1" N-BK7 PCX, f = 25.4 mm, ARC 350-700 nm | Thorlabs | LA1951-A | Not at DK/M | Not at DK/M | 3 | about $35-40 | n/a | U | [3DOptix spec](https://www.3doptix.com/catalog/optics/lens/thorlabs/LA1951-A) |
| B1-alt | Cart. B eyepiece: same Edmund 15 x 22.5 mm VIS PCX as A1-alt | Edmund Optics | page 8041 | Not at DK/M | Not at DK/M | 3 | $41.00 | n/a | P | see A1-alt |
| B1-cheap | Cart. B eyepiece, budget: 20-25 mm f, 18-25 mm dia loupe or viewfinder lens | AliExpress | various | Not at DK/M | Not at DK/M | 3 | about $2-6 | n/a | U | aliexpress.com |
| M1 | N52 disc magnet, 0.250" x 0.100" (6.35 x 2.54 mm), NiCuNi, 80 °C | Radial Magnets | 9150 | yes (PN not captured) | none | 30 | $0.35 | n/a | P | [DK](https://www.digikey.com/en/products/detail/radial-magnets-inc/9150/12820209) |
| M1-alt | N52 disc, 0.375" x 0.125", for more retention | Radial Magnets | 9151 | yes | none | 12 | price not captured | n/a | P | [DK](https://www.digikey.com/en/products/detail/radial-magnets-inc/9151/12820207) |
| M1-alt2 | N52 disc, 0.375" x 0.100" | Eneflux Armtek | EA-0110 | yes | none | 12 | $0.94 | n/a | P | [DK](https://www.digikey.com/en/products/detail/eneflux-armtek-magnetics-inc./EA-0110/14559295) |
| H1 | M2 brass hex standoff, 5 mm, F-F | Würth Elektronik | WA-SSTII series (PN TBD) | in the DK M2 standoff category | likely | 20 | about $0.30-0.60 | n/a | U | [DK M2 standoffs](https://www.digikey.com/en/products/filter/board-spacers-standoffs/m2/582) |
| H2 | M2 x 6 pan-head machine screw, stainless (focus slide and lid) | Essentra, APM or Würth | TBD | DK category | Mouser category | 100 | about $0.05-0.15 | n/a | U (no exact hit; Keystone at DK lists only M2.5, #4-40 and #6-32) | DK screws category |
| H3 | M2 heat-set insert, brass, for plastic (3-4 mm long) | McMaster-Carr (fallback) | 94180A307 (M2 x 0.4) | **not found at DK** | not checked | 100 | about $15 per 100 | n/a | U | mcmaster.com |
| H3-alt | M2 heat-set insert kit | Ruthex or CNC Kitchen | M2 x 3 / x 4 | Not at DK/M | Not at DK/M | 1 kit | about $12-20 | n/a | U | Amazon |
| H4 | Breakaway mount: magnets M1 plus a printed dovetail with a sacrificial PA12 shear tab (JLC3DP MJF) | in-house | n/a | n/a | n/a | 3 | about $1-3 per part | n/a | design item | [JLC3DP](https://jlc3dp.com/capabilities/3d-printing-service) |
| T1 | Illuminance meter (lux), sky and snow ambient | Extech | LT45 (or LT300) | DigiKey carries Extech (PN not captured) | likely | 1 shared | about $90-130 | n/a | U | DigiKey Extech |
| T2 | **Luminance meter** (cd/m², nits at the panel or eyepiece), 1° spot | Konica Minolta | LS-150 | Not at DK/M (quote only) | Not at DK/M | borrow or rent | about $5k-7k (quote only) | n/a | U (no public price found) | [KM](https://sensing.konicaminolta.us/us/products/ls-150-luminance-meter/) |
| T3 | ND 1.0 (10% T) absorptive filter, 25 mm (category-3 stand-in) | Edmund Optics (Hoya) | page 8291 (1.0 OD 25 mm) | Not at DK/M | Not at DK/M | 1-3 | $51.50 | 20+ in stock | P | [EO](https://www.edmundoptics.com/p/10-od-25mm-dia-absorptive-nd-filter/8291/) |
| T3-alt | ND 1.0, Ø25 mm absorptive, SM1-mounted | Thorlabs | NE10A-A | Not at DK/M | Not at DK/M | 1 | about $80 new (one used eBay listing: $77) | n/a | U | [3DOptix spec](https://3doptix.com/catalog/optics/filter/thorlabs/NE10A-A) |
| T3-kit | 25 mm absorptive ND kit, 6 filters (covers category 2-4) | Edmund Optics | page 12021 | Not at DK/M | Not at DK/M | 1 shared | price not captured | n/a | U | [EO](https://www.edmundoptics.com/p/25mm-dia-absorptive-nd-filter-kit-6-filters/12021/) |
| T4 | Real category-3 sunglass lens (8-18% T, which is about ND 0.75-1.1) | any retail sunglasses | n/a | n/a | n/a | 3 | about $15-25 | n/a | U | retail |

Part 1 subtotals for 3 builders (estimates; shared test gear counted once):

| Option | Lines | Subtotal |
|---|---|---|
| **P1-A Edmund economical** | A1 ×3 ($102), A2-alt 50/50 ×3 ($183), B1-alt ×3 ($123), M1 ×30 ($10.50), H1, H2 and H3 (about $40), T1 (about $110), T3 ($51.50), T4 (about $20) | **about $640** |
| **P1-B Edmund 30/70 premium** | P1-A with A2 #34-412 ×3 ($666) in place of A2-alt | **about $1,120** |
| P1-C Thorlabs | LA1074-A ×3, BSS10 ×3, LA1951-A ×3, plus the shared items above | about $950 (U) |
| **P1-D AliExpress tier** | A-cheap ×3, B1-cheap ×3, plus magnets, hardware, lux meter, ND filter and sunglasses | **about $250** (optics about $60) |
| Luminance meter | Rent or borrow the LS-150; or calibrate a phone or camera against the T1 lux meter on a diffuse target | quote only |

## Part 2: Concept 4, micro-OLED on an open FPGA

| # | Item | Mfr / vendor | MPN / SKU | DK PN | Mouser PN | Qty | Unit (qty 1) | Stock seen | Status | URL |
|---|---|---|---|---|---|---|---|---|---|---|
| F1 | **iCEBreaker** iCE40UP5K board, 3 PMOD ports, open toolchain | 1BitSquared | iCEBreaker v1.1 | not found | yes, via the 1BitSquared storefront at Mouser (PN not captured) | 1-2 | about $70-85 (2021-22 history) | 72 (old Mouser snapshot) | P | [Mouser 1BitSquared](https://www.mouser.com/manufacturer/1bitsquared/featured-products), [Crowd Supply](https://www.crowdsupply.com/1bitsquared/icebreaker-fpga) |
| F2 | iCE40 UltraPlus breakout | Lattice | iCE40UP5K-B-EVN | yes (PN not captured) | 842-ICE40UP5K-B-EVN | 1 | about £74 (about $99) at Mouser UK | 7 (+84 on order), cached | P | [findchips](https://www.findchips.com/search/iCE40) |
| F3 | CrossLink-NX eval board (only if the panel is MIPI DSI; hard D-PHY) | Lattice | LIFCL-40-EVN | likely (not captured) | not found | 1 | $134-139 (Newark-class quotes) | n/a | P | [findchips](https://findchips.com/search/-l40e%2C) |
| F4 | OrangeCrab ECP5-25F (Feather form factor) | Good Stuff Dept. / 1BitSquared | OrangeCrab r0.2.1 | not found | not found | (alt) | $99-129 | n/a | Not at DK/M (U) | GroupGets, 1BitSquared |
| F5 | ULX3S ECP5-12F/45F/85F | Radiona | ULX3S | not found | Crowd Supply items ship via Mouser, but no Mouser PN was found | (alt) | $115 / $135 / $155 (2020 pricing) | n/a | Not at DK/M (U) | [Crowd Supply](https://crowdsupply.com/radiona/ulx3s) |
| R1 | **Raspberry Pi Pico 2** (RP2350), FPGA-free PIO path | Raspberry Pi | SC1631 | 2648-SC1631CT-ND (cut tape; a plain -ND PN probably exists) | 358-SC1631 (U) | 3 | about $5 (EU €4.30) | 6,000-26,000 (regional DK) | P | [DK](https://digikey.sg/en/products/detail/raspberry-pi/SC1631/24627136) |
| D1 | HDMI driver board for 0.23" 640x400 micro-OLED (Sony **ECX336A/B/C**), 40.5 x 12.5 mm, powered from HDMI, brightness adjust | Display Components (HK), Tindie | Tindie listing | Not at DK/M | Not at DK/M | 1-2 | **$99.50** | not stated | V (Tindie page fetched) | [Tindie](https://tindie.com/products/oled-modules/hdmi-driver-board-for-the-sony-microdisplays) |
| D2 | Sony ECX336C 0.23" panel (bare) | Tindie / grey market | ECX336C | Not at DK/M | Not at DK/M | 1-2 | listings run $75 to $574 and do not map prices to products | n/a | U | same seller |
| D3 | Hicenda 0.23" 640x400 helmet module (panel + prism + board) | Hicenda | 0.23" kit | Not at DK/M | Not at DK/M | 1 | **$184**; 4,000-nit version **$389** | n/a | P (from research notes) | [Hicenda](https://www.hicendadisplay.com/products/0-39-inch-oled-microdisplay-module-rgb-1024x768-integrated-kits-for-ar) |
| D4 | Hicenda 0.39" 1024x768 or LCoS AR kit | Hicenda | 0.39" kit | Not at DK/M | Not at DK/M | (alt) | $286-415 | n/a | P | same |
| D5 | SeeYA 0.49" 1920x1080 1,800-3,000 nit + Type-C/HDMI-to-MIPI board | DisplayModule / Tindie | DM-OLED049-1002/1003 | Not at DK/M | Not at DK/M | (alt) | about $149-579 | samples "ship immediately" | P | [DisplayModule](https://www.displaymodule.com/products/0-49-micro-oledos-display-1920x1080-90hz-3000nits-mipi-1003) |

Part 2 subtotals (one bench rig shared by the EE, with spares where cheap):

| Option | Lines | Subtotal |
|---|---|---|
| **P2-A iCE40 + Sony 0.23"** | F1 ×1 (about $80) + R1 ×3 ($15) + D1 ×1 ($99.50) + D2 ×1 (about $100-200, U) | **about $300-400** |
| P2-B Hicenda reference | D3 4,000-nit ($389) + F1 + R1 ×3 | about $485 |
| P2-C MIPI path | F3 (about $139) + D5 (about $149-300) | about $290-440 |
| P2-D Lattice-only buy | F2 (about $99) + R1 ×3 + D1 + D2 | about $320-415 |

## Part 3: Concept 5, USB-C display on a Linux puck through Monado

| # | Item | Mfr / vendor | MPN / SKU | DK PN | Mouser PN | Qty | Unit (qty 1) | Stock seen | Status | URL |
|---|---|---|---|---|---|---|---|---|---|---|
| L1 | Raspberry Pi 5, 8 GB | Raspberry Pi | SC1112 | page 21658257 (one regional page shows **2648-SC1432-ND**, which may be a newer SKU; check) | 358-SC1112 | 1 | MSRP $80 → $95 (Dec 2025) → $125 (Feb 2026) → **about $175 (Apr 2026, BGR)** | DK CH 16,266 (CHF 137.55) | P | [DK](https://www.digikey.ch/en/products/detail/raspberry-pi/SC1112/21658257), [Mouser](https://ro.mouser.com/ProductDetail/358-SC1112), [BGR](https://www.bgr.com/2150106/raspberry-pi-cost-ram-crisis/) |
| L2 | Pi 27 W USB-C PD supply (5.1 V / 5 A) | Raspberry Pi | SC1157 / SC1158 (US plug and colour variants) | yes | 358-SC115x | 1 | about $12 | n/a | U | DK / Mouser |
| L3 | Pi 5 Active Cooler | Raspberry Pi | SC1148 | yes | 358-SC1148 | 1 | about $5 | n/a | U | DK / Mouser |
| L4 | microSD 64 GB A2 (Pi-branded or SanDisk) | Raspberry Pi | SC1628-class | yes | yes | 1 | about $10-15 | n/a | U | |
| L4-alt | M.2 HAT+ + 256 GB NVMe (Pi SSD) | Raspberry Pi | SC1166 + SC1440-class | yes | yes | 1 | about $12 + $30-45 | n/a | U | |
| L5 | Micro-HDMI to HDMI cable, 1 m | Raspberry Pi | SC0298-class | yes | yes | 1 | about $5 | n/a | U | |
| L6 | **HDMI → glasses adapter** (the Pi 5 has no DP alt mode; the adapter supplies DP-alt to the glasses plus power) | XREAL | "XREAL Adapter" (HDMI + USB-C power in) | Not at DK/M | Not at DK/M | 1 | about $59-69 | n/a | U | xreal.com |
| L6-alt | Viture HDMI dock ("Mobile Dock") | Viture | Mobile Dock | Not at DK/M | Not at DK/M | (alt) | about $99-129 | n/a | U | viture.com |
| L7 | **RK3588 alternative with native USB-C DP alt:** Radxa ROCK 5B (2x HDMI 2.1 + USB-C DP; needs about 30 W PD) | Radxa | ROCK 5B 8/16 GB | **not found** | **not found** | (alt) | reference $189 (pre-memory-crisis) | Ameridroid "low stock" | P | [hackerboards](https://hackerboards.com/board/radxa-rock-5b), [Radxa FAQ](https://docs.radxa.com/en/rock5/rock5b/faq), [Ameridroid](https://ameridroid.com/products/rock-5-model-b) |
| L7-alt | Orange Pi 5 (RK3588S; USB-C 3.1 with DP 1.4) | Shenzhen Xunlong | Orange Pi 5 8 GB | not found | not found | (alt) | about $100-130 | n/a | U | Amazon / AliExpress |
| I1 | BNO085 9-DoF fusion IMU breakout, STEMMA QT | Adafruit | 4754 | **1528-4754-ND** | 485-4754 (U) | 3 | **$24.95** | 921 (one snapshot); 0 + 4-week lead (another) | P | [DK](https://punchouttest.digikey.ca/en/products/detail/adafruit-industries-llc/4754/13426653), [Adafruit](https://adafruit.com/product/4754) |
| I2 | QT Py RP2040 (USB HID or CDC IMU bridge) | Adafruit | 4900 | 1528-4900-ND (U) | 485-4900 (U) | 3 | about $9.95 | n/a | U | adafruit.com/product/4900 |
| I3 | STEMMA QT cable, 50 mm | Adafruit | 4399 | 1528-4399-ND (U) | yes | 3 | about $0.95 | n/a | U | |
| C1 | Stereo camera: Luxonis OAK-D Lite (USB-C, on-board stereo depth) | Luxonis | OAK-D-LITE | DK listing unconfirmed | unconfirmed | 1 | about $149 | n/a | U | shop.luxonis.com |
| C1-alt | DIY stereo pair on the Pi 5's two CSI ports: 2x Camera Module 3 + 2x Pi 5 22-to-15-pin cable | Raspberry Pi | SC0872 + SC1128-class | yes | yes | 2 + 2 | about $25 + $1-3 | n/a | U | DK / Mouser |
| G1 | Borrowed birdbath glasses: XREAL One | XREAL | One | Not at DK/M | Not at DK/M | 1 (borrowed) | US launch $499; JP list ¥62,980, sale ¥35,980-55,980 | n/a | P (US street price unverified) | [Android Authority](https://www.androidauthority.com/xreal-smart-glasses-price-cut-3660556/) |
| G1-alt | XREAL One Pro | XREAL | One Pro | Not at DK/M | Not at DK/M | (alt) | $599 (cut from $649) | n/a | P | same |
| G1-alt2 | Viture Pro XR | Viture | Pro | Not at DK/M | Not at DK/M | (alt) | about $399-459 | n/a | U | viture.com |

Part 3 subtotals (one puck rig; IMU ×3 so each builder can test):

| Option | Lines | Subtotal |
|---|---|---|
| **P3-A Pi 5 + HDMI adapter** | L1 (about $175) + L2 + L3 + L4 + L5 (about $32) + L6 (about $65) + I1 ×3 ($74.85) + I2 ×3 ($29.85) + I3 ×3 (about $3) + C1-alt (about $55) | **about $435** (glasses borrowed) |
| **P3-B ROCK 5B, direct USB-C DP** | L7 (about $189-250, U) + 30 W PD supply (about $15) + storage (about $15) + IMU set (about $108) + C1 ($149) | **about $475-540** |
| Glasses, if they must be bought | G1 (about $399-499) or G1-alt2 | +$400-500 |

## Interface and pin facts for the system schematics

### Pod-0 mechanics

- Magnet retention: **2-4 N52 discs per pod**.
  - Radial 9150 is 6.35 mm dia x 2.54 mm and is rated to 80 °C, so keep it off the LiPo.
  - Pull force was not captured; measure it on the bench.
  - Pair each magnet with a steel disc or an opposite-pole magnet in the dock.
- Breakaway: the magnets set the release force, and the dovetail only registers position (no latch).
- Focus slide: M2 screw with **±2 mm travel**. Use M2 heat-set inserts in the SLA housing.

### Concept 4 (FPGA micro-OLED)

**Sony ECX336 bus (as Frame drives it):**
- Frame drives the panel with an **8-bit YCbCr parallel bus** from a CrossLink-NX.
- An MCU configures the panel over **SPI** (Frame uses an nRF52840).
- Source: [Frame hardware](https://docs.brilliant.xyz/frame/hardware).
- Budget about **17 FPGA or MCU pins**:

| Signal group | Pins |
|---|---|
| D[7:0] | 8 |
| PCLK, HSYNC, VSYNC, DE (or ENB) | 4 |
| SPI: SCK, MOSI, CS, MISO/SO | 4 |
| XCLR (reset) | 1 |

- Add panel power rails (Sony panels typically need about 1.8 V logic plus a higher analog or OLED rail, U). There is **no public datasheet**, so the exact pinout and FPC pin count are unverified.

**Tindie ECX336 driver board (D1):**
- **HDMI input only**, powered from HDMI.
- It is a bench tool for viewing the panel, not a parallel-bus interface.
- For RTL work you replace this board with your own FPC breakout.

**iCEBreaker (F1):**
- **3 PMOD ports**: PMOD1A, PMOD1B and PMOD2.
- Each port has 12 pins: 8 I/O + 2 GND + 2 3V3.
- 1A and 1B pair as a **double-wide PMOD (16 I/O)**. PMOD2 is often fitted with the button and LED snap-off board.
- That gives **24 I/O** through PMODs, enough for the 17-pin ECX336 bus. Logic is 3.3 V, so level-shift to the panel's 1.8 V I/O (U).
- FTDI FT2232H handles USB programming and a UART.

**iCE40UP5K-B-EVN (F2):** has 0.1" headers broken out from the UP5K, but **no PMOD** (U); plan to use jumper wires.

**OrangeCrab (F4):** Feather pin-out, about 20 usable I/O (U). Tight but workable.

**ULX3S (F5):** 2x 28-pin GPIO headers (about 56 I/O, U), plus a **GPDI (HDMI-connector) output** usable as a DVI source.

**CrossLink-NX LIFCL-40-EVN (F3):** MIPI D-PHY hard IP with MIPI connectors, plus PMOD or Raspberry Pi-style headers (U). This is the only board here that talks **MIPI DSI** natively. It is needed for the SeeYA 0.49" and DisplayModule MIPI panels.

**Micro-OLED inputs by option:**

| Option | Panel | Input |
|---|---|---|
| D1/D2 | Sony 0.23" | Parallel (YCbCr or RGB) + SPI; HDMI through the Tindie board |
| D3/D4 | Hicenda kits | Driver-board input, likely HDMI (U) |
| D5 | SeeYA 0.49" | **MIPI DSI**; Type-C (DP alt) or HDMI through the DisplayModule board |
| Waveguide module (from the report) | DMGTX0023WGNA | 24-bit parallel RGB or HDMI through its board |

**Pico 2 (R1):**
- 26 GPIO on the Pico header, with PIO for the parallel bus. 17 pins fit with room for an I2C IMU.
- Logic is 3.3 V, so the 1.8 V panel I/O needs a level shifter (U).

### Concept 5 (Linux puck)

**Raspberry Pi 5 video outputs:**
- **2x micro-HDMI** (4Kp60).
- **2x 4-lane MIPI 22-pin FPC**, each switchable between CSI and DSI.
- The **USB-C port is power-only, with no DisplayPort alt mode.**
- XREAL, Viture and similar glasses expect **USB-C DP alt-mode video plus 5 V**, so the Pi 5 needs an HDMI → USB-C DP-alt adapter with power in (L6).
- The Pi 5 also wants 5 V/5 A on its USB-C input. The 27 W PSU is required to give the USB ports their full 1.6 A.

**ROCK 5B (L7):**
- USB-C with **DP alt mode** (up to 8K@30 per Ameridroid), so the glasses plug straight in. It also has 2x HDMI 2.1 out.
- Needs about 30 W USB-PD. Radxa warns that some third-party PD supplies cause boot loops.

**IMU bridge (I1 + I2):**
- BNO085 over I2C at 0x4A (0x4B with the ADDR jumper) to the QT Py RP2040 through STEMMA QT: 3V3, GND, SDA, SCL.
- Bring INT and RST out for FIFO and sync timing (BNO085 pins: INT, RST, P0/P1 for mode).
- The QT Py enumerates over USB as a CDC or HID device to the puck, where a Monado 3DoF driver reads it.
- This mirrors the REV1 hooks: "IMU with FIFO + sync pin" and a "versioned USB protocol".

**Stereo camera:**
- OAK-D Lite: USB-C (USB 3), with depth computed on the device.
- DIY pair (C1-alt): uses both Pi 5 CSI ports, with the 22-pin to 15-pin cables.

## Unverified items and follow-ups

1. Open every DigiKey and Mouser line in a browser on order day; our fetcher was blocked.
2. Confirm that the DigiKey Pi 5 8 GB PN (2648-SC1112-ND vs 2648-SC1432-ND) and the US price are about $175.
3. Thorlabs prices (LA1074-A, LA1951-A, BSS10, NE10A-A) and the BSS10 vs BST10 R:T naming.
4. Edmund wedged 50/50 (page 3784): stock # and price.
5. Get a quote for the Konica Minolta LS-150 or LS-160, or find a rental (for example a university optics lab).
6. Whether OAK-D Lite, ULX3S, OrangeCrab and ROCK 5B are at DigiKey or Mouser today: none was found.
7. ECX336 pinout, rails and FPC: no public datasheet. Reverse-engineer from the Tindie board or Frame's open RTL and schematics (check the licence).
