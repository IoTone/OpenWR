# NFCTapInXR_AtomMatrix_proto1

NFC tap-in for XR: an M5 ATOM Matrix reads ISO14443A cards and tags with a
PN532 NFC Module V3 (the red board, sold as "PN532 NFC NXP RFID Module V3 Kit")
and sends each tap over BLE to a lens or phone. The 5×5 LED matrix shows the
link state and each read.

Replaces the Stamp-S3Bat + Adafruit shield attempt
(`../NFCTapInXR_StampS3Bat_proto1`).

## Wiring

Wiring diagram and phone staging steps: https://claude.ai/artifact/8U2xJeTyvCdK5huEYCufSJ

Header positions below are as seen from the ATOM's back (the side with the printed
pin labels); from the LED side the two headers swap sides. Go by the labels.

The reader runs in its serial mode (HSU, 115200 baud). Set the module's mode
switch to the `HSU 0 0` row of its silkscreen table: **both sliders pushed away
from the `ON KE` lettering**. On this particular module I2C and SPI never
answered in any switch setting; only HSU worked (2026-11-20).

The module's serial lines share pads with its other interfaces: its TX is the
pad marked `SDA` (4-pin header) or `MOSI` (8-pin header), its RX is the pad
marked `SCL` or `SS`. Use either header.

| PN532 module | ATOM Matrix | Notes |
|---|---|---|
| `VCC` | `3V3` (left header, top) | Keeps the module's signal lines at 3.3 V |
| `GND` | `GND` (right header, bottom) | |
| `MOSI` or `SDA` (module TX) | `G19` (left header, 3rd) | ATOM receives |
| `SS` or `SCL` (module RX) | `G22` (left header, 2nd) | ATOM transmits |
| `SCK`, `MISO`, `IRQ`, `RSTO` | not connected | |

The Grove port is not used because it carries 5 V, not 3.3 V.

Connect the module with the ATOM unplugged. Its capacitors hot-plugged onto a
running 3.3 V rail can reset the ESP32.

## Tags it reads

ISO14443A (MIFARE Classic, Ultralight, NTAG, DESFire), and it also polls for
FeliCa, ISO14443B and Jewel/Topaz, which are untested here. Receiver gain is
raised to 48 dB for small type A tags.

It cannot read ISO15693 / NFC-V tags such as ST25TV or ICODE: the PN532 chip
does not support that standard. 125 kHz proximity fobs are a different
technology and are not seen either.

## LED matrix

| Pattern | Meaning |
|---|---|
| Blinking blue | Advertising, waiting for a connection |
| Solid green | Connected |
| Rainbow, 1.2 s | Card read |
| Blinking red, 2 s | Connection dropped, then back to blinking blue |
| Orange flash | Bad read, or reader not answering (retried every 2 s) |

Brightness is fixed at 20, M5's limit for this matrix; higher overheats the LEDs
and the cover.

## BLE

- Name `NFCTapIn`, Nordic UART service, no pairing or bonding (Spectacles can't
  bond; see `../../apps/SpecsSwitchbackXR/BLE-NOTES.md`).
- The name is in the advertisement; the NUS UUID is in the scan response.
- Notifications on TX (`6E400003-…`), split into 20-byte chunks:
  - `TAP <id hex>\n` per card, e.g. `TAP 04A1B2C3D4E5F6`
  - `ERR read\n` for a failed read
- Write `ping` to RX (`6E400002-…`) and it answers `pong\n`.
- A card held on the reader taps once; it taps again after it has been away
  for 1.5 s.

The same lines are printed on the USB serial console at 115200.

## Build and flash

```sh
pio run -t upload
pio device monitor
```

If upload fails at 1500000 baud, set `upload_speed = 115200` in
`platformio.ini`.

Expected console:

```
NFCTapInXR ATOM Matrix
ble: advertising as NFCTapIn
nfc: PN532 firmware 1.6
nfc: ready
TAP 04A1B2C3D4E5F6
```

`nfc: no answer from the reader` with the matrix flashing orange every 2 s means
the reader isn't answering: check the mode switch (both sliders at HSU), the two
signal wires (not swapped), VCC and GND. The module reads its switch only at
power-up, so unplug USB after changing it.

## Testing without a lens

Use nRF Connect on a phone: connect to `NFCTapIn`, enable notifications on the
TX characteristic, tap a card.

## Libraries

Adafruit PN532 1.3.4, NimBLE-Arduino 2.2.2 and FastLED 3.9.11 (the same NimBLE
and FastLED versions as the HeadLight ATOM sketches), on PlatformIO
`espressif32@6.9.0`, board `m5stack-atom`. The Adafruit library does start-up
only; polling is the PN532's InAutoPoll command sent directly, because the
library has no call for it and its own arming call blocks over serial. Card
detection is non-blocking: the reader answers on the serial line only when a tag
arrives, so BLE and the LEDs keep running in between.
