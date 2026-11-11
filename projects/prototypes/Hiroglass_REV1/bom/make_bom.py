#!/usr/bin/env python3
"""Generate the Hiroglass REV1 BOMs.

    python3 make_bom.py

Writes, for each option, a full BOM CSV and import files for the DigiKey
and Mouser BOM tools, and regenerates README.md with the option totals.

Prices and stock were re-checked on 2026-11-10 (see sourcing.py and
BUILDABILITY.md for per-region sources, lead times and 1-5 unit costs).
Every line carries a status:
  checked     price and stock read on the seller's page on 2026-11-10
  snapshot    price seen in a search snippet, not on the page itself
  vendor      seen on the maker's own page (Tindie, Hicenda)
  estimate    no live figure; typical price
Re-check every line in the distributor cart before ordering.
"""
import csv
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
TEAM = 3  # builds: two dev units (one per site) and one demo unit
SITES = 2  # work happens at two sites; bench tools are one per site
PER_SITE = SITES / TEAM

# Each line: ref, description, mfr, mpn, digikey, mouser, other, unit, qty_per_kit,
#            status, note. qty_per_kit may be a float for team-shared items
#            (e.g. 1/3 = one per team). Empty distributor PN = look up by MPN.

OPTIONS = {}


def option(key, title, kits, summary, lines, extra_notes=()):
    OPTIONS[key] = dict(title=title, kits=kits, summary=summary, lines=lines,
                        notes=list(extra_notes))


SHARED_PUCK = [
    ("PUCK", "bq24074 LiPo charger, USB-C (set jumper to 500 mA)", "Adafruit", "4755",
     "1528-4755-ND", "485-4755", "", 14.95, 1, "checked", "in stock at Adafruit; not sold in Japan"),
    ("BAT", "LiPo 500 mAh, JST-PH", "Adafruit", "1578", "", "485-1578", "", 7.95, 1,
     "checked", "no thermistor; Japan: Sengoku DTP652533(PHR)"),
    ("SW", "JST-PH extension with on/off switch", "Adafruit", "3064", "", "", "", 2.95, 1,
     "checked", "out of stock at Adafruit, in stock at Switch Science; optional (unplug the cell)"),
    ("LEAD", "JST-PH 2-pin to male header, 200 mm", "Adafruit", "3814", "", "", "", 0.75, 1,
     "checked", ""),
    ("USB", "USB-A to USB-C cable", "Adafruit", "4474", "", "", "", 4.95, 2, "checked", ""),
    ("BB", "Half-size breadboard", "Adafruit", "64", "", "", "", 4.95, 1, "checked", ""),
    ("JMP", "Jumper wires, male/male", "SparkFun", "PRT-11026", "", "", "", 3.50, 1, "checked",
     "Adafruit 758 is out of stock"),
    ("HDR", "Header strip, 40-pin (one per site)", "SparkFun", "PRT-00116", "", "", "", 1.95, PER_SITE,
     "checked", "Adafruit 392 is out of stock"),
]
SENSORS = [
    ("IMU", "BMI270 IMU breakout, Qwiic", "SparkFun", "SEN-22397", "1568-22397-ND",
     "474-SEN-22397", "", 14.95, 1, "checked", "in stock at SparkFun and Switch Science"),
    ("ALS", "VEML7700 light sensor, STEMMA QT", "Adafruit", "4162", "1528-2891-ND", "", "", 4.95, 1,
     "checked", ""),
]

option("bench-a-beetle", "Bench kit A: Beetle ESP32-C3 (reuse goggl1)", TEAM,
       "Keeps goggl1's SPI display pins and Forth vocabulary. Wiring: schematics sheet B-A.",
       [("MCU", "Beetle ESP32-C3", "DFRobot", "DFR0868", "1738-DFR0868-ND", "", "", 7.90, 1,
         "checked", "skip if reusing goggl1 boards; DFRobot price, DigiKey stock unconfirmed"),
        ("OLED", "1.3\" 128x64 SSD1306 OLED, SPI/I2C (cut 2 jumpers for SPI)", "Adafruit", "938",
         "1528-1512-ND", "", "", 19.95, 1, "checked",
         "SSD1306; generic 1.3\" clones are SH1106. 0.96\" alternate: Adafruit 326"),
        *SENSORS,
        ("QTH", "Qwiic to male header cable", "SparkFun", "PRT-17912", "", "", "", 1.95, 1,
         "checked", "Beetle to IMU; Adafruit 4209 is out of stock"),
        ("QT", "STEMMA QT cable 50 mm", "Adafruit", "4399", "1528-4399-ND", "", "", 0.95, 1,
         "checked", "IMU to light sensor"),
        *SHARED_PUCK])

