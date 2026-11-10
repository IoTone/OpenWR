# SubverseReader REV1 BOMs

Bills of materials for the fourteen SubverseReader variants (sixteen BOMs: SR-14 has two builds, and SR-1C is the C5 alternative to SR-1). `make_bom.py` generates every file here; edit it, not the CSVs.

**Before ordering:** prices are 2026-11-06/07 snapshots; panels and XIAO boards are seller listings from 2026-11-07, see [AVAILABILITY.md](AVAILABILITY.md). Lines marked `estimate` have no live figure. Import the `*.digikey-import.csv` or `*.mouser-import.csv` file into the distributor's BOM tool for today's price; AliExpress, LCSC and vendor-direct lines are not in the import files.

**Quantities:** each order builds 5 units (PCB fabs ship 5 boards). `Per unit @5` is the order total divided by five; `Per unit @100` uses volume estimates (LCSC passives, factory-direct panels and cells) and excludes tooling.

**Ordering:** `order.py` writes [order/ORDER.md](order/ORDER.md): landed cost (parts + shipping per vendor), the date the parts are in hand, DigiKey one-click cart links and Newark paste lists for each variant. `python3 order.py --live` refreshes it from the DigiKey, Mouser and Newark APIs when their keys are set.

Target: **$50 or less per unit.**

| Variant | Order total (5) | Per unit @5 | Per unit @100 | Under $50? | Files |
|---|---|---|---|---|---|
| SR-1 Laminate: XIAO ESP32-S3 Plus on the Seeed EE05 board + 4.2in, no custom PCB | $200 | $40.04 | $31.94 | yes | [`SR-1.csv`](SR-1.csv) |
| SR-1C Laminate C5: DFRobot FireBeetle 2 ESP32-C5 + 4.2in, no custom PCB | $209 | $41.72 | $30.71 | yes | [`SR-1C.csv`](SR-1C.csv) |
| SR-2 All Access: ESP32-S3 custom PCB + 5.83in, card pocket | $275 | $54.99 | $34.46 | at volume | [`SR-2.csv`](SR-2.csv) |
| SR-3 Gray Pass: M5Stack PaperDIY S3 kit + printed shell | $183 | $36.55 | $26.81 | yes | [`SR-3.csv`](SR-3.csv) |
| SR-4 Headliner: 6in 300 ppi, epdiy-style ESP32-S3 board, card pocket | $339 | $67.85 | $41.30 | at volume | [`SR-4.csv`](SR-4.csv) |
| SR-5 Crew Clip: ESP32-C5 custom PCB + 4.2in landscape, belt clip | $199 | $39.76 | $23.14 | yes | [`SR-5.csv`](SR-5.csv) |
| SR-6 Night Shift: ESP32-C5 + 4.2in with front light | $243 | $48.61 | $29.52 | yes | [`SR-6.csv`](SR-6.csv) |
| SR-7 Poster: SR-2 board + 5.83in 4-colour panel, zine sleeve | $275 | $55.09 | $34.51 | at volume | [`SR-7.csv`](SR-7.csv) |
| SR-8 Quiet Pass: XIAO nRF52840 Plus on the Seeed EE05 board, no Wi-Fi, 4.2in | $234 | $46.84 | $34.83 | yes | [`SR-8.csv`](SR-8.csv) |
| SR-9 Roadie: ESP32-C5 + 4.2in with a PSP-style thumbstick | $210 | $42.04 | $25.36 | yes | [`SR-9.csv`](SR-9.csv) |
| SR-10 Credential: 85 x 55 mm card (CR80), 2.9in | $155 | $30.95 | $16.79 | yes | [`SR-10.csv`](SR-10.csv) |
| SR-11 Full Bleed: 3 x 4 in (76 x 102 mm), 3.97in 800x480 | $209 | $41.88 | $25.10 | yes | [`SR-11.csv`](SR-11.csv) |
| SR-12 Tour Laminate: 80 x 145 mm, 3.97in 800x480 | $218 | $43.54 | $26.06 | yes | [`SR-12.csv`](SR-12.csv) |
| SR-13 Hybrid Credential: 3.69 x 8.49 in (94 x 216 mm), 4.7in + ID card window | $195 | $39.05 | $27.96 | yes | [`SR-13.csv`](SR-13.csv) |
| SR-14 Tap Pass, Wi-Fi build: ESP32-C5 + NFC reader, 80 x 155 mm | $261 | $52.28 | $31.92 | at volume | [`SR-14W.csv`](SR-14W.csv) |
| SR-14 Tap Pass, tethered build: nRF52840 + NFC reader, 80 x 155 mm | $269 | $53.78 | $33.28 | at volume | [`SR-14B.csv`](SR-14B.csv) |

