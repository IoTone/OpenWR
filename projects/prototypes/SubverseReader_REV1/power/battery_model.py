#!/usr/bin/env python3
"""SubverseReader REV1 battery-life model.

    python3 battery_model.py            # writes README.md and life.json next to this file

Every number here is an estimate to be replaced by bench measurements (a Nordic
PPK2 or Joulescope on the battery lead). The model is deliberately simple:

    daily mAh = pages x E_page + sleep_uA x 24 h + extras (light, sync, NFC)
    days      = usable mAh / daily mAh,  usable = 85 % of rated (3.3 V cut-off, ageing)

Self-discharge of a LiPo (~2 %/month) is added to standby figures.

Auto-off ladder (same on every variant, see docs/DESIGN-SubverseReader.md, Power):
  1. after a page: light sleep for LINGER_S so the next press is instant, then deep sleep
     with the page left on the glass (e-paper holds it at zero power)
  2. radios, front light and NFC field each have their own short timeout
  3. after OFF_AFTER_DAYS idle, or below 5 % battery: draw the badge (or a "charge me"
     card), then cut power with the soft latch. Draw is then only the cell protection IC.
  Any key wakes from every level; from level 3 it is a cold boot that reopens the saved page.
"""
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
USABLE = 0.85
SELF_DISCHARGE = 0.02 / 30  # fraction per day
LINGER_S = 10               # light sleep after each key before deep sleep
MARGIN = 1.5                # real-world factor on every active energy figure
OFF_AFTER_DAYS = 7
BAD_SLEEP_UA = 250          # sensitivity: an unswitched microSD or a module board's LED/LDO
OFF_UA = 3.5                # latch off: DW01-class protection IC + charger leakage

# Per page turn, in mA*s: wake/boot, fetch+decompress, push frame, refresh, linger.
# Full refresh every 20 pages for 1-bit panels (ghost clean-up), amortised in.
PANEL = {
    # key: (label, refresh mA*s per page incl. amortised full refresh, needs MCU awake?)
    "spi29":  ("2.9in SPI 1-bit", 2.4 + 0.8, False),     # partial ~0.3 s at ~8 mA + full/20
    "spi42":  ("4.2in SPI 1-bit", 3.5 + 1.2, False),     # partial ~0.4 s, full ~3 s at ~8 mA
    "spi397": ("3.97in SPI 800x480", 4.5 + 1.5, False),
    "spi583": ("5.83in SPI 1-bit", 5.0 + 1.8, False),
    "par47":  ("4.7in parallel 16-grey", 45.0, True),   # S3 clocks every waveform frame
    "par60":  ("6in parallel 16-grey", 70.0, True),
    "c4_583": ("5.83in 4-colour", 110.0, False),         # ~12-15 s refresh at ~8 mA
}
MCU = {
    # key: (label, wake+work mA*s, light-sleep mA, deep-sleep chip uA)
    "c5":   ("ESP32-C5", 8.0, 0.35, 12),  # datasheet v1.5: deep sleep 12 uA typ
    "s3":   ("ESP32-S3", 9.0, 0.40, 9),
    "nrf":  ("nRF52840", 0.6, 0.004, 2),  # System ON idle ~3 uA; wake in microseconds
}
# Board overhead in deep sleep, uA, on top of the chip. Custom boards: RT9080 2 uA,
# DW01 3 uA, charger ~1 uA, panel/SD rail switched off.
BOARD_CUSTOM = 6
# extras: per-day mAh for a "festival day" profile
SYNC_WIFI_MAH = 0.5      # one Wi-Fi bulk sync (~20 s at ~90 mA)
SYNC_BLE_MAH = 0.15      # one BLE zine sync (~300 KB)
NFC_TAP_MAH = 0.03       # MFRC522 field ~50 mA for ~2 s average (off on first read)
WIFI_FETCH_MAH = 0.28    # SR-14W fetch of one zine pack over Wi-Fi
LIGHT_MA = 25            # SR-6 front light, mid level

