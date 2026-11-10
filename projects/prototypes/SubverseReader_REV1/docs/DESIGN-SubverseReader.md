# SubverseReader — design (REV1 ideation)

**Status:** ideation, 2026-11-06. Nothing built. Sixteen hardware variants (SR-14 in two builds; SR-1C and SR-1D are
alternative SR-1 builds) are
priced (`../bom/README.md`) and rendered (`../render/out/`); this document is
the software plan they share.
**Lineage:** maplet (the phone renders, the e-paper only shows a finished
bitmap), Switchback (Flutter app, BLE transport, device pairing), goggl1
(ESPIDFORTH console over BLE), Hiroglass REV1 (open hardware BOM method).

## 1. What it is

A laminate on a lanyard that happens to be a book. When you are not reading,
it is a pass: the screen holds the zine's cover or your own badge art with no
power at all. When you pick it up, a key press turns the page.

The phone is for getting things onto it. You browse Subverse in the app, pick
zines and books, and the app sends them. Then the phone can be off, in a
locker, or at home. Radios on the pass are off unless you ask for a sync.

## 2. Principles

1. **The phone renders, the pass blits.** Same rule as maplet. EPUB layout,
   PDF rasterising, dithering, fonts and hyphenation all happen on the phone,
   at the exact panel resolution. The pass stores finished pages and draws
   them. Firmware stays small, page turns stay fast, and every variant runs
   the same firmware with a different panel driver.
2. **Radios off by default.** Sync is a deliberate act (hold two keys, or
   choose Sync in the menu). Nothing listens in the background.
3. **The screen is never blank.** Sleep shows the badge or the cover. E-paper
   holds it for free.
4. **Open all the way.** Hardware under CERN-OHL-S, firmware and app under an
   open licence, an open pack format anyone can write.

## 3. The system

```
 Subverse website ── catalog feed (OPDS 2.0) ──▶ phone app ──▶ pass
   zines (PDF)                                    ├ catalog / library
   ebooks (EPUB)                                  ├ renderer (per panel profile)
   covers, metadata                               ├ pack builder (.svpk)
                                                  └ sync: BLE control, Wi-Fi bulk
                                                         ▲            │
                                    reading positions ───┘            ▼
                                                              flash / microSD
                                                              page cache → panel
```

## 4. Getting content from Subverse

I could not find a public "Subverse" site or API, so this needs your answer
(§12). The proposal: the site publishes an **OPDS 2.0** feed (JSON). It is the
open standard ebook apps use for catalogs. It carries titles, authors,
covers, acquisition links for PDF and EPUB, and groups (issues, series, new
this week), and Calibre, Thorium and KOReader already speak it, so Subverse
content reaches other open readers as well.

If the site cannot publish a feed yet, the app can read a plain JSON index
(`/reader/index.json`: id, title, cover URL, file URL, sha256, updated) and
move to OPDS later. The app also accepts files shared to it (Android share
sheet: PDF/EPUB) so people can carry their own zines.

Rights: zines are usually free to copy, and many carry CC licences. The pack
records the licence and source URL so the pass can show it on the colophon page.

## 5. The renderer (on the phone)

Each pass reports a **panel profile** at pairing: width, height, grey depth
(1, 2 or 4 bit), refresh modes, and orientation. Pages are rendered for that
profile only.

**PDF zines** (most of what Subverse will have):
- Rasterise with pdfium (Flutter: `pdfrx`) at panel resolution.
- **Zine-aware layout.** Detect two-up spreads and imposed print sheets and
  split them into reading order. Auto-crop margins. Optional *column mode*:
  find text columns or comic panels and give each its own screen, so an
  A5 zine reads well on a 4.2in panel.
- **Dither per region.** Threshold with a slight gamma for text and line art.
  Photos get Floyd–Steinberg on 1-bit panels and ordered dither or plain
  16-grey on SR-3 and SR-4. maplet's tuning (FS gain 1.4, ordered for photos)
  is the starting point.

**EPUB books:**
- Reflow on the phone with a small layout engine (or a WebView render to a
  bitmap at panel size) using the reader's font and size, with hyphenation.
- Changing font size means re-rendering and re-syncing that book. Over Wi-Fi
  that takes seconds; the app can also ship two sizes of a book in one pack.

**Every pack** also gets a cover page, a badge page (the cover or the user's
own pass art), a table of contents page, and a colophon page (source, licence,
date synced).

## 6. The pack format (`.svpk`)

One file per title, written by the app and read by the firmware with no parsing
beyond a table lookup.

| Part | Contents |
|---|---|
| header | magic `SVPK`, version, panel profile it was rendered for, page count, title, sha256 of source |
| page table | per page: offset, length, compression, refresh hint (text / image) |
| TOC | chapter names → page numbers (already rendered as a page too) |
| pages | 1/2/4 bpp bitmaps, run-length + heatshrink compressed |

Sizes, measured as expected rather than tested: a 1-bit 648×480 text page is
38.9 KB raw and roughly 5–10 KB compressed; a 1448×1072 4-bit page is 776 KB
raw and roughly 100–200 KB. A 40-page zine for SR-2 is about 300 KB, and
16 MB of flash holds about 40 of them. SR-4 wants the microSD card.

Positions, bookmarks and "finished" flags live in a small journal file on the
pass and go back to the phone at sync.

## 7. Sync and loading

Three ways in, one pack format. Every pass has USB-C.

- **BLE (Nordic UART service, same as goggl1/maplet):** pairing, status, small
  zines, positions. Realistic throughput is 50–90 KB/s with 2M PHY and data
  length extension (less on the nRF52840 when the phone's connection interval
  is long), so a 300 KB zine takes a few seconds.
- **Wi-Fi for bulk (ESP32 variants):** the app asks over BLE for Wi-Fi; the pass
  starts a soft AP with a one-time password; the app joins it *app-scoped*
  (Android `WifiNetworkSpecifier`, iOS `NEHotspotConfiguration`), so the phone
  keeps its cellular data; the pass pulls packs over HTTP with resume, at
  1–3 MB/s.
