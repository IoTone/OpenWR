# Hiroglass REV1 schematics

Schematics and wiring diagrams for every build option in [`../bom/`](../bom/README.md). The bench kits are wiring diagrams: off-the-shelf boards with each wire drawn. The Pod-0 boards are pin-level schematics that use net labels, KiCad-style: two pins with the same label are connected. The experimental branches are system wiring at board level.

Concepts 1, 2 and 3 share the same electronics and differ only in optics and mounting, so one Pod-0 set covers all three.

| Sheet | File | What it is |
|---|---|---|
| B-A | [bench-a-beetle.svg](sheets/bench-a-beetle.svg) | Bench kit A: Beetle ESP32-C3 with goggl1's SPI display pins, I2C sensors, puck harness |
| B-B | [bench-b-qtpy.svg](sheets/bench-b-qtpy.svg) | Bench kit B: QT Py ESP32-C3, everything daisy-chained on STEMMA QT |
| P-1 | [pod0-1-mcu-usb-power.svg](sheets/pod0-1-mcu-usb-power.svg) | Pod-0 pod: ESP32-C3-MINI-1, 24-pin USB-C with ESD, 3.3 V LDO, reset/boot/LED, tether |
| P-2 | [pod0-2-display-sensors.svg](sheets/pod0-2-display-sensors.svg) | Pod-0 pod: 0.96″ OLED on 30-pin FPC with its charge-pump parts and supply switch, BMI270, VEML7700 |
| P-3 | [pod0-3-puck.svg](sheets/pod0-3-puck.svg) | Pod-0 puck: BQ24074 charger at 500 mA with cold-charge cutoff, LiPo, power switch, MAX17048 gauge |
| X-4 | [concept4-microoled-driver.svg](sheets/concept4-microoled-driver.svg) | Concept 4: Pico 2 PIO (or iCEBreaker) driving a micro-OLED's parallel bus through level shifters |
| X-5 | [concept5-linux-puck.svg](sheets/concept5-linux-puck.svg) | Concept 5: Pi 5 puck, HDMI-to-glasses adapter, USB IMU bridge, stereo cameras, phone over BLE |

## Pod-0 GPIO map (ESP32-C3-MINI-1)

| GPIO | Module pin | Function | Note |
|---|---|---|---|
| 0 | 12 | BMI270 INT1 | |
| 1 | 13 | OLED D/C# | same as goggl1 |
| 2 | 5 | I2C SCL | strapping pin; the 4.7k pull-up also serves Espressif's recommended pull-up |
| 3 | 6 | OLED RES# | moved from goggl1's GPIO2 |
| 4 | 18 | OLED SCLK | same as goggl1 |
| 5 | 19 | I2C SDA | |
| 6 | 20 | OLED SDIN | same as goggl1 |
| 7 | 21 | OLED CS# | same as goggl1 |
| 8 | 22 | status LED, active low | strapping pin; must be high at boot (pull-up R3) |
| 9 | 23 | BOOT button | strapping pin |
| 10 | 16 | OLED VBAT enable | switches the panel's charge-pump supply |
| 18, 19 | 26, 27 | USB D−, D+ | native USB console and flashing |
| 20 | 30 | fuel-gauge alert | |
| 21 | 31 | UART TX test pad | |

## What to verify before layout

- USBLC6-2SC6 and MAX17048 pin numbers were not read from their datasheets (downloads failed); check both.
- The Winstar OLED alternate needs its own datasheet check against the Orient 30-pin map.
- Battery connector polarity: measure the cell lead before the first plug-in.
- Bench-kit pin labels come from vendor docs and memory; check each silkscreen.
- The concept 4 panel side is a signal list, not a pinout: Sony publishes no datasheet.

## Checks

`make_schematics.py` writes a netlist CSV per sheet into `netlists/`, plus `pod0-all.csv` across the three Pod-0 sheets. It reports any net with only one connection; the Pod-0 set currently passes. The netlist is the starting point for KiCad capture: every pin, net and part number is in it.

## Editing

Sheets are generated. Change `make_schematics.py` (sheets) or `schem.py` (drawing), then:

```
python3 make_schematics.py            # all sheets
python3 make_schematics.py pod0_3_puck  # one sheet
```
