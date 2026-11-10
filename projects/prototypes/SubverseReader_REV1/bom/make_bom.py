#!/usr/bin/env python3
"""Generate the SubverseReader REV1 BOMs.

    python3 make_bom.py

Writes, for each variant, a full BOM CSV and import files for the DigiKey and
Mouser BOM tools, and regenerates README.md with the totals.

Prices are snapshots from 2026-11-06, from search results only (no distributor
page was opened). Every line carries a status:
  vendor      price seen on a seller's page in a search result
  estimate    no live figure; typical small-quantity price
unit_usd is the price at prototype quantity (a batch of BUILDS units);
unit_usd_100 is an estimate at 100 units (LCSC/AliExpress/factory direct).
Re-check every line in the cart before ordering.
"""
import csv
import os

HERE = os.path.dirname(os.path.abspath(__file__))
BUILDS = 5  # a homebrew batch: PCB fabs ship 5 boards minimum

VARIANTS = {}


def variant(key, title, summary, lines, notes=(), **meta):
    """meta: dims (mm, W x H x D), grams (estimate), radio, sd, usb (what USB-C does)."""
    VARIANTS[key] = dict(title=title, summary=summary, lines=lines, notes=list(notes), **meta)


def L(ref, desc, mfr, mpn, dk, mo, other, unit, unit100, qty, status, note=""):
    return (ref, desc, mfr, mpn, dk, mo, other, unit, unit100, qty, status, note)


# ---------------------------------------------------------------- shared blocks
POWER = [  # custom-PCB power path: USB-C in, LiPo charge, low-Iq 3.3 V
    L("J1", "USB-C receptacle, 16-pin, USB 2.0", "GCT", "USB4105-GF-A", "2073-USB4105-GF-ACT-ND",
      "640-USB4105-GF-A", "", 0.84, 0.50, 1, "estimate", "charge + USB serial"),
    L("U2", "LiPo charger 500 mA, SOT-23-5", "Microchip", "MCP73831T-2ACI/OT",
      "MCP73831T-2ACI/OTCT-ND", "579-MCP73831T2ACIOT", "", 0.75, 0.45, 1, "estimate",
      "TP4056 at LCSC is the $0.15 alternative"),
    L("U3", "LDO 3.3 V 600 mA, 2 uA Iq", "Richtek", "RT9080-33GJ5", "", "", "LCSC", 0.35, 0.15, 1,
      "estimate", "Iq is what sets sleep life; avoid AP2112 (55 uA)"),
    L("U4", "USB ESD protection", "ST", "USBLC6-2SC6", "497-5235-1-ND", "511-USBLC6-2SC6", "",
      0.40, 0.15, 1, "estimate", ""),
    L("Q1", "P-MOSFET load switch, panel/SD rail off in sleep", "Diodes", "DMG2305UX-7",
      "DMG2305UX-7DICT-ND", "621-DMG2305UX-7", "", 0.30, 0.10, 1, "estimate", ""),
    L("J2", "JST-PH 2-pin SMD battery header", "JST", "S2B-PH-SM4-TB", "455-1749-1-ND",
      "306-S2BPHSM4TBLFSN", "", 0.30, 0.12, 1, "estimate", ""),
    L("RC", "Passives: 0402/0603 R and C, ~30 parts incl. battery divider", "Yageo/Samsung", "", "",
      "", "LCSC", 0.80, 0.25, 1, "estimate", "cut-tape minimums dominate at 5 builds"),
    L("SW", "Tact switch 3x4 mm (BOOT, RESET)", "Alps", "SKRPACE010", "", "688-SKRPAC", "", 0.22,
      0.08, 2, "estimate", ""),
]

SPI_EPD = [  # the Good Display DESPI-C02 reference front end for 24-pin SPI panels
    L("J3", "FPC 24-pin 0.5 mm, bottom contact", "Hirose", "FH12-24S-0.5SH(55)", "HFJ124CT-ND",
      "798-FH12-24S-0.5SH55", "", 1.10, 0.20, 1, "estimate", "generic at LCSC for volume"),
    L("EPB", "EPD boost: Si1308EDL MOSFET, 3x MBR0530, 10 uH, 1 uF/50 V caps", "Vishay/onsemi",
      "", "", "", "", 1.20, 0.45, 1, "estimate", "copy the DESPI-C02 schematic"),
]

ENCLOSURE = [
    L("ENC", "Enclosure, 3D-printed PETG, front + back shell (~45 g)", "", "", "", "", "own printer",
      0.90, 0.60, 1, "estimate", "STEP/STL from enclosure/"),
    L("CAP", "Key caps, printed TPU", "", "", "", "", "own printer", 0.10, 0.05, 1, "estimate", ""),
    L("LAM", "Laminate band: printed vinyl sticker", "", "", "", "", "sticker shop", 0.15, 0.05, 1,
      "estimate", "swappable 'pass' graphic"),
    L("HW", "M2x6 screws + M2 heat-set inserts, 4 each", "", "", "", "", "Amazon/Ali", 0.40, 0.20, 1,
      "estimate", ""),
]
LANYARD = L("LNY", "Lanyard 20 mm with swivel hook + 25 mm split ring", "", "", "", "",
            "Amazon (multi-pack); AliExpress for 100", 1.50, 0.55, 1, "estimate", "")
POCKET_ENC = L("ENC", "Enclosure, PETG, front + back shell + card pocket (~70 g)", "", "", "", "",
               "own printer", 1.40, 0.90, 1, "estimate", "pocket fits ID-1 card + ticket")


AMAZON_CELL = {350: 5.00, 1000: 6.50, 1200: 7.00, 1500: 8.00, 2000: 9.50}  # @5: multi-packs, 2026-11


def bat(mah, size, unit, unit100):
    """unit (the old AliExpress figure) is replaced by the Amazon multi-pack price; unit100 is Alibaba."""
    return L("BAT", f"LiPo {mah} mAh {size}, PCM, JST-PH 2.0 lead", "", "", "", "",
             "Amazon / hobby shop (Adafruit, SparkFun); Alibaba for 100",
             AMAZON_CELL.get(mah, unit), unit100, 1, "estimate",
             "protection board fitted; check JST polarity against the board before plugging in")


def LATCH(keys):
    """Soft-latch power + any-key wake: every key turns the pass on from off and wakes it
    from deep sleep. Each key gets a BAT54C (common cathode on the key): one anode to the
    latch gate (VBAT side), one to the ANY-key wake line on a single LP/RTC GPIO."""
    return [
        L("Q5", "P-MOSFET soft latch: any key pulls the gate low, drives LDO EN", "Diodes",
          "DMG2305UX-7", "DMG2305UX-7DICT-ND", "621-DMG2305UX-7", "", 0.30, 0.10, 1, "estimate",
          "off draw = cell protection IC only (~3 uA)"),
        L("Q6", "N-MOSFET HOLD: firmware keeps itself on, releases to switch off", "onsemi",
          "2N7002LT1G", "2N7002LT1GOSCT-ND", "863-2N7002LT1G", "", 0.12, 0.02, 1, "estimate", ""),
        L("D1", f"Dual Schottky, common cathode, one per key ({keys} keys)", "Nexperia",
          "BAT54C,215", "1727-BAT54C215CT-ND", "771-BAT54C-T/R", "", 0.12, 0.03, keys, "estimate",
          "latch gate + ANY-key wake line; VBUS gets one too so plugging in powers on"),
    ]


GDEY042 = L("EPD", "4.2in e-paper 400x300 B/W, SSD1683, 4-grey, fast/partial refresh", "Good Display",
            "GDEY042T81", "", "", "buyepaper.com", 11.60, 10.00, 1, "vendor",
            "buyepaper $11.60, 60 in stock 2026-11-07; Orient AES400300B00 $16.58 alt")

SD_SOCKET = L("J4", "microSD socket, push-push", "Hirose", "DM3AT-SF-PEJM5", "HR1941CT-ND",
              "798-DM3AT-SF-PEJM5", "", 1.95, 1.20, 1, "estimate", "")

# ---------------------------------------------------------------- variants
EE05 = L("DRV", "XIAO ePaper Display Board EE05: 24-pin SPI FPC + boost, 3 keys, JST 2.0 battery "
           "socket with slide switch, reset, panel and battery-sense load switches", "Seeed", "", "", "",
           "Seeed 100057220", 10.99, 9.99, 1, "vendor",
           "XIAO is SMD-soldered incl. back pads D11-D19 (panel RES on D11): hot air or reflow, "
           "or buy it assembled; takes the S3 Plus or nRF52840 Plus only, not C3/C5")

