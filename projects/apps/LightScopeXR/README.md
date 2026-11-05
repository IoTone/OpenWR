# LightScopeXR

*Open. Luminous. Control.*

**LightScopeXR** is a remote for a Sony α7R III on Snap Spectacles ('24): shutter, AF,
record, f-stop, shutter speed, ISO and EV, with a low-res look through the
viewfinder now and then. The lens talks Bluetooth to
**StickCPlus_AlphaFriend_OpenWR** (an M5StickC Plus SE), which joins the
camera's Wi-Fi and drives it.

Started as a copy of [`../SpecsSwitchbackXR/`](../SpecsSwitchbackXR/); its
Bluetooth links are in `Assets/ProjectScripts/ble/`.

- [`DESIGN.md`](DESIGN.md): roles, constraints, options and phasing

## Sound credits

The shutter sounds in `Assets/Sounds/` are from Freesound, converted to mono:

- `shutter_a7.wav`: "A700_shutter_single.WAV" by fastson,
  <https://freesound.org/people/fastson/sounds/99690/>, licensed
  [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/).
- `shutter_scout.wav`: "Camera Shutter Click" by Kodack,
  <https://freesound.org/people/Kodack/sounds/271010/>, CC0.
- `shutter_glasses.wav`: "Contarex camera shutter.wav" by Tonik1105,
  <https://freesound.org/people/Tonik1105/sounds/520684/>, CC0.