option("bench-b-qtpy", "Bench kit B: XIAO ESP32C3, all STEMMA QT", TEAM,
       "Everything on one I2C chain. The XIAO replaces the QT Py ESP32-C3, which is out of stock "
       "(18-week lead) and has no Japan radio certification. Wiring: schematics sheet B-B, "
       "with the XIAO's pins.",
       [("MCU", "Seeed XIAO ESP32C3", "Seeed", "113991054", "", "", "seeedstudio.com", 4.99, 1,
         "checked", "Japan: Akizuki, 技適 204-B00704. Was Adafruit QT Py ESP32-C3 5405 (out of stock)"),
        ("OLED", "1.3\" 128x64 SSD1306 OLED, STEMMA QT", "Adafruit", "938", "1528-1512-ND", "", "",
         19.95, 1, "checked", "I2C default 0x3D"),
        *SENSORS,
        ("QTH", "Qwiic to male header cable", "SparkFun", "PRT-17912", "", "", "", 1.95, 1,
         "checked", "the XIAO has no STEMMA QT socket"),
        ("QT", "STEMMA QT cable 50 mm", "Adafruit", "4399", "1528-4399-ND", "", "", 0.95, 2, "checked",
         "the 100 mm cable (4210) is out of stock at Adafruit"),
        *SHARED_PUCK])