variant("SR-1", "SR-1 Laminate: XIAO ESP32-S3 Plus on the Seeed EE05 board + 4.2in, no custom PCB",
        "One ready-made board carries the panel driver, three keys, battery socket and switch; "
        "add the panel, a cell and the shell. The C5 version with dual-band Wi-Fi is SR-1C.",
        [L("MCU", "XIAO ESP32-S3 Plus (16 MB flash, 8 MB PSRAM, Wi-Fi 4 2.4 GHz, BLE 5, USB OTG, "
           "charger on board)", "Seeed", "", "", "", "Seeed 102010671", 7.90, 7.90, 1, "vendor",
           "Seeed $7.90, in stock 2026-11-09"),
         EE05,
         GDEY042,
         bat(1000, "503450", 4.20, 2.60),
         *ENCLOSURE, LANYARD],
        ["Keys on the EE05: D1 (GPIO2), D2 (GPIO3), D9 (GPIO8), 10k pull-ups + 100 nF. All three "
         "are RTC GPIOs on the S3, so any key wakes it from deep sleep; no wake diodes.",
         "Panel 3.3 V comes through a TPS22916 load switch (PWR_EN on D6) and the battery divider "
         "through another (ADC_EN on D12, BAT_ADC on D0): both off in sleep.",
         "The slide switch drives a P-MOSFET in the battery lead, and plugging in USB turns it on, "
         "so it charges with the switch off. Charger is the XIAO's SGM40567.",
         "2.4 GHz only (the price of the EE05); the S3 gains a USB drive over USB-C. "
         "Measure deep-sleep current on the bench.",
         "No microSD: books live in 16 MB of flash."],
        dims="86 x 138 x 12.5", grams=95, radio="Wi-Fi 4 2.4 GHz, BLE 5",
        sd="no", usb="charge, USB drive, console")

variant("SR-1C", "SR-1C Laminate C5: DFRobot FireBeetle 2 ESP32-C5 + 4.2in, no custom PCB",
        "The SR-1 alternative that keeps dual-band Wi-Fi: a FireBeetle 2 C5 (charger, PH2.0 "
        "battery socket, switched 3.3 V rail) wired to a 24-pin e-paper adapter and three keys.",
        [L("MCU", "DFRobot FireBeetle 2 ESP32-C5 (C5-WROOM-1-N4, Wi-Fi 6 2.4/5 GHz, BLE 5, "
           "0.5 A charger, PH2.0 battery socket, 21 uA deep sleep), 25.4 x 60 mm", "DFRobot",
           "DFR1222", "1738-DFR1222-ND", "", "", 8.62, 7.90, 1, "vendor",
           "DigiKey 148 in stock 2026-11-09; DFRobot direct $7.90"),
         L("DRV", "Crystalfontz CFA-10084 e-paper adapter: 24-pin ZIF, boost, RESE jumper (open 3 ohm / "
           "shorted 0.47 ohm), 8-pin header (3.3V GND SCK MOSI CS D/C RST BUSY), 3.3-5.5 V", "Crystalfontz",
           "CFA-10084", "", "", "Crystalfontz", 11.68, 11.68, 1, "vendor",
           "leave RESE open (3 ohm, the default); US stock. Good Display DESPI-C02 (2.2 ohm) is the China-only exact match"),
         GDEY042,
         L("KEY", "Tact switch 6x6 mm THT (PREV, MENU, NEXT)", "Omron", "B3F-1000",
           "SW400-ND", "653-B3F-1000", "", 0.35, 0.12, 3, "estimate",
           "on GPIO3/4/5 (A2/A3/A4), all LP GPIOs: each wakes the chip, no diodes"),
         L("PWR", "Slide switch, battery disconnect (storage/shipping only)", "C&K", "JS102011SAQN",
           "401-2000-ND", "611-JS102011SAQN", "", 0.45, 0.20, 1, "estimate", ""),
         L("PRF", "Perfboard scrap + 30 AWG wire", "", "", "", "", "", 0.50, 0.20, 1, "estimate", ""),
         bat(1000, "503450", 4.20, 2.60),
         *ENCLOSURE, LANYARD],
        ["Panel wiring, FireBeetle -> CFA-10084 (the GDI pins, so DFRobot's own SPI map holds): "
         "SCK 23 -> SCK, MOSI 24 -> MOSI, CS 27 (D6) -> CS, DC 8 (D2) -> D/C, RST 26 (D3) -> RST, "
         "BUSY 6 (D12) -> BUSY (high while busy), 3V3_C -> 3.3V, GND -> GND. 3V3_C is switched by "
         "GPIO0 (off by default), so the panel is unpowered in sleep. GxEPD2 driver: GxEPD2_420_GDEY042T81.",
         "The panel datasheet's reference circuit (p. 31) uses a 2.2 ohm RESE resistor, as does the EE05. "
         "The CFA-10084 offers 3 ohm (jumper open, default) or 0.47 ohm: use 3 ohm, the nearest; if refresh "
         "is faint or fails, short the jumper. Panel faces the same way as the board's parts (datasheet).",
         "Battery voltage on GPIO1 (on-board divider). Keep GPIO2 free: strapping/JTAG.",
         "The board is 25.4 x 60 mm, longer than a XIAO stack; it lies beside the cell in SR-1's "
         "86 mm-wide shell. Check the shell layout before printing.",
         "4 MB flash, no PSRAM: fine for 1-bit 400x300 pages (15 KB a frame); no microSD."],
        dims="86 x 138 x 13", grams=100, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5",
        sd="no", usb="charge, console, Web Serial loader")

variant("SR-1D", "SR-1D Laminate Direct: all from China, shipped to Japan; XIAO ESP32-C5 + 4.2in",
        "The cheapest SR-1: one Seeed parcel (XIAO C5, driver board and the same 4.2in glass) plus "
        "one AliExpress order for keys, switch, cell and hardware. Hand-wired keys. Priced for "
        "delivery to Japan.",
        [L("MCU", "XIAO ESP32-C5 (Wi-Fi 6 2.4/5 GHz, BLE 5, charger on board)", "Seeed", "", "", "",
           "Seeed 100010048", 6.90, 6.50, 1, "vendor", "Seeed $6.90 (2026-11-09); Switch Science JP sold out"),
         L("DRV", "ePaper Driver Board for XIAO V2 (24-pin SPI, boost)", "Seeed", "", "", "",
           "Seeed 114993558", 5.99, 4.90, 1, "vendor",
           "RESE fixed at 0.47 ohm; Seeed lists this board for its own 4.2in GDEY042T81 panel"),
         L("EPD", "4.2in e-paper 400x300 B/W, SSD1683 (Seeed 104990857 = Good Display GDEY042T81)",
           "Seeed", "", "", "", "Seeed 104990857", 19.95, 10.00, 1, "vendor",
           "same parcel as the XIAO; buy-lcd sells the same glass at $13.80 but 0 in stock and ships separately"),
         L("KEY", "Tact switch 6x6 mm THT (PREV, MENU, NEXT), 50-pc pack", "", "", "", "", "AliExpress",
           0.05, 0.02, 3, "estimate", "pack ~$1.09; check the listing's normal price, not the new-buyer deal"),
         L("DIO", "Schottky diode 1N5819 / BAT43, any-key wake line", "", "", "", "", "AliExpress",
           0.03, 0.01, 3, "estimate", "Schottky, not 1N4148: the drop must stay under the C5's low threshold"),
         L("PWR", "Slide switch SS12D00, battery disconnect, 20-pc pack", "", "", "", "", "AliExpress",
           0.06, 0.03, 1, "estimate", ""),
         L("PRF", "Perfboard scrap + 30 AWG wire", "", "", "", "", "AliExpress", 0.20, 0.10, 1, "estimate", ""),
         L("BAT", "LiPo 1000 mAh 503450, PCM, JST-PH 2.0 lead", "", "", "", "",
           "AliExpress (check Japan delivery for cells); Akizuki Denshi fallback", 6.50, 2.60, 1,
           "estimate", "lithium cells often can't go by post to Japan: pick a courier option or buy locally"),
         L("ENC", "Enclosure, 3D-printed PETG, front + back shell (~45 g)", "", "", "", "", "own printer",
           0.90, 0.60, 1, "estimate", "STEP/STL from enclosure/"),
         L("CAP", "Key caps, printed TPU", "", "", "", "", "own printer", 0.10, 0.05, 1, "estimate", ""),
         L("LAM", "Laminate band: printed vinyl sticker", "", "", "", "", "sticker shop", 0.15, 0.05, 1,
           "estimate", "print locally"),
         L("HW", "M2x6 screws + M2 heat-set inserts, 4 each", "", "", "", "", "AliExpress", 0.20, 0.10, 1,
           "estimate", "packs of 50-100"),
         L("LNY", "Lanyard 20 mm with swivel hook + 25 mm split ring", "", "", "", "", "AliExpress", 0.60,
           0.43, 1, "estimate", "no plain stock listing checked; printed lots ~$0.43 each at 100")],
        ["Pins: the driver board takes D0 RST, D1 CS, D2 BUSY, D3 DC, D8 SCK, D10 MOSI. That uses both "
         "LP GPIOs on the edge (D0, D1), so the any-key wake line goes to an LP GPIO on the back JTAG "
         "pads (GPIO3, MTDI), through one Schottky per key; the keys also go to D4, D5 and D9 so the "
         "firmware can tell them apart.",
         "The driver board's boost resistor is 0.47 ohm, not the panel datasheet's 2.2; Seeed sells this "
         "pair together, so it should work. If refresh is faint, replace R2 with 2.2 ohm.",
         "Per-unit prices for AliExpress packs are per piece; you buy one pack for 5 builds.",
         "Japan: Seeed's OCS service is 4-7 working days (orders under $300); import consumption tax "
         "applies above the small-parcel threshold. Fees here are estimates; Seeed and AliExpress "
         "price shipping at checkout."],
        dims="86 x 138 x 12.5", grams=95, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5",
        sd="no", usb="charge, console, Web Serial loader")

