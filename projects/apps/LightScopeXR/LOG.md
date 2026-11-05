# LightScopeXR project log

The journey, newest last. The design itself is in [DESIGN.md](DESIGN.md);
this file is what we tried, what broke, and what we learned along the way.
Covers the lens and AlphaFriend
(`../../prototypes/alpha-fairy-espidforth/`).

## 2026-11-03 — design and step 1

- Copied the lens from SpecsSwitchbackXR. Wrote DESIGN.md and the design
  page (`docs/lightscopexr.html`, private artifact): N cameras, photo
  streaming, the Specs camera, a reel per camera.
- Looked for an ESP-IDF port of Alpha Fairy; none exists. Chose to vendor
  Alpha Fairy (not fork it on GitHub) at `fd1f298`, about 820 KB of its
  sources and docs; the images, PDFs and bundled Arduino libraries stay out.
- Step 1: ported `SonyHttpCamera` to `src/sony_http.c`. Its Arduino state
  machine became blocking calls, since each Forth word waits for its own
  reply. Kept Alpha Fairy's key scanner instead of cJSON (no PSRAM).
  Host test of the scanner against sample getEvent, error and
  actTakePicture replies passed. Build: RAM 30 %, flash 51 % of a slot.

## 2026-11-04 — L0 on the α7R III

- **Wrong board first.** The only USB device was `/dev/cu.usbmodem11201`;
  `esptool chip_id` said ESP32-C6FH4, the NanoC6. Nothing flashed. The
  StickC wasn't enumerating at all until it was replugged; then it showed
  as `/dev/cu.usbserial-BD52F760EE` (chip_id: ESP32-PICO-D4) and took the
  firmware first time.
- **Password typo.** `g` for `q`. The camera hides the text password
  behind its QR code screen; read it carefully.
- **Camera AP vanishing (reason 201).** Between attempts the camera's
  network kept disappearing. It only broadcasts while the Connection screen
  is up, and the camera powers down fast. Set Power Saving Start Time
  longer before testing.
- **Dropped 0.4 s after joining (reason 3).** The join completed
  (`auth → assoc → run`, WPA2-PSK, rssi −62) and then the camera sent a
  deauth and shut its AP. Asked whether the API wants proof we're an app:
  no, the drop came before DHCP, and nothing app-level runs that early.
  Added `wifi-ps!` and `ble!` switches and turned both off.
- **With power save off: got an address, then dropped ~5 s after joining.**
  `cam-join` worked (`cam ready ILCE-7RM3`), `cam-api` listed 49 methods,
  `cam-evt` read the settings. Polling `getEvent` every second did not
  stop the drop; it came about 5 s after association every time.
- **startLiveview keeps it up.** Imaging Edge Mobile always starts
  liveview right after connecting, so we did too: alive 30 s of polling
  and still connected after. The camera's screen just shows its normal
  shooting view. `cam-join` now calls `startLiveview`.
- **What the camera offers.** `getApplicationInfo`: "Smart Remote Control
  2.1.7". `startRecMode` answers 12 (no such method) and isn't needed.
  `ss!` works. `shoot` and `ev!` gave `err 1`: the drive mode was
  Continuous, which swaps `actTakePicture` for `startContShooting`, and the
  EV dial sat at −1, which takes EV away from the API. `setFNumber` is
  absent in Shutter priority, as expected.
- Committed the lens (`f89ac4a`) and the firmware (`958cee0`). Flashed the
  `startLiveview` build and turned BLE and power save back on.

## 2026-11-04 — Scout, the second camera

- Sony offline (its AP never came up in two 10-minute waits), so we built
  the second camera to drive the lens UX.
- The board on `/dev/cu.usbserial-022D4010` (CP2104): esptool says
  ESP32-D0WDQ6 rev 1.0, 4 MB flash; the factory firmware's log shows 4 MB
  PSRAM, an OV2640 and Model B pins (data 19/36/18/39/5/34/35/32, VSYNC 25). It cold-boots with
  `flash read err, 1000` and a watchdog reset before booting fine.
- `scout_m5camera_espidforth`: the maplet's render and dither, AlphaFriend's
  BLE console, a FAT roll on flash. First boot looped on the watchdog right
  after the camera started. Cause: esp32-camera's ISR warning prints a
  string from flash, and the roll's first format ran with the camera
  already streaming. Fixed by compiling at log level ERROR (runtime INFO)
  and mounting the roll first. The crashed boots had left a broken FAT
  (writes failed with "Permission denied"); `roll-format` fixed it.
- Working: `shoot` → `ok 0`, `ok 1`; `last`, `lv`, `roll-th`, `roll-ls`.
  Pulled frames back to PNG: real pictures, dithered as intended.
