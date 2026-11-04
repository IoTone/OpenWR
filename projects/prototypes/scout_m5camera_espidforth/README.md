# scout_m5camera_espidforth

Scout: the second camera for [LightScopeXR](../../apps/LightScopeXR/), on an
M5Camera (PSRAM) Model B (ESP32-D0WDQ6 rev 1, 4 MB flash, 4 MB PSRAM,
OV2640, CP2104). It speaks the lens's line contract directly over BLE
(Nordic UART, advertising as `Scout`), so the lens can be built and tested
against a real camera without the Sony.

Every picture is the maplet's look: cropped to the frame, contrast
stretched, dithered to 1 bit (1 = ink, packed MSB first). Frames and
thumbnails go to a FAT roll on the board's flash (about 2.1 MB: a 212×104
frame plus its thumbnail is about 3.4 KB, so roughly 600 shots). The board
has no SD slot.

On the LAN (v0.5.0) it also serves its colour JPEGs: it joins your Wi-Fi,
runs a small web server, and tells the glasses its address over BLE
(`url?`), since Spectacles have no mDNS. That is the test bed for pulling
frames over Wi-Fi while BLE carries control.

## Build and flash

```
pio run -t upload -t monitor     # /dev/cu.usbserial-* (CP2104)
```

The first boot after a power-on prints `flash read err, 1000`, then the
chip's watchdog resets it and it boots normally. That's this board at
cold start, not the firmware.

## Words (USB console or BLE)

```
caps?          \ caps scout shoot lv last roll sty fr web
cam?           \ cam ready OV2640 0
shoot          \ ok <n>: frame + 80x53 thumbnail into the roll
last           \ last 212 104, then base64 lines
lv             \ lv 80 53, then base64 lines (Bayer, steady between frames)
1 sty!         \ 0 grain (Floyd-Steinberg), 1 screen (Bayer)
1 fr!          \ 0 212x104 (maplet), 1 120x80 (3:2), 2 96x96
sty?  fr?
3 flip!        \ bit 0 vflip, bit 1 mirror; saved
roll?          \ roll <count> fs <free KB>
roll-ls        \ 0000.BIN ... roll: <count> frames   (as the maplet)
0 roll-b64     \ frame 0 as base64 lines             (as the maplet)
5 4 roll-th    \ rth 5 80 53 1 + base64, then 4, 3, 2
roll-format    \ wipe the roll
version        \ scout v0.5.0 (the lens's RollView reads roll-b64 from 0.3.2)

s" ssid" s" pass" wifi!   \ join a 2.4 GHz network; saved
0 wifi-ps!     \ Wi-Fi always awake; 1 = modem sleep (default, needed with BLE)
net?           \ net up 10.0.0.216 rssi -60 | net down
url?           \ url http://10.0.0.216/ ws://10.0.0.216/ws | url none
web?           \ web jpg <n> ws <n> mjpeg <n> last <client ip> <ms ago>
mem?           \ internal / largest block / PSRAM free
```

HTTP on port 80 (CORS open, no caching):

| Path | What |
|---|---|
| `/frame.jpg` | one fresh 320×240 JPEG |
| `/stream` | multipart MJPEG |
| `/ws` | WebSocket: any message in, one binary JPEG out |
| `/info` | `{"w":320,"h":240,"jpg":n,"ws":n,"mjpeg":n}` |
| `/` | a test page that loops `/frame.jpg` and shows its FPS |

From a Mac on the same network: MJPEG about 19 fps, `/ws` with one frame in
flight 7.7 fps, `/frame.jpg` about 5.5 fps.

`roll-ls`, `roll-b64` and `version` answer exactly as the maplet does, so
the lens's existing `RollView.ts` can read Scout's roll; 212×104 frames
are 2808 bytes, a size `Bitmap.ts` already knows.

## Notes

- Pins: esp32-camera's `CAMERA_MODEL_M5STACK_V2_PSRAM` (the factory
  firmware's boot log shows the same). XCLK 10 MHz, two frame buffers in
  PSRAM, JPEG QVGA. Shots decode a JPEG to grey before the dither, so the
  web frames and the 1-bit frames come from the same sensor mode.
- The sensor is mounted upside down: `3 flip!` (now saved) turns it upright.
- Internal RAM is tight with BLE, Wi-Fi and the camera together. Wi-Fi TX
  buffers must be static when PSRAM malloc is on, so `sdkconfig.defaults`
  cuts them to 8 (RX 6, BA window 6) and moves NimBLE to PSRAM; that took
  free internal RAM from ~20 KB to ~74 KB and let the web server start.
- esp32-camera's interrupt handler prints a warning whose text lives in
  flash; during a flash write that hangs the chip. The warning is compiled
  out by building at log level ERROR (`sdkconfig.defaults`); `main.c` sets
  the runtime level back to INFO.
- The roll mounts before the camera starts, so a first-boot format never
  runs under the camera's interrupts.

## Layout

| Path | What |
|---|---|
| `src/scout_cam.c` | camera, render + dither (from the maplet), roll, words |
| `src/scout_net.c` | Wi-Fi station, the web server, `url?` `web?` |
| `src/main.c` | UART0 + BLE console, housekeeping |
| `src/ble_console.c` | the NUS console, as in AlphaFriend and the maplet |
| `components/forth/` | ESPIDFORTH |
