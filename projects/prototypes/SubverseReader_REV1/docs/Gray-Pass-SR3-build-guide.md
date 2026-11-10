# SR-3 Gray Pass: build guide

**Status:** draft, 2026-11-09. Nobody has built this yet, so treat every step as a first
attempt. Each step ends with a check: don't move on until it passes.
**Design:** [DESIGN-SubverseReader.md](DESIGN-SubverseReader.md) §10 (SR-3 row), §8 (keys),
"Power: auto-off, any key wakes". **BOM:** [`../bom/SR-3.csv`](../bom/SR-3.csv).

The Gray Pass is a 4.7in 960×540 16-grey e-paper reader worn on a lanyard: ESP32-S3,
microSD, USB-C, three keys on the right edge (UP, OK, DOWN), a 1200 mAh cell, and a printed
PETG shell measuring 78 × 149 × 14.5 mm. You need no custom PCB. You add keys, a switch and
a bigger battery to a ready-made e-paper board.

## 0. Read this first

| | State on 2026-11-09 |
|---|---|
| Base board | The **M5Stack PaperDIY kit (K159) is sold out**. This guide builds on the **LilyGo T5 4.7in S3** (ships from LilyGo's China warehouse; sold out in the US and DE). The kit's differences are in [Appendix A](#appendix-a-m5stack-paperdiy-k159-instead). |
| Enclosure | **Not drawn yet.** No STL exists. §9 has a stop-gap (a rigid badge holder) and the dimensions the shell must meet. |
| Firmware | **Not written yet.** You can bring up the board, keys, sleep and grey images with LilyGo's examples (§5–§8). The SubverseReader firmware (pack reader, sync, menus) comes later; see DESIGN §8 and §12. |
| Unverified | LilyGo board outline, battery connector type, and whether 380 µA sleep can be lowered. Each is a check in the steps below. |

Budget: about **$52 a pass for 5** with the LilyGo board ($39.99 + the rest of the BOM),
**$37 with the M5 kit** if it comes back. Time: about 3 weeks for parts (the LilyGo board is the
slow one), then one evening of soldering and one of bring-up.

**LiPo safety.** Never short the cell's leads. Check polarity before every first connection.
Charge on a non-flammable surface, and stop using a cell that swells.

## 1. Parts

For one pass. Multiply by 5 for the batch the costs above assume. Prices are from the BOM
(2026-11-09).

| Ref | Part | Qty | Where | ~$ each | Notes |
|---|---|---|---|---|---|
| BRD | LilyGo T5 4.7in E-Paper **S3** (ESP32-S3R8, 16 MB flash, 8 MB OPI PSRAM, ED047TC1 960×540) | 1 | lilygo.cc / LilyGo AliExpress store | 39.99 | Make sure you get the **S3** board, not the original ESP32 T5 4.7. The **S3 Pro** is a different board. |
| KEY | Omron B3F-1000 6×6 mm tact switch | 3 | DigiKey SW400-ND / Mouser 653-B3F-1000 | 0.35 | UP, OK, DOWN. |
| D | Schottky diode, THT: BAT43, 1N5817 or 1N5819 | 2 | DigiKey / Mouser / bench stock | 0.15 | Joins UP and DOWN to the wake line (§4). **Use Schottky, not a 1N4148:** see §4. |
| PWR | C&K JS102011SAQN slide switch | 1 | DigiKey 401-2000-ND | 0.45 | Cuts the battery for storage and shipping. |
| BAT | LiPo 603450, 1200 mAh, protection board, JST-PH 2.0 lead | 1 | Amazon multi-pack / Adafruit / SparkFun | 7.00 | Check the connector and polarity against the board (§3). Max size 6.0 × 34 × 50 mm. |
| ENC | PETG shell, front + back (~45 g) | 1 | own printer | 0.90 | Not drawn yet (§9). |
| CAP | Key caps, printed TPU | 3 | own printer | 0.03 | |
| LAM | Vinyl laminate band ("PHOTO PIT") | 1 | sticker shop | 0.15 | |
| HW | M2×6 screws + M2 heat-set inserts | 4 + 4 | Amazon | 0.10 | |
| LNY | 20 mm lanyard, swivel hook, 25 mm split ring | 1 | Amazon multi-pack | 1.50 | |
| — | microSD card, 1–32 GB, FAT32 | 1 | anywhere | — | Not in the BOM. |
| — | 30 AWG silicone wire (3 colours), 2 mm heat-shrink, Kapton tape, foam tape | — | bench stock | — | |

The Distributor parts (KEY, PWR, D) fit one DigiKey or Mouser order. Upload
`../bom/SR-3.digikey-import.csv` and add the diodes by hand: they aren't in the BOM yet.

**Tools:** temperature-controlled soldering iron with a fine tip, flux, flush cutters,
tweezers, multimeter, USB-C data cable, 3D printer (PETG + TPU), heat-set insert tip.
For §8, a µA meter: Nordic PPK2, µCurrent, or a multimeter on its µA range with a bypass
jumper.

## 2. Bring up the bare board

Do this before soldering anything, so a dead board shows up while you can still return it.

1. Install Arduino IDE 2 and the **esp32 core 2.0.17**. LilyGo states "2.0.5 or above and
   below 3.0", and the library doesn't build on core 3.x.
2. Install the libraries: **LilyGo-EPD47** (from
   <https://github.com/Xinyuan-LilyGO/LilyGo-EPD47>, branch `esp32s3`), **SensorLib 0.3.1**,
   **Button2 2.3.2**.
3. Board settings:

   | Setting | Value |
   |---|---|
   | Board | ESP32S3 Dev Module |
   | USB CDC On Boot | Enabled |
   | Flash Size | 16MB (128Mb) |
   | Flash Mode | QIO 80MHz |
   | Partition Scheme | 16M Flash (3MB APP/9.9MB FATFS) |
   | PSRAM | OPI PSRAM |
   | USB Mode | Hardware CDC and JTAG |

   With arduino-cli the FQBN is
   `esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi`.
4. Flash the **demo** example. If the port doesn't appear or the upload fails, hold
   **BOOT**, tap **RST**, release BOOT, and upload again.
5. Flash **grayscale_test**.

**Check:** the demo draws, and grayscale_test shows 16 distinct bands with no stuck lines.
Then put a FAT32 microSD card in, open the serial monitor, and confirm the card is
detected (the demo or `SD.begin(42)` with SPI on 11/15/16).

## 3. Measure and inspect

Write these down. They decide the enclosure and the battery lead.

1. **Board outline** (length × width) and the **thickest point** (usually the
   microSD socket or the USB-C socket), using calipers. LilyGo doesn't publish them.
2. **Battery connector:** the type (JST-PH 2.0 or 1.25 mm) and which pin is marked + on
   the silkscreen or schematic (`schematic/T5-ePaper-S3-V2.3.pdf` in the LilyGo repo).
3. **Free pins:** find the header pins for **GPIO39, GPIO48, GPIO21** (or the
   BOOT/user-button pad for GPIO21), **3V3** and **GND**.

**Check, enclosure:** the board must fit in **74 × 128 × 9 mm**. That's the inside of
the 78 × 149 × 14.5 shell after 1.6 mm walls, the band zone and the cell. If it doesn't, the
shell grows (§9).

**Check, cell lead:** with the multimeter on DC volts, put red on the cell's red wire and
black on its black wire. You should read +3.6 to +4.2 V. Then confirm the board's + pin
takes that red wire. Generic cells come wired both ways round. If the polarity is reversed,
swap the two crimps in the housing: lift each latch with a needle and swap them. Don't cut
both wires at once.

## 4. Key wiring

The S3 can wake from deep sleep only on RTC GPIOs (0–21). On this board the only free one
is **GPIO21**, the user button. GPIO39 and GPIO48 are free but can't wake the chip. So every
key pulls GPIO21 low to wake the pass, and UP and DOWN also have a pin each, so the
firmware can tell which key was pressed. This is the same diode-OR ANY-key line SR-1 uses.

```
  GPIO21 ──┬──────────────┬───────────────┐      (GPIO21 has the on-board pull-up)
           │              │               │
          [OK]           D1              D2       D1, D2: Schottky, anode at GPIO21,
           │              │               │               cathode (bar) at the key
          GND    GPIO39 ──┤      GPIO48 ──┤
                          │               │
                         [UP]          [DOWN]
                          │               │
                         GND             GND
```

In words:
- **OK:** one side to GPIO21 (in parallel with the on-board user button), the other to GND.
- **UP:** one side to GND. The other side goes to GPIO39 and to the **cathode** (bar end) of
  D1. D1's anode goes to GPIO21.
- **DOWN:** the same, with GPIO48 and D2.

How the firmware reads it (internal pull-ups on 39 and 48):

| Pressed | GPIO21 | GPIO39 | GPIO48 |
|---|---|---|---|
| none | high | high | high |
| OK | low | high | high |
| UP | low | low | high |
| DOWN | low | high | low |
| UP + DOWN (sync) | low | low | low |

**Why Schottky:** a press pulls GPIO21 down to the diode's forward drop. The S3 reads
below about 0.8 V as low. A 1N4148 drops 0.6–0.7 V, which is too close. A Schottky drops
about 0.3 V.

Avoid **GPIO45** (a strapping pin) and **GPIO10** (LilyGo: analog input only). **GPIO0** is
BOOT and also the panel's config strobe, so don't add anything to it.

**Soldering:**
1. Cut three 120 mm lengths of 30 AWG, one colour per key, plus a shared GND.
2. Solder the diodes at the board end, at the GPIO21 pad, cathode toward the key.
   Heat-shrink each diode, then tape it down with Kapton.
3. Solder each B3F-1000 on flying leads. Use two legs on the same side, after checking with
   the meter that they open and close on a press: the other pairs are always connected.
   Heat-shrink the joints.
4. Before you power anything, measure with the meter: each key shorts its line to GND, and
   no line is shorted to 3V3 or to another line.

## 5. Key test and wake test

```cpp
// SR-3 key + wake test. LilyGo T5 4.7 S3, esp32 core 2.0.x.
#include <Arduino.h>
#include "driver/rtc_io.h"

const int K_ANY = 21, K_UP = 39, K_DOWN = 48;

const char *which() {
  bool any = !digitalRead(K_ANY), up = !digitalRead(K_UP), dn = !digitalRead(K_DOWN);
  if (up && dn) return "UP+DOWN (sync)";
  if (up) return "UP";
  if (dn) return "DOWN";
  if (any) return "OK";
  return "none";
}

void setup() {
  Serial.begin(115200);
  delay(1500);                              // let USB CDC come up
  pinMode(K_ANY, INPUT_PULLUP);
  pinMode(K_UP, INPUT_PULLUP);
  pinMode(K_DOWN, INPUT_PULLUP);
  Serial.printf("woke: cause %d, key %s\n", esp_sleep_get_wakeup_cause(), which());
}

void loop() {
  static uint32_t last = millis();
  const char *k = which();
  if (strcmp(k, "none")) { Serial.println(k); last = millis(); delay(200); }
  if (millis() - last > 10000) {            // 10 s idle: deep sleep, any key wakes
    Serial.println("sleeping");
    Serial.flush();
    rtc_gpio_pullup_en((gpio_num_t)K_ANY);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)K_ANY, 0);
    esp_deep_sleep_start();
  }
}
```

**Check:**
- Each key prints its own name, and UP + DOWN together prints `UP+DOWN (sync)`.
- After 10 s idle it prints `sleeping`. Each of the three keys then wakes it with
  `cause 2` (ext0), naming the key you pressed if you're still holding it.

The serial port drops while the chip sleeps. Reopen the monitor after each wake.

## 6. Battery and power switch

1. Wire the slide switch into the cell's **red** lead only: cell + → switch common pin →
   switch outer pin → board +. Leave black continuous.
2. Heat-shrink both joints. Put Kapton on the switch body where it touches the board.
3. Switch on with USB unplugged. The board should boot and run the demo from the cell.
4. Plug in USB. Charging should start: watch the charge LED, or the battery ADC on GPIO14
   rising over a few minutes.

**Check:** with the switch off, the cell reads the same voltage after an hour, so nothing
draws through the switch.

## 7. Grey images

The phone will render pages; for now, make test pages by hand:

1. Take a zine page or photo, scale it to **540 × 960** (portrait), and convert it to
   4-bit grey with ordered dither for photos and plain threshold plus gamma for text
   (DESIGN §5).
2. Convert it to a C header with the image converter in the LilyGo-EPD47 repo (the
   drawImages example uses its output; check the script's name and options in the repo),
   then show it with **drawImages**.

**Check:** text at 9–10 pt is readable at arm's length, and photos show smooth greys with
no banding. Note how long a full grey refresh takes; DESIGN §10 assumes about 1–1.5 s.

## 8. Measure sleep current

This is the most important measurement on the pass. DESIGN §10 explains why.

1. Flash the §5 test. Unplug USB. Put the µA meter in the battery + lead (after the
   switch).
2. Let it fall asleep, wait 10 s, and read the current.

| Sleep current | Life at 300 pages/day, 1200 mAh |
|---|---|
| 40 µA (what the battery model assumes) | ~4 months |
| 380 µA (LilyGo's own figure for this board) | ~2 months |

If you read about 380 µA, try removing the microSD card (an idle card draws 100–250 µA)
and powering the panel off (`epd_poweroff()`) before sleep. Record what each change does,
and add the result to `../power/README.md`.

## 9. Enclosure

**Stop-gap until the shell is drawn:** a rigid vertical badge holder made for a 4 × 6 in
card, with the board and cell taped to a 1.5 mm card stiffener, the keys poking through a
side slot, and the lanyard on the holder's own slot. It's crude but wearable, and fine for
testing reading on a lanyard.

**What the printed shell must meet** (from DESIGN "Enclosure" and the SR-3 render):

| | |
|---|---|
| Outside | 78 × 149 × 14.5 mm, two parts (front bezel + back), PETG, 1.6 mm walls |
| Window | active area 58.3 × 103.7 mm (portrait), glass recessed 1.3 mm behind the bezel |
| Band | laminate band zone above the screen for the vinyl sticker |
| Keys | UP / OK / DOWN on the right edge, TPU caps, B3F bodies held in printed pockets |
| Ports | USB-C bottom edge; microSD left edge near the bottom; slide switch with a recessed slot so it can't be knocked on |
| Fixing | 4 × M2 heat-set inserts in the back, M2×6 screws from the front |
| Lanyard | 18 mm slot at the top, 25 mm split ring |
| Cell | pocket behind the board, foam-taped, never pressed by a screw boss |

If §3 showed the board doesn't fit inside 74 × 128 × 9 mm, the shell grows to fit. Write
the measured size here when you have it.

Print settings to start from: PETG 0.2 mm layers, 4 perimeters, 25 % gyroid; TPU caps
at 0.12 mm. Press in the inserts at 230 °C.

## 10. Final assembly

1. Press the heat-set inserts into the back shell.
2. Lay the cell in its pocket on foam tape, leads toward the switch.
3. Fit the switch and the three keys into their pockets, then route the wires along the
   wall with Kapton.
4. Seat the board panel-down in the front bezel. Never press on the glass. Hold the board
   by its edges and protect the panel's flex cable: it's the part that breaks.
5. Plug in the cell with the switch **off**. Close the shell and fit the screws finger-tight,
   then a quarter turn more. PETG cracks if you overtighten.
6. Apply the band sticker and fit the split ring and lanyard.

**Check:**
- Switch on: the pass boots, the keys work through the caps, and the panel isn't bowed.
- USB-C and the microSD card go in without opening the shell.
- After a night asleep, the pass wakes on a key press.

## 11. Troubleshooting

| Symptom | Likely cause |
|---|---|
| No serial port | Charge-only USB cable, or USB CDC On Boot is off. Hold BOOT, tap RST. |
| Build fails in LilyGo-EPD47 | esp32 core 3.x installed. Go back to 2.0.17. |
| Panel shows stripes or nothing | Panel flex cable not seated, or the panel was never powered on (`epd_poweron()`). |
| A key wakes the pass but reads as OK | That key's diode is backwards, or its GPIO wire is off. |
| UP or DOWN won't wake the pass | Diode open or reversed. Measure it with the meter on diode test. |
| Wakes by itself | GPIO21 wire is floating or long: shorten it and keep the pull-up enabled. |
| Board resets when the panel refreshes | Weak cell or high resistance in the switch. Measure the cell under load. |
| Battery drains in days | Sleep current is high (§8): microSD card, panel left powered, or the code isn't really sleeping. |

## 12. What's still open

- Draw the SR-3 shell from the §3 measurements (this needs the board in hand).
- Firmware: port the reader core (DESIGN §8) onto epdiy for this board, with the key map
  from §4 and the USB drive mode on the S3 (DESIGN §7.2).
- Make the LilyGo board SR-3's primary base in `../bom/make_bom.py`, and add the two Schottky
  diodes. The BOM still lists the M5 kit.
- Whether this board can sleep below 380 µA.

---

## Appendix A: M5Stack PaperDIY (K159) instead

If the kit comes back in stock it's cheaper ($25), and the shell was sized for it
(115.3 × 62.5 × 5.5 mm). The differences from the LilyGo build (from
<https://docs.m5stack.com/en/products/sku/K159>):

| | K159 |
|---|---|
| Controller | ESP32-S3-WROOM-1-N16R8, ET047TC1 4.7in 960×540 16-grey |
| Keys | Two user keys are on board, on **G3 and G4** (check which is KEY1 and KEY2). Wire UP and DOWN in parallel with them and add OK on a third pin. G3 and G4 are RTC GPIOs, so they can wake the chip. The power button is handled by the M5PM1 power chip, not a GPIO. |
| Free pins | Only on two 0.5 mm 12-pin FPC connectors. FPC-SPI's SCK, MOSI and MISO (G39, G38, G40) are **shared with the microSD card**. Choose OK's pin from the FPC-I2S connector (for example G5) and check it on the schematic: <https://m5stack-doc.oss-cn-shenzhen.aliyuncs.com/1288/K159-B_PaperDIY_sche.pdf>. Soldering to an FPC needs a breakout. |
| Battery | 140 mAh cell on an **SH1.0-2P** connector. The 1200 mAh JST-PH cell needs an SH1.0-2P pigtail (check polarity). Charging is set for the small cell, so a full charge of the big one will be slow. |
| Power | M5PM1: click = on/reset, double-click = off. Hold the power button until the red LED blinks to enter download mode. |
| Panel pins | Different from the LilyGo board (DB0 = G6 and so on). Don't reuse LilyGo code without changing the pin map. |
| Sleep | The kit's sleep current isn't published. Measure it as in §8. |
