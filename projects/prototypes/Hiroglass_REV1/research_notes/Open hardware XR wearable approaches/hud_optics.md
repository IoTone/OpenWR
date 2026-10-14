# Low-cost optical architectures for a clip-on monocular HUD (Hiroglass REV1)

Research date: 2026-10-13. Many primary sources are older (2021-2024). The dates are noted where they matter. Several web lookups failed or turned up nothing (no Google Glass teardown numbers, no ActiveLook datasheet, no Recon Jet data, no AliExpress prices for birdbath modules), and these are listed under Gaps. They should not be filled in from memory.

## Which architectures are realistic for a 2-3 person team, and which are out of reach?

### Takeaway
There are three realistic options for REV1. (1) A flat beam-splitter or combiner plate with a single magnifier/collimating lens, which is the cheapest and fully DIY. (2) An off-the-shelf micro-OLED with prism or birdbath eyepiece module, which you buy rather than build (about $100-$400 per unit, and few vendors). (3) A look-around (occluded) display with a small eyepiece below or beside the eye, Vuzix M400 style, which wins on outdoor brightness. A solid freeform prism or birdbath designed from scratch is hard but possible with outsourced molding. Waveguides (diffractive or reflective) are out of reach to fabricate. They can only be bought as vendor modules, which today means quote-only and single-supplier.

