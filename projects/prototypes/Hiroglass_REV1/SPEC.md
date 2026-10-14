# Overview

Our goal is to design an open hardware design for making XR wearables.  We need to differentiate between HUD and Spatial XR.  The early version will be a HUD, with future versions running binocular spatial XR.  We may also experiment with passthru HUD glass displays and holographic type designs as side research.  The goal is to get things that can be connected to existing glasses, goggles, and helmets, calibrated, and used comfortably.  We will provide a companion app. 

## Hiroglass REV1 CORE Requirements

### Hiroglass REV1 for Glasses

A clip on attachment for glasses or sunglasses.

### Hiroglass REV1 for Goggles

Ideal for construction goggles, ski goggles, dirt bike goggles, etc.

### Hiroglass REV1 for Helmets/Hats

Ideal for an attachment to a bike helmet or ski helmet. 

## Design Goals

- We want to build something that manufacturable with components that can be sourced from the usual sources, Mouser, Digikey, etc., and seek to avoid too many custom pieces of hardware.  
- It is very likely we will need to experiment with throw away designs, possibly use FPGAs, and maybe get to developing custom ASICs down the road.  
- the goal is sub $300 devices that can interoperate with a mobile app, or use something clever like a Cardputer ADV or other cool compact MCU. 
- Puck for power and compute is ok in this round of design
- no particular optics technology is settled, however, we want to avoid costly components, so early designs may be lagging existing products on the market
- avoid components with single suppliers where possible
- We will own the entire software stack, nothing proprietary, all open where feasible
- Utilize our existing work on ProjectMagnet where possible https://github.com/IoTone/ProjectMagNET and https://projectmagnet-github-docs.pages.dev/docs/