# Pod-0 custom PCB set: one pod + one puck. Refs match schematics sheets P-1..P-3.
POD0 = [
    ("U1", "ESP32-C3-MINI-1 module, 4 MB", "Espressif", "ESP32-C3-MINI-1-N4X",
     "1965-ESP32-C3-MINI-1-N4XCT-ND", "", "", 3.26, 1, "checked",
     "1,518 at DigiKey. The N4 is not for new designs; N4X needs chip rev v1.1+ firmware support"),
    ("U2", "BMI270 IMU, LGA-14", "Bosch", "BMI270", "828-1091-1-ND", "262-BMI270", "", 4.23, 1,
     "checked", "32,243 at DigiKey"),
    ("U3", "VEML7700 ambient light sensor", "Vishay", "VEML7700-TT", "751-VEML7700-TTCT-ND",
     "78-VEML7700-TT", "", 1.44, 1, "checked", "14,393 at DigiKey"),
    ("U4", "3.3 V LDO 600 mA, SOT-23-5", "Diodes", "AP2112K-3.3TRG1", "AP2112K-3.3TRG1DICT-ND",
     "621-AP2112K-3.3TRG1", "", 0.28, 1, "checked", "406,515 at DigiKey; the TI alternate is out"),
    ("U5", "USB ESD protection, SOT-23-6", "ST", "USBLC6-2SC6", "497-5235-1-ND", "511-USBLC6-2SC6",
     "", 0.54, 1, "checked", "27,230 at DigiKey; check pinout vs datasheet"),
    ("DS1", "0.96\" 128x64 OLED panel, 30-pin 0.5 mm FPC", "Orient Display", "AOM12864A0-0.96WW", "",
     "", "orientdisplay.com", 4.53, 1, "checked",
     "169 at Orient's own store (Seattle). DigiKey: 0 stock, 8-week lead. Generic panels are 0.7 mm solder-down"),
    ("J1", "FPC connector 30-pin 0.5 mm ZIF", "Hirose", "FH12-30S-0.5SH(55)", "HFJ130CT-ND",
     "798-FH12-30S-0.5SH55", "", 2.64, 1, "checked", "4,414 at DigiKey"),
    ("J2", "USB-C receptacle 24-pin, full featured", "Amphenol", "12401610E4#2A",
     "12401610E4#2ACT-ND", "", "", 1.43, 1, "checked",
     "79,753 at DigiKey; the 20-week lead seen in October is gone"),
    ("J3,J12", "JST SH 10-pin side-entry header", "JST", "SM10B-SRSS-TB(LF)(SN)",
     "455-SM10B-SRSS-TBCT-ND", "306-SM10B-SRSSTBLFSN", "", 0.93, 2, "checked", "tether, one per board"),
    ("CBL", "JST SH 10-pin cable, both ends, 152 mm", "JST", "A10SR10SR30K152B", "455-3728-ND", "",
     "", 2.46, 1, "checked",
     "106 at DigiKey; the 152A (5,019 in stock) is the reversed pinning: check pin 1 before ordering"),
    ("SW1,SW2", "Tactile switch SMT (RESET, BOOT)", "C&K", "PTS810 SJM 250 SMTR LFS", "CKN10502CT-ND",
     "611-PTS810SJM250SMTR", "", 0.58, 2, "checked", "Japan: Alps SKRPACE010 at Akizuki (check pads)"),
    ("D1,D12", "LED green 0603", "Lite-On", "LTST-C191KGKT", "160-1446-1-ND", "859-LTST-C191KGKT",
     "", 0.15, 2, "checked", ""),
    ("D13", "LED red 0603", "Lite-On", "LTST-C191KRKT", "160-1447-1-ND", "", "", 0.15, 1, "checked", ""),
    ("Q1", "P-MOSFET SOT-23 (OLED supply)", "Diodes", "DMG2305UX-7", "DMG2305UX-7DICT-ND", "", "",
     0.38, 1, "checked",
     "74,154 at DigiKey. Replaces FDN338P (0 stock, 57-week lead); same G-S-D SOT-23, verify Vgs(th)"),
    ("Q2", "N-MOSFET SOT-23", "onsemi", "FDN337N", "", "", "", 0.77, 1, "checked",
     "222,614 at DigiKey. Replaces FDN335N (0 stock, 70-week lead); verify against the schematic"),
    ("U10", "LiPo charger with power path, VQFN-16", "TI", "BQ24074RGTR", "296-38873-1-ND",
     "595-BQ24074RGTR", "", 2.43, 1, "checked", "1,901 at DigiKey; RGTT has 8,549 at $3.19"),
    ("U11", "Fuel gauge, TDFN-8 (optional)", "Analog Devices", "MAX17048G+T10",
     "MAX17048G+T10CT-ND", "700-MAX17048G+T10", "", 4.61, 1, "checked",
     "21,841 at DigiKey; check pinout vs datasheet"),
    ("BT1", "LiPo 500 mAh, JST-PH", "Adafruit", "1578", "", "485-1578", "", 7.95, 1, "checked",
     "no NTC: bond RT1 to the cell. Japan: Sengoku DTP652533(PHR)"),
    ("RT1", "NTC 10k 0402", "Murata", "NCU15XH103F60RC", "", "", "", 0.10, 1, "checked",
     "39,686 at DigiKey. NCP15XH103F03RC is not for new designs (still 122,042 in stock)"),
    ("J10", "USB-C receptacle 16-pin", "GCT", "USB4105-GF-A", "2073-USB4105-GF-ACT-ND",
     "640-USB4105-GF-A", "", 0.80, 1, "checked", "263,569 at DigiKey"),
    ("J11", "JST PH 2-pin SMT header", "JST", "S2B-PH-SM4-TB(LF)(SN)", "455-S2B-PH-SM4-TBCT-ND",
     "306-S2BPHSM4TBLFSN", "", 0.49, 1, "checked", "verify cell polarity"),
    ("SW10", "Slide switch SMT", "C&K", "PCM12SMTR", "401-2016-1-ND", "611-PCM12SMTR", "", 1.28, 1,
     "checked", "obsolete, 2,726 left at DigiKey: enough for this run, replace before any larger one"),
    ("D10,D11", "Schottky 1 A SOD-123FL", "onsemi", "MBR120VLSFT1G", "MBR120VLSFT1GOSCT-ND", "", "",
     0.61, 2, "checked",
     "105,142 at DigiKey. Replaces PMEG2010AEH (0 stock until Oct 2027); verify pad fit and Vf"),
    # passives
    ("C1", "10 µF 0603 X5R", "Samsung", "CL10A106KP8NNNC", "", "", "", 0.10, 1, "estimate", ""),
    ("C2,C4,C5,C6", "100 nF 0402 X7R", "Samsung", "CL05B104KO5NNNC", "1276-1001-1-ND", "", "", 0.02,
     4, "estimate", ""),
    ("C3,C8,C9,C12,C13,C23", "1 µF 0402 X5R", "Samsung", "CL05A105KO5NNNC", "", "", "", 0.03, 6,
     "estimate", ""),
    ("C7,C21,C22", "4.7 µF 0603 X5R 10 V", "Samsung", "CL10A475KP8NNNC", "", "", "", 0.05, 3,
     "estimate", ""),
    ("C10,C11", "1 µF 0603 X5R 16 V (OLED pump)", "Murata", "GRM188R61C105KA93D", "", "", "", 0.05, 2,
     "estimate", ""),
    ("C14", "4.7 µF 0805 X7R 16 V (VCOMH)", "Murata", "GRM21BR71C475KE51L", "", "", "", 0.15, 1,
     "estimate", ""),
    ("C15", "2.2 µF 0603 X7R 16 V (VCC)", "Murata", "GRM188R71C225KE11D", "", "", "", 0.10, 1,
     "estimate", ""),
    ("C20", "1 µF 0603 X5R 25 V", "Samsung", "CL10A105KA8NNNC", "", "", "", 0.05, 1, "estimate", ""),
    ("R1,R2,R3,R17,R22", "10k 0402 1%", "Yageo", "RC0402FR-0710KL", "311-10.0KLRCT-ND", "", "",
     0.01, 5, "estimate", ""),
    ("R5,R6,R13,R14", "4.7k 0402 1%", "Yageo", "RC0402FR-074K7L", "311-4.70KLRCT-ND", "", "", 0.01,
     4, "estimate", ""),
    ("R8,R9,R20,R21", "5.1k 0402 1%", "Yageo", "RC0402FR-075K1L", "311-5.10KLRCT-ND", "", "", 0.01,
     4, "estimate", "USB-C CC"),
    ("R10,R11", "0 Ω 0402", "Yageo", "RC0402JR-070RL", "311-0.0JRCT-ND", "", "", 0.01, 2,
     "estimate", ""),
    ("R4", "1k 0402 1%", "Yageo", "RC0402FR-071KL", "311-1.00KLRCT-ND", "", "", 0.01, 1, "estimate", ""),
    ("R12", "620k 0402 1% (OLED IREF, tune)", "Yageo", "RC0402FR-07620KL", "311-620KLRCT-ND", "", "",
     0.01, 1, "estimate", "also order 470k and 560k"),
    ("R15,R16", "47k 0402 1%", "Yageo", "RC0402FR-0747KL", "311-47.0KLRCT-ND", "", "", 0.01, 2,
     "estimate", ""),
    ("R7", "100k 0402 1%", "Yageo", "RC0402FR-07100KL", "311-100KLRCT-ND", "", "", 0.01, 1,
     "estimate", ""),
    ("R23", "1.78k 0402 1% (ISET 500 mA)", "Yageo", "RC0402FR-071K78L", "", "", "", 0.01, 1,
     "estimate", ""),
    ("R24", "3.09k 0402 1% (ILIM 500 mA)", "Yageo", "RC0402FR-073K09L", "", "", "", 0.01, 1,
     "estimate", ""),
    ("R25,R26", "1.5k 0402 1%", "Yageo", "RC0402FR-071K5L", "", "", "", 0.01, 2, "estimate", ""),
    ("PCB", "Pod (4-layer) + puck (2-layer) PCBs, 3 of each", "OSH Park", "", "", "", "oshpark.com",
     23.25, 1 / TEAM, "checked",
     "US-made, free shipping, 9-14 days ($46.50 Super Swift, about a week). 6 of each for 4-5 units"),
    ("STN", "Solder paste stencils, pod + puck", "OSH Stencils", "", "", "", "oshstencils.com",
     20.00, 1 / TEAM, "estimate", "price not confirmed; needed for the LGA-14 and QFN parts"),
]
option("pod0-pcb", "Pod-0 custom PCB set (pod + puck)", TEAM,
       "Chip-level parts for schematics sheets P-1 to P-3. Hand or reflow assembly; "
       "ICs and passives get spares in the order quantities.", POD0,
       ["Three parts changed on 2026-11-10 because the originals have no stock: Q1 FDN338P to "
        "DMG2305UX-7, Q2 FDN335N to FDN337N, D10/D11 PMEG2010AEH to MBR120VLSFT1G. The schematics "
        "still show the old part numbers; check each substitute against its datasheet before layout.",
        "Buy the OLED panel from Orient Display's own store (169 in stock, $4.53); DigiKey has "
        "none for 8 weeks. Do not substitute a generic 0.96\" panel: those have a 0.7 mm "
        "solder-down tail that does not fit J1.",
        "U1 is now the ESP32-C3-MINI-1-N4X. The N4 is marked not for new designs."])

