# esp32c3_beetle_goggl1

A goggle heads-up display prototype: a transparent OLED in front of one eye,
driven by a coin-sized ESP32-C3, programmable from a Forth prompt over USB.

**Board:** DFRobot Beetle ESP32-C3 ([DFR0868](https://wiki.dfrobot.com/dfr0868/))
**Display:** DFRobot Fermion SSD1309 1.51" transparent OLED ([DFR0934](https://wiki.dfrobot.com/dfr0934/)), 128×64, connected by the 18-pin GDI ribbon
**Firmware:** [ESPIDFORTH](https://github.com/IoTone/ESPIDFORTH) v0.5.0 on ESP-IDF 5.3.1, with an SSD1309 vocabulary added

This is a port of DFRobot's U8g2 Arduino demos for the DFR0934 to ESPIDFORTH.
The C half is a bare-metal SSD1309 driver and 37 Forth words; the demos
themselves are Forth files you paste at the `ok>` prompt and edit while the
display is on your face.

## Why Forth for a HUD

A heads-up display is all layout decisions: where the clock goes, how big
the next-turn arrow is, whether the battery icon is in the way. Every one of
those is a number you only discover wearing the thing. With the OLED on an
Arduino sketch each tweak is an edit-build-flash cycle with the goggles off.
Here it is a line of typing with them on.

## Wiring

The OLED's FPC ribbon goes straight into the Beetle's GDI connector. Metal
contacts face the white part of the connector. Reversed is harmless, the
screen just stays dark, so flip the ribbon if nothing comes up.

The GDI connector exposes more than the OLED uses. Only these five lines
matter:

| Signal | Beetle GPIO |
|--------|-------------|
| SCLK   | 4  |
| MOSI   | 6  |
| DC     | 1  |
| RST    | 2  |
| CS     | 7  |

Also used: GPIO10 (onboard LED, the `led` word) and GPIO9 (BOOT button,
the `button?` word). Pin sources and caveats are in `src/pins_beetle_esp32c3.h`.

## Build and flash

Requires PlatformIO. The toolchain pins match ESPIDFORTH's, so if that repo
builds for you, this does too.

```bash
cd projects/prototypes/esp32c3_beetle_goggl1
pio run -t upload -t monitor
```

The Beetle enumerates as a USB CDC device (`/dev/cu.usbmodem*` on macOS).
If upload cannot find the port, hold BOOT, tap RST, release BOOT, and retry.

On boot the display shows the layout test pattern (see below) and the
console prints a banner followed by the `ok>` prompt. Try:

```
ok> oled-clear s" hello" 0 0 oled-str oled-show
ok> 2 oled-scale oled-clear s" 12:34" 20 20 oled-str oled-show
ok> 1 led  500 ms  0 led
```

## The power-on test pattern

The boot screen is a layout pattern, not a logo, because the first thing
you need from a new panel is to know what it can actually show. One photo
of the glass answers:

- **Which edges are visible.** A 1 px frame runs round the full 128x64
  controller area. A missing side is a hidden side.
- **Where the 56-row transparent window ends.** A dotted line on row 55,
  labelled `y56`. On this unit everything below it still shows, but over
  the opaque backing strip rather than the glass.
- **Which way x and y run.** Column ticks every 8 px along the top edge,
  longer every 32, labelled 32/64/96 with `0` top-left and `127` top-right.
  Row ticks down the left edge labelled 16/32/48, `63` bottom-right.
- **Where the centre is and whether pixels are square.** Crosshair at
  (64,32) with an r=10 circle.
- **Whether single pixels resolve.** An 8x8 checkerboard and a 2 px dot
  grid in small frames below the crosshair, plus a diagonal on the right.

`oled-test` redraws it at any time; `oled-splash` draws the logo instead.
To change the boot screen, swap the call in `main.c`.

## Files

| File | What it is |
|------|------------|
| `src/ssd1309.c/.h` | SSD1309 over ESP-IDF `spi_master`, 1 KB framebuffer, lines/boxes/circles/triangles/XBM, 5×7 font with scale and 4 directions. No Arduino, no U8g2. |
| `src/goggle_words.c/.h` | The Forth vocabulary: 37 words wrapping the driver, LED, button, delay, and integer sin/cos. |
| `src/pins_beetle_esp32c3.h` | Pin map with sources. |
| `src/main.c` | ESPIDFORTH's `main.c` plus display bring-up and the splash. |
| `components/forth/` | ESPIDFORTH's engine, vendored verbatim. See its `PROVENANCE.md`. |
| `forth/hello.fs` | Port of `HelloWorld.ino`. |
| `forth/graphics.fs` | Port of `GraphicsTest.ino`: 10 animated scenes. |
| `forth/cube.fs` | Port of `Cube.ino`: a rotating tetrahedron in integer maths. |
| `forth/fonts.fs` | Ports of `Font.ino` and `FontDirection.ino`. |

## Running the demos

Each `.fs` file is plain text. Send it with your terminal's "send file"
feature (or paste it line by line; lines are under the engine's 255-char
limit). The last line of each file runs the demo once. Afterwards the words
stay defined, so:

```
ok> hello
ok> graphics-test        \ one pass, ~12 s
ok> graphics-loop        \ repeat until BOOT is pressed
ok> 360 spin             \ two turns of the tetrahedron
ok> cube                 \ spin until BOOT is pressed
ok> font-demo  direction-demo
```

With `pio device monitor` the file-send feature is not available; `screen`
and `minicom` have one, or `cat forth/cube.fs > /dev/cu.usbmodem101` works
if the monitor is not also holding the port. The engine echoes each line
as it interprets it.

## The vocabulary

```
oled-clear      ( -- )                   clear the framebuffer
oled-show       ( -- )                   push it to the glass
oled-color      ( n -- )                 0 clear  1 set  2 xor
oled-pixel      ( x y -- )
oled-pixel@     ( x y -- 0|1 )
oled-line       ( x0 y0 x1 y1 -- )
oled-hline      ( x y w -- )
oled-vline      ( x y h -- )
oled-frame      ( x y w h -- )           hollow rectangle
oled-box        ( x y w h -- )           filled rectangle
oled-rframe     ( x y w h r -- )         rounded, hollow
oled-rbox       ( x y w h r -- )         rounded, filled
oled-circle     ( x y r -- )
oled-disc       ( x y r -- )
oled-triangle   ( x0 y0 x1 y1 x2 y2 -- ) filled
oled-xbm        ( addr w h x y -- )      XBM bitmap, 1-bits only
oled-xbm-solid  ( addr w h x y -- )      XBM bitmap, 0-bits clear too
oled-str        ( c-addr u x y -- )      5x7 text, top-left anchored
oled-glyph      ( c x y -- )
oled-scale      ( n -- )                 1..4, both axes
oled-scale-xy   ( sx sy -- )             independent x / y scale
oled-dir        ( n -- )                 0 right  1 down  2 left  3 up
oled-strw       ( u -- w )               pixel width of u chars
oled-on         ( flag -- )
oled-contrast   ( n -- )                 0..255
oled-invert     ( flag -- )
oled-flip       ( flag -- )              rotate 180 degrees
oled-test       ( -- )                   draw the power-on layout pattern
oled-splash     ( -- )                   draw the goggl1 logo
oled-width      ( -- 128 )
oled-height     ( -- 64 )
led             ( flag -- )
button?         ( -- flag )
ms              ( n -- )
us              ( -- t )
sin1k           ( deg -- n )             sin * 1000
cos1k           ( deg -- n )             cos * 1000
```

Nothing reaches the panel until `oled-show`. Draw a whole frame, then show
it: that is U8g2's `clearBuffer` / `sendBuffer` model, and it is also why
animation does not flicker.

## What changed in the port

**No U8g2, no fonts.** The Arduino demos lean on U8g2's font library
(`t0_17b`, `bubble`, `open_iconic`, Chinese and Japanese Unifont). Shipping
a font engine is out of scope for a 4 MB part with no PSRAM, so there is one
5×7 ASCII font with integer scaling. The `Font`, `Language` and `CommonIcon`
demos are therefore reduced to their layouts; `graphics.fs` skips the two
scenes that need Latin-1 and Unicode glyphs.

**Integer maths.** `Cube.ino` rotates doubles by 0.1° per frame and lets
rounding accumulate. The port keeps the base vertices fixed, accumulates an
integer angle, and rotates with `sin1k`/`cos1k`, so it never drifts. The
step is 2° at 30 ms, which is a turn every 6 s.

**The NONAME2 column offset.** DFRobot's sketches all use the
`U8G2_SSD1309_128X64_NONAME2` constructor, which differs from NONAME0 by a
2-column offset in the display RAM. The driver sends that offset on every
page write. Confirmed on hardware 2026-10-01: column 0 lands flush on the
left edge of the glass with no wrapped stripe.

**Text width.** The 5x7 font is 6 px per character per unit of scale, so
at scale 2 only 10 characters fit across. U8g2's t0_17b is tall and narrow;
use `1 2 oled-scale-xy` to get the same shape (72 px for "Hello World!").

**The init sequence** is U8g2's `u8x8_d_ssd1309_128x64_noname_init_seq`
command for command, including the 100 ms reset pulse U8g2 comments as
needed by "far east OLEDs".

## Known unknowns

- Resolved 2026-10-01 from a photo of the power-on pattern: all 64 rows and
  all 128 columns draw. The frame is intact on every side, `0`/`127`/`63`
  are all legible, pixels are square and single pixels resolve. The bottom
  8 rows (56 to 63) sit over the panel's opaque backing strip rather than
  the see-through glass, so they are not transparent but they are visible.
  Treat rows 0 to 55 as the HUD area and rows 56 to 63 as a status strip.
- SPI runs at 4 MHz because that is what U8g2 uses. The SSD1309 datasheet
  allows 10 MHz. Raise `clock_hz` in `goggle_words.c` once the ribbon is
  known to be clean.

## Weather HUD

The first use case for this hardware: the next eight hours of weather,
located by IP, in three themes. Design in
[`docs/DESIGN-weather-hud.md`](docs/DESIGN-weather-hud.md); all four phases
are implemented and verified on hardware. Provisioning and OTA are in
[`docs/DESIGN-provisioning-ota.md`](docs/DESIGN-provisioning-ota.md).

Firmware side (`src/net_words.c`, `src/wx_words.c`, `src/wx_sched.c`): WiFi
station with credentials in NVS, SNTP, geolocation (ipinfo.io, then
ip-api.com), Open-Meteo forecast parsed to integers and cached in NVS, a
scheduler task that refreshes every 15 min and calls `hud-tick`, BOOT button
cycling the theme. Flash is laid out with two OTA slots from the start.

Use-case side (`forth/weather-hud/`): six Forth files that together are
one **bundle**. Sent once, the bundle is applied all-or-nothing, stored in
NVS, and re-applied at every boot, so the HUD survives a power cycle with
no USB attached.

```bash
# first time: give the device your network, at the ok> prompt
ok> s" MySSID" s" MyPassword" wifi!

# install the use case (once; it persists)
python3 tools/send_forth.py --bundle /dev/cu.usbmodem101 forth/weather-hud/*.fs

# or, while developing a theme, send files plainly and iterate at the prompt
python3 tools/send_forth.py /dev/cu.usbmodem101 forth/weather-hud/11-glance.fs

ok> wx-test        \ load the design fixture when there is no network
ok> 1 theme!       \ 0 strip, 1 glance, 2 ribbon; BOOT button cycles too
ok> hud-now        \ redraw immediately
ok> wx-fetch .     \ force a forecast fetch, -1 on success
```

Words added by the HUD firmware, all documented in the headers:
`net-ok? wifi! net-ip clock-ok? clock-hm clock-s`,
`geo-ok? geo-lat geo-lon geo-city geo! geo-fetch`,
`wx-ok? wx-age wx-hour0 wx-temp wx-feels wx-pop wx-precip wx-snow wx-wind
wx-gust wx-wdir wx-code wx-cloud wx-uv wx-day? wx-fetch wx-test units! units@
wx-icon wx-icon2 wx-arrow`, `theme! theme@ hud-now`,
`bundle-begin bundle-apply bundle-forget bundle-info`, `pair unpair ble-on?`,
`ota! ota-keys version status`, `geo-auto geo-manual? wx-dump`, `reboot random`,
and `oled-num oled-num02 oled-numr` for drawing integers.

`tools/hud_mock.py` renders the three themes on the host with the device's
own driver; its output is in `docs/images/`.

### From a phone, or anything with Bluetooth

The same `ok>` prompt is available over BLE as a Nordic UART Service, so
any BLE terminal app works today and the OpenWR app only has to hide the
words behind buttons. The radio is off until you open a window:

- **Long-press BOOT** (1 s or more), or type `pair` on USB. The device
  advertises as `goggl1` for 3 minutes and the status strip shows a `B`.
- Connect and type what you would type on USB. `status` prints one line
  the app can parse; `s" ssid" s" pw" wifi!` provisions; `bundle-begin`
  … `bundle-end` installs a use case.
- **Firmware update**: releases are signed `.mnpkg` packages (the MagNET
  OTA package format; see `docs/DESIGN-provisioning-ota.md` §4). Build one
  with `tools/mnpkg/mnpkg.py`, serve it on the local network, and send
  `s" http://<host>/goggl1-x.y.z.mnpkg" ota!`. The goggle checks format,
  key, signature, target, size, hash, image and version before it boots it,
  writes the other OTA slot, reboots, and rolls back if the new image does
  not confirm itself. Switchback does all of this from the phone.

`tools/ble_console.py` (needs `pip install bleak`) is the stand-in app:

```bash
python3 tools/ble_console.py status "2 theme!"
python3 tools/ble_console.py --bundle forth/weather-hud/*.fs
python3 tools/ble_console.py 's" http://10.0.0.116:8000/firmware.bin" ota!'
```

## Next

- The OpenWR mobile app, Switchback: proposal in
  [`docs/DESIGN-app-switchback.md`](docs/DESIGN-app-switchback.md), mockups at
  https://claude.ai/artifact/XiEBJqZLtTHHxt1zpvuQmN. Starts from
  ProjectMagNET/magnet_app.
- Passkey pairing with the code shown on the glass (phase 5).
- Quieter console: WiFi and TLS logs currently interleave with the USB REPL.
- Pulling this vocabulary back into ESPIDFORTH as `examples/beetle-oled/`.