## Size, radios and storage

Dimensions are the enclosure as rendered (width x height x depth, mm). Weights are estimates: panel + cell + PETG + board, without the lanyard.

| Variant | W x H x D (mm) | ~g | Radios | microSD | USB-C does |
|---|---|---|---|---|---|
| SR-1 | 86 x 138 x 12.5 | 95 | Wi-Fi 4 2.4 GHz, BLE 5 | no | charge, USB drive, console |
| SR-1C | 86 x 138 x 13 | 100 | Wi-Fi 6 2.4 + 5 GHz, BLE 5 | no | charge, console, Web Serial loader |
| SR-2 | 109 x 177 x 19.5 (15.5 without pocket) | 150 | Wi-Fi 4 2.4 GHz, BLE 5 | yes | charge, USB drive, console |
| SR-3 | 78 x 149 x 14.5 | 110 | Wi-Fi 4 2.4 GHz, BLE 5 | yes (on the kit) | charge, USB drive, console |
| SR-4 | 109 x 172 x 17.5 (13.5 without pocket) | 150 | Wi-Fi 4 2.4 GHz, BLE 5 | yes | charge, USB drive, console |
| SR-5 | 104 x 109 x 21.5 with clip (16 body) | 110 | Wi-Fi 6 2.4 + 5 GHz, BLE 5 | yes | charge, console, Web Serial loader |
| SR-6 | 88 x 138 x 14.5 | 105 | Wi-Fi 6 2.4 + 5 GHz, BLE 5 | yes | charge, console, Web Serial loader |
| SR-7 | 109 x 177 x 19.5 (15.5 without sleeve) | 150 | Wi-Fi 4 2.4 GHz, BLE 5 | yes | charge, USB drive, console |
| SR-8 | 86 x 138 x 13 | 95 | BLE 5 only | yes | charge, USB drive, UF2 updates |
| SR-9 | 88 x 149 x 14.5 | 105 | Wi-Fi 6 2.4 + 5 GHz, BLE 5 | yes | charge, console, Web Serial loader |
| SR-10 | 85 x 55 x 8.5 | 40 | Wi-Fi 6 2.4 + 5 GHz, BLE 5 | yes | charge, console, Web Serial loader |
| SR-11 | 76.2 x 101.6 x 11 (+5 D-ring) | 70 | Wi-Fi 6 2.4 + 5 GHz, BLE 5 | yes | charge, console, Web Serial loader |
| SR-12 | 80 x 145 x 12 | 90 | Wi-Fi 6 2.4 + 5 GHz, BLE 5 | yes | charge, console, Web Serial loader |
| SR-13 | 94 x 216 x 14.5 | 165 | Wi-Fi 4 2.4 GHz, BLE 5 | yes (on the kit) | charge, USB drive, console |
| SR-14W | 80 x 155 x 12.5 | 95 | Wi-Fi 6 2.4 + 5 GHz, BLE 5, NFC reader + tag | yes | charge, console, Web Serial loader |
| SR-14B | 80 x 155 x 12.5 | 95 | BLE 5 only, NFC reader + tag | yes | charge, USB drive, UF2 updates |

## Battery

Stock cell (M) as in the BOM, with the thin (S) and extended (L) options. *Reader* = 300 page turns a day; *festival* = 150 pages, a sync and the variant's extras. Estimates from [`../power/battery_model.py`](../power/README.md), which also lists depth and cell prices per option.