variant("SR-2", "SR-2 All Access: ESP32-S3 custom PCB + 5.83in, card pocket",
        "The flagship lanyard pass: 648x480 page, 5-way + page keys, microSD, ID-card pocket "
        "with a window on the back.",
        [L("EPD", "5.83in e-paper 648x480 B/W, UC8179, fast/partial refresh", "Good Display",
           "GDEY0583T81", "", "", "buyepaper.com", 24.20, 20.00, 1, "vendor",
           "buyepaper $24.20, 93 in stock 2026-11-07; Seeed 104990858 is the same glass"),
         L("U1", "ESP32-S3-WROOM-1-N16R8 (16 MB flash, 8 MB PSRAM)", "Espressif",
           "ESP32-S3-WROOM-1-N16R8", "1965-ESP32-S3-WROOM-1-N16R8CT-ND", "356-ESP32S3WRM1N16R8",
           "", 4.95, 3.60, 1, "estimate", "N8R2 saves ~$0.80 if no PSRAM needed"),
         *POWER, *LATCH(7), *SPI_EPD,
         L("NAV", "5-way navigation switch", "Alps", "SKRHABE010", "", "688-SKRHAB", "", 1.60, 0.95,
           1, "estimate", ""),
         L("KEY", "Tact switch 6x3.5 mm SMD (PREV, NEXT)", "C&K", "PTS526 SM15 SMTR2 LFS",
           "CKN10504CT-ND", "611-PTS526SM15SMTR2L", "", 0.30, 0.10, 2, "estimate", ""),
         L("J4", "microSD socket, push-push", "Hirose", "DM3AT-SF-PEJM5", "HR1941CT-ND",
           "798-DM3AT-SF-PEJM5", "", 1.95, 1.20, 1, "estimate", "optional; 16 MB flash holds ~1500 pages"),
         L("PCB", "PCB 2-layer 1.0 mm, ~95x60 mm", "JLCPCB", "", "", "", "JLCPCB", 2.40, 0.70, 1,
           "estimate", "5 boards + shipping"),
         bat(1500, "604050", 5.40, 3.20),
         POCKET_ENC, *ENCLOSURE[1:], LANYARD],
        ["Over $50 at 5 builds because of the panel. At 100 units, with Good Display direct "
         "pricing and LCSC passives, it lands under $40.",
         "Keep the microSD socket: with the S3's USB OTG the card shows up as a USB drive on any "
         "computer, which is the no-phone way to load books."],
        dims="109 x 177 x 19.5 (15.5 without pocket)", grams=150, radio="Wi-Fi 4 2.4 GHz, BLE 5",
        sd="yes", usb="charge, USB drive, console")

variant("SR-3", "SR-3 Gray Pass: M5Stack PaperDIY S3 kit + printed shell",
        "Fastest route to a 16-grey, 960x540 page: M5Stack's PaperDIY Assembly Kit (ESP32-S3, "
        "4.7in ED047TC1, microSD, USB-C) in our enclosure with a real battery. No PCB work.",
        [L("BRD", "M5Stack PaperDIY Assembly Kit (ESP32-S3 N16R8, 4.7in 960x540 16-grey, microSD)",
           "M5Stack", "K159", "", "", "M5Stack store / AliExpress", 25.00, 22.00, 1, "vendor",
           "SOLD OUT 2026-11-07 (in stock 16-22 Oct only); fallback LilyGo T5 4.7in S3 $39.99"),
         L("KEY", "Tact switch 6x6 mm THT on flying leads (UP, OK, DOWN)", "Omron", "B3F-1000",
           "SW400-ND", "653-B3F-1000", "", 0.35, 0.12, 3, "estimate", "check free GPIOs on the kit"),
         L("PWR", "Slide switch, battery disconnect", "C&K", "JS102011SAQN", "401-2000-ND",
           "611-JS102011SAQN", "", 0.45, 0.20, 1, "estimate", ""),
         bat(1200, "603450", 4.80, 2.80),
         *ENCLOSURE, LANYARD],
        ["Replaces the LilyGo T5 4.7in S3 ($39.99, often sold out), which stays the fallback; "
         "same ED047TC1 glass, so firmware is shared.",
         "Check the kit's battery connector and charger current before fitting a 1200 mAh cell.",
         "Parallel panels draw more per refresh than SPI ones; still weeks of reading per charge."],
        dims="78 x 149 x 14.5", grams=110, radio="Wi-Fi 4 2.4 GHz, BLE 5", sd="yes (on the kit)",
        usb="charge, USB drive, console")

variant("SR-4", "SR-4 Headliner: 6in 300 ppi, epdiy-style ESP32-S3 board, card pocket",
        "Kindle-class 1448x1072 page from a 6in parallel panel, driven by an open epdiy-derived "
        "board. Side page rockers for either hand, card pocket. The stretch variant.",
        [L("EPD", "6in e-paper 1448x1072 16-grey, parallel 34-pin (Kindle PW3 class)", "E Ink",
           "ED060KC1", "", "", "AliExpress", 28.00, 22.00, 1, "estimate",
           "price unconfirmed: distributors quote only, one listing at $131; grade varies"),
         L("U1", "ESP32-S3-WROOM-1-N16R8", "Espressif", "ESP32-S3-WROOM-1-N16R8",
           "1965-ESP32-S3-WROOM-1-N16R8CT-ND", "356-ESP32S3WRM1N16R8", "", 4.95, 3.60, 1,
           "estimate", ""),
         L("U5", "E-paper PMIC (+15/-15/+22/-20 V rails, VCOM)", "TI", "TPS65185RGZR", "", "", "",
           4.80, 3.20, 1, "estimate", "OUT OF STOCK at TI 2026-11-07, no authorised listings"),
         L("U6", "I/O expander, I2C", "NXP", "PCA9555PW,118", "", "", "", 1.10, 0.60, 1, "estimate",
           "epdiy v7 reference"),
         L("J3", "FPC 34-pin 0.5 mm (ED060KC1)", "Hirose", "FH12-34S-0.5SH(55)", "", "", "", 1.20,
           0.40, 1, "estimate", "check pin 1 side against the panel"),
         L("PMX", "PMIC passives: 4.7 uH inductors, Schottky diodes, 4.7 uF/50 V caps", "", "", "",
           "", "LCSC", 1.00, 0.50, 1, "estimate", ""),
         *POWER, *LATCH(7),
         L("SIDE", "Side-actuated tact switch (page rockers, both edges)", "C&K",
           "PTS815 SJM 250 SMTR LFS", "", "", "", 0.30, 0.10, 4, "estimate",
           "two per rocker: up/down"),
         L("KEY", "Tact switch 3x4 mm (BACK, MENU, SYNC)", "Alps", "SKRPACE010", "", "688-SKRPAC",
           "", 0.22, 0.08, 3, "estimate", ""),
         L("J4", "microSD socket, push-push", "Hirose", "DM3AT-SF-PEJM5", "HR1941CT-ND",
           "798-DM3AT-SF-PEJM5", "", 1.95, 1.20, 1, "estimate", "1448x1072 4-bit pages are ~150 KB"),
         L("PCB", "PCB 4-layer 1.0 mm, ~100x50 mm", "JLCPCB", "", "", "", "JLCPCB", 4.50, 1.40, 1,
           "estimate", "4 layers for the parallel bus and HV rails"),
         bat(2000, "103450", 6.50, 3.80),
         POCKET_ENC, *ENCLOSURE[1:], LANYARD],
        ["Over the $50 line at prototype quantity and around $47 at 100: only pick this if the "
         "300 ppi page is the product.",
         "The epdiy V7 board on Tindie (~70 EUR) is the bench shortcut, but its seller warns it "
         "does not suit 6in panels; build our own from the epdiy schematics."],
        dims="109 x 172 x 17.5 (13.5 without pocket)", grams=150, radio="Wi-Fi 4 2.4 GHz, BLE 5",
        sd="yes", usb="charge, USB drive, console")

