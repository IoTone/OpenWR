# SpecsSwitchbackXR — Switchback on Spectacles (design proposal)

**Status:** proposal, 2026-10-22. The BLE link is proven on the glasses
(`BLE-NOTES.md`). The full page with lens mockups is
`docs/specs-switchbackxr.html` (published as a private artifact); this file
is the record in the repo. goggl1 frames are rendered by
`tools/goggle_frames.py` into `docs/images/`.

## Roles

| Device | Job | Holds |
|---|---|---|
| Phone (Switchback) | Basecamp: GPS, weather, tiles, plans, 1-bit map renders | sessions, pins, rolls |
| goggl1 | Glance: heart rate, pin arrow, minimap, photo; the hub both links meet at | session pins, current position, up to ~100 1 KB frames in the 128 KB of flash past the OTA slots |
| Spectacles | Lookout: Pulse, pin board, roll, remote; reads the strap directly | nothing durable |
| maplet (S3 Sense) | the camera and the microSD; its roll is what the glasses browse | rolls |

goggl1 has no card slot; the microSD is on the maplet. A goggle with
microSD is a later hardware revision.

## Constraints that shape it

- A lens that uses Bluetooth gets no open internet (seen: `PERMISSION_DENIED`
  with WebView in the project). Per Snap's docs it also loses the camera
  frame, location and the microphone; SkywriterBLE's `GPS_RAW` denial fits.
  To confirm with a camera probe. Extended Permissions lifts this on a
  developer's glasses only; such lenses cannot be published.
- So position comes from the phone through goggl1, and in a shipping lens
  pictures come from the maplet, not the glasses' camera.
- Drawing uses SpaceSVG (meshes), never WebView.
- goggl1 allows one connection today. Proposal: NimBLE at two connections,
  each received line runs whole with its output sent back to the link that
  sent it; screen and session updates go to both.

## Glass language

Additive display: black is clear glass, dark colours vanish, nothing sits on
a fill. Brand colours become light: Fog `#F3F4F1`, Moss `#7FC8A3` (for
Evergreen), Glacier `#78B7D4` (for River), Alpenglow `#F08A62`, Headlamp
`#F2A33A`, Hi-vis `#FFD23F`. Same type as the app. Hairlines, not panels.
The goggle's frames appear at true 128×64 inside the glasses.

## Views

1. **Pulse:** head-locked low left; bpm, zone as five arcs (60/70/80/90 % of
   max), time, cadence; the goggle's current frame mirrored.
2. **Pin board:** the phone's contour map as a tilted plate, track solid,
   plan dashed; pins also stand in the world at their bearing with distance.
   Pin now, or every 1/5/15 min.
3. **Roll:** the maplet's roll as a 1-bit contact sheet; send a frame to
   goggl1 or pin it where it was taken.
4. **Remote:** the goggle's screens as true-pixel cards; pinch sends `view!`.

## goggl1 contract additions

```
here!      ( lat5 lon5 cog -- )         position from the phone (1e-5 degrees)
pin!       ( id lat5 lon5 kind -- )     0 waypoint, 1 destination, 2 photo, 3 breadcrumb
pin-name!  ( id c-addr u -- )   pin-go ( id -- )   pin-del ( id -- )   pins-clear   pins.
sess-hr!   ( bpm zone cad kind -- )     from the glasses
img-begin ( slot -- )  img-hex! ( c-addr u -- )  img-end ( -- flag )   128x64, 1 024 B, 26 lines
map-geo!   ( lat5 lon5 lat5 lon5 -- )   corners of the map image; goggl1 draws you and pins live
view!      ( n -- )                     0 weather, 1 session, 2 pulse, 3 pin, 4 map, 5 photo
```

New screens (pixel-true renders in `docs/images/goggle-*.png`): Pulse, Pin,
Map, Photo.

## Phasing

| Phase | Delivers |
|---|---|
| G0 Probes | BLE without bonding (done); strap from the glasses (`HrProbe.ts`); two links at once |
| G1 Pulse | Pulse on the glasses from the strap; `sess-hr!` and Pulse on goggl1 (bundle 0.8) |
| G2 Two links | goggl1 at two connections; phone sends `here!` |
| G3 Pins | pin words, pin board, arrow screen, pin clock |
| G4 Pictures | `img` words, map screen with live pins, roll view |
| G5 Experimental | glasses camera to goggl1 under Extended Permissions |
