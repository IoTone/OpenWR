# SwitchbackXR

**SwitchbackXR** is the Switchback trail app on Snap Spectacles ('24). The
lens talks Bluetooth to a heart-rate strap, the **goggl1** ski goggle and the
**maplet** e-paper camera, with no bonding and no internet. Built in three
days for the Specs PDX Dev Meetup.

![SwitchbackXR icon](icon.png)

## Demos

- [SwitchbackXR demo 1](https://youtu.be/LUcOxbvzm3s)
- [SwitchbackXR demo 2](https://youtu.be/I-6D7DTMtBo)

## Slides

- [`docs/SwitchbackXR.pdf`](docs/SwitchbackXR.pdf): the meetup deck as a PDF
- [`docs/deck/`](docs/deck/): its source (see the README there)

## In the lens

| View | Does |
|---|---|
| **Pulse** | heart rate from the strap, on the glasses and the goggle |
| **Remote** | goggl1's screens as cards; a pinch picks what the goggle shows |
| **Roll** | browses the maplet's roll; **SEND** puts the selected shot on its e-paper |
| **Pin board** | pin flags around you; distant labels rise so they don't overlap |
| **Viewfinder** | **CAMERA** on the Roll opens it; **SHOOT** holds the shot 5 s, then it joins the Roll |

Scripts are in `Assets/ProjectScripts/` (`ble`, `pulse`, `remote`, `roll`,
`pins`, `snap`, `ui`). `ble/Links.ts` holds the three Bluetooth links (strap,
goggl1, maplet) that every view shares.

## More

- [`DESIGN.md`](DESIGN.md): roles of each device and the constraints
- [`BLE-NOTES.md`](BLE-NOTES.md): Bluetooth without bonding, tested on the glasses
- [`../switchback/`](../switchback/): the Android app
