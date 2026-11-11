#!/usr/bin/env python3
"""Buildability check for the Hiroglass REV1 BOMs: where each line can be bought, how long it
takes, and what a run of 1 to 5 units costs, for a US buyer and for a Japan buyer.

    python3 sourcing.py                     # plan from the check date
    python3 sourcing.py --date 2026-11-17   # in-hand dates from another order date

Reads the line lists in make_bom.py. Writes BUILDABILITY.md, sourcing/<option>.<region>.csv
and sourcing/sourcing.json.

Two scenarios:
  US  buy only from US sellers shipping from US stock.
  JP  buy only from sellers that deliver to Japan: Japanese shops first, DigiKey Japan
      (ships from the US, duty paid), and sellers in China or Hong Kong where needed.

Every source below was checked on 2026-11-10. `conf` says how:
  page         price and stock read on the seller's own page
  snippet      seen in a search result, not on the page
  converted    DigiKey's stock is one worldwide inventory; the yen price was not read and is
               converted from the US price
A second pass on 2026-11-10 read Edmund (US and Japan), Thorlabs, Orient Display, Seeed,
PiShop, K&J, MonotaRO, Sengoku, Amazon and AliExpress in a normal Chrome window driven by
Playwright. DigiKey's US and Japan sites showed a Cloudflare check on all but three pages,
and Mouser refused, so the DigiKey US figures are still the ones read on digikey.com.mx.
  est          no figure found; a typical price, to be confirmed at order
Research notes with the sources: ../research_notes/BOM sourcing US and Japan/
"""
import argparse
import csv
import datetime as dt
import json
import math
import os

import make_bom

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "sourcing")
CHECKED = "2026-11-10"
RUNS = (1, 2, 3, 4, 5)
USD_JPY = 158.29   # x-rates.com, 2026-11-09
JP_TAX = 0.10      # consumption tax, added where a shop lists prices before tax
REGIONS = {"US": "US sellers, US stock", "JP": "Delivered to Japan (Japan or China sellers)"}

# ------------------------------------------------------------------ vendors (edit these)
# vendor: fee per order, currency, free-shipping threshold (None = always charged),
#         days from order to door, True if listed prices exclude Japan's consumption tax,
#         import cost for parcels from abroad into Japan (tax rate, carrier fee in yen),
#         how the fee and days were established.
def V(fee, cur, free, days, ex_tax=False, imp=(0, 0), conf="est", frm=""):
    return dict(fee=fee, cur=cur, free=free, days=days, ex_tax=ex_tax, imp=imp, conf=conf, frm=frm)


VENDORS = {
    "US": {
        "DigiKey": V(4.99, "USD", None, 5, conf="page", frm="Thief River Falls, MN"),
        "Adafruit": V(9.00, "USD", None, 7, frm="New York, NY"),
        "SparkFun": V(8.00, "USD", 100, 8, frm="Colorado; free UPS Ground from $100"),
        "Seeed": V(8.00, "USD", None, 8, frm="US warehouse option; stock per warehouse not shown"),
        "Orient Display": V(10.00, "USD", 50, 7, frm="Seattle, WA; free ground over $50"),
        "Edmund Optics": V(15.00, "USD", None, 5, frm="Barrington, NJ"),
        "Thorlabs": V(10.00, "USD", None, 3, frm="Newton, NJ; page says available today"),
        "K&J Magnetics": V(8.00, "USD", None, 6, frm="Pennsylvania"),
        "Zoro": V(5.00, "USD", 50, 6, frm="US"),
        "Prusa": V(10.00, "USD", None, 12, frm="ship-from not confirmed; 4-7 working days to prepare"),
        "Extech": V(10.00, "USD", None, 7, frm="US"),
        "Amazon": V(0.00, "USD", None, 3, frm="US"),
        "PiShop.us": V(8.00, "USD", None, 6, frm="US"),
        "Newark": V(9.00, "USD", None, 5, frm="South Carolina"),
        "XREAL": V(0.00, "USD", None, 7, frm="ship-from not confirmed"),
        "OSH Park": V(0.00, "USD", None, 14, conf="page", frm="US fab; 9-14 days, about 7 with Super Swift"),
        "OSH Stencils": V(5.00, "USD", None, 7, frm="US"),
    },
    "JP": {
        "DigiKey Japan": V(2000, "JPY", 6000, 4, ex_tax=True, conf="page",
                           frm="US warehouse, duty paid; free from 6,000 yen"),
        "Akizuki": V(500, "JPY", 11000, 3, frm="Saitama"),
        "Switch Science": V(650, "JPY", 8000, 2, conf="page", frm="Japan; free from 8,000 yen"),
        "Sengoku": V(500, "JPY", None, 3, frm="Tokyo"),
        "Edmund Optics Japan": V(0, "JPY", None, 7, ex_tax=True, conf="page",
                                 frm="1-2 days from Japan stock, 5-7 from overseas stock"),
        "Misumi": V(0, "JPY", None, 3, frm="Japan"),
        "MonotaRO": V(500, "JPY", 3500, 3, frm="Japan; prices include tax; same day to a week by item"),
        "Japan hardware shop": V(500, "JPY", None, 3, frm="MonotaRO, Misumi or Amazon.co.jp; pages not readable"),
        "JLCPCB": V(15.00, "USD", None, 10, imp=(JP_TAX, 0), frm="Shenzhen; 2-3 days to build, 5-8 to ship"),
        "Orient Display (US)": V(30.00, "USD", None, 14, frm="Seattle; delivery to Japan not confirmed"),
        "Tindie (Hong Kong)": V(20.00, "USD", None, 14, imp=(JP_TAX, 2200), frm="Hong Kong"),
        "Hicenda": V(20.00, "USD", None, 14, imp=(JP_TAX, 2200), frm="Shenzhen"),
        "XREAL Japan": V(0, "JPY", None, 5, frm="ship-from not confirmed"),
    },
}


