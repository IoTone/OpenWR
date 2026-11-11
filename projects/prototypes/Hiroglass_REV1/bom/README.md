# Hiroglass REV1 BOMs

Orderable bills of materials for the Hiroglass REV1 build options. `make_bom.py` generates every file here; edit it, not the CSVs.

**Is it buildable?** See [`BUILDABILITY.md`](BUILDABILITY.md): where each line can be bought in the US and in Japan, lead times, lines to watch, and the cost of a run of 1 to 5 units. `sourcing.py` generates it.

**Before ordering:** prices and stock were re-checked on 2026-11-10. Lines whose `status` is `checked` were read on the seller's page; `snapshot` came from a search result; `estimate` has no live figure. Import the `*.digikey-import.csv` or `*.mouser-import.csv` file into the distributor's BOM tool to see today's price and stock. The totals below are US prices.

## Options

| Option | Builds | Order total | Per build | Files |
|---|---|---|---|---|
| Bench kit A: Beetle ESP32-C3 (reuse goggl1) | 3 | $291 | $97 | [`bench-a-beetle.csv`](bench-a-beetle.csv) |
| Bench kit B: XIAO ESP32C3, all STEMMA QT | 3 | $285 | $95 | [`bench-b-qtpy.csv`](bench-b-qtpy.csv) |
| Pod-0 custom PCB set (pod + puck) | 3 | $249 | $83 | [`pod0-pcb.csv`](pod0-pcb.csv) |
| Optics and mechanical for Pod-0 (Edmund tier) | 3 | $875 | $292 | [`optics-mech.csv`](optics-mech.csv) |
| Concept 4: micro-OLED driver experiment (one team set) | 1 | $768 | $768 | [`concept4-microoled.csv`](concept4-microoled.csv) |
| Concept 5: Linux puck + Monado (one team set) | 1 | $585 | $585 | [`concept5-linux-puck.csv`](concept5-linux-puck.csv) |

**Three of everything:** each option builds three units, two dev units (one at each site) and one demo unit. Bench tools (lux meter, ND filter, header pack, screws, inserts) are one per site; the PCB fab order is one for the team. Shared items are split across builds in the per-build column.

The Pod-0 PCB total includes two spares of each chip for hand assembly and cut-tape minimums on passives.

## Suggested combinations

- **Recommended first order:** a bench kit for each build plus optics-mech. Every line is in stock in the US today. In Japan use kit B: the XIAO ESP32C3 is the board with a Japan radio certificate and deep stock (Akizuki).
- **Pod-0 PCB set:** buildable, but three parts were swapped for in-stock substitutes that still need a datasheet check, and the OLED panel has one seller. Order the panels from Orient Display first.
- **Experimental branch:** concept 4 and concept 5 start as one exploration set each. If a concept is picked for CES at the go/no-go review (Nov 20), order two more sets so it also has three units.

## Bench kit A: Beetle ESP32-C3 (reuse goggl1)

Keeps goggl1's SPI display pins and Forth vocabulary. Wiring: schematics sheet B-A.

Files: [`bench-a-beetle.csv`](bench-a-beetle.csv) · [`bench-a-beetle.digikey-import.csv`](bench-a-beetle.digikey-import.csv) · [`bench-a-beetle.mouser-import.csv`](bench-a-beetle.mouser-import.csv)

## Bench kit B: XIAO ESP32C3, all STEMMA QT

Everything on one I2C chain. The XIAO replaces the QT Py ESP32-C3, which is out of stock (18-week lead) and has no Japan radio certification. Wiring: schematics sheet B-B, with the XIAO's pins.

Files: [`bench-b-qtpy.csv`](bench-b-qtpy.csv) · [`bench-b-qtpy.digikey-import.csv`](bench-b-qtpy.digikey-import.csv) · [`bench-b-qtpy.mouser-import.csv`](bench-b-qtpy.mouser-import.csv)

## Pod-0 custom PCB set (pod + puck)

Chip-level parts for schematics sheets P-1 to P-3. Hand or reflow assembly; ICs and passives get spares in the order quantities.

- Three parts changed on 2026-11-10 because the originals have no stock: Q1 FDN338P to DMG2305UX-7, Q2 FDN335N to FDN337N, D10/D11 PMEG2010AEH to MBR120VLSFT1G. The schematics still show the old part numbers; check each substitute against its datasheet before layout.
- Buy the OLED panel from Orient Display's own store (169 in stock, $4.53); DigiKey has none for 8 weeks. Do not substitute a generic 0.96" panel: those have a 0.7 mm solder-down tail that does not fit J1.
- U1 is now the ESP32-C3-MINI-1-N4X. The N4 is marked not for new designs.

Files: [`pod0-pcb.csv`](pod0-pcb.csv) · [`pod0-pcb.digikey-import.csv`](pod0-pcb.digikey-import.csv) · [`pod0-pcb.mouser-import.csv`](pod0-pcb.mouser-import.csv)

## Optics and mechanical for Pod-0 (Edmund tier)

DigiKey and Mouser do not stock usable lenses or beamsplitters, so optics come from Edmund. Budget tier: AliExpress PMMA aspheres and 30/70 plates at about $5–15 a set.

- For a 30R/70T combiner use Edmund #43-734 (1 mm float glass, same $61). #34-412 is a 3 mm fused-silica precision plate at $222.
- AliExpress tier (not a distributor order) is about $250 for the team including magnets, hardware and the lux meter.

Files: [`optics-mech.csv`](optics-mech.csv) · [`optics-mech.digikey-import.csv`](optics-mech.digikey-import.csv) · [`optics-mech.mouser-import.csv`](optics-mech.mouser-import.csv)

## Concept 4: micro-OLED driver experiment (one team set)

One bench set for the experimental branch. The panel is not sold by DigiKey or Mouser. Wiring: schematics sheet X-4.

- A bare ECX336 also needs a 10 V rail and a cathode supply, which this BOM does not have.
- The public ECX336BF datasheet leaves about 35 timing registers as 'separately submitted', so the init sequence is not fully open.

Files: [`concept4-microoled.csv`](concept4-microoled.csv) · [`concept4-microoled.digikey-import.csv`](concept4-microoled.digikey-import.csv) · [`concept4-microoled.mouser-import.csv`](concept4-microoled.mouser-import.csv)

## Concept 5: Linux puck + Monado (one team set)

One set, glasses included. Wiring: schematics sheet X-5.

- The ROCK 5B and 5B+ are out of stock everywhere checked, so the Pi 5 is the only board.

Files: [`concept5-linux-puck.csv`](concept5-linux-puck.csv) · [`concept5-linux-puck.digikey-import.csv`](concept5-linux-puck.digikey-import.csv) · [`concept5-linux-puck.mouser-import.csv`](concept5-linux-puck.mouser-import.csv)

## Sources

Research notes behind every line: [`research_notes/BOM and schematics/`](../research_notes/BOM%20and%20schematics/).
Schematics and wiring for each option: [`schematics/`](../schematics/README.md).