CELLS = {  # code: (mAh, thickness mm, footprint, unit @5, unit @100)
    "302540": (250, 3.0, "25x40", 2.80, 1.60),
    "402540": (350, 4.0, "25x40", 3.00, 1.80),
    "503040": (550, 5.0, "30x40", 3.40, 2.00),
    "303450": (500, 3.0, "34x50", 3.40, 2.00),
    "503450": (1000, 5.0, "34x50", 4.20, 2.60),
    "603450": (1200, 6.0, "34x50", 4.80, 2.80),
    "604050": (1500, 6.0, "40x50", 5.40, 3.20),
    "103450": (2000, 10.0, "34x50", 6.50, 3.80),
    "104050": (2500, 10.0, "40x50", 7.20, 4.20),
}

# variant: mcu, panel, extra sleep uA (module boards, SD idle etc), tiers (S, M=stock, L),
# stock depth mm, extras key, note
V = [
    ("SR-1", "Laminate", "s3", "spi42", 12, ("303450", "503450", "604050"), 12.5, "", "XIAO S3 Plus on EE05; EE05 switches panel + divider off: measure"),
    ("SR-1D", "Laminate Direct", "c5", "spi42", 12, ("303450", "503450", "604050"), 12.5, "", "XIAO C5 + Seeed driver board: measure"),
    ("SR-1C", "Laminate C5", "c5", "spi42", 9, ("303450", "503450", "604050"), 13, "", "FireBeetle 2 C5: 21 uA board total per DFRobot"),
    ("SR-2", "All Access", "s3", "spi583", BOARD_CUSTOM, ("503450", "604050", "104050"), 19.5, "", ""),
    ("SR-3", "Gray Pass", "s3", "par47", 40, ("303450", "603450", "103450"), 14.5, "", "M5 kit sleep path unknown"),
    ("SR-4", "Headliner", "s3", "par60", BOARD_CUSTOM + 2, ("603450", "103450", "104050"), 17.5, "", "TPS65185 + PCA9555 in sleep"),
    ("SR-5", "Crew Clip", "c5", "spi42", BOARD_CUSTOM, ("503450", "603450", "103450"), 21.5, "", ""),
    ("SR-6", "Night Shift", "c5", "spi42", BOARD_CUSTOM, ("503450", "604050", "104050"), 14.5, "light", "1 h of light a day"),
    ("SR-7", "Poster", "s3", "c4_583", BOARD_CUSTOM, ("503450", "604050", "104050"), 19.5, "", "slow colour refresh"),
    ("SR-8", "Quiet Pass", "nrf", "spi42", 5, ("303450", "503450", "604050"), 13, "ble", "SD rail must be switched"),
    ("SR-9", "Roadie", "c5", "spi42", BOARD_CUSTOM, ("503450", "603450", "103450"), 14.5, "", "stick powered only while awake"),
    ("SR-10", "Credential", "c5", "spi29", BOARD_CUSTOM, ("302540", "402540", "503040"), 8.5, "", ""),
    ("SR-11", "Full Bleed", "c5", "spi397", BOARD_CUSTOM, ("303450", "503450", "603450"), 11, "", ""),
    ("SR-12", "Tour Laminate", "c5", "spi397", BOARD_CUSTOM, ("303450", "603450", "604050"), 12, "", ""),
    ("SR-13", "Hybrid Credential", "s3", "par47", 40, ("503450", "604050", "104050"), 14.5, "", "M5 kit sleep path unknown"),
    ("SR-14W", "Tap Pass Wi-Fi", "c5", "spi397", BOARD_CUSTOM, ("303450", "603450", "103450"), 12.5, "tapw", "10 taps + 2 Wi-Fi fetches a day"),
    ("SR-14B", "Tap Pass tethered", "nrf", "spi397", BOARD_CUSTOM, ("303450", "603450", "103450"), 12.5, "tapb", "10 taps + 2 phone fetches a day"),
]


def e_page(mcu, panel):
    """mAh per page turn."""
    _, wake, ls_ma, _ = MCU[mcu]
    _, refresh, awake = PANEL[panel]
    # parallel refresh figures already include the MCU's active current
    mas = wake + refresh + LINGER_S * ls_ma
    return MARGIN * mas / 3600.0


def sleep_ua(mcu, extra):
    return MCU[mcu][3] + extra


def extras(kind, mcu):
    return MARGIN * _extras(kind, mcu)