# ------------------------------------------------------------------ sources (edit these)
def S(vendor, part, price, cur="USD", stock="in", lead=None, rating="ok", conf="page", note="",
      pack=1, sub=False):
    """price: one number, or [(min_qty, unit_price), ...]. stock: a count, "in" (in stock, no
    count shown), 0, or None (not known). lead: days to ship when stock is 0. sub: the part
    differs from the design and needs an engineering check."""
    breaks = price if isinstance(price, list) else [(1, price)]
    return dict(vendor=vendor, part=part, breaks=breaks, cur=cur, stock=stock, lead=lead,
                rating=rating, conf=conf, note=note, pack=pack, sub=sub)


DK, DKJ = "DigiKey", "DigiKey Japan"
BLOCKED = dict(vendor="", part="", breaks=[(1, None)], cur="USD", stock=0, lead=None,
               rating="out", conf="page", note="", pack=1, sub=False)


def blocked(note):
    return dict(BLOCKED, note=note)


def dkj(part, usd, stock, note="", sub=False, rating="ok"):
    """A DigiKey part whose yen price was not read: same worldwide stock, US price converted."""
    return S(DKJ, part, [(q, round(p * USD_JPY)) for q, p in usd], "JPY", stock, conf="converted",
             note=note, sub=sub, rating=rating)


SWAP_Q1 = "replaces FDN338P (0 stock, 57-week lead); check Vgs(th) and pinout"
SWAP_Q2 = "replaces FDN335N (0 stock, 70-week lead); check against the schematic"
SWAP_D = "replaces PMEG2010AEH (0 stock until Oct 2027); check pad fit and forward voltage"

