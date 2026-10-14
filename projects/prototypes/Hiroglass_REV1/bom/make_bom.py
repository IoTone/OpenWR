#!/usr/bin/env python3
"""Generate the Hiroglass REV1 BOMs.

    python3 make_bom.py

Writes, for each option, a full BOM CSV and import files for the DigiKey
and Mouser BOM tools, and regenerates README.md with the option totals.

Prices and stock are snapshots from 2026-10-13/14. DigiKey and Mouser
blocked automated page fetches, so every line carries a status:
  snapshot    price/stock seen in a search snapshot of the distributor page
  vendor      seen on the maker's own page (Adafruit, Edmund, Tindie)
  estimate    no live figure; typical price
Re-check every line in the distributor cart before ordering.
"""
import csv
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
TEAM = 3  # builders

# Each line: ref, description, mfr, mpn, digikey, mouser, other, unit, qty_per_kit,
#            status, note. qty_per_kit may be a float for team-shared items
#            (e.g. 1/3 = one per team). Empty distributor PN = look up by MPN.

OPTIONS = {}


def option(key, title, kits, summary, lines, extra_notes=()):
    OPTIONS[key] = dict(title=title, kits=kits, summary=summary, lines=lines,
                        notes=list(extra_notes))


SHARED_PUCK = [
    ("PUCK", "bq24074 LiPo charger, USB-C (set jumper to 500 mA)", "Adafruit", "4755",
     "1528-4755-ND", "485-4755", "", 14.95, 1, "snapshot", "Mouser lists it without a price"),
    ("BAT", "LiPo 500 mAh, JST-PH", "Adafruit", "1578", "", "485-1578", "", 7.95, 1,
     "snapshot", "no thermistor"),
    ("SW", "JST-PH extension with on/off switch", "Adafruit", "1131", "", "", "", 2.95, 1,
     "estimate", ""),
    ("LEAD", "JST-PH 2-pin to male header, 200 mm", "Adafruit", "3814", "", "", "", 0.75, 1,
     "estimate", ""),
    ("USB", "USB-A to USB-C cable", "Adafruit", "4474", "", "", "", 2.95, 2, "estimate", ""),
    ("BB", "Half-size breadboard", "Adafruit", "64", "", "", "", 5.00, 1, "estimate", ""),
    ("JMP", "Jumper wires, male/male", "Adafruit", "758", "", "", "", 3.95, 1, "estimate", ""),
    ("HDR", "Header strip pack (one per team)", "Adafruit", "392", "", "", "", 4.95, 1 / TEAM,
     "estimate", ""),
]
SENSORS = [
    ("IMU", "BMI270 IMU breakout, Qwiic", "SparkFun", "SEN-22397", "1568-22397-ND",
     "474-SEN-22397", "", 18.50, 1, "snapshot", "DigiKey 33, Mouser 113 (+208 due 10/16)"),
    ("ALS", "VEML7700 light sensor, STEMMA QT", "Adafruit", "4162", "1528-2891-ND", "", "", 4.95, 1,
     "snapshot", "DigiKey 125"),
]

option("bench-a-beetle", "Bench kit A: Beetle ESP32-C3 (reuse goggl1)", TEAM,
       "Keeps goggl1's SPI display pins and Forth vocabulary. Wiring: schematics sheet B-A.",
       [("MCU", "Beetle ESP32-C3", "DFRobot", "DFR0868", "1738-DFR0868-ND", "", "", 6.90, 1,
         "snapshot", "skip if reusing goggl1 boards"),
        ("OLED", "1.3\" 128x64 SSD1306 OLED, SPI/I2C (cut 2 jumpers for SPI)", "Adafruit", "938",
         "1528-1512-ND", "", "", 19.95, 1, "snapshot", "0.96\" alternate: Adafruit 326"),
        *SENSORS,
        ("QTH", "STEMMA QT to male header cable", "Adafruit", "4209", "", "", "", 0.95, 1,
         "estimate", "Beetle to IMU"),
        ("QT", "STEMMA QT cable 50 mm", "Adafruit", "4399", "1528-4399-ND", "", "", 0.95, 1,
         "snapshot", "IMU to light sensor"),
        *SHARED_PUCK])

