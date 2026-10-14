# Build one optics pod, then earn binocular

Hiroglass REV1 should start as **one small clip-on pod that takes swappable optics cartridges**, built on goggl1's ESP32-C3 electronics, with the battery in a pocket puck. The first two cartridges should be a flat-plate see-through combiner and a look-around eyepiece. Three more approaches belong later on the roadmap: a behind-the-visor reflector for helmets and goggles, a micro-OLED pod driven by an open FPGA, and a USB-C display peripheral on a Linux puck running the open OpenXR runtime Monado for binocular work. The silicon is cheap and comes from several suppliers: display, MCU, IMU, light sensor and charger cost about **$24 at quantity 1** from DigiKey and Mouser. Optics, mechanics and sunlight readability decide whether the device works and what it costs. Every bright near-eye microdisplay that was found is effectively single-supplier and absent from Mouser and DigiKey, so it breaks the spec's sourcing rule. The commodity monochrome OLEDs that keep the BOM open are too dim for a see-through combiner in snow. Two moves fix that: put the combiner behind the goggle or sunglass tint, or skip the combiner and let the user glance at a look-around eyepiece. Binocular spatial XR is a different machine. It needs a wired video link, a fast timestamped IMU and host-side rendering, so REV1 should commit now to a few cheap choices that keep that door open: route all USB-C pins, mount the IMU on the optics, use a shared rail, and store calibration on the device. One finding is easy to miss. goggl1 today puts a transparent OLED directly in front of the eye with no lens, and the eye cannot focus on a panel that close. Adding a collimating lens is REV1's first real optics task.

## Optics, not silicon, sets the cost and the sunlight limit

The core electronics are not the hard part. A 0.96" 128x64 SSD1306 PMOLED costs **$5.24 (Winstar) to $11.57 (Orient Display)** at DigiKey, and Raystar sells the same controller class at TME. An ESP32-C3-MINI-1 module is $3.26, a BMI270 IMU $4.23, a VEML7700 ambient light sensor $1.44 and a BQ24074 charger $3.19, all stocked by more than one distributor ([DigiKey Orient](https://www.digikey.com/en/products/detail/orient-display/AOM12864A0-0-96WW/22531883), [DigiKey Winstar](https://www.digikey.com/en/products/detail/winstar-display/WEO012864DWPP3N00006/20533252), [TME Raystar](https://www.tme.com/us/en-us/details/rex012864mxpp3n01/graphical-oled-displays/raystar-optronics/rex012864mxpp3n00001/), [DigiKey ESP32-C3-MINI-1](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-C3-MINI-1-N4/13877574), [DigiKey BMI270](https://www.digikey.com/en/products/detail/bosch-sensortec/BMI270/9974486)). Stock is shallow and changes fast: the Orient part went from 30 in stock to an 8-week lead on the same day. The shared SSD1306 controller is therefore the real second source, not any one part number. Choose ICM-42688-P only as an alternate, since DigiKey showed it at zero stock with a 45-week lead ([DigiKey](https://www.digikey.com/en/products/detail/tdk-invensense/icm-42688-p/10824934)).