# Keys are BOM refs. "option:ref" overrides a ref for one option.
SRC = {
    "US": {
        # bench kits
        "bench-a-beetle:MCU": S(DK, "DFRobot DFR0868", 7.90, stock=None, rating="watch", conf="est",
                                note="DigiKey lists it; stock and price not read. DFRobot direct ships from China"),
        "bench-b-qtpy:MCU": S("Seeed", "XIAO ESP32C3 113991054", 4.99,
                              note="in stock; replaces QT Py ESP32-C3 (out at Adafruit, 18-week lead at DigiKey)"),
        "OLED": S("Adafruit", "938", 19.95),
        "IMU": S("SparkFun", "SEN-22397", [(1, 14.95), (10, 14.20)]),
        "ALS": S("Adafruit", "4162", 4.95),
        "QTH": S("SparkFun", "PRT-17912", 1.95, note="Adafruit 4209 is out of stock"),
        "QT": S("Adafruit", "4399", 0.95),
        "PUCK": S("Adafruit", "4755", 14.95),
        "BAT": S("Adafruit", "1578", [(1, 7.95), (10, 7.16)], note="ground shipping only"),
        "SW": S("Adafruit", "3064", 2.95, stock=0, rating="out",
                note="out of stock, no restock date; optional: unplug the cell instead"),
        "LEAD": S("Adafruit", "3814", 0.75),
        "USB": S("Adafruit", "4474", 4.95),
        "BB": S("Adafruit", "64", 4.95),
        "JMP": S("SparkFun", "PRT-11026", 3.50, note="Adafruit 758 is out of stock"),
        "HDR": S("SparkFun", "PRT-00116", 1.95, note="Adafruit 392 is out of stock"),
        # Pod-0
        "U1": S(DK, "ESP32-C3-MINI-1-N4X", [(1, 3.26), (10, 2.819)], stock=1518,
                note="N4 is not for new designs (1,231 left)"),
        "U2": S(DK, "BMI270", [(1, 4.23), (5, 3.788), (10, 3.624)], stock=32243),
        "U3": S(DK, "VEML7700-TT", [(1, 1.44), (5, 1.272), (10, 1.209)], stock=14393),
        "U4": S(DK, "AP2112K-3.3TRG1", [(1, 0.28), (10, 0.191)], stock=406515),
        "U5": S(DK, "USBLC6-2SC6", [(1, 0.54), (10, 0.329)], stock=27230),
        "DS1": S("Orient Display", "AOM12864A0-0.96WW", 4.53, stock=169, rating="watch",
                 note="single source, no minimum order; DigiKey has 0 with an 8-week lead. "
                      "Page says SSD1315, DigiKey says SSD1306"),
        "J1": S(DK, "FH12-30S-0.5SH(55)", [(1, 2.64), (10, 2.244)], stock=4414),
        "J2": S(DK, "12401610E4#2A", [(1, 1.43), (10, 1.217)], stock=79753),
        "J3,J12": S(DK, "SM10B-SRSS-TB", [(1, 0.93), (10, 0.788)], stock=27096),
        "CBL": S(DK, "A10SR10SR30K152B", [(1, 2.46), (10, 2.09)], stock=106, rating="watch",
                 note="only 106 left; the 152A (5,019) is the reversed pinning"),
        "SW1,SW2": S(DK, "PTS810 SJM 250 SMTR LFS", [(1, 0.58), (10, 0.501)], stock=69043),
        "D1,D12": S(DK, "LTST-C191KGKT", [(1, 0.15), (10, 0.104)], stock=2228010),
        "D13": S(DK, "LTST-C191KRKT", [(1, 0.15), (10, 0.104)], stock=1334425),
        "Q1": S(DK, "DMG2305UX-7", [(1, 0.38), (10, 0.232)], stock=74154, sub=True, note=SWAP_Q1),
        "Q2": S(DK, "FDN337N", [(1, 0.77), (10, 0.479)], stock=222614, sub=True, note=SWAP_Q2),
        "U10": S(DK, "BQ24074RGTR", [(1, 2.43), (10, 1.807)], stock=1901),
        "U11": S(DK, "MAX17048G+T10", [(1, 4.61), (10, 3.499)], stock=21841),
        "BT1": S("Adafruit", "1578", [(1, 7.95), (10, 7.16)], note="no thermistor lead; ground shipping only"),
        "RT1": S(DK, "NCU15XH103F60RC", [(1, 0.10), (5, 0.068), (10, 0.065)], stock=39686,
                 note="NCP15XH103F03RC is not for new designs"),
        "J10": S(DK, "USB4105-GF-A", [(1, 0.80), (10, 0.676)], stock=263569),
        "J11": S(DK, "S2B-PH-SM4-TB", [(1, 0.49), (10, 0.417)], stock=133904),
        "SW10": S(DK, "PCM12SMTR", [(1, 1.28), (10, 1.087)], stock=2726, rating="watch",
                  note="obsolete; enough left for this run"),
        "D10,D11": S(DK, "MBR120VLSFT1G", [(1, 0.61), (10, 0.373)], stock=105142, sub=True, note=SWAP_D),
        "STN": S("OSH Stencils", "pod + puck stencils", 20.00, stock=None, conf="est",
                 note="price not confirmed"),
        # optics and mechanical
        "LENS-A": S("Edmund Optics", "#63-480", [(1, 34.00), (10, 30.75)], stock=20),
        "BS-A": S("Edmund Optics", "#43-736", [(1, 61.00), (10, 45.75)], stock=20,
                  note="30R/70T version: #43-734, same price, 20+ in stock. Thorlabs EBP1 30:70 is $41"),
        "LENS-B": S("Thorlabs", "LA1074-A (12.7 mm, f 20 mm, AR coated)", 39.10, sub=True,
                    note="Edmund #47-873 shows 'Contact us' in the US and Japan: not in stock. "
                         "This lens is 12.7 mm, not 15 mm, so cartridge B's bore changes"),
        "ND": S("Edmund Optics", "#48-090", [(1, 51.50), (11, 45.75)], stock=20),
        "MAG": S("K&J Magnetics", "D41-N52 (1/4 x 1/16 in)", 0.32,
                 note="1/16 in thick, the stocked size nearest the 1/10 in in the design"),
        "SCR": S("Zoro", "M2 x 6 pan, 100", 8.99, stock=None, conf="snippet"),
        "STO": S("Amazon", "M2 brass standoff kit, about 300 pieces", 14.99, pack=300,
                 note="one kit covers every run"),
        "INS": S("Prusa", "M2 short inserts, 100", 11.99, rating="watch",
                 note="ship-from not confirmed; McMaster 94180A307 is about $36"),
        "LUX": S("Extech", "LT40", 155.00, note="budget: Dr.meter LX1330B about $60 (snippet)"),
        # concept 4
        "MCU2": S("PiShop.us", "Pico 2 SC1631", 5.00),
        "LVL": S(DK, "SN74AXC8T245PWR", 1.20, stock=None, rating="watch", conf="est",
                 note="out at TI's own store; distributor stock not read"),
        "BRK": S("Amazon", "TSSOP-24 breakout", 3.00, stock=None, rating="watch", conf="est",
                 note="no true 24-pin board confirmed"),
        "LDO18": S(DK, "TLV75718PDBVR", 0.40, stock=None, rating="watch", conf="est",
                   note="distributor stock not read"),
        "PANEL": blocked("no seller ships an ECX336-class panel from US stock. Imports: Tindie $275 "
                         "(Hong Kong), DisplayModule $239 (origin not confirmed)"),
        "REF": blocked("optional reference module; Hicenda ships from Shenzhen only"),
        "FPGA": S("Newark", "Lattice ICE40UP5K-B-EVN", 65.62, stock=18, conf="snippet", sub=True,
                  note="replaces iCEBreaker (sold out at 1BitSquared)"),
        # concept 5
        "SBC": S("PiShop.us", "Raspberry Pi 5 8 GB", 175.00, note="also CanaKit $175; Adafruit limits 2"),
        "PSU": S("PiShop.us", "SC1157", 12.95),
        "FAN": S("PiShop.us", "SC1148", 12.95, note="$12.95 confirmed on the page; list price is $5"),
        "SD": S("Amazon", "microSD 64 GB", 12.00, stock=None, conf="est"),
        "HDMI": S("Amazon", "micro-HDMI cable", 5.00, stock=None, conf="est"),
        "ADP": S("Amazon", "HDMI to USB-C DisplayPort adapter", 69.00, stock=None, rating="watch",
                 conf="est", note="the team has adapters known to work with the Air 2; no part "
                                  "confirmed for sale"),
        "IMUB": S("Adafruit", "4900", 9.95, stock=94),
        "concept5-linux-puck:IMU": S("Adafruit", "4754 BNO085", [(1, 29.50), (10, 26.55)]),
        "CAM": S("PiShop.us", "Camera Module 3", 29.25),
        "GLS": S("XREAL", "XREAL Air 2", 199.00,
                 note="the team has one already; Monado supports Air, Air 2 and Air 2 Pro"),
    },
    "JP": {
        # bench kits (Switch Science and Akizuki prices include tax)
        "bench-a-beetle:MCU": S("Switch Science", "DFRobot DFR0868", 1268, "JPY", stock=9, rating="watch",
                                note="only 9 in stock; sold as 技適-marked"),
        "bench-b-qtpy:MCU": S("Akizuki", "XIAO ESP32C3 113991054", 1080, "JPY", stock=1460,
                              note="技適 204-B00704 printed on the page"),
        "OLED": S("Switch Science", "Adafruit 938", 4180, "JPY"),
        "IMU": S("Switch Science", "SparkFun SEN-22397", 3817, "JPY"),
        "ALS": S("Switch Science", "Adafruit 4162", 1210, "JPY"),
        "QTH": S("Switch Science", "Qwiic to 4-pin female, 150 mm", 407, "JPY",
                 note="female ends; Adafruit 4209 is out of stock"),
        "QT": S("Switch Science", "Adafruit 4210 (100 mm)", 209, "JPY", note="the 50 mm cable is out"),
        "PUCK": S("Switch Science", "Adafruit bq25185 charger (6091)", 1485, "JPY", sub=True,
                  note="the bq24074 board is not sold in Japan; different TI chip, check the firmware and wiring"),
        "BAT": S("Sengoku", "DTP652533(PHR) 500 mAh", 1580, "JPY", stock=None, rating="watch",
                 note="page lists Akihabara and Osaka as stock locations, no count; the only "
                      "500 mAh JST-PH cell found in Japan"),
        "SW": S("Switch Science", "Adafruit 3064", 726, "JPY"),
        "LEAD": S("Switch Science", "SSCI-022156", 264, "JPY"),
        "USB": S("Japan hardware shop", "USB-A to USB-C data cable", 500, "JPY", stock=None, conf="est"),
        "BB": S("Akizuki", "EIC-801", 450, "JPY", stock=2311),
        "JMP": S("Akizuki", "jumper wire kit 165-011-000", 650, "JPY", stock=940),
        "HDR": S("Akizuki", "1x40 pin header", 35, "JPY", stock=8000),
        # Pod-0 (DigiKey Japan prices exclude tax)
        "U1": S(DKJ, "ESP32-C3-MINI-1-N4X", [(1, 537), (10, 464.7)], "JPY", stock=1518,
                note="技適 201-210888 covers ESP32-C3-MINI-1; not confirmed for the N4X revision"),
        "U2": S(DKJ, "BMI270", [(1, 697), (10, 597.4)], "JPY", stock=32243),
        "U3": S(DKJ, "VEML7700-TT", [(1, 237), (5, 209.6), (10, 199.3)], "JPY", stock=14393),
        "U4": S(DKJ, "AP2112K-3.3TRG1", [(1, 46), (10, 31.5)], "JPY", stock=406515),
        "U5": S(DKJ, "USBLC6-2SC6", [(1, 89), (10, 54.2)], "JPY", stock=27230),
        "DS1": S("Orient Display (US)", "AOM12864A0-0.96WW", 4.53, stock=169, rating="watch", conf="est",
                 note="DigiKey Japan: 0 stock, 8-week lead, 1,907 yen. Delivery from Orient's US store to "
                      "Japan is not confirmed. Japanese and generic panels are 0.7 mm solder-down: wrong for J1"),
        "J1": S(DKJ, "FH12-30S-0.5SH(55)", [(1, 435), (10, 369.9)], "JPY", stock=4414),
        "J2": S(DKJ, "12401610E4#2A", [(1, 236), (10, 200.6)], "JPY", stock=79753),
        "J3,J12": S(DKJ, "SM10B-SRSS-TB", [(1, 153), (10, 129.9)], "JPY", stock=27096),
        "CBL": S(DKJ, "A10SR10SR30K152A", [(1, 405), (10, 344.5)], "JPY", stock=5019, rating="watch",
                 note="reversed pinning: check pin 1; the 152B has 106 left"),
        "SW1,SW2": S(DKJ, "PTS810 SJM 250 SMTR LFS", [(1, 96), (10, 82.6)], "JPY", stock=69043,
                     note="in stock on the main listing; the first check hit a listing with no count"),
        "D1,D12": S(DKJ, "LTST-C191KGKT", [(1, 25), (10, 17.1)], "JPY", stock=2228010),
        "D13": S(DKJ, "LTST-C191KRKT", [(1, 25), (10, 17.1)], "JPY", stock=1334425),
        "Q1": S(DKJ, "DMG2305UX-7", [(1, 63), (10, 38.2)], "JPY", stock=74154, sub=True,
                note=SWAP_Q1 + ". Or genuine FDN338P at LCSC (6,980), or IRLML6402 at Akizuki"),
        "Q2": dkj("FDN337N", [(1, 0.77), (10, 0.479)], 222614, SWAP_Q2 +
                  ". Or SSM3K345R at Akizuki", sub=True),
        "U10": S(DKJ, "BQ24074RGTR", [(1, 401), (10, 297.9)], "JPY", stock=1901),
        "U11": S(DKJ, "MAX17048G+T10", [(1, 760), (10, 576.8)], "JPY", stock=21841),
        "BT1": S("Sengoku", "DTP652533(PHR) 500 mAh", 1580, "JPY", stock=None, rating="watch",
                 note="no stock count shown; 35 x 25.5 x 6.7 mm, check the puck case"),
        "RT1": S(DKJ, "NCP15XH103F03RC", [(1, 20), (10, 15.3)], "JPY", stock=122042,
                 note="also at Akizuki, 20 for 130 yen"),
        "J10": S(DKJ, "USB4105-GF-A", [(1, 132), (10, 111.4)], "JPY", stock=263569),
        "J11": S(DKJ, "S2B-PH-SM4-TB", [(1, 81), (10, 68.7)], "JPY", stock=133904),
        "SW10": S(DKJ, "PCM12SMTR", [(1, 211), (10, 179.2)], "JPY", stock=2726, rating="watch",
                  note="obsolete; enough left for this run"),
        "D10,D11": dkj("MBR120VLSFT1G", [(1, 0.61), (10, 0.373)], 105142, SWAP_D +
                       ". Or RB160M-30TR at Akizuki", sub=True),
        "STN": S("JLCPCB", "stencil, with the board order", 8.00, stock=None, conf="est"),
        # optics and mechanical (Edmund Japan prices exclude tax)
        "LENS-A": S("Edmund Optics Japan", "#63-480", [(1, 4800), (10, 4350)], "JPY", stock=20),
        "BS-A": S("Edmund Optics Japan", "#43-736", [(1, 8550), (10, 6450)], "JPY", stock=20,
                  note="30R/70T version: #43-734, same price"),
        "LENS-B": S("Edmund Optics Japan", "#23-083 (15 x 22.5 mm, MgF2, clearance)", 2800, "JPY",
                    stock=20, rating="watch",
                    note="#47-873 shows 'contact us': not in stock. Same size lens with a simpler "
                         "coating; clearance item, no returns"),
        "ND": S("Edmund Optics Japan", "#48-090", 7250, "JPY", stock=20),
        "MAG": S("Japan hardware shop", "6 x 3 mm neodymium disc", 60, "JPY", stock=None, rating="watch",
                 conf="est", note="only price found: 679 yen a bag, quantity unknown"),
        "SCR": S("MonotaRO", "M2 x 6 pan-head screws, stainless", 582, "JPY",
                 note="from 582 yen a pack; pack size varies by listing"),
        "STO": S("MonotaRO", "Hirosugi ASB-E brass hex spacer, 4 pcs", 593, "JPY", pack=4),
        "INS": S("MonotaRO", "Hirosugi HSB brass heat-set insert, 50 pcs", 1428, "JPY",
                 note="ships same day to 7 days"),
        "LUX": S("Misumi", "Sanwa LX20", 24404, "JPY", stock=None, conf="snippet",
                 note="budget: Shinwa 78747 about 8,600 yen (snippet)"),
        # concept 4
        "MCU2": S("Akizuki", "Pico 2 SC1631", 990, "JPY", stock=1462),
        "LVL": dkj("SN74AXC8T245PWR", [(1, 1.20)], None, "stock not read", rating="watch"),
        "BRK": S("Akizuki", "AE-HTSSOP28-DIP", 80, "JPY", stock=2728, rating="watch",
                 note="28-pin board with a bottom pad; no 24-pin board at Akizuki"),
        "LDO18": dkj("TLV75718PDBVR", [(1, 0.40)], None, "stock not read", rating="watch"),
        "PANEL": S("Tindie (Hong Kong)", "0.23 in 640x400 micro-OLED + HDMI board", 275.00,
                   stock=None, rating="watch", note="no stock count or ship time shown"),
        "REF": S("Hicenda", "HTECX336CN 4,000 nit module", 389.00, stock=None, rating="watch",
                 note="optional reference module"),
        "FPGA": S("Akizuki", "Tang Nano 9K", 3780, "JPY", stock=101, sub=True,
                  note="replaces iCEBreaker (sold out); Gowin FPGA, open toolchain via Apicula"),
        # concept 5
        "SBC": S("Akizuki", "Raspberry Pi 5 8 GB", 35200, "JPY", stock=25,
                 note="also Switch Science 36,300 yen (20)"),
        "PSU": S("Akizuki", "27 W USB PD supply", 2750, "JPY", stock=545),
        "FAN": S("Switch Science", "SC1148", 1100, "JPY"),
        "SD": S("Japan hardware shop", "microSD 64 GB", 1500, "JPY", stock=None, conf="est"),
        "HDMI": S("Akizuki", "HDMI A to micro D, 2 m", 350, "JPY", stock=814),
        "ADP": S("Japan hardware shop", "HDMI to USB-C DisplayPort adapter", 10000, "JPY", stock=None,
                 rating="watch", conf="est", note="no confirmed part or source"),
        "IMUB": S("Switch Science", "Adafruit 4900", 2090, "JPY"),
        "concept5-linux-puck:IMU": S("Switch Science", "Conta BNO085 module", 4950, "JPY",
                                     note="Switch Science's own BNO085 board"),
        "concept5-linux-puck:QT": S("Switch Science", "Qwiic 50 mm", 308, "JPY"),
        "CAM": S("Akizuki", "Camera Module 3", 5940, "JPY", stock=112),
        "GLS": S("XREAL Japan", "XREAL 1S", 67980, "JPY", sub=True,
                 note="Air 2 is not sold standalone in Japan; the 1S needs the community driver, not Monado's"),
    },
}

