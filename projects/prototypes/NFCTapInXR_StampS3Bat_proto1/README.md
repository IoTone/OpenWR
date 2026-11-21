# NFCTapInXR_StampS3Bat_proto1

Arduino bring-up for NFCTapInXR: an Adafruit PN532 NFC shield (789) wired to an
M5 Stamp-S3Bat DIP over I2C. It proves the wiring, the board's switched 5 V
output and card reads. The reader moves into an ESPIDFORTH build after this works.

Wiring diagram: https://claude.ai/artifact/EbMxLxXUrN2aNy2oGRAUao

## Wiring

| Stamp-S3Bat | Shield | Carries |
|---|---|---|
| `G5` | `A4` | SDA |
| `G6` | `A5` | SCL |
| `G7` | `D2` | IRQ (wired for later; the Arduino library polls over I2C) |
| `GND` | `GND` | Ground |
| `3V3` | `5V` | Shield supply at 3.3 V (see below); wire it before plugging in USB |

Why 3.3 V into the shield's 5V pin: in the v1.3 schematic
([Adafruit-PN532-RFID-NFC-Shield](https://github.com/adafruit/Adafruit-PN532-RFID-NFC-Shield)),
A4/SDA and A5/SCL each have a 10 kΩ pull-up (R13, R14) to the shield's 5V pin,
behind BSS138 level shifters (Q1, Q2). Fed from 5 V or 3×AA, the I2C lines idle
at that voltage, which is above what the ESP32-S3 accepts. Fed from 3.3 V, they
idle at 3.3 V. The shield's ADP122 regulator then passes a little under 3.3 V to
the PN532, which runs from 2.7 V up. IOREF is not connected to anything on the
shield. IRQ (D2) is the PN532's own 3.3 V output, with no shifter.

The shield has about 60 µF of capacitors on its supply. Connect it while the
Stamp is unpowered, not hot.

Do not power the shield from `5VOUT` without a LiPo fitted. On 2026-11-14,
switching `5VOUT` on with the shield attached collapsed the board's supply:
the M5PM1 power-cycled everything every ~4 s and the board could not be
flashed until the wire was removed. The sketch leaves `5VOUT` off
(`READER_ON_5VOUT = false`).

Shield jumpers stay at the I2C default (SEL0 on, SEL1 off). Reader address 0x24.

No battery. Power comes from USB-C, or from an external supply on the `5VIN`
pin. Do not put AA cells on the JST socket: it is the LiPo charger output.

## Before connecting SDA and SCL

With only `3V3`→`5V` and `GND` wired, power the Stamp and measure A4 and A5 to
ground. Both should read about 3.3 V. Anything higher means the shield is being
fed from somewhere else: stop.

## Build and run

```sh
pio run -t upload
pio device monitor
```

If upload can't find the board, hold PWR about 3 seconds with USB connected to
enter download mode.

Expected console:

```
NFCTapInXR bring-up
5VOUT on
reader bus: 0x24
PN532 firmware 1.6
ready, present a card
TAP 04A1B2C3D4E5F6  (7-byte uid, Ultralight/NTAG class)
```

`reader bus: nothing` means the shield isn't answering: check 5VOUT, the SDA/SCL
wires, and the pull-up result above.

## Notes

- The M5PM1 power chip owns the board's own I2C bus (G48 SDA, G47 SCL), which
  does not reach the DIP header. `5VOUT` is off at power-up; the sketch sets
  M5PM1 GPIO1 (`5VOUT_EN`) high. The PMIC's `BOOST5V_EN` pin is unconnected on
  this board, so `setBoostEnable()` does not control it.
- The sketch turns charging off, since no battery is fitted.
- Board definition: PlatformIO has no Stamp-S3Bat; `m5stack-stamps3` matches
  its 8 MB flash and native USB.
