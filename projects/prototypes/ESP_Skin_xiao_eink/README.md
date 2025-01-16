# Overview

Designer "skin" for your devices.  ESP Skin XIAO Eink proto for Arduino is a technology concept designed to pair an epaper display with an ESP32 MCU, and allow the display to be toggled via a remote BLE connection, presumably from an App. 

## BOM

- XIAO ESP32C3
- XIAO Epaper Breakout Board: https://www.seeedstudio.com/ePaper-Breakout-Board-p-5804.html
- 2.13 Monochrome Flexible EPaper for this demo https://www.seeedstudio.com/2-13-Flexible-Monochrome-ePaper-Display-with-212x104-Pixels-p-5781.html
- Other supported (with changes to this code : https://www.seeedstudio.com/catalogsearch/result/?q=epaper

## Assembly

See the wiki: https://wiki.seeedstudio.com/XIAO-eInk-Expansion-Board/

Generally speaking, E-ink / E-paper solutions use screens and controllers made by a few companies (Waveshare, etc.).   They are notoriously difficult to actually get working due to knock off parts, cheap components, falky drivers.

This combination actually works.

## Software

Use Arduiono IDE.  Install the ESP32 line of boards. See the wiki above for details.  Currently no external libraries needed yet.  It is expected to add a NimBLE to the code base to add BLE functionality.

### Installation

## Artwork for EPaper

TODO: add tools for generating the designs.