variant("SR-5", "SR-5 Crew Clip: ESP32-C5 custom PCB + 4.2in landscape, belt clip",
        "Cheapest at volume. Landscape 4.2in on a belt clip, thumbwheel to scroll, dual-band "
        "Wi-Fi so sync works in a crowded venue.",
        [GDEY042,
         L("U1", "ESP32-C5-WROOM-1 module (N8R8)", "Espressif", "ESP32-C5-WROOM-1-N8R8", "", "", "",
           4.50, 3.40, 1, "estimate", "C5 modules are new; confirm stock first"),
         *POWER, *LATCH(3), *SPI_EPD,
         L("ENC2", "Roller encoder with push, side mount (thumbwheel)", "Alps", "EC10E1220501", "", "",
           "", 1.40, 0.80, 1, "estimate", "any 10 mm side roller with detents fits the cut-out"),
         L("KEY", "Tact switch 6x3.5 mm SMD (BACK, OK)", "C&K", "PTS526 SM15 SMTR2 LFS",
           "CKN10504CT-ND", "611-PTS526SM15SMTR2L", "", 0.30, 0.10, 2, "estimate", ""),
         SD_SOCKET,
         L("PCB", "PCB 2-layer 1.0 mm, ~90x70 mm", "JLCPCB", "", "", "", "JLCPCB", 2.40, 0.70, 1,
           "estimate", ""),
         bat(1200, "603450", 4.80, 2.80),
         L("CLP", "Spring-steel belt clip 60 mm, screw mount", "", "", "", "", "Amazon (multi-pack); AliExpress for 100", 1.50,
           0.40, 1, "estimate", ""),
         *ENCLOSURE],
        ["Same panel and chip as SR-1C, so SR-1C is the bench prototype for SR-5.",
         "The C5 cannot be a USB drive, so the microSD card is the mass loader: pull it, copy "
         "packs on a computer, put it back."],
        dims="104 x 109 x 21.5 with clip (16 body)", grams=110, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5",
        sd="yes", usb="charge, console, Web Serial loader")

variant("SR-6", "SR-6 Night Shift: ESP32-C5 + 4.2in with front light",
        "The lit one. E-paper has no backlight; this uses Good Display's front-light version of "
        "the 4.2in panel (a light guide over the glass, LEDs on the edge) with a LIGHT key.",
        [L("EPD", "4.2in e-paper 400x300 B/W with front light, SSD1683", "Good Display",
           "GDEY042T81-FL02", "", "", "buyepaper.com", 19.23, 16.00, 1, "vendor",
           "buyepaper $19.23, 58 in stock 2026-11-07"),
         L("U1", "ESP32-C5-WROOM-1 module (N8R8)", "Espressif", "ESP32-C5-WROOM-1-N8R8", "", "", "",
           4.50, 3.40, 1, "estimate", "confirm stock"),
         *POWER, *LATCH(4), *SPI_EPD,
         L("LED", "Front-light LED driver: constant-current boost, PWM dimming", "TI", "TPS61169DCKR",
           "", "", "", 0.90, 0.40, 1, "estimate",
           "check FL02 LED count/Vf; a PWM'd MOSFET + resistor is the cheap fallback"),
         L("KEY", "Tact switch 6x3.5 mm SMD (PREV, MENU, LIGHT, NEXT)", "C&K",
           "PTS526 SM15 SMTR2 LFS", "CKN10504CT-ND", "611-PTS526SM15SMTR2L", "", 0.30, 0.10, 4,
           "estimate", ""),
         SD_SOCKET,
         L("PCB", "PCB 2-layer 1.0 mm, ~80x60 mm", "JLCPCB", "", "", "", "JLCPCB", 2.40, 0.70, 1,
           "estimate", ""),
         bat(1500, "604050", 5.40, 3.20),
         *ENCLOSURE, LANYARD],
        ["Light on costs roughly 15-40 mA depending on level: about 40-90 hours of lit reading "
         "from 1500 mAh. Light off, it behaves like SR-1.",
         "Light switches off by itself after 2 minutes without a key press."],
        dims="88 x 138 x 14.5", grams=105, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5", sd="yes",
        usb="charge, console, Web Serial loader")

variant("SR-7", "SR-7 Poster: SR-2 board + 5.83in 4-colour panel, zine sleeve",
        "Black, white, red and yellow on the SR-2 board. Colour is for covers, flyers and the "
        "badge; refresh is slow, so stories stay black and white. The back sleeve holds a "
        "folded A7 paper mini-zine.",
        [L("EPD", "5.83in e-paper 648x480, 4-colour B/W/R/Y", "Good Display", "GDEY0583F41", "", "",
           "buyepaper / LaskaKit", 24.20, 20.00, 1, "vendor",
           "buyepaper $24.20, 59 in stock 2026-11-07; ~12 s refresh, no partial"),
         L("U1", "ESP32-S3-WROOM-1-N16R8", "Espressif", "ESP32-S3-WROOM-1-N16R8",
           "1965-ESP32-S3-WROOM-1-N16R8CT-ND", "356-ESP32S3WRM1N16R8", "", 4.95, 3.60, 1,
           "estimate", "same PCB as SR-2"),
         *POWER, *LATCH(7), *SPI_EPD,
         L("NAV", "5-way navigation switch", "Alps", "SKRHABE010", "", "688-SKRHAB", "", 1.60, 0.95,
           1, "estimate", ""),
         L("KEY", "Tact switch 6x3.5 mm SMD (PREV, NEXT)", "C&K", "PTS526 SM15 SMTR2 LFS",
           "CKN10504CT-ND", "611-PTS526SM15SMTR2L", "", 0.30, 0.10, 2, "estimate", ""),
         SD_SOCKET,
         L("PCB", "PCB 2-layer 1.0 mm, ~95x60 mm (SR-2 board)", "JLCPCB", "", "", "", "JLCPCB",
           2.40, 0.70, 1, "estimate", "order with SR-2"),
         bat(1500, "604050", 5.40, 3.20),
         L("ENC", "Enclosure, PETG, front + back shell + A7 zine sleeve (~75 g)", "", "", "", "",
           "own printer", 1.50, 0.95, 1, "estimate", "sleeve fits 74 x 105 mm folded paper"),
         *ENCLOSURE[1:], LANYARD],
        ["Spectra 6 (full colour) is the alternative glass; M5Stack's PaperColor ($75, 4in "
         "400x600) is a ready-made way to try it, but over budget.",
         "Same board as SR-2, so one PCB order covers both."],
        dims="109 x 177 x 19.5 (15.5 without sleeve)", grams=150, radio="Wi-Fi 4 2.4 GHz, BLE 5",
        sd="yes", usb="charge, USB drive, console")

variant("SR-8", "SR-8 Quiet Pass: XIAO nRF52840 Plus on the Seeed EE05 board, no Wi-Fi, 4.2in",
        "No Wi-Fi radio at all: Bluetooth LE and USB only. Lowest sleep current of the set, "
        "welcome where radios are not, and a real USB drive over USB-C.",
        [L("MCU", "XIAO nRF52840 Plus (BLE 5, native USB, charger on board)", "Seeed", "", "", "",
           "Seeed 102010672", 10.40, 8.99, 1, "vendor", "custom-board path: Raytac MDBT50Q-1MV2 module"),
         EE05,
         GDEY042,
         L("SDB", "microSD breakout, SPI", "Adafruit", "4682", "", "", "Adafruit", 3.50, 1.50, 1,
           "estimate", "wired to the EE05's SPI1 pads (D17 SCK, D18 MISO, D19 MOSI; headers not fitted); appears as a USB drive"),
         L("Q2", "P-MOSFET: microSD power off in sleep (an idle card draws 100-250 uA)", "Diodes",
           "DMG2305UX-7", "DMG2305UX-7DICT-ND", "621-DMG2305UX-7", "", 0.30, 0.10, 1, "estimate",
           "the breakout has no switch of its own"),
         L("PRF", "Perfboard scrap + 30 AWG wire", "", "", "", "", "", 0.50, 0.20, 1, "estimate", ""),
         bat(1000, "503450", 4.20, 2.60),
         *ENCLOSURE, LANYARD],
        ["BLE-only sync is slow for books (tens of KB/s), so the USB drive is the main loader "
         "and the app sends zines and positions over BLE.",
         "UF2 bootloader: firmware updates by dragging a file onto the drive.",
         "Same EE05 board as SR-1: keys, battery socket, switch and panel load switch on board. "
         "Seeed documents the EE05 with the nRF52840 Plus (it adds NFC pads on D7/D14 for that XIAO)."],
        dims="86 x 138 x 13", grams=95, radio="BLE 5 only", sd="yes", usb="charge, USB drive, UF2 updates")