def _extras(kind, mcu):
    sync = SYNC_BLE_MAH if mcu == "nrf" else SYNC_WIFI_MAH
    if kind == "light":
        return sync + LIGHT_MA * 1.0
    if kind == "tapw":
        return sync + 10 * NFC_TAP_MAH + 2 * WIFI_FETCH_MAH
    if kind == "tapb":
        return sync + 10 * NFC_TAP_MAH + 2 * SYNC_BLE_MAH
    return sync


def life(row, cell):
    key, name, mcu, panel, extra, tiers, depth, xk, note = row
    mah = CELLS[cell][0] * USABLE
    ep = e_page(mcu, panel)
    sl = sleep_ua(mcu, extra) * 24 / 1000  # mAh/day
    sd = CELLS[cell][0] * SELF_DISCHARGE
    reader = mah / (300 * ep + sl + sd)               # 300 pages/day, nothing else
    festival = mah / (150 * ep + sl + sd + extras(xk, mcu))
    standby = mah / (sl + sd)                          # asleep, never touched
    bad = mah / (300 * ep + BAD_SLEEP_UA * 24 / 1000 + sd)  # reader, sloppy sleep
    off_days = mah / (OFF_UA * 24 / 1000 + sd)        # after the latch cuts power
    return dict(pages=round(mah / ep), reader=reader, festival=festival,
                standby=standby, off=off_days, bad=bad, e_page=ep, sleep_ua=sleep_ua(mcu, extra))


# What auto-off protects against: the pass left in each awake state with no timeout.
LEFT_ON = [  # (state, mA on C5/S3, mA on nRF52840)
    ("Menu left open, CPU in light sleep", 0.35, 0.004),
    ("Menu left open, CPU awake (no sleep at all)", 28, 4),
    ("Sync left running (Wi-Fi / BLE connected)", 90, 6),
    ("SR-6 front light left on (mid)", 25 + 0.35, None),
    ("SR-14 NFC field left on", 50, 50),
]


def fmt_days(d):
    if d >= 365:
        return "1 yr+"  # self-discharge and cell ageing make finer figures meaningless
    if d >= 60:
        return f"{d / 30.4:.0f} mo"
    return f"{d:.0f} d"


def fmt_h(h):
    if h >= 24 * 365:
        return "1 yr+"
    if h >= 24 * 60:
        return f"{h / 24 / 30.4:.0f} mo"
    return f"{h:.0f} h" if h < 72 else f"{h / 24:.0f} d"