Bright panels are where the sourcing rule bites. No near-eye micro-OLED microdisplay turned up as stocked at DigiKey or Mouser. The 0.39"-0.49" parts sell through Tindie and DisplayModule-type resellers ([DisplayModule 0.39"](https://www.displaymodule.com/products/0-39-inch-micro-oled-display-1920x1080-with-mipi-i2c), [Tindie 0.39"](https://www.tindie.com/products/oled-modules/micro-ar-display-039-inch-1024x768-oled-display/)). Sony's ECX33x panels, which Brilliant Labs' Monocle and Frame glasses proved out, have **no public datasheet**. Monocle's own driver documentation could not find one ([Monocle docs](https://monocle-docs.readthedocs.io/en/latest/firmware_driver_api/ecx335af.html)). JBD microLED and Himax LCoS appear only in press releases and kit announcements, although Himax claims 350,000 nits at about 200 mW ([Himax CES 2026](https://www.nasdaq.com/press-release/himax-and-auo-partner-unveil-ultra-slim-high-brightness-lcos-microdisplay-ces-2026), [LEDinside on JBD kits](https://www.ledinside.com/news/2026/08/jbd_microled)). The only TI-backed display at DigiKey is the DLP2000 EVM at $118.80, with lead times of 12 to 54 weeks, and it is too bulky for a clip-on ([DigiKey](https://www.digikey.com/en/products/detail/texas-instruments/DLPDLCR2000EVM/7598640), [Farnell](https://be.farnell.com/texas-instruments/dlpdlcr2000evm/evaluation-module-display/dp/3009788)).

Brightness is the physics problem. Karl Guttag, a display analyst, writes that see-through AR outdoors needs **"thousands of nits delivered to the eye."** Frame's 3,000-nit Sony panel loses about three quarters of its light in a non-polarized birdbath (a beam splitter plus curved mirror), delivering roughly 700-750 nits, which he calls "fine indoors but marginal outdoors" ([KGOnTech 2023](https://kguttag.com/2023/03/12/microleds-with-waveguides-ces-ar-vr-mr-2023-pt-7/), [KGOnTech 2024](https://kguttag.com/2024/05/09/brilliant-labs-frame-ar-with-ai-glasses-a-little-more-on-the-apple-vision-pro/)). Commodity small OLEDs publish no luminance at all. Newhaven's color PMOLEDs at Mouser are rated **90-100 cd/m²** ([Mouser](https://www.mouser.hk/newhaven-full-color-oled-displays)), and no TFT under 1.5" brighter than 800 nits was found at a major distributor ([Orient Display](https://orientdisplay.com/tft-displays/tft-panels/high-brightness/)). Reviewers already single out bright-day legibility as the weak point of Rekkie, a shipping smart ski goggle ([GearJunkie](https://gearjunkie.com/winter/augmented-reality-smart-goggles-rekkie-review)). Two levers stay inside a commodity BOM. The first is to mount the combiner **behind** the sunglass or goggle tint, which dims the world but not the display; a category-3 lens should cut the brightness needed by roughly 5-10x (an estimate from standard tint categories, not measured). The second is a look-around (occluded) eyepiece, which delivers close to 100% of panel light. Vuzix's M400 takes that route and rates over 2,000 nits ([Vuzix](https://vuzix.com/en-ca/pages/compare-smart-glasses)).

Focus math is simple and favors cheap parts. With a single positive lens and the display just inside the focal length, the whole range from infinity to 2 m is about 0.3 mm of travel at f = 25 mm. Diopter adjustment costs about f² per diopter, roughly 0.6 mm/D, so a ±3 D range for goggle and helmet users needs only an M2 screw or a printed cam. A 0.96" panel behind a 40 mm lens gives about 30° horizontal FOV. These are thin-lens estimates from the research notes, not measurements. Wearers of prescription glasses get correction for free when the pod sits outside their lenses. The same math explains why goggl1's current DFRobot transparent OLED, mounted bare in front of the eye, cannot be focused: open builds that use a transparent OLED directly hit the same wall ([Hackster TOLED glasses](https://www.hackster.io/news/this-16-year-old-made-his-own-pair-of-toled-smart-glasses-790d0f2e1420)).

## Five approaches a team of 2-3 can build

The notes list six optical families. Waveguides drop out of the REV1 product path: a small team cannot fabricate them, and the bright projectors they need are quote-only from single suppliers ([LEDinside](https://ledinside.com/node/view/32728)). They still earn a parallel experiment, covered in the waveguide section below. The five approaches below remain, ordered from the cheapest and most open to the most ambitious.

### 1. Plate-combiner pod: goggl1 electronics, one lens, a tilted partial mirror

**Architecture.** A 0.96" SSD1306/SSD1309-class OLED sits at the focal plane of a 15-20 mm PMMA aspheric lens. The image folds off a 45° plate beamsplitter, either wedged or AR-coated with about 30R/70T, inside a matte-black SLA housing. An ESP32-C3 sits in the pod with BLE to the phone and a cable to a battery puck. The pod mounts on the common magnet-plus-dovetail dock described below.

**BOM.** About $24 of silicon, plus $5-30 for AliExpress acrylic or glass plates and lenses, or **$61-136 for Edmund glass plates** ([Edmund 50R/50T](https://www.edmundoptics.com/p/25mm-diameter-50r50t-plate-beamsplitter/5240/), [Edmund VIS](https://www.edmundoptics.com/p/25mm-dia-50r50t-vis-plate-beamsplitter/6279/)). PCBs, LiPo, cable and printed parts add an unpriced $40-120 (estimate). A quantity-1 prototype lands around **$75-215**.

**Sourcing risk.** Lowest of the five. Plates, lenses and SSD1306 glass all come from several suppliers.

**Outdoor readability.** Good indoors and on overcast days. In snow it is weak unless the plate sits behind the tint. Flat plates also produce ghost images from the rear face, which is why wedged plates exist ([US4874214](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/4874214), [Edmund wedged](https://edmundoptics.eu/p/25mm-diameter-50r-50t-vis-wedged-plate-beamsplitter/3784)).

**Reuse.** Nearly total. The ESP32-C3, the SPI OLED driver, the Forth vocabulary, the NUS console, bundles and signed `.mnpkg` OTA all carry over. Switchback's `lib/goggle` keeps working.

**Path to binocular.** Weak on its own. Field of view and eyebox are small, and BLE cannot carry video.

**Effort.** Bench optics in weeks, and a wearable in one or two months for one optomechanical person. This is where the documented DIY builds cluster (ESP32 or Arduino, a small OLED, a lens, acrylic or film), and their published lesson is that alignment and stray reflections are the hard part ([Arduino blog](https://blog.arduino.cc/2021/08/27/giving-google-glass-a-run-for-its-money/), [hackaday.io Smartglass](https://hackaday.io/project/92176-smartglass)).

### 2. Look-around eyepiece pod: the sunlight answer

**Architecture.** The same pod and electronics, but the cartridge is an opaque eyepiece the user glances into, placed beside or below the line of sight, as on the Vuzix M400 ([Vuzix M400](https://vuzix.com/products/M400-Smart-Glasses)).

**BOM.** The OLED plus eyepiece costs an estimated $20-60. Roughly, this is approach 1 with the plate removed.

**Sourcing risk.** Low.

**Outdoor readability.** The best of the commodity options, because no combiner throws light away. The cost is no see-through overlay.

**Reuse.** Identical to approach 1.

**Path to binocular.** None directly, but it validates the pod, the dock and the calibration flow.

**Effort.** Slightly less than approach 1, since there is no ghosting to fight.

The ergonomics research supports glanceable placement. A 2025 study recommends keeping pixels **outside about 8° of primary gaze**, vertically centered and +8.7° to +23.7° toward the ear, with a horizontal FOV near 15° for text ([arXiv 2505.09047](https://arxiv.org/html/2505.09047v1)). Garmin's Varia Vision is the closest commercial reference: a 29 g pod on a 1-2 g temple clip, worn on either side with automatic image flip. It was tied to Garmin's proprietary ANT+ remote-display link and now appears to be end of life ([DC Rainmaker](https://dcrainmaker.com/2016/01/garmin-varia-vision-hud.html), [Multitronic](https://www.multitronic.fi/en/products/1672760)).

### 3. Behind-the-visor reflector for helmets and goggles

**Architecture.** A small display and projection lens sit at the side or brow and reflect off the goggle lens or helmet visor. A bare visor reflects about 4%. A strip of HUD film, or a small separate combiner, can sit behind the tint. This is the classic helmet-HUD design: the display at the lens focal plane puts the image at infinity ([US10884245](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/10884245)).

**BOM.** The optics add an estimated $10-40 plus film on top of the pod electronics.

**Sourcing risk.** Low for parts. The real risk is geometry: every goggle and visor curves differently, which works against "one clip fits all." Using the visor itself as the combiner also removes per-wearer adjustment ([US5543968](https://patents.google.com/patent/US5543968)). Shoei's Opticson avoids this by putting a separate combiner on an adjustable flip-up bracket ([MCN](https://motorcyclenews.com/news/new-tech/shoei-opticson-hud-display-helmet)).

**Outdoor readability.** Excellent see-through, and the behind-tint placement is the strongest lever there is for snow.

**Reuse.** Full electronics reuse.

**Path to binocular.** Modest. Large visors can host wide-FOV research combiners, which is essentially what Project North Star does.

**Effort.** Medium. Expect a parametric adapter for each goggle family, plus anti-fog and vent clearance if the pod sits inside the goggle.

**Recommendation.** Treat the practical REV1 version as "approach 1's cartridge on a goggle-interior adapter," and keep the true off-axis visor reflector as a side experiment.

### 4. Micro-OLED pod driven by an open FPGA

**Architecture.** A Sony 0.23" 640x400 micro-OLED, or a SeeYA 0.49" 1080p, behind a prism or small birdbath. A small FPGA drives the display: Frame uses a Lattice CrossLink-NX feeding the panel an 8-bit YCbCr bus while an nRF52840 configures it over SPI ([Frame hardware manual](https://docs.brilliant.xyz/frame/hardware)). The MCU can stay an ESP32-C3, or move to an nRF52840/nRF54L15 module. Power and compute can move to the puck.

**BOM.** Panel-plus-prism kits run **$184 for a 0.23" 640x400 helmet module, $389 for the 4,000-nit version**, and $286-415 for 0.39"/LCoS kits ([Hicenda](https://www.hicendadisplay.com/products/0-39-inch-oled-microdisplay-module-rgb-1024x768-integrated-kits-for-ar)). Tindie sells Sony HDMI driver boards and SeeYA boards at $149-579 ([Tindie Sony](https://tindie.com/products/oled-modules/hdmi-driver-board-for-the-sony-microdisplays), [Tindie SeeYA](https://www.tindie.com/products/oled-module/0491080p-micro-oled-display-1920x1080-1800nits/)). Prototypes cost an estimated $200-450, which is at or over the $300 retail ceiling.

**Sourcing risk.** High. Sony is single-source with no public datasheet, and SeeYA part numbers vary across listings.

**Outdoor readability.** Better than PMOLED. A 3,000-4,000 nit panel works well in a look-around layout, but through a birdbath it is still marginal outdoors.

**Reuse.** The protocol and app carry over unchanged, because content stays retained-mode primitives. BLE cannot stream video anyway: 640x400 at 4 bpp is 128 KB per frame, and ESP32-S3 measures about **46-63 KB/s** of real BLE throughput ([esp32.com](https://esp32.com/viewtopic.php?p=134134)).

**Path to binocular.** Good. The same panel class and FPGA scale to two eyes.

**Effort.** Months of RTL and board work for the EE. Frame's RTL is the best reference, but its repo carries an "Other" license rather than a standard one, so check reuse terms first ([brilliantlabsAR](https://github.com/orgs/brilliantlabsAR/repositories)). Prefer iCE40 or ECP5 parts with the fully open Yosys/nextpnr toolchain, unless MIPI D-PHY forces CrossLink-NX. An untested alternative is to drive the parallel bus with RP2350 PIO and no FPGA at all.

### 5. USB-C display peripheral on a Linux puck, through OpenXR

**Architecture.** The head unit becomes a "dumb display plus sensors" device. Video arrives over USB-C DisplayPort alt mode or HDMI, the IMU streams over USB, and a puck (RK3588, Pi 5/CM5, or a phone) renders through Monado, the open OpenXR runtime. The first stereo display can be borrowed: wheaney's GPL-3.0 XRLinuxDriver already supports Xreal, Viture, Rokid and RayNeo glasses, which gives several vendors and no lock-in to one ([XRLinuxDriver](https://github.com/wheaney/XRLinuxDriver)). Monado 25.1.0 added further drivers and Cardboard calibration on Android ([GamingOnLinux](https://www.gamingonlinux.com/2025/12/open-source-xr-runtime-monado-25-1-0-released-with-expanded-hardware-support/)). The DIY optics step is a dual Sony ECX335 birdbath module: Tindie lists one with a 50° FOV, at prices that range widely, from $75 to $569 ([Tindie](https://tindie.com/products/oled-modules/bird-bath-lenses-ar-glasses-optical-module-fov50)).

**BOM.** For context, HoloKit X, an open-source phone-based stereo AR viewer, costs $129 ([HoloKit](https://holokit.io/products/holokit-x)), and the open Relativty VR headset costs about $200 DIY ([Hackster](https://www.hackster.io/news/relativty-is-an-open-source-vr-headset-you-can-build-for-200-cfe0bd1ec522)). Glasses and SBC prices were not verified. A DIY pair lands at or above $300 before compute.

**Sourcing risk.** Medium for the borrowed-glasses route, because several vendors are supported. High for the DIY route.

**Outdoor readability.** Birdbath class, so marginal in sun.

**Reuse.** Switchback and the BLE protocol become a sideband for settings and HUD widgets. The new work is a Monado driver.

**Path to binocular.** This is the binocular path itself.

**Effort.** Mostly software. 3DoF head tracking is solved. 6DoF through Monado's Basalt tracker is described as CPU-heavy and below commercial quality ([Collabora](https://www.collabora.com/news-and-blog/news-and-events/openxr-and-monado-recent-progress-and-whats-to-come.html), [FOSDEM 2026 slides](https://fosdem.org/2026/events/attachments/T38AKM-monado-and-beyond/slides/266219/monado_an_tfzl8ak.pdf)). Project North Star (Apache-2.0) is the best open reference for wide-FOV mechanics, but its newest variant dates from June 2023 and it depends on Intel RealSense trackers ([North Star docs](https://docs.projectnorthstar.org/)).

## Approaches 1 and 2 share a pod, so build them together

| | 1. Plate combiner | 2. Look-around | 3. Visor reflector | 4. Micro-OLED + FPGA | 5. USB-C + Monado |
|---|---|---|---|---|---|
| Proto BOM (est.) | $75-215 | $70-190 | $80-200 | $200-450 | $150-450 + puck |
| Single-supplier parts | none | none | none | panel (Sony/SeeYA) | panel or glasses vendor |
| Outdoor (snow) | weak; good behind tint | strong (100% of panel light) | strong behind tint | good look-around, marginal birdbath | marginal |
| See-through | yes | no | yes | yes (prism/birdbath) | yes |
| goggl1/Switchback reuse | full | full | full | protocol + app | sideband only |
| Path to binocular | low | low | modest | good | direct |
| Team effort | 1-2 months | 1-2 months | 2-3 months + per-goggle adapters | 3-6 months, FPGA skills | 3-6 months, software-heavy |
| Fits spec today | yes | yes | yes | strains $300 and sourcing | strains sourcing |

The table makes the sequencing plain. Approaches 1 and 2 differ only in the optics cartridge, so one pod housing, one PCB and one dock serve both. That gives REV1 a see-through option and a sunlit option for the price of one electronics design. Approach 3 is mostly a mounting variant of approach 1. Approaches 4 and 5 are where the spec's FPGA and puck allowances earn their keep, and both fail the sourcing rule today because of their panels. They belong on an experimental branch until a second micro-OLED supplier is proven or JBD's announced kits reach open retail. Steelmanning the other side: buying a $48 LILYGO T-Glass (ESP32-S3 with a prism AMOLED) or a $184 Hicenda module would put "what a real module feels like" on someone's face within a week ([CNX Software](https://www.cnx-software.com/2024/07/01/lilygo-t-glass-esp32-powered-smart-glasses-with-1-1-inch-prism-display-imu-microphone/?amp=1)). That is worth doing as a reference. It is not worth designing around: the T-Glass prism cuts the panel to 126x126 usable pixels, and stock has come and gone.

## REV1 sequencing: Pod-0 first, binocular hooks from day one

**The concrete first prototype is "Pod-0":**

- **Pod electronics.** An ESP32-C3-MINI-1 (the Beetle board for the very first build), an opaque 0.96" SSD1306-class OLED, a BMI270 IMU and a VEML7700 light sensor.
- **Cartridges.** Two snap-in optics cartridges. Cartridge A is a PMMA aspheric lens plus a wedged 30/70 plate. Cartridge B is a look-around eyepiece. Each has an M2-screw focus slide with about ±2 mm of travel.
- **Housing.** A black SLA housing, with MJF PA12 clips from JLC3DP, which quotes SLA from $0.30 per part and MJF from $1.00 ([JLC3DP](https://jlc3dp.com/capabilities/3d-printing-service)).
- **Puck.** A BQ24074 charger and a LiPo in a pocket puck. The puck keeps the battery warm, which matters because cold discharge can lose up to half of capacity at -10 °C ([XJTU study](https://scholar.xjtu.edu.cn/en/publications/%E9%94%82%E7%94%B5%E6%B1%A0%E6%80%A7%E8%83%BD%E4%B8%8E%E6%B8%A9%E5%BA%A6%E7%9B%B8%E5%85%B3%E6%80%A7%E7%9A%84%E5%9F%BA%E7%A1%80%E5%AE%9E%E9%AA%8C%E7%A0%94%E7%A9%B6/)). The charger's NTC input should block charging below freezing.
- **Tether.** A USB-C cable with every pin routed, so a later revision can carry DisplayPort alt mode without a new connector.
- **Mount.** Two to four N52 disc magnets for retention plus a keyed dovetail for registration. Each target gets an adapter: a temple clamp, a goggle-frame clip, or a breakaway helmet and brim mount. The breakaway matters because Giro and Specialized advise that helmet mounts should release on impact ([VitalMTB quoting manufacturer reps](https://www.vitalmtb.com/forums/The-Hub,2/Camera-Mounts-on-Helmets-Potential-Safety-Concern,8206)).
- **Weight target.** At most 25-30 g on the head. One study found nose-load discomfort turns sharply at about 60 g ([Du et al., AHFE 2022](https://openaccess.cms-conferences.org/publications/book/978-1-958651-23-0/article/978-1-958651-23-0_14)), and front-loaded mass is the worst case ([Chang et al.](https://pure.psu.edu/en/publications/effects-of-weight-balance-on-a-3d-tv-shutter-type-glasses-subject/)).
- **Orientation.** Mirror-mountable for either eye, because eye-dominance evidence gives no simple rule ([HAL Evry](https://hal-univ-evry.archives-ouvertes.fr/hal-02477329v1)).

**Phases.** All durations are estimates.

1. **Pod-0 bench optics and fit (weeks 1-6).** Build the pod and both cartridges, and fit-test them on the three mount targets.
2. **Pod-1 field unit (months 2-4).** A custom PCB, the three adapters, a goggle-interior variant of cartridge A, and Switchback calibration screens.
3. **Experimental branch (from month 3).** Approach 4, the FPGA micro-OLED pod, and approach 5, a Monado driver for Pod-1 that exposes 3DoF orientation from its IMU, tested against borrowed birdbath glasses.

The goal is for apps to target OpenXR before any binocular optics exist. The binocular hooks cost almost nothing if REV1 commits to them now:

- An IMU with a FIFO and a sync pin, rigidly mounted to the optics with its offset from the display documented.
- A rail standard that a second eye pod and a stereo camera bar can share, with 55-72 mm IPD travel.
- A per-unit calibration blob stored on the device.
- A small, versioned USB protocol alongside BLE.

**Role split.**

| Person | Owns | Tools and timing |
|---|---|---|
| 1, EE/firmware | Pod and puck PCBs, panel drivers, the protocol on the device, later the FPGA | KiCad; PCB spins every 4-6 weeks |
| 2, Optomech/industrial | Cartridges, the dock standard and adapters, fit and comfort testing, nits and eyebox measurements | Parametric CadQuery/OpenSCAD so users can print adapters for their own eyewear; weekly FDM loops, MJF/SLA orders every 2-3 weeks |
| 3, Software/systems | Switchback (pairing, calibration, mount profiles, bundles), the public protocol spec, the Monado driver, docs, BOM and sourcing alternates, FCC/CE homework | |

With only two people, person 3's work splits between the other two. Firmware absorbs the protocol and Monado. Optomech absorbs docs and sourcing.

## OpenWR principles turn into specific design rules

The history of the category makes the principles concrete. North Focals stopped working on **31 July 2020**, about a month after Google bought North and shut the servers off ([GearBrain](https://www.gearbrain.com/north-focals-smart-glasses-disabled-2646835849.html)). Recon Jet died with Intel's exit in 2017 ([Bicycle Retailer](https://bicycleretailer.com/product-tech/2017/07/21/intel-quietly-discontinues-recon-jet-smart-sunglasses)). Google Glass Enterprise Edition 2 kept working after support ended only because its apps were sideloadable Android ([Gigazine](https://gigazine.net/gsc_news/en/20230316-google-glass-enterprise-edition-discontinued)). Even OpenGlass, which was MIT-licensed, stranded users within about a year, and its app expected cloud LLM keys ([OpenGlass](https://github.com/BasedHardware/OpenGlass)). Four rules follow:

- **No server dependency.** Every feature works with zero servers.
- **A published protocol.** The BLE protocol is public and versioned, so anything can replace Switchback.
- **Owner keys.** Owners can install their own firmware signing key. goggl1's signed `.mnpkg` OTA and the OpenWR release keys already point this way.
- **Rebuildable from the repo.** Native CAD and KiCad sources, not the PDFs and STLs Brilliant Labs publishes for Frame ([Frame hardware docs](https://docs.brilliant.xyz/frame/hardware/)).

For licenses, use **CERN-OHL-P** for hardware, MIT or Apache-2.0 for firmware and the app, and CC-BY-4.0 for docs, and apply for OSHWA certification ([CERN-OHL v2](https://ohwr.org/project/cernohl/wikis/Documents/CERN-OHL-version-2), [OSHWA](https://certification.oshwa.org/requirements.html)).

The protocol needs one deliberate evolution. Today Switchback drives goggl1 entirely through Forth console lines over the Nordic UART Service. That is wonderfully hackable, but it is not a stable contract for third parties. ActiveLook, the display protocol behind Engo and Julbo sports glasses, shows the alternative: a documented GATT service with a 304x256 canvas, drawing primitives, and stored fonts, layouts, gauges and pages. Updates become short "display layout N with value" messages, governed by back-pressure on a control characteristic and correlated by query IDs. That openness is why Garmin, Suunto, Apple Watch and Navigr8 drive ActiveLook glasses without a phone app ([ActiveLook API](https://github.com/ActiveLook/Activelook-API-Documentation/blob/main/ActiveLook_API.md), [advnture](https://www.advnture.com/news/these-smart-shades-will-beam-stats-from-your-garmin-watch-in-front-of-your-eyes)).

Hiroglass should therefore ship two things: its own versioned retained-mode service, with the Forth console kept as the power-user channel, and optionally a second service that speaks the ActiveLook wire format. Two cautions apply to the second. The ActiveLook brand and badges sit behind a Brand License Agreement, so describe Hiroglass factually as "speaks the ActiveLook BLE protocol" and never use the name or logo unlicensed ([ActiveLook partnership](https://www.activelook.net/news-blog/activelook-eyewear-partnership)). And the API document carries no license, so do not copy its text. MentraOS, which is Apache-2.0 but built around a phone, a cloud component and an app store, belongs as a compatibility target rather than a foundation ([MentraOS](https://github.com/Mentra-Community/MentraOS)). Brilliant Labs' BSD-3 `frame_ble` Flutter package is directly useful reference code for Switchback's BLE layer ([pub.dev](https://pub.dev/packages/frame_ble/)).

Modularity has a physical form as well. The magnet-and-dovetail dock and the shared rail are the OpenWR "devices link together" principle in hardware, and they should be published as standards that other modules, such as lights, cameras or a second eye, can target. One regulatory caution: OSHA treats modified eye protection as still needing Z87.1 compliance ([OSHA](https://www.osha.gov/laws-regs/standardinterpretations/2007-01-29)). The construction variant should mount to the hardhat and leave safety eyewear unmodified, and nothing should be marketed as Z87-compatible without testing.

## Waveguides: a parallel experiment, not the REV1 product path

A follow-up check in October 2026 found the picture more open than "quote-only". A waveguide module can now be bought in quantity 1 without a sales call. What still needs a quote and an NDA is the bright green microLED projector that makes a waveguide readable outdoors. Nothing in this class meets the Mouser/DigiKey, multi-supplier rule yet.

**What you can buy today.**

- **DisplayModule DMGTX0023WGNA:** a geometric (reflective array) waveguide paired with a Sony 0.23" 640x400 micro-OLED, $499 at quantity 1 with 5-20% volume discounts. It gives a 26° FOV, a 10.8x5.4 mm eyebox and about 12 g, but **only up to about 400 nits at the eye**. Input is 24-bit parallel RGB or HDMI through a separate driver board, and the electrical datasheet is under NDA ([DisplayModule](https://www.displaymodule.com/products/dmgtx0023wgna-0-23-micro-oled-optical-waveguide-module-ar-smart-glasse)).
- **Alibaba, MOQ 1:** waveguide modules at roughly $400-590 and $1,600-2,100 from different sellers ([Alibaba](https://electronics.alibaba.com/supplier/waveguide-display-module-ar-glasses)). One green-microLED-class waveguide is listed openly at $800-1,000, MOQ 5, without its projector.
- **Quote or NDA only:** Goertek, Lochn, Cellid, Lumus, Dispelix, DigiLens, North Ocean and Tripole publish no prices. JBD's 0.13" green microLED projector has been in mass production since 2024, but no distributor, Tindie or Mouser listing was found ([eeNews](https://www.eenewseurope.com/en/jbd-offers-small-but-bright-microled-projector)).

**Why it is hard to make economical.** Diffractive waveguides pass only about 2% of the projector's light to the eye; geometric ones about 5-15% ([eLight](https://elight.springeropen.com/articles/10.1186/s43593-023-00057-z/tables/2), [KGOnTech 2025](https://kguttag.com/2025/05/29/exclusive-playnitride-single-chip-full-color-microled-lumus-waveguide-with-a-fantasically-tiny-projector/)). Snow needs thousands of nits at the eye, so only a green microLED projector gets there; a micro-OLED waveguide is an indoor display. The SPI OLED in goggl1 cannot feed a waveguide at all, and the ESP32-C3 cannot drive the micro-OLED's RGB or HDMI input, so a waveguide pod also forces an ESP32-S3-class MCU or a Linux puck. Diffractive types add rainbow glare, and prescription users need an inner insert.

**The cheapest credible experiments.**

- **Indoor optics feel, about $500-650:** the DisplayModule module plus a driver board, fed from an ESP32-S3 or a Pi. It answers what a waveguide looks like at 25 mm eye relief on a clip-on.
- **Outdoor-capable, about $460-500:** buy finished green microLED waveguide glasses and drive them as a display over BLE instead of rebuilding the optics. A used Even G1 (about $459 on eBay) has an official demo app that documents a Nordic UART link taking 1-bit 576x136 images; the G2 has a community-reverse-engineered protocol ([i-soxi/even-g2-protocol](https://github.com/i-soxi/even-g2-protocol)). The Vuzix Z100 (about $499) has an official SDK. This tests Switchback's protocol and the HUD content on outdoor-grade optics, with nothing to fabricate. Pulling the optics out of these glasses is not practical.
- **Part-level green microLED build:** needs a JBD quote and NDAs, likely over $5,000 in engineering samples, so it is out of scope for REV1.
- **DIY holographic combiners:** Litiholo kits (about $100-330) and Bayfol photopolymer film can record demo holographic combiners, and a 3D-printed, oil-filled geometric waveguide has been shown by a university group. Good for learning, not product quality.

**Outlook.** At 10k units, the parts imply roughly $50-150 for a waveguide plus engine (an estimate from JBD's "tens of dollars" panel guidance and $250-400 OEM glasses at MOQ 3,000). A rough guess, not a sourced forecast, is that commodity waveguide engines reach AliExpress and LCSC around 2027-28 and Mouser/DigiKey with second sources in 2028 or later. Keep the dock and the optics cartridge interface ready to accept a waveguide cartridge when that happens.

## Experiments to run before committing

The first experiment is a **brightness bake-off**. Measure the nits of the SSD1306, SSD1309 and RM67162 panels on hand, since none publish a figure. Then test cartridge A outside versus behind a category-3 lens, and cartridge B, against sunlit snow or a bright sky. This one experiment decides whether the commodity BOM can carry the snow use case, or whether the 3,000-4,000 nit Hicenda-class module has to come forward.

The second is an **eyebox and eye-relief test**. A clip-on in front of prescription glasses sits 20-30 mm from the eye, which shrinks the eyebox. The notes predict alignment will be the main user complaint, and no DIY build has published measured FOV, eyebox or nits, so Hiroglass would be first.

The third is a **fit matrix**: five glasses frames, three goggles (ski, moto, construction) and two helmets, testing pantoscopic tilt and wrap compensation and whether a re-mounted pod keeps its calibration.

Five smaller checks follow:

- **FPC pinouts.** Confirm whether the Winstar, Orient and Raystar SSD1306 FPCs share a pinout, which decides whether one carrier footprint is a true second source.
- **ActiveLook interop.** Load the Apache-2.0 Garmin ActiveLook data field against a Hiroglass that speaks the protocol, to see whether it rejects unknown Device Information strings ([Garmin sample](https://github.com/fsallstrom/ActiveLook-Dex-DF)).
- **FPGA-free panel drive.** Prove that RP2350 PIO or an iCE40 can drive a Sony ECX336 at 640x400.
- **ESP32-P4 DSI limits.** Check the ESP32-P4's MIPI DSI ceiling before betting on SeeYA panels.
- **Puck SoC choice.** Benchmark Basalt on an RK3588 and a Pi 5 using Monado's CC-BY SLAM datasets before choosing the puck SoC ([Hugging Face](https://huggingface.co/datasets/collabora/monado-slam-datasets/blob/d9f2c2c679e3bc1735bb4ba7eb76e2bb759f9c0c/README.md)).

Four open questions are worth tracking because they could change the plan:

- Has Sony EOL'd the ECX335 or ECX336?
- Do JBD's November 2026 kits reach buyers without an NDA?
- Can any birdbath glasses vendor accept a custom side-by-side stereo signal without a firmware toggle?
- Do helmet accessories affect CPSC, ECE or Snell certification? No source addressed this.

## Conclusion

The research changes where the risk in Hiroglass sits. An "XR wearable" sounds like a display-and-compute problem, but for a three-person open-hardware team the electronics are a solved $24 subsystem that goggl1 has already built and shipped OTA. The unsolved parts are a lens, a mirror and a clip: whether a commodity OLED can be read in sunlight, whether the eyebox survives 25 mm of eye relief, and whether one pod registers repeatably on glasses, goggles and helmets. Those questions are cheap to answer with printed parts and a luminance meter, and the answers decide whether REV1 stays on the multi-source BOM or has to accept a single-supplier micro-OLED.

The second insight concerns binocular. Spatial XR is less a future optics project than a future interface project. The open ecosystem has already converged on a USB-attached head unit, host-side rendering, and OpenXR through Monado. A team that routes the full USB-C connector, timestamps its IMU, stores calibration on the device and writes a 3DoF Monado driver for a monocular HUD in 2026 will find that binocular means adding a second eye and a camera bar, not starting over. The dock and protocol, published openly, are the parts most likely to outlast any particular display, and that is where OpenWR's principles pay off most directly.