# Lines bought once per site whatever the run size, reported apart from the unit cost.
TOOLS = {"LUX", "ND"}
# Lines that do not stop a build when they cannot be bought.
OPTIONAL = {"SW", "REF"}
# Optional lines left out of the cost even when they can be bought.
NOT_COSTED = {"REF"}
# What a builder actually orders together.
BUNDLES = [
    ("kit-a", "Kit build A: Beetle bench kit + optics", ["bench-a-beetle", "optics-mech"]),
    ("kit-b", "Kit build B: XIAO bench kit + optics", ["bench-b-qtpy", "optics-mech"]),
    ("pod0", "Pod-0 build: custom PCB set + optics", ["pod0-pcb", "optics-mech"]),
]
# Parts dropped from the design because they cannot be bought.
DROPPED = [
    ("Q1 FDN338P", "0 at DigiKey, 57-week factory lead", "DMG2305UX-7 (74,154 in stock)"),
    ("Q2 FDN335N", "0 at DigiKey, 70-week factory lead", "FDN337N (222,614 in stock)"),
    ("D10, D11 PMEG2010AEH", "0 at DigiKey, next 3,000 due 2027-10-04", "MBR120VLSFT1G (105,142 in stock)"),
    ("U1 ESP32-C3-MINI-1-N4", "not for new designs, 1,231 left", "ESP32-C3-MINI-1-N4X (1,518 in stock)"),
    ("RT1 NCP15XH103F03RC", "not for new designs, 122,042 left", "NCU15XH103F60RC (39,686 in stock)"),
    ("Kit B MCU: Adafruit QT Py ESP32-C3", "out at Adafruit, 18-week lead at DigiKey, no 技適",
     "Seeed XIAO ESP32C3 ($4.99; Akizuki 1,080 yen, 1,460 in stock)"),
    ("Kit switch cable: Adafruit 1131", "wrong part: a plain extension", "Adafruit 3064 (out in the US, in stock in Japan)"),
    ("30R/70T plate: Edmund #34-412", "a $222 precision fused-silica plate", "Edmund #43-734, 1 mm float glass, $61"),
    ("Lux meter: Extech LT45", "about $315, not $110", "Extech LT40 $155, or Dr.meter LX1330B about $60"),
    ("Concept 4 FPGA: iCEBreaker", "sold out at 1BitSquared", "Lattice ICE40UP5K-B-EVN (US), Tang Nano 9K (Japan)"),
    ("Concept 5 board: ROCK 5B / 5B+", "out of stock everywhere checked", "Raspberry Pi 5 8 GB"),
]


