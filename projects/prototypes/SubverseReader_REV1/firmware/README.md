# SubverseReader firmware

## sr15: SR-15 Dial Pass on the Elecrow CrowPanel ESP32 4.2in

ESP-IDF 5.3 via PlatformIO (same pins as maplet), ESPIDFORTH console on USB-C (CH340, 115200) and
Bluetooth (NUS, advertises as `SR-15`).

```
cd sr15 && pio run -t upload        # 460800; the CH340 drops bytes faster than that
```

- Reads `.svpk` packs from `/packs` on the microSD card (built by `../tools/svpk.py`, or sent by
  `../tools/sr_push.py` over USB, or by the phone app over Bluetooth).
- Shelf: dial right/left moves, push opens. Page: dial right/push next, left previous, MENU page
  number, EXIT shelf; hold MENU first page, hold EXIT sleep. Hold MENU + EXIT: 3-minute Bluetooth
  sync window.
- Deep-sleeps after `sr-sleep!` seconds idle (0 = never); any control wakes it and acts.
- **Two board revisions, two panel controllers.** Boards with a green round sticker on the back
  use an UltraChip UC81xx, not the SSD1683 in Elecrow's main examples, and need GPIO41 high.
  `epd.c` detects which by the BUSY level after reset; `epd-ctl!` forces one.
- Console words: `sr-info`, `sr-test`, `epd-rot!`, `sr-open`, `sr-page`, `sd-recv`, `sd-rm`, …
  (list in `src/words.c`).

`backup/` (git-ignored) holds the factory image read off the bench board; Elecrow publishes it
under no licence, so it stays local. Restore with `esptool.py write_flash 0 <file>`.