variant("SR-9", "SR-9 Roadie: ESP32-C5 + 4.2in with a PSP-style thumbstick",
        "Analog thumbstick at the left thumb, OK and BACK at the right. The stick pans a "
        "zoomed zine page (pages stored at 2x), scrolls menus fast, and moves page by page "
        "with a flick.",
        [GDEY042,
         L("U1", "ESP32-C5-WROOM-1 module (N8R8)", "Espressif", "ESP32-C5-WROOM-1-N8R8", "", "", "",
           4.50, 3.40, 1, "estimate", "confirm stock; PSRAM holds the 2x page"),
         *POWER, *LATCH(2), *SPI_EPD,
         L("JOY", "PSP 2-axis analog thumbstick, 2x 10K pots, 19x19x9 mm, no click", "Adafruit",
           "444", "", "", "Adafruit", 3.50, 2.80, 1, "vendor",
           "$3.50 (1-9), $2.80 (100+); Adafruit 512 adds a select click + breakout"),
         L("Q2", "P-MOSFET to power the stick only while awake", "Diodes", "DMG2305UX-7",
           "DMG2305UX-7DICT-ND", "621-DMG2305UX-7", "", 0.30, 0.10, 1, "estimate",
           "pots draw ~0.66 mA at 3.3 V: ~16 mAh/day if left on"),
         L("KEY", "Tact switch 6x3.5 mm SMD (OK, BACK)", "C&K", "PTS526 SM15 SMTR2 LFS",
           "CKN10504CT-ND", "611-PTS526SM15SMTR2L", "", 0.30, 0.10, 2, "estimate",
           "keys wake the pass; the stick cannot"),
         SD_SOCKET,
         L("PCB", "PCB 2-layer 1.0 mm, ~80x70 mm", "JLCPCB", "", "", "", "JLCPCB", 2.40, 0.70, 1,
           "estimate", ""),
         bat(1200, "603450", 4.80, 2.80),
         *ENCLOSURE, LANYARD],
        ["The 444 has four pads and no mounting holes: Adafruit's advice is wires and glue. "
         "The enclosure gets a printed cradle that clamps it from behind the bezel, and a "
         "TPU dust skirt around the cap.",
         "No click on the 444, so OK is a separate key. Use Adafruit 512 or 3246 if a pressable "
         "stick is wanted.",
         "Read both axes with the C5's ADC only while awake; dead zone + calibration at boot."],
        dims="88 x 149 x 14.5", grams=105, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5", sd="yes",
        usb="charge, console, Web Serial loader")

GDEM0397 = L("EPD", "3.97in e-paper 800x480 B/W, 235 ppi, SPI (outline 96.6 x 56.2 mm)", "Good Display",
             "GDEM0397T81P", "", "", "buyepaper.com", 14.70, 12.50, 1, "vendor",
             "buyepaper $14.70, 94 in stock 2026-11-07; 4-colour twin GDEM0397F81")
EDGE = lambda n, names: L("KEY", f"Side-actuated tact switch ({names})", "C&K", "PTS815 SJM 250 SMTR LFS",
                          "", "", "", 0.30, 0.10, n, "estimate", "edge keys")

variant("SR-10", "SR-10 Credential: 85 x 55 mm card (CR80), 2.9in",
        "The size of an ID card or a 3.5 x 2 in business-card pass, 8.5 mm thick, with a slot "
        "punched at the top like a real card badge. A credential and schedule screen first: "
        "296x128 shows about eight lines, so it is for badges, set times and short notes, not books.",
        [L("EPD", "2.9in e-paper 296x128 B/W, fast/partial refresh (outline 79 x 36.7 mm)",
           "Good Display", "GDEY029T94", "", "", "buyepaper.com", 6.39, 5.50, 1, "vendor",
           "same glass class as maplet's ssd1680-290"),
         L("U1", "ESP32-C5-WROOM-1 module (N8R8)", "Espressif", "ESP32-C5-WROOM-1-N8R8", "", "", "",
           4.50, 3.40, 1, "estimate", "beside the cell, not stacked, to stay 8.5 mm"),
         *POWER, *LATCH(3), *SPI_EPD, EDGE(3, "UP, OK, DOWN on the right edge"), SD_SOCKET,
         L("PCB", "PCB 2-layer 0.8 mm, card outline", "JLCPCB", "", "", "", "JLCPCB", 2.40, 0.70, 1,
           "estimate", ""),
         L("BAT", "LiPo 350 mAh 402540-class, 4 mm, PCM, JST-SH lead", "", "", "", "", "Amazon / hobby shop (Adafruit, SparkFun); Alibaba for 100",
           5.00, 1.80, 1, "estimate", "thinnest cell that fits beside the module"),
         L("ENC", "Enclosure, PETG, 1.2 mm walls (~18 g)", "", "", "", "", "own printer", 0.40, 0.30, 1,
           "estimate", "print flat; walls this thin want a 0.25 mm nozzle"),
         *ENCLOSURE[1:], LANYARD],
        ["Not a book reader. Pair it with a bigger pass, or use it as the credential that also "
         "carries tonight's running order and a zine's cover.",
         "Thinness is the hard part: 8.5 mm leaves ~4 mm for module and cell side by side."],
        dims="85 x 55 x 8.5", grams=40, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5", sd="yes",
        usb="charge, console, Web Serial loader")

variant("SR-11", "SR-11 Full Bleed: 3 x 4 in (76 x 102 mm), 3.97in 800x480",
        "The whole face is screen. The 3.97in panel fills a 3 x 4 in pass, so the pass graphic is "
        "drawn by the firmware at the top of the page instead of printed on a band, and the "
        "lanyard hooks a fixed D-ring on the back. All keys are on the edges.",
        [GDEM0397,
         L("U1", "ESP32-C5-WROOM-1 module (N8R8)", "Espressif", "ESP32-C5-WROOM-1-N8R8", "", "", "",
           4.50, 3.40, 1, "estimate", ""),
         *POWER, *LATCH(4), *SPI_EPD, EDGE(4, "UP, OK, DOWN right; BACK left"), SD_SOCKET,
         L("PCB", "PCB 2-layer 1.0 mm, ~70 x 95 mm", "JLCPCB", "", "", "", "JLCPCB", 2.40, 0.70, 1,
           "estimate", ""),
         L("DR", "Stainless D-ring 10 mm, moulded into the back shell", "", "", "", "", "Amazon (multi-pack); AliExpress for 100",
           0.20, 0.08, 1, "estimate", "keeps the outline exactly 3 x 4 in"),
         bat(1000, "503450", 4.20, 2.60),
         *ENCLOSURE, LANYARD],
        ["Tight: the panel outline is 96.6 mm inside a 101.6 mm pass, so the bezel is 2.5 mm top "
         "and bottom. Check the FPC bend room before the first print.",
         "800x480 at 235 ppi: the sharpest text of any SPI variant, for the price of a 4.2in."],
        dims="76.2 x 101.6 x 11 (+5 D-ring)", grams=70, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5", sd="yes",
        usb="charge, console, Web Serial loader")

variant("SR-12", "SR-12 Tour Laminate: 80 x 145 mm, 3.97in 800x480",
        "A classic 80 x 145 mm laminate: band, 3.97in 235 ppi page, 5-way and page keys. The "
        "everyday reader at a sub-4 in width.",
        [GDEM0397,
         L("U1", "ESP32-C5-WROOM-1 module (N8R8)", "Espressif", "ESP32-C5-WROOM-1-N8R8", "", "", "",
           4.50, 3.40, 1, "estimate", ""),
         *POWER, *LATCH(7), *SPI_EPD,
         L("NAV", "5-way navigation switch", "Alps", "SKRHABE010", "", "688-SKRHAB", "", 1.60, 0.95,
           1, "estimate", ""),
         L("KEY", "Tact switch 6x3.5 mm SMD (PREV, NEXT)", "C&K", "PTS526 SM15 SMTR2 LFS",
           "CKN10504CT-ND", "611-PTS526SM15SMTR2L", "", 0.30, 0.10, 2, "estimate", ""),
         SD_SOCKET,
         L("PCB", "PCB 2-layer 1.0 mm, ~70 x 125 mm", "JLCPCB", "", "", "", "JLCPCB", 2.40, 0.70, 1,
           "estimate", ""),
         bat(1200, "603450", 4.80, 2.80),
         *ENCLOSURE, LANYARD],
        ["SR-3 (78 x 149) is within 4 mm of this size; trimming its key zone makes it 80 x 145 too."],
        dims="80 x 145 x 12", grams=90, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5", sd="yes",
        usb="charge, console, Web Serial loader")

variant("SR-13", "SR-13 Hybrid Credential: 3.69 x 8.49 in (94 x 216 mm), 4.7in + ID card window",
        "The long tour laminate. A 4.7in 16-grey reader on top and, below the keys, a clear "
        "window holding the venue's printed ID-1 credential (photo, hologram, barcode), which "
        "slides in from the left edge. One pass instead of two on the lanyard.",
        [L("BRD", "M5Stack PaperDIY Assembly Kit (ESP32-S3, 4.7in 960x540 16-grey, microSD)",
           "M5Stack", "K159", "", "", "M5Stack store / AliExpress", 25.00, 22.00, 1, "vendor",
           "SOLD OUT 2026-11-07; fallback LilyGo T5 4.7in S3 $39.99"),
         L("KEY", "Tact switch 6x6 mm THT on flying leads (PREV, MENU, NEXT)", "Omron", "B3F-1000",
           "SW400-ND", "653-B3F-1000", "", 0.35, 0.12, 3, "estimate", ""),
         L("PWR", "Slide switch, battery disconnect", "C&K", "JS102011SAQN", "401-2000-ND",
           "611-JS102011SAQN", "", 0.45, 0.20, 1, "estimate", ""),
         bat(1500, "604050", 5.40, 3.20),
         L("WIN", "Clear PETG/PC window 0.5 mm, 88 x 57 mm, laser cut", "", "", "", "", "Ponoko/local",
           0.60, 0.25, 1, "estimate", "credential sleeve front"),
         L("ENC", "Enclosure, PETG, front + back shell with credential sleeve (~90 g)", "", "", "", "",
           "own printer", 1.80, 1.10, 1, "estimate", ""),
         *ENCLOSURE[1:], LANYARD],
        ["The credential stays paper so venue staff can check it the way they always do; the "
         "reader never pretends to be the credential.",
         "216 mm is long: the lanyard ring is at the top, so it hangs from chest to belt."],
        dims="94 x 216 x 14.5", grams=165, radio="Wi-Fi 4 2.4 GHz, BLE 5", sd="yes (on the kit)",
        usb="charge, USB drive, console")