| Variant | S | M (stock) | L | Reader, M | Festival, M | Page turns per charge, M |
|---|---|---|---|---|---|---|
| SR-1 | 500 mAh, 10.5 mm | 1000 mAh, 12.5 mm | 1500 mAh, 13.5 mm | 8 mo | 9 mo | 115,254 |
| SR-1C | 500 mAh, 11 mm | 1000 mAh, 13 mm | 1500 mAh, 14 mm | 9 mo | 10 mo | 125,926 |
| SR-2 | 1000 mAh, 18.5 mm | 1500 mAh, 19.5 mm | 2500 mAh, 23.5 mm | 11 mo | 1 yr+ | 154,545 |
| SR-3 | 500 mAh, 11.5 mm | 1200 mAh, 14.5 mm | 2000 mAh, 18.5 mm | 4 mo | 5 mo | 42,207 |
| SR-4 | 1200 mAh, 13.5 mm | 2000 mAh, 17.5 mm | 2500 mAh, 17.5 mm | 5 mo | 7 mo | 49,157 |
| SR-5 | 1000 mAh, 20.5 mm | 1200 mAh, 21.5 mm | 2000 mAh, 25.5 mm | 10 mo | 11 mo | 151,111 |
| SR-6 | 1000 mAh, 13.5 mm | 1500 mAh, 14.5 mm | 2500 mAh, 18.5 mm | 1 yr+ | 31 d | 188,889 |
| SR-7 | 1000 mAh, 18.5 mm | 1500 mAh, 19.5 mm | 2500 mAh, 23.5 mm | 2 mo | 4 mo | 24,878 |
| SR-8 | 500 mAh, 11 mm | 1000 mAh, 13 mm | 1500 mAh, 14 mm | 1 yr+ | 1 yr+ | 382,022 |
| SR-9 | 1000 mAh, 13.5 mm | 1200 mAh, 14.5 mm | 2000 mAh, 18.5 mm | 10 mo | 11 mo | 151,111 |
| SR-10 | 250 mAh, 7.5 mm | 350 mAh, 8.5 mm | 550 mAh, 9.5 mm | 4 mo | 4 mo | 48,571 |
| SR-11 | 500 mAh, 9 mm | 1000 mAh, 11 mm | 1200 mAh, 12 mm | 9 mo | 10 mo | 116,571 |
| SR-12 | 500 mAh, 9 mm | 1200 mAh, 12 mm | 1500 mAh, 12 mm | 10 mo | 11 mo | 139,886 |
| SR-13 | 1000 mAh, 13.5 mm | 1500 mAh, 14.5 mm | 2500 mAh, 18.5 mm | 4 mo | 6 mo | 52,759 |
| SR-14W | 500 mAh, 9.5 mm | 1200 mAh, 12.5 mm | 2000 mAh, 16.5 mm | 10 mo | 8 mo | 139,886 |
| SR-14B | 500 mAh, 9.5 mm | 1200 mAh, 12.5 mm | 2000 mAh, 16.5 mm | 1 yr+ | 1 yr+ | 368,675 |

## SR-1 Laminate: XIAO ESP32-S3 Plus on the Seeed EE05 board + 4.2in, no custom PCB

One ready-made board carries the panel driver, three keys, battery socket and switch; add the panel, a cell and the shell. The C5 version with dual-band Wi-Fi is SR-1C.

**86 x 138 x 12.5 mm · ~95 g · Wi-Fi 4 2.4 GHz, BLE 5 · microSD: no · USB-C: charge, USB drive, console**

- Keys on the EE05: D1 (GPIO2), D2 (GPIO3), D9 (GPIO8), 10k pull-ups + 100 nF. All three are RTC GPIOs on the S3, so any key wakes it from deep sleep; no wake diodes.
- Panel 3.3 V comes through a TPS22916 load switch (PWR_EN on D6) and the battery divider through another (ADC_EN on D12, BAT_ADC on D0): both off in sleep.
- The slide switch drives a P-MOSFET in the battery lead, and plugging in USB turns it on, so it charges with the switch off. Charger is the XIAO's SGM40567.
- 2.4 GHz only (the price of the EE05); the S3 gains a USB drive over USB-C. Measure deep-sleep current on the bench.
- No microSD: books live in 16 MB of flash.

Files: [`SR-1.csv`](SR-1.csv) · [`SR-1.digikey-import.csv`](SR-1.digikey-import.csv) · [`SR-1.mouser-import.csv`](SR-1.mouser-import.csv)

## SR-1C Laminate C5: DFRobot FireBeetle 2 ESP32-C5 + 4.2in, no custom PCB

The SR-1 alternative that keeps dual-band Wi-Fi: a FireBeetle 2 C5 (charger, PH2.0 battery socket, switched 3.3 V rail) wired to a 24-pin e-paper adapter and three keys.

**86 x 138 x 13 mm · ~100 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5 · microSD: no · USB-C: charge, console, Web Serial loader**

