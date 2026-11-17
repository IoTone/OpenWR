# reelhub_wavec6touch_espidforth

ReelHub: the shot hub for [LightScopeXR](../../apps/LightScopeXR/), on a
Waveshare ESP32-C6-Touch-LCD-1.47 (ESP32-C6FH8 rev v0.2, 8 MB flash, no
PSRAM, JD9853 172×320 LCD, AXS5106L touch). It shows the latest colour
shot, swipes through the reel, and fires every camera at once.

It coordinates, rather than AlphaFriend: AlphaFriend has ~49 KB of heap and
sits on the α7's own Wi-Fi (so it can't reach Scout or the CameraX), and the
setup has to work with the α7 off. ReelHub has the RAM, a screen, flash for
the cache, and is on the LAN and BLE at once.

## Use

- **Swipe** left for the newer shot, right for the older one.
- **Hold** a finger still on the screen for **3 s** to shoot; a bar at the
  bottom right fills while you hold.
- **Double-tap** the picture for a round shutter button; tap it to shoot.
  It hides itself after 6 s.
- Shooting flashes the screen white (the board has no speaker and, unlike
  the non-touch board, no RGB LED), prints `fire <n>` on every console so the
  glasses take theirs, then pulls each camera's picture into the reel.

The screen is landscape, 320×172: the picture fitted into the left 258×172,
a strip on the right with its number, camera, time and size.

## The reel

`/reel/0001.JPG` + `0001.TXT` on a 5.9 MB FAT partition in the C6's own
flash (no SD card needed): a few hundred medium JPEGs. The oldest go when it
is nearly full. Pictures are kept as sent (Scout's 320×240 is already
small); they're scaled to fit as they're drawn (ROM TJpgDec, decoded into
16-row strips and resampled, so any size costs two small buffers).

## Cameras (0.2.0)

LAN cameras (Scout firmware 0.8.0+: Scout, CameraX) are found by
broadcasting `who` every 20 s; there are no addresses to type. A shot sends
one `snap <id>` to all of them at once (broadcast, and straight to each
known camera, resent every 300 ms until each answers, up to 2 s), so the
frames are taken together; then the hub fetches each one's `/snap.jpg`. See
Scout's README for the messages.

| camera | how | state |
|---|---|---|
| Scout | UDP `snap` + HTTP `/snap.jpg` | working |
| CameraX | the same (Scout firmware, `-e camerax`) | to flash |
| glasses | `fire <n>` line on the BLE console | lens side to do |
| AlphaFriend (α7) | BLE central link, `shoot` + a medium `pvj` | when the α7 is back |

## Words (USB console or BLE "ReelHub")

```
shoot                 \ fire all
cams?                 \ cam <name> <url> seen <s> ago ... cams <n>
cams-find             \ ask now
reel?                 \ reel <count> free <KB> showing <n>
reel-ls               \ <n> <camera> <w>x<h> <time> <bytes>
3 reel-show           \ show shot 3
reel-wipe             \ delete every shot
touch-cal             \ tap the three crosses; saves the touch map
n touch-map!  touch-log
n rot!  card  redraw  \ screen orientation (0 is right on the bench), test card
s" ssid" s" pass" wifi!  wifi?  wifi-seen  n wifi-forget  n wifi-first  net?
s" PST8PDT,M3.2.0,M11.1.0" tz!     \ time zone for the shots' times (saved)
mem?  version
```

Wi-Fi is Scout 0.7.0's: up to five saved networks in order, falling back
down the list. Passwords stay in NVS and are never printed.

## Build and flash

```
pio run -t upload --upload-port /dev/cu.usbmodem*     # USB-C, USB-serial-JTAG
pio device monitor                                     # close it before flashing
```

PlatformIO's ESP-IDF 5.3.1 (the C6FH8 needs ≥ 5.1.4).

## Measured (2026-11-17)

- Free internal heap with Wi-Fi + BLE up: 155 KB (largest block 126 KB).
- Scout shot into the reel: 6–13 KB in 280–810 ms (0.1.0, /frame.jpg);
  0.2.0: snap answered in 160–380 ms, frame fetched in 0.65–2.8 s.
- A shot in the first seconds after the hub boots can find no camera yet.
- Touch map 1 (swap x/y) from `touch-cal`; screen rot 0.

## Sources

- Display bring-up and the JD9853 driver (Espressif, Apache-2.0) from
  ProjectMagNET's MagNET_OTA_WaveC6LED.
- AXS5106L report format as Zephyr's `input_axs5106l` driver.
- Forth: ESPIDFORTH 0.5.0, vendored as in Scout (`components/forth`).
- BLE console: Scout's `ble_console.c`.
