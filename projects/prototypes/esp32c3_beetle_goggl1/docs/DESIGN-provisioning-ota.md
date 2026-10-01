# goggl1 provisioning and OTA — design (weather HUD phase 4)

**Status:** device side and the Switchback app implemented 2026-10-01; packages are `.mnpkg` (below).
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

Packages are **MagNET OTA packages (`.mnpkg`, wire format 1)**, the format
ProjectMagNET defined for its mesh nodes (`docs/OTA-PACKAGE.md`, rev 1.4),
so one release format serves every IoTone device: a 256-byte signed header
(magic `MNPK`, chip and board ids, version, SHA-256 of the image, variant,
key id, Ed25519 or P-256 signature over the first 192 bytes) followed by
the ESP-IDF app image byte for byte. Nothing in the format is specific to
the mesh transport, which is the point.

`ota! ( url u -- )` fetches a `.mnpkg` over HTTP(S) and runs the spec's
nine checks in order, refusing with the check number and name:

| # | Check | Refusal |
|---|---|---|
| 1 | magic, format version, header length, algorithm, flags, reserved bytes | `E_PKG_FORMAT` |
| 2 | key id is in the compiled-in store | `E_PKG_KEY` |
| 3 | signature over header bytes 0..191 (Ed25519 via Monocypher) | `E_PKG_SIG` |
| 4 | chip `0x0005` ESP32-C3, board `0` or `0x0503` Beetle, ESP-IDF image | `E_PKG_TARGET` |
| 5 | `REQUIRES_REPARTITION` clear | `E_PKG_REPARTITION` |
| 6 | image fits the other OTA slot | `E_PKG_SIZE` |
| 7 | SHA-256 of the payload, once as written and once read back from flash | `E_PKG_SHA` |
| 8 | ESP-IDF's own image check (`esp_ota_end`) | `E_PKG_IMAGE` |
| 9 | running version is at least `min_running` | `E_PKG_MIN` |

Checks 1 to 6 happen on the first 256 bytes, before anything is written.
On success the slot is marked bootable and the chip reboots; the new image
must reach "stored bundle applied, scheduler running" to cancel the
bootloader's rollback, which is where `ota_mark_app_valid_cancel_rollback`
is called. `ota-keys` prints the key store. A raw `firmware.bin` URL still
works for bench use and says so.

**Board id.** Beetle ESP32-C3 is not in the spec's registry yet; this
firmware and `tools/mnpkg/mnpkg.py` use **`0x0503`** (high byte = chip) and
the row should be appended to OTA-PACKAGE.md §3.1.

**Keys.** The release-key store is `src/mnpkg_keys.h`, generated by
`mnpkg.py c-keys`. It currently holds the MagNET DEV Ed25519 key
(`f0846c794440d0ea`), so the same bench key signs goggle and mesh packages;
the firmware prints "DEV, bench only" at boot while it carries it. Replace
before anything leaves the bench; `mnpkg.py build` refuses the DEV key for
any variant not starting with `dev`.

**Building a package:**

```
pio run -e beetle_c3
tools/mnpkg/mnpkg.py build --chip esp32c3 --board beetle-esp32c3 \
    --variant dev-weather --version 0.5.3+1 --fw-version 0.5.3 \
    --key tools/mnpkg/keys/dev_release_ed25519.pem \
    .pio/build/beetle_c3/firmware.bin -o releases/goggl1-0.5.3-dev-weather.mnpkg
tools/mnpkg/mnpkg.py verify releases/goggl1-0.5.3-dev-weather.mnpkg --pubkey tools/mnpkg/keys/dev_release_ed25519.pem
```

**Where the package comes from.** OpenWR's principle is no third-party
cloud required, so the phone hosts it: Switchback embeds releases under
`assets/releases/`, serves one over its WiFi address on port 8777 for the
duration of the install, and sends `ota!` with that URL. RobotARme will
host releases as its own release kind later; the device side does not
change for that, only the URL. Plain HTTP is allowed because the package,
not the transport, is what is verified.

Verified 2026-10-01: Switchback on an Android phone served
`goggl1-0.5.3-dev-weather.mnpkg` (1.1 MB) to the goggle over the home WiFi,
the goggle refused a package verified with the wrong primitive at check 3
(which is the check working), then with standard Ed25519 passed all nine,
staged `ota_1`, rebooted into 0.5.3, re-applied the stored bundle and
confirmed the image, with progress shown on the phone from the console.

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
