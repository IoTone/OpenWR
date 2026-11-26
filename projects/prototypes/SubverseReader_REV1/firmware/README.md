# SubverseReader pass firmware

One source tree in `pass/`, three builds. ESP-IDF via PlatformIO, an ESPIDFORTH console on USB-C
and on Bluetooth (Nordic UART Service; the pass advertises as `SR-15` or `SR-16`).

| Build | Pass | Board | Packs live on | Controls |
|---|---|---|---|---|
| `sr15` | SR-15 Dial Pass | Elecrow CrowPanel ESP32 4.2in (ESP32-S3) | microSD card | dial, MENU, EXIT |
| `sr16_s3` | SR-16 Setlist | XIAO ESP32-S3 + Seeed XIAO ePaper board + 3.7in panel | the XIAO's flash (6.4 MB) | three keys, or none |
| `sr16_c5` | SR-16 Setlist | XIAO ESP32-C5, same boards | the XIAO's flash (6.4 MB) | three keys, or none |

```
cd pass
pio run -e sr15 -t upload          # 460800; the CrowPanel's CH340 drops bytes faster than that
pio run -e sr16_s3 -t upload       # 115200 over the XIAO's own USB port
```

`sr16_c5` needs ESP-IDF 5.5, which only the pioarduino platform packages, and that needs
PlatformIO on Python 3.10 or newer. On this Mac the stock PlatformIO runs on Python 3.9, so the C5
build uses a second PlatformIO kept apart from the first:

```
PLATFORMIO_CORE_DIR=~/.platformio-c5 ~/.platformio-c5/penv/bin/pio run -e sr16_c5 -t upload
```

(`uv venv --python 3.13 ~/.platformio-c5/penv && uv pip install --python ~/.platformio-c5/penv/bin/python platformio`
sets it up.) All three builds compile; **only `sr15` has run on hardware so far.**

## What it does

- Reads `.svpk` packs from `/packs` on the store. Packs are built by `../tools/svpk.py` or the
  phone app, and arrive by `../tools/sr_push.py` over USB or from the app over Bluetooth.
- Shows a shelf, pages, a badge. Page turns are quick refreshes with a full one every eighth.
- Deep-sleeps after `sr-sleep!` seconds idle (0 = never) with the page left on the glass.
- **Auto-turn** (`30 sr-auto!`): a page every 30 s, round and round. The pass sleeps between turns.
- **Page number** (`sr-where`): the title and "page N of M" over the page for five seconds.
- **Panel bench checks** (SR-16): `epd-diag` prints which pin BUSY is on and how long the last
  refresh took (healthy: about 50 ms power-on, 340 ms quick, 1000 ms full). `epd-probe` tests
  each panel line and what BUSY does around a reset; restart the pass afterwards. A power-on
  that never ends is almost always the ribbon.
- **Timed check-in** (`300 sr-sync!`): while asleep, wake every 5 minutes and advertise for 5 s
  without touching the glass, so a waiting phone can connect. On by default only on a pass with
  no keys.

### Controls

| Pass | Shelf | Page | Bluetooth sync window (3 min) |
|---|---|---|---|
| SR-15 | dial moves, push opens, EXIT badge | dial or push turns, MENU page number, EXIT shelf, hold MENU first page, hold EXIT sleep | hold MENU + EXIT |
| SR-16, three keys (`3 sr-keys!`) | PREV/NEXT move, MENU opens, hold MENU badge | PREV/NEXT turn, MENU page number, hold MENU shelf, hold PREV first page | hold PREV + NEXT |
| SR-16N, no keys (`0 sr-keys!`, the default) | BOOT press moves down, hold opens | BOOT press = next page (wraps), hold = next zine | every BOOT hold; power-up; the timed check-in |

A pass with no keys is driven from the phone app's pass screen: PREV, NEXT, pick a zine,
auto-turn, sleep. The XIAO's BOOT button is its one physical key.

### SR-16 wiring

| Signal | XIAO pin | ESP32-C5 | ESP32-S3 |
|---|---|---|---|
| panel RST / CS / DC / SCK / MOSI | D0 / D1 / D3 / D8 / D10 | 1 / 0 / 7 / 8 / 10 | 1 / 2 / 4 / 7 / 9 |
| panel BUSY | D2 (driver board V2) or D5 (older breakout): found at start | 25 or 24 | 3 or 6 |
| PREV / MENU / NEXT keys to GND | D4 / D7 / D9 | 23 / 12 / 9 | 5 / 44 / 8 |
| wake line (C5 only) | back pad MTDI, a Schottky to each key | 3 | not needed |
| battery sense | on the XIAO | 6, enabled by 26 | none |

On the S3, PREV, NEXT and BOOT wake the pass; MENU (D7) can't. On the C5 only the wake line can,
so without the diodes a sleeping C5 pass wakes on its timer or its reset button only.

## First test of an SR-16 (nothing here has been on hardware yet)

1. Plug the panel's ribbon into the driver board, the XIAO into its socket, USB-C into the Mac.
   Flash `sr16_s3` or `sr16_c5`. If the port doesn't show, hold BOOT while plugging in.
2. Open the console (`pio device monitor`, lines end in CR) and type `sr-info`. Expect
   `panel UC8253` and a `pass sr16 240x416 keys 0 ...` line. `panel none` means no BUSY signal:
   check the ribbon and its latch.
3. `sr-test`. The glass should show TOP at the top, readable. Upside down: `2 epd-rot!`.
   Mirrored: `1 epd-rot!` or `3 epd-rot!`. It is kept.
4. `0 sr-sleep!` keeps it awake on the bench.
5. `python3 ../tools/sr_push.py --opds https://spicemine.iotj.cc/opds` builds the catalog for
   the 240x416 panel and sends it. `sr-df` shows the room left.
6. `0 sr-open`, `sr-next`, `sr-prev`. Then press BOOT: next page.
7. Phone: open the app's pass tab, Find my pass (it listens for 3 minutes after power-up),
   then use the remote and send a zine.
8. `120 sr-sleep!`, then check it sleeps and comes back: BOOT on the S3; on any XIAO the app's
   Wait for my pass within 5 minutes.

Things to watch for, because they are guesses until measured:

- **Refresh quality.** The full and quick refresh codes come from Good Display's sample for the
  same class of panel. Ghosting after many quick turns: lower `sr-full-every!` from 8.
- **Check-in window.** 5 s may be short for a phone to notice and connect; it is
  `SYNC_WINDOW_S` in `reader.c` and the `5` in `main.c`.
- **Battery on the C5.** It reads the XIAO's own battery pads. A cell on the driver board's JST
  may not reach them; `sr-bat` says "not wired" then.
- **Sleep current** and whether the panel holds its picture cleanly through the chip's deep
  sleep (reset and chip-select are held high).

## Notes

- **SR-15 has two board revisions with two panel controllers.** Boards with a green round sticker
  use an UltraChip UC81xx, not the SSD1683 in Elecrow's main examples, and need GPIO41 high.
  `epd_crowpanel42.c` detects which by the BUSY level after reset; `epd-ctl!` forces one.
- Console words are listed at the top of `pass/src/words.c`.
- `../tools/fake_pass.py` is a pretend SR-16 on the Mac's Bluetooth for phone-app work with no
  hardware. It proved the app's remote and pack transfer; it says nothing about this firmware.
- `backup/` (git-ignored) holds the factory image read off the bench CrowPanel; Elecrow publishes
  it under no licence, so it stays local. Restore with `esptool.py write_flash 0 <file>`.