OPTICS = [
    ("LENS-A", "Plano-convex 15 mm x f20 mm, uncoated (cartridge A)", "Edmund Optics", "63-480", "",
     "", "edmundoptics.com", 34.00, 1, "checked", "20+ in stock; Japan 4,800 yen"),
    ("BS-A", "Plate beamsplitter 50R/50T float glass 25 mm", "Edmund Optics", "43-736", "", "",
     "edmundoptics.com", 61.00, 1, "checked",
     "20+ in stock. 30R/70T version is #43-734 at the same price (not #34-412, a $222 precision plate)"),
    ("LENS-B", "Eyepiece lens 12.7 mm x f20 mm, AR coated (cartridge B)", "Thorlabs", "LA1074-A",
     "", "", "thorlabs.com", 39.10, 1, "checked",
     "Edmund #47-873 (15 x 22.5 mm) shows 'Contact us' in the US and Japan. Japan: Edmund #23-083, 2,800 yen"),
    ("ND", "ND 1.0 filter 25 mm (≈ category-3 tint) for the brightness test", "Edmund Optics",
     "48-090", "", "", "edmundoptics.com", 51.50, PER_SITE, "checked", "one per site"),
    ("MAG", "N52 disc magnet 6.35 x 1.6 mm", "K&J Magnetics", "D41-N52", "", "", "kjmagnetics.com", 0.32,
     10, "checked", "1/4 x 1/16 in, in stock; rated 80 °C: keep away from the LiPo"),
    ("SCR", "M2 x 6 pan-head screws (100)", "Zoro", "(M2-0.40 x 6 pan)", "", "", "zoro.com", 8.99,
     PER_SITE, "snapshot", "one per site"),
    ("STO", "M2 standoffs", "Würth", "(pick at order)", "", "", "", 0.45, 6, "estimate", ""),
    ("INS", "M2 heat-set inserts (100)", "Prusa Research", "(M2 short, 100 pcs)", "", "", "prusa3d.com",
     11.99, PER_SITE, "checked", "one per site; McMaster 94180A307 is about $36"),
    ("LUX", "Lux meter", "Extech", "LT40", "", "", "extech.com", 155.00, PER_SITE, "checked",
     "one per site. The LT45 is about $315. Budget: Dr.meter LX1330B about $60, UNI-T UT383 about $24"),
]
option("optics-mech", "Optics and mechanical for Pod-0 (Edmund tier)", TEAM,
       "DigiKey and Mouser do not stock usable lenses or beamsplitters, so optics come from "
       "Edmund. Budget tier: AliExpress PMMA aspheres and 30/70 plates at about $5–15 a set.",
       OPTICS,
       ["For a 30R/70T combiner use Edmund #43-734 (1 mm float glass, same $61). #34-412 is a "
        "3 mm fused-silica precision plate at $222.",
        "AliExpress tier (not a distributor order) is about $250 for the team including "
        "magnets, hardware and the lux meter."])