option("bench-b-qtpy", "Bench kit B: QT Py ESP32-C3, all STEMMA QT", TEAM,
       "Everything on one I2C chain, no soldering on the bench. Wiring: schematics sheet B-B.",
       [("MCU", "QT Py ESP32-C3", "Adafruit", "5405", "1528-5405-ND", "", "", 9.95, 1,
         "snapshot", "often out of stock; fallback Seeed XIAO ESP32C3 113991054 ($4.99)"),
        ("OLED", "1.3\" 128x64 SSD1306 OLED, STEMMA QT", "Adafruit", "938", "1528-1512-ND", "", "",
         19.95, 1, "snapshot", "I2C default 0x3D"),
        *SENSORS,
        ("QT", "STEMMA QT cable 100 mm", "Adafruit", "4210", "", "", "", 0.95, 3, "estimate",
         "DigiKey lists Adafruit 4528 (also 100 mm)"),
        *SHARED_PUCK])

# Pod-0 custom PCB set: one pod + one puck. Refs match schematics sheets P-1..P-3.
POD0 = [
    ("U1", "ESP32-C3-MINI-1 module, 4 MB", "Espressif", "ESP32-C3-MINI-1-N4",
     "1965-ESP32-C3-MINI-1-N4CT-ND", "356-ESP32C3MINI1N4", "", 3.26, 1, "snapshot", "2,779 at DigiKey"),
    ("U2", "BMI270 IMU, LGA-14", "Bosch", "BMI270", "828-1082-1-ND", "262-BMI270", "", 4.23, 1,
     "snapshot", "~50k at DigiKey"),
    ("U3", "VEML7700 ambient light sensor", "Vishay", "VEML7700-TT", "VEML7700-TTCT-ND",
     "78-VEML7700-TT", "", 1.53, 1, "snapshot", ""),
    ("U4", "3.3 V LDO 600 mA, SOT-23-5", "Diodes", "AP2112K-3.3TRG1", "AP2112K-3.3TRG1DICT-ND",
     "621-AP2112K-3.3TRG1", "", 0.45, 1, "estimate", "alt TI TLV75733PDBVR, same footprint"),
    ("U5", "USB ESD protection, SOT-23-6", "ST", "USBLC6-2SC6", "497-5235-1-ND", "511-USBLC6-2SC6",
     "", 0.50, 1, "estimate", "check pinout vs datasheet"),
    ("DS1", "0.96\" 128x64 OLED panel, 30-pin FPC", "Orient Display", "AOM12864A0-0.96WW", "", "",
     "", 11.57, 1, "snapshot", "DigiKey product 22531883; 0 stock, 8-week lead. Alt Winstar WEO012864DWPP3N00006 $5.24 (4 in stock)"),
    ("J1", "FPC connector 30-pin 0.5 mm ZIF", "Hirose", "FH12-30S-0.5SH(55)", "HFJ130CT-ND",
     "798-FH12-30S-0.5SH55", "", 1.40, 1, "estimate", ""),
    ("J2", "USB-C receptacle 24-pin, full featured", "Amphenol", "12401610E4#2A", "", "", "", 2.50,
     1, "estimate", "20-week lead seen; fallback GCT USB4105-GF-A (USB 2.0 only)"),
    ("J3,J12", "JST SH 10-pin side-entry header", "JST", "SM10B-SRSS-TB(LF)(SN)", "",
     "306-SM10B-SRSSTBLFSN", "", 0.90, 2, "estimate", "tether, one per board"),
    ("CBL", "JST SH 10-pin cable, both ends", "JST", "A10SR10SR30K152B", "", "", "", 2.00, 1,
     "estimate", "check length/part at order; or crimp your own"),
    ("SW1,SW2", "Tactile switch SMT (RESET, BOOT)", "C&K", "PTS810 SJM 250 SMTR LFS", "",
     "611-PTS810SJM250SMTR", "", 0.25, 2, "estimate", ""),
    ("D1,D12", "LED green 0603", "Lite-On", "LTST-C191KGKT", "160-1446-1-ND", "859-LTST-C191KGKT",
     "", 0.30, 2, "estimate", ""),
    ("D13", "LED red 0603", "Lite-On", "LTST-C191KRKT", "", "", "", 0.30, 1, "estimate", ""),
    ("Q1", "P-MOSFET SOT-23 (OLED supply)", "onsemi", "FDN338P", "FDN338PCT-ND", "512-FDN338P", "",
     0.40, 1, "estimate", ""),
    ("Q2", "N-MOSFET SOT-23", "onsemi", "FDN335N", "FDN335NCT-ND", "512-FDN335N", "", 0.40, 1,
     "estimate", ""),
    ("U10", "LiPo charger with power path, VQFN-16", "TI", "BQ24074RGTR", "", "595-BQ24074RGTR", "",
     3.19, 1, "snapshot", "9,247 at DigiKey"),
    ("U11", "Fuel gauge, TDFN-8 (optional)", "Analog Devices", "MAX17048G+T10",
     "MAX17048G+T10CT-ND", "700-MAX17048G+T10", "", 2.50, 1, "estimate", "check pinout vs datasheet"),
    ("BT1", "LiPo 500 mAh, JST-PH", "Adafruit", "1578", "", "485-1578", "", 7.95, 1, "snapshot",
     "no NTC: bond RT1 to the cell"),
    ("RT1", "NTC 10k 0402", "Murata", "NCP15XH103F03RC", "490-4801-1-ND", "81-NCP15XH103F03RC", "",
     0.10, 1, "estimate", ""),
    ("J10", "USB-C receptacle 16-pin", "GCT", "USB4105-GF-A", "2073-USB4105-GF-ACT-ND",
     "640-USB4105-GF-A", "", 0.80, 1, "estimate", ""),
    ("J11", "JST PH 2-pin SMT header", "JST", "S2B-PH-SM4-TB(LF)(SN)", "455-1749-1-ND",
     "306-S2BPHSM4TBLFSN", "", 0.40, 1, "estimate", "verify cell polarity"),
    ("SW10", "Slide switch SMT", "C&K", "PCM12SMTR", "", "611-PCM12SMTR", "", 0.70, 1, "estimate", ""),
    ("D10,D11", "Schottky 1 A SOD-123F", "Nexperia", "PMEG2010AEH", "", "", "", 0.35, 2, "estimate", ""),
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
    ("PCB", "Pod + puck PCBs, 5 each, 4-layer pod / 2-layer puck", "JLCPCB/PCBWay/OSHPark", "", "",
     "", "fab", 60.00, 1 / TEAM, "estimate", "one order for the team; not a DigiKey line"),
]
option("pod0-pcb", "Pod-0 custom PCB set (pod + puck)", TEAM,
       "Chip-level parts for schematics sheets P-1 to P-3. Hand or reflow assembly; "
       "ICs and passives get spares in the order quantities.", POD0,
       ["The OLED panel is the supply risk: Orient showed 0 stock with an 8-week lead and "
        "Winstar had 4 pieces. Order panels first.",
        "J2 (full 24-pin USB-C) showed a 20-week lead. The GCT USB4105-GF-A fits a "
        "different footprint and drops the DisplayPort lanes; decide before layout."])

