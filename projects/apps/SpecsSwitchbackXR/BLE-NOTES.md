# SpecsSwitchbackXR — BLE without bonding

Goal: Switchback on Snap Spectacles ('24), talking straight to the maplet,
goggl1 and the HR strap over BLE: no phone, no WebSocket relay.

## What blocks bonding

Lens Studio's `Bluetooth.BluetoothCentralModule` (Experimental API) can scan,
connect, read, write (with and without response), and register for
notifications. It has no `createBond()`, no pairing callback, and no
`requestMtu()`; `gatt.mtu` is read-only. SkywriterBLE found that a peripheral
whose CCCD needs encryption makes `registerNotifications` hang forever, and
that turning bonding off in the firmware fixed it. The typings even list
`NotifyEncryptionRequired` as a property, so a probe can spot such a
characteristic before trying it.

## Our devices don't ask for it

- maplet and goggl1 (`src/ble_console.c`): the Nordic UART service, RX
  `WRITE | WRITE_NO_RSP`, TX `NOTIFY`, with no `_ENC`/`_AUTHEN` flags and no
  SMP setup. "Pairing" is only a 180 s advertising window (`pair`, or the
  first boot, or 3 min after a drop).
- Rhythm 24 strap: the standard Heart Rate service, no encryption.
  Switchback reads it without bonding.

## Checked from the Mac (2026-10-21, maplet fw 0.3.1)

Done the way a lens would do it: connect, never pair, subscribe, write in
20-byte chunks.

| step | result |
|---|---|
| connect, list characteristics | RX `write, write-without-response`; TX `notify` |
| subscribe to TX | ok, no pairing prompt |
| `n 2 + .` ×20, write with response | median 89 ms (61–120) |
| same, write without response | median 90 ms (90–150) |
| 51-byte line split into 3 chunks | `55 0 ok>`, stack clean |

Round trips include the console echo and the `ok>` prompt.

## On the glasses (2026-10-22, Spectacles '24, NusProbe.ts): PASS

| step | result |
|---|---|
| scan by name | found maplet, rssi -46, 765 ms |
| connect | 787 ms; `gatt.mtu` reads 0 |
| subscribe to TX | **70 ms, no bonding** |
| `n 2 + .` ×20, writeValue | median 100 ms (65–152) |
| same, writeValueWithoutResponse | median 149 ms (100–153) |
| 51-byte line in 20-byte chunks | `55 0 ok>`, stack clean |

The first run failed with `Scan failed: PERMISSION_DENIED`. The lens also
had the WebView package (InternetModule, open internet), and Spectacles
won't give one lens both open internet and Bluetooth. With the WebView
demos (`PolynodeDemo`) turned off, the scan went through. So SpecsSwitchbackXR
must be a Bluetooth-only lens: no WebView or InternetModule, unless the
glasses have Extended Permissions on, and lenses built that way can't be
published.

Use `writeValue` (with response): it was faster here than without response.

## How the probe was run

`Assets/ProjectScripts/ble/NusProbe.ts` runs the same steps on Spectacles.
Its setup steps are at the top of the file; the Bluetooth Central Module
asset is in `Assets/Bluetooth/`. To decide BLE vs WebSocket, it needs to show:

1. the scan finds `maplet` by name (the NUS UUID is only in the scan
   response, so the probe filters by name);
2. `registerNotifications` on TX returns, which is the step that hung on
   SkywriterBLE;
3. round-trip times are close to the Mac's.

Other things to check on the glasses:

- **One link at a time.** The firmware allows a single connection, so the
  phone app has to let go before the glasses can connect.
- **Advertising window.** The device only advertises during its window.
- **MTU.** If the MTU stays at 23, notifications come in 20-byte pieces. The
  probe reassembles them up to `ok>`.