def main():
    out, js = [], {}
    out += ["# SubverseReader REV1 battery life", "",
            "Generated by `battery_model.py`; edit that, not this file. **Estimates**, to be "
            "replaced by bench numbers (PPK2 or Joulescope on the battery lead). Usable capacity "
            f"is {USABLE:.0%} of rated; LiPo self-discharge {SELF_DISCHARGE * 3000:.0f} %/month "
            "is included in standby.", "",
            "**Profiles.** *Reader*: 300 page turns a day (2–3 h of reading), nothing else. "
            "*Festival*: 150 page turns, one sync a day, plus the variant's extra (SR-6 one hour "
            "of front light; SR-14 ten taps and two fetches). *Asleep*: never touched, deep "
            f"sleep with the page on screen. *Off*: after {OFF_AFTER_DAYS} idle days the latch "
            "cuts power; only the cell's protection IC draws.", "",
            "## Stock cells", "",
            "| Variant | Cell | mAh | Page turns per charge | Reader | Festival | Asleep | Off | Reader at 250 µA sleep | Sleep µA | mAh/page |",
            "|---|---|---|---|---|---|---|---|---|---|---|"]
    for row in V:
        cell = row[5][1]
        r = life(row, cell)
        out.append(f"| {row[0]} {row[1]} | {cell} | {CELLS[cell][0]} | {r['pages']:,} | "
                   f"{fmt_days(r['reader'])} | {fmt_days(r['festival'])} | {fmt_days(r['standby'])} | "
                   f"{fmt_days(r['off'])} | {fmt_days(r['bad'])} | {r['sleep_ua']:.0f} | {r['e_page']:.4f} |")
    out += ["", "## Battery options per variant", "",
            "S is the thin option, M the stock cell in the BOM, L the extended cell. Depth "
            "change is the cell's thickness difference against stock; the shell grows or "
            "shrinks by the same amount. Cell prices are estimates (protected cells with "
            "JST-PH leads, AliExpress / factory direct).", "",
            "| Variant | Option | Cell | mAh | Depth (mm) | Cell @5 | Cell @100 | Reader | Festival | Page turns |",
            "|---|---|---|---|---|---|---|---|---|---|"]
    for row in V:
        key, name, mcu, panel, extra, tiers, depth, xk, note = row
        stock_t = CELLS[tiers[1]][1]
        js[key] = dict(name=name, sleep_ua=sleep_ua(mcu, extra), e_page=e_page(mcu, panel),
                       note=note, tiers=[])
        for tag, cell in zip("SML", tiers):
            mah, t, fp, u5, u100 = CELLS[cell]
            r = life(row, cell)
            d = depth + (t - stock_t)
            js[key]["tiers"].append(dict(opt=tag, cell=cell, mah=mah, depth=round(d, 1),
                                         u5=u5, u100=u100, pages=r["pages"],
                                         reader=round(r["reader"]), festival=round(r["festival"]),
                                         standby=round(r["standby"]), off=round(r["off"]),
                                         bad=round(r["bad"])))
            out.append(f"| {key if tag == 'S' else ''} | {tag}{' (stock)' if tag == 'M' else ''} | "
                       f"{cell} ({fp}) | {mah} | {d:g} | ${u5:.2f} | ${u100:.2f} | "
                       f"{fmt_days(r['reader'])} | {fmt_days(r['festival'])} | {r['pages']:,} |")
    out += ["", "## What auto-off protects against", "",
            "Time to flat if the pass were left in one awake state with no timeout (1200 mAh "
            "cell, 85 % usable). Every one of these states has its own timeout in firmware.", "",
            "| Left on | ESP32 passes | nRF52840 passes |", "|---|---|---|"]
    js["_left_on"] = []
    for state, ma, man in LEFT_ON:
        h1 = 1200 * USABLE / ma
        h2 = (1200 * USABLE / man) if man else None
        js["_left_on"].append([state, fmt_h(h1), fmt_h(h2) if h2 else "—"])
        out.append(f"| {state} | {fmt_h(h1)} | {fmt_h(h2) if h2 else '—'} |")
    out += ["", "## Assumptions", "",
            f"- Every active figure (page turns, sync, light, NFC) is multiplied by {MARGIN} "
            "for real-world firmware; the per-component figures are datasheet-typical.",
            f"- Page turn = wake + read and decompress + push frame + refresh + {LINGER_S} s light "
            "sleep before deep sleep. 1-bit panels do a full refresh every 20 pages.",
            "- Parallel 16-grey panels (SR-3, SR-4, SR-13) keep the S3 awake for the whole "
            "waveform (~1–1.5 s at ~90 mA). A fast 2-level text mode would cut this roughly in half.",
            "- Deep sleep: ESP32-C5 ~12 µA (datasheet typ), S3 ~9 µA, nRF52840 ~2 µA (chip, RTC on), plus board "
            f"overhead: {BOARD_CUSTOM} µA on our boards (2 µA LDO, protection IC, charger); "
            "module boards are guesses until measured.",
            "- Panel and microSD sit on a switched rail. An unswitched microSD idles at "
            "100–250 µA and would be the largest sleep draw on the pass.",
            "- Not modelled: cold-temperature capacity loss (~20 % at 0 °C), cell ageing past 300 cycles.",
            ""]
    with open(os.path.join(HERE, "README.md"), "w") as f:
        f.write("\n".join(out))
    with open(os.path.join(HERE, "life.json"), "w") as f:
        json.dump(js, f, indent=1)
    for row in V:
        r = life(row, row[5][1])
        print(f"{row[0]:7} pages {r['pages']:>7,}  reader {fmt_days(r['reader']):>7}  festival "
              f"{fmt_days(r['festival']):>7}  asleep {fmt_days(r['standby']):>7}  off {fmt_days(r['off'])}")


main()
