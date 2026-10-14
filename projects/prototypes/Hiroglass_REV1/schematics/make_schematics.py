#!/usr/bin/env python3
"""Generate the Hiroglass REV1 schematic and wiring sheets as SVG.

    python3 make_schematics.py      # writes sheets/*.svg and netlists/*.csv

Each sheet is a function below. Part numbers and pin maps come from
research_notes/BOM and schematics/ and from the bom/ CSVs; anything marked
"verify" there is marked on the sheet too.
"""
import csv
import os
import sys

from schem import Sheet, MUTED, POWER, ACCENT

HERE = os.path.dirname(os.path.abspath(__file__))
SHEETS = os.path.join(HERE, "sheets")
NETS = os.path.join(HERE, "netlists")


REGISTRY = {}


def save(s, name):
    REGISTRY[name] = s
    os.makedirs(SHEETS, exist_ok=True)
    os.makedirs(NETS, exist_ok=True)
    with open(os.path.join(SHEETS, name + ".svg"), "w") as f:
        f.write(s.svg())
    with open(os.path.join(NETS, name + ".csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["net", "connections"])
        for net, conns in s.netlist().items():
            w.writerow([net, " ".join(f"{r}.{p}" for r, p in conns)])
    bad = s.check()
    if bad and not name.startswith("pod0-"):
        print(f"{name}: single-connection nets: {', '.join(bad)}", file=sys.stderr)
    return bad


def check_pod0():
    """Pod and puck share nets across three sheets: check them as one design."""
    from collections import defaultdict
    nets = defaultdict(set)
    for name, s in REGISTRY.items():
        if name.startswith("pod0-"):
            for n, conns in s.netlist().items():
                nets[n] |= {(name, r, p) for r, p in conns}
    bad = [n for n, c in nets.items() if len(c) < 2 and not n.startswith("TP_")]
    with open(os.path.join(NETS, "pod0-all.csv"), "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["net", "connections"])
        for n in sorted(nets):
            w.writerow([n, " ".join(f"{sh}:{r}.{p}" for sh, r, p in sorted(nets[n]))])
    print("pod0 cross-sheet check:", "ok" if not bad else "single-connection nets: " + ", ".join(bad))


QT = ("3V3", "GND", "SDA", "SCL")   # STEMMA QT / Qwiic port, in cable colour order
QT_COLORS = ("red", "black", "blue", "yellow")


def qt_cable(s, a, b, xs, label):
    """Four-wire STEMMA QT / Qwiic cable between two ports."""
    for (pin, col), x in zip(zip(QT, QT_COLORS), xs):
        s.wire((a[0], f"{a[1]} {pin}"), (b[0], f"{b[1]} {pin}"), col, via_x=x,
               label=label if pin == "3V3" else None)


def puck_harness(s, mcu, vin_pin, gnd_pin, mcu_x, x0=60, y0=640):
    """LiPo -> Adafruit 4755 (bq24074) -> switch extension -> MCU 5 V input."""
    s.board("LiPo 500 mAh", x0, y0, pins_right=("BATT +", "BATT −"), width=150,
            sub="Adafruit 1578 · JST-PH", color="#5b4a2e")
    s.board("bq24074 charger", x0 + 250, y0 - 40,
            pins_left=("BATT +", "BATT −", "", "USB-C in"),
            pins_right=("LOAD +", "LOAD −"), width=190, sub="Adafruit 4755")
    s.board("switch ext.", x0 + 540, y0 - 10, pins_left=("IN +", "IN −"),
            pins_right=("OUT +", "OUT −"), width=140, sub="Adafruit 1131 → 3814 lead",
            color="#3b4656")
    s.wire(("LiPo 500 mAh", "BATT +"), ("bq24074 charger", "BATT +"), "red", via_x=x0 + 200)
    s.wire(("LiPo 500 mAh", "BATT −"), ("bq24074 charger", "BATT −"), "black", via_x=x0 + 215)
    s.wire(("bq24074 charger", "LOAD +"), ("switch ext.", "IN +"), "red", via_x=x0 + 500)
    s.wire(("bq24074 charger", "LOAD −"), ("switch ext.", "IN −"), "black", via_x=x0 + 515)
    # The MCU's power pins face left, so go up and around to reach them.
    vx, gx = mcu_x - 30, mcu_x - 20
    (ox, oy), (gox, goy) = s.pins[("switch ext.", "OUT +")], s.pins[("switch ext.", "OUT −")]
    s.wire(("switch ext.", "OUT +"), (mcu, vin_pin), "red",
           path=[(ox + 15, oy), (ox + 15, y0 - 70), (vx, y0 - 70), (vx, s.pins[(mcu, vin_pin)][1])])
    s.wire(("switch ext.", "OUT −"), (mcu, gnd_pin), "black",
           path=[(gox + 30, goy), (gox + 30, y0 - 80), (gx, y0 - 80), (gx, s.pins[(mcu, gnd_pin)][1])])


# ---------------------------------------------------------------------------
def bench_a():
    s = Sheet("Bench kit A · Beetle ESP32-C3", "Reuses goggl1's board and SPI pin map",
              "B-A", h=1000)
    s.board("Beetle ESP32-C3", 640, 150,
            pins_left=("VIN (5 V)", "GND", "BAT  (leave open)"),
            pins_right=("3V3", "GND", "GPIO4  SCK", "GPIO6  MOSI", "GPIO7  CS",
                        "GPIO1  DC", "GPIO2  RST", "", "GPIO5  SDA", "GPIO3  SCL"),
            width=230, sub="DFRobot DFR0868")
    s.board("OLED 1.3″ SSD1306", 1120, 130,
            pins_left=("Vin", "GND", "Clk", "Data", "CS", "DC", "Rst"),
            width=200, sub="Adafruit 938 · SPI mode")
    s.board("BMI270 IMU", 1120, 470,
            pins_left=("QT1 3V3", "QT1 GND", "QT1 SDA", "QT1 SCL"),
            pins_right=("QT2 3V3", "QT2 GND", "QT2 SDA", "QT2 SCL"),
            width=200, sub="SparkFun SEN-22397 · Qwiic · 0x68")
    s.board("VEML7700", 1390, 470,
            pins_left=("QT 3V3", "QT GND", "QT SDA", "QT SCL"),
            width=150, sub="Adafruit 4162 · 0x10")
    B, O = "Beetle ESP32-C3", "OLED 1.3″ SSD1306"
    for i, (p, q, c) in enumerate((("3V3", "Vin", "red"), ("GND", "GND", "black"),
                                   ("GPIO4  SCK", "Clk", "yellow"),
                                   ("GPIO6  MOSI", "Data", "blue"),
                                   ("GPIO7  CS", "CS", "orange"),
                                   ("GPIO1  DC", "DC", "green"),
                                   ("GPIO2  RST", "Rst", "purple"))):
        s.wire((B, p), (O, q), c, via_x=930 + i * 22)
    # I2C to the IMU via a Qwiic-to-male-header cable (Adafruit 4209)
    for i, (p, q, c) in enumerate((("3V3", "QT1 3V3", "red"), ("GND", "QT1 GND", "black"),
                                   ("GPIO5  SDA", "QT1 SDA", "blue"),
                                   ("GPIO3  SCL", "QT1 SCL", "yellow"))):
        s.wire((B, p), ("BMI270 IMU", q), c, via_x=(1100, 1088, 1010, 1024)[i],
               label="QT→header cable 4209" if i == 2 else None)
    qt_cable(s, ("BMI270 IMU", "QT2"), ("VEML7700", "QT"), (1340, 1352, 1364, 1376),
             "QT cable 4399")
    puck_harness(s, B, "VIN (5 V)", "GND", 640, x0=60, y0=520)
    s.note(60, 130, 470, "Bench A in one look", [
        "Display keeps goggl1's SPI pins, so the Forth vocabulary",
        "runs as-is; the SSD1306 init differs slightly from SSD1309.",
        "I2C is new, on GPIO5 / GPIO3, clear of strapping pins 8 and 9.",
        "BMI270 at 0x68, VEML7700 at 0x10.",
        "SPI on the 938: cut its two jumpers and solder the header.",
        "GPIO2 (RST) is a strapping pin: if boot misbehaves,",
        "move RST to GPIO0 or tie the OLED's Rst high.",
    ])
    s.note(60, 820, 700, "Power and safety", [
        "Solder the 4755's charge jumper to 500 mA: the default 1 A is 2C for a 500 mAh cell.",
        "Charger LOAD goes to the Beetle's VIN pin only, never to 3V3 or BAT.",
        "The Beetle has its own LiPo charger; leave its BAT pin open in this kit.",
        "Unplug the puck while flashing over the Beetle's USB-C.",
    ])
    s.note(800, 820, 330, "Verify before wiring", [
        "Pin labels on the 938, 22397 and 4755",
        "are from Adafruit/SparkFun docs and",
        "memory; check each silkscreen.",
    ])
    return save(s, "bench-a-beetle")


def bench_b():
    s = Sheet("Bench kit B · QT Py ESP32-C3", "Everything on STEMMA QT, no soldering on the bench",
              "B-B", h=1000)
    s.board("QT Py ESP32-C3", 120, 150,
            pins_right=("QT 3V3", "QT GND", "QT SDA", "QT SCL"),
            pins_left=("5V", "GND"),
            width=200, sub="Adafruit 5405 · SDA=GPIO5 SCL=GPIO6")
    s.board("OLED 1.3″ SSD1306", 520, 150,
            pins_left=("QT1 3V3", "QT1 GND", "QT1 SDA", "QT1 SCL"),
            pins_right=("QT2 3V3", "QT2 GND", "QT2 SDA", "QT2 SCL"),
            width=200, sub="Adafruit 938 · I2C default · 0x3D")
    s.board("BMI270 IMU", 920, 150,
            pins_left=("QT1 3V3", "QT1 GND", "QT1 SDA", "QT1 SCL"),
            pins_right=("QT2 3V3", "QT2 GND", "QT2 SDA", "QT2 SCL"),
            width=200, sub="SparkFun SEN-22397 · 0x68")
    s.board("VEML7700", 1320, 150,
            pins_left=("QT 3V3", "QT GND", "QT SDA", "QT SCL"),
            width=170, sub="Adafruit 4162 · 0x10")
    qt_cable(s, ("QT Py ESP32-C3", "QT"), ("OLED 1.3″ SSD1306", "QT1"), (400, 412, 424, 436),
             "QT 4210")
    qt_cable(s, ("OLED 1.3″ SSD1306", "QT2"), ("BMI270 IMU", "QT1"), (800, 812, 824, 836),
             "QT 4210")
    qt_cable(s, ("BMI270 IMU", "QT2"), ("VEML7700", "QT1".replace("1", "")), (1200, 1212, 1224, 1236),
             "QT 4210")
    puck_harness(s, "QT Py ESP32-C3", "5V", "GND", 120, x0=60, y0=520)
    s.note(980, 470, 520, "Bench B in one look", [
        "One I2C bus daisy-chained over STEMMA QT: QT Py → OLED → IMU → light.",
        "Addresses: OLED 0x3D (0x3C with jumper), BMI270 0x68, VEML7700 0x10.",
        "The display moves to I2C, so goggl1's SPI driver needs an I2C path;",
        "at 400 kHz a full 1 KB frame takes about 25 ms.",
        "QT Py ESP32-C3 is often out of stock. The Seeed XIAO ESP32C3",
        "works too but needs headers soldered and a QT→header cable.",
    ])
    s.note(980, 640, 520, "Optional: SPI display on the QT Py", [
        "Cut the 938's two jumpers and solder its header, then:",
        "SCK GPIO10 → Clk · MO GPIO7 → Data · A0 GPIO4 → CS",
        "A1 GPIO3 → DC · A2 GPIO1 → Rst · 3V → Vin",
    ])
    s.note(60, 820, 700, "Power and safety", [
        "On the bench the QT Py's own USB-C powers everything; the puck is optional.",
        "Puck mode needs one soldered header on the QT Py's 5V / GND.",
        "Set the 4755 charge jumper to 500 mA. Keep LOAD off the 3V pin.",
    ])
    return save(s, "bench-b-qtpy")


# ---------------------------------------------------------------------------
# Pod-0 custom boards. Pins from the datasheets listed in
# research_notes/BOM and schematics/pod0_pcb_parts.md.

def pod0_1_mcu():
    s = Sheet("Pod-0 pod · MCU, USB, power", "ESP32-C3-MINI-1, USB-C, 3.3 V LDO, tether",
              "P-1")
    # USB-C receptacle, all 24 pins brought out (DisplayPort lanes to test pads)
    s.ic("J2", "USB-C receptacle, 24 pin", 70, 110, width=150,
         mpn="Amphenol 12401610E4#2A (alt GCT USB4105-GF-A, 16 pin)",
         right=[("A4 A9 B4 B9", "VBUS", "VBUS_POD"),
                ("A1 A12 B1 B12", "GND", "GND"),
                ("A5", "CC1", "CC1"), ("B5", "CC2", "CC2"),
                ("A6 B6", "D+", "USB_C_DP"), ("A7 B7", "D−", "USB_C_DN"),
                ("A8", "SBU1", "TP_SBU1"), ("B8", "SBU2", "TP_SBU2"),
                ("A2", "TX1+", "TP_TX1P"), ("A3", "TX1−", "TP_TX1N"),
                ("B11", "RX1+", "TP_RX1P"), ("B10", "RX1−", "TP_RX1N"),
                ("B2", "TX2+", "TP_TX2P"), ("B3", "TX2−", "TP_TX2N"),
                ("A11", "RX2+", "TP_RX2P"), ("A10", "RX2−", "TP_RX2N"),
                ("S", "SHIELD", "GND")],
         top="lanes → test pads (DP alt mode later)")
    s.two_pin("R", "R8", "5.1k", 90, 560, "CC1", "GND", mpn="Yageo RC0402FR-075K1L")
    s.two_pin("R", "R9", "5.1k", 170, 560, "CC2", "GND", mpn="Yageo RC0402FR-075K1L")
    s.two_pin("C", "C9", "1µF", 250, 560, "VBUS_POD", "GND", mpn="Samsung CL05A105KO5NNNC")

    # ESD and series resistors
    s.ic("U5", "USBLC6-2SC6", 520, 110, width=120, mpn="ST USBLC6-2SC6",
         left=[("1", "I/O1", "USB_C_DN"), ("3", "I/O2", "USB_C_DP"), ("2", "GND", "GND")],
         right=[("6", "I/O1", "USB_C_DN"), ("4", "I/O2", "USB_C_DP"), ("5", "VBUS", "VBUS_POD")],
         top="pins: verify vs. ST datasheet")
    s.two_pin("R", "R10", "0Ω", 470, 290, "USB_C_DP", "USB_DP", vertical=False,
              mpn="Yageo RC0402JR-070RL")
    s.two_pin("R", "R11", "0Ω", 470, 350, "USB_C_DN", "USB_DN", vertical=False,
              mpn="Yageo RC0402JR-070RL")

    # LDO
    s.ic("U4", "AP2112K-3.3 (alt TLV75733P)", 520, 470, width=130,
         mpn="Diodes AP2112K-3.3TRG1",
         left=[("1", "VIN", "VSYS"), ("3", "EN", "PWR_EN"), ("2", "GND", "GND")],
         right=[("5", "VOUT", "+3V3"), ("4", "NC", None)], top="600 mA · same pinout as TLV757P")
    s.two_pin("C", "C7", "4.7µF", 470, 650, "VSYS", "GND", mpn="Samsung CL10A475KP8NNNC")
    s.two_pin("R", "R7", "100k", 550, 650, "PWR_EN", "GND", mpn="Yageo RC0402FR-07100KL")
    s.two_pin("C", "C8", "1µF", 630, 650, "+3V3", "GND", mpn="Samsung CL05A105KO5NNNC")

    # MCU
    s.ic("U1", "ESP32-C3-MINI-1-N4", 1010, 110, width=210, mpn="Espressif ESP32-C3-MINI-1-N4",
         left=[("3", "3V3", "+3V3"), ("1 2 11 14 36–53", "GND", "GND"), ("8", "EN", "EN"),
               ("23", "IO9", "BOOT"), ("22", "IO8", "LED_N"),
               ("26", "IO18 D−", "USB_DN"), ("27", "IO19 D+", "USB_DP"),
               ("30", "IO20 RXD0", "GAUGE_ALRT"), ("31", "IO21 TXD0", "TP_TXD0")],
         right=[("18", "IO4", "OLED_SCLK"), ("20", "IO6", "OLED_SDIN"),
                ("21", "IO7", "OLED_CS_N"), ("13", "IO1", "OLED_DC"),
                ("6", "IO3", "OLED_RES_N"), ("16", "IO10", "OLED_VBAT_EN"),
                ("19", "IO5", "I2C_SDA"), ("5", "IO2", "I2C_SCL"),
                ("12", "IO0", "IMU_INT1")],
         top="NC: 4 7 9 10 15 17 24 25 28 29 32–35")
    s.two_pin("C", "C1", "10µF", 1030, 420, "+3V3", "GND", mpn="Samsung CL10A106KP8NNNC")
    s.two_pin("C", "C2", "100nF", 1110, 420, "+3V3", "GND", mpn="Samsung CL05B104KO5NNNC")

    # reset, boot, LED
    s.two_pin("R", "R1", "10k", 760, 560, "+3V3", "EN", mpn="Yageo RC0402FR-0710KL")
    s.two_pin("C", "C3", "1µF", 840, 560, "EN", "GND", mpn="Samsung CL05A105KO5NNNC")
    s.two_pin("SW", "SW1", "RESET", 920, 560, "EN", "GND", mpn="C&K PTS810 SJM 250 SMTR LFS")
    s.two_pin("R", "R2", "10k", 760, 760, "+3V3", "BOOT", mpn="Yageo RC0402FR-0710KL")
    s.two_pin("SW", "SW2", "BOOT", 840, 760, "BOOT", "GND", mpn="C&K PTS810 SJM 250 SMTR LFS")
    s.two_pin("R", "R3", "10k", 1240, 560, "+3V3", "LED_N", mpn="Yageo RC0402FR-0710KL")
    s.two_pin("R", "R4", "1k", 1320, 560, "+3V3", "LED_A", mpn="Yageo RC0402FR-071KL")
    s.two_pin("LED", "D1", "green", 1320, 700, "LED_A", "LED_N", mpn="Lite-On LTST-C191KGKT")
    s.two_pin("R", "R17", "10k", 1400, 560, "+3V3", "GAUGE_ALRT", mpn="Yageo RC0402FR-0710KL")

    # tether to the puck
    s.ic("J3", "Tether, JST SH 10", 70, 790, width=140, mpn="JST SM10B-SRSS-TB(LF)(SN)",
         right=[("1", "VBUS_POD", "VBUS_POD"), ("2", "VSYS", "VSYS"), ("3", "VSYS", "VSYS"),
                ("4", "GND", "GND"), ("5", "GND", "GND"), ("6", "VCELL", "VCELL"),
                ("7", "PWR_EN", "PWR_EN"), ("8", "SDA", "I2C_SDA"), ("9", "SCL", "I2C_SCL"),
                ("10", "ALRT", "GAUGE_ALRT")])
    s.note(420, 830, 560, "Notes", [
        "VBUS from this USB-C goes down the tether to the puck charger;",
        "the pod runs only from VSYS (4.4 V on USB, cell voltage on battery).",
        "PWR_EN comes from the puck's slide switch; R7 holds the LDO off.",
        "GPIO8 is a strapping pin: the LED pulls it high, R3 makes sure.",
        "VCELL feeds the OLED supply switch on sheet P-2.",
    ])
    return save(s, "pod0-1-mcu-usb-power")


def pod0_2_display_sensors():
    s = Sheet("Pod-0 pod · display and sensors", "0.96″ SSD1306 panel on FPC, BMI270, VEML7700",
              "P-2")
    nets = {1: "GND", 2: "OLED_C2P", 3: "OLED_C2N", 4: "OLED_C1P", 5: "OLED_C1N",
            6: "OLED_VBAT", 7: None, 8: "GND", 9: "+3V3", 10: "GND", 11: "GND", 12: "GND",
            13: "OLED_CS_N", 14: "OLED_RES_N", 15: "OLED_DC", 16: "OLED_RW", 17: "OLED_ERD",
            18: "OLED_SCLK", 19: "OLED_SDIN", 20: None, 21: "GND", 22: "GND", 23: "GND",
            24: "GND", 25: "GND", 26: "OLED_IREF", 27: "OLED_VCOMH", 28: "OLED_VCC",
            29: "GND", 30: "GND"}
    names = {1: "NC/GND", 2: "C2P", 3: "C2N", 4: "C1P", 5: "C1N", 6: "VBAT", 7: "NC",
             8: "VSS", 9: "VDD", 10: "BS0", 11: "BS1", 12: "BS2", 13: "CS#", 14: "RES#",
             15: "D/C#", 16: "R/W#", 17: "E/RD#", 18: "D0 SCLK", 19: "D1 SDIN", 20: "D2",
             21: "D3", 22: "D4", 23: "D5", 24: "D6", 25: "D7", 26: "IREF", 27: "VCOMH",
             28: "VCC", 29: "VLSS", 30: "NC/GND"}
    pins = [(str(n), names[n], nets[n]) for n in range(1, 31)]
    s.ic("J1", "FH12-30S-0.5SH ← 0.96″ OLED", 190, 110, width=170,
         mpn="Hirose FH12-30S-0.5SH(55); panel Orient AOM12864A0-0.96WW",
         left=pins[:15], right=pins[15:], top="BS[2:0]=000 → 4-wire SPI · D2 open")
    # panel support parts
    xs = iter(range(70, 1600, 95))
    y = 540
    for kind, ref, val, a, b, mpn in (
            ("C", "C10", "1µF", "OLED_C2P", "OLED_C2N", "Murata GRM188R61C105KA93D"),
            ("C", "C11", "1µF", "OLED_C1P", "OLED_C1N", "Murata GRM188R61C105KA93D"),
            ("C", "C12", "1µF", "OLED_VBAT", "GND", "Samsung CL05A105KO5NNNC"),
            ("C", "C13", "1µF", "+3V3", "GND", "Samsung CL05A105KO5NNNC"),
            ("R", "R12", "620k", "OLED_IREF", "GND", "Yageo RC0402FR-07620KL"),
            ("C", "C14", "4.7µF", "OLED_VCOMH", "GND", "Murata GRM21BR71C475KE51L"),
            ("C", "C15", "2.2µF", "OLED_VCC", "GND", "Murata GRM188R71C225KE11D"),
            ("R", "R13", "4.7k", "OLED_RW", "GND", "Yageo RC0402FR-074K7L"),
            ("R", "R14", "4.7k", "OLED_ERD", "GND", "Yageo RC0402FR-074K7L")):
        s.two_pin(kind, ref, val, next(xs) + 20, y, a, b, mpn=mpn)
    # OLED supply switch
    s.ic("Q1", "FDN338P  P-FET", 820, 130, width=120, mpn="onsemi FDN338P",
         left=[("2", "S", "VCELL"), ("1", "G", "Q1_G")], right=[("3", "D", "OLED_VBAT")])
    s.ic("Q2", "FDN335N  N-FET", 820, 300, width=120, mpn="onsemi FDN335N",
         left=[("1", "G", "OLED_VBAT_EN"), ("2", "S", "GND")], right=[("3", "D", "Q1_G")])
    s.two_pin("R", "R15", "47k", 1120, 130, "VCELL", "Q1_G", mpn="Yageo RC0402FR-0747KL")
    s.two_pin("R", "R16", "47k", 1120, 300, "OLED_VBAT_EN", "GND", mpn="Yageo RC0402FR-0747KL")
    # IMU
    s.ic("U2", "BMI270", 1250, 110, width=130, mpn="Bosch BMI270",
         left=[("8", "VDD", "+3V3"), ("5", "VDDIO", "+3V3"), ("12", "CSB", "+3V3"),
               ("1", "SDO", "GND"), ("6", "GNDIO", "GND"), ("7", "GND", "GND"),
               ("2", "ASDx", None), ("3", "ASCx", None)],
         right=[("14", "SDx", "I2C_SDA"), ("13", "SCx", "I2C_SCL"), ("4", "INT1", "IMU_INT1"),
                ("9", "INT2", None), ("10", "OCSB", None), ("11", "OSDO", None)],
         top="I2C 0x68 (SDO low)")
    # light sensor
    s.ic("U3", "VEML7700-TT", 1250, 400, width=130, mpn="Vishay VEML7700-TT",
         left=[("2", "VDD", "+3V3"), ("3", "GND", "GND")],
         right=[("4", "SDA", "I2C_SDA"), ("1", "SCL", "I2C_SCL")], top="I2C 0x10")
    for i, (kind, ref, val, a, b, mpn) in enumerate((
            ("C", "C4", "100nF", "+3V3", "GND", "Samsung CL05B104KO5NNNC"),
            ("C", "C5", "100nF", "+3V3", "GND", "Samsung CL05B104KO5NNNC"),
            ("C", "C6", "100nF", "+3V3", "GND", "Samsung CL05B104KO5NNNC"),
            ("R", "R5", "4.7k", "+3V3", "I2C_SDA", "Yageo RC0402FR-074K7L"),
            ("R", "R6", "4.7k", "+3V3", "I2C_SCL", "Yageo RC0402FR-074K7L"))):
        s.two_pin(kind, ref, val, 980 + i * 95, 540, a, b, mpn=mpn)
    s.note(70, 730, 640, "Display notes", [
        "The panel's charge pump needs VBAT 3.5–4.2 V (4.5 V max), so it runs from the",
        "cell through Q1, switched by GPIO10. Below ~3.5 V cell the panel dims.",
        "Firmware must send 8D 14 (charge pump on); goggl1's SSD1309 code does not.",
        "IREF: 620k is the panel datasheet value; the SSD1306 formula gives 470–560k.",
        "Leave R12 on one 0402 footprint and tune for brightness.",
        "Panel glass is 26.7 mm wide: it sits in the optics cartridge; only the FPC",
        "reaches this board. Winstar alternate: confirm its 30-pin map first.",
    ])
    s.note(760, 730, 520, "Sensor notes", [
        "BMI270 ASDx/ASCx: leave open or tie to VDDIO, never to GND.",
        "The IMU sits on the optics, not the battery: mount it rigidly",
        "and record its offset from the display (binocular hook).",
        "One I2C bus: BMI270 0x68, VEML7700 0x10, MAX17048 0x36 (puck).",
    ])
    return save(s, "pod0-2-display-sensors")


def pod0_3_puck():
    s = Sheet("Pod-0 puck · charger and battery", "BQ24074, 500 mAh LiPo with NTC, power switch, fuel gauge",
              "P-3")
    s.ic("J10", "USB-C receptacle, 16 pin", 70, 110, width=150, mpn="GCT USB4105-GF-A",
         right=[("A4 B9", "VBUS", "VBUS_PUCK"), ("B4 A9", "VBUS", "VBUS_PUCK"),
                ("A1 B12", "GND", "GND"), ("B1 A12", "GND", "GND"),
                ("A5", "CC1", "PCC1"), ("B5", "CC2", "PCC2"),
                ("A6 B6", "D+", None), ("A7 B7", "D−", None),
                ("A8 B8", "SBU", None), ("S", "SHIELD", "GND")], top="charge only, no data")
    s.two_pin("R", "R20", "5.1k", 90, 420, "PCC1", "GND", mpn="Yageo RC0402FR-075K1L")
    s.two_pin("R", "R21", "5.1k", 170, 420, "PCC2", "GND", mpn="Yageo RC0402FR-075K1L")
    s.two_pin("D", "D10", "PMEG2010AEH", 500, 150, "VBUS_PUCK", "CHG_IN", vertical=False,
              mpn="Nexperia PMEG2010AEH")
    s.two_pin("D", "D11", "PMEG2010AEH", 500, 240, "VBUS_POD", "CHG_IN", vertical=False,
              mpn="Nexperia PMEG2010AEH")
    s.two_pin("C", "C20", "1µF 25V", 540, 320, "CHG_IN", "GND", mpn="Samsung CL10A105KA8NNNC")
    s.ic("U10", "BQ24074RGT", 820, 110, width=150, mpn="TI BQ24074RGTR",
         left=[("13", "IN", "CHG_IN"), ("4", "CE#", "GND"), ("5", "EN2", "GND"),
               ("6", "EN1", "EN1"), ("16", "ISET", "ISET"), ("12", "ILIM", "ILIM"),
               ("14", "TMR", None), ("15", "ITERM", None), ("8 EP", "VSS", "GND")],
         right=[("10 11", "OUT", "VSYS"), ("2 3", "BAT", "VCELL"), ("1", "TS", "NTC_TS"),
                ("7", "PGOOD#", "PGOOD_N"), ("9", "CHG#", "CHG_N")],
         top="EN2=0 EN1=1 → 500 mA USB · ISET 500 mA")
    for i, (kind, ref, val, a, b, mpn) in enumerate((
            ("R", "R22", "10k", "VSYS", "EN1", "Yageo RC0402FR-0710KL"),
            ("R", "R23", "1.78k 1%", "ISET", "GND", "Yageo RC0402FR-071K78L"),
            ("R", "R24", "3.09k 1%", "ILIM", "GND", "Yageo RC0402FR-073K09L"),
            ("C", "C21", "4.7µF", "VSYS", "GND", "Samsung CL10A475KP8NNNC"),
            ("C", "C22", "4.7µF", "VCELL", "GND", "Samsung CL10A475KP8NNNC"),
            ("NTC", "RT1", "10k NTC", "NTC_TS", "GND", "Murata NCP15XH103F03RC"))):
        s.two_pin(kind, ref, val, 620 + i * 100, 470, a, b, mpn=mpn)
    s.two_pin("R", "R25", "1.5k", 1200, 130, "VSYS", "PG_A", mpn="Yageo RC0402FR-071K5L")
    s.two_pin("LED", "D12", "green", 1200, 230, "PG_A", "PGOOD_N", mpn="Lite-On LTST-C191KGKT")
    s.two_pin("R", "R26", "1.5k", 1290, 130, "VSYS", "CHG_A", mpn="Yageo RC0402FR-071K5L")
    s.two_pin("LED", "D13", "red", 1290, 230, "CHG_A", "CHG_N", mpn="Lite-On LTST-C191KRKT")
    s.ic("J11", "Battery, JST PH 2", 1400, 420, width=120, mpn="JST S2B-PH-SM4-TB",
         left=[("1", "+", "VCELL"), ("2", "−", "GND")], top="verify cell polarity")
    s.ic("SW10", "Slide switch", 1180, 640, width=120, mpn="C&K PCM12SMTR",
         left=[("1", "A", "VSYS"), ("3", "B", None)], right=[("2", "COM", "PWR_EN")],
         top="pod LDO enable, µA only")
    s.ic("U11", "MAX17048 (optional)", 160, 640, width=150, mpn="ADI MAX17048G+T10",
         left=[("2", "CELL", "VCELL"), ("3", "VDD", "VCELL"), ("1", "CTG", "GND"),
               ("6", "QSTRT", "GND"), ("4 EP", "GND", "GND")],
         right=[("8", "SDA", "I2C_SDA"), ("7", "SCL", "I2C_SCL"), ("5", "ALRT", "GAUGE_ALRT")],
         top="0x36 · pins: verify vs. ADI datasheet")
    s.two_pin("C", "C23", "1µF", 470, 640, "VCELL", "GND", mpn="Samsung CL05A105KO5NNNC")
    s.ic("J12", "Tether, JST SH 10", 600, 680, width=140, mpn="JST SM10B-SRSS-TB(LF)(SN)",
         right=[("1", "VBUS_POD", "VBUS_POD"), ("2", "VSYS", "VSYS"), ("3", "VSYS", "VSYS"),
                ("4", "GND", "GND"), ("5", "GND", "GND"), ("6", "VCELL", "VCELL"),
                ("7", "PWR_EN", "PWR_EN"), ("8", "SDA", "I2C_SDA"), ("9", "SCL", "I2C_SCL"),
                ("10", "ALRT", "GAUGE_ALRT")], top="same pin order as J3 on the pod")
    s.note(960, 790, 600, "Charger settings (TI SLUS810N)", [
        "ISET 1.78k → 890/R = 500 mA fast charge (±10 %), 1C for a 500 mAh cell.",
        "ILIM 3.09k → 1550/R = 502 mA input limit; must be fitted.",
        "TS + 10k NTC on the cell: charging only between 0 and 50 °C.",
        "TMR and ITERM open: default timers and 10 % termination.",
        "OUT (VSYS) is 4.4 V on USB and follows the cell on battery.",
        "Charge from either USB-C: D10/D11 diode-OR the two VBUS lines.",
    ])
    return save(s, "pod0-3-puck")


# ---------------------------------------------------------------------------
# Experimental branches: system wiring, board level.

def concept4_fpga_microoled():
    s = Sheet("Concept 4 · micro-OLED on an open driver", "Pico 2 PIO (or iCEBreaker PMOD) → 1.8 V parallel bus",
              "X-4", h=1100)
    s.board("Pod MCU · ESP32-C3", 60, 150, pins_right=("GPIO21 TX", "GPIO20 RX", "GND", "VSYS"),
            width=190, sub="Pod-0 board, sheet P-1")
    s.board("Pico 2 · RP2350", 420, 110,
            pins_left=("GP1 UART0 RX", "GP0 UART0 TX", "GND", "VSYS"),
            pins_right=("GP2–GP9  D0–D7", "GP10  PCLK", "GP11  HSYNC", "GP12  VSYNC",
                        "GP13  DE", "GP18  SPI0 SCK", "GP19  SPI0 TX", "GP17  SPI0 CS",
                        "GP20  XCLR", "3V3(OUT)", "GND "),
            width=230, sub="Raspberry Pi SC1631 · PIO drives the bus")
    s.board("Level shift ×2", 840, 110,
            pins_left=("A  D0–D7", "A  PCLK", "A  HSYNC", "A  VSYNC", "A  DE", "A  SCK",
                       "A  SDI", "A  CS", "A  XCLR", "VCCA 3V3", "GND"),
            pins_right=("B  D0–D7", "B  PCLK", "B  HSYNC", "B  VSYNC", "B  DE", "B  SCK",
                        "B  SDI", "B  CS", "B  XCLR", "VCCB 1V8", "GND "),
            width=200, sub="TI SN74AXC8T245 ×2 · DIR = A→B")
    s.board("Micro-OLED panel", 1240, 110,
            pins_left=("D0–D7  (YCbCr 8-bit)", "PCLK", "HSYNC", "VSYNC", "DE", "SCK", "SDI",
                       "CS", "XCLR", "VDD 1V8", "GND", "VPANEL (?)"),
            width=230, sub="Sony ECX336-class 0.23″ 640×400", color="#5b2b22")
    s.board("1.8 V LDO", 840, 560, pins_left=("VIN", "GND"), pins_right=("VOUT 1V8",), width=170,
            sub="TI TLV75718PDBVR")
    s.board("Panel rail", 1240, 560, pins_left=("VIN", "GND"), pins_right=("VPANEL",), width=170,
            sub="per panel datasheet: unknown", color="#5b2b22")
    E, P, L, O = "Pod MCU · ESP32-C3", "Pico 2 · RP2350", "Level shift ×2", "Micro-OLED panel"
    s.wire((E, "GPIO21 TX"), (P, "GP1 UART0 RX"), "green", via_x=330, label="UART cmds")
    s.wire((E, "GPIO20 RX"), (P, "GP0 UART0 TX"), "purple", via_x=345)
    s.wire((E, "GND"), (P, "GND"), "black", via_x=360)
    s.wire((E, "VSYS"), (P, "VSYS"), "red", via_x=375)
    sig = (("GP2–GP9  D0–D7", "A  D0–D7", "B  D0–D7", "D0–D7  (YCbCr 8-bit)", "blue"),
           ("GP10  PCLK", "A  PCLK", "B  PCLK", "PCLK", "orange"),
           ("GP11  HSYNC", "A  HSYNC", "B  HSYNC", "HSYNC", "yellow"),
           ("GP12  VSYNC", "A  VSYNC", "B  VSYNC", "VSYNC", "yellow"),
           ("GP13  DE", "A  DE", "B  DE", "DE", "yellow"),
           ("GP18  SPI0 SCK", "A  SCK", "B  SCK", "SCK", "green"),
           ("GP19  SPI0 TX", "A  SDI", "B  SDI", "SDI", "green"),
           ("GP17  SPI0 CS", "A  CS", "B  CS", "CS", "green"),
           ("GP20  XCLR", "A  XCLR", "B  XCLR", "XCLR", "purple"))
    for i, (pp, a, b, o, c) in enumerate(sig):
        s.wire((P, pp), (L, a), c, via_x=700 + i * 12, label="×8" if i == 0 else None)
        s.wire((L, b), (O, o), c, via_x=1100 + i * 12, label="×8" if i == 0 else None)
    s.wire((P, "3V3(OUT)"), (L, "VCCA 3V3"), "red", via_x=812)
    s.wire((P, "GND "), (L, "GND"), "black", via_x=826)
    s.wire(("1.8 V LDO", "VOUT 1V8"), (L, "VCCB 1V8"), "red",
           path=[(1060, s.pins[("1.8 V LDO", "VOUT 1V8")][1]), (1060, 440), (1080, 440),
                 (1080, s.pins[(L, "VCCB 1V8")][1])])
    s.wire(("1.8 V LDO", "VOUT 1V8"), (O, "VDD 1V8"), "red",
           path=[(1060, s.pins[("1.8 V LDO", "VOUT 1V8")][1]), (1060, 470), (1215, 470),
                 (1215, s.pins[(O, "VDD 1V8")][1])])
    s.wire(("Panel rail", "VPANEL"), (O, "VPANEL (?)"), "red",
           path=[(1500, s.pins[("Panel rail", "VPANEL")][1]), (1500, 520), (1225, 520),
                 (1225, s.pins[(O, "VPANEL (?)")][1])])
    s.note(60, 560, 700, "What is and isn't known", [
        "Known: Brilliant Labs Frame drives this panel class with an 8-bit YCbCr",
        "parallel bus plus SPI configuration (Frame hardware docs).",
        "Not known: Sony publishes no datasheet, so connector pinout, the panel",
        "rail voltages and the init sequence are unverified. Treat the panel side",
        "of this sheet as a signal list, not a pinout.",
        "Bus budget: ~17 signals; Pico 2 has 26 GPIO, so the IMU fits on I2C too.",
        "Level shifting assumes 1.8 V panel logic; drop the shifters if it is 3.3 V.",
    ])
    s.note(60, 790, 700, "iCEBreaker instead of Pico 2", [
        "PMOD 1A + 1B as a double PMOD: 16 I/O for D0–D7 and the four timing lines;",
        "PMOD 2 for SPI config + XCLR. Same level shifters. Toolchain: Yosys + nextpnr.",
        "Reference: Frame's RTL (check its license before reuse).",
    ])
    s.note(800, 790, 420, "Pod-0 pins on this branch", [
        "VSYS comes from the Pod-0 puck tether (sheet P-3).",
        "GPIO20 is the fuel-gauge alert on Pod-0: leave",
        "the optional gauge off, or move the UART pins.",
    ])
    return save(s, "concept4-microoled-driver")


def concept5_linux_puck():
    s = Sheet("Concept 5 · Linux puck through Monado", "Pi 5 (or ROCK 5B) renders; glasses display; USB IMU for 3DoF",
              "X-5", h=1100)
    s.board("27 W USB-C PSU", 60, 150, pins_right=("USB-C PD out",), width=170,
            sub="Raspberry Pi SC1157/SC1158")
    s.board("Puck · Raspberry Pi 5", 420, 120,
            pins_left=("USB-C power in", "", "CSI/DSI 0", "CSI/DSI 1"),
            pins_right=("micro-HDMI 0", "USB 3.0 port", "USB 2.0 port", "BLE (onboard)"),
            width=230, sub="SC1112 8 GB · Monado + OpenXR apps")
    s.board("HDMI → USB-C adapter", 820, 110,
            pins_left=("HDMI in", "USB-C 5 V in"), pins_right=("USB-C DP alt out",), width=200,
            sub="XREAL Adapter or Viture dock (not at DigiKey)")
    s.board("Birdbath glasses", 1240, 110, pins_left=("USB-C  DP alt + 5 V",), width=220,
            sub="XREAL One / Viture · borrowed", color="#5b2b22")
    s.board("IMU bridge · QT Py RP2040", 820, 380,
            pins_left=("USB-C",), pins_right=("QT 3V3", "QT GND", "QT SDA", "QT SCL", "A0  INT",
                                             "A1  RST"),
            width=210, sub="Adafruit 4900 · USB CDC/HID to Monado")
    s.board("BNO085 IMU", 1240, 380,
            pins_left=("QT 3V3", "QT GND", "QT SDA", "QT SCL", "INT", "RST"), width=170,
            sub="Adafruit 4754 · 0x4A · mount on the optics")
    s.board("Camera Module 3 ×2", 60, 420, pins_right=("cam L  22-pin", "cam R  22-pin"), width=200,
            sub="SC0872 · stereo pair for 6DoF (Basalt)")
    s.board("Phone · Switchback", 60, 700, pins_right=("BLE",), width=180, sub="settings, HUD widgets")
    s.wire(("27 W USB-C PSU", "USB-C PD out"), ("Puck · Raspberry Pi 5", "USB-C power in"), "red",
           via_x=330, label="5 V 5 A")
    s.wire(("Puck · Raspberry Pi 5", "micro-HDMI 0"), ("HDMI → USB-C adapter", "HDMI in"), "blue",
           via_x=730, label="HDMI")
    s.wire(("Puck · Raspberry Pi 5", "USB 2.0 port"), ("HDMI → USB-C adapter", "USB-C 5 V in"), "red",
           via_x=760, label="5 V")
    s.wire(("HDMI → USB-C adapter", "USB-C DP alt out"), ("Birdbath glasses", "USB-C  DP alt + 5 V"),
           "orange", via_x=1130, label="DP alt")
    s.wire(("Puck · Raspberry Pi 5", "USB 3.0 port"), ("IMU bridge · QT Py RP2040", "USB-C"), "purple",
           path=[(700, s.pins[("Puck · Raspberry Pi 5", "USB 3.0 port")][1]), (700, 0 + 455),
                 (790, 455), (790, s.pins[("IMU bridge · QT Py RP2040", "USB-C")][1])],
           label="USB")
    for i, (p, c) in enumerate((("QT 3V3", "red"), ("QT GND", "black"), ("QT SDA", "blue"),
                                ("QT SCL", "yellow"), ("A0  INT", "green"), ("A1  RST", "purple"))):
        q = p.replace("A0  ", "").replace("A1  ", "")
        s.wire(("IMU bridge · QT Py RP2040", p), ("BNO085 IMU", q), c, via_x=1130 + i * 14)
    s.wire(("Camera Module 3 ×2", "cam L  22-pin"), ("Puck · Raspberry Pi 5", "CSI/DSI 0"), "grey",
           via_x=340, label="CSI")
    s.wire(("Camera Module 3 ×2", "cam R  22-pin"), ("Puck · Raspberry Pi 5", "CSI/DSI 1"), "grey",
           via_x=355)
    s.wire(("Phone · Switchback", "BLE"), ("Puck · Raspberry Pi 5", "BLE (onboard)"), "green",
           path=[(300, s.pins[("Phone · Switchback", "BLE")][1]), (300, 800), (690, 800),
                 (690, s.pins[("Puck · Raspberry Pi 5", "BLE (onboard)")][1])], label="BLE sideband")
    s.note(820, 640, 700, "Why the adapter", [
        "The Pi 5's USB-C port is power-only: no DisplayPort alt mode. Birdbath",
        "glasses need DP over USB-C plus 5 V, hence the powered HDMI adapter.",
        "ROCK 5B instead: its USB-C carries DP alt mode, so the glasses plug in",
        "directly and the adapter goes away (needs ~30 W PD; not at DigiKey/Mouser).",
    ])
    s.note(820, 790, 700, "Software path", [
        "XRLinuxDriver (GPL-3.0) or Monado's own driver for the glasses, 3DoF first.",
        "Write a Monado driver for the IMU bridge: it is the same driver the",
        "monocular Pod-0 needs later over its own USB-C (binocular hook).",
        "6DoF: Basalt on the stereo pair; benchmark CPU load before choosing the SoC.",
    ])
    return save(s, "concept5-linux-puck")


if __name__ == "__main__":
    which = sys.argv[1:] or [n for n in dir() if n.startswith(("bench_", "pod0_", "concept"))]
    for name in which:
        globals()[name]()
    if any(n.startswith("pod0_") for n in which):
        check_pod0()
    print("wrote", sorted(os.listdir(SHEETS)))
