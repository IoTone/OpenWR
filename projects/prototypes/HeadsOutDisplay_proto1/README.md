# Overview

HeadsOut Display is an RGB indicator of your VR status, much like a status in IM messenger, it indicates when you are do not want to be disturbed, when you are free, or possibly occupied with a work call.  It gives people in the external default world a way to know if you should be disturbed while in VR.

Customization of our computing experience and persionalization is and has always been important to gamers.  Companies like RAZER, Corsair, and Steelseries have been proving this for years, that gamers and PC users want to personalize.  An HMD is an accessory as much as it is a compute device.  Like a phone, there needs to be more ways to augment its functionality and also provide a way to show off your own personality.

## BOM

This uses an RGB LED matrix coupled with a BLE controller.  The entire BOM cost is less than $30.  Currently it is powered by USB-C (3.3V) but could be run from a LiP0 battery.

- SEEED Xiao BLE Sense 52840
- Adafruit Neo 5x5 (TODO: add link)

## References

- https://learn.adafruit.com/neopixel-novelty-cufflinks-with-scrolling-text/wiring-diagram
- https://learn.adafruit.com/adafruit-5x5-neopixel-grid-bff/arduino

## Software

Need some notes here ... need to use the nrf52 MBed board, not the non-mbed version.  Won't compile the ArduinoBLE libraries.  

The Xiao BLE Sense can be swapped out for the ESP32-S3 or ESP32-C3.  However this needs to be verified with testing. 

### Arduino Side

TODO

### Vision Pro

TODO

### iPhone/Android

No app is needed.  You can use nrfConnect app or any BLE capable scanner to toggle values in the HeadsOut device.