# SubverseReader (phone app)

Browse a Subverse catalog (OPDS 2.0; default `https://spicemine.iotj.cc/opds`), render zines on the
phone for an e-paper pass, and send them to it over Bluetooth. `io.iotone.subversereader`, Android
first. The pass side is `projects/prototypes/SubverseReader_REV1` (firmware for SR-15 and SR-16 in `firmware/pass`).

- **Subverse tab:** the catalog. Tap a zine to render it for the pass's panel (SR-15 300×400 or
  SR-16 240×416, 1-bit) and preview the exact pages.
- **My pass tab:** find a pass (it advertises for 3 minutes after power-up, or after holding
  MENU + EXIT), connect, see the zines on its card, open or remove one.
- **Send:** packs (`.svpk` v0, same as `tools/svpk.py`) go over the Nordic UART service with the
  firmware's `sd-recv` and a CRC; about 30 KB/s.

Code: `lib/catalog/opds.dart` (feed), `lib/pack/` (render + pack format, tested in
`test/svpk_test.dart`), `lib/pass/pass_link.dart` (Bluetooth), `lib/theme/` (looks).

**Looks.** `lib/theme/brand.dart` is the interface; `punk.dart` is direction 5, Xerox Punk, from the
brand board. Add a direction as another `Brand` and list it in `kBrands`. Fonts are bundled in
`assets/fonts` with their licences (OFL / Apache 2.0).

**Dependencies to keep pinned.** `flutter_blue_plus` stays on 1.x (BSD): 2.x requires a paid
licence for any company use. `permission_handler` ^11 (13 needs a newer Kotlin plugin).

```
flutter test
flutter build apk --profile && adb install -r build/app/outputs/flutter-apk/app-profile.apk
```