NFC_READER = [
    L("U7", "NFC reader IC, CLRC663 plus, 13.56 MHz, low-power card detect", "NXP",
      "CLRC66303HN", "", "", "Newark / Avnet", 6.80, 5.30, 1, "vendor",
      "MFRC522 is EOL (NXP: use CLRC663 plus); Newark 3,628 at CA$9.41, 2026-11-07"),
    L("Y2", "Crystal 27.12 MHz for the reader", "", "", "", "", "LCSC", 0.50, 0.20, 1, "estimate", ""),
    L("ANT", "Reader antenna matching: EMC filter inductors + C0G caps (PCB coil ~40 x 25 mm)",
      "Murata/Yageo", "", "", "", "LCSC", 0.60, 0.25, 1, "estimate", "tune on the bench with a VNA or NanoVNA"),
    L("FER", "Ferrite sheet 40 x 30 mm, between coil and battery/ground", "", "", "", "", "Amazon (NFC anti-metal sticker)",
      0.60, 0.25, 1, "estimate", "without it the cell detunes the antenna"),
    L("Q3", "P-MOSFET: reader powered only while scanning", "Diodes", "DMG2305UX-7",
      "DMG2305UX-7DICT-ND", "621-DMG2305UX-7", "", 0.30, 0.10, 1, "estimate", ""),
]
SR14_COMMON = [
    GDEM0397, *SPI_EPD, *NFC_READER,
    EDGE(3, "UP, OK, DOWN on the right edge"),
    L("KEY", "Tact switch 6x3.5 mm SMD (TAP)", "C&K", "PTS526 SM15 SMTR2 LFS", "CKN10504CT-ND",
      "611-PTS526SM15SMTR2L", "", 0.30, 0.10, 1, "estimate", "turns the reader field on for 10 s"),
    SD_SOCKET,
    L("PCB", "PCB 2-layer 1.0 mm, ~70 x 140 mm incl. reader coil", "JLCPCB", "", "", "", "JLCPCB",
      2.80, 0.80, 1, "estimate", ""),
    bat(1200, "603450", 4.80, 2.80),
    *ENCLOSURE, LANYARD,
]

variant("SR-14W", "SR-14 Tap Pass, Wi-Fi build: ESP32-C5 + NFC reader, 80 x 155 mm",
        "Tap the back of the pass on a tag on a poster, flyer, record sleeve or merch table. A "
        "tag holds either a link to the zine on Subverse or a small zine itself. The Wi-Fi build "
        "fetches linked zines on its own (known network or the phone's hotspot, either band) and "
        "falls back to the phone over BLE when there is no Wi-Fi.",
        [L("U1", "ESP32-C5-WROOM-1 module (N8R8)", "Espressif", "ESP32-C5-WROOM-1-N8R8", "", "", "",
           4.50, 3.40, 1, "estimate", ""),
         *POWER, *LATCH(4),
         L("U8", "Dynamic NFC tag, I2C (the pass is also a tag: share link, BLE pairing)", "ST",
           "ST25DV04K-JFR6D3", "", "", "", 0.90, 0.50, 1, "estimate",
           "12-pin part with LPD (1.3 uA); the SO8 IER6S3 idles at 76 uA"),
         *SR14_COMMON],
        ["Fetching over Wi-Fi needs packs rendered for the pass's panel by a server (Subverse or "
         "an open pack-render service), because the pass cannot render PDFs itself. Without "
         "that, linked zines wait for the phone.",
         "Reader is powered only while scanning (TAP key, 10 s); the ST25DV answers phones "
         "with no power from the pass."],
        dims="80 x 155 x 12.5", grams=95, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5, NFC reader + tag",
        sd="yes", usb="charge, console, Web Serial loader")

variant("SR-14B", "SR-14 Tap Pass, tethered build: nRF52840 + NFC reader, 80 x 155 mm",
        "Same pass, no Wi-Fi. A tapped link goes over BLE to the phone app, which downloads the "
        "zine, renders it for the panel and sends the pages back. Small zines stored on the tag "
        "show at once, phone or not.",
        [L("U1", "nRF52840 module (BLE 5, native USB, NFC-A tag peripheral)", "Raytac",
           "MDBT50Q-1MV2", "", "", "", 7.80, 5.50, 1, "estimate",
           "its NFCT peripheral makes the pass a tag with just a coil: no ST25DV"),
         *POWER,
         *SR14_COMMON],
        ["Cheaper to run, not to build: the nRF module costs more than the C5, but there is no "
         "dynamic-tag chip and sleep current is the lowest of the set.",
         "USB-C is a USB drive here (nRF native USB), so it also loads packs with no phone."],
        dims="80 x 155 x 12.5", grams=95, radio="BLE 5 only, NFC reader + tag", sd="yes",
        usb="charge, USB drive, UF2 updates")


variant("SR-15", "SR-15 Dial Pass: Elecrow CrowPanel 4.2in (ESP32-S3) with its thumb dial",
        "A finished e-paper board in our shell: the CrowPanel's 3-way thumb dial turns pages, "
        "MENU and EXIT sit beside it. Panel, microSD, charger and keys are already on the board; "
        "add a cell, a power switch and a two-resistor battery divider.",
        [L("BRD", "Elecrow CrowPanel ESP32 4.2in E-Paper HMI (DIE07300S): ESP32-S3-WROOM-1-N8R8, "
           "400x300 SSD1683, dial (UP/DOWN/push) + MENU + EXIT, microSD, LiPo charger, SH1.0 battery "
           "socket, 107 x 87 mm board", "Elecrow", "", "", "", "Elecrow DIE07300S", 26.90, 22.00, 1,
           "vendor", "Elecrow $26.90 in stock 2026-11-09; ships in an acrylic case we don't use"),
         bat(1200, "603450", 4.80, 2.80),
         L("ADP", "Battery pigtail: JST-PH 2.0 socket to SH1.0 2-pin plug", "", "", "", "",
           "Amazon (adapter cable pack); AliExpress for 100", 0.80, 0.15, 1, "estimate", "the board takes SH1.0; check polarity, these come both ways"),
         L("PWR", "Slide switch, battery disconnect (the board has none)", "C&K", "JS102011SAQN",
           "401-2000-ND", "611-JS102011SAQN", "", 0.45, 0.20, 1, "estimate", ""),
         L("DIV", "Battery divider: 2x 100k 0603/THT to IO3 (ADC1) on the header", "Yageo", "", "", "",
           "", 0.05, 0.02, 1, "estimate", "the board has no battery sense; 100k/100k draws ~20 uA, "
           "or switch the low side from a spare GPIO"),
         L("ENC", "Enclosure, 3D-printed PETG, front + back shell sized for the 107 x 87 board (~55 g)",
           "", "", "", "", "own printer", 1.10, 0.75, 1, "estimate", "STEP model of the board in Elecrow's repo"),
         L("CAP", "Dial extension cap + key caps, printed TPU", "", "", "", "", "own printer", 0.15, 0.08, 1,
           "estimate", ""),
         L("LAM", "Laminate band: printed vinyl sticker", "", "", "", "", "sticker shop", 0.15, 0.05, 1,
           "estimate", "swappable 'pass' graphic"),
         L("HW", "M2x6 screws + M2 heat-set inserts, 4 each", "", "", "", "", "Amazon/Ali", 0.40, 0.20, 1,
           "estimate", "the board has no mounting holes: printed clamps hold it by the edges"),
         LANYARD],
        ["Controls (schematic): dial UP IO6, DOWN IO4, push IO5; MENU IO2; EXIT IO1; all with "
         "pull-ups, all RTC GPIOs, so any of them wakes the S3 from deep sleep. The dial is a "
         "3-way toggle with push (Elecrow part TM_2024A), not an encoder.",
         "Power switches on the board: IO7 cuts the panel's ground, IO42 the microSD's; drive both "
         "low before deep sleep. The red LED (IO41) is GPIO-driven.",
         "USB-C goes through a CH340C, not the S3's native USB, so no USB drive: load packs by "
         "microSD or the Web Serial loader. The CH340C runs from VBUS only, so it costs nothing on battery.",
         "Charger is a TP4054-class part at ~500 mA; regulation is an RY3420 buck. Measure sleep "
         "current on the bench: the buck's quiescent current isn't published.",
         "Elecrow ships PCBA V1.0, V1.2 and V1.2A (green sticker); the panel driver differs, so "
         "check which one arrives."],
        dims="95 x 140 x 16", grams=150, radio="Wi-Fi 4 2.4 GHz, BLE 5", sd="yes",
        usb="charge, console (CH340), Web Serial loader")


