# Pod-0 / Pod-1 custom PCB: parts, pinouts, reference circuits

Research date: 2026-10-14. Scope: a 2-board design (POD on the head, PUCK on the strap or in a pocket) for the Hiroglass Pod-0 clip-on HUD, drawn so the schematic can be captured pin by pin.

## How the facts were checked

| Mark | Meaning |
|---|---|
| **DS** | Read today in the manufacturer datasheet PDF (text extracted locally). The revision is given in the source list. |
| **mem** | From memory or a standard (USB-C pinout, distributor part-number patterns). This has **not** been verified today. Check it before layout. |
| **snap** | A price or stock figure from the 2026-10-13 research (`sourceable_parts.md`). It may be stale. |
| **unv** | Not verified. |

**Distributor data is the weak spot.** digikey.com returns HTTP 403 to automated fetches, and mouser.com was not reached. No stock figure in this file was seen live today. DigiKey and Mouser part numbers marked *mem* follow each distributor's usual numbering scheme, so look each one up before you order. The datasheets are the strong part: pin numbers and passive values marked DS come straight from the manufacturers' PDFs.

---

## 1. Architecture decisions made here

1. **The OLED is the bare Orient Display AOM12864A0-0.96WW.** It is the only 0.96" 128x64 panel whose datasheet I could get and read in full. Its 30-pin FPC has a 0.5 mm pitch, a 0.3 mm thick tail and a stiffener, so it fits a ZIF connector (DS).
   - The datasheet is inconsistent about the controller. The title block says "SSD1315 or compatible", the spec list says "Driver IC: SSD1306", and DigiKey lists SSD1306. SSD1315 accepts the SSD1306 command set, and the goggl1 SSD1309 driver is command-compatible apart from charge-pump setup (command 0x8D). Firmware has to send `8D 14` (enable the charge pump), which SSD1309 code does not do.
   - Winstar WEO012864DWPP3N00006 is the second source. Its datasheet (texim-europe.com) would not download, so its FPC pin compatibility is **unv**. The 30-pin layout of this panel class (C2P/C2N/C1P/C1N/VBAT/NC/VSS/VDD/BS0-2/CS#/RES#/D/C#/R/W#/E/RD#/D0-D7/IREF/VCOMH/VCC/VLSS) is the same one Winstar's search snippet listed.
2. **The OLED uses its internal charge pump.** VBAT must be 3.5-4.2 V (DS), not 3.3 V, so the pod needs a battery-level rail. The tether therefore carries **VCELL**, the cell voltage, as well as **VSYS**. Orient's reference circuit puts a P/N FET switch (FDN338P + FDN335N) on VBAT, and Orient says the switch is mandatory: "add Electronic Switch circuit, otherwise ... leak current". It is driven from GPIO10.
3. **The 3.3 V LDO must deliver at least 500 mA.** Espressif specifies that the external supply deliver at least 0.5 A (DS, MINI-1 datasheet §5, I_VDD). That rules out the TPS7A02, which is rated 200 mA.
   - Primary: **AP2112K-3.3TRG1**, 600 mA, Iq 55 µA, 250 mV dropout at 600 mA (DS).
   - Alternate: **TI TLV75733PDBVR**, 1 A, I_GND 33 µA (DS). Both are SOT-23-5 with the same pinout (DS), so either can go on one footprint.
   - The XC6220 (1 A, about 8 µA Iq) is a possible third option (**unv**).
4. **The BQ24074 has no SYSOFF pin.** On the BQ24074, pin 15 is **ITERM**; SYSOFF exists only on the '75 and '79 (DS). The power switch therefore cannot use SYSOFF. Instead, the puck's slide switch drives the pod LDO's **EN** through the tether (PWR_EN). The switch then carries only microamps, and the LDO draws about 0.01 µA when off. The charger and fuel gauge stay connected to the cell.
5. **The pod's USB-C VBUS goes down the tether to the puck.** There it is diode-ORed with the puck's own USB-C into the BQ24074 IN pin. Either port then charges the cell and powers the system, and the pod's port also carries USB data for flashing. Without the puck attached, the pod does not run. An optional DNP Schottky from VBUS to the LDO input would allow bench bring-up.
6. **USB-C on the pod: a 24-pin, full-featured receptacle.**
   - Recommended: **Amphenol ICC 12401610E4#2A**, a 24-position SMT right-angle receptacle rated USB 3.2 Gen 2 (DigiKey lists it; the search snapshot showed a 20-week lead time, and stock is **unv**). Route TX1/TX2/RX1/RX2 and SBU1/SBU2 to a pad field or test points so a later board revision can add DP alt mode. The ESP32-C3 itself cannot source DisplayPort.
   - **GCT USB4110-GF-A is *not* full-featured.** DigiKey lists it as "24 (16+8 Dummy)", USB 2.0. USB4085 is also a USB 2.0 part (*mem*).
   - Hirose CX70M-24P1 (24-pin, USB 3.2 Gen 1, mid-mount) is listed at DigiKey, but its lifecycle status is disputed (one source says EOL), so it is **unv**.
   - If no 24-pin part is in stock, fall back to **GCT USB4105-GF-A** (16-pin USB 2.0) and defer DP.
7. **GPIO changes from goggl1.** Keep SCK=4, MOSI=6, CS=7 and DC=1. Move OLED RST from GPIO2 to GPIO3, which is a one-line change in `pins_*.h`. GPIO2 becomes I2C SCL, so its 4.7 k pull-up also serves as the GPIO2 pull-up that Espressif recommends.
8. **Battery.** I found no DigiKey or Mouser stocked 400-600 mAh LiPo with a built-in 10 k NTC and JST-PH. The plan is an Adafruit 500 mAh cell (#1578, on DigiKey; it has no thermistor) plus a 10 k NTC on the puck PCB, bonded to the cell with Kapton or thermal tape. A 3-wire NTC pack from a battery house (Jauch or LiPol custom) is a Pod-1 production item.

---

## 2. BOM: active and electromechanical parts

Prices are USD at qty 1. "Stock today" is **not verified** for every row (see the note at the top).

### POD board

| Ref | Part | MPN | Mfr | Package | DigiKey PN | Mouser PN | Qty-1 price | Stock seen | Datasheet |
|---|---|---|---|---|---|---|---|---|---|
| U1 | Wi-Fi/BLE module | ESP32-C3-MINI-1-N4 (or -1U-N4 with a u.FL connector) | Espressif | 53-pad LGA module, 13.2x16.6 mm | 1965-ESP32-C3-MINI-1-N4CT-ND (*mem*) | 356-ESP32C3MINI1N4 (*mem*) | $3.26 (*snap*) | 2,779 at DigiKey (*snap*) | [MINI-1 v2.2](https://documentation.espressif.com/esp32-c3-mini-1_datasheet_en.pdf) |
| U2 | 6-axis IMU | BMI270 | Bosch Sensortec | LGA-14, 2.5x3.0x0.83 mm | 828-1082-1-ND (*mem*) | 262-BMI270 (*mem*) | $4.23 (*snap*) | about 50k at DigiKey (*snap*) | [BST-BMI270-DS000-08](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi270-ds000.pdf) |
| U3 | Ambient light sensor | VEML7700-TT (the -TR is the same die in a different reel orientation) | Vishay | 4-pin SMD, 6.8x2.35x3.0 mm | VEML7700-TTCT-ND (*mem*) | 78-VEML7700-TT (*mem*) | $1.53 DigiKey / $1.58 Mouser (*snap*) | 4,895 at Mouser (*snap*) | [VEML7700 84286](https://www.vishay.com/docs/84286/veml7700.pdf) |
| U4 | 3.3 V LDO, 600 mA | AP2112K-3.3TRG1 | Diodes Inc. | SOT-23-5 (SOT25) | AP2112K-3.3TRG1DICT-ND (*mem*) | 621-AP2112K-3.3TRG1 (*mem*) | about $0.45 (*unv*) | *unv* | [AP2112](https://www.diodes.com/assets/Datasheets/AP2112.pdf) |
| U4-alt | 3.3 V LDO, 1 A | TLV75733PDBVR | TI | SOT-23-5 (DBV) | 296-TLV75733PDBVRCT-ND (*mem*) | 595-TLV75733PDBVR (*mem*) | about $0.40 (*unv*) | *unv* | [TLV757P](https://www.ti.com/lit/ds/symlink/tlv757p.pdf) |
| U5 | USB ESD | USBLC6-2SC6 | ST | SOT-23-6 | 497-5235-1-ND (*mem*) | 511-USBLC6-2SC6 (*mem*) | about $0.50 (*unv*) | *unv* | [USBLC6-2](https://www.st.com/resource/en/datasheet/usblc6-2.pdf) |
| U5-alt | USB ESD | TPD2E2U06DRLR | TI | SOT-5X3 | *unv* | 595-TPD2E2U06DRLR (*mem*) | *unv* | *unv* | ti.com |
| DS1 | OLED panel, 0.96" 128x64, white | AOM12864A0-0.96WW | Orient Display | 30-pin FPC, 0.5 mm pitch, 0.3 mm thick | (DigiKey product 22531883) | not on Mouser (*unv*) | $11.57 (*snap*, live 10-13) | 0 at DigiKey, 8-week lead (*snap*) | [Orient datasheet Rev A](https://orientdisplay.com/products/oled/aom12864a0-0-96ww-datasheet) |
| DS1-alt | OLED panel | WEO012864DWPP3N00006 | Winstar | 30-pin FPC (pitch *unv*) | (DigiKey Marketplace 20533252) | *unv* | $5.24 (*snap*) | 4 (*snap*) | texim-europe.com, file id 134744 |
| J1 | FPC ZIF, 30-pin, 0.5 mm, right-angle | FH12-30S-0.5SH(55) | Hirose | SMT | HFJ130CT-ND (*mem*) | 798-FH12-30S-0.5SH55 (*mem*) | about $1.40 (*unv*) | *unv* | hirose.com FH12. Contact side (top or bottom) is *unv*; check the panel's contact face against it |
| J2 | USB-C 24-pin, full-featured | 12401610E4#2A | Amphenol ICC | SMT right-angle | (DigiKey product 5775519) | 523-12401610E4#2A (*mem*) | *unv* | 20-week lead (*snap*) | amphenol-icc.com |
| J2-alt | USB-C 16-pin, USB 2.0 | USB4105-GF-A | GCT | SMT + THT pegs | 2073-USB4105-GF-ACT-ND (*mem*) | 640-USB4105-GF-A (*mem*) | about $0.80 (*unv*) | *unv* | gct.co |
| J3 | Tether, 10-pin, 1.0 mm | SM10B-SRSS-TB(LF)(SN) | JST | SMT right-angle | 455-1797-1-ND (*unv*) | 306-SM10B-SRSSTBLFSN (*mem*) | about $0.90 (*unv*) | *unv* | jst-mfg.com SH |
| SW1, SW2 | BOOT, RESET tactile | PTS810 SJM 250 SMTR LFS | C&K | 4.2x3.2 mm SMT | CKN10685CT-ND (*unv*) | 611-PTS810SJM250SMTR (*mem*) | about $0.25 (*unv*) | *unv* | ckswitches.com |
| D1 | Status LED, green | LTST-C191KGKT | Lite-On | 0603 | 160-1446-1-ND (*mem*) | 859-LTST-C191KGKT (*mem*) | about $0.30 (*unv*) | *unv* | liteon.com |
| Q1 | OLED VBAT high-side switch | FDN338P | onsemi | SOT-23 | FDN338PCT-ND (*mem*) | 512-FDN338P (*mem*) | about $0.40 (*unv*) | *unv* | onsemi.com |
| Q2 | Q1 gate driver | FDN335N | onsemi | SOT-23 | FDN335NCT-ND (*mem*) | 512-FDN335N (*mem*) | about $0.40 (*unv*) | *unv* | onsemi.com |

### PUCK board

| Ref | Part | MPN | Mfr | Package | DigiKey PN | Mouser PN | Qty-1 price | Stock seen | Datasheet |
|---|---|---|---|---|---|---|---|---|---|
| U10 | Li-ion charger with power path | BQ24074RGTR (or RGTT, a 250-piece reel) | TI | VQFN-16 RGT, 3x3 mm, with exposed pad | 296-xxxx (*unv*) | 595-BQ24074RGTR (*mem*) | $3.19 (*snap*) | 9,247 cut tape at DigiKey (*snap*) | [SLUS810N, Oct 2021](https://www.ti.com/lit/ds/symlink/bq24074.pdf) |
| U11 (optional) | Fuel gauge | MAX17048G+T10 | Analog Devices (Maxim) | TDFN-8, 2x2 mm, with exposed pad | MAX17048G+T10CT-ND (*mem*) | 700-MAX17048G+T10 (*mem*) | about $2.50 (*unv*) | *unv* | [MAX17048/49](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX17048-MAX17049.pdf) (download failed; pins are *mem*) |
| BT1 | 1S LiPo, 500 mAh, 2-pin JST-PH, protection board, no NTC | 1578 | Adafruit | 29x36x4.75 mm (*mem*) | 1528-1858-ND (*unv*) | not stocked | about $7.95 (*unv*) | *unv* | adafruit.com/product/1578 |
| RT1 | 10 k NTC, B=3380, bonded to the cell | NCP15XH103F03RC | Murata | 0402 | 490-4801-1-ND (*mem*) | 81-NCP15XH103F03RC (*mem*) | about $0.10 | *unv* | murata.com |
| J10 | USB-C 16-pin (charging) | USB4105-GF-A | GCT | SMT + THT | as J2-alt | as J2-alt | | | |
| J11 | Battery, JST-PH 2-pin, SMT right-angle | S2B-PH-SM4-TB(LF)(SN) | JST | SMT | 455-1749-1-ND (*mem*) | 306-S2BPHSM4TBLFSN (*mem*) | about $0.40 | *unv* | jst-mfg.com PH |
| J12 | Tether, 10-pin | SM10B-SRSS-TB(LF)(SN) | JST | SMT | as J3 | as J3 | | | |
| SW10 | Power slide switch (switches PWR_EN, µA only) | PCM12SMTR | C&K | SMT SPDT | 401-2016-1-ND (*unv*) | 611-PCM12SMTR (*mem*) | about $0.70 | *unv* | ckswitches.com |
| D10, D11 | Input OR Schottkys, 1 A | PMEG2010AEH | Nexperia | SOD-123F | 1727-xxxx (*unv*) | 771-PMEG2010AEH-T/R (*unv*) | about $0.35 | *unv* | nexperia.com |
| D12, D13 | CHG (red) and PGOOD (green) LEDs | LTST-C191KRKT / LTST-C191KGKT | Lite-On | 0603 | 160-1447-1-ND / 160-1446-1-ND (*mem*) | 859-... | about $0.30 | | |
| — | Tether cable, 10-pin SH, both ends | A10SR10SR30K152B (*unv*) or JST pre-crimped A1010 wires + SHR-10V-S housings | JST / Molex | — | *unv* | *unv* | | | |

---

## 3. Per-IC pin tables (schematic-ready)

### U1 ESP32-C3-MINI-1 (DS, datasheet v2.2, Table 3-1; 53 pins)

| Pin(s) | Name | Connect to |
|---|---|---|
| 1, 2, 11, 14, 36-53 | GND | GND. Pins 36-53 include the central GND pads. |
| 3 | 3V3 | +3V3. Place 10 µF + 0.1 µF next to the pin (Espressif reference C1/C2). |
| 4, 7, 9, 10, 15, 17, 24, 25, 28, 29, 32-35 | NC | no connection |
| 5 | IO2 | **I2C_SCL**. A 4.7 k pull-up to 3V3 doubles as the recommended GPIO2 pull-up (strapping). |
| 6 | IO3 | **OLED_RES#** |
| 8 | EN | 10 k to 3V3, 1 µF to GND, RESET button to GND |
| 12 | IO0 | **IMU_INT1** (BMI270 pin 4) |
| 13 | IO1 | **OLED_D/C#** |
| 16 | IO10 | **OLED_VBAT_EN**, to the Q2 gate, with a 47 k pull-down |
| 18 | IO4 | **OLED_SCLK** (panel D0) |
| 19 | IO5 | **I2C_SDA** (4.7 k pull-up) |
| 20 | IO6 | **OLED_SDIN** (panel D1) |
| 21 | IO7 | **OLED_CS#** |
| 22 | IO8 | **LED#**: the LED cathode via 1 k (anode at 3V3), plus a 10 k pull-up (strapping; must be 1 for download boot) |
| 23 | IO9 | **BOOT** button to GND, 10 k pull-up (strapping; 0 means download boot) |
| 26 | IO18 | USB_D− (0 Ω series, Espressif R6) |
| 27 | IO19 | USB_D+ (0 Ω series, Espressif R4) |
| 30 | RXD0 (GPIO20) | test pad (UART0 RX) |
| 31 | TXD0 (GPIO21) | test pad (UART0 TX) |

Boot straps (DS Table 4-3): SPI boot is GPIO9=1. Joint download (USB-Serial-JTAG or UART) is GPIO8=1 with GPIO9=0. GPIO2 "does not determine" boot mode, but Espressif recommends pulling it up. The default straps are GPIO2 floating, GPIO8 floating and GPIO9 weak pull-up.

### U2 BMI270 (DS, Table 22; I2C mode, address 0x68 with SDO to GND)

| Pin | Name | Connect to |
|---|---|---|
| 1 | SDO | GND (address 0x68) |
| 2 | ASDx | VDDIO (or NC; never GND) |
| 3 | ASCx | VDDIO (or NC; never GND) |
| 4 | INT1 | GPIO0 |
| 5 | VDDIO | 3V3, 100 nF |
| 6 | GNDIO | GND |
| 7 | GND | GND |
| 8 | VDD | 3V3, 100 nF |
| 9 | INT2 | NC (Bosch: "do not connect" when unused) |
| 10 | OCSB | NC (may go to GND only if ois_en=0) |
| 11 | OSDO | NC |
| 12 | CSB | VDDIO (selects I2C) |
| 13 | SCx | I2C_SCL |
| 14 | SDx | I2C_SDA |

### U3 VEML7700 (DS; address 0x10 fixed, no INT pin)

| Pin | Name | Connect to |
|---|---|---|
| 1 | SCL | I2C_SCL |
| 2 | VDD | 3V3 (2.5-3.6 V), 100 nF. The optional 10 Ω + 10 µF filter in the application circuit is not needed here. |
| 3 | GND | GND |
| 4 | SDA | I2C_SDA |

Vishay recommends 2.2-4.7 k I2C pull-ups.

### U4 AP2112K-3.3 / U4-alt TLV75733PDBVR (DS; same pinout)

| Pin | AP2112K (SOT25) | TLV757P (DBV) | Connect to |
|---|---|---|---|
| 1 | VIN | IN | VSYS from the tether, 1 µF (DS minimum; use 4.7 µF because of cable inductance) |
| 2 | GND | GND | GND |
| 3 | EN | EN | PWR_EN from the tether, 100 k to GND |
| 4 | NC | NC | NC |
| 5 | VOUT | OUT | +3V3, 1 µF (DS minimum, X5R/X7R) plus the module's 10 µF |

### U5 USBLC6-2SC6 (*mem*; check against the ST datasheet)

| Pin | Name | Connect to |
|---|---|---|
| 1 | I/O1 | D− (connector side) |
| 6 | I/O1 | D− (to IO18) |
| 3 | I/O2 | D+ (connector side) |
| 4 | I/O2 | D+ (to IO19) |
| 2 | GND | GND |
| 5 | VBUS | VBUS |

Pins 1/6 and 3/4 are flow-through pairs on the same line.

### DS1 OLED AOM12864A0-0.96WW, 30-pin FPC to J1 (DS, Orient p.3-4 and the 4-wire SPI reference circuit on p.11)

| Pin | Symbol | Connect to (4-wire SPI, internal charge pump) |
|---|---|---|
| 1 | NC (GND) | GND |
| 2 | C2P | C2 1 µF/16 V X5R to pin 3 |
| 3 | C2N | C2 |
| 4 | C1P | C1 1 µF/16 V X5R to pin 5 |
| 5 | C1N | C1 |
| 6 | VBAT (labelled VDDB in the figure) | OLED_VBAT (Q1 drain), C6 1 µF |
| 7 | NC | NC |
| 8 | VSS | GND |
| 9 | VDD | 3V3 (spec 1.65-3.3 V), C5 1 µF |
| 10 | BS0 | GND |
| 11 | BS1 | GND |
| 12 | BS2 | GND (BS2..0 = 000 selects 4-wire SPI) |
| 13 | CS# | GPIO7 |
| 14 | RES# | GPIO3 |
| 15 | D/C# | GPIO1 |
| 16 | R/W# | R4 4.7 k to GND |
| 17 | E/RD# | R5 4.7 k to GND |
| 18 | D0 (SCLK) | GPIO4 |
| 19 | D1 (SDIN) | GPIO6 |
| 20 | D2 | NC (the datasheet says unused pins go to VSS "except for D2 in serial mode") |
| 21-25 | D3-D7 | GND |
| 26 | IREF | R1 620 k to GND (DS) |
| 27 | VCOMH | C4 4.7 µF/16 V X7R to GND |
| 28 | VCC | C3 2.2 µF/16 V X7R to GND (7.0-7.5 V from the pump) |
| 29 | VLSS | GND |
| 30 | NC (GND) | GND |

The **VBAT switch** follows the datasheet reference: VCELL connects to the Q1 (FDN338P) source, and the drain feeds OLED VBAT. R2 47 k runs from Q1 gate to source. The Q1 gate connects to the Q2 (FDN335N) drain, Q2's source goes to GND, and Q2's gate connects to GPIO10 with R3 47 k to GND.

The VBAT range is 3.5-4.2 V and the absolute maximum is 4.5 V. That is why VBAT comes from VCELL rather than VSYS, since VSYS reaches 4.4 V on USB power.

Luminance is 80 min / 110 typ cd/m², and 120 nits on the drawing. Operating range is −40 to +85 °C.

On IREF, the datasheet's figure of 620 k comes with a confusingly worded footnote. The SSD1306 formula I_REF ≈ (VCC − 2.5 V)/R ≈ 10 µA gives about 470-510 k at VCC = 7.5 V. Lay out a single 0402 resistor and start at 620 k. Drop to 470-560 k if the panel is too dim (**unv**).

### J2 USB-C (24-pin, standard receptacle assignment, *mem*)

| Pin | Signal | Pod net |
|---|---|---|
| A1, A12, B1, B12 | GND | GND |
| A4, A9, B4, B9 | VBUS | VBUS_POD, to USBLC6 pin 5 and tether pin 1; 1 µF to GND |
| A5 | CC1 | 5.1 k to GND |
| B5 | CC2 | 5.1 k to GND (separate resistor) |
| A6, B6 | D+ | joined, to USBLC6, then IO19 |
| A7, B7 | D− | joined, to USBLC6, then IO18 |
| A8 / B8 | SBU1 / SBU2 | to test pads (future DP AUX) |
| A2/A3, B2/B3 | TX1+/−, TX2+/− | to test pads / reserved diff pairs (future DP lanes) |
| A10/A11, B10/B11 | RX2−/+, RX1−/+ | to test pads / reserved |
| Shell | shield | GND, or GND through 1 M ∥ 4.7 nF (optional) |

On USB4105-GF-A (16-pin), the TX/RX pins are absent, and the GND and VBUS pairs share pads.

### J3 / J12 Tether, JST SH 10-pin (pin 1 is the same net on both boards)

| Pin | Net | Notes |
|---|---|---|
| 1 | VBUS_POD | pod USB VBUS, to puck D11, then BQ24074 IN |
| 2 | VSYS | BQ24074 OUT, to pod LDO IN |
| 3 | VSYS | paralleled (SH is about 1 A per pin) |
| 4 | GND | |
| 5 | GND | |
| 6 | VCELL | cell +, to the OLED VBAT switch (about 20 mA) |
| 7 | PWR_EN | puck SW10 connects it to VSYS; drives pod LDO EN |
| 8 | I2C_SDA | to MAX17048. Pull-ups are on the pod only. |
| 9 | I2C_SCL | to MAX17048 |
| 10 | SPARE | MAX17048 ALRT# or BQ CHG#. On the pod: a test pad plus a DNP 0 Ω to RXD0/GPIO20. |

### U10 BQ24074 (DS, SLUS810N; RGT VQFN-16)

| Pin | Name | Connect to | Why (datasheet) |
|---|---|---|---|
| 1 | TS | RT1 10 k NTC to VSS | 75 µA bias current. VCOLD = 2.1 V and VHOT = 0.30 V, which with a 10 k Type-2 NTC gives **charge only between 0 °C and 50 °C, with 3 °C hysteresis**. Below 0 °C, charging is suspended (DS). |
| 2, 3 | BAT | cell + (J11 pin 1), 4.7 µF | 4.7-47 µF |
| 4 | CE# | VSS | internal 285 k pull-down; do not float |
| 5 | EN2 | VSS | EN2/EN1 = 0/1 selects USB500 mode (500 mA input) |
| 6 | EN1 | OUT (or a 10 k pull-up to OUT) | see above |
| 7 | PGOOD# | green LED + 1.5 k to OUT | open drain |
| 8 | VSS | GND | |
| 9 | CHG# | red LED + 1.5 k to OUT (optionally also to tether pin 10) | open drain |
| 10, 11 | OUT | VSYS, 4.7 µF (4.7-47 µF) | regulates to 4.4 V on USB, otherwise tracks VBAT |
| 12 | ILIM | **3.09 k** 1% to VSS | I_IN = 1550/R = 502 mA in EN2=1/EN1=0 mode. Fitted anyway; "leaving ILIM unconnected disables all charging". |
| 13 | IN | cathodes of D10 (puck USB VBUS) and D11 (VBUS_POD), 1 µF/25 V | 4.35-10.5 V operating, 28 V absolute maximum |
| 14 | TMR | open | default timers: precharge 30 min, fast charge 5 h (DS) |
| 15 | ITERM | open | default termination is 10 % of I_CHG = 50 mA |
| 16 | ISET | **1.78 k** 1% to VSS | I_CHG = 890/R = 500 mA (K_ISET 797-975, so ±10 %) |
| EP | thermal pad | VSS | |

At 500 mA the charge rate is 1C for a 500 mAh cell. That is the usual maximum. For a 400 mAh cell use ISET = 2.2 k (about 405 mA). On a 500 mA USB500 input, the system load takes priority through the power path, so charging slows while Wi-Fi is active.

### U11 MAX17048 (optional; TDFN-8; *mem*, check against the ADI datasheet). Address 0x36.

| Pin | Name | Connect to |
|---|---|---|
| 1 | CTG | GND |
| 2 | CELL | cell + (Kelvin trace to J11) |
| 3 | VDD | cell +, 1 µF (*unv*) |
| 4 | GND | GND |
| 5 | ALRT | tether pin 10 (open drain; pull-up on the pod, 10 k to 3V3) |
| 6 | QSTRT | GND |
| 7 | SCL | tether pin 9 |
| 8 | SDA | tether pin 8 |
| EP | — | GND |

When the pod is off, its 3V3 pull-ups disappear, so SDA and SCL sit low. The MAX17048 may then enter sleep after about 2.5 s with both lines low (**unv**), which is harmless.

---

## 4. GPIO map (pod)

| GPIO | Module pin | Function | Notes |
|---|---|---|---|
| 0 | 12 | IMU_INT1 | also XTAL_32K_P (unused) |
| 1 | 13 | OLED D/C# | same as goggl1 |
| 2 | 5 | I2C SCL | strapping pin; the 4.7 k pull-up keeps it high, as Espressif recommends |
| 3 | 6 | OLED RES# | **moved from goggl1 GPIO2** |
| 4 | 18 | OLED SCLK | same as goggl1 |
| 5 | 19 | I2C SDA | (goggl1's MISO, which it never used) |
| 6 | 20 | OLED SDIN (MOSI) | same as goggl1 |
| 7 | 21 | OLED CS# | same as goggl1 |
| 8 | 22 | Status LED, active low (anode at 3V3) | strapping pin: 10 k pull-up, high at boot |
| 9 | 23 | BOOT button, active low | strapping pin: 10 k pull-up |
| 10 | 16 | OLED_VBAT_EN, active high | 47 k pull-down holds the panel unpowered through reset |
| 18 / 19 | 26 / 27 | USB D− / D+ | native USB-Serial-JTAG |
| 20 / 21 | 30 / 31 | UART0 RX / TX | test pads; 20 can take tether SPARE |

Firmware changes: set `GOGGL_PIN_RST` to 3, `GOGGL_PIN_LED` to 8 and make it active low, add an OLED VBAT enable on GPIO10 (switch it on before `8D 14 / AF`), and add I2C on SDA 5 / SCL 2.

I2C addresses: BMI270 0x68, VEML7700 0x10, MAX17048 0x36. They do not conflict.

---

## 5. Passives

All values come from the datasheets cited above unless noted. The example parts are generic. The Yageo DigiKey numbers follow the stable `311-<value>LRCT-ND` scheme. The Samsung and Murata DigiKey numbers are *mem*, so look them up by MPN.

### POD

| Ref | Value | Size | Purpose | Example MPN / DigiKey PN |
|---|---|---|---|---|
| C1 | 10 µF 10 V X5R | 0603 | ESP 3V3 bulk (Espressif C1) | Samsung CL10A106KP8NNNC / 1276-1871-1-ND (*mem*) |
| C2 | 100 nF 16 V X7R | 0402 | ESP 3V3 (Espressif C2) | Samsung CL05B104KO5NNNC / 1276-1001-1-ND |
| C3 | 1 µF 16 V X5R | 0402 | EN delay (with R1) | Samsung CL05A105KO5NNNC (*mem*) |
| R1 | 10 k 1% | 0402 | EN pull-up | Yageo RC0402FR-0710KL / 311-10.0KLRCT-ND |
| R2, R3 | 10 k | 0402 | GPIO9 and GPIO8 pull-ups | as R1 |
| R4, R5 | 4.7 k | 0402 | I2C SDA/SCL pull-ups (also the GPIO2 strap) | RC0402FR-074K7L / 311-4.70KLRCT-ND |
| R6 | 1 k | 0402 | status LED (about 1.3 mA) | RC0402FR-071KL / 311-1.00KLRCT-ND |
| R7, R8 | 5.1 k 1% | 0402 | USB-C CC1, CC2 | RC0402FR-075K1L / 311-5.10KLRCT-ND |
| R9, R10 | 0 Ω | 0402 | USB D+/D− series (Espressif R4/R6; can be swapped for 22 Ω) | RC0402JR-070RL / 311-0.0JRCT-ND |
| C4 | 1 µF 16 V | 0402 | VBUS bypass at the connector | as C3 |
| C5 | 4.7 µF 10 V X5R | 0603 | LDO input | Samsung CL10A475KP8NNNC (*mem*) |
| C6 | 1 µF 16 V X5R | 0402 | LDO output (the 10 µF C1 sits on the same rail) | as C3 |
| R11 | 100 k | 0402 | LDO EN pull-down | RC0402FR-07100KL / 311-100KLRCT-ND |
| C7, C8 | 100 nF | 0402 | BMI270 VDD, VDDIO | as C2 |
| C9 | 100 nF | 0402 | VEML7700 VDD | as C2 |
| C10, C11 | 1 µF 16 V X5R | 0603 | OLED C1P-C1N, C2P-C2N flying capacitors | Murata GRM188R61C105KA93D (*mem*) |
| C12 | 2.2 µF 16 V X7R | 0603 | OLED VCC | Murata GRM188R71C225KE11D (*mem*) |
| C13 | 4.7 µF 16 V X7R | 0805 | OLED VCOMH | Murata GRM21BR71C475KE51L (*mem*) |
| C14, C15 | 1 µF 6.3 V+ X5R | 0402 | OLED VDD (C5), VBAT (C6) | as C3 |
| R12 | 620 k 1% | 0402 | OLED IREF | RC0402FR-07620KL / 311-620KLRCT-ND |
| R13, R14 | 4.7 k | 0402 | OLED R/W#, E/RD# to GND | as R4 |
| R15, R16 | 47 k | 0402 | Q1 gate-source, Q2 gate pull-down | RC0402FR-0747KL / 311-47.0KLRCT-ND |
| R17 | 10 k | 0402 | ALRT/SPARE pull-up (if the fuel gauge is fitted) | as R1 |
| R18 | 0 Ω DNP | 0402 | SPARE to GPIO20 | as R9 |

### PUCK

| Ref | Value | Size | Purpose | Example MPN / DigiKey PN |
|---|---|---|---|---|
| C20 | 1 µF 25 V X5R | 0603 | BQ24074 IN | Samsung CL10A105KA8NNNC (*mem*) |
| C21 | 4.7 µF 10 V X5R | 0603 | BQ24074 OUT | Samsung CL10A475KP8NNNC (*mem*) |
| C22 | 4.7 µF 10 V X5R | 0603 | BQ24074 BAT | same |
| R20 | 1.78 k 1% | 0402 | ISET, 500 mA | RC0402FR-071K78L / 311-1.78KLRCT-ND |
| R21 | 3.09 k 1% | 0402 | ILIM, 502 mA | RC0402FR-073K09L / 311-3.09KLRCT-ND |
| R22 | 10 k | 0402 | EN1 to OUT | as R1 |
| R23, R24 | 1.5 k | 0402 | CHG# and PGOOD# LEDs | RC0402FR-071K5L / 311-1.50KLRCT-ND |
| R25, R26 | 5.1 k 1% | 0402 | puck USB-C CC1, CC2 | as R7 |
| C23 | 1 µF 25 V | 0603 | puck VBUS bypass | as C20 |
| R27 | 100 k | 0402 | PWR_EN pull-down at the switch (optional; R11 is on the pod) | as R11 |
| C24 | 1 µF 16 V | 0402 | MAX17048 VDD (optional) | as C3 |
| RT1 | 10 k NTC | 0402 | TS, bonded to the cell | Murata NCP15XH103F03RC / 490-4801-1-ND (*mem*) |

---

## 6. Open items before layout

1. Look up every DigiKey and Mouser part number, price and stock by hand. None was visible to automation today.
2. Get the Winstar WEO012864D datasheet (texim-europe file 134744, or winstar.com.tw) and compare its 30-pin FPC map with the Orient part. Then check the FH12 contact side against both panels' contact faces.
3. Check the USBLC6-2SC6 and MAX17048 pin tables against their PDFs. Both downloads failed today, so those tables are *mem*.
4. Check stock and lead time for the 24-pin USB-C (Amphenol 12401610E4#2A). If it is not available, use USB4105-GF-A.
5. The board size target of 25 mm or less conflicts with the panel: the 0.96" panel glass is 26.7 mm wide. It mounts in the optics cartridge, not on the PCB, and only the FPC reaches J1.
6. On the MINI-1 module, keep a copper-free antenna keep-out per Espressif §10.

## Sources

- Espressif ESP32-C3-MINI-1 / MINI-1U Datasheet v2.2: https://documentation.espressif.com/esp32-c3-mini-1_datasheet_en.pdf (pin table 3-1, boot straps table 4-3, peripheral schematic §9, I_VDD ≥ 0.5 A)
- Bosch BMI270 datasheet BST-BMI270-DS000-08: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi270-ds000.pdf (Table 22, I2C address §, 100 nF decoupling)
- Vishay VEML7700 datasheet (doc 84286): https://www.vishay.com/docs/84286/veml7700.pdf
- TI BQ24074 SLUS810N: https://www.ti.com/lit/ds/symlink/bq24074.pdf (Table 7-1, 7-2; K_ISET 890, K_ILIM 1550, K_TMR 48; TS thresholds)
- Diodes AP2112: https://www.diodes.com/assets/Datasheets/AP2112.pdf
- TI TLV757P: https://www.ti.com/lit/ds/symlink/tlv757p.pdf
- Orient Display AOM12864A0-0.96WW Rev A: https://orientdisplay.com/products/oled/aom12864a0-0-96ww-datasheet
- DigiKey USB4110-GF-A listing (16+8 dummy, USB 2.0): https://www.digikey.com/en/products/detail/gct/USB4110-GF-A/10384547
- DigiKey Amphenol 12401610E4#2A: https://www.digikey.com/en/products/detail/amphenol-icc-commercial-products/12401610E4-2A/5775519
- DigiKey Hirose CX70M-24P1: https://www.digikey.com/en/products/detail/hirose-electric-co-ltd/CX70M-24P1/6036266
- Earlier price snapshots: `../Open hardware XR wearable approaches/sourceable_parts.md`