option("concept4-microoled", "Concept 4: micro-OLED driver experiment (one team set)", 1,
       "One bench set for the experimental branch. The panel is not sold by DigiKey or Mouser. "
       "Wiring: schematics sheet X-4.",
       [("MCU2", "Raspberry Pi Pico 2 (RP2350)", "Raspberry Pi", "SC1631", "2648-SC1631CT-ND",
         "358-SC1631", "", 5.00, 3, "snapshot", "PIO bus driver"),
        ("LVL", "8-bit dual-supply translator, TSSOP-24", "TI", "SN74AXC8T245PWR", "", "", "", 1.20, 2,
         "estimate", "needs a TSSOP-24 breakout"),
        ("BRK", "TSSOP-24 to DIP breakout", "", "(pick at order)", "", "", "", 3.00, 2, "estimate", ""),
        ("LDO18", "1.8 V LDO, SOT-23-5", "TI", "TLV75718PDBVR", "", "", "", 0.40, 1, "estimate", ""),
        ("PANEL", "Sony ECX336-class micro-OLED + HDMI driver board", "Display Components (HK)", "",
         "", "", "Tindie", 275.00, 1, "vendor",
         "read $275 on 2026-11-10 (was $99.50). DisplayModule: panel $139 + HDMI board $119. No US stock"),
        ("REF", "Hicenda 0.23\" 640x400 module, 4,000 nit (reference)", "Hicenda", "", "", "",
         "hicendadisplay.com", 389.00, 1, "vendor", "$184 for the standard version"),
        ("FPGA", "iCEBreaker FPGA board (optional)", "1BitSquared", "iCEBreaker", "", "", "1bitsquared.com",
         79.95, 1, "checked",
         "sold out at 1BitSquared. US: Lattice ICE40UP5K-B-EVN $65.62 (Newark). Japan: Tang Nano 9K")],
       ["A bare ECX336 also needs a 10 V rail and a cathode supply, which this BOM does not have.",
        "The public ECX336BF datasheet leaves about 35 timing registers as 'separately submitted', "
        "so the init sequence is not fully open."])

