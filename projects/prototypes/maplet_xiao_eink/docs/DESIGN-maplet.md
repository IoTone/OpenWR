# maplet — session maps on e-paper (design)

**Status:** Postcard works end to end, 2026-10-02: Switchback pairs the
maplet as a second device, reads its safe area, renders the session's map
from AWS Terrain Tiles on the phone, and pushes it over BLE (36 lines, about
17 s); the maplet shows it and keeps it in slot 0. The bench panel has dead
gate zones at both ends, so `epd-safe!` limits drawing to the 156 columns
that render; a fresh panel gets `0 212 epd-safe!`.
**Hardware (primary):** Seeed XIAO ESP32S3 Sense (OV2640 camera, microSD,
8 MB PSRAM, 8 MB flash) + XIAO ePaper Breakout Board (SKU 105990172) + 2.13"
flexible monochrome e-paper, 212×104 (SKU 104990847, UC8151-class controller,
the panel GxEPD2 calls `213_flex`). The XIAO ESP32C3 is kept as a second
PlatformIO env (`xiao_c3`) for the camera-less builds.
**Lineage:** the ESP_Skin prototypes in this repo (raw driver, GxEPD2
driver); the goggl1 firmware for everything that is not the display.

## 1. What it is

A patch of e-paper you wear or clip on. Before a session it shows the map
of where you are going; after, the map of where you went, with the track,
the contours, and the numbers. It never updates live: e-paper takes two
seconds and a flash to refresh, and the goggle is the live display. It
holds its image with no power, so the last session is on your sleeve until
the next one.

Everything hard happens on the phone. maplet is a frame: it receives a
finished 212×104 bitmap over the same Forth console goggl1 has, shows it,
and keeps the last few in flash.

## 2. Where the map comes from

Free and open, verified live for Portland on 2026-10-02:

| Source | Role | Terms |
|---|---|---|
| **AWS Terrain Tiles** (Terrarium PNG, 256 px, zoom to 15; 3DEP in the US, SRTM/Copernicus elsewhere) | elevation grid: contours, hillshade, slope, profile | free, no key, attribution line |
| **USGS 3DEP** via EPQS | spot elevations when a single number is wanted | public domain |
| USGS Topo basemap tiles | optional backdrop on the phone | public domain |

Terrain Tiles are the base. A session's bounding box at zoom 14 is a few
95 KB tiles, decoded on the phone into metres per pixel; contours come from
marching squares at an interval chosen for the terrain (10 m in the city,
50 m in the mountains), hillshade from the gradient. Tiles are cached, so a
region you ride often is offline after the first time.

Nothing in this is road data. OSM trails can be layered later if wanted.


## 3. Three plans for the experience

Real renders at the panel's resolution, from twelve zoom-14 AWS Terrain
Tiles over Forest Park, Portland, drawn with the firmware's font5x7. The
raster work (hillshade, dither, marching-squares contours, track) is in
`tools/render/maplet_render.p` (Pop-11); `tools/render/compose.py` adds
text and the photo. Full-size PNGs in `docs/images/`.

### Plan 1 · Postcard (after the session) — build this first

![Postcard](images/plan1_postcard_x3.png)

At **Stop**, the phone renders the loop you just did: hillshade, contours,
the track as a 3 px line with a start ring and end dot, an inverted banner
with the place, and a stats line (time, distance, climb). One flash, then
it stays until the next session. C3 build, no new firmware words. Any past
session in the log can be sent again.

### Plan 2 · Field Notes (before the session)

![Field Notes](images/plan2_fieldnotes_x3.png)

A notebook page: a notes column on the left (plan name, distance, climb,
estimated time from your average pace for that sport, sunset) and the
route on the right as a dashed line with numbered waypoints over contours.
From **Plan** / **Map where I am** on the Session tab. Same build as
Postcard. Optionally flips into a Postcard at Stop.

### Plan 3 · Snapshot (during the session, S3 Sense)

![Snapshot](images/plan3_snapshot_x3.png)

The S3 Sense takes its own pictures: on `cam-snap` (phone tap or device
button) it grabs a frame, dithers it to one bit (ordered dither reads
better than error diffusion for photos on this panel), drops in the 76×64
minimap inset the phone sent at Start, stamps time and distance from the
`sess!` stream, shows it, and writes it to the SD card as the session's
roll. At Stop the phone pulls the roll over BLE for the session gallery.