# ------------------------------------------------------------------ model
def usd(amount, cur):
    return amount / USD_JPY if cur == "JPY" else amount


def source_for(region, option, line):
    ref = line[0]
    table = SRC[region]
    src = table.get(f"{option}:{ref}") or table.get(ref)
    if src:
        return src
    if option == "pod0-pcb" and ref[0] in "CR" and ref[1].isdigit():   # commodity passives
        vendor = DK if region == "US" else DKJ
        unit = line[7] if region == "US" else max(1, round(line[7] * USD_JPY))
        return S(vendor, line[3], unit, "USD" if region == "US" else "JPY", stock="in", conf="snippet",
                 note="commodity; five values spot-checked, all in stock")
    return None


def pcb_source(region, units):
    if region == "US":
        sets = 1 if units <= 3 else 2
        return S("OSH Park", f"{3 * sets} pods (4-layer) + {3 * sets} pucks (2-layer)", 23.25 * sets,
                 stock="in", note="made in the US; $5 and $10 per sq in per 3 boards; Super Swift doubles it")
    return S("JLCPCB", "5 pods (4-layer) + 5 pucks (2-layer)", 30.00, stock=None, conf="est",
             note="price for these sizes not confirmed; DHL about $15, economy post about $2")


def run_qty(option, line, units):
    ref, qpk = line[0], line[8]
    if ref in ("PCB", "STN") or qpk < 1:
        return 1                      # one order, or one per site
    need = qpk * units
    if option != "pod0-pcb":
        return math.ceil(need)
    if line[7] < 0.25:
        return max(10, math.ceil(need * 1.5))
    if ref.startswith(("U", "Q", "D", "J", "SW", "RT")):
        return math.ceil(need) + 2    # spares for hand assembly
    return math.ceil(need)