option("concept5-linux-puck", "Concept 5: Linux puck + Monado (one team set)", 1,
       "One set, glasses included. Wiring: schematics sheet X-5.",
       [("SBC", "Raspberry Pi 5, 8 GB", "Raspberry Pi", "SC1112", "", "358-SC1112", "", 175.00, 1,
         "checked", "PiShop.us and CanaKit; Japan 35,200 yen at Akizuki"),
        ("PSU", "27 W USB-C power supply", "Raspberry Pi", "SC1157", "", "", "", 12.95, 1, "checked", ""),
        ("FAN", "Active Cooler", "Raspberry Pi", "SC1148", "", "", "", 12.95, 1, "checked",
         "PiShop.us; the list price is $5"),
        ("SD", "microSD 64 GB", "", "(pick at order)", "", "", "", 12.00, 1, "estimate", ""),
        ("HDMI", "micro-HDMI to HDMI cable", "Raspberry Pi", "(pick at order)", "", "", "", 5.00, 1,
         "estimate", ""),
        ("ADP", "HDMI to USB-C (DisplayPort) adapter for the glasses", "", "(pick at order)", "", "",
         "", 69.00, 1, "estimate",
         "the team has adapters known to work with the Air 2; none confirmed for sale (VITURE's fits VITURE only)"),
        ("IMUB", "QT Py RP2040 (USB IMU bridge)", "Adafruit", "4900", "", "", "", 9.95, 1, "estimate", ""),
        ("IMU", "BNO085 9-DoF IMU breakout", "Adafruit", "4754", "1528-4754-ND", "", "", 29.50, 1,
         "checked", ""),
        ("QT", "STEMMA QT cable 50 mm", "Adafruit", "4399", "1528-4399-ND", "", "", 0.95, 1,
         "snapshot", ""),
        ("CAM", "Camera Module 3 (stereo pair)", "Raspberry Pi", "SC0872", "", "", "", 29.25, 2,
         "checked", "plus Pi 5 camera cables"),
        ("GLS", "XREAL Air 2 display glasses", "XREAL", "Air 2", "", "", "xreal.com",
         199.00, 1, "checked",
         "the team has one. The only family Monado supports directly; not sold standalone in Japan")],
       ["The ROCK 5B and 5B+ are out of stock everywhere checked, so the Pi 5 is the only board."])


# ---------------------------------------------------------------------------
def order_qty(line, kits, loose_parts):
    ref, *_ , unit, qpk, status, note = line
    need = round(qpk * kits, 6)
    if not loose_parts:
        return math.ceil(need)
    if unit < 0.25:          # passives come on cut tape in tens
        return max(10, math.ceil(need * 1.5))
    if ref.startswith(("U", "Q", "D", "J", "SW", "RT")):
        return math.ceil(need) + 2   # spares for hand assembly
    return math.ceil(need)


