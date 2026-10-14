# Microdisplays and Driving Electronics for a Low-Cost Open-Hardware HUD / XR Wearable (Hiroglass REV1)

Research date: 2026-10-13. Research was cut short by a time budget (about 16 tool calls) and a network outage partway through. Many sourcing questions (exact Digikey prices for small OLEDs, IMUs, charger ICs) were not checked live, so they appear under Gaps or as clearly labeled background-knowledge inferences. Prices are retailer snapshots from search results. Confirm them before buying.

## Display candidates and tradeoffs (resolution, brightness, interface, power, price, sourcing, NDA)

### Takeaway
For REV1 the practical choices are: (a) a Sony 0.23" 640x400 micro-OLED (ECX336 family), the panel Brilliant Labs Frame uses, driven by a small FPGA, or (b) a SeeYA 0.49" 1920x1080 MIPI-DSI micro-OLED, sold with Type-C/HDMI driver boards on Tindie and DisplayModule for about $149 and up. DLP2000 is the only TI-backed, Digikey-stocked, openly documented option, but it is bulky and power-hungry. JBD microLED and Himax LCoS are high-brightness and see-through friendly, but I found no evidence of hobbyist-quantity, non-NDA purchase.

### Cited Findings
**Sony micro-OLED (ECX33x)**
- Brilliant Labs Frame uses a 0.23" micro-OLED, 640x400 RGB, optically bonded to a prism with a 20° FOV. It connects to the FPGA in 8-bit YCbCr mode (4 wires Y, 2 Cb, 2 Cr), with up to 16 colors per frame. — [Frame Hardware Manual](https://docs.brilliant.xyz/frame/hardware)
- The Sony ECX336C is a 0.23" nHD+ (640x400) microdisplay. A Tindie seller ("Display Components", Hong Kong) sells an HDMI driver board for the 0.23" 600x400 micro-OLED that it says works with Sony ECX336A/B/C and can be powered from HDMI. The price list on the page runs about $75 to $574 and does not clearly map prices to products. — [Tindie HDMI driver board for 0.23" micro OLED](https://tindie.com/products/oled-modules/hdmi-driver-board-for-the-sony-microdisplays)
- The same seller offers an HDMI board for the Sony ECX334C, a 0.39" 1024x768 panel. — [Tindie ECX334C HDMI board](https://www.tindie.com/products/oled-modules/hdmi-controller-driver-board-for-the-microdisplays/)
- FRAMOS lists Sony ECX334A/ECX334C as distributed products. Sony describes the ECX334C module as integrating both a panel driver and a logic driver. — [FRAMOS ECX334C](https://www.framos.com/it/ecx334c-22710); [FRAMOS news](https://framos.com/en/news/sony-s-new-super-small-and-fast-solution-to-electronic-view-finders-and-wearables)
- Brilliant Labs Monocle used the Sony ECX335AF. The image data path goes straight to an FPGA and the MCU handles only the SPI configuration interface. Monocle's docs say no public datasheet appears to be published online for the ECX335AF, and they cite the ECX336CN datasheet (section 10.8) for luminance. The driver exposes six luminance presets (DIM to BRIGHT) plus sleep/awake. — [Monocle docs, ECX335AF driver](https://monocle-docs.readthedocs.io/en/latest/firmware_driver_api/ecx335af.html)
- Sony has announced high-brightness OLED microdisplays (details not extracted). — [OLED-Info](https://www.oled-info.com/sony-announces-new-high-brightness-oled-microdisplays)

**SeeYA (China) micro-OLED**
- SeeYA SEY049-19201080: 0.49", 1920x1080, 90 Hz, MIPI DSI, 5 µm pixel pitch, in 1,800-nit and 3,000-nit versions plus monochrome green. Demo kits are available. — [OLED Marketplace](https://marketplace.oled-info.com/view/194/SEY049-19201080)
- A Tindie seller sells the 0.49" 1080p 1,800-nit MIPI panel with a controller board in Type-C, mini-HDMI, HDMI and binocular variants, priced about $149 to $579. — [Tindie 0.49" 1080p](https://www.tindie.com/products/oled-module/0491080p-micro-oled-display-1920x1080-1800nits/); [Tindie listing 2](https://www.tindie.com/products/oled-module/049-micro-oledos-display-1920x1080-1800nits-mipi/)
- DisplayModule sells 0.49" 1920x1080 MIPI micro-OLEDs (1,800 and 3,000 nits, 90 Hz) with a Type-C-to-MIPI drive board. It says samples ship immediately and factory bulk lead time is 3 weeks. It also sells a 0.7" 1920x1080 3,000-nit micro-OLED with LVDS. — [DisplayModule 0.49" 3000 nit](https://www.displaymodule.com/products/0-49-micro-oledos-display-1920x1080-90hz-3000nits-mipi-1003); [DisplayModule 0.7" LVDS](https://www.displaymodule.com/de/products/0-7-inch-micro-oled-high-brightness-display-1920x1080-3000nits-with-lvds); [DM-OLED049 datasheet PDF](https://cdn.shopify.com/s/files/1/0264/7629/files/DM-OLED049-1002.pdf)
- Part-number variants differ: one SeeYA datasheet is titled SY049WDM01, and an eBay listing uses SY049LDM02 for a monochrome 20,000-nit version. — [search summary of eBay listing](https://www.ebay.de/itm/126422741675)

**TI DLP pico**
- DLPDLCR2000EVM (DLP2000 DMD + DLPA1000 PMIC + DLPC2607 controller): $118.80 at DigiKey US with 40 in stock, part status Active, 12-week lead time. Farnell BE lists €108.31 with a 54-week manufacturer lead time. The launch price was $99. — [DigiKey](https://www.digikey.com/en/products/detail/texas-instruments/DLPDLCR2000EVM/7598640); [Farnell](https://be.farnell.com/texas-instruments/dlpdlcr2000evm/evaluation-module-display/dp/3009788); [TI tool page](https://www.ti.com/tool/DLPDLCR2000EVM); [EPDT launch article](https://www.epdtonthenet.net/article/135966/Industry-s-most-affordable-DLP-solution.aspx)
- TI also lists a DLPM2000EVM tool. — [TI DLPM2000EVM](https://edgeworker.ti.com/tool/ja-jp/DLPM2000EVM)

**Himax LCoS**
- Himax front-lit LCoS (CES 2026 with AUO waveguide): 720x720, up to 350,000 nits and 1 lumen at about 200 mW, illumination plus panel in a volume as small as 0.09 cc. — [Nasdaq/Himax press release](https://www.nasdaq.com/press-release/himax-and-auo-partner-unveil-ultra-slim-high-brightness-lcos-microdisplay-ces-2026)
- Earlier generation (2025): color-sequential front-lit LCoS, 400K nits, about 300 mW typical, 720x720 at a 4.25 µm pitch. Himax also showed a dual-edge front-lit LCoS at Display Week 2026. — [CES 2025 release](https://www-web.itiger.com/news/2494722825); [EEJournal Display Week 2026](https://www.eejournal.com/industry_news/himax-to-showcase-industry-leading-high-contrast-dual-edge-front-lit-lcos-microdisplay-at-sid-display-week-2026/)
- I could not confirm the "HX7318" part number from any source.

**JBD microLED**
- JBD Hummingbird polychrome microLED projector (0.13" panels) was announced in 2023 and has been in mass production since May 2023. — [Macau Business / JBD PR](https://macaubusiness.com/jbd-launches-hummingbird-polychrome-microled-projector-an-industry-supremacy-for-consumer-ar-market-equipped-with-sid-2023-display-of-the-year-award-winning-0-13-microled-panels-in-mass-p/)
- In August 2026, JBD said it would release the JBD4UM480P series (0.13", 640x480, AmµLED family) "in November", with four kits: a single monochrome display, a set of monochrome R/G/B displays, and monochrome and polychrome projector kits. — [LEDinside Aug 2026](https://www.ledinside.com/news/2026/08/jbd_microled)
- JBD's Roadrunner platform (2.5 µm pitch) is moving toward mass production, with rollout expected in 2H 2026. — [MicroLED-Info](https://www.microled-info.com/jbd-launches-its-latest-microled-microdisplay-platform-reaching-pixel-pitch-25)

**Brilliant Labs Halo (2025) as a reference point**
- Halo uses a 0.2" full-color micro-OLED that the wearer looks up into (not a beam-splitter lens). It runs on an Alif B1 with an NPU, claims up to 14 h battery, launched at $299, and has design files and code published on GitHub. — [Road to VR](https://roadtovr.com/brilliant-labs-halo-smart-glasses-price-release-date/); [Hackster](https://www.hackster.io/news/brilliant-labs-unveils-the-halo-smart-glasses-with-narrative-ai-and-vibe-mode-9755a165531d); [Wareable](https://www.wareable.com/wearable-tech/brilliant-labs-halo-smart-glasses-ai-release-date-price)

### Inferences
- **Sony ECX33x is the open-hardware precedent.** Monocle and Frame both prove an ECX335/336 driven by a small FPGA works at under $350 retail. But Sony datasheets are not public: Monocle found none, and Frame's FPGA code is effectively the open documentation. It is single-source (Sony), sold through FRAMOS or grey-market Tindie/AliExpress sellers.
- **SeeYA 0.49" 1080p is the easiest "buy it today with a board" option.** It is sold at quantity 1, with Type-C or HDMI driver boards, from more than one reseller (Tindie and DisplayModule). It is MIPI DSI, though, so driving it natively needs a DSI-capable host. Its 1080p resolution is also overkill for a text HUD.
- **For REV1 monocular text/glyph HUD:** a tiny SPI OLED (SSD1306/SH1106/SSD1351) behind a birdbath or prism is cheapest and fully open. The existing goggl1 path likely extends naturally here. Sony 640x400 plus FPGA is the step up. The "see-through outdoors" story needs either high-nit micro-OLED (3,000 nits or more) or LCoS/microLED, which are NDA/volume parts.
- **Brightness classes, from cited figures:** micro-OLED about 1,800 to 3,000 nits (SeeYA). LCoS 350k to 400k nits at the panel (Himax, before waveguide losses). Panel-level nits are not eye-level nits after a combiner.
- **Optics direction:** Halo's move to a "look up into" module suggests that, for low cost, a small display module with simple optics beats waveguides.

### Gaps
- No live prices or datasheet checks were done for SSD1306/SH1106 (0.42" to 0.96"), SSD1351, ST7735, ST7789, GC9A01, or transmissive LCD with backlight. These are widely multi-sourced on LCSC, AliExpress and Adafruit, but no source was gathered in this session.
- BOE and Olightek micro-OLED listings and prices were not researched.
- Sony ECX336C/ECX337 nit and power figures and the current Sony part status were not found. I found no confirmation of whether ECX335 is EOL.
- Omnivision LCoS (OP02220 etc.) was not researched.
- Price, NDA status and Digikey availability of the JBD4UM480P kits were not found. I could not confirm whether the "November" release happened.
- Whether the DLP230NP has an affordable EVM was not checked.

## Which microdisplays can be bought in quantity 1-100 without NDA; breakout/driver boards

### Takeaway
Without NDA at quantity 1-100: TI DLP2000 EVM (Digikey, about $119), SeeYA 0.49" 1080p micro-OLED with Type-C/HDMI boards (Tindie/DisplayModule, about $149+), and Sony ECX336/ECX334 with HDMI boards (Tindie, grey market, datasheets not public). JBD and Himax appeared only as press releases and kits, with no open retail channel found.

### Cited Findings
- Sony ECX336A/B/C HDMI driver board on Tindie. — [Tindie](https://tindie.com/products/oled-modules/hdmi-driver-board-for-the-sony-microdisplays)
- SeeYA 0.49" with Type-C/HDMI/binocular boards, $149 to $579. — [Tindie](https://www.tindie.com/products/oled-module/0491080p-micro-oled-display-1920x1080-1800nits/)
- DisplayModule says samples ship immediately and it offers a Type-C-to-MIPI board. — [DisplayModule](https://www.displaymodule.com/products/0-49-micro-oledos-display-1920x1080-90hz-3000nits-mipi-1003)
- DLPDLCR2000EVM is in stock at DigiKey and Active. — [DigiKey](https://www.digikey.com/en/products/detail/texas-instruments/DLPDLCR2000EVM/7598640)
- Open-source driver precedents: Monocle firmware has an ECX335AF driver. Frame's FPGA source is in the Frame codebase repo (per the manual). — [Monocle docs](https://monocle-docs.readthedocs.io/en/latest/firmware_driver_api/ecx335af.html); [Frame Hardware Manual](https://docs.brilliant.xyz/frame/hardware)
- A GitHub search turned up no standalone open-source FPGA (Verilog/VHDL) driver for ECX335/336 outside the Brilliant Labs repos. — [search result summary; Monocle docs as closest hit](https://monocle-docs.readthedocs.io/en/latest/firmware_driver_api/ecx335af.html)

### Inferences
- Brilliant Labs' Frame FPGA (Verilog) and nRF52 firmware are the best reusable open reference for driving a Sony 640x400 micro-OLED. Check their license before reuse.
- A Type-C (DisplayPort Alt-Mode) or HDMI driver board for SeeYA lets a Linux puck (Pi or Rockchip) or a phone drive the display with no custom display electronics. That is fast for experimentation, but board firmware is closed.

### Gaps
- No AliExpress listings were verified, and supplier counts per panel were not established.
- OpenGlass and Hackaday ECX335 projects were not reviewed.

## Compute/driver options; driving MIPI DSI; open FPGA toolchains

### Takeaway
The proven open architecture is BLE MCU (nRF52840) + small FPGA (Lattice CrossLink-NX) + Sony micro-OLED over a parallel YCbCr bus, with the MCU configuring the panel over SPI. For MIPI DSI micro-OLEDs, the low-cost options are ESP32-P4 (native MIPI DSI, cheap boards about $35 to $60), a Linux SoC puck, or a DSI-capable FPGA (CrossLink-NX has MIPI D-PHY hard IP).

### Cited Findings
- Frame: Lattice CrossLink-NX LIFCL-17 (17k LUTs, 432 kb EBR, 2.56 Mb large RAM) for display graphics acceleration and the camera interface. It talks to the MCU over SPI, is loaded via SPI at boot, and is power-gated dynamically. The MCU is an nRF52840 (Cortex-M4F at 64 MHz, 1 MB flash, 256 KB RAM, BT 5.3) running a Lua-based OS. — [Frame Hardware Manual](https://docs.brilliant.xyz/frame/hardware)
- Monocle: the MCU does SPI config only and the display data path goes to an FPGA. — [Monocle docs](https://monocle-docs.readthedocs.io/en/latest/firmware_driver_api/ecx335af.html)
- Halo moved to the Alif B1 (MCU plus NPU). — [Road to VR](https://roadtovr.com/brilliant-labs-halo-smart-glasses-price-release-date/)
- ESP32-P4 MIPI-DSI boards: Guition JC1060P470 7" 1024x600 at about $34 to $42. Waveshare ESP32-P4-WIFI6 7" dev board at $47 to $56. 4D Systems ESP32-P4 8" and 9" modules at £82 to £107. None found at 1080p. — [atomic14](https://www.atomic14.com/esp32/boards/guition-jc1060p470/); [Waveshare MIPI LCDs](https://www.waveshare.com/product/esp32-related/displays/lcd-mipi.htm); [RS 4D Systems](https://uk.rs-online.com/web/p/lcd-colour-displays/0800330)

### Inferences
- **To drive a MIPI DSI micro-OLED:** you need a DSI host (ESP32-P4, a Pi CM4/CM5 or Rockchip on the puck, STM32U5/H7 parts with DSI, or an FPGA with D-PHY such as CrossLink-NX). You also need the panel's init sequence, which usually comes from the vendor or the driver-board vendor. That is the true "NDA" bottleneck. Whether ESP32-P4's DSI bandwidth can do 1920x1080 at 60-90 Hz was not verified, and it is likely limited (unverified).
- **Open toolchains:** Yosys/nextpnr support iCE40 and ECP5 well. Support for CrossLink-NX (Nexus) through Project Oxide exists but is less mature (background knowledge, not sourced here). Choosing an iCE40 or ECP5 for parallel-RGB/YCbCr panels keeps the stack fully open. MIPI D-PHY hard IP on Nexus is a reason to accept a partially vendor toolchain.
- RP2040/RP2350 PIO can bit-bang parallel RGB/YCbCr for 640x400 at moderate refresh rates. This is a candidate FPGA-free path for Sony ECX336 (inference, untested).

### Gaps
- RP2350, nRF54, STM32 LTDC/DSI and Gowin specifics and prices were not researched.
- Exact ESP32-P4 DSI lane rate and maximum resolution were not sourced.

## Power budgets and battery

### Takeaway
A complete Frame-class device (micro-OLED + FPGA + nRF52840 + camera) draws about 80 mA typical (45 to 100 mA) on a 210 mAh battery, so about 2 to 3 h. A 4 to 8 h target needs roughly 400 to 800 mAh or a tethered puck. LCoS light engines run about 200 to 300 mW on their own.

### Cited Findings
- Frame: 210 mAh (2 x 105 mAh Li-ion). Operating 45 to 100 mA (80 mA typical). Sleep 580 µA, shutdown 132 µA. PMIC is the MAX77654 (ADI, integrated charger). The charging cradle has a 140 mAh cell and USB-C. — [Frame Hardware Manual](https://docs.brilliant.xyz/frame/hardware)
- Himax LCoS: about 200 mW (2026 gen) and about 300 mW (2025 gen). — [Himax CES 2026](https://www.nasdaq.com/press-release/himax-and-auo-partner-unveil-ultra-slim-high-brightness-lcos-microdisplay-ces-2026); [Himax CES 2025](https://www-web.itiger.com/news/2494722825)
- Halo claims up to 14 h of battery. — [Wareable](https://www.wareable.com/wearable-tech/brilliant-labs-halo-smart-glasses-ai-release-date-price)

### Inferences
- At 80 mA, 4 h needs about 320 mAh and 8 h about 640 mAh, plus margin. Arithmetic from Frame's figures.
- A puck with an 18650 or a 1,000 to 2,000 mAh LiPo makes Linux SoCs and DSI micro-OLEDs viable. A headpiece-only design should stay MCU-class.
- The MAX77654 is a proven, single-source PMIC choice. Multi-source alternatives (TI BQ2407x/BQ25xxx, generic TP4056-class chargers) were not verified here.

### Gaps
- Per-display mW for SSD1306/ST7789/Sony/SeeYA panels was not sourced. DLP2000 EVM power was not sourced.
- USB-C PD sink ICs were not researched.

## Sensors commonly paired

### Takeaway
Frame's set is a good minimal reference: a 6-axis accelerometer/e-compass (MEMSIC MC6470) with a tap interrupt, an OmniVision OV09734 720p camera through the FPGA, and a TDK ICS-41351 PDM MEMS mic straight to the MCU.

### Cited Findings
- Frame sensors: MEMSIC MC6470 IMU (accelerometer plus e-compass, interrupt for always-on tap). OV09734 camera (1280x720, cropped to 720x720, YCbCr, configured through the FPGA). TDK ICS-41351 MEMS mic (4 to 16 bit, 4 to 20 kHz). — [Frame Hardware Manual](https://docs.brilliant.xyz/frame/hardware)
- Halo includes an optical sensor, IMU, mic array and bone-conduction speakers. — [Road to VR](https://roadtovr.com/brilliant-labs-halo-smart-glasses-price-release-date/)

### Inferences
- For head-tracked HUD content, a gyro-equipped IMU (BMI270, ICM-42688-P, LSM6DSO, all multi-sourced through distributors) is preferable to Frame's accel/mag-only MC6470 (background knowledge).
- An ambient light sensor for auto-brightness was not evidenced in the Frame manual.

### Gaps
- No sourcing or price data was gathered for BMI270/ICM-42688/LSM6DSO, ALS parts, or OV2640/OV5640/GC0308.

## Wireless: BLE throughput vs. primitives; Frame/Monocle/ActiveLook protocols

### Takeaway
Even ideal BLE 2M PHY (about 1.3 to 1.4 Mbps application throughput) cannot stream uncompressed micro-OLED video. Real ESP32-S3 numbers are about 46 to 63 KB/s. So shipping commercial glasses render on the glass: ActiveLook uses drawing commands plus preloaded fonts, bitmaps and layouts, and Frame runs Lua on the nRF52840.

### Cited Findings
- BLE 2M PHY with DLE and ATT MTU 247: calculated about 1,301 kbps at a 7.5 ms interval and about 1,366 to 1,401 kbps at long intervals (nRF52-to-nRF52). iOS and Android limits vary by OS version. — [Novel Bits](https://novelbits.io/bluetooth-5-speed-maximum-throughput/)
- DLE raises the link-layer payload from 27 to 251 bytes, and the optimal MTU is 247. — [Nordic DevZone](https://devzone.nordicsemi.com/f/nordic-q-a/28682/ble_app_att_mtu_throughput-throughput-calculation-packets-per-interval-data-length-extension)
- ESP32-S3 (IDF 5.0.2) throughput example: 46 KB/s measured. The README claims about 63 KB/s (500 kbps). A reply claims more than 60 KB/s with 2M, DLE, tuned intervals and L2CAP CoC (unverified forum claim). — [esp32.com](https://esp32.com/viewtopic.php?p=134134); [esp32.com reply](https://esp32.com/viewtopic.php?p=134183)
- ActiveLook: custom GATT "Commands Interface" service with five characteristics. Commands draw shapes and text and display preloaded bitmaps, layouts and fonts to reduce traffic and save power. The full spec is in ActiveLook_API.md (not reviewed). — [ActiveLook API docs](https://github.com/ActiveLook/Activelook-API-Documentation)
- Frame runs a Lua-based OS on the nRF52840 with OTA firmware. — [Frame Hardware Manual](https://docs.brilliant.xyz/frame/hardware)

### Inferences
- 640x400 at 16 colors (4 bpp) is 128 KB per frame, so 1 to 2 fps over a real phone link uncompressed. Text and primitives are tiny by comparison. This matches the goggl1 command approach. Design the Switchback/ProjectMagNET protocol as primitives plus cached assets, ActiveLook-style.
- For video or mirrored screens, use Wi-Fi (ESP32-S3/C6 or a puck SoC) or a cable to the puck.

### Gaps
- Frame's BLE MTU and data protocol details were not extracted (see the Frame SDK docs).
- The ActiveLook command table and display spec (resolution and color depth) were not reviewed.

## Supply-chain risk: multi-source parts and pin-compatible alternates

### Takeaway
Microdisplays are the single-source choke point. Sony, SeeYA, JBD, Himax and TI each sole-source their panels with proprietary init and timing, while MCUs, IMUs, chargers and small SPI OLED/TFT controllers are multi-sourced. Design the display interface so the panel is swappable (FPGA or PIO frontend, parallel RGB/YCbCr or SPI).

### Cited Findings
- Sony ECX335AF has no public datasheet. — [Monocle docs](https://monocle-docs.readthedocs.io/en/latest/firmware_driver_api/ecx335af.html)
- DLP2000 EVM lead times range from 12 weeks (DigiKey) to 54 weeks (Farnell), despite stock on hand. — [DigiKey](https://www.digikey.com/en/products/detail/texas-instruments/DLPDLCR2000EVM/7598640); [Farnell](https://be.farnell.com/texas-instruments/dlpdlcr2000evm/evaluation-module-display/dp/3009788)
- SeeYA 0.49" is resold by at least two channels (Tindie seller, DisplayModule), with inconsistent part numbers across listings. — [OLED Marketplace](https://marketplace.oled-info.com/view/194/SEY049-19201080); [DisplayModule](https://www.displaymodule.com/products/0-49-micro-oledos-display-1920x1080-90hz-3000nits-mipi-1003)

### Inferences
- The 0.49" 1080p MIPI micro-OLED format appears to be offered by several Chinese makers (SeeYA, BOE and others). That could give a semi-second-source at the board level, but init sequences differ. Not verified.
- Small SSD1306/SH1106 and ST7789 modules are effectively commodity, with many clones and pin-compatible footprints (background knowledge).

### Gaps
- No verified list of pin-compatible micro-OLED alternates was found.
- EOL status of Sony ECX335 and ECX336 was not confirmed.