- **USB-C mass loading, no phone:** see §7.2.

Session: `HELLO` (firmware, panel profile, free space, battery) → shelf diff
(what is there by sha256, what should be) → transfer packs in chunks with CRC
→ pull back the journal → `BYE`, radios off. The app shows the pass's shelf
as the source of truth.

### 7.1 Why the ESP32-C5: nobody should have to know what 2.4 vs 5 GHz means

People don't know which band their router or hotspot is on, and the failures
look random: an ESP32-S3 (2.4 GHz only) cannot see a 5 GHz-only SSID, a
phone hotspot set to 5 GHz is invisible to it, and a festival's 2.4 GHz band is
too crowded to move a book. The C5 is dual-band Wi-Fi 6, so the pass just works
on either band. The firmware and app hide the question entirely:

1. **Default: the pass hosts.** The pass's own soft AP, joined by the app with no
   user input. On the C5 it comes up on 5 GHz (UNII-1, channels 36–48, no DFS
   wait) and falls back to 2.4 GHz channel 1/6/11, whichever is quieter in a
   quick scan. The user never sees an SSID.
2. **Optional: join a known network** (home Wi-Fi, for later "sync while
   charging"). The app hands over the SSID and password over BLE; the C5 finds
   it on whatever band the router uses. On S3 variants the app warns before
   sending a network the pass cannot see (5 GHz-only), in plain words.
3. **If Wi-Fi fails, BLE still works**, just slower. Sync never dead-ends.

### 7.2 Storage and the USB-C loader

**microSD is the flexible answer** and is on every variant except SR-1 and SR-1C
(books live in flash). A card holds thousands of packs, survives a dead board,
can be loaded in any computer, and lets one pass carry a whole library.

What USB-C can do depends on the chip, and this is the catch with the C5:

| Chip | USB on the chip | Mass loading over USB-C |
|---|---|---|
| ESP32-S3 (SR-1, SR-2, SR-3, SR-4, SR-7) | USB OTG (full speed) + Serial/JTAG | **USB drive**: TinyUSB MSC exposes the microSD card (or a FAT partition in flash). Drag packs on, eject, read. |
| nRF52840 (SR-8) | native USB device | **USB drive** (TinyUSB MSC) + **UF2 bootloader**: firmware updates by dragging a file. |
| ESP32-C5 (SR-1C, SR-5, SR-6) | Serial/JTAG only, no OTG | No USB drive. Two loaders instead: **pull the microSD card** and copy packs on a computer, or the **Web Serial loader**: a Subverse web page (Chrome/Edge) streams packs to the pass over the USB serial port. Expect a few hundred KB/s; fine for zines. |

The desktop side is a **pack builder** (the phone renderer in a desktop or web
build) so people without the app can turn PDFs into `.svpk` files for a
specific pass profile. On a USB drive the pass also accepts a plain `inbox/`
folder of PNG pages as a crude pack, for hackers.

Drive mode rule: while mounted as a USB drive the pass does not read the card
itself (one writer at a time). On eject it re-scans the shelf and shows what
arrived.

## 8. Firmware (on the pass)

ESP-IDF in C, with the ESPIDFORTH console kept for bring-up and scripting,
as in goggl1 and maplet. SR-8 (nRF52840) runs the same reader core on Zephyr
(nRF Connect SDK) behind a thin platform layer: panel SPI, flash/SD, keys, BLE,
USB, sleep. The pack format and page logic are shared C. The maplet run-time panel driver table (`epd-driver!`)
carries over: SSD1683 (4.2in), UC8179 (5.83in), and an epdiy back end for the
parallel panels (ED047TC1, ED060KC1).

States:

| State | Radios | What happens |
|---|---|---|
| **Sleep** | off | deep sleep; the screen shows a page or the badge; keys wake it |
| **Page** | off | wake on key → read page from pack → partial refresh → sleep in < 1 s |
| **Menu** | off | shelf, TOC, go to page, settings, badge picker |
| **Sync** | BLE, then Wi-Fi if asked | only after Sync is chosen; 3-minute timeout |
| **Charge** | off | USB in: show charge state; USB serial console for dev |

Refresh: partial refresh for page turns, full refresh every N pages (default
8, settable) and whenever an image page appears, to clear ghosting.

Navigation is the same model on every variant, mapped to its keys:

| Action | SR-1, SR-8 | SR-2, SR-7 | SR-3 | SR-4 | SR-5 | SR-6 | SR-9 |
|---|---|---|---|---|---|---|---|
| next / previous page | NEXT / PREV | NEXT / PREV or 5-way ◀▶ | side DOWN / UP | either side rocker | wheel | NEXT / PREV | flick stick ◀▶ |
| open menu / select | MENU | 5-way press | side OK | MENU | OK / wheel press | MENU | OK |
| back | long MENU | 5-way ▲ | long OK | BACK | BACK | long MENU | BACK |
| zoom / pan | | | | | | | hold OK + stick: zoom; stick: pan |
| sync | hold PREV+NEXT 2 s | hold PREV+NEXT | hold UP+DOWN | SYNC | hold BACK+OK | hold PREV+NEXT | hold OK+BACK |
| front light | | | | | | LIGHT: on / level; hold: off | |
| badge / sleep now | hold MENU 2 s | hold 5-way | hold OK | hold MENU | hold OK | hold MENU | hold BACK |

On SR-7 (4-colour) page turns are full refreshes of about 12 s, so the
firmware shows text pages in black and white with the fastest waveform the
panel allows and keeps colour for covers, image pages and the badge. That
variant is for passes and posters first, reading second.

## 9. The app

Start from Switchback's Flutter code base (BLE transport, pairing, Android
first), but as its own app: the audience is different. Suggested id
`io.iotone.subversereader`, following the `io.iotone.*` rule for branded apps
(not yet decided).

