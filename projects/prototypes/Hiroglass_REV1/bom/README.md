# Hiroglass REV1 BOMs

Orderable bills of materials for the Hiroglass REV1 build options. `make_bom.py` generates every file here; edit it, not the CSVs.

**Before ordering:** prices and stock are snapshots from 2026-10-13/14. DigiKey and Mouser blocked automated page fetches, so nothing was confirmed live. Import the `*.digikey-import.csv` or `*.mouser-import.csv` file into the distributor's BOM tool, and the cart will show today's price and stock. Lines whose `status` is `estimate` have no live figure at all.

## Options

| Option | Builds | Order total | Per build | Files |
|---|---|---|---|---|
| Bench kit A: Beetle ESP32-C3 (reuse goggl1) | 3 | $291 | $97 | [`bench-a-beetle.csv`](bench-a-beetle.csv) |
| Bench kit B: QT Py ESP32-C3, all STEMMA QT | 3 | $303 | $101 | [`bench-b-qtpy.csv`](bench-b-qtpy.csv) |
| Pod-0 custom PCB set (pod + puck) | 3 | $284 | $95 | [`pod0-pcb.csv`](pod0-pcb.csv) |
| Optics and mechanical for Pod-0 (Edmund tier) | 3 | $800 | $267 | [`optics-mech.csv`](optics-mech.csv) |
| Concept 4: micro-OLED driver experiment (one team set) | 1 | $592 | $592 | [`concept4-microoled.csv`](concept4-microoled.csv) |
| Concept 5: Linux puck + Monado (one team set) | 1 | $364 | $364 | [`concept5-linux-puck.csv`](concept5-linux-puck.csv) |

**Three of everything:** each option builds three units, two dev units (one at each site) and one demo unit. Bench tools (lux meter, ND filter, header pack, screws, inserts) are one per site; the PCB fab order is one for the team. Shared items are split across builds in the per-build column.

The Pod-0 PCB total includes two spares of each chip for hand assembly and cut-tape minimums on passives.

## Suggested combinations

- **Start this week, cheapest:** bench kit A (reuses goggl1 boards) plus the AliExpress optics tier. The bench kits are complete for electronics; optics come from optics-mech.
- **Recommended first order:** bench kit A for all three builds, plus optics-mech (Edmund tier). This builds Pod-0 cartridges A and B on the bench and runs the brightness test.
- **Once Pod-0 works on the bench:** the Pod-0 PCB set. Order the OLED panels first; they are the long-lead item.
- **Experimental branch:** concept 4 and concept 5 start as one exploration set each. If a concept is picked for CES at the go/no-go review (Nov 20), order two more sets so it also has three units.

## Bench kit A: Beetle ESP32-C3 (reuse goggl1)

Keeps goggl1's SPI display pins and Forth vocabulary. Wiring: schematics sheet B-A.

Files: [`bench-a-beetle.csv`](bench-a-beetle.csv) · [`bench-a-beetle.digikey-import.csv`](bench-a-beetle.digikey-import.csv) · [`bench-a-beetle.mouser-import.csv`](bench-a-beetle.mouser-import.csv)

## Bench kit B: QT Py ESP32-C3, all STEMMA QT

Everything on one I2C chain, no soldering on the bench. Wiring: schematics sheet B-B.

Files: [`bench-b-qtpy.csv`](bench-b-qtpy.csv) · [`bench-b-qtpy.digikey-import.csv`](bench-b-qtpy.digikey-import.csv) · [`bench-b-qtpy.mouser-import.csv`](bench-b-qtpy.mouser-import.csv)

## Pod-0 custom PCB set (pod + puck)

Chip-level parts for schematics sheets P-1 to P-3. Hand or reflow assembly; ICs and passives get spares in the order quantities.

- The OLED panel is the supply risk: Orient showed 0 stock with an 8-week lead and Winstar had 4 pieces. Order panels first.
- J2 (full 24-pin USB-C) showed a 20-week lead. The GCT USB4105-GF-A fits a different footprint and drops the DisplayPort lanes; decide before layout.

Files: [`pod0-pcb.csv`](pod0-pcb.csv) · [`pod0-pcb.digikey-import.csv`](pod0-pcb.digikey-import.csv) · [`pod0-pcb.mouser-import.csv`](pod0-pcb.mouser-import.csv)

## Optics and mechanical for Pod-0 (Edmund tier)

DigiKey and Mouser do not stock usable lenses or beamsplitters, so optics come from Edmund. Budget tier: AliExpress PMMA aspheres and 30/70 plates at about $5–15 a set.

- Swapping to Edmund #34-412 30R/70T plates adds about $160 per kit.
- AliExpress tier (not a distributor order) is about $250 for the team including magnets, hardware and the lux meter.

Files: [`optics-mech.csv`](optics-mech.csv) · [`optics-mech.digikey-import.csv`](optics-mech.digikey-import.csv) · [`optics-mech.mouser-import.csv`](optics-mech.mouser-import.csv)

## Concept 4: micro-OLED driver experiment (one team set)

One bench set for the experimental branch. The panel is not sold by DigiKey or Mouser. Wiring: schematics sheet X-4.

Files: [`concept4-microoled.csv`](concept4-microoled.csv) · [`concept4-microoled.digikey-import.csv`](concept4-microoled.digikey-import.csv) · [`concept4-microoled.mouser-import.csv`](concept4-microoled.mouser-import.csv)

## Concept 5: Linux puck + Monado (one team set)

One set. Glasses are borrowed or bought separately. Wiring: schematics sheet X-5.

Files: [`concept5-linux-puck.csv`](concept5-linux-puck.csv) · [`concept5-linux-puck.digikey-import.csv`](concept5-linux-puck.digikey-import.csv) · [`concept5-linux-puck.mouser-import.csv`](concept5-linux-puck.mouser-import.csv)

## Sources

Research notes behind every line: [`research_notes/BOM and schematics/`](../research_notes/BOM%20and%20schematics/).
Schematics and wiring for each option: [`schematics/`](../schematics/README.md).