- Idea: Scout serves frames on the LAN and sends its URL over BLE; the
  lens grabs frames at 5–10 FPS (earlier work reached that). The BLE +
  WebView `PERMISSION_DENIED` from SwitchbackXR may have been the order of
  requests, not a rule; WebView may reach a LAN `http`/`ws` URL. To test
  next session (see DESIGN.md open items).

## 2026-11-04 — Scout on the LAN

- Scout v0.5.0: the sensor now runs in JPEG (the dither path decodes it);
  Wi-Fi station, a web server (`/frame.jpg`, `/stream`, `/ws`, `/info`, a
  test page at `/`), and `url?` / `web?` on the BLE console.
- First start: `ESP_ERR_HTTPD_TASK`. BLE + Wi-Fi + camera left ~20 KB of
  internal RAM. With PSRAM malloc on, Wi-Fi TX buffers must be static; cut
  them 16 → 8, static RX 10 → 6 (BA window 6), NimBLE to PSRAM: 74 KB free,
  server up.
- From the Mac (Wi-Fi modem sleep on, BLE advertising): MJPEG ~19 fps,
  `/ws` one-in-flight 7.7 fps, `/frame.jpg` new connection each ~180 ms
  (5.5 fps); QVGA JPEGs ~6.3 KB. The first `/frame.jpg` batch right after
  boot timed out (8 of 21); not seen again.
- `LanProbe.ts` (lens, object `LanProbe`, disabled) runs fetch / media /
  ws / webview with `ble-first`, `net-first` or `net-only`. Glasses run next.

## 2026-11-04 — LanProbe on the glasses (ble-first)

- **A Bluetooth lens can reach a LAN IP over plain http and ws.** BLE
  connected to Scout and read `url?`, then with the link still up:
  `fetch` PASS (20 frames, 766 ms each, 1.3 fps, ~12.7 KB); `ws` PASS (16
  frames, 658 ms each, 1.5 fps); BLE still live afterwards (154 ms). Scout's
  `web?` saw the glasses at 10.0.0.165. No Extended Permissions; the lens
  declares InternetModule as before.
- `media` (RemoteMediaModule + makeResourceFromUrl) FAIL: "media download
  failed". `webview` FAIL: no onReady in 15 s, and the glasses showed an
  "input text permission denied" notice.
- 1.5 fps against the Mac's 7.7 over the same `/ws`. Suspect: Scout's one
  radio shares time with the live BLE link. Next run drops BLE and times
  again, and tries `ws` with 2 requests in flight.
- Second run (13:19): fetch 1.2 fps, ws 1.4, **ws x2 2.9 fps**. With BLE
  dropped: fetch 1.1, ws 1.9. So the live BLE link is not the bottleneck;
  each request costs ~0.7 s on the glasses' side (the Mac: ~0.13 s), and
  keeping two requests in flight doubles the rate. Next: x4 in flight, or
  Scout pushing frames over the socket unasked.
- "PERMISSION_DENIED INPUT_TEXT" isn't in Lens Studio's logs (the adb
  device it lists is the Sharp phone, not the glasses). Best guess: the
  WebView wants Text Input, which a lens must declare (TextInputModule);
  LanProbe now declares it.
- Third run (13:30, TextInputModule declared): ws x2 2.9 fps, **ws x4 2.8**:
  more in flight doesn't help, so it's throughput, ~36 KB/s at ~12.5 KB a
  frame. lwIP's default 5760-byte send window over the glasses' longer
  round trip gives about that. Scout now uses a 32 KB send buffer and a
  16 KB window (PSRAM); the Mac still sees 5.7 fps at 12.9 KB a frame.
  WebView still never ready.
- Fourth run (13:50, Scout's bigger TCP buffers), BLE connected: fetch
  1.9 fps, ws 2.7, ws x2 6.2, **ws x4 9.1 fps** (110 ms a frame, ~13 KB).
  The window was the limit. **WebView PASS**: ready in 1.2 s, loaded
  Scout's page. BLE still live afterwards. Without BLE: fetch 3.0, ws 3.6,
  ws x2 5.0 (no better than with it). The INPUT_TEXT notice still showed,
  so it doesn't block the WebView; its source is still unknown.
- Fifth run (18:53, after Scout was power-cycled; same IP), BLE connected:
  BLE live in 1.5 s, url over BLE. fetch 1.7 fps, ws 2.3, ws x2 6.0,
  **ws x4 8.3 fps** (121 ms, ~10.5 KB). WebView PASS, ready in 1.7 s. BLE
  still live (117 ms). Without BLE: fetch 2.7, ws 3.8, ws x2 4.8. Media
  from URL still FAIL. Repeats the fourth run. No INPUT_TEXT notice this
  time (same lens: TextInputModule declared, WebView step included), so
  it showed in runs 1-4 but not here; cause still unknown.
- Conclusion: control over BLE, frames over `ws` with ~4 requests in
  flight, 5-10 fps, works on the glasses without Extended Permissions.