### Cited Findings
**A. Flat beam-splitter or combiner plate (+ lens), the DIY baseline**
- DIY guides call a semi-transparent film at 45 degrees in front of the eye the easiest route. They recommend a black 3D-printed or black-acrylic housing to suppress internal reflections. The same guides call waveguides "extremely difficult to fabricate outside a professional setting" — [inairspace guide](https://inairspace.com/blogs/learn-with-inair/heads-up-display-glasses-diy-your-ultimate-guide-to-building-smart-eyewear) (a low-authority blog. Treat as consensus, not evidence).
- Arduino blog build (2021): a 0.49" OLED driven by an Arduino Nano 33 BLE over I2C. A lens and mirror project the screen onto a clear phone case lined with car HUD film. The frame is off-the-shelf, the STLs are published, and the software was unfinished — [Arduino blog](https://blog.arduino.cc/2021/08/27/giving-google-glass-a-run-for-its-money/).
- Hackaday.io "Smartglass": ESP32 with BLE to a phone. A 0.96" OLED is "magnified with a lens and projected on the acrylic glass screen" — [hackaday.io/project/92176](https://hackaday.io/project/92176-smartglass).
- MohammedMMC/Smart-Glasses: ESP32-C3 + 128x64 SSD1306 + BLE5 (time, calls, texts). This is almost exactly goggl1's electronics class — via [search summary](https://hackaday.io/project/92176-smartglass) (repo name only. I did not fetch it).
- XPrism (Tsinghua): ESP32-PICO-D4, GPL-3.0, with an Android controller — [GitHub open531/XPrism](https://github.com/open531/XPrism/blob/main/README.en.md).
- Teen-built TOLED smart glasses (transparent OLED used directly, no combiner) — [Hackster](https://www.hackster.io/news/this-16-year-old-made-his-own-pair-of-toled-smart-glasses-790d0f2e1420) (not fetched. Note that a TOLED right in front of the eye cannot be focused on without a lens).
- Edmund Optics 25 mm 50R/50T plate beamsplitters: $61 (float glass, 1 mm, #43-736). The VIS-coated version is $136 at qty 1-5 and $108 at qty 6-25 (#35-942/#45-852) — [Edmund #43-736 class listing](https://www.edmundoptics.com/p/25mm-diameter-50r50t-plate-beamsplitter/5240/), [Edmund VIS #6279](https://www.edmundoptics.com/p/25mm-dia-50r50t-vis-plate-beamsplitter/6279/). Undated cached prices.
- Flat partially reflecting plates produce "parasitic images such as reflections from the rear face", and such collimators often have a limited field — [US4874214](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/4874214). Wedged plates exist for this reason (Edmund sells a 25 mm 50R/50T wedged plate — [Edmund EU #3784](https://edmundoptics.eu/p/25mm-diameter-50r-50t-vis-wedged-plate-beamsplitter/3784)).
- Beam-splitter cube variant: Hackaday "DIY 2-way holographic display" uses a 40 mm glass cube. The cube's 45 degree inner face transmits half the light and reflects half — [hackaday.io/project/205891](https://hackaday.io/project/205891-diy-2-way-holographic-display). HoloCubic (ESP32 + 240x240 TFT + beam splitter) notes that the splitter and OCA glue are hard to source and costly — [summary](https://www.neura.market/ai-tools-directory/all/peng-zhihui-holocubic).

**B. Birdbath ("look-through-the-beamsplitter", Google Glass / Monocle / Frame)**
- Guttag: Monocle and Frame are non-polarized 50/50 beam splitter + curved-mirror birdbaths. Monocle uses two solid optical parts and Frame uses three. Glass is "perhaps the most famous product with this birdbath type". Light is split twice, so about 25% reaches the eye — [KGOnTech, May 2024](https://kguttag.com/2024/05/09/brilliant-labs-frame-ar-with-ai-glasses-a-little-more-on-the-apple-vision-pro/).
- Monocle/Frame use a Sony 0.23" 640x400 micro-OLED rated 3,000 nits, so at most about 750 nits reaches the eye ("likely less"). Guttag later said about 700 nits is "fine indoors but marginal outdoors". The FOV is 20 degrees, and the image sits below and off-center from the line of sight — [KGOnTech](https://kguttag.com/2024/05/09/brilliant-labs-frame-ar-with-ai-glasses-a-little-more-on-the-apple-vision-pro/).
- Birdbath problems Guttag lists: a visible out-of-focus seam at the splitter edge, forward light leakage ("undesirable in social situations") with no polarizer, and "a linear change in FOV results in a cubic increase in the volume of material". Polarizing beam splitter + quarter-wave plate designs (a Raontech reference) recover efficiency and cut leakage — [KGOnTech](https://kguttag.com/2024/05/09/brilliant-labs-frame-ar-with-ai-glasses-a-little-more-on-the-apple-vision-pro/).
- Commercial binocular birdbath modules from DisplayModule: 0.7" 1920x1080, 42 degree FOV, $1,499. The 47 degree version is $999 — [DisplayModule 42 deg](https://www.displaymodule.com/collections/all-products/products/birdbath-1920rgb-x-1080-araugmented-reality-optical-module-fov-42-degree-lvds), [47 deg](https://www.displaymodule.com/collections/all-products/products/binocular-araugmented-reality-glasses-birdbath-1920-1080-47-fov-optical-module-with-lvds). These are far over budget and need LVDS/MIPI hosts.

**C. Prism eyepiece / small prism combiner (Glass-style, LILYGO T-Glass, Vufine)**
- LILYGO T-Glass: an ESP32-S3 dev platform with a 1.1" LTPS AMOLED (JD9613, 294x126 native). The prism limits the usable area to 126x126 px. It has a 3D-printed prism holder and costs about $48 on Amazon. It was out of stock at AliExpress and the LILYGO store (mid-2024) — [CNX Software](https://www.cnx-software.com/2024/07/01/lilygo-t-glass-esp32-powered-smart-glasses-with-1-1-inch-prism-display-imu-microphone/?amp=1).
- Vufine+: 960x540 LCOS with custom optics, giving a virtual 4" screen at about 12" (about 30 cm). It is monocular and clips to glasses in front of the right eye — [B&H](https://www.bhphotovideo.com/c/product/1331245-REG/vufine_vufine_a_high_definition.html), [BetaNews review](https://betanews.com/review/vufine-wearable-display-review/). No teardown found.
- Prism patents exist for molded-plastic near-eye prisms (weight reduction) — [US8125716](https://patents.google.com/patent/US8125716).

**D. Look-around / occluded display (Vuzix M-series, Recon Jet class)**
- Vuzix M400: display type "Occluded", 640x360 OLED, 16.8 degree diagonal FOV ("5 in. screen at 17 in."), >2000 nits, >10,000:1 contrast, left or right eye — [Vuzix compare](https://vuzix.com/en-ca/pages/compare-smart-glasses), [Vuzix M400](https://vuzix.com/products/M400-Smart-Glasses).

**E. Off-axis reflective combiner / visor reflection (motorcycle helmet HUDs)**
- Classic helmet HUD: a microdisplay + projection lens, with the display at the lens focal plane so the image is at infinity. The combiner is a semitransparent beam-splitting coating on the visor — [US10884245](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/10884245), [US10573271](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/10573271).
- Using the visor itself as the combiner/collimator saves weight, but you can no longer move the display to fit each wearer's eye position — [US5543968](https://patents.google.com/patent/US5543968).
- Shoei Opticson uses a separate combiner on an adjustable flip-up bracket rather than the visor — [MCN](https://motorcyclenews.com/news/new-tech/shoei-opticson-hud-display-helmet).
- A cemented convex/concave meniscus combiner cancels power for the see-through view — [US7397606](https://patents.google.com/patent/US7397606).

**F. Waveguides (out of reach to build)**
- JBD/WaveOptics "Leopard" kit (2021): 0.13" JBD green microLED VGA, 16 gray levels, about 4M nits, Katana diffractive waveguide, aimed at monocular notification glasses — [Display Daily](https://displaydaily.com/?p=76305), [Yole](https://www.yolegroup.com/industry-news/ar-smartglass-supplier-partnership-to-deliver-the-next-generation-of-ar-wearables/).
- JBD monochrome projector dev kit: about 0.35 cc, 30 degree FOV, about 0.6 g, in R, G or B — [LEDinside](https://www.ledinside.com/node/32441). No public prices found. JBD and Dispelix reference designs "at multiple price points" also have no published prices — [LEDinside](https://ledinside.com/node/view/32728).
- Guttag: because of pupil expansion and losses, "only a tiny fraction of the light-in makes it to the eye". Lumus claims its reflective waveguides are 5 to more than 10x more efficient than diffractive ones (a vendor claim) — [KGOnTech 2023](https://kguttag.com/2023/03/12/microleds-with-waveguides-ces-ar-vr-mr-2023-pt-7/).

### Inferences
Summary comparison. The numbers are from the cited sources where given. Cells marked (est.) are first-principles estimates, not measurements.

| # | Architecture | FOV | Eyebox | Brightness to eye | See-through | Weight/size | Collimation | Unit cost (proto) | DIY feasibility | Sourcing risk |
|---|---|---|---|---|---|---|---|---|---|---|
| A | Flat plate combiner + 1 lens + small OLED/LCD | 10-25 deg (est., set by panel size / lens f) | Medium, set by lens aperture (est. 6-10 mm with a 15-20 mm lens) | Panel nits x R (R = 20-50%) | Good (T = 50-80%). Ghosting from the rear face unless wedged/AR-coated | 10-25 g optics (est.). Bulky in depth along the 45 deg fold | Display just inside the lens focal length. Easy to make adjustable | $5-$30 (AliExpress acrylic/glass splitters, PMMA aspheric lenses). $61-$136 for Edmund glass | High. Weeks | Low. Many suppliers for plates, lenses and OLEDs |
| B | Birdbath (cube or solid two-piece) | 20 deg (Monocle) | Small-medium | About 25% of panel (non-polarized). Up to about 2x with PBS+QWP | Good, but seam lines and forward leakage | Volume grows with FOV^3 | Built into the curved mirror | Buy: $100s-$1,500 modules. DIY from a cube + concave mirror is possible | Medium (DIY bench version). Low (production quality) | Medium-high for modules |
| C | Prism eyepiece / small prism (T-Glass, Vufine, Hicenda kits) | Narrow. T-Glass limited to 126x126 px | Small (must align to the pupil) | Panel nits x prism efficiency (unknown) | Partial (prism sits in view, at its edge) | Light, compact | Fixed in the module (Vufine about 30 cm, near) | $48 (T-Glass, whole dev kit) to $184-$415 (Hicenda panel+prism+board) | High if buying a module | Medium. Few sellers, stock churn |
| D | Look-around occluded (Vuzix M400 style) | 15-20 deg | Small | About 100% of panel (no combiner loss) | None in the display area. You glance at it | Light, compact | Eyepiece lens. Easy diopter adjust | $20-$60 (OLED + eyepiece) (est.) | High | Low |
| E | Off-axis visor/goggle-lens reflection | 10-20 deg (est.) | Medium-large if collimated | Panel x visor reflectance (4% bare, more with a coating/film) | Excellent | Projector off to the side. Optics get large | Projection lens with display at focal plane. Off-axis curved visor causes distortion/astigmatism | $10-$40 + HUD film | Medium. Each goggle/visor geometry differs | Low |
| F | Waveguide (diffractive/reflective) | 20-30 deg | Large | Thousands of nits only with microLED or LCOS | Excellent | Thinnest | Built into the waveguide | Quote-only. Not public | Not feasible to fabricate. Integrate only | High. Single supplier |

- REV1 recommendation (inference): run two parallel tracks. Track 1 is A: goggl1's existing OLED + PMMA aspheric + an AR-coated or wedged plate in a black printed housing, as the open, multi-source baseline. Track 2 is D: a look-around eyepiece for the snow/sun case, because it skips the combiner loss entirely. Test C (T-Glass, Hicenda kit) as a quick reference for "what a bought module feels like". Do not plan REV1 around B production modules or F.
- Clip-on geometry favors A and D. A thin plate or a small eyepiece can hang below or beside the eye from a clip. E is attractive for helmets and goggles specifically, because the large curved goggle lens is already in front of the eye, but every goggle model has different curvature, which defeats "one clip fits all".
- goggl1-class panels (SSD1306 128x64 0.96"/0.49", I2C) are monochrome, typically a few hundred nits (est.). They are adequate for indoor/overcast A builds and not for snow with a 50% combiner. That points to either a brighter panel or architecture D for outdoor use.

### Gaps
- Google Glass prism numbers (FOV, virtual image distance, efficiency): searches found no teardown with measurements. Only a hands-on piece came up ([Android Central](https://www.androidcentral.com/under-microscope-google-glass)).
- Recon Jet optics: no source found.
- Freeform prism (true off-axis freeform, e.g. Epson/Olympus style) costs and DIY routes: no sources found. Outsourced injection molding is an inference, not sourced.
- No waveguide module price is public. JBD/Dispelix/Lumus are sales-quote only.

## Off-the-shelf "HUD optical engine" modules: what exists, what it costs, multiple suppliers?

### Takeaway
Bare micro-OLED panels with HDMI boards are widely sold (AliExpress, Alibaba, Hicenda, DisplayModule, YX Microdisplay). Integrated panel+prism kits exist, but at $184-$415 each from small sellers, and they need HDMI/MIPI hosts, not an ESP32-C3. Consumer HUD engines (ActiveLook, Monocle) are not sold as parts. The only ESP32-native optical module found is the LILYGO T-Glass (about $48, with stock problems).

### Cited Findings
- Hicenda 0.39" 1024x768 Si-OLED (500 nits) + "15X magnifying optical prism" + HDMI/CVBS driver kit: $286. A 0.23" 640x400 optical module "for helmet-mounted" use: $184. A 0.23" 640x400 4,000-nit module: $389. A 0.38" LCOS 1280x720 + HDMI board + optical prism: $415 — [Hicenda](https://www.hicendadisplay.com/products/0-39-inch-oled-microdisplay-module-rgb-1024x768-integrated-kits-for-ar).
- The same 0.23" 640x400 4,000 cd/m2 prism+HDMI module is also on AliExpress — [AliExpress 3256805265783262](https://www.aliexpress.us/item/3256805265783262.html?gatewayAdapt=deu2usa4itemAdapt).
- AliExpress 0.39" FHD micro-OLED with HDMI-to-MIPI Type-C board (1500 cd/m2, 8.75x4.97 mm active area), sold for AR/gunsight/night-vision. It is unclear whether optics are included. The listing is about 672 days old — [AliExpress 1005002019892277](https://www.aliexpress.com/item/1005002019892277.html). Another 0.39" FHD HDMI listing — [AliExpress 1005011653696669](https://www.aliexpress.com/i/1005011653696669.html).
- Alibaba aggregator: 0.39" 1024x768 micro-OLED viewfinder modules at about $56.50-$93.20, MOQ 5 — [Alibaba](https://electronics.alibaba.com/product/portable-electronic-viewfinder).
- Other panel vendors: DisplayModule 0.39" 1024x768 1800 cd/m2 — [DisplayModule](https://www.displaymodule.com/products/0-39-inch-1024x768-micro-oled-1800cd-dmgxo0039xgna). DisplayModule 0.49" 1920x1080 3000 nits MIPI and 0.6" 1920x1080 6000 nits MIPI — [0.49"](https://www.displaymodule.com/products/0-49-micro-oledos-display-1920x1080-90hz-3000nits-mipi-1003), [0.6"](https://displaymodule.com/products/0-6-micro-oledos-display-1920x1080-120hz-6000nits-mipi-1006). YX Microdisplay — [catalog](https://www.yxmicrodisplay.com/collections/microdisplays). Displayman — [catalog](https://displayman.com/micro-oled-displays/). A 0.39" 1024x768 1000-nit unit is even on Walmart.ca — [Walmart.ca](https://www.walmart.ca/en/ip/Microoled-0-39-inch-1024-768-OLED-Microdisplay-1000nits-for-AR-VR-Scope-EVF/7FMJ7QZZLW79).
- Caveat: micro-OLED panels normally cannot connect directly to HDMI or Type-C, and the driver board must match the exact panel model — [search summary of displaymodule guide](https://www.displaymodule.com/fr/blogs/knowledge/custom-micro-oled-display-manufacturer-selection-guide-ar-glasses-evf-near-eye-integration). Seller specs conflict (some call the 0.39" panel 1024x768, others FHD) — [AliExpress search](https://www.aliexpress.us/w/wholesale-micro-oled-display-hdmi.html).
- ActiveLook (Microoled): sold embedded in eyewear such as the Julbo EVAD-1, with UVEX and others adopting it. Module weight is quoted as 9 g in one source and 6 g in another (conflicting secondary sources). Microoled claims about 1 mW display power — [Ubergizmo](https://www.ubergizmo.com/2021/01/activelook-head-up-display-smartglass), [CEA](https://cea.fr/english/Pages/innovation/start-ups/microoled.aspx). No standalone module price or nits/FOV datasheet was found.
- LILYGO T-Glass (ESP32-S3 + 1.1" AMOLED + prism): about $48 — [CNX Software](https://www.cnx-software.com/2024/07/01/lilygo-t-glass-esp32-powered-smart-glasses-with-1-1-inch-prism-display-imu-microphone/?amp=1).

### Inferences
- An ESP32-C3 cannot drive MIPI or HDMI micro-OLEDs. Using these modules means either an SPI-capable small panel (SSD1306/SH1106 OLED, small SPI AMOLED like the T-Glass JD9613), or an MCU/SoC with MIPI-DSI (e.g. ESP32-P4) plus a matched driver board. This is a hardware decision that couples optics to the electronics roadmap.
- At $184-$415, panel+prism kits alone exceed most of a $300 retail budget once the retail markup is applied. They suit reference prototypes only. The DIY plate/eyepiece route with $5-$30 optics fits the budget.
- Bare micro-OLED panels are multi-sourced (BOE, Sony, SeeYA and other makers, sold through many resellers). Integrated optical modules are not.

### Gaps
- No AliExpress price found for a complete cheap birdbath module. Searches returned only DisplayModule's $999-$1,499 binocular units.
- No ActiveLook dev kit price or optical spec. The Julbo EVAD-1 spec sheet was not retrieved.
- No reliable teardown data for the Vufine optics.

## Focusing and collimation: virtual image at 1-3 m or infinity, plus diopter adjustment for glasses wearers

### Takeaway
Use a single positive lens with the display just inside its focal length. Display at the focal plane gives infinity. Moving the display about f^2 per diopter toward the lens brings the image closer, which is about 0.6 mm per diopter for f = 25 mm. That gives a simple screw or slide diopter adjustment. Commercial products place images from about 30 cm (Vufine) to infinity (helmet HUDs). About 2 m suits a HUD. For a clip-on over prescription glasses, the glasses' own lens corrects the image when the combiner sits outside them.

### Cited Findings
- Helmet HUD collimation: the display screen sits at the lens focal plane, so the image is projected to infinity — [US10884245](https://image-ppubs.uspto.gov/dirsearch-public/print/downloadPdf/10884245).
- Vufine+ places a virtual 4" screen at about 12" (30 cm) — [B&H](https://www.bhphotovideo.com/c/product/1331245-REG/vufine_vufine_a_high_definition.html).
- Guttag notes that prescription bonding (AddOptics) for Frame may be permanent, doesn't handle bifocals or progressives, and that near virtual-image placement adds complexity — [KGOnTech](https://kguttag.com/2024/05/09/brilliant-labs-frame-ar-with-ai-glasses-a-little-more-on-the-apple-vision-pro/).
- Vuzix M400's FOV is specified as a "5 in. screen at 17 in." equivalent — [Vuzix](https://vuzix.com/en-ca/pages/compare-smart-glasses). This is an angular spec and does not by itself give the focus distance.

### Inferences
These are thin-lens first principles, not sourced measurements.
- Image distance: for a lens of focal length f and display distance d < f, the virtual image is at v = f·d/(f−d). For f = 25 mm, an image at 2 m needs d ≈ 24.69 mm. An image at infinity needs d = 25 mm. The whole "infinity to 2 m" range is about 0.3 mm of travel.
- Diopter adjustment: Δd ≈ f²·ΔP. For f = 25 mm (0.025 m), that is 0.625 mm per diopter, so ±3 D needs about ±1.9 mm of focus travel. This is easy with an M2 screw or a printed helical cam. For f = 40 mm it is 1.6 mm per diopter.
- FOV ≈ 2·atan(w/2f), where w is the display width. A 0.96" SSD1306 (about 22 mm wide) with f = 40 mm gives about 30 deg horizontal. A 0.49" (about 11 mm) with f = 25 mm gives about 25 deg. Shorter f means a bigger FOV but a smaller eyebox and more aberration from a cheap single lens.
- Eyebox: roughly the lens clear aperture minus what the FOV "uses up" at the eye relief. Clip-ons sitting in front of glasses have 20-30 mm eye relief, which shrinks the eyebox, so use a larger lens (≥15-20 mm diameter PMMA aspheric). Expect alignment to be the main usability complaint.
- Wearers of prescription glasses: if the combiner/eyepiece is outside the user's glasses (a clip-on), their prescription already corrects the virtual image. Set the image at 2 m and give a ±3 D adjust for non-glasses users (goggles, helmet). Inside goggles (OTG over glasses), the same reasoning applies.
- Placing the image at 1-3 m rather than infinity reduces vergence/accommodation mismatch for near tasks. For a monocular HUD glanced at while skiing or riding, about 2-3 m or infinity is reasonable (inference).

### Gaps
- No sourced measurement of comfortable image distance specifically for monocular sports HUDs.
- No DIY build was found that documents a diopter mechanism with measured results.

## Outdoor readability: nits required at the eye for daylight and snow, and how combiner reflectivity affects it

### Takeaway
Outdoor see-through AR needs "thousands of nits to the eye" (Guttag). About 700 nits to the eye (Monocle) is "marginal outdoors". Vuzix's microLED waveguide engine at up to 4,100 nits is called outdoor-capable. A 50/50 combiner halves the panel's light, and a non-polarized birdbath quarters it. So a see-through design needs a 3,000-10,000 nit panel for snow. Alternatively, mount the combiner behind the goggle/sunglass tint, or go look-around (occluded), where the panel's full nits reach the eye.

### Cited Findings
- "Outdoor use requires thousands of nits delivered to the eye". MicroLEDs are hundreds to over a thousand times brighter than micro-OLEDs. Micro-OLED roadmaps reach about 30K nits. The Vuzix Shield/Ultralite green microLED + waveguide is rated up to 4,100 nits, "bright enough for outdoor use" — [KGOnTech 2023](https://kguttag.com/2023/03/12/microleds-with-waveguides-ces-ar-vr-mr-2023-pt-7/).
- Monocle/Frame: a 3,000-nit panel gives at most about 750 nits (about 700 estimated) to the eye, "fine indoors but marginal outdoors" — [KGOnTech 2024](https://kguttag.com/2024/05/09/brilliant-labs-frame-ar-with-ai-glasses-a-little-more-on-the-apple-vision-pro/).
- Vuzix M400 (occluded, no combiner) rates >2,000 nits — [Vuzix](https://vuzix.com/en-ca/pages/compare-smart-glasses).
- For context on direct-view displays: signage guides give 1,500-2,500 nits outdoors and 2,500+ in direct sun — [Crown TV](https://www.crowntv-us.com/blog/what-are-display-nits), [AVIXA](https://xchange.avixa.org/posts/how-to-choose-the-right-brightness-for-indoor-and-outdoor-commercial-displays). Sunlight-readable LCDs need ≥700 nits backlight — [Global American](https://globalamericaninc.com/?p=30784). These numbers are for opaque screens, not see-through AR, so they are context only.
- Available panel brightness: 500 nits (Hicenda 0.39"), 1,000-1,800 (generic 0.39"), 3,000 (0.49" FHD, Sony 0.23"), 4,000 (0.23" 640x400), 6,000 (0.6" FHD) — [Hicenda](https://www.hicendadisplay.com/products/0-39-inch-oled-microdisplay-module-rgb-1024x768-integrated-kits-for-ar), [DisplayModule 0.39"](https://www.displaymodule.com/products/0-39-inch-1024x768-micro-oled-1800cd-dmgxo0039xgna), [DisplayModule 0.49"](https://www.displaymodule.com/products/0-49-micro-oledos-display-1920x1080-90hz-3000nits-mipi-1003), [DisplayModule 0.6"](https://displaymodule.com/products/0-6-micro-oledos-display-1920x1080-120hz-6000nits-mipi-1006).

### Inferences
- Contrast model: perceived contrast ≈ (L_panel·R_combiner + L_world·T_world) / (L_world·T_world). R_combiner is the combiner's reflectivity and T_world is the transmission of everything between the world and the eye (goggle tint × combiner transmission). Raising R helps the display but darkens the world (R + T ≤ 1). A 30R/70T plate is a common compromise (inference).
- The ski-goggle trick: put the combiner inside the goggle or behind the sunglass lens. A category-3 tinted lens (roughly 8-18% visible light transmission, per standard tint categories; not fetched here) dims the world about 5-10x but does not dim the display. That cuts the nits needed by the same factor. This is probably the single biggest lever for Hiroglass's snow case. Mounting outside the tint loses it.
- A look-around display (D) delivers about 100% of panel nits. With a 2,000-4,000 nit micro-OLED it matches commercial outdoor HUDs without a combiner, at the cost of see-through.
- Monochrome green helps: the eye is most sensitive near 555 nm, and microLED/OLED greens are brightest. goggl1-style monochrome OLEDs (white/blue/yellow) are likely a few hundred nits (est.) and will wash out on snow behind a 50% combiner unless the combiner is behind the goggle tint.
- Add an auto-brightness ambient light sensor and a black backing or shroud behind the combiner area to improve contrast.

### Gaps
- No sourced luminance figure for sunlit snow fields was obtained (it is commonly quoted in the 10^4 cd/m2 range, but that is unverified here).
- No measured nits-to-eye for ActiveLook/Julbo EVAD-1 (a snow and sports product), which would be the most relevant benchmark.

## Documented open-source and DIY builds, and what they learned

### Takeaway
Open DIY HUD builds cluster around ESP32 + small I2C/SPI OLED + a single lens + an acrylic, film, or beam-splitter combiner, which is goggl1's class. Their documented lessons: optics alignment and internal reflections are the hard part, a black housing matters, beam splitters and glue can be costly or hard to source, small prism modules sharply limit usable pixels, and software is usually left unfinished.

### Cited Findings
- Arduino Nano 33 BLE + 0.49" OLED + lens + mirror + car HUD film on a phone case. STLs published, software unfinished — [Arduino blog 2021](https://blog.arduino.cc/2021/08/27/giving-google-glass-a-run-for-its-money/).
- Hackaday.io Smartglass: ESP32 BLE, 0.96" OLED, lens, acrylic screen combiner — [hackaday.io](https://hackaday.io/project/92176-smartglass).
- XPrism: ESP32-PICO-D4, GPL-3.0, Android controller — [GitHub](https://github.com/open531/XPrism/blob/main/README.en.md).
- LILYGO T-Glass: the prism cuts the usable display to 126x126 of 294x126 px. One commenter calls it "a little limiting, but manageable", and another notes that off-center corner displays cause eye strain — [CNX Software](https://www.cnx-software.com/2024/07/01/lilygo-t-glass-esp32-powered-smart-glasses-with-1-1-inch-prism-display-imu-microphone/?amp=1).
- OpenGlass (XIAO ESP32S3, camera/AI, no display) is no longer maintained and moved to Omi — [README](https://cdn.jsdelivr.net/gh/basedhardware/openglass@main/README.md).
- HoloCubic: the beam splitter and OCA glue are hard to source and costly — [summary](https://www.neura.market/ai-tools-directory/all/peng-zhihui-holocubic).
- The Hack Club "Glare" project exists (not fetched) — [hardware.hackclub.com](https://hardware.hackclub.com/projects/rr-creator/glare).
- Guttag's commercial lessons for birdbaths: seams, forward leakage, FOV-cubed volume, and the 25% efficiency of non-polarized designs — [KGOnTech](https://kguttag.com/2024/05/09/brilliant-labs-frame-ar-with-ai-glasses-a-little-more-on-the-apple-vision-pro/).

### Inferences
- Hiroglass REV1 can reuse goggl1's ESP32-C3 + BLE stack unchanged with architecture A or D. The optics work is mechanical (a printed housing, a focus slide, a clip interface), which suits a 2-3 person team on a timescale of months.
- Publish optics as parametric CAD (lens f, plate angle, eye relief), so builders can swap to whatever lens and plate their local supplier stocks. That directly addresses the "avoid single-supplier" goal.
- Lessons to design in from the start: wedged or AR-coated combiner (ghosting), a matte black interior, a centered or slightly-below-center image (strain), ±3 D focus travel, and a combiner mounted behind the tint for outdoor use.

### Gaps
- None of the DIY builds found publish measured FOV, eyebox, or nits. Their lessons are qualitative.
- I did not fetch the Hack Club Glare page, the Hackster TOLED article, or the hackaday Smartglass log details, so their specific lessons were not captured.
- No SPIE/Optica paper on low-cost DIY HUD combiners was retrieved within this research budget.