Screens:
1. **Subverse:** the catalog feed, issues, new this week, search.
2. **Library:** what is downloaded on the phone, plus files shared in.
3. **Pass:** the shelf on the pass, free space, battery, last sync. Add and
   remove titles; the change is queued until the next sync.
4. **Preview:** the exact pages the pass will show, rendered with its panel
   profile, so zine layout choices (split spreads, column mode, dither) are
   made by looking.
5. **Badge:** pick the sleep image: a zine cover, a photo dithered for the
   panel, or a pass template with a name and a role.

## 10. Hardware variants

All nine are rendered front and back in `../render/out/`. BOM detail is in
`../bom/README.md` (regenerate with `make_bom.py`). Dimensions are the
enclosure as rendered; weights are estimates without the lanyard.

| Variant | Wear | Panel | Chip | Navigation | Back | W × H × D (mm) | ~g | microSD | USB-C | @5 | @100 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| **SR-1 Laminate** | lanyard | 4.2in 400×300 1-bit | XIAO ESP32-S3 Plus on Seeed **EE05** (2.4 GHz) | 3 keys (on the EE05) | plain | 86 × 138 × 12.5 | 95 | no | USB drive | $40 | $32 |
| **SR-1C Laminate C5** | lanyard | 4.2in 400×300 1-bit | DFRobot FireBeetle 2 ESP32-C5 (2.4 + 5 GHz) + Crystalfontz CFA-10084 adapter | 3 keys | plain | 86 × 138 × 13 | 100 | no | charge, Web Serial | $43 | $34 |
| **SR-1D Laminate Direct** | lanyard | 4.2in 400×300 1-bit | XIAO ESP32-C5 + Seeed driver board, **all from China, shipped to Japan** | 3 keys | plain | 86 × 138 × 12.5 | 95 | no | charge, Web Serial | $42 | $25 |
| **SR-2 All Access** | lanyard | 5.83in 648×480 1-bit | ESP32-S3 PCB | 5-way + 2 keys | card pocket | 109 × 177 × 19.5 | 150 | yes | USB drive | $55 | $34 |
| **SR-3 Gray Pass** ([build guide](Gray-Pass-SR3-build-guide.md)) | lanyard | 4.7in 960×540 16-grey | M5Stack PaperDIY (S3) | 3 side keys | plain | 78 × 149 × 14.5 | 110 | yes | USB drive | $37 | $27 |
| **SR-4 Headliner** | lanyard | 6in 1448×1072 300 ppi 16-grey | ESP32-S3 + epdiy PMIC | rockers both edges + 3 keys | card pocket | 109 × 172 × 17.5 | 150 | yes | USB drive | $68 | $41 |
| **SR-5 Crew Clip** | belt clip | 4.2in 400×300 landscape | ESP32-C5 PCB | thumbwheel + 2 keys | spring clip | 104 × 109 × 21.5 | 110 | yes | charge, Web Serial | $40 | $23 |
| **SR-6 Night Shift** | lanyard | 4.2in 400×300 **front light** | ESP32-C5 PCB | 4 keys incl. LIGHT | plain | 88 × 138 × 14.5 | 105 | yes | charge, Web Serial | $49 | $30 |
| **SR-7 Poster** | lanyard | 5.83in 648×480 **4-colour** B/W/R/Y | ESP32-S3 (SR-2 board) | 5-way + 2 keys | A7 zine sleeve | 109 × 177 × 19.5 | 150 | yes | USB drive | $55 | $35 |
| **SR-8 Quiet Pass** | lanyard | 4.2in 400×300 1-bit | XIAO nRF52840 Plus on Seeed **EE05**, **no Wi-Fi** | 3 keys (on the EE05) | plain | 86 × 138 × 13 | 95 | yes | USB drive + UF2 | $47 | $35 |
| **SR-9 Roadie** | lanyard | 4.2in 400×300 1-bit | ESP32-C5 PCB | **PSP thumbstick** (Adafruit 444) + OK / BACK | plain | 88 × 149 × 14.5 | 105 | yes | charge, Web Serial | $42 | $25 |
| **SR-15 Dial Pass** | lanyard | 4.2in 400×300 1-bit | Elecrow CrowPanel 4.2in (ESP32-S3), finished board | **thumb dial** (up / down / push) + MENU + EXIT | plain | 95 × 140 × 16 | 150 | yes | charge, Web Serial (CH340) | $39 | $27 |
| **SR-16 Setlist** | lanyard | **3.7in 240×416 portrait** 1-bit, ~130 dpi | XIAO ESP32-C5 on Seeed's XIAO driver board | 3 keys (hand-wired) | plain | 64 × 112 × 12 | 70 | no | charge, Web Serial | $37 | $24 |

Depth with pocket/clip included; SR-2/SR-7 are 15.5 mm without the pocket,
SR-4 13.5 mm, SR-5's body 16 mm.

On "backlight": e-paper is reflective and cannot be backlit. SR-6 uses a
**front light**, a light guide laminated over the glass with LEDs on one edge,
which is what Kindle-class readers do. Good Display sells the 4.2in panel with
it fitted (GDEY042T81-FL02).

Ordering: `../bom/order.py` writes `../bom/order/ORDER.md`, the per-store shopping list,
landed cost and parts-in-hand date for each variant (cells, lanyards and clips come
from Amazon or a hobby shop for prototypes, Alibaba for 100).

Recommended path: build **SR-1** first (ready-made boards only, proves firmware and
pack format), then **SR-2** as the product candidate, with **SR-7** on the same
board. SR-3 on the M5Stack kit is the cheap 16-grey dev kit; to build one now (on the
LilyGo T5 4.7in S3 while the kit is sold out), follow
[Gray-Pass-SR3-build-guide.md](Gray-Pass-SR3-build-guide.md). SR-8 is SR-1 with
the nRF52840 swapped in and should be built alongside it, so the platform layer
is honest from day one.