OPTICS = [
    ("LENS-A", "Plano-convex 15 mm x f20 mm, uncoated (cartridge A)", "Edmund Optics", "63-480", "",
     "", "edmundoptics.com", 34.00, 1, "vendor", "budget: Edmund 15 x f22.5 clearance $18.20"),
    ("BS-A", "Plate beamsplitter 50R/50T float glass 25 mm", "Edmund Optics", "43-736", "", "",
     "edmundoptics.com", 61.00, 1, "vendor", "30R/70T VIS is #34-412 at $222"),
    ("LENS-B", "Eyepiece lens 15 mm x f22.5 mm VIS AR (cartridge B)", "Edmund Optics", "(page 8041)",
     "", "", "edmundoptics.com", 41.00, 1, "vendor", "or Thorlabs LA1951-A"),
    ("ND", "ND 1.0 filter 25 mm (≈ category-3 tint) for the brightness test", "Edmund Optics",
     "(page 8291)", "", "", "edmundoptics.com", 51.50, 1 / TEAM, "vendor", "one per team"),
    ("MAG", "N52 disc magnet 6.35 x 2.54 mm", "Radial Magnets", "9150", "", "", "", 0.35, 10,
     "snapshot", "rated 80 °C: keep away from the LiPo"),
    ("SCR", "M2 x 6 pan-head screws (100)", "Essentra / Würth", "(pick at order)", "", "", "", 10.00,
     1 / TEAM, "estimate", "DigiKey hardware category"),
    ("STO", "M2 standoffs", "Würth", "(pick at order)", "", "", "", 0.45, 6, "estimate", ""),
    ("INS", "M2 heat-set inserts (100)", "Ruthex / McMaster 94180A307", "", "", "", "McMaster", 15.00,
     1 / TEAM, "estimate", "not at DigiKey"),
    ("LUX", "Lux meter", "Extech", "LT45", "", "", "", 110.00, 1 / TEAM, "estimate",
     "DigiKey carries Extech; a luminance meter (Konica Minolta LS-150) is quote-only, borrow one"),
]
option("optics-mech", "Optics and mechanical for Pod-0 (Edmund tier)", TEAM,
       "DigiKey and Mouser do not stock usable lenses or beamsplitters, so optics come from "
       "Edmund. Budget tier: AliExpress PMMA aspheres and 30/70 plates at about $5–15 a set.",
       OPTICS,
       ["Swapping to Edmund #34-412 30R/70T plates adds about $160 per kit.",
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
         "", "", "Tindie", 99.50, 1, "vendor", "HDMI viewer board, for seeing the panel"),
        ("REF", "Hicenda 0.23\" 640x400 module, 4,000 nit (reference)", "Hicenda", "", "", "",
         "hicendadisplay.com", 389.00, 1, "vendor", "$184 for the standard version"),
        ("FPGA", "iCEBreaker FPGA board (optional)", "1BitSquared", "iCEBreaker", "", "", "Mouser storefront",
         80.00, 1, "estimate", "Yosys/nextpnr route")])