def unit_price(src, qty):
    price = None
    for min_qty, p in src["breaks"]:
        if qty >= min_qty:
            price = p
    return price


def line_days(region, src):
    if src["rating"] == "out" and not src["vendor"]:
        return None
    if src["stock"] == 0:
        return src["lead"]
    return VENDORS[region][src["vendor"]]["days"]


def plan(region, options, units, order_date):
    """Cost and timing for `units` of the given options, bought as one order per vendor."""
    rows, by_vendor = [], {}
    for option in options:
        for line in make_bom.OPTIONS[option]["lines"]:
            ref = line[0]
            src = pcb_source(region, units) if ref == "PCB" else source_for(region, option, line)
            if src is None:
                src = dict(BLOCKED, rating="watch", conf="est", vendor="", note="no source researched",
                           breaks=[(1, line[7])])
            qty = run_qty(option, line, units)
            packs = math.ceil(qty / src["pack"])
            price = unit_price(src, packs)
            buyable = (bool(src["vendor"]) and price is not None and src["stock"] != 0
                       and ref not in NOT_COSTED)
            ext = packs * price if buyable else 0.0
            vendor = VENDORS[region].get(src["vendor"])
            if buyable:
                by_vendor.setdefault(src["vendor"], 0.0)
                by_vendor[src["vendor"]] += ext
            tax = ext * JP_TAX if buyable and vendor["ex_tax"] else 0.0
            rows.append(dict(option=option, ref=ref, desc=line[1], src=src, qty=packs, price=price,
                             ext_usd=usd(ext + tax, src["cur"]) if buyable else 0.0, buyable=buyable,
                             tool=ref in TOOLS, optional=ref in OPTIONAL,
                             days=line_days(region, src) if buyable or src["stock"] == 0 else None))
    shipping = 0.0
    for name, subtotal in by_vendor.items():
        v = VENDORS[region][name]
        fee = 0.0 if (v["free"] is not None and subtotal >= v["free"]) else v["fee"]
        rate, carrier = v["imp"]
        if rate and usd(subtotal, v["cur"]) * USD_JPY > 10000:   # Japan's small-parcel exemption
            fee += subtotal * rate + (carrier / USD_JPY if v["cur"] == "USD" else carrier)
        shipping += usd(fee, v["cur"])
    needed = [r for r in rows if not r["optional"]]
    parts = sum(r["ext_usd"] for r in rows if not r["tool"])
    tools = sum(r["ext_usd"] for r in rows if r["tool"])
    blockers = [r for r in needed if not r["buyable"]]
    subs = [r for r in needed if r["buyable"] and r["src"]["sub"]]
    watch = [r for r in needed if r["buyable"] and r["src"]["rating"] == "watch"]
    unconfirmed = [r for r in needed if r["buyable"] and r["src"]["conf"] == "est"]
    timed = [r for r in needed if r["buyable"] and r["days"]]
    gate = max(timed, key=lambda r: r["days"]) if timed else None
    verdict = "blocked" if blockers else "substitutions" if subs else "buildable"
    return dict(rows=rows, parts=parts, tools=tools, shipping=shipping, total=parts + shipping,
                per_unit=(parts + shipping) / units, vendors=len(by_vendor), verdict=verdict,
                blockers=blockers, subs=subs, watch=watch, unconfirmed=unconfirmed, gate=gate,
                days=gate["days"] if gate else None,
                in_hand=(order_date + dt.timedelta(days=gate["days"])).isoformat() if gate else None)


# ------------------------------------------------------------------ output
VERDICT = {"buildable": "Buildable now", "substitutions": "Buildable with substitutions", "blocked": "Blocked"}


def money(x):
    return f"${x:,.0f}"


def stock_text(src):
    s = src["stock"]
    if s == 0:
        return f"0, {src['lead']} d lead" if src["lead"] else "0"
    return "in stock" if s == "in" else "not shown" if s is None else f"{s:,}"


def price_text(src, price):
    if price is None:
        return "-"
    return f"¥{price:,.0f}" if src["cur"] == "JPY" else f"${price:,.2f}"


def cap(text):
    return text[:1].upper() + text[1:]


def ref_name(r):
    return f"{r['ref']} {r['src']['part'] or r['desc']}".strip()