| | Postcard | Field Notes | Snapshot |
|---|---|---|---|
| Moment | after | before | during |
| Board | C3 | C3 | S3 Sense |
| Renders | phone | phone | phone inset, device photo |
| BLE bytes | 2 808 in | 2 808 in | 608 in; 2 808 out per frame |
| Flashes | 1 | 1 (2 with flip) | per snapshot |
| SD | no | no | yes |
| New words | none | none | `cam-snap`, `epd-photo`, `roll-ls`, `roll-get` |

**Recommendation:** Postcard first. It builds the renderer, the tile cache
and the second-device console, which the other two reuse.

## 4. How the picture is made

Parameters that read well at one bit on this panel, from the mockups:

1. Tiles: Terrarium PNG, zoom 14 in the city (13.6 m/px on the canvas),
   zoom 13 in the mountains. `elev = (R*256 + G + B/256) - 32768`.
2. Grid: the track's box padded 10 %, widened to 212:104, bilinear sample.
3. Hillshade: Lambert from the gradient, relief ×2, light from the
   west-southwest (the Tualatin ridges run NW–SE, so NW light is flat).
   Only slopes darker than flat ground take ink.
4. Dither: Floyd–Steinberg on the shade, gain 1.4, capped at 75 % ink.
5. Contours: marching squares at 25 m, every fourth (100 m) drawn 3 px.
   Interval scales with relief: 10 m under 100 m of range, 50 m over 600 m.
6. Track: Bresenham 3 px over a 7 px white halo; ring at start, dot at
   end; a planned route is dashed.
7. Furniture: 500 m scale bar bottom-left, north arrow top-right, 5×7 text
   at 2× for the banner, 1× for stats.
8. Pack: 27 bytes/row, MSB first, 1 = black, 2 808 bytes, 36 hex lines of
   80 bytes via `epd-hex!`; the driver rotates to native 104×212 on
   `epd-show`.

## 5. Hardware: XIAO ESP32S3 Sense

Env `xiao_s3_sense` (default). Breakout → S3 GPIO: RST 1, CS 2, DC 4,
BUSY 6, SCK 7, MOSI 9; BOOT button GPIO0; USB on 19/20 (never touched).
The Sense's microSD shares the SPI bus with CS on GPIO21 (also the user
LED, so no LED); the OV2640 has its own pins. 8 MB flash: two 3 MB OTA
slots plus a 1.9 MB `maps` partition (`partitions_s3.csv`); octal PSRAM on.
.mnpkg chip 0x0004, board 0x0408 `xiao-esp32s3` (proposed). Flashing on
macOS needs 115200 baud (`upload_speed` in platformio.ini); at 460800 the
USB-serial-JTAG drops the data phase with "No serial data received".
Snapshot runs on the bench with an OV5640 module and a 2 GB card: `snap`
takes about 4 s from the phone's tap to the refresh, and the roll holds the
2 808-byte frames plus 320x240 PGM originals. Sample frame:

![Snapshot from the bench](images/snapshot_bench.png)

## 6. The device contract

The console goggl1 already has, plus a small e-paper vocabulary:

```
epd-clear      ( -- )              white canvas
epd-cursor!    ( n -- )            byte cursor into the canvas, 0 = top-left
epd-hex!       ( c-addr u -- )     append bytes given as hex text at the cursor
epd-str        ( c-addr u x y -- ) 5x7 text, for the device's own messages
epd-line / epd-box / epd-pixel     as on the goggle
epd-show       ( -- )              full refresh (about 2 s), then panel sleep
epd-save       ( n -- )            keep the canvas as map n in flash (0..3)
epd-load       ( n -- flag )       bring map n back to the canvas
epd-slot@      ( -- n )            last slot saved
epd-splash     ( -- )              the boot picture
cam-snap       ( -- flag )         S3: grab a frame into PSRAM          (planned)
epd-photo      ( x y w h -- )      S3: dither the frame into the canvas (planned)
roll-ls        ( -- )              S3: list the session roll on SD      (planned)
roll-get       ( n -- )            S3: print bitmap n as hex lines      (planned)
```

The canvas is landscape, 212 wide by 104 tall, 27 bytes a row, MSB first,
1 = black (the natural way to draw). The driver rotates to the panel's
native 104×212 on `epd-show`. A full picture is 2 808 bytes; as hex over
BLE in 80-byte lines that is 36 lines and about three seconds. At boot the
device shows the last saved map, or its splash if there is none.

## 7. Phone side

Done in Switchback (`lib/map/`, `lib/goggle/maplet.dart`,
`lib/screens/session_detail_screen.dart`): sessions keep the GPS track
(one point per 10 m) and climb; a session's detail screen shows the real
1-bit render and **Send to maplet**; Stop sends the Postcard by itself when a
maplet is connected; Trailhead has a maplet card; `/pair-maplet` pairs it;
**Add a demo session** (Forest Park loop) on the Session tab exercises the
chain without a run.