variant("SR-16", "SR-16 Setlist: XIAO ESP32-C5 + Elecrow 3.7in 240x416 portrait, slim pass",
        "The slim reader: a 3.7in portrait panel (Elecrow DIE04037S, UC8253, ~130 dpi) on Seeed's "
        "XIAO driver board, the size of a real backstage laminate. About half a 4.2in page of text, "
        "half the width of SR-1. No custom PCB; hand-wired keys as on SR-1D.",
        [L("MCU", "XIAO ESP32-C5 (Wi-Fi 6 2.4/5 GHz, BLE 5, charger on board)", "Seeed", "", "", "",
           "Seeed 100010048", 6.90, 6.50, 1, "vendor", "Seeed $6.90 (2026-11-09); RobotShop pre-soldered $8.31"),
         L("DRV", "ePaper Driver Board for XIAO V2 (24-pin SPI, boost)", "Seeed", "", "", "",
           "Seeed 114993558", 5.99, 4.90, 1, "vendor",
           "RESE fixed at 0.47 ohm, the value Elecrow's own 3.7in CrowPanel uses with this panel"),
         L("EPD", "3.7in e-paper 240x416 B/W, UC8253, partial refresh, 24-pin FPC (Elecrow DIE04037S; "
           "Good Display GDEY037T03 class), panel 53 x 93 x 1 mm", "Elecrow", "", "", "",
           "Elecrow DIE04037S", 11.90, 8.00, 1, "vendor",
           "Elecrow $11.90 in stock 2026-11-10; same order as SR-15's board"),
         L("KEY", "Tact switch 6x6 mm THT (PREV, MENU, NEXT)", "Omron", "B3F-1000",
           "SW400-ND", "653-B3F-1000", "", 0.35, 0.12, 3, "estimate", "on the long right edge"),
         L("DIO", "Schottky diode 1N5817, any-key wake line", "Diodes Inc", "1N5817-T", "", "", "",
           0.15, 0.05, 3, "estimate", "Schottky, not 1N4148: the drop must stay under the C5's low threshold"),
         L("PWR", "Slide switch, battery disconnect", "C&K", "JS102011SAQN",
           "401-2000-ND", "611-JS102011SAQN", "", 0.45, 0.20, 1, "estimate", ""),
         L("PRF", "Perfboard scrap + 30 AWG wire", "", "", "", "", "", 0.50, 0.20, 1, "estimate", ""),
         bat(1200, "603450", 4.80, 2.80),
         L("ENC", "Enclosure, 3D-printed PETG, front + back shell for the 53 x 93 panel (~35 g)", "", "", "",
           "", "own printer", 0.70, 0.50, 1, "estimate", "measure the panel first: see notes"),
         *ENCLOSURE[1:], LANYARD],
        ["Pins, as SR-1D: the driver board takes D0 RST, D1 CS, D2 BUSY, D3 DC, D8 SCK, D10 MOSI. "
         "The any-key wake line goes to GPIO3 (MTDI, back pad) through one Schottky per key; the keys "
         "also go to D4, D5 and D9 so the firmware can tell them apart.",
         "Controller is a UC8253 (UltraChip), not a Solomon part. GxEPD2 driver: GxEPD2_370_GDEY037T03 "
         "(240x416, partial ~350 ms, full ~1.5 s per the driver). Same 24-pin pinout as the other panels.",
         "RESE: Elecrow's CrowPanel 3.7in schematic (GitHub, Elecrow-RD) drives this panel with 0.47 ohm "
         "and an Si1308EDL, the same as Seeed's driver board. If you use a CFA-10084 instead, short its "
         "RESE jumper (0.47 ohm).",
         "Size: the listing gives a 53 x 93 x 1 mm panel but also a 67 x 93.4 x 2.6 mm 'PCB size' and "
         "50 g, so it may come on a carrier board. Measure one before printing; the 64 mm width below "
         "assumes the bare panel.",
         "Text per page: ~110-130 words at reading size (a 4.2in page holds ~250), so a book takes "
         "about twice the page turns; the battery figures assume the same 300 turns a day as the others."],
        dims="64 x 112 x 12", grams=70, radio="Wi-Fi 6 2.4 + 5 GHz, BLE 5",
        sd="no", usb="charge, console, Web Serial loader")


# ---------------------------------------------------------------- availability
# Checked 2026-11-07 (seller pages where reachable; DigiKey/Mouser/LCSC blocked automated
# checks, so their stock is unconfirmed). ok = in stock now; watch = in stock but single
# source, broker-only, long distributor lead time or thin stock for 100 units;
# out = sold out, EOL, discontinued or no confirmed source.
AVAIL = {  # match key (MPN, or a description fragment): (rating, where/stock, mitigation)
    "K159": ("out", "M5Stack: sold out (in stock 16-22 Oct only); K159-B kit also out; "
             "M5PaperS3 EOL, PaperColor out since May",
             "LilyGo T5 4.7in S3 ($39.99; China warehouse in stock, US/DE sold out): same glass, same firmware"),
    "TPS65185RGZR": ("out", "TI.com out of stock; no authorised listings",
                     "brokers at a premium, or a discrete HV supply (epdiy v4/v5 style)"),
    "ED060KC1": ("out", "distributors quote only (AERI); one marketplace listing at $131",
                 "buy from Kindle-spares sellers and test on arrival; price unconfirmed"),
    "ESP32-C5-WROOM-1": ("watch", "N8R8: Unikey ~13,000; DigiKey 20-week lead on N4; samples from Espressif",
                         "order modules first; ESP32-C5-WROOM-1-N8R8 or N16R8 are drop-in"),
    "RT9080-33GJ5": ("watch", "genuine Richtek only at brokers (Win Source); LCSC lists copies",
                     "Holtek HT7833 (4 uA Iq, 500 mA) is everywhere"),
    "ST25DV04K": ("watch", "IER6S3 Newark 346, Avnet 0; JFR6D3 unconfirmed",
                  "power VCC from the switched rail if only the SO8 part is found"),
    "XIAO ESP32-C5": ("ok", "Seeed in stock $6.99; DigiKey 13-week lead",
                      "buy direct from Seeed"),
    "XIAO nRF52840": ("ok", "Seeed in stock $10.40", ""),
    "DIE04037S": ("ok", "Elecrow $11.90 in stock (2026-11-10)", "Good Display GDEY037T03 is the same class of panel"),
    "DIE07300S": ("ok", "Elecrow $26.90 in stock (2026-11-09); also Tindie, OpenELab", "ship-from not stated"),
    "104990857": ("ok", "Seeed $19.95; reseller Pakronics 30 in stock (2026-11-09)",
                  "buy-lcd GDEY042T81 $13.80 (0 in stock 2026-11-09)"),
    "XIAO ESP32-S3 Plus": ("ok", "Seeed in stock $7.90 (2026-11-09)", ""),
    "XIAO ePaper Display Board EE05": ("ok", "Seeed in stock $10.99, $9.99 at 10+ (2026-11-09)",
                                       "Seeed's 114993558 driver board + hand-wired keys"),
    "DFR1222": ("ok", "DigiKey 148 in stock, 2-week lead (2026-11-09)", "DFRobot direct $7.90"),
    "CFA-10084": ("ok", "Crystalfontz (Spokane, WA) 506 in stock, $11.68 (2026-11-09)",
                  "Waveshare e-Paper Driver HAT 13512 on 3R (RobotShop US, backorder); Good Display DESPI-C02 "
                  "on 2.2 ohm (China, 0 in stock); not Seeed 114993558 (fixed 0.47 ohm)"),
    "114993558": ("ok", "Seeed in stock $5.99 (replaces the discontinued 105990172)", ""),
    "GDEY042T81-FL02": ("ok", "buyepaper 58", "100 units: order from Good Display direct"),
    "GDEY042T81": ("ok", "buyepaper 60", "100 units: order from Good Display direct"),
    "GDEY0583T81": ("ok", "buyepaper 93", ""),
    "GDEY0583F41": ("ok", "buyepaper 59 (LaskaKit was sold out in Oct)", ""),
    "GDEM0397T81P": ("ok", "buyepaper 94", ""),
    "GDEY029T94": ("ok", "buyepaper 73", ""),
    "CLRC66303HN": ("ok", "Avnet 12,000; Newark 3,628", "PN5180 also stocked"),
    "MDBT50Q-1MV2": ("ok", "TME 1,274; Win Source 7,800", ""),
    "MCP73831": ("ok", "Avnet 21,000", "TP4056 at LCSC"),
    "444": ("watch", "Adafruit 35 in stock", "100 units: generic PSP-1000 sticks (AliExpress)"),
    "4682": ("ok", "Adafruit in stock", ""),
    "ESP32-S3-WROOM-1": ("ok", "widely stocked (not re-checked)", ""),
    "TPS61169": ("ok", "commodity TI part (not re-checked)", ""),
    "PCA9555": ("ok", "commodity (not re-checked)", ""),
}


def avail(line):
    ref, desc, mfr, mpn = line[:4]
    for k, v in AVAIL.items():
        if (k in mpn) if mpn else (k in desc):
            return v
    if ref in ("BAT",):
        return ("ok", "generic LiPo pouch cells, many sellers", "")
    if mpn:
        return ("ok", "commodity part (not re-checked)", "")
    return ("ok", "printed, generic or cut-tape", "")


