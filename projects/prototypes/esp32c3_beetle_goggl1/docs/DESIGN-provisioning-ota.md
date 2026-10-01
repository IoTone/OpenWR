# goggl1 provisioning and OTA — design (weather HUD phase 4)

**Status:** device side implemented 2026-10-01; mobile app not started.
**Depends on:** `DESIGN-weather-hud.md` §2 (the bundle) and §8 (phase 4).

## 1. The one decision that shapes everything

The phone does not get a provisioning protocol. It gets the same `ok>`
prompt the USB cable gets, over Bluetooth LE, using the Nordic UART Service
(NUS) that every BLE terminal app already speaks.

Everything the app will ever need to do is already a Forth word:

| App action | What the app sends |
|---|---|
| Set WiFi | `s" MySSID" s" MyPassword" wifi!` |
| Set location by hand | `455234 -1226762 geo!` |
| Units | `1 units!` |
| Theme | `2 theme!` |
| Push a new use case | `bundle-begin` … lines … `bundle-end` |
| Update firmware | `s" http://192.168.1.10:8000/firmware.bin" ota!` |
| Ask how it is doing | `status` |

So the app is a terminal with buttons on it. The protocol is the language
the device already has, which means there is nothing to version, nothing
to keep in sync between app and firmware, and the first "app" is nRF
Connect or Serial Bluetooth Terminal, installed from the store today. The
OpenWR app, when it exists, only hides the words behind a form.

## 2. Service

Nordic UART Service, unchanged, because it is what the tools expect:

| UUID | Role |
|---|---|
| `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` | service |
| `6E400002-…` RX | write, write-without-response: bytes into the console |
| `6E400003-…` TX | notify: everything the console prints |

Bytes written to RX go into the same input path the USB console uses, so
bundle capture, line editing and the `ok>` prompt behave identically. TX
carries console output in MTU-sized chunks (up to 180 bytes after MTU
exchange; 20 before). Output is mirrored to USB and BLE; input is merged.

Advertised name: `goggl1`. One connection at a time.

## 3. When the radio is on

Not by default. A HUD that beacons continuously is a tracking device, and
NimBLE's host and controller cost RAM the forecast fetch would like to
have. So:

- **Long press of BOOT (1 s or more)** opens a **3-minute pairing window**:
  advertising starts, the status strip shows a `B`, and a connection
  extends the window for as long as it lasts.
- **`pair`** at the USB prompt does the same, for testing without a thumb.
- When the window closes with nobody connected, advertising stops.
- A connected client keeps the link until it disconnects; if the window is
  still open the device advertises again, otherwise it goes quiet. It never
  re-advertises on its own outside a window.

Security in this phase is the button. Anyone within BLE range *during a
window the wearer opened by hand* gets the prompt, which is the same trust
model as the USB port on the device in the wearer's hand. Phase 5 adds
LE Secure Connections with a passkey shown on the glass, which this panel
is unusually good at.

Short press still cycles the theme; the scheduler distinguishes the two by
hold time (100 ms to 1 s short, 1 s or more long).

## 4. OTA

`ota! ( url u -- )` fetches a firmware image over HTTP or HTTPS with
`esp_https_ota`, writes it to the other OTA slot (the partition table has
had two since phase 1), marks it bootable, and reboots. Progress prints to
the console every 10 % so a BLE client sees it.

Rollback is on: the new image must call `esp_ota_mark_app_valid_cancel_rollback`
after it has booted far enough to be trusted, which here means "the stored
bundle applied and the scheduler started". A new image that panics before
that point is rolled back by the bootloader on the next reset.

**Where does the image come from?** OpenWR's principle is no third-party
cloud required. The app serves the file itself: a phone can run a tiny
HTTP server on the WiFi network it shares with the device, and the URL it
sends is its own address. For development, `python3 -m http.server` in
`.pio/build/beetle_c3/` on the laptop does the same thing. Plain HTTP is
allowed for this reason; the image is signed-and-verified by the
bootloader's app hash, not by the transport. A hosted release channel over
HTTPS is additive, not required.

## 5. Status

`status` prints one line the app can parse by splitting on spaces:

```
goggl1 v0.4.0 net=1 ip=192.168.1.42 geo=Portland wx=1 age=312 clock=1 theme=2 units=1 bundle=weather-hud 0.3 heap=71240 ble=1
```

## 6. Memory

NimBLE as peripheral only, one connection, no central or observer roles,
and pools sized for exactly that (`BLE_MAX_ACT=2`, small ACL and event
pools). lwIP windows and WiFi buffers were trimmed in the same pass, and
the Forth engine's static tables shrank from 512×64-char names to 320×24.

Measured on the device, WiFi associated, after a live TLS forecast fetch:

| Build | Free heap | Low-water mark | Static RAM |
|---|---|---|---|
| BLE on, defaults | 47 KB | 8 KB | 102 KB |
| BLE on, trimmed (shipped) | 104 KB | 67 KB | 74 KB |

The BLE host is initialised at boot (deinit/reinit of the controller is not
worth its failure modes); only advertising is gated.

## 7. Verification

`tools/ble_console.py` is the stand-in app: a `bleak` client that finds
`goggl1`, connects, sends lines, and prints notifications.

Done 2026-10-01 from a laptop over BLE only (USB used just to type `pair`
in place of the button): connected at MTU 185, read `status`, changed the
theme, pushed the 104-line bundle, and sent `ota!` pointing at a
`python3 -m http.server` on the laptop. The device fetched 1.1 MB, wrote
`ota_1`, rebooted, reported `v0.4.1 on ota_1`, re-applied the stored bundle,
and had a fresh forecast on the glass within 25 s.

## 8. Open

- Passkey pairing with the code on the glass (phase 5).
- BLE MTU on iOS is 185; Android varies. Chunking handles both; a very
  chatty `words` listing is the worst case and is fine.
- A long press while a window is open closes it. Not implemented; the
  window times out instead.
