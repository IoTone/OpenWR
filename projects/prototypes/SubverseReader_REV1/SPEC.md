# SubverseReader REV1 — spec

A wearable e-paper reader the size of a backstage pass. You sync zines, ebooks
and PDFs from the Subverse website with a phone app, then read on the pass
with the phone off, for days or weeks per charge.

Open hardware for manufacturing, in the same spirit as Hiroglass REV1: parts
from the usual sources, buildable with homebrew equipment, every layer of the
software ours and open.

## Requirements

- **Form:** backstage pass / laminate. Worn on a neck lanyard or a belt clip.
- **Display:** e-paper, big enough to read on (not maplet's strip). No touch.
- **Compute:** ESP32-C5 or ESP32-S3.
- **Power:** LiPo with a USB charging circuit. Radios off while reading; target
  days to weeks per charge. Auto-off with no power switch: every key turns the pass
  back on. Thin / stock / extended cell option per variant, with estimated battery life.
- **Input:** physical buttons and some way to navigate (keys, 5-way, wheel).
- **Phone:** a mobile app pulls zines, ebooks and PDFs from the Subverse
  website and pushes them to the reader. The phone does not need to be on to read.
- **Price:** $50 or less per unit (the cheap e-paper readers on the market sit
  around $99).
- **Radios:** ESP32-C5 preferred for dual-band Wi-Fi, so sync works whether a
  network is 2.4 or 5 GHz and users never have to know the difference. At least
  one variant uses an nRF52 (BLE only, no Wi-Fi).
- **Storage and loading:** say per variant whether there is a microSD slot.
  USB-C on every unit; a no-phone mass loader (USB drive where the chip has
  USB OTG, otherwise microSD or a USB-serial loader).
- **Display options:** at least one variant with a front light ("backlight")
  and one with a 4-colour panel.
- **NFC:** one variant reads NFC tags holding zines (stored on the tag) or links to them on the
  Subverse site; a Wi-Fi build and a phone-tethered build.
- **Pass sizes:** some options must match stock backstage-pass sizes under 4 in wide
  (3 × 4 in, 3.5 × 2 in / 85 × 55 mm, 80 × 145 mm, 3.69 × 8.49 in).
- **Enclosure:** required. At least one design has a back pocket for paper or
  credit-card-sized things.
- **Deliverables for this round:** fourteen priced variants (incl. front light, 4-colour, nRF52840 no Wi-Fi, analog thumbstick, four built to stock pass sizes under 4 in wide, and an NFC tap-to-collect pass in Wi-Fi and phone-tethered builds) with BOMs, an
  enclosure for each, a Blender render of each, and software ideation.

## Where things are

| Path | What |
|---|---|
| `docs/DESIGN-SubverseReader.md` | software ideation, sync, firmware, variants, power |
| `bom/` | `make_bom.py` and the generated BOMs and distributor import files |
| `render/make_renders.py` | parametric Blender models (SR-1 to SR-9); `render/out/SR-*.png` |
| `power/` | `battery_model.py`: battery life per variant and cell option (README.md, life.json) |
| `bom/AVAILABILITY.md` | parts availability, checked 2026-11-07 |
| `render/viewer/` | three.js 3D review of all fourteen, published as the design page |
