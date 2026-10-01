# Switchback — the OpenWR app (design proposal)

**Status:** proposal, 2026-10-01. Nothing built. The full proposal with phone
mockups of the three experiences is the published page linked from the
project README; this file is the record in the repo.
**Starts from:** `ProjectMagNET/magnet_app` (Flutter 3.24, flutter_blue_plus,
provider, go_router, EN + JA), surveyed 2026-10-01.
**Talks to:** goggl1 over the Nordic UART console
(`DESIGN-provisioning-ota.md`).

## Brand

OpenWR: Open. Weird. Wild. Strong. #PNWbuilt. Palette: Fog `#F3F4F1`,
Basalt `#1B1F1D`, Evergreen `#1F4D3A`, River `#2E6F8E`, Alpenglow `#E0683F`,
plus Headlamp amber `#F2A33A` and Hi-vis `#FFD23F` for the night and storm
experiences. Type: Bricolage Grotesque (display), Atkinson Hyperlegible
(body), IBM Plex Mono (data). Voice: second person, present tense, short.
Mark: a two-stroke switchback in a rounded square, legible at 16 px and as
an 8×8 glyph on the glass.

App name: **Switchback** (a trail term: where the path turns and you
choose). Alternates: Treeline, Cairn.

## Three experiences

The app's whole look follows the conditions the goggle reports, with a pin
to override. Squall wins over Headlamp when both apply.

| Experience | When | Look | What it leads with |
|---|---|---|---|
| **Dawn Patrol** | day | Fog ground, Evergreen actions | one big temperature and a sentence answering "can I go?" |
| **Headlamp** | `is_day` = 0 | true black, amber text, red-shift toggle, fewer and larger buttons | the overnight low, a brightness control that dims the glass too |
| **Squall** | code ≥ 95, or rain 61–67 with gusts ≥ 40, or rain probability ≥ 70 % within 3 h | slate ground, hi-vis yellow, rain blue, glove-sized targets | a warning bar with rain start and gusts, and a turn-back time |

## Screens

Trailhead (home), Pair (long press, scan, connect; first-run WiFi /
location / units cards), Looks (glass theme picker with live previews,
animation, brightness), Session (the sports half: phone streams time, pace,
distance, heart rate to the glass as Forth lines), Console (the raw `ok>`
prompt), Update (OTA served from the phone; bundle push).

Session is the second use-case bundle, `session-hud`, installed when a
session starts and swapped back to the weather bundle when it ends. No
firmware change.

## Device contract

The app never gets a protocol; every screen is a form over console lines:
`status`, `wifi!`, `geo!`, `units!`, `theme!`, `anim-on !`, `oled-contrast`,
`bundle-begin … bundle-end`, `ota!`, `version`, `sess!`.

## From magnet_app

Keep: shell, router, provider setup, permission onboarding, BLE scanner
(filter `goggl1`), radar view (as the find-your-goggle aid), WiFi scan for
the SSID picker, theme controller's font scale / high contrast / reduce
motion, diagnostics log, About, EN + JA, scanner tests, pre-commit hook.

Replace: `lib/magnet/` (HCP protocol over the MagNET GATT service) with
`lib/goggle/` (NUS transport, console, status parser); `lib/theme/tokens.dart`
six dark presets with three experience themes; `magnet_node_screen`,
`mesh_screen`, `mesh_test_tab` with Pair/Setup and Console.

Remove: credentials / operator identity (PBKDF2, pointycastle), mesh
session, provision log, mDNS relay proxy, starter-app image leftovers.

Rename: `magnet_app` → `switchback`; `io.iotone.magnet` → `io.iotone.switchback`
(or `org.openwr.switchback`); display names MagNET → Switchback in Android,
iOS, and both `.arb` files; purge "Meshmore SNS" / "IoTone Japan" /
"Responsive IoT 2026" from macOS, Linux, Windows, web (or drop those
targets except macOS); new icon and splash on Fog.

Where: copy into OpenWR as `projects/apps/switchback/` at the start of
phase A.

## Phasing

A rebrand and themes ✓ · B console, pairing, status, experience auto-select ✓ ·
C setup and looks ✓ · D update (phone-hosted `.mnpkg`, see DESIGN-provisioning-ota.md §4) ✓ ·
E session ✓ 2026-10-02 (as a mode in the weather bundle, `15-session.fs`, not a
second bundle: one word to switch instead of an 8 KB push; heart-rate strap still open).

## Open questions

Name; platform order (Android first for BLE and the on-phone HTTP server,
iOS for the first users); heart-rate strap in phase E; keep Japanese
(proposed yes); the glass cannot red-shift, only dim.
