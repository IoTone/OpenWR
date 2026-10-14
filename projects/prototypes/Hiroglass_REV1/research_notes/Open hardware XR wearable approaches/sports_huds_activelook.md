# Sports HUD reference products and the ActiveLook open protocol

Research date: 2026-10-13. 14 tool calls. Brilliant Labs, MentraOS, Even G1, OpenGlass, North Focals and the Recon shutdown were already covered elsewhere and are left out here.

## Q1. ActiveLook (Microoled): GATT, frame format, command set, licensing, module specs, products, and whether Hiroglass could be ActiveLook-compatible

### Takeaway
ActiveLook is a fully documented, small-display command protocol over a custom BLE GATT service: a 304x256 canvas, primitives, and stored fonts, images, layouts, gauges, pages, animations and configs. It is the closest existing match to goggl1's command/primitive model, and it already has watch-side clients (Garmin Connect IQ data field, SuuntoPlus, Apple Watch). The docs and SDKs are public; the iOS SDK and Garmin sample are Apache-2.0 and the config tools are CC0. But the *brand* is controlled by a Brand License Agreement and an "Application Certification Plan", so Hiroglass can likely implement the wire protocol but should not use the ActiveLook name or badges without a license.

### Cited Findings
**GATT (API doc for firmware 4.12.0b, doc rev fw-4.12.0_doc-revC)** — [ActiveLook API doc](https://github.com/ActiveLook/Activelook-API-Documentation/blob/main/ActiveLook_API.md)
- Standard services: Generic Access 0x1800 (Device Name, Appearance, Preferred Conn Params), Device Information 0x180A (Manufacturer, Model, Serial, HW, FW, SW version strings), Battery 0x180F (Battery Level read and notify; it notifies about every 30 s when enabled).
- Custom service `0783B03E-8535-B5A0-7140-A304D2495CB7`:
  - TX `...CB8`: Notify (device to phone responses)
  - Control `...CB9`: Notify (flow control: `0x02` means the Rx buffer is 75% full, stop sending; `0x01` means resume)
  - RX `...CBA`: Write and Write Without Response (phone to device commands)
  - Gesture Event `...CBB`: Notify (value 1 when the proximity gesture fires)
  - Touch Event `...CBC`: Notify (short press under 3 s)
- The firmware update service is `0xFEF5` (reserved).

**Frame format** — [ActiveLook API doc](https://github.com/ActiveLook/Activelook-API-Documentation/blob/main/ActiveLook_API.md)
- `0xFF | cmdId (1B) | cmdFormat (1B) | length (1 or 2 B) | queryId (0–15 B) | data | 0xAA`.
- In the format byte, bit 5 set means a 2-byte length field. Bits 4–1 give the query ID size (0–15 bytes). The query ID is echoed back so asynchronous replies can be matched to their commands.
- Length counts the whole frame, header and footer included. The maximum is 533 bytes (512 data + 15 query ID). Multi-byte values are big-endian.
- Commands may span several MTU chunks; the receiver reassembles them using the length field and the footer.
- Write With Response is recommended, and required in practice for image and font uploads, which go in chunks of up to 512 bytes.

**Display** — [ActiveLook API doc](https://github.com/ActiveLook/Activelook-API-Documentation/blob/main/ActiveLook_API.md)
- 304 x 256 px. The MDP05 panel has 16 grey levels. The MDP08 colour panel has "81 RG colours" (colour glasses only).

**Command set (IDs)** — [ActiveLook API doc](https://github.com/ActiveLook/Activelook-API-Documentation/blob/main/ActiveLook_API.md)
- **General:** 0x00 displayPower, 0x01 clear, 0x02 grey, 0x03 demo, 0x05 battery, 0x06 vers, 0x08 led, 0x09 shift, 0x0A settings, 0xE0 shutdown, 0xE1 reset, 0xE2 error (device to master), 0xE3 rdDevInfo, 0xA5 pixelCount.
- **Luminance:** 0x10 luma.
- **Sensors:** 0x20 sensor, 0x21 gesture, 0x22 als (auto-brightness).
- **Primitives:** 0x30 grayscale, 0x3D color, 0x31 point, 0x32 line, 0x33 rect, 0x34 rectf, 0x35 circ, 0x36 circf, 0x3C arc, 0x38 polyline, 0x37 txt, 0x3E txtColor, 0x39 holdFlush (double-buffer style batching).
- **Images:** 0x41 imgSave, 0x42 imgDisplay, 0x44 imgStream, 0x46 imgDelete, 0x47 imgList.
- **Fonts:** 0x50 fontList, 0x51 fontSave, 0x52 fontSelect, 0x53 fontDelete.
- **Layouts:** 0x60–0x6B (save, saveInColor, delete, display, clear, list, position, displayExtended, get, clearExtended, clearAndDisplay[Extended]).
- **Gauges:** 0x70 display, 0x71 save, 0x72 delete, 0x73 list, 0x74 get.
- **Pages:** 0x80–0x86 (save, get, delete, display, clear, list, clearAndDisplay).
- **Animations:** 0x95–0x99.
- **Configurations (namespaced asset bundles):** 0xD0 cfgWrite, 0xD1 cfgRead, 0xD2 cfgSet, 0xD3 cfgList, 0xD4 cfgRename, 0xD5 cfgDelete, 0xD6 cfgDeleteLessUsed, 0xD7 cfgFreeSpace, 0xD8 cfgGetNb.
- **Deprecated** since 4.0.0/4.6.0: 0x40, 0x45, 0xA1–0xA3 (the old configID scheme).

**Licensing of docs and SDKs** — [GitHub ActiveLook org](https://github.com/ActiveLook)
- The org has 8 repos: android-sdk (Java, licence not shown), Activelook-API-Documentation (licence not shown), ios-sdk (Swift, Apache-2.0), demo-app, Garmin-Datafield-sample-code (Monkey C, Apache-2.0), Products, Config-Generator (Python, CC0-1.0), Activelook-Visual-Assets (CC0-1.0).
- The API doc itself has no licence statement; it credits the technology to MICROOLED and gives contact@activelook.net — [API doc](https://github.com/ActiveLook/Activelook-API-Documentation/blob/main/ActiveLook_API.md).
- No Flutter or web SDK is listed — [GitHub ActiveLook org](https://github.com/ActiveLook).

**Brand and partner terms** — [ActiveLook Eyewear Partnership Program](https://www.activelook.net/news-blog/activelook-eyewear-partnership)
- Brands must sign the ActiveLook Brand License Agreement.
- There are four ways in: distribution of an off-the-shelf product (e.g. Engo), re-branding or co-branding existing glasses, a new eyewear product on an existing platform, or a new optical engine. Availability "depends on the business opportunity".
- The "Powered by ActiveLook" badge is required on packaging, point of sale, ads, web and promo videos longer than 10 s, and posts must use #activelook.
- Companion apps must follow the "ActiveLook Application Certification Plan" and may use a "works with ActiveLook" badge.
- ActiveLook calls itself "an open protocol for smart devices".
- The page gives no royalty terms and no module specs.

**Reference design** — [ActiveLook news](https://www.activelook.net/news-blog/microoled-quanta-computer-and-stmicroelectronics-partner-to-deliver-reference-design-foractivelook-enabled-augmented-reality-smart-glasses)
- In February 2025, Microoled, Quanta Computer and STMicroelectronics announced a reference design for ActiveLook-enabled glasses, aimed at helping manufacturers ship faster.

**Module specs (incomplete)**
- Microoled claims the ActiveLook display draws about 1 mW, "30 times less energy than the competition" — [CEA](https://cea.fr/english/Pages/innovation/start-ups/microoled.aspx).
- Module weight is quoted as 6 g in one source and 9 g in another (conflicting) — see the [search results via ubergizmo/sportsdigest](https://www.ubergizmo.com/2021/01/activelook-head-up-display-smartglass).
- The optics are described only as "see-through optics not blocking the field of view" — [API doc repo](https://github.com/ActiveLook/Activelook-API-Documentation).
- Engo says data appears to float about 5.5 ft in front of the user — [road.cc](https://road.cc/content/tech-news/engo-eyewears-new-smart-sunnies-real-time-data-lenses-287531).

**Products and integrations**
- Julbo (EVAD-1), Engo and Cosmo Connected (CosmoVision, urban navigation) are the eyewear partners — [ActiveLook news (Enterprise Light)](https://www.activelook.net/news-blog/microoled-announces-the-activelook-enterprise-light-ar-platform-a-hands-free-heads-up-data-display-eyewear-solution-for-industry-operations); [Julbo FAQ](https://www.julbo.com/en_wo/faq/evad-1-connected-glasses/what-is-activelook-the-embedded-technology-in-our-evad-1-connected-glasses).
- Engo is Microoled's own brand, sold under a distribution agreement — [Partnership page](https://www.activelook.net/news-blog/activelook-eyewear-partnership).
- Non-eyewear integrations: Navigr8 bike navigation (April 2024), VSpeak aeromodelling (March 2024), UL Control ultralight aviation (2023), and Apple Watch sensor data — [ActiveLook news: Navigr8](https://www.activelook.net/news-blog/microoled-announces-activelook-integration-with-navigr8-for-hud-navigation), [VSpeak](https://www.activelook.net/news-blog/microoled-announces-activelook-ar-glasses-integration-with-vspeak-to-deliver-and-hands-free-data-in-real-time-for-aeromodeling), [UL Control](https://www.activelook.net/news-blog/microoled-partners-with-ul-control-a-company-specialized-in-innovation-flight-instruments-for-ultra-light-aircraft-to-bring-activelook-heads-up-display-technology-to-the-world-of-ultra-light-aviation), [Apple Watch](https://www.activelook.net/news-blog/microoled-introduces-the-first-ar-solution-to-enable-real-time-display-of-apple-watch-sensor-data).
- **Engo 2** (2022):
  - 36 g standard / 41 g large; the nosepiece electronics unit is 13 g.
  - AMOLED display on the right eye, 10–12 h battery, $299.95 — [Endurance Sportswire](https://www.endurancesportswire.com/engo-eyewear-introduces-engo-2-the-lightest-heads-up-display-for-training-and-competition-for-endurance-athletes/); [Xataka](https://www.xatakaon.com/wearables/for-weeks-ive-been-running-with-these-glasses-that-project-data-into-my-field-of-vision-the-future-of-running-may-have-to-wait).
  - On Garmin it runs through the ActiveLook Connect IQ data field (Edge, Fenix, Forerunner, Venu) — [Montre Cardio GPS](https://www.montre-cardio-gps.fr/test-engo-2-lunettes-connectees-avec-visualisation-tete-haute-pour-le-sport/reglage-engo-2/).
  - SuuntoPlus app for Suunto 9/5. A reviewer found no Coros or Polar support and no structured workouts — [Xataka](https://www.xatakaon.com/wearables/for-weeks-ive-been-running-with-these-glasses-that-project-data-into-my-field-of-vision-the-future-of-running-may-have-to-wait).
- **Engo 3** (current): 38.5 g and 20 h battery, works with 81+ Garmin devices, and with Apple Watch through the ActiveLook app (partial: only the HR widget is in colour, and there are no power or radar widgets on Apple Watch). Strava Live Segments are exclusive to engo + Apple Watch. Dashboards come from 40+ widgets in the companion app — [engo3 product page](https://engoeyewear.com/en-ww/products/engo-3).
- **Engo3 Titanium** (2026): 28.9 g and more than 24 h battery — [road.cc](https://road.cc/?p=1732991).
- I found no source naming Wahoo compatibility.

### Inferences
- **The protocol has the same shape as goggl1's.** A command stream of primitives plus stored, named assets (fonts, images, layouts, gauges, pages) that the phone or watch fills by ID matches goggl1's command/primitive design. Hiroglass could expose the ActiveLook service UUIDs and frame format as a *second* GATT service next to its own versioned protocol. The Garmin Connect IQ ActiveLook data field (and SuuntoPlus, Apple Watch and Navigr8) would then in principle drive it with no phone.
- **Compatibility work.**
  - Firmware would need to render at a virtual 304x256 and scale or crop to the Hiroglass panel.
  - It would need the Control characteristic's flow control and the 2-byte-length / query-ID echo.
  - It would also need the cfg* family. Watch apps upload and select a named "config" of layouts and fonts and then mostly send layoutDisplay plus values. Without cfgWrite/cfgSet and layoutSave/layoutDisplay, the stock data field will not render anything useful.
  - The Garmin sample code is Apache-2.0, so the expected command traffic can be studied directly.
- **Legal and IP risk.**
  - Interoperating with a published wire protocol (UUIDs, opcodes) is generally lower-risk than copying code. The API doc has no licence, so do not copy its text verbatim into Hiroglass docs.
  - The ActiveLook name and badges are trademark-controlled through the Brand License Agreement. Marketing should say something factual such as "speaks the ActiveLook BLE protocol", not "ActiveLook compatible" with a logo, unless licensed.
  - Watch apps might check Device Information strings (manufacturer or model) or firmware version and refuse unknown devices. That is unverified; test with the Garmin data field.
  - Patents on the protocol were not researched.
  - Microoled could change the protocol in a future firmware, but older watch apps would keep the current version alive.
- **Lessons from the design.** The Control-characteristic back-pressure, query-ID correlation, the explicit length+footer framing over multi-chunk MTU, and cfgFreeSpace/cfgDeleteLessUsed for flash management are all worth copying into the Hiroglass protocol, whatever compatibility decision is made.

### Gaps
- Nits, FOV in degrees, the exact optical architecture (birdbath, freeform prism or partial mirror) and the panel model of the ActiveLook / Engo module: no reliable source found.
- Royalty and price terms, and whether the bare optical module can be bought by small third parties: not public. The partnership page suggests case-by-case and business-driven.
- Licence of the android-sdk and API-doc repos is not shown on the org page.
- Whether a Wahoo, Strava standalone or Coros integration exists in 2026: not found.
- Whether the Garmin data field checks device identity: unknown.

## Q2. Sports HUDs: mounting, optics, openness, status (2026)

### Takeaway
Across this category the displays are monocular, the protocols are closed and the apps are tied to one vendor. The few with open SDKs are developer-edition products (Vuzix Z100, Everysight). The clearest open-protocol precedent is ActiveLook. Helmet HUDs (Forcite, CrossHelmet) got caught up in consolidation or have murky status.

### Cited Findings
- **Everysight Maverick Sport**
  - 43 g, full-colour Sony micro-OLED, 22° FOV, 640x400, up to 10 h with the display on — [Everysight support](https://support.everysight.com/hc/en-us/articles/27997949123740-Maverick-Sport-specs-weight-and-comfort).
  - "BEAM" optics (no waveguide), $399 at Best Buy — [Best Buy](https://www.bestbuy.com/product/everysight-maverick-sport-ar-sunglasses-w-full-color-heads-up-display-10hr-battery-narrow-metallic-grey/C5KYFCQKP3).
  - A developer edition sold in 2024 at $399 — [Yanko Design](https://www.yankodesign.com/2024/02/26/if-the-apple-vision-pro-and-the-google-glass-had-a-baby-these-ar-glasses-would-be-it/).
- **Everysight Raptor**: 98 g and about 8 h — [DC Rainmaker 2018](https://dcrainmaker.com/2018/04/everysight-cycling-display.html); 101 g — [road.cc](https://road.cc/content/review/251030-everysight-raptor-glasses). Its discontinuation is unverified.
- **Rekkie smart snow goggles**
  - Transparent HUD showing speed, altitude, time, compass and group-member locations, with a dimmed time-only mode and a glove-friendly button.
  - Bluetooth to the phone for music, calls and notifications. LiPo battery charged over USB-C, "only a few hours" of battery.
  - Hard to read on very bright days. About $349 — [GearJunkie](https://gearjunkie.com/winter/augmented-reality-smart-goggles-rekkie-review); [Looper](https://www.looper.com/1449356/rekkie-shark-tank-product-didnt-know).
  - No intercom feature confirmed.
- **Vuzix Z100**
  - 640x480 monochrome green microLED waveguide, 30° FOV, right eye. Up to 48 h battery, BLE through the Vuzix Connect app.
  - Temple tap sensor and prescription inserts. $499.99 consumer; the $799 developer package includes the SDK and samples — [Notebookcheck](https://www.notebookcheck.net/Vuzix-debuts-ultralight-Z100-smart-glasses-developer-s-edition-to-enterprise-customers-wanting-better-optimized-staff.792371.0.html); [Wearify](https://thewearify.com/vuzix-z100-smart-glasses-now-available-for-everyone/).
  - Weight conflicts: 35 g (Notebookcheck), 38 g, 30 g ([Reality Atlas](https://www.reality-atlas.com/hardware/vuzix-z100)).
  - The SDK licence and any fees are undocumented — [XR Today](https://www.xrtoday.com/?p=55596).
- **Forcite MK1S**
  - Motorcycle helmet with HUD (a small LED display for turn-by-turn navigation and alerts), 1080p chin camera, audio and mic — [Visordown](https://www.visordown.com/news/industry/gopro-acquires-tech-enabled-helmet-company-forcite); [Stuff](https://stuff.co.za/2024/02/09/gopro-tech-enabled-helmet-by-2025/).
  - GoPro agreed to acquire Forcite in January 2024, planned GoPro-branded helmets and partner integrations, and aimed for a helmet in 2025 — same sources.
  - Equity-crowdfunding investors reportedly took a loss on the deal — [Business News Australia](https://direct.businessnewsaustralia.com/articles/gopro-to-acquire-sydney-helmet-tech-scale-up-forcite.html).
- **CrossHelmet X1 (Borderless Inc., Tokyo)**
  - Smart motorcycle helmet with a companion app for battery, lights, noise settings and maps — [xBhp](https://www.xbhp.com/talkies/news/35172-crosshelmet-x1-smart-motorcycle-helmet-concept?p=1717906).
  - The company also runs CrossWare industrial helmets — [CareerCross](https://careercross.com/en/company/detail-433697).
  - No bankruptcy report found. The cautionary precedent is Skully, which went bankrupt and left riders stranded — [Android Authority](https://www.androidauthority.com/crosshelmet-smart-motorcycle-helmet-801245).

### Inferences
- **Mounting.** Three patterns exist:
  1. Integrated frames (Engo, Everysight, Vuzix, Julbo): best weight, but Rx and fit are locked to the frame.
  2. Goggle-integrated (Rekkie): the electronics live in the goggle.
  3. Clip-on pods (Garmin Varia Vision, Recon Jet-style): fit any glasses.
- **Hiroglass target.** Hiroglass wants to fit glasses, goggles and helmets, so the Varia Vision clip pattern is the closest reference. The target to beat is about 30 g pod weight; integrated-frame competitors are now at 29–43 g *total*.
- **Brightness.** Bright-day legibility is the recurring complaint (Rekkie). Sports HUDs need high nits plus auto-luma; ActiveLook has an ALS command for this.

### Gaps
- Smith/Oakley Mod Live (Recon) mount and spec details were not re-fetched; earlier research covered the Recon shutdown.
- Livall, Solos and Kopin-based cycling HUDs: not researched within the call budget.
- Forcite's post-acquisition status in 2026 (whether a GoPro helmet shipped, and app support for MK1S owners): not found.
- The current Rekkie model and its 2026 status: not found.
- Display nits for Everysight, Rekkie and Vuzix: not found.

## Q3. Garmin Varia Vision (clip-on monocular HUD for glasses)

### Takeaway
The Varia Vision is the canonical clip-on-to-any-sunglasses HUD:
- **Pod:** about 29 g, 8 h claimed battery, $399, launched January 2016.
- **Mount:** a 1–2 g quick-release mount on the temple, rotated 90° to detach. Fits either side, with auto-flip.
- **Link:** ANT+ "Remote Display", tied to Garmin Edge units.
- **Status:** it appears to be end-of-life, from weak retailer signals only.

### Cited Findings
- **Size and battery**
  - 29 g excluding the mount; about 60 g with typical sunglasses. The mount weighs 1–2 g — [DC Rainmaker](https://dcrainmaker.com/2016/01/garmin-varia-vision-hud.html).
  - Retailers list 29.7 g, about 60 x 19.7 x 19.8 mm and "ambidextrous" — [Nashbar](https://www.nashbar.com/garmin-varia-vision-headsup-display-010-01952-00/p481716).
  - 8 h claimed battery, custom charging cable — [DC Rainmaker](https://dcrainmaker.com/2016/01/garmin-varia-vision-hud.html).
- **Mount and display** — [DC Rainmaker](https://dcrainmaker.com/2016/01/garmin-varia-vision-hud.html)
  - Quick-release mount on "any sunglasses", "no zip-ties"; rotate the pod 90° to detach.
  - The pod flexes a few mm fore and aft and tilts a few mm up and down. Works on either side, with automatic image flip.
  - Colour display, auto brightness with manual override, inverted colours for night. Resolution and display tech not stated.
- **Controls and pairing** — [DC Rainmaker](https://dcrainmaker.com/2016/01/garmin-varia-vision-hud.html)
  - Power button plus a swipe strip on the side (like Recon Jet). Vibration alerts, no audio.
  - Pairs to an Edge 520/1000 as a "Remote Display" (ANT+ ID), or directly to the Varia radar. The Fenix 3, FR920XT and Epix were not supported.
  - Five pages of up to four fields, plus Strava Segments, navigation and Virtual Partner. $399, announced January 5, 2016.
- **Status:** a Finnish retailer marks it "end of life or currently not available" — [Multitronic](https://www.multitronic.fi/en/products/1672760). There is no Garmin announcement.

### Inferences
- **Mount.** The Varia mount is the right reference mechanism for Hiroglass's glasses mode: a light temple clip, a rotate-to-release pod, a few mm of eyebox adjustment, and a firmware flip for left or right. For goggles and helmets, Hiroglass needs alternative adapters (strap clip, helmet shell or visor arm) on the same pod interface.
- **Protocol.** It used a proprietary ANT+ remote-display link and only Garmin heads could drive it. When Garmin quietly ended it, nothing else could drive it. That is the counter-example to the OpenWR "open versioned protocol" principle.

### Gaps
- Official Garmin discontinuation date, panel resolution, nits and optics type: not found.
- Width range of the temple clip: not found.

## Q4. Google Glass Explorer and Enterprise: optics and end-of-life lessons

### Takeaway
Glass used a prism projector over the right eye: 640x360, which Google described as equal to a 25" HD screen seen from 8 ft. Enterprise Edition 2 (Snapdragon XR1, Android 8.1, $999) stopped selling on March 15, 2023, and support ended September 15, 2023. Devices kept working and third-party apps could still be updated, but there were no further OS updates.

### Cited Findings
- **Explorer display:** prism projector, 640x360, "equivalent of a 25 in screen from 8 ft" — [Wikipedia](https://en.wikipedia.org/wiki/Google_Glass); [TechRadar](https://www.techradar.com/news/video/google-glass-specs-revealed-as-android-app-goes-live-1144865).
- **Resolution source:** the 640x360 figure was inferred from the SDK's UI guidelines rather than stated directly — [AnandTech](https://at-web1.www.anandtech.com/show/6901/google-announces-glass-specs-developer-sdk-shipment-to-glass-explorers-in-waves).
- **Enterprise Edition 2 hardware:** launched May 2019, Snapdragon XR1, Android 8.1, $999 — [9to5Google](https://9to5google.com/2023/03/15/google-glass-enterprise-edition-discontinued/).
- **Enterprise Edition 2 end of life:**
  - Sales ended March 15, 2023. Support (replacement devices) ran to September 15, 2023, with no software updates planned — [9to5Google](https://9to5google.com/2023/03/15/google-glass-enterprise-edition-discontinued/); [PhoneArena](https://www.phonearena.com/news/google-ends-sales-google-glass-enterprise-edition-2_id146267).
  - Devices kept working and third-party developers could still update apps, but "Meet on Glass" was guaranteed only until September 15 — [Gigazine](https://gigazine.net/gsc_news/en/20230316-google-glass-enterprise-edition-discontinued).

### Inferences
- **FOV estimate.** 25" at 8 ft works out to about 14–15° diagonal (simple geometry; Google gave no official degree figure). That is comparable to small sports HUDs and well below Maverick (22°) and Z100 (30°).
- **End-of-life lesson.** EE2 "kept working" only because its apps were locally sideloadable Android. Cloud-dependent features (Meet) were the parts at risk. This supports the Hiroglass principle of no required cloud, plus a published protocol and bootloader, so owners can keep devices alive after a vendor exits.

### Gaps
- Official FOV in degrees and brightness (nits) for the Glass prism: not found in these sources.
- EE2 display resolution was not confirmed in these sources.
