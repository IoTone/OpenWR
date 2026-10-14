# Waveguides for Hiroglass REV1: can a small team actually buy one?

Research date: 2026-10-13. Prices are list prices seen on that date or quoted by the cited
page. Alibaba prices are seller asks and often hide setup or tooling terms, so confirm with
the seller before planning around them. Anything marked "unverified" came from an aggregator
or a secondary source.

## Verdict

The earlier note said waveguides are "out of reach" because JBD, Dispelix and Lumus sell only
through sales quotes. **That is wrong about waveguides in general.** It is still right about
the tier-1 microLED parts. As of Oct 2026:

- **You can buy a waveguide module in qty 1 without a quote.** These are array (geometric)
  waveguides with a micro-OLED engine, about $119-$499 retail, and $400-$900 on Alibaba with
  MOQ 1.
- **You cannot buy a green microLED engine plus waveguide (JBD 0.13" class) in qty 1 through
  a distributor.** No Mouser, Digikey, Tindie or AliExpress listing turned up. JBD, Goertek,
  Cellid, Lochn and Lumus are all OEM-quote only. The closest open listing is a mono-green
  waveguide *without* its projector: $800-1,000, MOQ 5.
- **None of these parts fit the brief of "Mouser/Digikey-class and not single-supplier".**
  Every buyable module is a one-vendor Shenzhen part with an NDA'd datasheet and no
  guaranteed second source.
- **The cheapest working waveguide hardware is a consumer donor.** Vuzix Z100 (about $499
  new) and Even G1 (about $459 used, $499 new) are monochrome-green microLED waveguide
  glasses that already have documented or official BLE display paths. Using one means
  driving its existing BLE display path, not salvaging the optics.

**Recommendation for REV1:** keep the SPI OLED with a free-space or birdbath combiner as the
product path. Run a waveguide experiment off to the side with one of these:
- (a) the $119-$499 DisplayModule array-waveguide module, driven over parallel RGB or HDMI
  from a puck;
- (b) a Z100 or Even G1 used as a BLE "display peripheral" for the ESP32-C3. This is the
  cheapest way to test outdoor readability.

## 1. Waveguide modules you can buy off the shelf (qty 1-100)

| Item | Type | Display | Price / MOQ | Notes | Source |
|---|---|---|---|---|---|
| DisplayModule DMGTX0023WGNA 0.23" micro-OLED waveguide module | Array (geometric, multi-reflection), 1.5 mm | Sony ECX336CN 640x400 micro-OLED, 3000 nit panel | $499 SKU (a "from $119" variant also appears on the collection page); 5-20% off at 10-500+ units; add to cart | 26° diag FOV; **0-400 nits at the eye**; >80% transmittance; eyebox 10.8x5.4 mm at 15 mm; engine 23.5x14.8x9.5 mm, about 12 g. Input is parallel RGB/YCbCr 24-bit; an optional micro-HDMI driver board is sold separately. Electrical datasheet is under NDA. | [displaymodule product](https://www.displaymodule.com/products/dmgtx0023wgna-0-23-micro-oled-optical-waveguide-module-ar-smart-glasse), [collection](https://www.displaymodule.com/collections/ar-vr) |
| DisplayModule DMGTX0068WUNA 0.68" micro-OLED waveguide module | waveguide | micro-OLED | $899, add to cart | 50° FOV | [collection](https://www.displaymodule.com/collections/ar-vr) |
| DisplayModule ARM-101 1280x720 waveguide module | waveguide | – | $999, sold as "order samples, immediate delivery" | product page now returns 404 | (search result, displaymodule.com) |
| Zhenshi Intelligent monocular/binocular micro-OLED waveguide module | waveguide (type not stated) | OLED | $400-590, MOQ 1 | Alibaba | [Alibaba supplier list](https://electronics.alibaba.com/supplier/waveguide-display-module-ar-glasses) |
| Zhenshi monocular LCoS "holographic" module | the title says birdbath, so it is probably not a true waveguide | LCoS | $416-518, MOQ 1 | | same |
| Shenzhen Control Electronic 640x400 OLED waveguide module | waveguide | OLED | $1,600-2,100, MOQ 1 (another listing for the N901 is $598-2,198) | | [Alibaba optical-waveguide](https://electronics.alibaba.com/product/optical-waveguide) |
| Shenzhen Wenqiao full-lamination **mono-green waveguide** | waveguide only (type not stated, likely diffractive) | none; you supply a microLED projector | $800-1,000, MOQ 5 | This is the only green-microLED-class waveguide listed openly. You still need the projector. | [Alibaba supplier list](https://electronics.alibaba.com/supplier/waveguide-display-module-ar-glasses) |
| Wanwei Zongheng holographic waveguide "concept prototype" frames | holographic | – | $240, MOQ 1 | These are probably industrial-design dummies. Verify before buying. | same |
| Aggregator price bands for modules (unverified) | diffractive / geometric | – | about $95-210 diffractive, about $180-320 geometric; another page says $180-290 diffractive | treat as indicative only | [accio](https://www.accio.ai/find-product/ar-optical-waveguide-module), [Alibaba optical-waveguide](https://electronics.alibaba.com/product/optical-waveguide) |

**OEM-only (quote, NDA, volume):** I found no public pricing for any of these.
- Goertek's microLED single-green diffractive module: 30° FOV, >1500 nits at the eye.
- Lochn Optics: array and diffractive modules.
- Cellid's plastic waveguide plus microprojector. It is sold as an assembled module: 30° FOV,
  3.2 g, 1.24 mm thick. The Dec 2025 green VGA reference design uses an Alif E7.
- The Lumus Maximus/Z-lens, Dispelix and DigiLens dev kits.
- North Ocean, Tripole and Crystal Optech also turned up no public listing.

Sources: [Goertek](https://goertek.com/en/content/details15_186673.html),
[Auganix Cellid 2026 ref designs](https://www.auganix.org/?p=17917),
[dealroom Lochn](https://app.dealroom.co/companies/shenzhen_lochn_optics_technology_ltd).

**Price at 10k units:** No public figures. Industry commentary and JBD's own statements put
microLED panels at "tens of dollars" in volume
([eeNews](https://www.eenewseurope.com/en/jbd-offers-small-but-bright-microled-projector)).
Finished OEM glasses sell at $250-400 with MOQ 3000 (Metavision, Alibaba), which suggests an
engine plus waveguide BOM of roughly $50-150 at volume. That figure is my inference.

## 2. Projectors that pair with waveguides (microLED / LCoS)

- **JBD 0.13" VGA mono microLED (JBD4UM480P, Hummingbird family):** 640x480, green up to
  about 2M nits, panel price in the "tens of dollars". Hummingbird Mini II has been in mass
  production since 2024. JBD says it offers Hummingbird I dev kits, but no public price or
  distributor turned up. It is quote-only.
  [ledinside](https://www.ledinside.com/news/2026/08/jbd_microled),
  [eeNews](https://www.eenewseurope.com/en/jbd-offers-small-but-bright-microled-projector)
- **WaveOptics "Leopard" kit (JBD + Loki engine 0.6 g + Katana waveguide, mono green):** This
  was the reference kit, but WaveOptics was acquired by Snap and the kit is no longer sold.
  [microled-info](https://www.microled-info.com/waveoptics-partners-jbd-incorporate-microleds-its-leopard-ar-dev-kit)
- **Avegant and Himax LCoS engines:** OEM only, and no small-buyer channel turned up.
- **Retail LCoS waveguide module:** One full-color 1080p LCoS module is listed at $3,999,
  which is out of budget.
- **Micro-OLEDs that are actually buyable:** Sony ECX336/ECX335 and SeeYA 0.39-0.71" panels
  run $72-$322 on Alibaba with MOQ 1-2. These are what the buyable waveguide modules use.
  Their weak point is brightness: micro-OLED gives only about 3000 nits at the panel and
  about 400 nits at the eye through a waveguide.

**The SPI OLED cannot feed a waveguide.** Waveguide in-couplers need a collimated projector
with a small, bright source, and a 0.96-1.3" SPI OLED is far too large and too dim (about
100-500 nits). REV1 would need a micro-OLED (parallel RGB, MIPI or HDMI, which the C3 cannot
drive) or a microLED panel with its own driver ASIC.

## 3. Donor and teardown route

| Device | Optics | Price | Open driving path |
|---|---|---|---|
| Vuzix Z100 | green microLED + waveguide, 640x480, 30° FOV, BLE | $499 new (Dev Edition was $799) | Official Android/iOS SDK. It also works with MentraOS (formerly AugmentOS, MIT license). BLE only, so it pairs with an ESP32 as the "brain". [Notebookcheck](https://www.notebookcheck.net/Vuzix-debuts-ultralight-Z100-smart-glasses-developer-s-edition-to-enterprise-customers-wanting-better-optimized-staff.792371.0.html), [arcompare](https://www.arcompare.com/ar-glasses/vuzix-z100/) |
| Even Realities G1 | green microLED + waveguide, binocular, built for prescription lenses | $499 new (launched at $599); **$459 used** on eBay | Official demo app at github.com/even-realities/EvenDemoApp. It uses a Nordic-UART-style GATT service and sends 1-bit 576x136 BMPs. Gadgetbridge supports it. [eBay](https://www.ebay.com/itm/317999034669), [EvenDemoApp index](https://docsearch.algolia.com/mcp/docs/repo/even-realities/evendemoapp), [Guttag](https://kguttag.com/2024/08/18/even-realities-g1-minimalist-ar-glasses-with-integrated-prescription-lenses/) |
| Even Realities G2 | green microLED, 576x288 4-bit gray, up to 1200 nits | from $599 | Community reverse-engineered protocol at github.com/i-soxi/even-g2-protocol (handshake, CRC-16, teleprompter text). Official dev hub at hub.evenrealities.com. [repo](https://github.com/i-soxi/even-g2-protocol), [hub](https://hub.evenrealities.com/docs/get-started/overview) |
| Rokid Glasses | green microLED waveguide, 30° FOV | $699 | closed |
| RayNeo X3 Pro | full-color waveguide | $1,420-1,500 (Alibaba) | closed |
| INMO Air3 | full-color waveguide | $849-1,199 (Alibaba) | closed |
| Brilliant Labs Halo | 0.2" color micro-OLED, **not a waveguide** (free-space or prism) | $399 | fully open source |

Sources for Rokid and Halo prices: [cybernews](https://cybernews.com/vr-ar/best-smart-glasses/),
[oxivue](https://oxivue.com/guides/how-much-do-smart-glasses-cost). I found no prices for INMO
Go or Meizu StarV.

**Caveats:**
- Salvaging the optics alone is impractical. The projector sits on a proprietary flex with a
  custom driver ASIC and is glued and aligned to the waveguide.
- Treat the device as a BLE display and drive it with your own firmware on the phone or puck
  side.
- Supply depends on one vendor's consumer SKU. That is fine for a test, but not for a BOM.
- Reverse-engineering for interoperability is generally fine in the US (DMCA 1201(f)).
  Reselling modified units and the vendor's ToS are separate risks.
- The form factor is a full pair of glasses, not a clip-on.

## 4. DIY and maker waveguides

- **Litiholo kits:** about $100-130 for a single color and $329 for full color (older
  prices; film prices rose recently). They record display holograms on photopolymer. In
  principle you can record a simple HOE combiner or in/out-coupler pair with them, but
  laser-line efficiency, angular selectivity and humidity sensitivity make them a science
  project. [gadgeteer](https://the-gadgeteer.com/?p=68923),
  [litiholo blog](https://litiholo.com/blog/?p=298)
- **Bayfol HX (Covestro photopolymer):** This is the standard material, at about 10 mJ/cm².
  It is sold B2B and swells with humidity and temperature.
  [Gallego et al. SPIE 2021](https://rua.ua.es/dspace/bitstream/10045/125306/1/Gallego_etal_2021_ProcSPIE-11774.pdf)
- **University of Melbourne liquid geometric waveguide:** a 3D-printed frame filled with
  silicone oil, with dielectric mirror slats between cover glasses; 38x28.5x3.2 mm. It avoids
  dicing and polishing, but uses Stratasys PolyJet. This is the most maker-plausible design.
  Expect visible ghosting and seams, and stray light.
  [3DPI](https://3dprintingindustry.com/news/researchers-develop-low-cost-optical-waveguide-for-ar-using-3d-printing-242599/)
- **Edmund/Thorlabs:** They sell no AR waveguides. You could build a crude 1-mirror
  "light-guide" from a glass slab plus a partial mirror (a beamsplitter cube or plate at
  $30-150). Optically that is a thick combiner, not a pupil-expanding waveguide, so the
  eyebox is tiny.
- **Realistic quality:** DIY gives a demo, not a product. Pupil expansion, uniformity and
  ghost control need sub-micron gratings or polished, coated mirror arrays.

## 5. Practical constraints

- **Efficiency:**
  - Diffractive waveguides deliver about 400-1000 nits/lm at 30° FOV, up to about 2%.
  - Geometric waveguides deliver about 650-1000 nits/lm at 50° FOV, about 5% (2D EPE) and
    about 15% (1D EPE). [eLight review table](https://elight.springeropen.com/articles/10.1186/s43593-023-00057-z/tables/2)
  - Lumus says its reflective design passes about 0.7% of display nits to the eye, which is
    about 5-7x a diffractive design.
    [Guttag 2025](https://kguttag.com/2025/05/29/exclusive-playnitride-single-chip-full-color-microled-lumus-waveguide-with-a-fantasically-tiny-projector/),
    [Guttag 2022](https://lumus.com/karl-guttag-lumus-type-waveguides-with-microleds-maybe-10x-more-efficient-than-diffractive-waveguides)
  - Rule of thumb: a source at 1M nits gives about 1-5k nits at the eye.
- **Outdoor and snow:** You need about 2-3k nits or more at the eye, plus a tinted
  outer lens. A micro-OLED waveguide at about 400 nits at the eye is **unreadable on
  snow**. Green microLED is the only realistic option, for example the Goertek module at
  over 1500 nits.
- **FOV and eyebox:** 26-30° diagonal is typical for this class; 640x480 is plenty for HUD
  glyphs. A 10x5 mm eyebox at 15 mm eye relief is tight for a clip-on that moves with the
  helmet, so alignment and adjustability matter.
- **Rainbow artifacts:** Diffractive (SRG) waveguides show rainbow glare from overhead sun
  and lights. Geometric and array waveguides mostly avoid this but can show stripes or
  ghosts.
- **Prescription:** Waveguides are flat plates, so the prescription goes in an inner insert
  (Even G1 bonds it on). A clip-on placed in front of the user's own glasses works but adds
  eye relief, which shrinks the effective eyebox.
- **Monochrome green vs color:** Green microLED is the efficient, available option at about
  2M nits. Color microLED and color LCoS cost more power and money. Green is the right choice
  for a sports HUD.

## 6. Bottom line

- **Cheapest credible lab experiment, about $500-650:** a DisplayModule 0.23" array
  waveguide module ($499, or the "from $119" variant if it is real) plus its micro-HDMI
  driver board (estimated at about $50-100). Feed it from a puck that has video out, such as
  a Pi Zero 2W or an RP2040 or ESP32-S3 with DPI/RGB. The ESP32-S3 has a parallel RGB LCD
  peripheral and the C3 does not. Indoors this is good. Outdoors on snow it is not, at about
  400 nits.
- **Cheapest outdoor-credible test, about $460-500:** a used Even G1 or a new Vuzix Z100,
  driven over BLE from the ESP32-C3 or the Switchback app. This tests readability, eyebox
  and UX with real green microLED waveguides, without any optics work.
- **True part-level green microLED path:** the Wenqiao mono-green waveguide ($800-1,000 x5)
  plus a JBD panel or engine obtained by quote. This needs NDAs and probably more than
  $5k of commitment, so it is not REV1 material.
- **When waveguides become a mainstream part:** JBD panels in mass production at "tens of
  dollars", plastic waveguides in mass production (Cellid, 2025) and $250-400 OEM glasses at
  MOQ 3000 all point to commodity engine modules (waveguide + microLED + driver, SPI or MIPI)
  appearing on AliExpress or LCSC-class channels around **2027-2028**. That is my estimate.
  Distributor stocking on Mouser or Digikey with second sources is probably **2028 or
  later**. Watch for a JBD or Goertek engine listed with a public datasheet and an SPI/QSPI
  interface; that is the signal for Hiroglass.