def write():
    rows_summary = []
    for key, o in OPTIONS.items():
        total, full = 0.0, []
        for line in o["lines"]:
            ref, desc, mfr, mpn, dk, mo, other, unit, qpk, status, note = line
            q = order_qty(line, o["kits"], key == "pod0-pcb")
            ext = q * unit
            total += ext
            full.append([ref, desc, mfr, mpn, dk, mo, other, f"{unit:.2f}",
                         round(qpk, 3), q, f"{ext:.2f}", status, note])
        with open(os.path.join(HERE, f"{key}.csv"), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["ref", "description", "manufacturer", "mpn", "digikey_pn", "mouser_pn",
                        "other_vendor", "unit_usd", "qty_per_kit", "order_qty", "ext_usd",
                        "status", "note"])
            w.writerows(full)
        for dist, col in (("digikey", 4), ("mouser", 5)):
            with open(os.path.join(HERE, f"{key}.{dist}-import.csv"), "w", newline="") as f:
                w = csv.writer(f)
                w.writerow(["Quantity", "Manufacturer Part Number", "Manufacturer",
                            f"{dist.title()} Part Number", "Customer Reference"])
                for r in full:
                    if r[6] or not r[3] or r[3].startswith("("):
                        continue        # not a distributor line
                    w.writerow([r[9], r[3], r[2], r[col], r[0]])
        rows_summary.append((key, o, total))
    return rows_summary


def readme(rows):
    out = ["# Hiroglass REV1 BOMs", "",
           "Orderable bills of materials for the Hiroglass REV1 build options. "
           "`make_bom.py` generates every file here; edit it, not the CSVs.", "",
           "**Is it buildable?** See [`BUILDABILITY.md`](BUILDABILITY.md): where each line can be "
           "bought in the US and in Japan, lead times, lines to watch, and the cost of a run of "
           "1 to 5 units. `sourcing.py` generates it.", "",
           "**Before ordering:** prices and stock were re-checked on 2026-11-10. Lines whose "
           "`status` is `checked` were read on the seller's page; `snapshot` came from a search "
           "result; `estimate` has no live figure. Import the `*.digikey-import.csv` or "
           "`*.mouser-import.csv` file into the distributor's BOM tool to see today's price and "
           "stock. The totals below are US prices.", "",
           "## Options", "",
           "| Option | Builds | Order total | Per build | Files |",
           "|---|---|---|---|---|"]
    for key, o, total in rows:
        out.append(f"| {o['title']} | {o['kits']} | ${total:,.0f} | ${total / o['kits']:,.0f} | "
                   f"[`{key}.csv`]({key}.csv) |")
    out += ["",
            "**Three of everything:** each option builds three units, two dev units (one at "
            "each site) and one demo unit. Bench tools (lux meter, ND filter, header pack, "
            "screws, inserts) are one per site; the PCB fab order is one for the team. "
            "Shared items are split across builds in the per-build column.",
            "",
            "The Pod-0 PCB total includes two spares of each chip for hand assembly and "
            "cut-tape minimums on passives.",
            "",
            "## Suggested combinations", "",
            "- **Recommended first order:** a bench kit for each build plus optics-mech. Every "
            "line is in stock in the US today. In Japan use kit B: the XIAO ESP32C3 is the board "
            "with a Japan radio certificate and deep stock (Akizuki).",
            "- **Pod-0 PCB set:** buildable, but three parts were swapped for in-stock substitutes "
            "that still need a datasheet check, and the OLED panel has one seller. Order the "
            "panels from Orient Display first.",
            "- **Experimental branch:** concept 4 and concept 5 start as one exploration set each. "
            "If a concept is picked for CES at the go/no-go review (Nov 20), order two more sets "
            "so it also has three units.",
            ""]
    for key, o, total in rows:
        out += [f"## {o['title']}", "", o["summary"], ""]
        for n in o["notes"]:
            out.append(f"- {n}")
        if o["notes"]:
            out.append("")
        out.append(f"Files: [`{key}.csv`]({key}.csv) · "
                   f"[`{key}.digikey-import.csv`]({key}.digikey-import.csv) · "
                   f"[`{key}.mouser-import.csv`]({key}.mouser-import.csv)")
        out.append("")
    out += ["## Sources", "",
            "Research notes behind every line: "
            "[`research_notes/BOM and schematics/`](../research_notes/BOM%20and%20schematics/).",
            "Schematics and wiring for each option: [`schematics/`](../schematics/README.md).", ""]
    with open(os.path.join(HERE, "README.md"), "w") as f:
        f.write("\n".join(out))


if __name__ == "__main__":
    rows = write()
    readme(rows)
    for key, o, total in rows:
        print(f"{key:24s} {o['kits']} builds  ${total:8.2f}  (${total / o['kits']:.2f} each)")
