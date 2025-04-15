# Overview

Headlight allows you to illuminate a dark space while using a VR HMD.  Most VR headsets don't work well in low light, simply, hand tracking stops working and the device may shut down or become jittery.

This low cost accessory adds a way to easily illuminate by connecting from within your VR HMD or through an external phone app.

![Image](https://github.com/user-attachments/assets/46f90650-a5bf-43ab-97a6-d09036cfe079)
![Image](https://github.com/user-attachments/assets/03c7a16b-f6c7-40c2-8667-241dc1d23dc1)
![Image](https://github.com/user-attachments/assets/52f30634-f6bb-447a-badb-a4c612918ebd)

## BOM

- https://shop.m5stack.com/products/atom-lite-esp32-development-kit
- https://shop.m5stack.com/products/hex-rgb-led-board-sk6812

## References

- UNIT wiki: https://docs.m5stack.com/en/unit/hex
- FastLED: https://racheldebarros.com/arduino-projects/how-to-use-fastled-with-arduino-to-program-led-strips/

## Software

Board setup on Arduino:

- ESP32
- M5Stack
- FastLED


Required libraries:

- M5 Unified
- NimBLE
- 

### Arduino Side

- Install code and attach to power your M5Stack ATOM.  
- configure the HEX via a grove connector included in the Unit Hex.
- power should be sufficient (3.7-5V) to your M5 ATOM

### Vision Pro

Within your BLE settings, just search for HeadLight-OpenWR.


### iPhone/Android

No app is needed.  You can use nrfConnect app or any BLE capable scanner to connect / disconnect.  Search for HeadLight-OpenWR.

## Known Issues

- No security
- No control over colors
- No pattern control
- How about 2 or 3 HEX Units together?
- TODO: The VisionPro app for this