- Sessions record the GPS track (they already compute distance from it).
- A session's detail screen renders the map and offers **Send to maplet**.
- Pairing: maplet advertises as `maplet` with the same window rules as the
  goggle (power-up, and after a drop). Switchback keeps one console per
  device; the Pair screen lists both kinds.
- Pre-session: **Map where I am** on the Session tab.

## 8. What it is not, yet

Live updates (the goggle's job), partial refresh (this panel's LUT does a
full flash; fine for a patch), a user button (the XIAO has none; the app
flips maps), and trails from OSM.

## 9. Panels: one driver table, chosen from the console

The firmware drives more than one e-paper panel. `src/epd.c` holds a table
of drivers; each row names a panel and carries its canvas size, its native
frame (sources x gates) and four steps: init, write, refresh, sleep. The
active row is chosen at run time and remembered in NVS, so the same image
runs on every panel and a swap is a console line, not a reflash.

| Driver | Panel | Canvas | Controller notes |
|---|---|---|---|
| `uc8151-213flex` | 2.13in flexible mono, Seeed 104990847 | 212 x 104 | BUSY low = busy; LUTs from register (GxEPD2_213_flex); bench knobs epd-geom!, epd-cdi!, epd-old!, epd-lut! |
| `ssd1680-290` | 2.9in mono, Seeed 104990853 | 296 x 128 | BUSY high = busy; OTP waveform, update 0xF7; RAM 0x24/0x26, 1 = white (Seeed_GFX Setup504, GxEPD2_290_T94) |

Words:

```
epd-drivers                       list them, * on the active one
s" ssd1680-290" epd-driver!       switch, remember, reset the safe area
epd-driver                        print the active one
epd-size@      ( -- w h )         canvas size
epd-rot!       ( n -- )           0..3 if the picture lands mirrored or upside down
epd-test epd-show                 the calibration pattern, always the first look
```

The canvas, the drawing words, Snapshot, the PAIR badge and the phone all
follow the active size. The status line carries `epd=driver:WxH`, and
Switchback renders for it. Saved maps record the size they were drawn for;
a map from another panel is not shown, the splash is. On the S3 the four
slots live in the `maps` partition (8 KB each) because a 296 x 128 frame
(4 736 bytes) times four does not fit the 24 KB NVS.

Bringing up a new panel:

1. Find the controller (Seeed_GFX `User_Setups/Setup5xx_*.h` names it) and
   its init, refresh and sleep (`TFT_Drivers/<IC>_Defines.h`, or GxEPD2).
2. Check which BUSY level means busy and whether RAM data is 1 = white.
3. Add the four functions and a row; raise `EPD_W_MAX` / `EPD_H_MAX` if it
   is the biggest panel yet.
4. Flash, `s" name" epd-driver!`, `epd-test epd-show`: border on the glass
   edge, 1 px checker clean, "0,0 TOP-L" top left. Fix orientation with
   `epd-rot!`.
5. Send a Postcard from Switchback.

First light, 2026-10-13: `ssd1680-290` on the 2.9in panel shows the test
pattern edge to edge (no RAM offset, all 296 x 128 live), 1 px checker
clean, refresh 2.1 s, text upright with the flex cable to the right
(rotation 0). The choice persists across reboot.

**Bookmarked, low priority: portrait orientation.** The 2.9in unit will
be used landscape (decided 2026-10-13), which is what the firmware and
Switchback do today; switching to portrait is unlikely. Kept here in case
a later panel or mount needs it. `epd-rot!` already turns or mirrors the
landscape canvas. Proposed scheme, not built:

- `epd-orient!` ( n -- ): 0 landscape, 1 portrait, 2 landscape turned 180,
  3 portrait turned 180. Portrait swaps the canvas to 128 x 296; the
  driver keeps its native frame and `to_native()` picks the mapping.
  `epd-rot!` stays for mirror fixes on odd glass.
- Stored per panel in NVS; the status line reports the canvas as it
  stands (`epd=ssd1680-290:128x296`), so Switchback already gets the
  right size.
- Saved maps carry their size, so a landscape map is not shown in
  portrait and vice versa (already true today).
- Switchback needs portrait layouts: Postcard with the banner on top and a
  tall map, Field Notes with the notes block under the map, Snapshot with
  the inset at the bottom. The renderer takes w x h already; the layouts
  are the work.
- Optional later: an orientation toggle on the maplet page in the app.

Pins: the breakout board (105990172) puts BUSY on D5. Seeed's ePaper driver
board puts it on D2, so a panel on that board needs its own pin header.