def write_csv(key, region, options, order_date):
    plans = {n: plan(region, options, n, order_date) for n in RUNS}
    path = os.path.join(OUT, f"{key}.{region.lower()}.csv")
    with open(path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["option", "ref", "description", "vendor", "part_bought", "currency", "unit_price_1",
                    "stock", "days_to_door", "rating", "confidence", "substitute", "one_time_tool",
                    *[f"qty_{n}" for n in RUNS], *[f"usd_{n}" for n in RUNS], "note"])
        for i, r in enumerate(plans[1]["rows"]):
            s = r["src"]
            w.writerow([r["option"], r["ref"], r["desc"], s["vendor"], s["part"], s["cur"],
                        s["breaks"][0][1] if s["breaks"][0][1] is not None else "", stock_text(s),
                        r["days"] or "", s["rating"], s["conf"], "yes" if s["sub"] else "",
                        "yes" if r["tool"] else "",
                        *[plans[n]["rows"][i]["qty"] for n in RUNS],
                        *[f"{plans[n]['rows'][i]['ext_usd']:.2f}" for n in RUNS], s["note"]])
    return plans


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--date", default=CHECKED, help="order date, YYYY-MM-DD")
    order_date = dt.date.fromisoformat(ap.parse_args().date)
    os.makedirs(OUT, exist_ok=True)

    entries = [(k, o["title"], [k]) for k, o in make_bom.OPTIONS.items()] + BUNDLES
    results = {}
    for key, title, options in entries:
        for region in REGIONS:
            results[key, region] = write_csv(key, region, options, order_date)

    md = []
    w = md.append
    w("# Hiroglass REV1: what is buildable\n")
    w(f"Generated by `sourcing.py`. Sources checked **{CHECKED}**; in-hand dates assume an order placed "
      f"**{order_date.isoformat()}**. Edit the script, not this file.\n")
    w("Two buyers are compared. **US** buys only from US sellers shipping from US stock. **Japan** buys only "
      "from sellers that deliver to Japan: Japanese shops first, DigiKey Japan (ships from the US, duty paid), "
      "and sellers in China or Hong Kong where nothing else exists.\n")
    w("Cost is parts plus one shipping fee per seller, in US dollars "
      f"(¥{USD_JPY:.2f} to the dollar), with Japan's 10% consumption tax where a shop lists prices before tax. "
      "US sales tax is not included. *In hand* is the slowest line's delivery; PCB assembly, 3D printing and "
      "bring-up time come after it.\n")

    w("## Verdict\n")
    w("| Build | Buyer | Verdict | 1 unit | 3 units | 5 units | Per unit at 5 | Parts in hand | Slowest line |")
    w("|---|---|---|---|---|---|---|---|---|")
    for key, title, options in BUNDLES + [e for e in entries if e[0].startswith("concept")]:
        for region in REGIONS:
            p = results[key, region]
            g = p[5]["gate"]
            stuck = p[5]["verdict"] == "blocked"
            hand = f"{p[5]['in_hand']} ({p[5]['days']} d)" if g and not stuck else "-"
            slow = f"{g['ref']} from {g['src']['vendor']}" if g and not stuck else (
                p[5]["blockers"][0]["ref"] + ": no source" if stuck else "-")
            cost = [money(p[n]["total"]) if p[n]["verdict"] != "blocked" else f"({money(p[n]['total'])})"
                    for n in (1, 3, 5)]
            w(f"| {title} | {region} | **{VERDICT[p[5]['verdict']]}** | {cost[0]} | {cost[1]} | {cost[2]} | "
              f"{'-' if stuck else money(p[5]['per_unit'])} | {hand} | {slow} |")
    w("")
    w("Costs in brackets leave out the lines that cannot be bought. Bench tools (lux meter, ND filter) are "
      "bought once per site and are not in these figures: about "
      f"{money(results['optics-mech', 'US'][1]['tools'])} in the US and "
      f"{money(results['optics-mech', 'JP'][1]['tools'])} in Japan.\n")

    w("## Cost of a run of 1 to 5 units\n")
    w("Parts plus shipping for the whole run, then the cost per unit. Small runs are dominated by shipping "
      "fees, minimum quantities (boards come in threes or fives, passives in tens) and spares.\n")
    for region, label in REGIONS.items():
        w(f"### {label}\n")
        w("| Option | " + " | ".join(f"{n}" for n in RUNS) + " | Per unit at 1 | Per unit at 5 | Sellers |")
        w("|---|" + "---|" * (len(RUNS) + 3))
        for key, title, options in entries:
            p = results[key, region]
            w(f"| {title} | " + " | ".join(money(p[n]["total"]) for n in RUNS) +
              f" | {money(p[1]['per_unit'])} | {money(p[5]['per_unit'])} | {p[5]['vendors']} |")
        w("")

    w("## Lead times and lines to watch\n")
    for key, title, options in entries[:len(make_bom.OPTIONS)]:
        w(f"### {title}\n")
        for region in REGIONS:
            p = results[key, region][5]
            hand = (f"parts in hand {p['in_hand']} ({p['days']} days)"
                    if p["gate"] and p["verdict"] != "blocked" else "no in-hand date")
            w(f"**{region}: {VERDICT[p['verdict']]}**, {hand}, {p['vendors']} sellers, "
              f"{len(p['unconfirmed'])} of {len(p['rows'])} lines priced by estimate.\n")
            for label, group in (("Cannot be bought", p["blockers"]), ("Substitute part, needs an engineering check", p["subs"]),
                                 ("Watch", [r for r in p["watch"] if not r["src"]["sub"]])):
                for r in group:
                    s = r["src"]
                    where = f"{s['vendor']}, {stock_text(s)}" if s["vendor"] else "no source"
                    w(f"- **{label}:** {ref_name(r)} ({where}). {cap(s['note'])}".rstrip(". ") + ".")
            left_out = [r for r in p["rows"] if r["optional"] and not r["buyable"]]
            for r in left_out:
                w(f"- **Left out (optional):** {ref_name(r)}. {cap(r['src']['note'])}.")
            w("")

    w("## Parts changed or dropped on 2026-11-10\n")
    w("| Was | Problem | Now |\n|---|---|---|")
    for was, problem, now in DROPPED:
        w(f"| {was} | {problem} | {now} |")
    w("")
    w("The MOSFET and diode substitutes were picked for stock and package only. Each must be checked "
      "against its datasheet and the Pod-0 schematic before layout.\n")

    w("## Rules that affect the build\n")
    w("- **US tariffs on boards from China:** JLCPCB collects 35% or more up front on US orders and requires "
      "duty-paid shipping, so OSH Park (US-made, free shipping) is the US board source.")
    w("- **US de minimis:** the $800 duty exemption is suspended for all countries, so AliExpress optics and "
      "panels pay duty and possibly an entry fee.")
    w("- **Japan small parcels:** a customs value of ¥10,000 or less is exempt from duty and consumption tax. "
      "Above it, add 10% tax and a carrier fee (DHL: ¥2,200). The rule changes on 2028-04-01, not before.")
    w("- **Japan radio law (技適):** ESP32-C3-MINI-1 has certificate 201-210888 and the XIAO ESP32C3 has "
      "204-B00704 (seller's page). The QT Py ESP32-C3 and generic SuperMini boards have none: do not use them "
      "in Japan. A 180-day experimental notification exists for uncertified Wi-Fi and Bluetooth devices.")
    w("- **Japan battery law (PSE):** applies to cells of 400 Wh/L or more. A 500 mAh pouch cell works out "
      "at about 310 to 370 Wh/L from its outer size, under the line. This is a calculation, not a ruling.")
    w("- **LiPo shipping:** SparkFun does not ship cells abroad; cells travel by ground in the US. Buy cells "
      "in the country where they are used.\n")

    w("## What could not be confirmed\n")
    w("- **DigiKey US and Mouser.** Both refused automated reading, and a normal Chrome window driven "
      "by Playwright got a Cloudflare check on DigiKey and a refusal on Mouser. DigiKey's US prices and "
      "stock are the ones read on its Mexico storefront (digikey.com.mx, priced in US dollars). The three "
      "DigiKey pages that did load in Chrome matched those figures, which supports one shared inventory, "
      "but any US tariff surcharge at checkout is not included. Nothing was read at Mouser.")
    w("- **Read in Chrome on the second pass:** Edmund US and Japan, Thorlabs, Orient Display, Seeed, "
      "PiShop, K&J, MonotaRO, Sengoku, Amazon and AliExpress search pages.")
    w("- **Still unread:** Newark, Arrow, McMaster, Zoro, Taobao, Misumi and Marutsu.")
    w("- **Shipping fees:** only DigiKey US ($4.99), DigiKey Japan (¥2,000 under ¥6,000), Switch Science "
      "and OSH Park were read. Every other fee in the seller table is an estimate; fees appear only at "
      "checkout, and no cart was touched.")
    w("- **Japan:** magnets, a USB cable source, the battery's stock count, JLCPCB's price for these two "
      "boards, and delivery of the OLED panel from Orient's US store.")
    w("- **Pod-0 OLED:** Orient's page says SSD1315 and DigiKey says SSD1306 for the same part. Both are "
      "command-compatible in normal use, but read the datasheet before ordering.")
    w("- **Concept 5:** no HDMI to USB-C DisplayPort adapter for the glasses was confirmed in either country.\n")

    w("## Seller assumptions\n")
    for region, label in REGIONS.items():
        w(f"### {label}\n")
        w("| Seller | Shipping fee | Free from | Days to door | Ships from | Fee and days |")
        w("|---|---|---|---|---|---|")
        for name, v in VENDORS[region].items():
            sym = "¥" if v["cur"] == "JPY" else "$"
            fee = f"{sym}{v['fee']:,.0f}" if v["cur"] == "JPY" else f"{sym}{v['fee']:,.2f}"
            free = f"{sym}{v['free']:,}" if v["free"] else "-"
            w(f"| {name} | {fee} | {free} | {v['days']} | {v['frm']} | "
              f"{'read on the page' if v['conf'] == 'page' else 'estimate'} |")
        w("")
    w("## Files\n")
    w("- `sourcing/<option>.us.csv` and `sourcing/<option>.jp.csv`: every line with its seller, part bought, "
      "stock, days to door, rating, and quantity and cost for runs of 1 to 5.")
    w("- `sourcing/sourcing.json`: the same results for other tools.")
    w("- Research notes: [`research_notes/BOM sourcing US and Japan/`]"
      "(../research_notes/BOM%20sourcing%20US%20and%20Japan/).")
    with open(os.path.join(HERE, "BUILDABILITY.md"), "w") as f:
        f.write("\n".join(md) + "\n")

    out = dict(checked=CHECKED, order_date=order_date.isoformat(), usd_jpy=USD_JPY, entries=[])
    for key, title, options in entries:
        e = dict(key=key, title=title, options=options, regions={})
        for region in REGIONS:
            p = results[key, region]
            e["regions"][region] = dict(
                verdict=p[5]["verdict"], in_hand=p[5]["in_hand"], days=p[5]["days"],
                gate=ref_name(p[5]["gate"]) if p[5]["gate"] else None, vendors=p[5]["vendors"],
                tools=round(p[1]["tools"], 2),
                runs={n: dict(total=round(p[n]["total"], 2), parts=round(p[n]["parts"], 2),
                              shipping=round(p[n]["shipping"], 2), per_unit=round(p[n]["per_unit"], 2))
                      for n in RUNS},
                lines=[dict(option=r["option"], ref=r["ref"], desc=r["desc"], vendor=r["src"]["vendor"],
                            part=r["src"]["part"], price=price_text(r["src"], r["src"]["breaks"][0][1]),
                            stock=stock_text(r["src"]), days=r["days"], rating=r["src"]["rating"],
                            conf=r["src"]["conf"], sub=r["src"]["sub"], tool=r["tool"],
                            optional=r["optional"], buyable=r["buyable"], note=r["src"]["note"],
                            qty={n: p[n]["rows"][i]["qty"] for n in RUNS},
                            usd={n: round(p[n]["rows"][i]["ext_usd"], 2) for n in RUNS})
                       for i, r in enumerate(p[5]["rows"])])
        out["entries"].append(e)
    with open(os.path.join(OUT, "sourcing.json"), "w") as f:
        json.dump(out, f, indent=1, ensure_ascii=False)

    for key, title, options in entries:
        for region in REGIONS:
            p = results[key, region]
            print(f"{key:20s} {region}  {p[5]['verdict']:13s} " +
                  " ".join(f"{p[n]['total']:8.0f}" for n in RUNS) + f"   in hand {p[5]['in_hand']}")


if __name__ == "__main__":
    main()
