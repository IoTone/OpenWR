# LightScopeXR — Sony α7R III remote on Spectacles (design)

**Status:** 2026-11-04. AlphaFriend talks to the α7R III (L0 mostly
passed); the lens is not built yet. The lens is a copy of
`../SpecsSwitchbackXR`; its Bluetooth link (`Assets/ProjectScripts/ble/`) is
proven on the glasses (see `../SpecsSwitchbackXR/BLE-NOTES.md`). How we got
here, step by step: [LOG.md](LOG.md).

Goal: a stand-in for Imaging Edge Mobile that works from Spectacles ('24).
Control first: shutter, AF, record, f-stop, shutter speed, ISO, EV. Then an
occasional look through the viewfinder and at the last shot. A low-res
picture is fine.

## Roles

| Device | Job |
|---|---|
| Spectacles (LightScopeXR) | the remote: buttons, dials, camera state, a low-res frame now and then. Bluetooth only. |
| StickCPlus_AlphaFriend_OpenWR (M5StickC Plus SE, ESP32-PICO-D4) | the proxy: Nordic UART (NUS) peripheral to the glasses, Wi-Fi client to the camera; its screen shows setup state |
| Sony α7R III (ILCE-7RM3) | the camera, in **Ctrl w/ Smartphone** (its own Wi-Fi AP) |

## Constraints that shape it

- **No Bluetooth and internet in one lens.** Spectacles refused the BLE scan
  (`PERMISSION_DENIED`) while the lens also had internet access. That only
  works with Extended Permissions, and then the lens can't be published.
  So the lens can't use BLE for the trigger and also pull the viewfinder
  over Wi-Fi.
- **A lens can't speak the camera's protocols.** It has `fetch` and
  WebSockets (`https`/`wss` in Snap's docs), and no plain TCP sockets.
  Nobody has confirmed that a lens can reach a LAN address like
  `192.168.122.1` over plain HTTP.
- **What the α7R III offers:**
  - Wi-Fi, **Ctrl w/ Smartphone**: the camera is the AP (`DIRECT-xxxx:ILCE-7RM3`,
    WPA2, camera at `192.168.122.1`) and runs Sony's older JSON-RPC Camera
    Remote API over HTTP, as "Smart Remote Control 2.1.7". Verified on our
    body; see the next list.
  - Bluetooth remote (as RMT-P1BT): needs **firmware 3.00 or later** and
    `Bluetooth Rmt Ctrl` set to On. Shutter, half-press and record only, with
    no exposure settings. It needs bonding: an ESP32 can bond, the glasses
    can't.
  - PC Remote: over USB for sure. Over Wi-Fi (PTP/IP) not confirmed for
    this body.
  - Sony's Camera Remote SDK does **not** list the α7R III (its list starts
    at the 7R IV), so the SDK route is out.
- **Bluetooth to the glasses is slow.** A 4.7 KB maplet frame takes several
  seconds to reach the glasses. A 640-wide liveview JPEG is tens of KB.
  So the StickC shrinks each frame first: decode at 1/8 scale with TJpgDec
  (about 80×53), dither it, and send about 0.5–2 KB with the roll's `b64`
  framing. That should give one picture every 1–3 s.
- **The StickC Plus SE is tight:** no PSRAM, 520 KB SRAM, one 2.4 GHz radio
  shared by Wi-Fi and BLE, and no IMU (Alpha Fairy's tilt features need
  stubbing).

## What the α7R III's Smart Remote needs (verified)

- **Start liveview right after joining.** Without `startLiveview` the
  camera deauths the client and closes its AP about 5 s after the join,
  whatever else the client sends. The stream needn't be read.
  `cam-join` does this.
- **No `startRecMode`.** It answers 12 (no such method); remote mode is
  already on.
- **The method list follows the body's controls.** `getAvailableApiList`
  (49 methods in Shutter priority, Continuous drive) changes with them:
  - Drive mode **Continuous** swaps `actTakePicture` for
    `startContShooting` / `stopContShooting`; `setContShootingMode`
    ("Single", "Continuous") is offered, so AlphaFriend can switch it.
  - The **EV dial** off 0 takes `setExposureCompensation` away.
  - **S** mode has no `setFNumber` (A and M not checked yet).

  So the lens draws a dial only when its setter is in the current list, and
  re-asks after every `evt mode`.
- **The AP is short-lived.** It broadcasts only while the camera's
  Connection screen is up, and goes when the camera powers down. AlphaFriend
  keeps retrying, so turning Ctrl w/ Smartphone on is the only step at the
  camera.

## Prior art

[Alpha Fairy](https://github.com/frank26080115/alpha-fairy) (MIT, Arduino,
M5StickC-Plus, last commit May 2023). It already speaks PTP/IP and, for
older bodies, the JSON-RPC API in STA mode (it joins the camera's AP). It
does shutter, video, f-stop/shutter speed/ISO/EV, focus pull and stacking,
and an intervalometer. The α7R III isn't on its tested list.

## Options considered

| | Path | Verdict |
|---|---|---|
| **A** | Glasses ⇄BLE NUS⇄ AlphaFriend ⇄Wi-Fi JSON-RPC⇄ camera AP | **Build this.** The lens stays publishable, gets full exposure control and a low-res liveview. |
| A+ | A, plus AlphaFriend bonded to the camera as a BLE remote | Later: a trigger that still works when Wi-Fi drops. Check whether the camera allows the BT remote while in Ctrl w/ Smartphone. |
| B | Glasses join the camera AP and call JSON-RPC directly; AlphaFriend is only a BLE trigger | Developer glasses only (Extended Permissions), and it depends on plain HTTP to a LAN IP. Run one probe and don't build on it. |
| C | Camera on the home/router Wi-Fi, relay to a cloud `wss://`, an internet-only lens | Better pictures, but it needs a server, adds latency, and the lens loses BLE. Smartphone control on this body uses its own AP, so it needs a relay box anyway. |
| D | Pi + Camera Remote SDK over USB | Out: the SDK doesn't support the α7R III. |
| E | ESP32-S3 USB host, PTP over the cable | Possible fallback (full PTP, no Wi-Fi). Needs a PTP host driver written. |

## AlphaFriend line contract (over NUS)

Plain lines, one reply each, the same shape as the goggl1/maplet consoles.
The words are Forth, so arguments come first. Lines marked • work today.

```
• cam?                  -> cam <state> <model> <rssi>   state: off|join|api|ready|err
• cam-join              -> cam ready ILCE-7RM3 -63      find the camera, start liveview
• cam-api               -> api <method> ...             what the camera offers right now
• cam-evt               -> evt <key> <value> ...        status mode f ss iso ev focus af shoot
• shoot                 -> ok | err <code>              actTakePicture (Single drive)
• af  af-  rec  rec-                                    half-press, release, movie
• 56 f!                                                 f/5.6 (x10)
• s" 1/250" ss!                                         s" 2s" ss! is 2"
• 100 iso!                                              0 iso! is AUTO
• -3 ev!                                                thirds: -1.0 EV
  mode? f? ss? iso? ev? -> current value + available list
  lv                    -> lv <w> <h> <b64...>          one shrunk liveview frame
  last                  -> last <w> <h> <b64...>        shrunk postview of the last shot
  evt <key> <value>        pushed when getEvent reports a change
  roll?                 -> roll <count> sd <free>       frames stored on this device
  <n> 4 roll-th         -> rth <n> 80 53 1 <b64...>     4 reel thumbnails from frame n back
  <n> roll-b64          -> one whole stored frame (the maplet's word today)
  roll-put <w> <h> <bits> <b64...> -> ok <n>            store a Specs frame on this card
```

Every setter answers `ok` or `err <code>` with Sony's error code (1 means
the method isn't offered in the camera's current state).

## One radio, two links

The PICO-D4 has one 2.4 GHz radio for both the glasses (BLE) and the camera
(Wi-Fi). ESP-IDF shares it by time slicing, which is built around Wi-Fi
modem sleep. `WIFI_PS_NONE` is accepted with BLE running, but coexistence
warns that the station then stays awake only when the radio is free.

- **Modem sleep is the default** (`1 wifi-ps!`). With it on and no
  liveview, the camera dropped the StickC 0.4 s after the join. Whether
  liveview alone keeps the link up with modem sleep on: see Open items.
- **BLE only advertises when idle** and stays connected to one pair of
  glasses. A pairing window re-opens whenever nobody is connected, since
  the glasses can't bond.
- **Test switches:** `0 ble!` + `reboot` boots without BLE; `0 wifi-ps!`
  keeps the Wi-Fi radio awake. Both are saved in NVS.

## On the StickC screen

Wi-Fi: SSID / joined / API ready; camera model; glasses link up or down;
last command and its result; battery. A button to re-scan the camera AP. A
setup page for the camera SSID and password (Alpha Fairy has one).

## Phasing

| Phase | Delivers |
|---|---|
| L0 Go/no-go | AlphaFriend on the camera's AP: `getAvailableApiList`, `getEvent`, a setter, `actTakePicture`, one liveview frame. Done except the shot (drive mode) and the frame. |
| L1 Proxy | AlphaFriend on ESPIDFORTH (see below): `sony_http.c`, NimBLE NUS + the contract above; round-trip times from the Mac with Wi-Fi active (as in BLE-NOTES). |
| L2 Remote | Lens: Remote panel (shutter, AF, rec), dials for f/ss/iso/ev, live `evt` state. The Specs camera from `snap/Viewfinder.ts`. |
| L3 Peek | `lv` on request and every N s; `last` after each shot; AlphaFriend's reel in LittleFS; Reels view. Reuse `ui/Bitmap.ts`, `snap/Dither.ts`. |
| L4 Trigger | A+: BLE remote bond as a backup shutter. |
| Probe | B on Extended Permissions: can a lens reach `http://192.168.122.1`? |

## Photo streaming

Every picture that reaches the glasses is made on AlphaFriend, because
Bluetooth carries about 1.4 KB/s. There are three sizes.

- **Liveview stream** (`lv+` / `lv-`):
  - AlphaFriend decodes each liveview JPEG at 1/8 scale (80×53) and
    dithers it to 1 bit with a fixed Bayer pattern. Floyd–Steinberg would
    change every row from frame to frame.
  - It pushes only the rows that changed (`lvd <seq> <row> <b64>`), with
    a key frame every 10 s.
  - Expect about 1–2 fps holding still, about 1 fps panning.
  - Commands go ahead of stream rows.
  - Only the active camera streams.
- **Postview:**
  - `last` is an 80×53 version right after each shot.
  - `pv 240 160 2` is a 2-bit version in four interlaced passes (9.6 KB).
    The first pass arrives in about 0.2 s and the image is sharp in
    about 9 s.
  - AlphaFriend keeps the postview until the next shot.
- **Full resolution:** never over BLE. A 42 MP JPEG is about 15 MB, which
  is hours of Bluetooth. It stays on the card. After the session the
  camera's own Send to Smartphone moves the keepers marked in the Sheet.
- **Experiment:** on Extended Permissions glasses, AlphaFriend could run
  its own AP and serve MJPEG to the lens. That depends on a lens reaching
  a LAN address, and the lens can't be published.

## Specs as a camera

The glasses' own camera is a fourth camera on the Rack, called Specs.
SwitchbackXR already shoots with it (`snap/Viewfinder.ts`,
`snap/Dither.ts`): a head-locked target, the frame cropped to it and
dithered on the lens.

- **Stylized on purpose**, like the maplet: 1-bit grain (Floyd–Steinberg),
  1-bit screen (Bayer) or 2-bit, cropped to 212×104, 3:2 or square. The
  frame is the picture, not a stand-in for an original.
- **Stored on an SD card.** The lens keeps the last few. Each shot also
  goes to a linked Scout's or maplet's card with `roll-put` (212×104 1-bit
  is 2.7 KB, about 2.7 s over BLE, in the background).
- **No link needed**, so its peek runs at the lens's frame rate. Declaring
  `InternetModule` lets camera and BLE run together (Transparent
  Permission, still publishable), as in SwitchbackXR.
- **It joins Sync**, so the glasses can fire with the other cameras.

## A reel per camera

Each camera keeps its own roll, maplet style. The Reels view (it replaced
the Sheet) shows one strip per camera, newest first, 4 thumbnails a page
(`roll-th`, about 2 s), cached on the lens. A whole frame comes over only
when picked.

| Camera | Reel lives in | How far back |
|---|---|---|
| α7R III | AlphaFriend's flash: the 80×53 postview of each shot (530 B) | every session AlphaFriend ran, ~2 000 frames per MB of LittleFS. Older card contents need Contents Transfer (open). |
| Scout | its flash (M5Camera has no SD): frame + thumbnail, ~3.4 KB a shot | ~600 shots in 2.1 MB |
| maplet | its microSD (`roll-save` rolls) | the whole card |
| Specs | the lens (last few) + a linked Scout or maplet SD | the whole card |

## AlphaFriend firmware: Alpha Fairy on ESPIDFORTH

Alpha Fairy is Arduino (arduino-esp32, itself on ESP-IDF); no ESP-IDF fork
of it turned up. So vendor it for its camera knowledge and port that onto the
stack goggl1 and the maplet run: ESPIDFORTH on ESP-IDF 5.3, the NimBLE NUS
console (`ble_console.c`), bundles and OTA. ESPIDFORTH already has
`env:esp32` for the Plus SE's PICO-D4.

| Alpha Fairy | Becomes | When |
|---|---|---|
| `libraries/SonyHttpCamera/` (~43 KB): JSON-RPC, liveview, postview | `src/sony_http.c` on `esp_http_client`, Alpha Fairy's key scanner (no cJSON); words `cam-join` `cam-rpc` `shoot` `f!` `ss!` `iso!` `ev!` `lv` | first: the α7R III's path |
| `AutoConnect.ino`, `WifiUtils.ino` | the maplet's `net_words.c` (STA, NVS) plus a `DIRECT-*` scan | first |
| `libraries/PtpIpCamera/` (~85 KB): PTP/IP :15740, Sony props | `src/ptpip.c` on lwIP sockets, same words | only if PC Remote over Wi-Fi works on this body, or for newer Sonys |
| `Intervalometer.ino`, `FocusStack.ino`, `ShutterStep.ino` | Forth words over the C words (`seq int`, `seq brk`) | L5 |
| menus, IMU tilt, Lepton, sound trigger, IR | left behind (no IMU on the SE; the glasses are the menu) | — |

- C for the wire (HTTP, JSON, JPEG decode, dither), Forth for the policy
  (retries, sequences, what to push), the same split as the maplet's
  `cam_words.c`. Policy changes by bundle, without a reflash.
- No PSRAM: one ~40 KB liveview JPEG buffer, decoded straight to 80×53
  with the TJpgDec in the ESP32's ROM.
- Screen: ST7789 135×240 on `esp_lcd`.
- MIT: the vendored copy keeps Alpha Fairy's notice; ported files say where they
  came from.

## N cameras

The UX prototype is `docs/lightscopexr.html`, published as a private
artifact. Its views are Rack, Shot, Peek, Reels and Sequence.

- **One link per camera.** Each camera is a link in `Links.ts`. It reports
  its capabilities (`caps?`), and the views draw only those. The α7R III
  gets f, ss, iso and ev dials; an ESP32-CAM "Scout" gets size, quality and
  flash; the maplet gets a shutter only; Specs gets style and frame.
- **Scout cameras skip the proxy.** They speak the same NUS contract
  directly. The first is an M5Camera (PSRAM) Model B
  (`projects/prototypes/scout_m5camera_espidforth/`): shoot, last, lv,
  style and frame, a roll on flash. It answers `roll-ls`/`roll-b64` as the
  maplet does, so the lens's RollView reads it unchanged. JPEG size,
  quality and a flash LED are not there yet.
- **One AlphaFriend per Sony.** The camera's AP takes one client.
- **Link budget:** plan for three cameras, test four. Only the active
  camera peeks; the rest answer `cam?` every 5 s.
- **Sync fire:** one `shoot` per link, in turn, about 40–100 ms apart. An
  `arm`/`fire` pair comes later.
- **Sequences run on the device** (`seq int`, `seq brk`), so a dropped link
  doesn't stop them.

## Repo layout

- `projects/apps/LightScopeXR/`: the lens (this project)
- `projects/prototypes/alpha-fairy-espidforth/`: AlphaFriend, the proxy firmware
  (Alpha Fairy vendored in `third_party/alpha-fairy/`, ported into `src/`)
- `projects/prototypes/scout_m5camera_espidforth/`: Scout, the second camera

## Open items

- Copied from SwitchbackXR, still to trim: Pulse, Pins, goggl1 and maplet
  scripts, and the WebView package (a BLE lens can't use it).
- `LightScopeXR.esproj` still carries SwitchbackXR's `documentId`. Check in
  Lens Studio that pushing this lens doesn't replace the SwitchbackXR draft.
  If it does, use File → Save As there.
- **Wi-Fi and BLE together:** does the camera keep the StickC with modem
  sleep on, now that liveview starts at once? If not, BLE has to pause
  while the camera is linked, or the proxy needs a second radio.
- **Shooting in Continuous:** switch the body to Single
  (`setContShootingMode`), or make `shoot` a short
  `startContShooting`/`stopContShooting` burst? Undecided.
- **Pixels over the LAN, control over BLE (to test).** The camera serves
  frames over plain HTTP/WS on the LAN and sends its URL over BLE (Specs
  have no mDNS, so connect by IP); the lens grabs frames periodically.
  5–10 FPS is enough; full motion isn't the goal. Earlier work reached
  about 5–10 FPS this way. The user believes WebView can open a LAN
  `http`/`ws` URL, and that the earlier BLE `PERMISSION_DENIED` with
  WebView may have come from the order of requests (BLE scan vs. WebView
  load), not a hard rule. Experiments: WebView then BLE, BLE then WebView,
  InternetModule fetch to a LAN IP, each with BLE connected. Scout is the
  test bed (hardware JPEG, `url?` word). If it holds, BLE dither frames
  become the fallback.
- Does PC Remote over Wi-Fi (PTP/IP) work on the α7R III? If so, port
  `PtpIpCamera` too.
- Can AlphaFriend list the card (`avContent` / `getContentList`) while in
  remote shooting? Without it the α7R III reel holds only what AlphaFriend saw.
