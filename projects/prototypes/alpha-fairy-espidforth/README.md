# alpha-fairy-espidforth

AlphaFriend, the camera proxy for [LightScopeXR](../../apps/LightScopeXR/):
[Alpha Fairy](https://github.com/frank26080115/alpha-fairy)'s Sony camera code
ported from Arduino onto ESPIDFORTH, on an M5StickC Plus SE (ESP32-PICO-D4).
The glasses talk to it over BLE (Nordic UART); it joins the camera's own Wi-Fi
and drives it with Sony's Camera Remote API (JSON-RPC over HTTP).

Status: joins an α7R III, reads its settings and sets shutter speed. The
design is in [LightScopeXR/DESIGN.md](../../apps/LightScopeXR/DESIGN.md); the
journey is in [LightScopeXR/LOG.md](../../apps/LightScopeXR/LOG.md).

## Build and flash

```
pio run                         # env stickc_plus_se
pio run -t upload -t monitor    # the StickC shows up as /dev/cu.usbserial-*
```

## L0 from the console (USB or BLE)

On the α7R III: Menu → Network → Ctrl w/ Smartphone → On, then Connection.
The text SSID and password are behind the QR code screen. Before testing,
set Power Saving Start Time longer, Drive Mode to Single (Continuous has no
`actTakePicture`), and the EV dial to 0 (otherwise no remote EV). Then:

```
s" DIRECT-xxxx:ILCE-7RM3" s" xxxxxxxx" wifi!   \ once; kept in NVS
cam-join        \ -> cam ready ILCE-7RM3 -63   (also starts liveview, see below)
cam-api         \ -> api getAvailableApiList ... api 49 methods
cam-evt         \ -> evt status IDLE / evt mode Shutter / evt f 2.8 / evt ss 1/200 ...
56 f!           \ f/5.6           -> ok | err <code>
s" 1/250" ss!   \ s" 2s" ss! is 2"
100 iso!        \ 0 iso! is AUTO
-3 ev!          \ thirds: -1.0 EV
af  af-  shoot  rec  rec-
1 cam-dbg       \ print every request and reply
s" getContShootingMode" cam-rpc   \ any no-argument method, raw reply
```

`cam-join` calls `startLiveview` straight away: without it the camera drops
the client and closes its AP about 5 s after the join.

Radio test switches, saved in NVS:

```
0 ble!  reboot    \ boot without BLE (1 ble! reboot to bring it back)
0 wifi-ps!        \ Wi-Fi radio always awake; 1 wifi-ps! is modem sleep (default)
```

The screen shows the Wi-Fi state, the camera state and model, whether the
glasses are linked, and a message line (`s" text" msg!`). The alpha at the
right is red until the camera is linked, then orange and breathing. On the
StickC Plus the AXP192 powers the panel and backlight, so `display.c`
switches its LDO2/LDO3 on at boot; a board without the AXP192 gets the
Plus2 layout (backlight on GPIO27).

Holding the front button for a second runs `cam-join` again. AlphaFriend
advertises as `AlphaFriend` whenever nothing is connected, since the
glasses can't bond.

## Layout

| Path | What |
|---|---|
| `src/sony_http.c` | the port: SSDP, dd.xml, init, JSON scanner, commands, Forth words |
| `src/net_words.c`, `src/ble_console.c` | from the maplet: Wi-Fi station + NVS, the BLE console |
| `src/display.c` | the status screen: AXP192 power, ST7789, 5x7 font, the alpha |
| `src/main.c` | UART0 + BLE console, the housekeeping task |
| `components/forth/` | ESPIDFORTH, vendored as in the maplet |
| `third_party/alpha-fairy/` | the Alpha Fairy sources the port reads from (see PROVENANCE.md) |

## Next

- Wi-Fi and BLE together on the camera link (modem sleep on).
- `shoot` in Continuous drive: switch to Single, or a short burst.
- `lv`: one liveview frame, decoded at 1/8 scale with the ROM TJpgDec, dithered.
- `last` / `pv`: the postview, shrunk; the reel in LittleFS.
- `PtpIpCamera` only if PC Remote over Wi-Fi works on the α7R III.