option("concept5-linux-puck", "Concept 5: Linux puck + Monado (one team set)", 1,
       "One set. Glasses are borrowed or bought separately. Wiring: schematics sheet X-5.",
       [("SBC", "Raspberry Pi 5, 8 GB", "Raspberry Pi", "SC1112", "", "358-SC1112", "", 175.00, 1,
         "snapshot", "price has risen sharply in 2026; check"),
        ("PSU", "27 W USB-C power supply", "Raspberry Pi", "SC1157", "", "", "", 12.00, 1, "estimate", ""),
        ("FAN", "Active Cooler", "Raspberry Pi", "SC1148", "", "", "", 5.00, 1, "estimate", ""),
        ("SD", "microSD 64 GB", "", "(pick at order)", "", "", "", 12.00, 1, "estimate", ""),
        ("HDMI", "micro-HDMI to HDMI cable", "Raspberry Pi", "(pick at order)", "", "", "", 5.00, 1,
         "estimate", ""),
        ("ADP", "HDMI to glasses adapter (XREAL Adapter)", "XREAL", "", "", "", "xreal.com", 69.00, 1,
         "estimate", "not needed with a ROCK 5B"),
        ("IMUB", "QT Py RP2040 (USB IMU bridge)", "Adafruit", "4900", "", "", "", 9.95, 1, "estimate", ""),
        ("IMU", "BNO085 9-DoF IMU breakout", "Adafruit", "4754", "1528-4754-ND", "", "", 24.95, 1,
         "snapshot", ""),
        ("QT", "STEMMA QT cable 50 mm", "Adafruit", "4399", "1528-4399-ND", "", "", 0.95, 1,
         "snapshot", ""),
        ("CAM", "Camera Module 3 (stereo pair)", "Raspberry Pi", "SC0872", "", "", "", 25.00, 2,
         "estimate", "plus Pi 5 camera cables"),
        ("GLS", "Birdbath glasses (XREAL One / Viture)", "XREAL / Viture", "", "", "", "retail",
         0.00, 1, "estimate", "borrowed: $400–500 if bought")])


# ---------------------------------------------------------------------------
def order_qty(line, kits, loose_parts):
    ref, *_ , unit, qpk, status, note = line
    need = qpk * kits
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
           "**Before ordering:** prices and stock are snapshots from 2026-10-13/14. DigiKey and "
           "Mouser blocked automated page fetches, so nothing was confirmed live. Import the "
           "`*.digikey-import.csv` or `*.mouser-import.csv` file into the distributor's BOM tool, "
           "and the cart will show today's price and stock. Lines whose `status` is `estimate` "
           "have no live figure at all.", "",
           "## Options", "",
           "| Option | Builds | Order total | Per build | Files |",
           "|---|---|---|---|---|"]
    for key, o, total in rows:
        out.append(f"| {o['title']} | {o['kits']} | ${total:,.0f} | ${total / o['kits']:,.0f} | "
                   f"[`{key}.csv`]({key}.csv) |")
    out += ["",
            "The Pod-0 PCB total includes two spares of each chip for hand assembly and "
            "cut-tape minimums on passives. Team-shared items (lux meter, header pack, "
            "PCB order) are split across builders in the per-build column.",
            "",
            "## Suggested combinations", "",
            "- **Start this week, cheapest:** bench kit A (reuses goggl1 boards) plus the "
            "AliExpress optics tier. The bench kits are complete for electronics; optics come "
            "from optics-mech.",
            "- **Recommended first order:** bench kit A or B for all three builders, plus "
            "optics-mech (Edmund tier). This builds Pod-0 cartridges A and B on the bench and "
            "runs the brightness test.",
            "- **Once Pod-0 works on the bench:** the Pod-0 PCB set. Order the OLED panels first; "
            "they are the long-lead item.",
            "- **Experimental branch, month 3:** concept 4 or concept 5, one set for the team.",
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
