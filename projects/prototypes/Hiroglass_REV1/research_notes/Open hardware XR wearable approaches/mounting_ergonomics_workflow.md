# Mounting, Ergonomics, Calibration, and Small-Team Workflow for Clip-On HUD Accessories (Hiroglass REV1)

Research date: 2026-10-13. Research was cut short by a network outage, so a few areas (Forcite/CrossHelmet teardowns, patents, PCB fab comparisons, KiCad/CAD tool docs) were not fetched and are listed under Gaps. Inferences are the researcher's design reasoning, not sourced facts.

## Mounting mechanisms across glasses, goggles, and helmets (and a common interface)

### Takeaway
Commercial products split into two camps. Some build the HUD into the eyewear: Engo, Rekkie, and Recon Jet put the electronics in the frame or pods. Others clip a small module onto a magnetic or mechanical dock: Vufine+ uses a magnetic dock of about 8.5 g. Recon's Snow2 showed that one display/compute core can be moved between compatible goggles. For Hiroglass, a small universal dock (magnet plus keyed rail) with cheap per-target adapters is the most practical common interface.

### Cited Findings
- Vufine+ display weighs about 27 g. Its separate Magnetic Docking Station / Docking Station+ weighs 0.3 oz (8.5 g) and attaches to glasses; the display snaps on magnetically — [B&H Docking Station+](https://www.bhphotovideo.com/c/product/1333097-REG/vufine_vud_200_docking_station_plus.html); [B&H Docking Station](https://www.bhphotovideo.com/c/product/1333098-REG/vufine_vud_100_docking_station.html); [desertcart listing (27.22 g)](https://www.desertcart.pl/products/43341566)
- Recon Jet (2013): two pods of 14 g each (28 g total) attached to a 32 g frame. A connection runs along the top of the frame, and the glasses can be worn without the pods. Pods were weight-matched "to ensure balance" (compute on one side, battery pod on the other, with charging through the compute pod). Replacement battery pods were expected to cost $30–50 — [DC Rainmaker](https://www.dcrainmaker.com/2013/08/endurance-sports-display.html)
- Recon Snow2: owners of earlier Recon units could swap the internals into their existing compatible goggles for $399 instead of buying new goggles. This is precedent for a removable HUD core that docks into a goggle frame pocket — [DC Rainmaker](https://www.dcrainmaker.com/2013/08/endurance-sports-display.html)
- Engo: the display electronics are integrated into the frame. The "tech package" weighs 13 g. Whole-glasses weights are reported as 36–41 g for Engo 2, 28.9 g for a titanium (3D-printed temple) model, and under 40 g for the third generation — [Slowtwitch](https://www.slowtwitch.com/Products/Electronics/Getting_Started_with_Engo_Eyewear_8673.html); [TCT Magazine](https://www.tctmagazine.com/new-engo-smart-glasses-feature-3d-printed-titanium-temples/); [Pulse2](https://pulse2.com/engo-eyewear-raises-e5-million-to-expand-augmented-reality-sports-glasses/)
- Rekkie: HUD integrated in a ski goggle, about 9 oz total, 10–15 h battery, LiPo charged over USB-C (3–4 h to full). Reviewers report it is hard to read in bright sun — [GearJunkie](https://gearjunkie.com/winter/augmented-reality-smart-goggles-rekkie-review). 3D printing was used to integrate the AR module into the goggle — [3DPrinting.com](https://3dprinting.com/news/3d-printing-used-to-integrate-ar-in-ski-goggles)
- Helmet mounts: Giro and Specialized say camera/light mounts should break away on impact. Specialized's rep said hard-mounting or bolting a device "may increase risk by adding leverage to rotation." GoPro says its adhesive mounts are not designed to survive significant impact — [VitalMTB forum, quoting manufacturer reps](https://www.vitalmtb.com/forums/The-Hub,2/Camera-Mounts-on-Helmets-Potential-Safety-Concern,8206); [road.cc](https://road.cc/content/news/133135-did-michael-schumacher’s-helmet-cam-cause-brain-injury)

### Inferences
- **Common interface proposal:** a small "Hiroglass dock" with 2–4 N52 disc magnets (Digikey/Mouser stock neodymium discs) for retention, plus a keyed dovetail or T-slot for registration, so the magnets hold and the rail locates. Each target gets a cheap adapter:
  - (a) glasses: a temple-arm clamp. Options are a TPU sleeve, a two-screw clamp, or a spring clip in the Vufine style.
  - (b) goggles: a frame-edge clip, or a strap-buckle plate positioned at the outrigger.
  - (c) helmets/hats: a visor or brim clip, or a GoPro-style two-prong adapter on a breakaway (magnet or sacrificial) interface.
- Magnetic retention doubles as a breakaway feature for helmet use, which is consistent with the manufacturer statements above.
- The Recon Jet battery-opposite-compute layout is the precedent for counterbalancing. For Hiroglass, the tethered puck removes most battery mass from the head, which is a strong advantage over integrated products.
- GoPro's two-/three-prong interface is a de facto open accessory standard with plenty of off-the-shelf adapters. It is an option for helmets, but its rigidity argues for a breakaway element.

### Gaps
- No teardown or mount detail was found for Forcite, CrossHelmet, Everysight Raptor, or Vuzix clip-on designs (not fetched due to time/outage).
- Recon Snow2's goggle-pocket mechanics (frame cutout geometry, connector) were not documented in the sources found.
- No patents on clip-on HUD mounts were reviewed.
- No Printables/Thingiverse/Hackaday clip designs were surveyed.

## Ergonomics: weight, balance, eye placement, safety/PPE

### Takeaway
Nose-bridge load drives discomfort more than total mass. One study found the discomfort inflection at about 60 g on the nose and 1.2 N temple clamping force, and front-loaded mass is worst. The display should sit outside an approximately 8° radius of the primary gaze direction. One paper recommends a lateral (temporal) offset over placement below the line of sight for social and everyday use, though car-HUD work favors about 5° below. Adding an accessory to Z87.1 eyewear likely takes it out of documented compliance unless the combination is tested.

### Cited Findings
- AR-glasses study (AHFE 2022): nose loads of 30–75 g and clamping forces of 0.3–1.5 N were tested over 1 h of wear. The inflection point of subjective discomfort was a 60 g nose load and 1.2 N head-side clamping force — [Du et al., AHFE 2022](https://openaccess.cms-conferences.org/publications/book/978-1-958651-23-0/article/978-1-958651-23-0_14)
- Weight distribution (3D shutter glasses): front-of-temple mass produced the highest nose load (0.34 N), versus 0.22 N middle, 0.20 N ear, and 0.16 N rear. Discomfort fell as mass moved rearward — [Chang et al., Int. J. Industrial Ergonomics (PSU)](https://pure.psu.edu/en/publications/effects-of-weight-balance-on-a-3d-tv-shutter-type-glasses-subject/)
- Heavier headsets (500–600 g range): the less-comfortable quartile lost about 11 min of comfortable wear per 33 g added. Females experienced earlier and more severe discomfort — [Work journal 2024](https://dx.doi.org/10.3233/wor-220689)
- Benchmarks: lightweight everyday monocular HWDs (Vuzix Z100, Even Realities G1) weigh under 40 g total — [arXiv 2505.09047](https://arxiv.org/html/2505.09047v1). Engo 2 weighs 36–41 g total — [Slowtwitch](https://www.slowtwitch.com/Products/Electronics/Getting_Started_with_Engo_Eyewear_8673.html). A vendor blog claims under 50 g is comfortable for long wear, but this is unsupported — [RayNeo blog](https://www.rayneo.com/blogs/news/smart-glasses-daily-wear-weight-comfort)
- Monocular placement (right-eye study, arXiv 2025):
  - Keep pixels outside an 8° radius of the primary position of gaze (PPOG). Google Glass's bottom edge sits about 8° above line of sight.
  - Recommended region is vertically centered, +8.7° to +23.7° toward the ear (+19.6° conservative for extended use, up to +30° for brief glances).
  - Corner and top positions performed worst.
  - Below-line-of-sight placement was rated socially awkward but significantly reduced binocular rivalry in one cited study.
  - 15° horizontal FOV suits text; 10° was too narrow for captions.
  - Frame pantoscopic tilt (5–15°) and wrap (4–11°) shift the virtual image, so a clip-on must compensate per frame.

  Source: [arXiv 2505.09047](https://arxiv.org/html/2505.09047v1)
- Car HUD study: monocular HUD viewing missed more information and gave slower responses than binocular. The optimum location was about 5° diagonally down from line of sight — [Univ. of Gloucestershire eprint](https://eprints.glos.ac.uk/604/)
- Eye dominance: aviation work (Bayle et al.) distinguishes sighting from sensorial dominance and concludes a more comprehensive test is needed before eye choice can be settled. There is no simple rule that "the dominant eye is best" — [HAL Evry](https://hal-univ-evry.archives-ouvertes.fr/hal-02477329v1); [HAL 02639457](https://hal-univ-evry.archives-ouvertes.fr/hal-02639457)
- Recon Jet was right-eye only at launch, and Recon said there was no left option "at this time" — [DC Rainmaker](https://www.dcrainmaker.com/2013/08/endurance-sports-display.html)
- PPE: OSHA's 2007 interpretation says modified eye protection must still meet ANSI Z87.1 to comply with 29 CFR 1910.133. Z87.1-2025 is current. The Z87 marking signals conformance of the protector as placed in service — [OSHA interpretation 2007-01-29](https://www.osha.gov/laws-regs/standardinterpretations/2007-01-29); [ANSI blog Z87.1-2025](https://blog.ansi.org/ansi/ansi-isea-z87-1-2025-safety-glasses-eye-protection/)

### Inferences
- **Weight budget:** the on-head module plus adapter should stay at or under about 25–30 g, with most mass near or behind the hinge. Added to a 25–35 g frame, that keeps total nose load under the roughly 60 g inflection. The tethered puck is how this becomes achievable.
- Route the tether down the temple and behind the ear toward the collar or neck, so cable weight pulls rearward rather than on the hinge. Add strain relief and a clip at the collar.
- Design the module to be **mirror-mountable**: the same module flipped 180°, with the display content rotated in software. This supports left/right eye choice, since dominance evidence is unsettled. Let users choose.
- Default placement: temporal-upper or temporal-center, at least 8° off gaze. For skiing and cycling, a below-gaze option (about 5–10° down) may suit glanceable speed data better. The adapter should allow both.
- **PPE:** do not market the construction variant as Z87.1-compatible without testing. A safer approach is a helmet- or hardhat-mounted variant that leaves the protective eyewear unmodified, plus a statement that a clip-on to Z87 glasses may void compliance.
- **Heat:** keeping the SoC and battery in the puck limits on-face heat to the display driver and backlight.

### Gaps
- No quantitative skin-temperature limits for face-worn devices were sourced. IEC 62368-1 touch-temperature limits are a likely reference but were not verified.
- No source addressed whether helmet accessories affect CPSC, ECE, or Snell helmet certification. A Giro rep mentioned an ASTM subcommittee, with no details found.
- No study was found that directly compares above- vs. below-gaze monocular placement for sports.

## Adjustment and calibration

### Takeaway
Commercial products combine coarse mechanical fit with app-based digital micro-adjustment. Engo walks users through positioning in the app, then offers "micro-adjustment" of the data display, and testers often still need the fine-tune step. Proper eye placement matters: Recon Jet required refocusing when it was not "just right."

### Cited Findings
- Engo setup: the app prompts display positioning on first power-up, and a third app tab provides micro-adjustment of the data display. A reviewer found it slightly off after initial setup. Engo 2 arms bend by hand and the nose piece adjusts; Engo 1 arms required a screwdriver — [Slowtwitch](https://www.slowtwitch.com/Products/Electronics/Getting_Started_with_Engo_Eyewear_8673.html)
- Recon Jet: when positioned "just right" it works well, otherwise the wearer must refocus. The display was designed to be just inside peripheral vision — [DC Rainmaker](https://www.dcrainmaker.com/2013/08/endurance-sports-display.html)
- Frame pantoscopic and wrap angles shift the virtual image vertically and horizontally — [arXiv 2505.09047](https://arxiv.org/html/2505.09047v1)

### Inferences
- **Mechanical degrees of freedom:**
  - Coarse: x (fore-aft along the temple, via a rail with detents), y (vertical, via a slotted adapter or adapter variants), z (eye relief).
  - Rotational: pitch (to cancel pantoscopic tilt) and yaw (to cancel wrap).
  - Roll can be handled in software.
  - A friction ball joint (for example 6–8 mm, or an off-the-shelf mini ball head) covers pitch, yaw, and roll in one part but is less repeatable. A rail plus a single-axis friction hinge is more repeatable for re-mounting.
- **Software calibration:** image x/y offset (sub-window within a larger panel, as Engo-style micro-adjust does), rotation/roll, keystone, brightness with ambient-light auto, mirroring for left/right eye, and a stored per-mount profile (glasses, goggle, helmet).
- **Focus:** a fixed-focus optic (virtual image at about 1–3 m) avoids diopter mechanisms. Prescription users wear the module over their own glasses, so diopter correction comes free in the glasses variant. Goggle and helmet variants may need a small slide-focus.
- Users should be able to re-attach the module to a calibrated adapter without redoing calibration. That argues for keyed registration rather than a free magnet.

### Gaps
- Vuzix, Everysight, and Vufine user-calibration procedures were not documented in the fetched sources.
- No IPD-adjustment data was gathered for monocular clip-ons.

## Environmental: cold, fog, water, vibration, impact

### Takeaway
Cold is the dominant ski risk. Li-ion delivers much less capacity below 0 °C and must not be charged below freezing, which favors keeping the battery in a puck under clothing. Impact safety on helmets argues for breakaway mounts.

### Cited Findings
- Li-ion cold performance:
  - Charge capacity dropped about 12% between 20 °C and -10 °C in one study.
  - 2C discharge capacity fade at -10 °C reached as much as 50%.

  Source: [Xi'an Jiaotong Univ. study](https://scholar.xjtu.edu.cn/en/publications/%E9%94%82%E7%94%B5%E6%B1%A0%E6%80%A7%E8%83%BD%E4%B8%8E%E6%B8%A9%E5%BA%A6%E7%9B%B8%E5%85%B3%E6%80%A7%E7%9A%84%E5%9F%BA%E7%A1%80%E5%AE%9E%E9%AA%8C%E7%A0%94%E7%A9%B6/)
- Cycling at 0 °C caused significant lithium deposition and reduced thermal stability. Losses at -20 °C were largely reversible on warming — [Univ. of Surrey](https://openresearch.surrey.ac.uk/esploro/outputs/journalArticle/Effect-of-low-temperature-aging-on/991074223902346)
- Subfreezing charging can plate metallic lithium permanently. This is a Battery University warning, quoted second-hand — [BackpackingLight forum](https://backpackinglight.com/forums/topic/do-rechargeable-lithium-ion-batteries-lose-charge-in-the-cold/)
- Rekkie: a reviewer expects below-freezing temperatures to reduce battery life, while a retailer claims 10 h "even in very cold conditions" (marketing) — [GearJunkie](https://gearjunkie.com/winter/augmented-reality-smart-goggles-rekkie-review); [Snowemotion](https://www.snowemotion.com/en/ski-goggles/10308-hud-rekkie-ski-goggles.html)
- Helmet impact: breakaway mounts are advised by Giro and Specialized, and rigid mounts may add rotational leverage — [VitalMTB](https://www.vitalmtb.com/forums/The-Hub,2/Camera-Mounts-on-Helmets-Potential-Safety-Concern,8206)

### Inferences
- **Battery in the puck:** keep it inside a jacket pocket and use a charge-temperature cutoff (the charger IC's NTC input blocks charging below 0 °C). This is standard in Li-ion charger ICs such as TI BQ2407x/BQ25xxx.
- **Fogging:** a goggle-mounted module outside the goggle lens avoids interior fog. A module inside the goggle needs anti-fog coating and must not block vents.
- **Water and sweat:** target IP54-ish for the head module, with a conformal-coated PCB and gasketed or ultrasonically sealed housing. Avoid exposed pogo pins on the head side, or use magnetic pogo connectors rated for wet use.
- **Vibration (dirt bike):** use a positive mechanical lock (detent or latch) in addition to magnets, threadlocker on screws, strain-relieved tether, and avoid heavy cantilevered optics.
- **Cold plastics:** PETG and PLA become brittle in cold. Nylon (PA12/PA11) and TPU stay tougher (see Fabrication).

### Gaps
- No sourced fogging test data or IP-rating practices for HUD products were found.
- No sourced vibration specs (for example MIL-STD-810 or ISO 16750 profiles) for helmet electronics were gathered.

## Fabrication: 3D printing, materials, molding, hardware

### Takeaway
For a 2–3 person team, the practical route is fast FDM in-house iteration, then outsourced MJF PA12 (functional clips and housings) and SLA (optical/cosmetic parts) from JLC3DP/PCBWay at a few dollars per part, and TPU for compliant clamps and pads. Injection molding only pays off at volume.

### Cited Findings
- JLC3DP processes:
  - SLA resins (Ledo 6060, Black, Transparent 8001, about 10 more): tolerance to ±0.2 mm, from $0.30, about 2-day build.
  - MJF PA12/PA11/PAC: tolerance to ±0.3 mm (±0.4% above 100 mm), from $1.00, about 3-day build.
  - FDM in ABS/ASA/PLA/PA12-CF/TPU: ±0.3 mm, from $1.00.

  Source: [JLC3DP](https://jlc3dp.com/); [JLC3DP capabilities](https://jlc3dp.com/capabilities/3d-printing-service)
- Engo used 3D-printed titanium temples in a production product — [TCT Magazine](https://www.tctmagazine.com/new-engo-smart-glasses-feature-3d-printed-titanium-temples/). Rekkie used 3D printing to integrate the AR module into goggles — [3DPrinting.com](https://3dprinting.com/news/3d-printing-used-to-integrate-ar-in-ski-goggles)

### Inferences
- **Material map:**
  - Clips and clamps: MJF PA12 (tough, good snap-fit fatigue, cold-tolerant), or in-house PETG/PA-CF for prototypes.
  - Grip pads and compliant temple sleeves: TPU 85–95A.
  - Display/optics housings and light-tight parts: black SLA resin, avoiding brittle standard resins for snap features.
- Off-the-shelf hardware: neodymium disc magnets, M1.6/M2 screws with heat-set inserts, GoPro-pattern thumbscrews, mini ball-head mounts, and magnetic pogo connectors (Mill-Max/CUI on Digikey) for the tether.
- MJF at a few dollars per part means injection molding (typically thousands of dollars per aluminum tool) only pays off in the hundreds-to-thousands quantity range. This is a general rule of thumb, not sourced here.
- **Parametric adapters:** clips should be CadQuery/OpenSCAD scripts parameterized by temple width/thickness, goggle frame lip, and helmet visor thickness. Users then print one for their own eyewear, which maximizes "few custom parts" compatibility.

### Gaps
- PCBWay, Shapeways, and Xometry material and price comparisons were not fetched.
- Injection molding break-even figures were not sourced.

## Small-team workflow: tools, fab, licensing, certification, regulatory

### Takeaway
Use a fully open toolchain (KiCad, FreeCAD/CadQuery/OpenSCAD, Git), a permissive hardware license (CERN-OHL-P fits the "permissive" requirement), and OSHWA certification for credibility. Use a pre-certified BLE module so that FCC/CE work reduces mainly to "Contains FCC ID" labeling plus host unintentional-radiator compliance.

### Cited Findings
- CERN-OHL v2 has three variants:
  - **P (permissive):** keep notices; MIT/BSD-like.
  - **W (weakly reciprocal):** modifications stay open, but larger designs incorporating the part need not be.
  - **S (strongly reciprocal):** incorporating designs must be released under it.

  Sources: [CERN OHL v2 (ohwr.org)](https://ohwr.org/project/cernohl/wikis/Documents/CERN-OHL-version-2); [Weinberg 2025](https://michaelweinberg.org/blog/2025/08/08/which-open-hardare-license/)
- OSI approved the CERN-OHL family in January 2021 — [OSI license-review list](https://lists.opensource.org/pipermail/license-review_lists.opensource.org/2021-January/005066.html)
- OSHWA certification:
  - Requirements: compliance with the OSHW definition, open licenses for hardware, software, and docs, and links to all documentation.
  - Closed third-party parts are allowed (best efforts to identify them).
  - Each product bearing the mark must be registered, and each version can be certified separately.
  - The requirements page states certification lasts one year with annual renewal.
  - No fee is mentioned.

  Sources: [OSHWA requirements](https://certification.oshwa.org/requirements.html); [OSHWA application](https://application.oshwa.org/apply)
- FCC modular approval for the host:
  - Label the host "Contains FCC ID: …".
  - Use only the approved antenna types and gain.
  - Keep RF parameters within the grant.
  - Evaluate co-location and RF exposure.
  - RF hardware or firmware changes may need a permissive change.
  - The host maker retains regulatory responsibility.
  - Limited modular approvals require host-level evaluation.

  Source: [IB-Lenhardt KB](https://ib-lenhardt.com/kb/fcc-requirements/modular-approval)

### Inferences
- **Role split for 2–3 people:**
  - (1) EE/firmware: KiCad, the module/puck PCBs, BLE, display driver.
  - (2) Mechanical/industrial design: CadQuery/FreeCAD/OnShape, adapters, fit testing, materials.
  - (3, if present) App/software calibration, docs, and compliance/sourcing.
  - With two people, merge 3 into 1 and 2.
- **Cadence:**
  - Mechanical: weekly FDM loops in-house; MJF/SLA orders every 2–3 weeks.
  - PCB: spins every 4–6 weeks via JLCPCB/PCBWay assembly (EU alternatives: Aisler; small US boards: OSH Park). Not sourced here.
- **Version control:** keep KiCad and CadQuery/OpenSCAD source (text-diffable) in Git, export STEP/STL/Gerbers in CI or at release tags, and keep a BOM with Mouser/Digikey part numbers. OnShape's free tier makes designs public, which fits open hardware, but it is cloud-only and not git-native.
- **Licenses:** CERN-OHL-P for hardware, MIT or Apache-2.0 for firmware and app, CC-BY-4.0 for docs. CERN-OHL-P is permissive and OSHWA-compatible. Solderpad is an Apache-derived alternative.
- **Regulatory:** pick a module with a full (not limited) modular grant and integrated or pre-approved antenna. For FCC, the host still needs Part 15B unintentional-radiator compliance (SDoC). For EU, the CE RED applies to the whole product even with a certified module (assessment can leverage the module's reports). Both points are general knowledge and were not verified in fetched sources; this applies especially to EU RED cybersecurity requirements (EN 18031), which have applied since Aug 2025.

### Gaps
- JLCPCB/PCBWay/OSHPark/Aisler capability and price comparisons were not fetched.
- Host Part 15B testing requirements and EU RED module-integration rules were not confirmed from primary sources in this session.
- No sourced case studies on 2–3 person hardware team cadence were gathered.
- OSHWA's annual-renewal statement comes from the fetched page summary and should be re-verified directly.