**SR-1 and SR-8 on the EE05 (2026-11-09).** Seeed's XIAO ePaper Display Board EE05 ($10.99)
carries the 24-pin panel driver, three keys (D1, D2, D9), a JST 2.0 battery socket with a
slide switch, and load switches that cut the panel rail and the battery divider in sleep
(schematic V1.0). It replaces the driver board, hand-wired keys, diodes and switch. Two
catches: the XIAO is soldered flat using its back pads (panel reset is on D11), so it takes
hot air or reflow, and only the **S3 Plus** and **nRF52840 Plus** fit; the C5 and C3 XIAOs have
no back pads. So SR-1 trades the C5's 5 GHz for the S3's USB drive. **SR-1C** keeps the C5:
a DFRobot FireBeetle 2 ESP32-C5 ($7.90, DigiKey stock) with Crystalfontz's CFA-10084 adapter
(US stock; RESE jumper left open for 3 Ω, the nearest to the panel datasheet's 2.2 Ω; Good
Display's DESPI-C02 has 2.2 Ω but ships from China and was out of stock on 2026-11-09) and three keys on LP GPIO3/4/5, each of which wakes it directly. At 5 units, SR-1 lands at
$49.24 a pass and SR-1C at $52.85, both with all parts in hand around Nov 18–19.
**SR-1D** is the cheapest SR-1, bought only from China and priced for delivery to Japan:
one Seeed parcel (XIAO ESP32-C5, ePaper Driver Board 114993558 and Seeed's 4.2in panel,
which is the same GDEY042T81 glass) and one AliExpress order (keys, Schottky diodes, slide
switch, cell, screws, lanyard). $44.19 a pass landed for 5, $25 at 100. The keys share an
any-key wake line on the C5's back JTAG pad (GPIO3), because the driver board takes D0 and D1.
LiPo cells often can't travel by post to Japan, so buy the cell with a courier option or locally.

### SR-14 Tap Pass: zines on NFC tags

Tags go where zines already live: posters, flyers, record sleeves, the merch table, the bathroom
door. Tap the back of the pass on one and the zine behind it goes on the shelf.

**What a tag holds**

| On the tag | Tag | What the pass does |
|---|---|---|
| Link to the zine on Subverse (NDEF URI, ~40 B) | NTAG213 and up (~$0.10–0.20 each) | Queues it, then fetches it (Wi-Fi build) or hands it to the phone (tethered build) |
| A micro-zine stored in the tag | NTAG216 888 B: a poem, setlist, manifesto. ST25DV64K 8 KB: several pages of text or one small 1-bit image | Shows it at once, offline. NDEF MIME record `application/vnd.subverse.micro` holding compressed plain text with a tiny header (title, author, licence, source URL); laid out on the pass with its built-in font |
| Both | NTAG216 and up | First page now, full issue at the next fetch |

**Two builds, one 80 × 155 × 12.5 mm enclosure** (3.97in 800×480 panel, TAP key on the front,
UP/OK/DOWN on the edge, reader coil behind a printed tap target on the back):

| | SR-14W Wi-Fi | SR-14B tethered |
|---|---|---|
| Chip | ESP32-C5 | nRF52840 (Raytac MDBT50Q) |
| A tapped link | fetched by the pass over Wi-Fi (known network or the phone's hotspot, either band); BLE to the phone if no Wi-Fi | sent over BLE to the phone; the app downloads, renders and sends pages back |
| Pass as a tag | ST25DV04K dynamic tag | the nRF52840's own NFC-A tag peripheral, coil only |
| USB-C | charge, console, Web Serial | USB drive, UF2 |
| Per unit @5 / @100 | $52.28 / $31.92 | $53.78 / $33.28 |

- **Reader:** NXP MFRC522 (ISO14443A: NTAG21x, NTAG I2C, Mifare Ultralight), PCB coil
  ~40 × 25 mm with a ferrite sheet between it and the cell. Powered by a MOSFET only for 10 s after
  TAP, so it costs nothing in sleep. PN532 is the alternative if card emulation or
  peer-to-peer is wanted from the reader itself.
- **The Wi-Fi fetch needs server-side packs.** The pass cannot render PDFs, so a link must resolve to
  a pack rendered for the pass's panel profile: `…/zine/07?profile=gdem0397-1bit` → `.svpk`. Either
  Subverse serves these, or we publish an open pack-render service (the phone renderer as a
  container) that caches per profile. Until one exists, the Wi-Fi build behaves like the tethered one.
- **The pass is a tag too.** A phone tapping the pass gets the link to the zine on screen (or the
  user's badge page), and NFC handover pairs BLE with no menu. Two passes trade zines by tapping
  one onto the other: A's reader reads B's dynamic tag.
- **Trust and privacy.** Links are followed only on Subverse or domains the user has approved;
  anything else is shown as a link, not fetched. Tags carry no user data and the pass sends
  nothing when it reads one. Authentic Subverse tags can use NTAG 424 DNA (SUN signed URLs) later.
- Tag costs live outside the pass BOM; a starter pack of 10 NTAG215 + 2 NTAG216 is about $2.40.

### Stock pass sizes (SR-10 to SR-13)

Backstage passes come in stock sizes: 3 × 4 in, 3.5 × 2 in, 85 × 55 mm, 80 × 145 mm,
3.69 × 8.49 in, and the wider 4.25 × 5.5 in, 4 × 6 in, 105 × 148 mm. Four variants are built to the
exact outline of the ones under 4 in wide. They exist only in the 3D review
(`../render/viewer/`), not as Blender renders.

| Stock size | Variant | Outline (mm) | Panel | Chip | Controls | microSD | @5 | @100 |
|---|---|---|---|---|---|---|---|---|
| 85 × 55 mm (CR80) ≈ 3.5 × 2 in | **SR-10 Credential** | 85 × 55 × 8.5 | 2.9in 296×128 (GDEY029T94, $8.70) | ESP32-C5 | UP/OK/DOWN on the right edge | yes | $30 | $19 |
| 3 × 4 in | **SR-11 Full Bleed** | 76.2 × 101.6 × 11 (+5 D-ring) | 3.97in 800×480, 235 ppi (GDEM0397T81P, $17.50) | ESP32-C5 | UP/OK/DOWN right, BACK left | yes | $41 | $26 |
| 80 × 145 mm | **SR-12 Tour Laminate** | 80 × 145 × 12 | 3.97in 800×480, 235 ppi | ESP32-C5 | 5-way + PREV/NEXT | yes | $42 | $27 |
| 3.69 × 8.49 in (94 × 216) | **SR-13 Hybrid Credential** | 94 × 216 × 14.5 | 4.7in 960×540 16-grey (PaperDIY kit) | ESP32-S3 | PREV/MENU/NEXT | yes (kit) | $36 | $28 |

- **The 3.97in 800×480 panel is the find here.** Its outline is 96.6 × 56.2 mm, so it fits
  a 3 in wide pass. At 235 ppi it gives the sharpest text of any SPI panel in the set, for
  $17.50. A 4-colour twin (GDEM0397F81) is the same size and price.
- **SR-10 is a credential, not a reader.** 296×128 shows about eight lines: badge, running
  order, a zine cover. It is 8.5 mm thick, so the module sits beside a 350 mAh cell rather
  than stacked on it.
- **SR-11 has no band.** The panel fills the 3 × 4 in face (2.5 mm bezel top and bottom; check
  FPC bend room), so the firmware draws the pass graphic at the top of the page and the
  lanyard hooks a D-ring moulded into the back.
- **SR-13 carries the venue's own credential.** A clear window below the keys holds a printed
  ID-1 badge (photo, hologram, barcode) that slides in from the left edge, so staff check it
  the usual way.
- Near misses: SR-3 (78 × 149) is 4 mm off 80 × 145; SR-1 and SR-8 (86 × 138) fall between
  stock sizes.

### SR-15 Dial Pass: the CrowPanel

Elecrow's CrowPanel ESP32 4.2in E-Paper HMI (DIE07300S, $26.90) is a finished board: ESP32-S3
(8 MB flash, 8 MB PSRAM), the same 400×300 SSD1683 glass as SR-1, microSD, a LiPo charger and
its own controls. Because the controls differ, it is its own pass rather than an SR-1 build.

- **Navigation.** A 3-way thumb dial (Elecrow TM_2024A: up, down, push; a toggle, not an encoder)
  with MENU and EXIT beside it. Dial up/down turns pages, push selects, MENU opens the menu,
  EXIT is back; hold MENU + EXIT 2 s to sync; hold EXIT for the badge. Every control is on an RTC
  GPIO (dial IO6/IO4/IO5, MENU IO2, EXIT IO1), so any of them wakes the pass.
- **Power.** IO7 switches the panel's ground and IO42 the microSD's, so both go dark in sleep.
  The charger is a TP4054-class part (~500 mA) feeding an RY3420 buck; its quiescent current is
  unknown, so the battery model assumes 60 µA of board overhead until it is measured (7 months
  at 300 pages/day on 1200 mAh). The board has no power switch and no battery sense: SR-15 adds a
  slide switch in the cell lead and a 100k/100k divider to IO3.
- **Battery socket** is SH1.0 2-pin; the cell plugs in through a JST-PH-to-SH1.0 adapter lead.
- **USB-C goes through a CH340C**, not the S3's native USB, so there is no USB drive: packs load
  by microSD or the Web Serial loader.
- **Mechanics.** The board is 107 × 87 mm with no mounting holes; the shell clamps it by the
  edges, landscape board turned portrait, with the dial on the right edge. Elecrow's repo has a
  STEP model to draw against. Elecrow has shipped PCBA V1.0, V1.2 and V1.2A with different
  panel drivers; check which arrives.
- **Cost:** $38.50 a pass at 5 ($44.10 landed), $26.80 at 100: the cheapest pass
  you can build today (no board to draw), with parts in hand in about 10 days.

### SR-16 Setlist: the slim 3.7in pass

A pass the size of a real backstage laminate, from off-the-shelf parts: Elecrow's bare 3.7in
panel (DIE04037S, $11.90, 240×416, UC8253, ~130 dpi, 47 × 82 mm active area) on Seeed's
ePaper Driver Board for XIAO V2 with a XIAO ESP32-C5, three hand-wired keys and a 1200 mAh
cell. Portrait, half the width of SR-1, sharper than any other 1-bit pass.

- **Panel.** UC8253, not a Solomon controller. GxEPD2 has a driver for Good Display's
  GDEY037T03 (240×416 UC8253): partial refresh ~350 ms, full ~1.5 s. It has the standard 24-pin
  pinout, so it plugs into the same adapters as the 4.2in.
- **Boost resistor.** Elecrow's own CrowPanel 3.7in board (schematic on Elecrow-RD's GitHub)
  drives this panel with RESE = 0.47 Ω and an Si1308EDL. Seeed's driver board has 0.47 Ω fixed,
  so the pair should work as-is (on a CFA-10084, short the RESE jumper).
- **Wiring** is SR-1D's: driver board on D0 RST, D1 CS, D2 BUSY, D3 DC, D8 SCK, D10 MOSI; the
  keys share a Schottky wake line on GPIO3 (back pad) and also go to D4, D5, D9.
- **Text.** About 110–130 words a page at reading size against ~250 on 4.2in: books take about
  twice the page turns. The battery model counts 300 turns a day like the others (10 months on
  1200 mAh), so a heavy reader should expect closer to half that.
- **Size check.** The listing gives the panel as 53 × 93 × 1 mm but also a 67 × 93.4 × 2.6 mm
  "PCB size" and 50 g, so it may arrive on a carrier board. Measure one before printing; the
  64 × 112 × 12 mm pass assumes the bare panel.
- **Cost:** $37.09 a pass at 5 ($45.69 landed: Seeed, Elecrow, DigiKey and Amazon orders),
  $24.46 at 100. The panel rides the same Elecrow order as SR-15's board.

### SR-9 and the thumbstick

The Adafruit 444 is a PSP-style analog stick: two 10K potentiometers,
19 × 19 × 9 mm, $3.50, no click and no mounting holes. What it changes:

- **Pan instead of reflow.** Zines are laid out for paper. For SR-9 the
  renderer also stores each PDF page at 2× (800 × 600 for the 4.2in panel,
  60 KB at 1-bit, ~15 KB compressed); the stick moves a viewport over it and a
  corner map shows where you are. Hold OK and push up/down to zoom 1× / 1.5× /
  2×. Each pan step is a partial refresh (~0.3 s), so movement is in steps,
  rate set by how far the stick is pushed.
- **Fast menus.** Stick deflection sets scroll speed through a long shelf or
  a table of contents.
- **Power.** The pots draw about 0.66 mA whenever powered (~16 mAh a day), so
  a MOSFET powers the stick only while the pass is awake. The stick cannot wake
  the pass; OK and BACK do. Calibrate the centre at each wake and use a dead
  zone.
- **Mechanics.** No mounting holes: a printed cradle clamps it behind the
  bezel, with a TPU skirt around the cap to keep grit out. For a pressable
  stick, Adafruit 512 (with select button and breakout) or 3246 fit the same
  cut-out with a deeper cradle.

### Enclosure

Two-part 3D-printed PETG shell (front bezel + back), joined with four M2
screws into heat-set inserts. Wall 1.6 mm, panel recessed 1.3 mm behind the
bezel so the glass is protected on a lanyard. The top has an 18 mm lanyard
slot with a 25 mm split ring. A printed vinyl laminate band above the screen
carries the pass graphic (GUEST, ALL ACCESS, CREW, AFTER HOURS, VIP,
OFFLINE …) and is meant to be swapped. USB-C on the bottom edge; microSD slot
on the left edge near the bottom. Key caps in TPU.

**Card pocket (SR-2, SR-4):** a raised pocket on the back, inner size
88.6 × 56 × 1.6 mm, which holds an ID-1 card (85.6 × 54 × 0.76 mm) plus a
folded ticket. Open at the top with a 22 mm thumb notch, and a window in the
outer wall shows the card like a badge holder. Adds about 4 mm of depth and
25 g of PETG.

**Zine sleeve (SR-7):** the same pocket sized for an A7 folded paper mini-zine
(74 × 105 mm), so the pass carries one paper issue alongside the digital ones.

### Power: auto-off, any key wakes

A static e-paper page costs nothing, so the pass can be "off" almost all the
time and still show what you were reading. There is **no power switch** on the
custom boards: every key is the on button.

**The auto-off ladder** (same on every variant):

| Level | When | Draw | What's on screen | Wake |
|---|---|---|---|---|
| 0 awake | a key was pressed | 25–40 mA (ESP32), 3–5 mA (nRF) | page being drawn | — |
| 1 linger | 10 s after the last key | light sleep, ~0.35 mA | the page | any key, instant |
| 2 asleep | after linger | deep sleep, ~16 µA (custom ESP32), ~8 µA (nRF) | the page (or the badge, a setting) | any key, ~0.3 s |
| 3 off | 7 idle days, or below 5 % | latch open: only the cell's protection IC, ~3 µA | badge, or a "charge me" card at low battery | any key: cold boot, reopens the saved page |

Each awake feature has its own timeout on top: Wi-Fi/BLE sync 3 minutes, the
SR-6 front light 2 minutes (dims at 1), the SR-14 NFC field 10 s or the first
read. Reading position is saved to flash on every page turn, so level 3 loses
nothing. At 5 % the pass stops at a safe 3.4 V instead of letting the cell's
protection board cut it at 2.5 V, which is what ages LiPo cells.

**Hardware for "any key wakes":**

- *Custom ESP32 boards* get `LATCH(n)` in the BOM (about $1 at 5, $0.30 at 100).
  Each key has a BAT54C diode pair. One diode pulls the gate of a P-MOSFET that
  drives the RT9080's EN pin, so a press from off powers the board up. The
  other pulls a shared ANY-key line on one LP/RTC GPIO, so a press wakes the
  board from deep sleep. A 2N7002 lets the firmware hold itself on and let go
  to switch off. VBUS has a diode too, so plugging in powers on and shows the
  charge screen. The charger sits on the battery side of the latch, so it
  charges while off.
- *nRF52840 passes* (SR-8, SR-14B) don't need the latch. System OFF is
  ~0.4 µA and any GPIO wakes it.
- *EE05 passes (SR-1 on the S3 Plus, SR-8)*: the keys are on D1, D2, D9, all RTC GPIOs on
  the S3, so each wakes it; the EE05's slide switch stays, for storage and shipping.
- *FireBeetle 2 C5 (SR-1C)*: keys on LP GPIO3/4/5 wake it directly; GPIO0 switches the
  panel's 3V3_C rail off in sleep.
- *M5Stack kit passes (SR-3, SR-13)*: the kit's own power path; the keys must
  land on RTC GPIOs. Check on the bench.
- Thumbstick (SR-9) and thumbwheel rotation (SR-5) don't wake the pass; their
  keys do.
- Panel and microSD sit on a switched rail. An idle microSD draws 100–250 µA,
  more than everything else in deep sleep put together. SR-8's breakout got a
  P-MOSFET for this.

**Battery life** (estimates from `../power/battery_model.py`, with a 1.5×
real-world margin on every active figure; details in `../power/README.md`):

| Variant | Cell (stock) | Page turns per charge | Reader, 300 pages/day | Festival day* | If sleep were 250 µA |
|---|---|---|---|---|---|
| SR-1 Laminate | 1000 mAh | ~115,000 | 8 months | 9 months | 3 months |
| SR-1C Laminate C5 | 1000 | ~126,000 | 9 months | 10 months | 3 months |
| SR-2 All Access | 1500 | ~155,000 | 11 months | 1 yr+ | 4 months |
| SR-3 Gray Pass | 1200 | ~42,000 | 4 months | 5 months | 2 months |
| SR-4 Headliner | 2000 | ~49,000 | 5 months | 7 months | 3 months |
| SR-5 Crew Clip | 1200 | ~151,000 | 10 months | 11 months | 4 months |
| SR-6 Night Shift | 1500 | ~189,000 | 1 yr+ | **31 days** (1 h light/day) | 5 months |
| SR-7 Poster | 1500 | ~25,000 | 3 months | 4 months | 2 months |
| SR-8 Quiet Pass | 1000 | ~382,000 | 1 yr+ | 1 yr+ | 4 months |
| SR-9 Roadie | 1200 | ~151,000 | 10 months | 11 months | 4 months |
| SR-10 Credential | 350 | ~49,000 | 4 months | 4 months | 5 weeks |
| SR-11 Full Bleed | 1000 | ~117,000 | 9 months | 10 months | 3 months |
| SR-12 Tour Laminate | 1200 | ~140,000 | 10 months | 11 months | 4 months |
| SR-13 Hybrid Credential | 1500 | ~53,000 | 4 months | 6 months | 3 months |
| SR-15 Dial Pass | 1200 | ~138,000 | 7 months | 8 months | 3 months |
| SR-16 Setlist | 1200 | ~161,000 | 10 months | 11 months | 4 months |
| SR-14W Tap Pass Wi-Fi | 1200 | ~140,000 | 10 months | 8 months (10 taps, 2 fetches) | 4 months |
| SR-14B Tap Pass tethered | 1200 | ~369,000 | 1 yr+ | 1 yr+ | 4 months |

\* Festival day: 150 page turns, one sync, and the variant's extra.

What these say: with deep sleep done right, **page turns and the front light
set battery life**. Standby is a year or more (LiPo self-discharge, ~2 %/month,
is the limit). The last column is the warning: one sloppy part in sleep
(an unswitched microSD, a module board's LED or LDO) costs two thirds of the
battery life. Sleep current is still the first thing to measure.

Without timeouts, the same 1200 mAh cell goes flat in 11 hours with sync left
running, 20 hours with the NFC field on, 40 hours with the SR-6 light on, or
36 hours on a menu with the CPU never sleeping.

**Battery options:** every variant lists a thin (S), stock (M) and extended (L)
cell in `../power/README.md`, with the depth change and price. Examples: SR-1
goes from 500 mAh at 10.5 mm to 1500 mAh at 13.5 mm. SR-6, the one where the
battery matters, goes to 2500 mAh at 18.5 mm (about 7 weeks of an hour's
light a day). SR-10 tops out at 550 mAh at 9.5 mm.

### Availability (checked 2026-11-07)

Full table: `../bom/AVAILABILITY.md`; each BOM CSV has `availability`
columns. Ratings are **ok**, **watch** (single source, broker-only, long
lead or thin stock for 100) and **out**.

- **Out:** M5Stack PaperDIY (SR-3, SR-13) → LilyGo T5 4.7in S3. TI
  TPS65185 and the ED060KC1 panel (SR-4) → brokers / discrete HV supply /
  Kindle-spares sellers.
- **Changed in the BOMs:**
  - Seeed breakout 105990172 is discontinued; the BOMs now use the ePaper Driver
    Board 114993558.
  - MFRC522 is EOL; now the CLRC663 plus (+$3.60, adds low-power card detect).
  - ESP32-C5-WROOM-1-N8R4 isn't an orderable module code; now N8R8.
  - ST25DV04K-IER6S3 (SO8, no low-power pin, idles at 76 µA); now the 12-pin
    JFR6D3.
- **Watch:** C5 modules (DigiKey 20-week lead; brokers hold N8R8), RT9080
  (genuine only at brokers; HT7833 is the drop-in), Adafruit 444 (35 in stock).
- **Ok:** every Good Display panel (buyepaper, 58–94 each, cheaper than last
  revision), XIAO C5 and nRF52840 Plus from Seeed, CLRC663, MDBT50Q.

Upshot: the SPI-glass passes can be ordered today; the 16-grey passes depend
on parts that come and go.

## 11. Alternatives worth knowing

### Chips

No ESP32 has both 5 GHz Wi-Fi and USB OTG. That is the main trade:

| Chip | Wi-Fi | BLE | USB | PSRAM / parallel panels | Good for | Watch out |
|---|---|---|---|---|---|---|
| **ESP32-C5** | Wi-Fi 6, **2.4 + 5 GHz** | 5 | Serial/JTAG only | quad PSRAM; SPI panels | the "just works" sync story; venues | new, module stock; no USB drive |
| ESP32-S3 | Wi-Fi 4, 2.4 only | 5 | **OTG** + Serial/JTAG | octal PSRAM; LCD/i80 for epdiy | parallel 16-grey panels, USB drive | can't see 5 GHz-only networks |
| ESP32-C3 | Wi-Fi 4, 2.4 only | 5 | Serial/JTAG | none | cheapest 1-bit SPI pass (XIAO C3 ~$5) | 400 KB RAM; no 5 GHz, no USB drive |
| ESP32-C6 | Wi-Fi 6, 2.4 only | 5 + 802.15.4 | Serial/JTAG | none | low-power Wi-Fi 6 at C3 money | still 2.4 only |
| ESP32-C61 | Wi-Fi 6, 2.4 only | 5 | Serial/JTAG | quad PSRAM | cheap with PSRAM | 2.4 only |
| **nRF52840** | none | 5 (+ 802.15.4) | **native USB device** | 256 KB RAM | lowest sleep, USB drive, UF2, radio-quiet | BLE-only sync is slow for books |
| nRF54L15 | none | 5.4 | none | 256 KB RAM | cheaper/lower power than '840 | no USB: loses the USB drive |

Ways around the C5's missing USB OTG, if the flagship must be both dual-band
and a USB drive: (a) microSD as the sneakernet loader (chosen), (b) Web Serial
loader (chosen), (c) a $0.50 USB-to-SD bridge chip in the USB path, or (d) a
two-chip board (S3 for USB and panels, C5 as a Wi-Fi co-processor via
esp-hosted), which adds about $4 and power. (c) and (d) are not in any BOM.

### Ready-made bases

| Base | Price | What it gives | Verdict |
|---|---|---|---|
| **M5Stack PaperDIY Assembly Kit** (K159) | $25 | ESP32-S3 N16R8, 4.7in 960×540 16-grey, microSD, USB-C, 140 mAh cell | SR-3's base, but **sold out** (2026-11-07; in stock 16–22 Oct only). Every M5 e-paper product is out. |
| M5Stack M5PaperS3 | ~$59, **EOL** per M5Stack | same panel class with touch, enclosure, battery | Finished device, but over budget and touch we don't need. |
| M5Stack PaperColor | $75 | ESP32-S3, 4in Spectra 6 colour 400×600, 1250 mAh | Quick way to try full colour; over budget. SR-7 uses cheaper 4-colour glass. |
| LilyGo T5 4.7in S3 | $39.99; China warehouse in stock, US/DE sold out | ED047TC1 16-grey, S3, microSD, charger | **The buildable SR-3/SR-13 base today** ($51.54 / $54.04 at 5). |
| Seeed XIAO + ePaper breakout | $6.90–11.19 + $5.90 | any XIAO (C5, C3, S3, nRF52840) on the same 24-pin panels | What SR-1 and SR-8 use; one breakout, swap the brain. |
| Seeed XIAO ePaper Display Board **EE05** (100057220) | $10.99, $9.99 at 10+; in stock | 24-pin FPC + boost, **3 keys** (D1, D2, D9), **JST 2.0 battery socket with slide switch** (P-MOSFET; USB turns it on to charge), TPS22916 load switches on the panel rail (D6) and battery divider (D12); charger is the XIAO's | **Now SR-1 and SR-8's base.** XIAO S3 Plus or nRF52840 Plus only, SMD-soldered incl. back pads; the C5/C3 don't fit. |
| DFRobot **FireBeetle 2 ESP32-C5** (DFR1222) | $7.90; DigiKey 1738-DFR1222-ND, 148 in stock | C5-WROOM-1-N4 (4 MB, no PSRAM), PH2.0 LiPo with 0.5 A charger, 21 µA deep sleep, IO0-switched 3V3_C rail, LP GPIO2–6 broken out, 25.4 × 60 mm | **SR-1C's base**: the dual-band SR-1. No keys, and its GDI connector is for TFTs, so it needs a 24-pin e-paper adapter wired over SPI. |
| DFRobot Beetle ESP32-C3 (DFR0868) / C6 (DFR1117) | $7.90 / $5.90 | 25 × 20.5 mm, TP4057 charger, 4 MB, no PSRAM; C6 sleeps at 14–37 µA | Not the XIAO footprint, 2.4 GHz only, 18-pin GDI connector for TFTs. Cheap bench option for a 1-bit pass, not a better base. |
| DFRobot 4.7in ESP32-S3 E-Ink (DFR0835) | $55, "only 1 left" | 540×960 16-grey, S3 16 MB/8 MB, PH2.0 + protection, 2 keys | Another finished SR-3 base, but over budget and nearly gone. |

DFRobot sells no 4.2in or 2.9in panel and no 24-pin e-paper adapter (checked 2026-11-09).

## 12. Build order

1. SR-1 and SR-8 on the bench together: same EE05 board and glass, S3 Plus and
   nRF52840 Plus (SR-1C on the FireBeetle C5 when the C5 band logic is tested). Port the maplet panel table; add SSD1683. Measure sleep current.
2. Pack format v0 and a desktop pack builder (Python) so firmware can be
   tested without the app.
3. USB drive mode on SR-8 (TinyUSB MSC); Web Serial loader for SR-1C.
4. App: Pass screen + BLE sync of a hand-made pack, then the PDF renderer.
5. Wi-Fi bulk sync with the band logic of §7.1.
6. SR-2/SR-7 schematic and PCB (one board, two panels); enclosure v1 with the
   card pocket and zine sleeve; print and fit.
7. SR-6 front light bring-up on the FL02 panel.
8. Catalog integration once the Subverse feed exists.

## 13. Open questions

1. **What is the Subverse website, and can it publish a catalog feed** (OPDS
   2.0 or a JSON index)? Who controls it?
2. SR-14 Wi-Fi fetch: can Subverse serve packs pre-rendered per panel profile, or do we run an
   open pack-render service?
3. SR-9: keep the 444 (cheap, no click) or move to a pressable stick
   (Adafruit 512) so OK can live under the thumb?
4. App: a new app (`io.iotone.subversereader`?) or a Subverse mode inside
   Switchback?
5. Is 5.83in the right reading size for the flagship, or is a 4.2in pass
   with column mode enough?
6. Should the flagship move to the C5 (dual-band, loader by microSD/Web Serial)
   or stay on the S3 (USB drive, 2.4 GHz only)? The table in §11 is the trade.
7. 16 greys or buyable? Commit SR-3/SR-13 to LilyGo boards when in stock,
   or make SR-12's 3.97in 800×480 the sharp-text pass?
8. Front light on the 5.83in flagship too? Good Display offers front light as
   an add-on for that panel, but no price was found.
