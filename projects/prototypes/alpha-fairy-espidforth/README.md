# alpha-fairy-espidforth

AlphaFriend, the camera proxy for [LightScopeXR](../../apps/LightScopeXR/):
[Alpha Fairy](https://github.com/frank26080115/alpha-fairy)'s Sony camera code
ported from Arduino onto ESPIDFORTH, on an M5StickC Plus SE (ESP32-PICO-D4).
The glasses talk to it over BLE (Nordic UART); it joins the camera's own Wi-Fi
and drives it with Sony's Camera Remote API (JSON-RPC over HTTP).

Status: step 1. `sony_http.c` (from Alpha Fairy's `SonyHttpCamera`) builds and
its JSON scanner passes a host test. Not yet run against a camera.

## Build and flash

```
pio run                         # env stickc_plus_se
pio run -t upload -t monitor    # the StickC shows up as /dev/cu.usbserial-*
```

## L0 from the console (USB or BLE)

On the α7R III: Menu → Network → Ctrl w/ Smartphone → On, and read the SSID
and password it shows. Then:

```
s" DIRECT-xxxx:ILCE-7RM3" s" xxxxxxxx" wifi!   \ once; kept in NVS
cam-join        \ -> cam ready ILCE-7RM3 -52
cam-api         \ -> api getEvent actTakePicture setFNumber ...  (the go/no-go)
cam-evt         \ -> evt mode Manual / evt f 5.6 / evt ss 1/250 / evt iso 100 ...
56 f!           \ f/5.6           -> ok | err <code>
s" 1/250" ss!   \ s" 2s" ss! is 2"
100 iso!        \ 0 iso! is AUTO
-3 ev!          \ thirds: -1.0 EV
af  af-  shoot  rec  rec-
1 cam-dbg       \ print every request and reply
```

Holding the front button for a second runs `cam-join` again. AlphaFriend
advertises as `AlphaFriend` whenever nothing is connected, since the
glasses can't bond.

## Layout

| Path | What |
|---|---|
| `src/sony_http.c` | the port: SSDP, dd.xml, init, JSON scanner, commands, Forth words |
| `src/net_words.c`, `src/ble_console.c` | from the maplet: Wi-Fi station + NVS, the BLE console |
| `src/main.c` | UART0 + BLE console, the housekeeping task |
| `components/forth/` | ESPIDFORTH, vendored as in the maplet |
| `third_party/alpha-fairy/` | the Alpha Fairy sources the port reads from (see PROVENANCE.md) |

## Next

- L0 on the real camera: `cam-api` shows which methods this body lists.
- `lv`: one liveview frame, decoded at 1/8 scale with the ROM TJpgDec, dithered.
- `last` / `pv`: the postview, shrunk; the reel in LittleFS.
- `PtpIpCamera` only if PC Remote over Wi-Fi works on the α7R III.