RANK = {"ok": 0, "watch": 1, "out": 2}

# ---------------------------------------------------------------- output
COLS = ["ref", "description", "manufacturer", "mpn", "digikey_pn", "mouser_pn", "other_vendor",
        "unit_usd", "unit_usd_100", "qty_per_unit", "order_qty", "ext_usd", "status", "note",
        "availability", "availability_note"]


def write(key, v):
    rows, total, total100 = [], 0.0, 0.0
    for line in v["lines"]:
        (ref, desc, mfr, mpn, dk, mo, other, unit, unit100, qty, status, note) = line
        av = avail(line)
        order = qty * BUILDS
        ext = unit * order
        total += ext
        total100 += unit100 * qty
        rows.append([ref, desc, mfr, mpn, dk, mo, other, f"{unit:.2f}", f"{unit100:.2f}", qty,
                     order, f"{ext:.2f}", status, note, av[0], av[1]])
    with open(os.path.join(HERE, f"{key}.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(COLS)
        w.writerows(rows)
    for dist, idx in (("digikey", 4), ("mouser", 5)):
        with open(os.path.join(HERE, f"{key}.{dist}-import.csv"), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["Quantity", "Manufacturer Part Number", "Manufacturer",
                        "Digikey Part Number" if dist == "digikey" else "Mouser Part Number",
                        "Customer Reference"])
            for r in v["lines"]:
                if r[3]:  # has an MPN: orderable from a distributor
                    w.writerow([r[9] * BUILDS, r[3], r[2], r[idx], r[0]])
    return total, total100


def write_availability(summary):
    out = ["# SubverseReader REV1: BOM availability", "",
           "Generated by `make_bom.py` from its `AVAIL` table; checked **2026-11-07**. Seller "
           "pages were read where reachable. DigiKey, Mouser, LCSC and AliExpress blocked "
           "automated checks, so their stock is unconfirmed. Ratings: **ok** in stock now; "
           "**watch** in stock but single source, broker-only, a long distributor lead time, or "
           "thin for 100 units; **out** sold out, EOL, discontinued or no confirmed source.", "",
           "## By variant", "",
           "| Variant | Rating | Lines to watch | Build-today fix |", "|---|---|---|---|"]
    for key, v, total, per, t100 in summary:
        rated = [(avail(l), l) for l in v["lines"]]
        worst = max((RANK[a[0]] for a, _ in rated), default=0)
        flag = sorted({(a[0], l[3] or l[1].split(",")[0]) for a, l in rated if a[0] != "ok"},
                      key=lambda t: -RANK[t[0]])
        fixes = sorted({a[2] for a, l in rated if a[0] == "out" and a[2]})
        out.append(f"| {key} | {['ok', 'watch', 'out'][worst]} | "
                   f"{'; '.join(f'{m} ({r})' for r, m in flag) or '—'} | {'; '.join(fixes) or '—'} |")
    out += ["", "## By part", "", "| Part | Rating | Where, stock | Mitigation | Used in |",
            "|---|---|---|---|---|"]
    used = {}
    for key, v, *_ in summary:
        for l in v["lines"]:
            for k in AVAIL:
                if (k in l[3]) if l[3] else (k in l[1]):
                    used.setdefault(k, set()).add(key)
                    break
    order = sorted(AVAIL.items(), key=lambda kv: -RANK[kv[1][0]])
    for k, (r, where, mit) in order:
        if k not in used:
            continue
        vs = ", ".join(sorted(used[k], key=lambda x: (int(x[3:].rstrip("WBCD")), x)))
        out.append(f"| {k} | **{r}** | {where} | {mit or '—'} | {vs} |")
    out += ["", "Changes made after this check: Seeed 105990172 (discontinued) became 114993558; "
            "MFRC522 (EOL) became CLRC663 plus; ESP32-C5-WROOM-1-N8R4 (not an orderable code) "
            "became N8R8; ST25DV04K-IER6S3 became the 12-pin JFR6D3 with low-power-down; panel "
            "and XIAO prices updated to the day's listings.", ""]
    with open(os.path.join(HERE, "AVAILABILITY.md"), "w") as f:
        f.write("\n".join(out))


def battery():
    """Battery-life figures from ../power/battery_model.py (run that first)."""
    path = os.path.join(HERE, "..", "power", "life.json")
    if not os.path.exists(path):
        return {}
    import json
    with open(path) as f:
        return json.load(f)


def fmt_days(d):
    return "1 yr+" if d >= 365 else (f"{d / 30.4:.0f} mo" if d >= 60 else f"{d:.0f} d")


def main():
    life = battery()
    summary = []
    for key, v in VARIANTS.items():
        total, t100 = write(key, v)
        summary.append((key, v, total, total / BUILDS, t100))
    out = ["# SubverseReader REV1 BOMs", "",
           "Bills of materials for the sixteen SubverseReader variants (nineteen BOMs: SR-14 has two builds; SR-1C and SR-1D are alternative SR-1 builds). `make_bom.py` generates every "
           "file here; edit it, not the CSVs.", "",
           "**Before ordering:** prices are 2026-11-06/07 snapshots; panels and XIAO boards are "
           "seller listings from 2026-11-07, see [AVAILABILITY.md](AVAILABILITY.md). Lines marked `estimate` have no live figure. Import the "
           "`*.digikey-import.csv` or `*.mouser-import.csv` file into the distributor's BOM tool "
           "for today's price; AliExpress, LCSC and vendor-direct lines are not in the import "
           "files.", "",
           f"**Quantities:** each order builds {BUILDS} units (PCB fabs ship 5 boards). "
           "`Per unit @5` is the order total divided by five; `Per unit @100` uses volume "
           "estimates (LCSC passives, factory-direct panels and cells) and excludes tooling.", "",
           "**Ordering:** `order.py` writes [order/ORDER.md](order/ORDER.md): landed cost (parts + "
           "shipping per vendor), the date the parts are in hand, DigiKey one-click cart links and "
           "Newark paste lists for each variant. `python3 order.py --live` refreshes it from the "
           "DigiKey, Mouser and Newark APIs when their keys are set.", "",
           "Target: **$50 or less per unit.**", "",
           "| Variant | Order total (5) | Per unit @5 | Per unit @100 | Under $50? | Files |",
           "|---|---|---|---|---|---|"]
    for key, v, total, per, t100 in summary:
        ok = "yes" if per <= 50 else ("at volume" if t100 <= 50 else "no")
        out.append(f"| {v['title']} | ${total:,.0f} | ${per:,.2f} | ${t100:,.2f} | {ok} | "
                   f"[`{key}.csv`]({key}.csv) |")
    out += ["", "## Size, radios and storage", "",
            "Dimensions are the enclosure as rendered (width x height x depth, mm). Weights are "
            "estimates: panel + cell + PETG + board, without the lanyard.", "",
            "| Variant | W x H x D (mm) | ~g | Radios | microSD | USB-C does |",
            "|---|---|---|---|---|---|"]
    for key, v, total, per, t100 in summary:
        out.append(f"| {key} | {v['dims']} | {v['grams']} | {v['radio']} | {v['sd']} | {v['usb']} |")
    if life:
        out += ["", "## Battery", "",
                "Stock cell (M) as in the BOM, with the thin (S) and extended (L) options. "
                "*Reader* = 300 page turns a day; *festival* = 150 pages, a sync and the "
                "variant's extras. Estimates from [`../power/battery_model.py`](../power/README.md), "
                "which also lists depth and cell prices per option.", "",
                "| Variant | S | M (stock) | L | Reader, M | Festival, M | Page turns per charge, M |",
                "|---|---|---|---|---|---|---|"]
        for key, v, total, per, t100 in summary:
            b = life.get(key)
            if not b:
                continue
            S, M, Lg = b["tiers"]
            out.append(f"| {key} | {S['mah']} mAh, {S['depth']:g} mm | {M['mah']} mAh, {M['depth']:g} mm | "
                       f"{Lg['mah']} mAh, {Lg['depth']:g} mm | {fmt_days(M['reader'])} | "
                       f"{fmt_days(M['festival'])} | {M['pages']:,} |")
    out.append("")
    for key, v, total, per, t100 in summary:
        out += [f"## {v['title']}", "", v["summary"], "",
                f"**{v['dims']} mm · ~{v['grams']} g · {v['radio']} · microSD: {v['sd']} · "
                f"USB-C: {v['usb']}**", ""]
        out += [f"- {n}" for n in v["notes"]]
        out += ["", f"Files: [`{key}.csv`]({key}.csv) · [`{key}.digikey-import.csv`]"
                f"({key}.digikey-import.csv) · [`{key}.mouser-import.csv`]({key}.mouser-import.csv)",
                ""]
    with open(os.path.join(HERE, "README.md"), "w") as f:
        f.write("\n".join(out))
    write_availability(summary)
    for key, v, total, per, t100 in summary:
        print(f"{key}: order ${total:.2f}  per unit @5 ${per:.2f}  @100 ${t100:.2f}")


main()