- Panel wiring (the GDI pins, so DFRobot's own SPI map holds): SCK 23, MOSI 24, CS 27 (D6), DC 8 (D2), RST 26 (D3), BUSY 6 (D12). Adapter 3.3 V from 3V3_C, which GPIO0 switches (off by default), so the panel is unpowered in sleep.
- Battery voltage on GPIO1 (on-board divider). Keep GPIO2 free: strapping/JTAG.
- The board is 25.4 x 60 mm, longer than a XIAO stack; it lies beside the cell in SR-1's 86 mm-wide shell. Check the shell layout before printing.
- 4 MB flash, no PSRAM: fine for 1-bit 400x300 pages (15 KB a frame); no microSD.

Files: [`SR-1C.csv`](SR-1C.csv) · [`SR-1C.digikey-import.csv`](SR-1C.digikey-import.csv) · [`SR-1C.mouser-import.csv`](SR-1C.mouser-import.csv)

## SR-2 All Access: ESP32-S3 custom PCB + 5.83in, card pocket

The flagship lanyard pass: 648x480 page, 5-way + page keys, microSD, ID-card pocket with a window on the back.

**109 x 177 x 19.5 (15.5 without pocket) mm · ~150 g · Wi-Fi 4 2.4 GHz, BLE 5 · microSD: yes · USB-C: charge, USB drive, console**

- Over $50 at 5 builds because of the panel. At 100 units, with Good Display direct pricing and LCSC passives, it lands under $40.
- Keep the microSD socket: with the S3's USB OTG the card shows up as a USB drive on any computer, which is the no-phone way to load books.

Files: [`SR-2.csv`](SR-2.csv) · [`SR-2.digikey-import.csv`](SR-2.digikey-import.csv) · [`SR-2.mouser-import.csv`](SR-2.mouser-import.csv)

## SR-3 Gray Pass: M5Stack PaperDIY S3 kit + printed shell

Fastest route to a 16-grey, 960x540 page: M5Stack's PaperDIY Assembly Kit (ESP32-S3, 4.7in ED047TC1, microSD, USB-C) in our enclosure with a real battery. No PCB work.

**78 x 149 x 14.5 mm · ~110 g · Wi-Fi 4 2.4 GHz, BLE 5 · microSD: yes (on the kit) · USB-C: charge, USB drive, console**

- Replaces the LilyGo T5 4.7in S3 ($39.99, often sold out), which stays the fallback; same ED047TC1 glass, so firmware is shared.
- Check the kit's battery connector and charger current before fitting a 1200 mAh cell.
- Parallel panels draw more per refresh than SPI ones; still weeks of reading per charge.

Files: [`SR-3.csv`](SR-3.csv) · [`SR-3.digikey-import.csv`](SR-3.digikey-import.csv) · [`SR-3.mouser-import.csv`](SR-3.mouser-import.csv)

## SR-4 Headliner: 6in 300 ppi, epdiy-style ESP32-S3 board, card pocket

Kindle-class 1448x1072 page from a 6in parallel panel, driven by an open epdiy-derived board. Side page rockers for either hand, card pocket. The stretch variant.

**109 x 172 x 17.5 (13.5 without pocket) mm · ~150 g · Wi-Fi 4 2.4 GHz, BLE 5 · microSD: yes · USB-C: charge, USB drive, console**

- Over the $50 line at prototype quantity and around $47 at 100: only pick this if the 300 ppi page is the product.
- The epdiy V7 board on Tindie (~70 EUR) is the bench shortcut, but its seller warns it does not suit 6in panels; build our own from the epdiy schematics.

Files: [`SR-4.csv`](SR-4.csv) · [`SR-4.digikey-import.csv`](SR-4.digikey-import.csv) · [`SR-4.mouser-import.csv`](SR-4.mouser-import.csv)

## SR-5 Crew Clip: ESP32-C5 custom PCB + 4.2in landscape, belt clip

Cheapest at volume. Landscape 4.2in on a belt clip, thumbwheel to scroll, dual-band Wi-Fi so sync works in a crowded venue.

**104 x 109 x 21.5 with clip (16 body) mm · ~110 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5 · microSD: yes · USB-C: charge, console, Web Serial loader**

- Same panel and chip as SR-1C, so SR-1C is the bench prototype for SR-5.
- The C5 cannot be a USB drive, so the microSD card is the mass loader: pull it, copy packs on a computer, put it back.

Files: [`SR-5.csv`](SR-5.csv) · [`SR-5.digikey-import.csv`](SR-5.digikey-import.csv) · [`SR-5.mouser-import.csv`](SR-5.mouser-import.csv)

## SR-6 Night Shift: ESP32-C5 + 4.2in with front light

The lit one. E-paper has no backlight; this uses Good Display's front-light version of the 4.2in panel (a light guide over the glass, LEDs on the edge) with a LIGHT key.

**88 x 138 x 14.5 mm · ~105 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5 · microSD: yes · USB-C: charge, console, Web Serial loader**

- Light on costs roughly 15-40 mA depending on level: about 40-90 hours of lit reading from 1500 mAh. Light off, it behaves like SR-1.
- Light switches off by itself after 2 minutes without a key press.

Files: [`SR-6.csv`](SR-6.csv) · [`SR-6.digikey-import.csv`](SR-6.digikey-import.csv) · [`SR-6.mouser-import.csv`](SR-6.mouser-import.csv)

## SR-7 Poster: SR-2 board + 5.83in 4-colour panel, zine sleeve

Black, white, red and yellow on the SR-2 board. Colour is for covers, flyers and the badge; refresh is slow, so stories stay black and white. The back sleeve holds a folded A7 paper mini-zine.

**109 x 177 x 19.5 (15.5 without sleeve) mm · ~150 g · Wi-Fi 4 2.4 GHz, BLE 5 · microSD: yes · USB-C: charge, USB drive, console**

- Spectra 6 (full colour) is the alternative glass; M5Stack's PaperColor ($75, 4in 400x600) is a ready-made way to try it, but over budget.
- Same board as SR-2, so one PCB order covers both.

Files: [`SR-7.csv`](SR-7.csv) · [`SR-7.digikey-import.csv`](SR-7.digikey-import.csv) · [`SR-7.mouser-import.csv`](SR-7.mouser-import.csv)

## SR-8 Quiet Pass: XIAO nRF52840 Plus on the Seeed EE05 board, no Wi-Fi, 4.2in

No Wi-Fi radio at all: Bluetooth LE and USB only. Lowest sleep current of the set, welcome where radios are not, and a real USB drive over USB-C.

**86 x 138 x 13 mm · ~95 g · BLE 5 only · microSD: yes · USB-C: charge, USB drive, UF2 updates**

- BLE-only sync is slow for books (tens of KB/s), so the USB drive is the main loader and the app sends zines and positions over BLE.
- UF2 bootloader: firmware updates by dragging a file onto the drive.
- Same EE05 board as SR-1: keys, battery socket, switch and panel load switch on board. Seeed documents the EE05 with the nRF52840 Plus (it adds NFC pads on D7/D14 for that XIAO).

Files: [`SR-8.csv`](SR-8.csv) · [`SR-8.digikey-import.csv`](SR-8.digikey-import.csv) · [`SR-8.mouser-import.csv`](SR-8.mouser-import.csv)

## SR-9 Roadie: ESP32-C5 + 4.2in with a PSP-style thumbstick

Analog thumbstick at the left thumb, OK and BACK at the right. The stick pans a zoomed zine page (pages stored at 2x), scrolls menus fast, and moves page by page with a flick.

**88 x 149 x 14.5 mm · ~105 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5 · microSD: yes · USB-C: charge, console, Web Serial loader**

- The 444 has four pads and no mounting holes: Adafruit's advice is wires and glue. The enclosure gets a printed cradle that clamps it from behind the bezel, and a TPU dust skirt around the cap.
- No click on the 444, so OK is a separate key. Use Adafruit 512 or 3246 if a pressable stick is wanted.
- Read both axes with the C5's ADC only while awake; dead zone + calibration at boot.

Files: [`SR-9.csv`](SR-9.csv) · [`SR-9.digikey-import.csv`](SR-9.digikey-import.csv) · [`SR-9.mouser-import.csv`](SR-9.mouser-import.csv)

## SR-10 Credential: 85 x 55 mm card (CR80), 2.9in

The size of an ID card or a 3.5 x 2 in business-card pass, 8.5 mm thick, with a slot punched at the top like a real card badge. A credential and schedule screen first: 296x128 shows about eight lines, so it is for badges, set times and short notes, not books.

**85 x 55 x 8.5 mm · ~40 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5 · microSD: yes · USB-C: charge, console, Web Serial loader**

- Not a book reader. Pair it with a bigger pass, or use it as the credential that also carries tonight's running order and a zine's cover.
- Thinness is the hard part: 8.5 mm leaves ~4 mm for module and cell side by side.

Files: [`SR-10.csv`](SR-10.csv) · [`SR-10.digikey-import.csv`](SR-10.digikey-import.csv) · [`SR-10.mouser-import.csv`](SR-10.mouser-import.csv)

## SR-11 Full Bleed: 3 x 4 in (76 x 102 mm), 3.97in 800x480

The whole face is screen. The 3.97in panel fills a 3 x 4 in pass, so the pass graphic is drawn by the firmware at the top of the page instead of printed on a band, and the lanyard hooks a fixed D-ring on the back. All keys are on the edges.

**76.2 x 101.6 x 11 (+5 D-ring) mm · ~70 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5 · microSD: yes · USB-C: charge, console, Web Serial loader**

- Tight: the panel outline is 96.6 mm inside a 101.6 mm pass, so the bezel is 2.5 mm top and bottom. Check the FPC bend room before the first print.
- 800x480 at 235 ppi: the sharpest text of any SPI variant, for the price of a 4.2in.

Files: [`SR-11.csv`](SR-11.csv) · [`SR-11.digikey-import.csv`](SR-11.digikey-import.csv) · [`SR-11.mouser-import.csv`](SR-11.mouser-import.csv)

## SR-12 Tour Laminate: 80 x 145 mm, 3.97in 800x480

A classic 80 x 145 mm laminate: band, 3.97in 235 ppi page, 5-way and page keys. The everyday reader at a sub-4 in width.

**80 x 145 x 12 mm · ~90 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5 · microSD: yes · USB-C: charge, console, Web Serial loader**

- SR-3 (78 x 149) is within 4 mm of this size; trimming its key zone makes it 80 x 145 too.

Files: [`SR-12.csv`](SR-12.csv) · [`SR-12.digikey-import.csv`](SR-12.digikey-import.csv) · [`SR-12.mouser-import.csv`](SR-12.mouser-import.csv)

## SR-13 Hybrid Credential: 3.69 x 8.49 in (94 x 216 mm), 4.7in + ID card window

The long tour laminate. A 4.7in 16-grey reader on top and, below the keys, a clear window holding the venue's printed ID-1 credential (photo, hologram, barcode), which slides in from the left edge. One pass instead of two on the lanyard.

**94 x 216 x 14.5 mm · ~165 g · Wi-Fi 4 2.4 GHz, BLE 5 · microSD: yes (on the kit) · USB-C: charge, USB drive, console**

- The credential stays paper so venue staff can check it the way they always do; the reader never pretends to be the credential.
- 216 mm is long: the lanyard ring is at the top, so it hangs from chest to belt.

Files: [`SR-13.csv`](SR-13.csv) · [`SR-13.digikey-import.csv`](SR-13.digikey-import.csv) · [`SR-13.mouser-import.csv`](SR-13.mouser-import.csv)

## SR-14 Tap Pass, Wi-Fi build: ESP32-C5 + NFC reader, 80 x 155 mm

Tap the back of the pass on a tag on a poster, flyer, record sleeve or merch table. A tag holds either a link to the zine on Subverse or a small zine itself. The Wi-Fi build fetches linked zines on its own (known network or the phone's hotspot, either band) and falls back to the phone over BLE when there is no Wi-Fi.

**80 x 155 x 12.5 mm · ~95 g · Wi-Fi 6 2.4 + 5 GHz, BLE 5, NFC reader + tag · microSD: yes · USB-C: charge, console, Web Serial loader**

- Fetching over Wi-Fi needs packs rendered for the pass's panel by a server (Subverse or an open pack-render service), because the pass cannot render PDFs itself. Without that, linked zines wait for the phone.
- Reader is powered only while scanning (TAP key, 10 s); the ST25DV answers phones with no power from the pass.

Files: [`SR-14W.csv`](SR-14W.csv) · [`SR-14W.digikey-import.csv`](SR-14W.digikey-import.csv) · [`SR-14W.mouser-import.csv`](SR-14W.mouser-import.csv)

## SR-14 Tap Pass, tethered build: nRF52840 + NFC reader, 80 x 155 mm

Same pass, no Wi-Fi. A tapped link goes over BLE to the phone app, which downloads the zine, renders it for the panel and sends the pages back. Small zines stored on the tag show at once, phone or not.

**80 x 155 x 12.5 mm · ~95 g · BLE 5 only, NFC reader + tag · microSD: yes · USB-C: charge, USB drive, UF2 updates**

- Cheaper to run, not to build: the nRF module costs more than the C5, but there is no dynamic-tag chip and sleep current is the lowest of the set.
- USB-C is a USB drive here (nRF native USB), so it also loads packs with no phone.

Files: [`SR-14B.csv`](SR-14B.csv) · [`SR-14B.digikey-import.csv`](SR-14B.digikey-import.csv) · [`SR-14B.mouser-import.csv`](SR-14B.mouser-import.csv)
