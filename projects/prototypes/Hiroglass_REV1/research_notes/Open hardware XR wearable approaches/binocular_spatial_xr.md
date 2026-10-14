# Open-hardware paths to binocular spatial XR (post-monocular HUD)

Research date: 2026-10-13. Scope: open approaches a 2-3 person team could take after Hiroglass REV1 (monocular clip-on HUD). Constraints: under $300, Mouser/Digikey where possible, no single-supplier parts, open software, tethered puck allowed, FPGAs allowed.

Note on method: about 16 search and fetch calls. One coordinator-noted network outage happened partway through. The FOSDEM 2026 Monado slides could not be fetched (404 on the archive), so claims from them come only from search snippets. Everything not backed by a source is listed under Inferences or Gaps.

## Q1: Open binocular AR/VR designs: what exists and what is alive (late 2026)

### Takeaway
Of the open binocular hardware, Project North Star is the most complete reflective-AR design, but its momentum stopped around 2020-2023. Relativty, the 3DoF VR reference, has been quiet since mid-2024. The most active open path in 2026 is software, not hardware: Monado (the OpenXR runtime) plus wheaney's XRLinuxDriver and Breezy Desktop, driving commercial birdbath glasses (Xreal, Viture, Rokid, RayNeo) over USB-C DisplayPort alt mode. Phone-in-a-reflector viewers such as HoloKit X are the cheapest route to 6DoF stereo AR.

