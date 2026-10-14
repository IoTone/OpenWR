# Open (and partially open) smart glasses and HUD projects, and their software stacks: reference designs for Hiroglass REV1

Research date: 2026-10-13. The research ran with a budget of about 16 tool calls and was interrupted by a network outage. Coverage is uneven: Brilliant Labs, MentraOS, ActiveLook, OpenGlass, North Focals and Recon are well sourced. Several items in the brief could not be checked against sources and are listed under Gaps rather than guessed.

## Q1: Catalog: what each project actually opens, license, hardware, price, maturity

### Takeaway
Brilliant Labs (Frame, Halo) is the closest reference design to Hiroglass. It publishes firmware on GitHub, uses permissive licenses for its SDKs and Halo's Zephyr stack, puts a Lua runtime on the glasses, and uses a two-characteristic BLE protocol. Its hardware openness is weaker than the marketing suggests: Frame ships schematic PDFs and STLs, not native CAD files with a stated license. MentraOS is the main open-source glasses OS (Apache-2.0), but it is a phone-plus-cloud app platform layered on other companies' hardware. ActiveLook is the main open BLE HUD command protocol in sports eyewear.

### Cited Findings

**Brilliant Labs (org: github.com/brilliantlabsAR)**
- The org has 25 repositories. halo-firmware is described as "Zephyr RTOS on Alif Balletto, Lua runtime, BLE OTA updates" (58 stars, updated 2026-10-07). brilliant_sdk is the SDK monorepo (BSD-3-Clause, 117 stars, updated 2026-10-06). halo-zephyr-alif, halo-hal-alif and halo-mcuboot are Apache-2.0 and were updated Aug 2026. frame-codebase has 531 stars, was last updated 2025-10-05, and shows no standard license. monocle-micropython has 235 stars. noa-flutter (the Noa app, Dart) has 84 stars and was last updated 2025-05-13. noa-assistant has 200 stars. The docs repo was updated 2026-10-13. — [brilliantlabsAR GitHub org](https://github.com/brilliantlabsAR)
- frame-codebase is labelled "Other" for license in the org listing, unlike the BSD-3/Apache repos. Its three parts are the nRF52 application (Lua, Bluetooth, AI tasks, power management), the nRF52 bootloader, and the FPGA RTL. It builds and flashes to an nRF52840 DK. — [frame-codebase](https://github.com/brilliantlabsAR/frame-codebase); [org repositories](https://github.com/orgs/brilliantlabsAR/repositories)
- Frame hardware:
  - Nordic nRF52840 (Cortex-M4F at 64 MHz, 1 MB flash, 256 KB RAM, BT 5.3)
  - Lattice CrossLink-NX LIFCL-17 FPGA
  - 0.23" micro-OLED, 640x400 RGB, 20° FOV, up to 16 colours per frame
  - 6 mm lenses
  - OmniVision OV09734 720p camera, cropped to 720x720
  - 210 mAh battery (2x105 mAh), plus a 140 mAh battery in the charging cradle

  Published hardware files are schematic PDFs (Frame and cradle), mechanical drawings as PDFs, and STL models of the Frame, cradle and Rx clip. The docs page states no license for these files. — [Frame hardware docs](https://docs.brilliant.xyz/frame/hardware/)
- Halo hardware:
  - Alif Balletto B1 (Cortex-M55 plus NPU)
  - Colour micro-OLED in the lens corner
  - Optics adjustable from +2 to −6 diopters
  - Two bone-conduction speakers, two mics, a 6-axis IMU and an optical sensor
  - BT 5.3, no Wi-Fi
  - About 14 h battery, about 40 g

  Pre-orders opened at $299. — [Road to VR](https://roadtovr.com/brilliant-labs-halo-smart-glasses-price-release-date/); [Wareable](https://www.wareable.com/wearable-tech/brilliant-labs-halo-smart-glasses-ai-release-date-price)
- Halo's price and ship date conflict across sources:
  - $299 with a November 2025 launch — [Tom's Guide](https://www.tomsguide.com/computing/smart-glasses/brilliant-labs-unveils-halo-the-worlds-thinnest-ai-glasses-and-it-wants-to-be-your-everyday-specs)
  - Shipping from Q1 2026 — [Gadget Flow](https://thegadgetflow.com/product/halo-ai-glasses/)
  - About $399 — [Abakcus](https://abakcus.com/gadgets/brilliant-labs-halo)
  - $349 — [yespress](https://yespress.io/brilliant-labs)
- A June 2026 review scored Halo 7.5/10 and called it a developer and hobbyist device. It flagged a small company, early software, a small glanceable display and unproven longevity. — [AI Tool Testing review](https://aitooltesting.com/reviews/brilliant-halo-review/)
- Brilliant Labs markets Halo as "Open-source. On-device AI… made for makers." — [Brilliant Labs on X](https://x.com/brilliantlabsAR/status/2049253182384071010?lang=en)

**Mentra / MentraOS (formerly AugmentOS)**
- MentraOS is Apache-2.0 ("Every component is open source under the Apache License 2.0"). It has about 2.4k stars, 362 forks and more than 20,000 commits on the dev branch.
  - Supported glasses: Mentra Live, Even Realities G2, Vuzix Z100 and Even Realities G1. NIMO is listed as coming soon.
  - Glasses apps run on the phone inside the "Mentra Runtime" and share one connection to the glasses. Several apps can run at once.
  - Apps are distributed through the Mentra MiniApp Store, and manufacturers can embed the runtime in their own apps.
  - The repo contains `cloud-v2` and `sdk` directories.

  — [Mentra-Community/MentraOS](https://github.com/Mentra-Community/MentraOS)
- Mentra Live has a 12 MP, 119° camera, three mics, stereo speakers and a MediaTek MTK8766, and deliberately no display. Some third-party sources describe the license as MIT, which conflicts with the official repo's Apache-2.0. Reported prices are $249 pre-order and $349 at February shipping. — [Glitchwire](https://glitchwire.com/news/mentra-live-smart-glasses-bet-on-open-source-to-break-the-closed-ecosystem/); [MentraOS page](https://mentraglass.com/os)
- Mentra raised $8M to launch an open-source smart-glasses OS (July 2025). — [Forbes](https://www.forbes.com/sites/charliefink/2025/07/01/mentra-raises-8-million-to-launch-open-source-os-for-smart-glasses/)
- Mentra-Community/OpenSourceSmartGlasses is the older open hardware project: "Open Source smart glasses designed to be 1. All day wearable 2. Immediately useful 3. Extendable for makers". — [OpenSourceSmartGlasses](https://github.com/Mentra-Community/OpenSourceSmartGlasses)

**ActiveLook (Microoled) / Engo / Julbo**
- ActiveLook displays "can all be controlled through an open and free wireless interface". The interface is a custom BLE GATT service, the "ActiveLook Commands Interface", with five characteristics. Basic commands draw shapes and print text. A second group manages preloaded bitmaps, layouts and fonts, which reduces traffic over the link. The repo has 18 commits, and the detailed spec is in `ActiveLook_API.md`. — [ActiveLook API Documentation](https://github.com/ActiveLook/Activelook-API-Documentation)
- The ActiveLook app supports Engo 1, Engo 2, Julbo EVAD-1, Julbo EVAD-2 and the ActiveLook DK. — [ActiveLook Sport Eyewear app](https://apps.apple.com/app/id1464023773)
- Third parties build on the protocol: a Garmin Connect IQ data field (ActiveLook Dex DF) implements its own BLE module, and Suunto watches have worked with Engo/ActiveLook since December 2022. — [ActiveLook-Dex-DF](https://github.com/fsallstrom/ActiveLook-Dex-DF); [advnture](https://www.advnture.com/news/these-smart-shades-will-beam-stats-from-your-garmin-watch-in-front-of-your-eyes)

**Even Realities G1 / G2**
- Even Realities publishes an official demo app. The G1 uses Nordic UART-style GATT services, with one BLE device per arm named `G1_{channel}_{side}_{id}`, and the app pairs both arms. — [EvenDemoApp docs index](https://docsearch.algolia.com/mcp/docs/repo/even-realities/evendemoapp)
- Gadgetbridge (open-source Android) supports the G1 and recently improved its connection reliability. — [Gadgetbridge mirror](https://zff.dev/tecncr/Gadgetbridge)
- The G2 protocol has been reverse-engineered by the community (i-soxi/even-g2-protocol), which says it works over standard BLE with no special pairing. — [even-g2-protocol](https://github.com/i-soxi/even-g2-protocol)
- MentraOS drives both the G1 and the G2. — [MentraOS](https://github.com/Mentra-Community/MentraOS)

**OpenGlass (BasedHardware / Omi)**
- OpenGlass is a DIY build for under $25 of parts: a Seeed XIAO ESP32S3 Sense, a 250 mAh EEMB LP502030 battery and a 3D-printed mount.
  - Firmware is an Arduino `.ino` (needs OPI PSRAM set).
  - The app is Expo/React Native and needs Groq and OpenAI keys, or optionally a self-hosted Ollama with moondream.
  - The license is MIT.
  - The README says the repo moved to Omi and is no longer supported.

  — [BasedHardware/OpenGlass](https://github.com/BasedHardware/OpenGlass)
- Omi docs say the OpenGlass devkit is not for sale. The parts list and case are in the Omi repo, and the device sends periodic photos and audio to the Omi app. — [Omi docs: OpenGlass](https://docs.omi.me/docs/hardware/OpenGlass)

**Project North Star (Leap Motion / Ultraleap)**
- Announced in 2018 as a research AR headset, not a product. It uses large birdbath-style combiners and side-mounted displays at 1600x1440 per eye and 120 fps, with a combined FOV over 100°. Leap Motion claimed it could be built for about $100 at scale. Files were released on GitHub under Apache-2.0. The first release lacked electronics schematics and software. — [Road to VR](https://www.roadtovr.com/leap-motion-reveals-project-north-star-an-open-source-wide-fov-ar-headset-dev-kit/); [Tom's Hardware](https://www.tomshardware.com/uk/news/project-north-star-open-source-documents,37222.html)

### Inferences
- Brilliant Labs is closest to Hiroglass's principles: permissive host SDKs (BSD-3), an Apache-2.0 Zephyr/MCUboot stack, and a BLE-plus-Lua design with no required app. Even there, hardware openness amounts to PDFs and STLs without a declared license. Hiroglass can stand out by publishing native KiCad and STEP files under CERN-OHL-P or similar with explicit license files.
- frame-codebase was last updated in Oct 2025 while Halo repos are active. This suggests Frame is in maintenance mode one product generation later. That fits a pattern where a small company keeps maintaining only its newest product.
- The Halo stack (Zephyr, MCUboot, Lua, BLE OTA) is a credible template for Hiroglass firmware. It is all Apache-2.0 and maps onto ESP32/nRF parts available from Mouser and Digikey.
- MentraOS is better treated as a possible compatibility target (a Hiroglass driver in the Mentra Runtime) than as a foundation. Its app model and store are phone- and cloud-centric.

### Gaps
- Brilliant **Monocle**: hardware specs, license and support status were not fetched in this session. All that was confirmed is that the monocle-micropython repo exists (235 stars).
- Halo: the license on the halo-firmware repo itself is not shown, and it is unconfirmed whether Halo native CAD or schematics are published. The current price conflicts across sources ($299/$349/$399).
- North Star's status after 2018 (Ultraleap involvement, CombineReality kits, community forks, last commit) was not found.
- Not researched for lack of budget: Vufine, Hackaday open smartglass projects, Kopin/Vuzix dev kit SDK terms, Meta Ray-Ban Display hackability, Sightful, Rokid and Xreal SDK status. GitHub star and last-commit data for MentraOS (last commit date) and ActiveLook (license) were also not shown.

## Q2: Software stack patterns: on-glass rendering vs. streaming, BLE GATT designs, app architecture, HUD content standards

### Takeaway
Three patterns dominate:
1. **On-glass scripting.** Frame and Halo run Lua on the glasses, and the host sends code or strings over a two-characteristic BLE UART.
2. **Retained-mode command protocol.** ActiveLook sends draw primitives plus preloaded fonts, bitmaps and layouts, so a short command updates a value.
3. **Phone or cloud runtime.** MentraOS and OpenGlass run apps on the phone or a server and push display content or pull sensor data.

For a BLE-bandwidth HUD at REV1, pattern 2 is the most robust and pattern 1 the most flexible.

### Cited Findings
- Frame has one BLE service, `7A230001-5475-A6A4-654C-8431F6AD49C4`, with TX and RX characteristics. The host writes Lua strings to TX and enables notify on RX. Frame evaluates each message and replies only on `print()` or an error, and is not a full REPL. Larger scripts are uploaded to the device and run with `require()`. — [Frame BLE specs](https://docs.brilliant.xyz/frame/building-apps-bluetooth-specs)
- The host libraries `frame_ble` (Flutter/Dart, pub.dev, v3.0.0) and `frame-ble` (Python) are BSD-3-Clause. — [frame_ble on pub.dev](https://pub.dev/packages/frame_ble/); [frame-ble-python docs](https://frame-ble-python.readthedocs.io/en/stable/api.html)
- Halo firmware runs a Lua runtime with BLE OTA on Zephyr, using MCUboot (Apache-2.0). — [brilliantlabsAR org](https://github.com/brilliantlabsAR)
- ActiveLook has five GATT characteristics. Commands cover drawing shapes and text plus managing preloaded layouts, bitmaps and fonts, explicitly to cut BLE traffic. — [ActiveLook API docs](https://github.com/ActiveLook/Activelook-API-Documentation)
- MentraOS uses a session-based API on the phone for display, camera, mic and speaker. Several apps share one glasses connection, and there is a MiniApp Store and a cloud component. — [MentraOS](https://github.com/Mentra-Community/MentraOS); [Glitchwire](https://glitchwire.com/news/mentra-live-smart-glasses-bet-on-open-source-to-break-the-closed-ecosystem/)
- The Even G1 uses a Nordic-UART-style service on each arm, so the host manages two BLE links. — [EvenDemoApp docs](https://docsearch.algolia.com/mcp/docs/repo/even-realities/evendemoapp)
- In OpenGlass and Omi, the ESP32S3 streams periodic photos and audio to the phone app, and the app calls cloud LLMs (Groq/OpenAI) or a self-hosted Ollama. — [OpenGlass](https://github.com/BasedHardware/OpenGlass); [Omi docs](https://docs.omi.me/docs/hardware/OpenGlass)

### Inferences
- Recommended for Hiroglass REV1:
  - Model the protocol on ActiveLook's retained-mode commands: preload fonts, icons and layouts, then send short "set field N = value" messages. Add an optional Frame-style script channel later.
  - Document the GATT service publicly and version it, so watches (Garmin CIQ, Wear OS) and third parties can drive it without Switchback, as Garmin and Suunto do with ActiveLook.
  - Write a Gadgetbridge or MentraOS driver to gain an ecosystem at little cost.
- Frame's `frame_ble` package is Flutter and BSD-3, which makes it directly useful reference code for Switchback (Flutter) BLE handling.
- On OpenXR/Monado: none of the sourced HUD projects use OpenXR. It becomes relevant only for the later binocular spatial version, where North Star (Apache-2.0) is the historical open reference. No sources were gathered on Monado in this session.

### Gaps
- ActiveLook's exact UUIDs, frame format and license were not retrieved (they are in `ActiveLook_API.md`).
- The MentraOS wire protocol to the glasses, and whether its cloud is mandatory or self-hostable, were not confirmed.
- No sources were gathered on Monado or OpenXR support for glasses-class devices, or on whether any open "HUD content" standard exists beyond vendor protocols.

## Q3: Lessons and failure modes: shutdowns, bricking, stranded users

### Takeaway
North Focals is the defining cautionary tale. Google acquired North, and about a month later a server shutdown made every pair stop working on 31 July 2020, refunds notwithstanding. Intel's exit killed Recon Jet in 2017. Dependence on servers or a single vendor app is the main way smart glasses die, which directly supports OpenWR's "no cloud, no account, no revocation" principle.

### Cited Findings
- North said it would turn off the servers Focals needed, so the glasses would lose all functionality on 31 July 2020. This came about a month after Google acquired North, and Focals 2.0 was cancelled. — [GearBrain](https://www.gearbrain.com/north-focals-smart-glasses-disabled-2646835849.html); [MobileSyrup](https://mobilesyrup.com/2020/07/28/north-focals-stop-working-july-31/); [ITBusiness.ca](https://itbusiness.ca/news/norths-focals-to-shut-down-a-month-after-its-acquisition-by-google/115782)
- North issued full refunds for paid Focals orders. Support ran until 31 Dec 2020, and Focals had earlier been cut from $999 to $599, which suggests weak sales. — [GearBrain](https://www.gearbrain.com/north-focals-smart-glasses-disabled-2646835849.html); [IPS News](https://ipsnews.net/business/?p=35006)
- Intel quietly discontinued Recon Jet and stopped taking orders on 29 Sept 2017. — [Bicycle Retailer](https://bicycleretailer.com/product-tech/2017/07/21/intel-quietly-discontinues-recon-jet-smart-sunglasses)
- Intel shut the roughly 100-person Recon unit in summer 2017 but said it would fulfil existing contracts. — [Bloomberg](https://www.bloomberg.com/news/articles/2017-10-06/intel-is-said-to-shutter-its-augmented-reality-goggles-brand); [UploadVR](https://uploadvr.com/report-intel-shutters-recon-ar-brand/)
- Intel shut down its New Devices Group, including the Vaunt glasses, in 2018. — [Healthcare Innovation](https://hcinnovationgroup.com/clinical-it/article/13009709/intel-reportedly-shutting-down-its-smart-glasses-group)
- OpenGlass was abandoned within about a year and folded into Omi, and its devkit is no longer sold. Even MIT-licensed projects strand users when the maintainer moves on. — [OpenGlass](https://github.com/BasedHardware/OpenGlass); [Omi docs](https://docs.omi.me/docs/hardware/OpenGlass)
- North Star's initial "open" release lacked electronics schematics and software. — [Tom's Hardware](https://www.tomshardware.com/uk/news/project-north-star-open-source-documents,37222.html)

### Inferences
- Design rules that follow:
  - Every feature must work with zero servers.
  - The BLE protocol must be published so third parties can replace the official app.
  - Firmware signing must allow the owner to install their own key, so OTA cannot be used to revoke features. This fits the existing OpenWR release-key work.
  - The project must stay buildable from the repo if the team disappears.
- "Open" releases that omit electronics (North Star 2018) or ship only PDFs and STLs (Frame) fall short of open hardware. Hiroglass should ship source CAD and a BOM with Mouser and Digikey part numbers.

### Gaps
- Google Glass Explorer lessons (privacy backlash, the 2015 consumer wind-down, the Enterprise Edition ending in 2023) were not sourced in this session.
- No source confirmed whether Recon Jet or Snow2 owners lost app or cloud functionality after 2017.

## Q4: Sports HUD products for goggles, glasses and helmets: mounts, protocols, survivors and casualties

### Takeaway
Of the brands checked, only the ActiveLook ecosystem (Engo, Julbo) has an openly documented protocol, and it has gathered third-party integrations from Garmin and Suunto. Recon died with Intel's exit. Forcite was absorbed by GoPro. Everysight has moved toward AI glasses. This session found no current evidence on Rekkie's status.

### Cited Findings
- ActiveLook is in Engo 1 and 2 and Julbo EVAD-1 and 2. It uses an open BLE protocol, and Garmin and Suunto watches drive it directly. — [ActiveLook app](https://apps.apple.com/app/id1464023773); [advnture](https://www.advnture.com/news/these-smart-shades-will-beam-stats-from-your-garmin-watch-in-front-of-your-eyes); [ActiveLook API docs](https://github.com/ActiveLook/Activelook-API-Documentation)
- Recon Jet was discontinued with Intel's wearables exit in 2017. — [Bicycle Retailer](https://bicycleretailer.com/product-tech/2017/07/21/intel-quietly-discontinues-recon-jet-smart-sunglasses); [Wikipedia: Recon Instruments](https://en.wikipedia.org/wiki/Recon_Instruments)
- Forcite (MK1S smart motorcycle helmet with app-driven road alerts) was acquired by GoPro in January 2024. — [CB Insights](https://www.cbinsights.com/investor/forcite); [TechCrunch](https://techcrunch.com/2023/06/23/forcite-seeks-the-smart-helmet-success-that-skully-squandered)
- Everysight will show its Maverick AI glasses at AWE USA 2026, and about 3,500 people backed Maverick AI and AI Pro on Kickstarter. — [Everysight LinkedIn](https://linkedin.com/company/everysight)
- Livall still sells smart bike helmets (EVO21 V2 with crash detection, SOS and lights), but none with a HUD was found. — [MediaMarkt CH](https://www.mediamarkt.ch/de/product/_livall-evo21-v2-m-54-58-cm-smarter-helm-2292934.html)

### Inferences
- An open, documented BLE command set (ActiveLook) is what drew watch-maker integrations. That is a direct model for Hiroglass and goggl1 to reach Garmin and Suunto users.
- Sports HUD vendors are often acquired or exit (Recon to Intel, Forcite to GoPro). Users of a closed app and cloud bear that risk, which reinforces Q3's design rules.

### Gaps
- Not researched or not found: mounting mechanisms (goggle strap or clip, helmet visor) for Recon Snow2, Rekkie, Engo, Everysight Raptor, Smith/Oakley Mod Live, CrossHelmet and Livall; Rekkie's status; Raptor's discontinuation date; CrossHelmet's status; the Oakley Airwave and Mod Live protocols.
