# NFCTapInXR

A Spectacles ('24) lens that turns NFC taps into things in the air. It started
as a copy of `../SpecsSwitchbackXR` and keeps that project's Bluetooth hub and
drawing helpers; the Switchback views (Pulse, Remote, Roll, Pin, Snap, Links)
are still in the scene but switched off.

The reader is `../../prototypes/NFCTapInXR_AtomMatrix_proto1`: an M5 ATOM
Matrix with a PN532, advertising as `NFCTapIn` and notifying `TAP <id hex>`
over the Nordic UART service, with no bonding.

## Scenes

| # | Scene | What happens |
|---|---|---|
| 0 | Trading card | An original creature card (Glimmerpup) turns to face you; pinch it to hold and move it |
| 1 | Sign-in | "Signing in as Iron Man": a spinner with three steps, then a check and "Session secured" |
| 2 | 3D model | Scan hoops sweep up while a model spins into existence |
| 3 | Fare | A transit pass slides in and its balance counts down by the $2.75 fare; when it can't pay, it tops itself up |

Under the stage are five pinch chips: one per scene, to play it without a tag,
and `RESET`, which forgets the learned tags and restores the balance to $20.00.

## Which tag plays which scene

Either set the four tag inputs on the `TapIn` object (`cardTag`, `signInTag`,
`modelTag`, `fareTag`) to tag IDs as the reader prints them, or leave them
empty: the first four different tags tapped are bound to scenes 0 to 3 in that
order and remembered between runs. A fifth, unknown tag is refused until
`RESET` is pinched.

The reader reads ISO14443A tags (MIFARE, NTAG). It cannot read ISO15693 tags
such as ST25TV.

## Scene setup

One object, `TapIn`, carries the `TapInXR` script. Its inputs:

| Input | Set to |
|---|---|
| `bluetoothModule` | the project's Bluetooth Central Module asset |
| `imageMaterial` | `Image` (unlit, base texture) |
| `textTemplate` | the Text on `TapIn/FontTemplate` (font source; hidden at start) |
| `camera` | `Camera Object` |
| `modelPrefab` | optional; a 3D model to materialize instead of the built-in crystal |

## Code

`Assets/ProjectScripts/tap/`:

- `TapLink.ts` finds the reader by name, subscribes and turns notifications
  into tap and bad-read callbacks. It retries after a drop.
- `TapInXR.ts` builds the stage and the four scenes and maps tags to scenes.
- `Paint.ts` paints the cards, rings and the creature into procedural
  textures, so the scenes need no image assets.

## Running it

In the editor preview there is no Bluetooth, so the lens plays the four scenes
in turn every 5 seconds (`previewDemo`). On the glasses it searches for
`NFCTapIn`; the caption under the stage shows the link state and each tap.

Keep the lens Bluetooth-only: with the WebView demos enabled the scan fails
with `PERMISSION_DENIED` (see `../SpecsSwitchbackXR/BLE-NOTES.md`).

## Status (2026-11-21)

Compiles and runs in the Lens Studio preview; all four scenes checked there by
eye. Not yet run on the glasses, so the Bluetooth link to the reader, pinch
holding of the card, and sizes at real scale are untested.