### Cited Findings
**Project North Star (Leap Motion / Ultraleap, community-maintained)**
- Original 2018 spec: two 1600x1440 displays, combined FOV over 100 degrees, 120 fps, built for under $100 "at scale". Mechanical and electronic designs were released under Apache 2.0 — [Road to VR](https://www.roadtovr.com/leap-motion-reveals-project-north-star-an-open-source-wide-fov-ar-headset-dev-kit/); [GamesBeat](https://gamesbeat.com/leap-motion-shows-crazy-looking-100-north-star-ar-headset-with-hand-tracking/)
- Leap Motion said the goal was to explore what a fully realized AR experience feels like, not to ship a consumer headset — [Road to VR](https://www.roadtovr.com/leap-motion-reveals-project-north-star-an-open-source-wide-fov-ar-headset-dev-kit/)
- The community docs list these variants:
  - **Northstar Next** (CombineReality, "Active", dated 06 Jun 2023): XR50/T261 spatial tracking, SIR 170 hand tracking, BoboVR headgear. The docs call it the recommended starting point for new users, for modularity and affordability.
  - **Deck X** (CombineReality, "Active", 21 Aug 2020): T261 tracking, LMC/SIR 170 hands, integrated board that cuts the cables to two (USB 3 + mini-DP).
  - **Release 3-2** (Leap Motion): "In development, date TBD".
  - **Releases 2, 3, 3-1**: outdated, 2018-2019.

  — [Project North Star docs](https://docs.projectnorthstar.org/)
- The most recent dated variant in the docs is June 2023 — [Project North Star docs](https://docs.projectnorthstar.org/)
- The leapmotion/ProjectNorthStar GitHub repo is public and not archived. It has 55 commits, 1.8k stars, 279 forks, and 25 open issues. The Mechanical folder holds the BOM, fabrication and assembly docs, and the Software folder holds setup and calibration — [GitHub leapmotion/ProjectNorthStar](https://github.com/leapmotion/ProjectNorthStar)
- CombineReality announced the CR Deck Mk.1 in 2020: a North Star remix built from off-the-shelf parts with Ultraleap hand tracking. They planned a Kickstarter for an injection-molded version. Mk.2 was shown only as sketches — [UploadVR](https://uploadvr.com/cr-deck-mk-1-open-source-ar-headset-project-north-star/)
- A 2019 hands-on review describes community 3D-printed builds with reduced FOV — [Skarred Ghost](https://skarredghost.com/2019/10/30/north-star-virtualizer-2-hands-review/)
- The T261/T265 named in these designs are Intel RealSense tracking modules. Intel discontinued the T265, so this is a single-supplier, end-of-life dependency. — no fetched source; see Gaps.

**Relativty (open VR headset)**
- Fully open hardware, software and firmware. SteamVR support, 2K at 120 FPS. The basic model is 3DoF with no controllers. The README points people who need 6DoF and controllers to HadesVR — [Relativty README](https://cdn.jsdelivr.net/gh/relativty/relativty@master/README.md); [GitHub](https://github.com/relativty/Relativty)
- About $200 DIY. Custom PCB with a SAM3X8E Cortex-M3 and an MPU-6050 IMU. GPL-3.0 — [Hackster](https://www.hackster.io/news/relativty-is-an-open-source-vr-headset-you-can-build-for-200-cfe0bd1ec522); [OSSInsight](https://ossinsight.io/analyze/relativty/Relativty)
- Last push July 2024, per a directory listing, so treat it as dormant — [ecosyste.ms](https://awesome.ecosyste.ms/projects/github.com%2Frelativty%2Frelativty)

**Monado / OpenXR open runtime**
- Monado aims to be a complete, conformant open-source implementation of OpenXR on Linux, Android and Windows — [Monado site](https://monado.freedesktop.org/)
- Monado 25.1.0 was released 10 Dec 2025. It added drivers for the Blubur S1, Rift DK2 and SolarXR IPC, and improved the Razer Hydra, Vive Pro 2, Xreal Air 2 Ultra and a Fujitsu headset. On Android it can scan and save Cardboard calibration QR codes. It also added XR_EXT_hand_tracking_data_source and fixed hand-pose bugs — [GamingOnLinux](https://www.gamingonlinux.com/2025/12/open-source-xr-runtime-monado-25-1-0-released-with-expanded-hardware-support/)
- Monado's hardware table lists Xreal Air, Air 2 and Air 2 Pro as 3DoF on Linux. The original nreal Air driver was reverse-engineered — [Monado webpage index.md](https://gitlab.freedesktop.org/monado/webpage/-/raw/master/index.md); [Monado MR 1798](https://gitlab.freedesktop.org/monado/monado/merge_requests/1798)

**Birdbath glasses as displays (USB-C DP alt mode) with open drivers**
- wheaney/XRLinuxDriver (GPL-3.0) supports:
  - VITURE One, Lite, Pro, Luma, Luma Pro and Luma Ultra; Beast and Pro 2 are also listed, Beast needing full SDK support
  - TCL/RayNeo NXTWEAR S/S+, Air 2, 2s and 3s/Pro
  - Rokid Max and Air
  - XREAL Air, Air 2, Air 2 Pro, Air 2 Ultra, One, One Pro and 1S

  It turns head motion into mouse input and an external broadcast. Breezy Desktop supplies the 3DoF virtual display. Some "Supporter Tier" features are paid through Ko-fi — [GitHub wheaney/XRLinuxDriver](https://github.com/wheaney/XRLinuxDriver)
- Breezy Desktop: GNOME version released June 2024, Vulkan version also available. Monado support and KDE KWin effects are on the roadmap — [GamingOnLinux](https://gamingonlinux.com/2024/10/xr-glasses-on-linux-steam-deck-get-vastly-better-with-the-xr-gaming-plugin); [ecosyste.ms wheaney](https://sponsors.ecosyste.ms/accounts/wheaney)
- VITURE published a Linux SDK, which was incorporated into the driver — [GamingOnLinux](https://gamingonlinux.com/2024/10/xr-glasses-on-linux-steam-deck-get-vastly-better-with-the-xr-gaming-plugin)
- The Rust crate `ar-drivers` is a separate open driver for Rokid (Air, Max), Nreal and others — [docs.rs ar-drivers](https://docs.rs/crate/ar-drivers/latest)
- Xreal Air stereo mode is 1920x1080 per eye (3840x1080 "Full SBS"). The spec says up to 120 Hz in 2D and 90 Hz in 3D. A forum user reports 3D is limited to 60 Hz, which conflicts with the spec. 3D mode is toggled by a button hold — [droon.ee spec](https://droon.ee/toode/xreal-air-ar-prillid/?lang=en); [Xreal community](https://community.xreal.com/t/get-3840x1080-resolution-on-windows-make-screen-smaller-in-your-field-of-view/3379); [Xreal 3D mode tutorial](https://tutorials.xreal.com/docs/glasses/one-series/osd/3d-mode/)
- January 2026: draft code submitted to KDE Plasma turns it into a full VR desktop — [GamingOnLinux](https://www.gamingonlinux.com/2026/01/draft-code-submitted-to-kde-plasma-turns-it-into-a-full-vr-desktop/page=1) (title only; contents not fetched)

**Phone-based reflective viewers**
- HoloKit X: open-source iPhone stereoscopic AR headset, maintained by Reality Design Lab. 6DoF via ARKit, LiDAR environment sensing, 60 degree FOV, $129. Works best on LiDAR iPhones (12 Pro through 15 Pro Max) — [HoloKit docs](https://docs.holokit.io/product/holokit-x); [HoloKit store](https://holokit.io/products/holokit-x)
- A 2023 review said its status as an ongoing project limits the user experience — [AppleInsider](https://appleinsider.com/articles/23/02/08/holokit-x-review-still-developing-stereoscopic-ar)

### Inferences
- **Four concrete open paths:**
  - **(A) Commercial birdbath glasses as the display**, with an open software stack on the puck: XRLinuxDriver or ar-drivers, then Monado/OpenXR, then the app. Cheapest and fastest to binocular. The optics are not open, but the software is, and several vendors (Xreal, Viture, Rokid, RayNeo) are supported, which avoids single-supplier lock-in.
  - **(B) DIY birdbath from micro-OLED modules** (see Q3). Open mechanics and electronics, using Sony ECX335-class panels and Tindie or HK-sourced optics.
  - **(C) A North Star-style large reflective combiner**, best for wide-FOV research. Needs a replacement for the RealSense tracker, such as camera-based VIO on the puck.
  - **(D) Phone-as-compute in a reflector or Cardboard viewer** (HoloKit, Cardboard via Monado Android). Best for software and UX iteration before the custom optics are ready.
- North Star, CR Deck X and Relativty should be treated as reference designs to mine (mechanics, calibration ideas), not as live supply chains. Mark North Star "dormant since ~2023" and Relativty "dormant since 2024-07".
- Paths A and D let a team of 2-3 put most of its effort into the open software layer (Monado drivers, the protocol), which carries over to custom hardware later.

### Gaps
- Could not confirm an official Intel EOL notice for the T265/T261 in this session (it is widely known, but not fetched).
- No current CombineReality product availability or pricing found for Deck X or Northstar Next.
- Not verified whether XRLinuxDriver itself feeds Monado today. "Monado support" was a Breezy roadmap item.
- No Zapbox status was checked. HadesVR (the 6DoF Relativty successor) was not researched.

## Q2: Tracking (3DoF vs 6DoF), hand tracking, and compute

### Takeaway
3DoF IMU-only is solved and open: every glasses driver above does it. 6DoF open VIO exists through Monado's Basalt fork, but by Collabora's own account it is CPU-heavy and below commercial quality. Hand tracking in Monado uses stereo cameras and an ONNX model. No reliable published benchmarks were found for VIO on a Pi 5 or RK3588, so the compute choice should be validated with Monado's SLAM datasets before committing.

### Cited Findings
- Monado's main inside-out tracker is a Collabora fork of Basalt, integrated through the `vit_interface`. Persistent mapping was being added — [Collabora: OpenXR and Monado progress](https://www.collabora.com/news-and-blog/news-and-events/openxr-and-monado-recent-progress-and-whats-to-come.html); [Collabora: State of Monado VIT (Dec 2022)](https://Collabora.com/news-and-blog/blog/2022/12/20/state-of-monado-visual-inertial-tracking/)
- FOSDEM 2026 Collabora talk, per a search snippet (slides not retrievable): Basalt is CPU-heavy, and its tracking quality is not yet at the level of commercial systems. Monado's hand tracking uses stereo cameras plus an ONNX model trained on open and synthetic data — [FOSDEM 2026 slides (snippet)](https://fosdem.org/2026/events/attachments/T38AKM-monado-and-beyond/slides/266219/monado_an_tfzl8ak.pdf)
- Monado lists XR_EXT_hand_tracking, including Leap Motion via Ultraleap v2 — [Monado webpage](https://gitlab.freedesktop.org/monado/webpage/-/raw/master/index.md)
- The Monado SLAM datasets are egocentric visual-inertial recordings under CC-BY 4.0, made to improve the Basalt tracker — [Hugging Face collabora/monado-slam-datasets](https://huggingface.co/datasets/collabora/monado-slam-datasets/blob/d9f2c2c679e3bc1735bb4ba7eb76e2bb759f9c0c/README.md); [Collabora](https://collabora.com/news-and-blog/news-and-events/monado-slam-datasets-now-available.html)
- A Basalt issue asks for low-power settings on Raspberry Pi and Orange Pi hosts. The snippet showed no answers — [Basalt issue 117](https://gitlab.com/VladyslavUsenko/basalt/-/issues/117)
- A Politecnico di Torino thesis analyzes OpenVINS on a Rockchip board (Rock Pi N10). Numbers were not in the snippet — [PoliTo thesis](https://webthesis.biblio.polito.it/21099/1/tesi.pdf)
- ModalAI says its GPU-accelerated OpenVINS build runs 3 cameras at 30 fps in real time, on its own hardware. This is a vendor claim — [ModalAI forum](https://forum.modalai.com/post/25334)
- RK3588: Cortex-A76/A55 with a 6 TOPS NPU (useful for hand-tracking inference) — [tinycomputers.io](https://tinycomputers.io/categories/rk3588s.xml)
- HoloKit gets 6DoF from the phone's ARKit, which shows that phone-as-compute provides production-grade VIO without any custom SLAM — [HoloKit docs](https://docs.holokit.io/product/holokit-x)

### Inferences
- **Staged tracking plan:**
  - Stage 1: 3DoF from a good IMU (already in the REV1 path).
  - Stage 2: 6DoF via the phone (ARCore/ARKit) or via Monado+Basalt on an x86 or RK3588 puck, using a global-shutter stereo camera pair plus an IMU that is hardware-timestamped against the camera exposures.
  - Stage 3: hand tracking with Monado's ONNX pipeline on the NPU or GPU.
- **Compute ranking (judgment, not benchmarked):**
  - Jetson Orin Nano: best GPU and CUDA, but around $250 alone, which strains the budget.
  - RK3588 boards: multiple vendors (Radxa, Orange Pi, Firefly), which suits the no-single-supplier rule. Mali GPU and Panfrost/Panthor open drivers. The NPU uses a mostly closed toolchain.
  - Raspberry Pi 5/CM5: most open and best-documented, but weakest for VIO plus rendering.
  - Phone: the most capability per dollar, but the least open.
- Basalt and OpenVINS both need tight camera-IMU time sync. That is the hardware requirement most worth designing for early.

### Gaps
- No measured fps or CPU load for Basalt or OpenVINS on Pi 5, CM5 or RK3588 was found. This is the key unknown for picking the puck SoC.
- Not confirmed which open hand-tracking models (beyond Monado's) run in real time on an RK3588 NPU.
- No figures on Monado's SLAM CPU cost were found beyond "CPU heavy".

## Q3: Display and optics for binocular, IPD, VAC, calibration

### Takeaway
For a DIY binocular build under $300, the practical display is a pair of 0.71" 1920x1080 Sony ECX335-class micro-OLEDs, sold with HDMI driver boards and even complete 50 degree birdbath modules on Tindie. A North Star-style combiner with 3.5" LCDs gives a wide FOV but is bulky. Two-display alignment and calibration has open tooling only within the North Star and Monado ecosystems.

### Cited Findings
- A Tindie "Display Components" (Hong Kong) listing sells a birdbath optics module: 0.71" Sony ECX335SN 1920x1080 micro-OLED, 50 degree FOV, dual HDMI controller board. Listed prices varied from about $75 to $569 depending on configuration — [Tindie birdbath module](https://tindie.com/products/oled-modules/bird-bath-lenses-ar-glasses-optical-module-fov50)
- A standalone HDMI driver board for the ECX335A/ECX335S: 37.5 x 16.5 mm, 3.5-5 V — [Tindie driver board](https://tindie.com/products/oled-modules/hdmi-driver-board-for-071-inch-micro-oled-display)
- 0.5" Sony ECX337 (1280x960) HDMI and CVBS boards also exist, CVBS in a dual-display version — [Tindie ECX337 HDMI](https://www.tindie.com/products/oled-modules/hdmi-controller-board-for-05-arvr-microdisplay/); [Tindie ECX337 CVBS](https://tindie.com/products/oled-modules/cvbsav-board-for-sony-microdisplays)
- Sony ECX335S, distributed by FRAMOS: Full HD, up to 3,000 cd/m2, 1.8 cm diagonal, 21.44 x 15.62 mm outline, 100,000:1 contrast. The ECX335B variant is up to 500 cd/m2 with an LVDS interface, so check the exact variant before buying a driver board — [FRAMOS](https://www.framos.com/news/small-bright-high-contrast-framos-offers-new-high-resolution-oled-microdisplay-for-ar-applications); [Electronic Specifier](https://electronicspecifier.com/optoelectronics/high-resolution-oled-microdisplay-for-ar-applications)
- North Star uses two 1600x1440 panels at 120 fps for over 100 degrees combined FOV — [Road to VR](https://www.roadtovr.com/leap-motion-reveals-project-north-star-an-open-source-wide-fov-ar-headset-dev-kit/)
- North Star calibration documentation lives in the repo's Software folder — [GitHub](https://github.com/leapmotion/ProjectNorthStar)
- Monado on Android supports Cardboard calibration QR codes, an open per-viewer lens and IPD distortion profile format — [GamingOnLinux](https://www.gamingonlinux.com/2025/12/open-source-xr-runtime-monado-25-1-0-released-with-expanded-hardware-support/)

### Inferences
- Sony micro-OLEDs are effectively single-source (Sony), sold through FRAMOS and grey-market Tindie/HK sellers, and are not stocked at Mouser or Digikey. Consider multi-vendor alternatives such as BOE, SeeYA, or Olightek micro-OLEDs, or North Star-style LCDs from multiple Chinese panel vendors. Abstract the display behind a standard interface (MIPI DSI, or HDMI/DP via a bridge) so the panel can be swapped.
- VR (rather than AR) with Fresnel or pancake lenses plus 2.9-3.5" LCDs is cheapest, but it gives up passthrough. Video passthrough would need the same stereo cameras as VIO.
- IPD: birdbath eyeboxes are small, so mechanical IPD adjustment (a sliding rail per eye) is needed. Vergence-accommodation conflict is inherent to fixed-focus birdbaths. Placing the virtual image at about 2-4 m is the usual mitigation (common practice; not sourced here).
- Two-display alignment needs a camera-based calibration jig: per-eye distortion mesh plus extrinsics. Storing per-unit calibration on the device (an EEPROM or flash blob readable by the host) mirrors how commercial glasses work.

### Gaps
- No Mouser/Digikey-stocked micro-OLED with a Western datasheet was confirmed in this session.
- No open-source standalone two-display AR calibration tool, independent of North Star or Monado, was found. Not searched deeply.
- Pancake or Fresnel lens sourcing and prices were not researched.

## Q4: Latency and motion-to-photon constraints on architecture

### Takeaway
No primary latency measurements were found in this session. The architectural conclusion is still clear: binocular head-locked or world-locked content needs a wired video link (USB-C DP alt mode, HDMI, or MIPI) and a high-rate IMU path that is timestamped and fed to the renderer for late-stage reprojection. BLE can carry only HUD-style telemetry.

### Cited Findings
- Commercial glasses drivers expose head motion from the glasses' own IMU over USB to the host, and the host renders the 3DoF virtual display (Breezy). This is the host-renders plus glasses-IMU-over-USB architecture — [GitHub XRLinuxDriver](https://github.com/wheaney/XRLinuxDriver)
- North Star Deck X reduces its tether to USB 3 plus mini-DP — [Project North Star docs](https://docs.projectnorthstar.org/)
- Xreal stereo runs at up to 90 Hz in 3D per spec (60 Hz per one user report) — [droon.ee](https://droon.ee/toode/xreal-air-ar-prillid/?lang=en); [Xreal community](https://community.xreal.com/t/get-3840x1080-resolution-on-windows-make-screen-smaller-in-your-field-of-view/3379)
- The only latency evidence found was anecdotal: one user said games show "low to no lag" compared with a Quest 2 — [Steam community, Deep Desktop](https://steamcommunity.com/app/2947590/discussions/0/4637114871638463644)

### Inferences
- The widely cited target is about 20 ms motion-to-photon for world-locked VR/AR. That figure is industry lore (Carmack, Abrash) and was not sourced in this session. Meeting it at 60-90 Hz frames requires:
  - IMU at 500-1000 Hz streamed over USB, not BLE
  - render-time pose prediction
  - timewarp or reprojection close to scanout, which Monado's compositor does
  - a display with low persistence
- BLE connection intervals (7.5 ms minimum, often 15-30 ms, plus retries) and its throughput rule it out for video and make it marginal even for pose. This matches the REV1 team's own finding (see the project memory on BLE latency).
- An FPGA is a sensible experiment for a display-side "late warp" (reprojection on the glasses side of the link) or for exact camera/IMU timestamping. Keep it optional.

### Gaps
- No measured motion-to-photon numbers for Xreal, Viture or Rokid, or for Monado's compositor, were found.
- Xreal IMU sample rate and DP alt-mode specifics are unconfirmed.

## Q5: Holographic / passthrough research directions realistic for DIY (brief)

### Takeaway
This received little research time. Pepper's-ghost or beamsplitter combiners, which are what North Star and HoloKit are at heart, are the realistic DIY "passthrough HUD glass". True holographic optical elements and light-field displays remain lab-grade.

### Cited Findings
- HoloKit X uses a phone screen reflected through a combiner for stereoscopic AR with a 60 degree FOV — [HoloKit docs](https://docs.holokit.io/product/holokit-x)
- North Star uses large reflective combiners for its 100 degree FOV — [Road to VR](https://www.roadtovr.com/leap-motion-reveals-project-north-star-an-open-source-wide-fov-ar-headset-dev-kit/)

### Inferences
- **Realistic DIY options:**
  - beamsplitter or partial-mirror combiners (Pepper's ghost)
  - off-the-shelf birdbath modules
  - North Star combiners, printed or community-sourced
- **Speculative options:**
  - HOE kits (photopolymer films such as Litiholo-style hologram kits) for static holograms, not dynamic displays
  - Looking Glass-style lenticular light-field displays (not head-mounted)
- Waveguides are not DIY.

### Gaps
- No sources were gathered on DIY HOE recording for AR combiners, light-field HMDs, or Litiholo specifics. Not researched due to the tool budget.

## Q6: REV1 design choices that keep the binocular path open, and minimal binocular BOM

### Takeaway
Design REV1 so the head unit is a "dumb display plus sensors" peripheral of a puck: a USB-C tether that can carry DP alt mode or USB 3, a hardware-timestamped IMU rigidly mounted to the optics, a standard mounting rail, a per-unit calibration blob, and a host-side Monado driver. With those in place, binocular is mostly a matter of adding a second eye and cameras. A minimal binocular prototype fits under $300 using borrowed or purchased birdbath glasses (path A), or roughly at the limit with a DIY micro-OLED pair (path B).

### Cited Findings
- The existing open ecosystem converges on a USB-attached head unit with host-side rendering and an IMU stream (XRLinuxDriver, Monado drivers) — [GitHub XRLinuxDriver](https://github.com/wheaney/XRLinuxDriver); [Monado webpage](https://gitlab.freedesktop.org/monado/webpage/-/raw/master/index.md)
- Deck X shows a two-cable (USB 3 + DP) head unit as a proven pattern — [Project North Star docs](https://docs.projectnorthstar.org/)
- Monado runs on Linux, Android and Windows, so a single OpenXR driver covers a phone or SBC puck — [Monado site](https://monado.freedesktop.org/)
- Price anchors:
  - HoloKit X: $129 — [HoloKit store](https://holokit.io/products/holokit-x)
  - Relativty: about $200 DIY — [Hackster](https://www.hackster.io/news/relativty-is-an-open-source-vr-headset-you-can-build-for-200-cfe0bd1ec522)
  - Tindie 50 degree birdbath module: about $75-569 depending on option — [Tindie](https://tindie.com/products/oled-modules/bird-bath-lenses-ar-glasses-optical-module-fov50)

### Inferences
- **REV1 choices to make now:**
  1. **Tether**
     - Use USB-C with the full pin set routed: SuperSpeed pairs and SBU for DP alt mode, even if REV1 uses only USB 2 or power.
     - Leave the ESP32 or BLE link as an optional sideband only.
  2. **IMU**
     - Choose a 6-axis or 9-axis part with a FIFO that can run at 500-1000 Hz and has a hardware sync or timestamp input pin (FSYNC-style), so cameras can be synchronized later.
     - Mount it on the optics module, not on the clip, and document its extrinsic to the display.
     - Pick a part family with multiple vendors and Digikey/Mouser stock, such as ST LSM6DSx or TDK ICM-4xxxx.
  3. **Mechanics**
     - Use a rail or dovetail interface standard (for example a 20 mm rail with M2 inserts) that a second eye pod and a stereo camera bar can share.
     - Allow IPD sliding of 55-72 mm.
  4. **Software protocol**
     - Define a small, open, versioned USB HID or CDC protocol covering:
       - IMU samples with timestamps
       - device info
       - calibration blob read
       - display mode and brightness
     - Write a Monado driver for REV1 now, even if it is 3DoF and monocular, so apps target OpenXR from day one.
     - Also emit an XRLinuxDriver-style broadcast for Breezy compatibility.
  5. **Display abstraction**
     - Drive the panel over a standard interface (HDMI/DP via a bridge, or MIPI DSI) rather than a vendor-specific SPI path. This lets one host render for both eyes as a single SBS frame, the same convention Xreal's Full SBS uses.
  6. **Calibration**
     - Store a per-unit calibration (distortion mesh, IMU-to-display extrinsic, eye-pod ID) on the device.
- **Minimal binocular BOM (estimates, not quoted):**
  - **Path A**
    - Used or new birdbath glasses: about $150-300 (not sourced here)
    - Monado and XRLinuxDriver: $0
    - Existing laptop, or a puck SBC at about $100-150
    - Total: about $150-300 for 3DoF stereo.
    - Add a global-shutter stereo camera with IMU (for example an OV9281 pair, about $60-100; not sourced) for 6DoF on Basalt.
  - **Path B (DIY)**
    - Two ECX335 panels with HDMI driver boards, or a dual birdbath module: likely $150-400 (the Tindie price range is wide)
    - Printed frame plus rail: about $20
    - IMU board: about $15
    - Two-HDMI or DP-to-dual-MIPI bridge: varies
    - Total: around or above $300 before compute.
  - **Path D**
    - Cardboard or HoloKit viewer: $10-129
    - An existing phone running Monado Android or ARCore
    - Total: under $150.
  - **Path C (North Star)**
    - Cost depends on combiner and panel sourcing. Likely over $300 once a tracker is included. Not verified.

### Gaps
- No current prices were verified for Xreal, Viture or Rokid glasses, OV9281 stereo modules, or RK3588 boards in late 2026.
- No open-source dual-panel driver board on Mouser/Digikey parts (for example a Toshiba TC358xxx or Lontium LT6911-class bridge) was researched. This is a key open-hardware gap for path B.
- Whether any vendor's glasses accept a fully custom stereo signal without a firmware button toggle was not confirmed.
