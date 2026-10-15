#!/usr/bin/env python3
"""Hiroglass REV1 roadmap to CES 2027.

One source for the plan: writes PLAN.md, and with --github creates or updates
the labels, milestones, issues and the GitHub Project (Kanban board) to match.

    python3 roadmap.py            # write PLAN.md only
    python3 roadmap.py --github   # also sync GitHub (needs: gh auth refresh -s project)

Re-running is safe: issues are matched by title, milestones by title, the
project by title.
"""
import json
import subprocess
import sys
from datetime import date
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = "IoTone/OpenWR"
ORG = "IoTone"
PROJECT_TITLE = "Hiroglass REV1 to CES 2027"
TRACKING_ISSUE = 18
TREE = "https://github.com/IoTone/OpenWR/blob/dev/projects/prototypes/Hiroglass_REV1"

START = date(2026, 10, 16)  # Friday kickoff
CES = (date(2027, 1, 6), date(2027, 1, 9))

MILESTONES = [
    # key, title, due, what "done" means
    ("M0", "M0 Kickoff and orders", "2026-10-23",
     "Every part for the bench kits, optics and Pod-0 is ordered; the schematic verify list is closed."),
    ("M1", "M1 Bench HUDs lit", "2026-11-06",
     "Two bench rigs show the HUD through the plate-combiner optic on a pair of glasses."),
    ("M2", "M2 Pod-0 rev A to fab", "2026-11-13",
     "Pod and puck boards are reviewed and ordered with a stencil."),
    ("M3", "M3 Demo bets chosen", "2026-11-20",
     "Go/no-go review picks the CES demo set; concepts 2-5 each have a result or a stop."),
    ("M4", "M4 Pod-0 rev A running", "2026-12-11",
     "Rev A boards run the HUD firmware on battery, in a printed enclosure, with the app."),
    ("M5", "M5 Demo freeze", "2026-12-18",
     "Demo kits, spares, videos and the open-hardware release are done. Only fixes after this."),
    ("M6", "M6 CES 2027", "2027-01-05",
     "Dry run, pack and travel. CES runs January 6-9, 2027."),
]

TRACKS = {
    "pod0": ("Pod-0 electronics", "1d76db"),
    "c1": ("C1 plate combiner", "0e8a16"),
    "c2": ("C2 look-around eyepiece", "5319e7"),
    "c3": ("C3 behind-visor reflector", "fbca04"),
    "c4": ("C4 micro-OLED FPGA", "b60205"),
    "c5": ("C5 Linux puck + Monado", "d93f0b"),
    "fw": ("Firmware and app", "006b75"),
    "ces": ("Program and CES", "c5def5"),
}

CLASSES = {
    "expedite": ("Expedite", "b60205", "Long lead or blocks everyone. Jumps the WIP limit."),
    "fixed": ("Fixed date", "d93f0b", "Has a hard date; pull it before standard work."),
    "standard": ("Standard", "0e8a16", "Normal flow, first in first out."),
    "intangible": ("Intangible", "cccccc", "Docs, cleanup, release hygiene. Pull when a lane is idle."),
}

OWNERS = {"S": "Senior", "J": "Junior", "P": "Pair"}
PEOPLE = {"S": ["truedat101"], "J": ["lk911"], "P": ["truedat101", "lk911"]}

# id, title, owner, track, milestone, start, target, size, class, depends-on ids,
# body (what and why), acceptance criteria
TASKS = [
    # ---- M0: kickoff and orders -------------------------------------------
    (1, "Order Pod-0 long-lead parts and settle the USB-C fallback", "S", "pod0", "M0",
     "2026-10-16", "2026-10-20", "S", "expedite", [],
     "The full 24-pin Amphenol 12401610E4#2A USB-C receptacle quotes a 20-week lead, and the Orient "
     "AOM12864A0-0.96WW panel showed 0 stock with an 8-week lead. Neither can wait for layout.",
     ["Pod-0 uses the GCT USB4105-GF-A (no DisplayPort lanes) unless the Amphenol part is in stock this week; the choice is written in schematics/README.md",
      "OLED panels ordered: Winstar alternate (in stock) plus Orient if any arrive by Dec 1",
      "Pod-0 PCB BOM ordered for 3 builds from the bom/pod0-pcb.digikey-import.csv or mouser-import cart",
      "Order numbers and ETAs posted as a comment here"]),
    (2, "Order bench kits and the optics-mech set", "J", "ces", "M0",
     "2026-10-16", "2026-10-20", "S", "expedite", [],
     "Bench rigs are how both engineers see a HUD in week 2. Optics come from Edmund, which DigiKey and Mouser do not stock.",
     ["Bench kit A (Beetle) for 2 builds and bench kit B (QT Py) for 1 build ordered from the import CSVs in bom/",
      "Edmund #63-480 lenses, #43-736 50/50 and #34-412 30/70 plates ordered per bom/optics-mech.csv",
      "Live cart prices recorded back into bom/make_bom.py and the CSVs regenerated",
      "ETAs posted as a comment here"]),
    (3, "Order the concept 5 Linux puck set", "J", "c5", "M0",
     "2026-10-16", "2026-10-21", "S", "standard", [],
     "The Pi 5's USB-C port is power-only, so USB-C glasses need the ROCK 5B (DisplayPort alt mode) or an HDMI adapter.",
     ["bom/concept5-linux-puck.csv set ordered, ROCK 5B path preferred",
      "Senior signs off on the glasses choice before the order goes in"]),
    (4, "Close the schematic verify list before KiCad capture", "S", "pod0", "M0",
     "2026-10-16", "2026-10-22", "S", "fixed", [],
     "Some pin numbers in the SVG schematics came from secondary sources because datasheet downloads failed.",
     ["USBLC6-2SC6 and MAX17048 pin numbers checked against the datasheets",
      "Winstar OLED pin map checked against the Orient panel and the FPC footprint",
      "Battery connector polarity checked against the cells we bought",
      "Bench-kit silkscreen labels checked against sheets B-A and B-B",
      "schematics/README.md verify list updated; fixes regenerated with make_schematics.py"]),
    (5, "Set up the shared toolchain and repo conventions", "J", "ces", "M0",
     "2026-10-16", "2026-10-21", "S", "standard", [],
     "Both engineers need the same tools so either one can pick up the other's card.",
     ["ESP-IDF, ESPIDFORTH and the goggl1 build flash a Beetle on both machines",
      "KiCad 9, FreeCAD or OpenSCAD, and a slicer installed; versions noted in plan/PLAN.md",
      "A short CONTRIBUTING note for Hiroglass_REV1: folder layout, branch naming, how to regenerate BOMs and schematics"]),

    # ---- Pod-0 electronics -------------------------------------------------
    (6, "Capture Pod-0 schematics in KiCad", "S", "pod0", "M2",
     "2026-10-19", "2026-10-30", "L", "fixed", [4],
     "Turn the three Pod-0 SVG sheets and schematics/netlists/pod0-all.csv into a KiCad project.",
     ["KiCad schematic for the pod board and the puck board, matching the GPIO map in schematics/README.md",
      "ERC clean; the KiCad netlist diffs clean against netlists/pod0-all.csv except for named changes",
      "Footprints assigned for every BOM line, with LCSC or DigiKey numbers in fields",
      "Junior reviews against the datasheets and signs off in the PR"]),
    (7, "Lay out the Pod-0 pod and puck boards", "S", "pod0", "M2",
     "2026-10-29", "2026-11-11", "L", "fixed", [6],
     "Two boards joined by the 10-pin JST SH tether. The pod must fit behind a glasses arm.",
     ["Pod board outline from the concept sheet; antenna keep-out for the ESP32-C3",
      "DRC clean; fab outputs (gerbers, drill, BOM, pick-and-place) generated by a script in the repo",
      "Test pads for UART TX, 3V3, VBAT, SDA/SCL",
      "Junior runs the layout review checklist and files any findings as comments"]),
    (8, "Pod-0 rev A design review and fab order", "P", "pod0", "M2",
     "2026-11-12", "2026-11-13", "S", "fixed", [7],
     "One-hour review by both engineers, then order. Assembly is by hand, so order a stencil.",
     ["Review notes committed next to the KiCad project",
      "Boards ordered (5 of each) with a stencil; ETA posted",
      "Any parts the layout changed are reordered"]),
    (9, "Print the Pod-0 enclosure, puck case and clip mounts", "J", "pod0", "M4",
     "2026-11-16", "2026-12-04", "M", "standard", [7],
     "Uses the board outlines from layout. Mounts reuse whatever clip works best in concepts 1-3.",
     ["Parametric source (FreeCAD or OpenSCAD) and STLs in the repo",
      "Pod fits a glasses arm, a goggle strap and a helmet edge",
      "Puck holds the cell, board and USB-C opening; tether strain relief"]),
    (10, "Assemble Pod-0 rev A and bring up power", "S", "pod0", "M4",
     "2026-11-30", "2026-12-04", "M", "fixed", [8],
     "Power first: BQ24074 charger, AP2112K LDO, the FDN338P/FDN335N OLED VBAT switch.",
     ["Three pods and three pucks assembled",
      "Charger ISET (500 mA) and TS window (0-50 C) measured, not assumed",
      "3V3 rail holds under Wi-Fi/BLE transmit bursts",
      "OLED VBAT follows OLED_VBAT_EN (GPIO10)",
      "Rework list filed as issues for rev B"]),
    (11, "Bring up Pod-0 firmware on rev A", "P", "fw", "M4",
     "2026-12-01", "2026-12-11", "M", "fixed", [10, 13],
     "Move the bench firmware onto the board. Senior leads, junior drives the keyboard.",
     ["SSD1306 over SPI with the charge pump on (8D 14), IREF tuned for brightness",
      "BMI270 on INT1 (GPIO0), VEML7700 and MAX17048 on I2C (SCL 2, SDA 5)",
      "Battery percentage on the HUD; ALRT (GPIO20) handled",
      "BLE HUD pages from the app render on the pod"]),
    (12, "Battery runtime and charge safety test", "J", "pod0", "M4",
     "2026-12-07", "2026-12-11", "S", "standard", [10],
     "We will hand these to strangers at CES. They must not get hot or die in an hour.",
     ["Runtime measured at demo brightness, BLE connected",
      "Charging stops outside 0-50 C (NTC test with a hot plate or freezer pack)",
      "Results table in the repo"]),

    # ---- firmware and app ----------------------------------------------------
    (13, "Bench HUD firmware: goggl1 on the SSD1306 rig", "J", "fw", "M1",
     "2026-10-21", "2026-11-04", "M", "standard", [2, 5],
     "Start from the goggl1 ESPIDFORTH firmware. The same code base must later run on Pod-0, so keep the pin map in one place.",
     ["Bench kit A shows Switchback HUD pages over BLE on the SSD1306",
      "Pin map matches the Pod-0 GPIO map where the bench wiring allows",
      "USB console does not block when no host is attached"]),
    (14, "Auto brightness and head-tilt wake", "J", "fw", "M3",
     "2026-11-05", "2026-11-18", "M", "standard", [13],
     "VEML7700 drives HUD brightness; BMI270 wakes the display on a head tilt and dims it otherwise.",
     ["Readable on the bench in a dark room and next to a window",
      "Wake-on-tilt latency under 300 ms",
      "Power draw with the display dimmed recorded"]),
    (15, "Switchback: Hiroglass device profile", "J", "fw", "M4",
     "2026-11-23", "2026-12-11", "M", "fixed", [13],
     "Builds on the existing Switchback app (Android first). Hiroglass appears as a device beside goggl1.",
     ["Pair, pick HUD pages, set brightness, show the pod's battery level",
      "Works with bench rigs and Pod-0 rev A",
      "Demo mode that cycles pages with no phone input"]),

    # ---- concept 1: plate combiner ----------------------------------------
    (16, "C1: plate-combiner optical bench test", "J", "c1", "M1",
     "2026-10-26", "2026-11-04", "M", "fixed", [2],
     "Compare the 50/50 and 30/70 plates with the #63-480 lens. This is the baseline every other optic is judged against.",
     ["Virtual image distance and field of view measured",
      "Through-the-lens photos indoors and outdoors, with the lux reading",
      "Recommendation for the default cartridge written up in research_notes"]),
    (17, "C1: printed clip-on for glasses and goggles", "J", "c1", "M1",
     "2026-11-02", "2026-11-06", "S", "fixed", [16],
     "First wearable demo. Holds the bench electronics and the chosen combiner.",
     ["Clips onto two different pairs of glasses and one goggle",
      "Image stays aligned after taking it off and on five times",
      "STL and source in the repo"]),

    # ---- concept 2: look-around eyepiece -----------------------------------
    (18, "C2: look-around eyepiece prototype", "J", "c2", "M3",
     "2026-11-09", "2026-11-18", "M", "standard", [16],
     "The display sits below the line of sight; the wearer glances down. Brighter than a combiner and easier to build.",
     ["Wearable prototype on glasses",
      "Same photo and lux protocol as C1, so the results compare",
      "Go/no-go note for the M3 review"]),

    # ---- concept 3: behind-the-visor reflector ------------------------------
    (19, "C3: behind-the-visor reflector on a helmet", "J", "c3", "M3",
     "2026-11-16", "2026-11-20", "M", "standard", [16],
     "Uses a ski or bike helmet visor as the combiner. Senior reviews the geometry before printing.",
     ["Mount on one helmet with a visor",
      "Readable image at arm's-length focus",
      "Go/no-go note for the M3 review"]),

    # ---- concept 4: micro-OLED on FPGA --------------------------------------
    (20, "C4 gate: can we buy a micro-OLED and its datasheet?", "S", "c4", "M3",
     "2026-10-19", "2026-11-06", "S", "standard", [],
     "The ECX336 pinout is not public. This card is emails and quotes, a few hours spread over three weeks.",
     ["Sony ECX336 or an alternate micro-OLED quoted with a datasheet we may publish",
      "If the datasheet is NDA-only, record that; it decides whether C4 can be open hardware",
      "Order bom/concept4-microoled.csv only if the answer is yes"]),
    (21, "C4 spike: FPGA test pattern on the micro-OLED", "S", "c4", "M3",
     "2026-11-16", "2026-11-20", "M", "standard", [20],
     "Timebox: one week. Open toolchain (yosys/nextpnr) only. If C4 gate said no, skip this and show C4 as the board plus the concept rendering.",
     ["Test pattern on the panel from an iCE40 or ECP5 board, or a written stop reason",
      "Verilog and build script in the repo"]),

    # ---- concept 5: Linux puck + Monado -------------------------------------
    (22, "C5: Monado OpenXR scene on USB-C glasses", "S", "c5", "M3",
     "2026-11-16", "2026-11-25", "M", "standard", [3],
     "Proves the spatial-XR path on parts anyone can buy. Fab wait on Pod-0 makes room for this.",
     ["ROCK 5B boots a Linux image with Monado; build steps in the repo",
      "An OpenXR sample renders on the glasses at native refresh",
      "3DoF head tracking from the glasses IMU, or a written gap"]),
    (23, "C5: Hiroglass HUD as a spatial panel", "P", "c5", "M5",
     "2026-12-07", "2026-12-16", "M", "standard", [22, 15],
     "The same HUD pages the pod shows, placed in space on the glasses. Shows the path from REV1 HUD to binocular XR.",
     ["HUD pages from Switchback render as a world-locked panel",
      "Runs for 30 minutes without a restart"]),

    # ---- program and CES ------------------------------------------------------
    (24, "M3 go/no-go review: choose the CES demo set", "P", "ces", "M3",
     "2026-11-20", "2026-11-20", "S", "fixed", [17, 18, 19, 21, 22],
     "One meeting, with the product owner. Each concept gets go, show-as-rendering, or stop.",
     ["Decision per concept recorded in plan/PLAN.md",
      "Cards for dropped concepts closed as not planned, with the reason",
      "Board replenished for M4 and M5"]),
    (25, "CES demo script and booth plan", "J", "ces", "M5",
     "2026-11-30", "2026-12-11", "S", "fixed", [24],
     "What each station shows, a 60-second pitch, and how a visitor tries a pod in under a minute.",
     ["Station list with the hardware each needs",
      "Pitch and an FAQ (price, open license, where to get the files)",
      "Booth signage text with a QR code to the repo"]),
    (26, "Open-hardware release pack for the CES demos", "J", "ces", "M5",
     "2026-12-07", "2026-12-18", "M", "intangible", [8, 24],
     "Everything someone needs to build one: the README design principles promise a permissive license and click-to-build.",
     ["Permissive hardware and software licenses chosen and added",
      "Per-demo README with BOM, gerbers, STLs and firmware build steps",
      "Tagged release on GitHub"]),
    (27, "Through-the-lens demo videos and photos", "J", "ces", "M5",
     "2026-12-14", "2026-12-18", "S", "standard", [11, 24],
     "Booth loop and social posts. Shot through the optic so people see what the wearer sees.",
     ["One 30-60 second clip per demo",
      "Stills for the README"]),
    (28, "Demo freeze: build kits, spares and travel case", "P", "ces", "M5",
     "2026-12-14", "2026-12-18", "M", "fixed", [11, 24],
     "Two working units of each chosen demo plus spares, chargers and cables. After this only fixes.",
     ["Kit checklist committed; every item ticked",
      "Each unit runs a full charge cycle without a fault",
      "Firmware and app builds tagged"]),
    (29, "CES dry run and pack", "P", "ces", "M6",
     "2027-01-04", "2027-01-05", "S", "fixed", [28],
     "Run the booth script end to end with someone who has not seen the demos, then pack.",
     ["Dry run done, issues fixed or noted",
      "Batteries charged; carry-on case packed (cells stay in carry-on)"]),
]

STATUSES = [
    ("Backlog", "GRAY", "Planned, not ready to start."),
    ("Ready", "BLUE", "Dependencies met; pull from here. Order matters: top is next."),
    ("In progress", "YELLOW", "WIP limit: 2 cards per engineer, 4 total."),
    ("Review", "PURPLE", "Waiting for the other engineer or a test. WIP limit 3."),
    ("Blocked", "RED", "Say what blocks it in a comment, and who can unblock."),
    ("Done", "GREEN", "Acceptance criteria met."),
]

FIELDS = {
    "Owner": list(OWNERS.values()),
    "Track": [t[0] for t in TRACKS.values()],
    "Size": ["S", "M", "L"],
    "Class of service": [c[0] for c in CLASSES.values()],
}

MS = {m[0]: m for m in MILESTONES}
BY_ID = {t[0]: t for t in TASKS}


def fmt(d):
    return date.fromisoformat(d).strftime("%b %d").replace(" 0", " ")


# ---------------------------------------------------------------- PLAN.md


def write_plan(issue_numbers=None, project_url=None):
    issue_numbers = issue_numbers or {}

    def ref(tid):
        n = issue_numbers.get(tid)
        return f"#{n}" if n else f"T{tid}"

    out = []
    w = out.append
    w("# Hiroglass REV1 plan: kickoff to CES 2027\n")
    w("Generated by `roadmap.py`. Edit the script, not this file.\n")
    if project_url:
        w(f"Board: {project_url}\n")
    w(f"Kickoff **Friday {START:%B %-d, %Y}**. CES runs **January 6-9, 2027** in Las Vegas. "
      "Demo freeze is **December 18**, so the holidays are buffer, not plan.\n")
    w("## Who starts on what\n")
    w("| | Senior (@truedat101) | Junior (@lk911) |\n|---|---|---|")
    w("| Friday Oct 16 | Long-lead Pod-0 orders and the USB-C fallback; start the schematic verify list | Order bench kits, optics and the concept 5 set; set up the toolchain |")
    w("| Weeks 1-4 | Pod-0 KiCad capture and layout to fab (M2); concept 4 sourcing by email on the side | Bench HUD firmware, then concept 1 plate combiner to a wearable clip-on (M1) |")
    w("| Weeks 5-6 | Concept 4 FPGA spike (1 week timebox), concept 5 Monado bring-up while boards are at fab | Concept 2 eyepiece and concept 3 visor reflector; auto brightness |")
    w("| Weeks 7-8 | Pod-0 rev A power bring-up, then firmware bring-up paired with Junior | Enclosure and mounts, Switchback profile, battery tests, booth script |")
    w("| Week 9 | Concept 5 spatial panel, demo freeze | Release pack, videos, demo freeze |")
    w("")
    w("Why this split: Pod-0 is the critical path and the riskiest electronics, so the senior owns it from day one. "
      "The junior owns concepts 1-3 end to end, which are real deliverables that need judgment, not just assembly. "
      "The two meet on firmware bring-up and the C5 spatial panel, where pairing teaches the most. "
      "Concepts 4 and 5 run in the fab wait, timeboxed, so they cannot starve Pod-0.\n")
    w("## Parallel paths\n")
    w("| Path | Bet | Owner | Decides at | If it fails |\n|---|---|---|---|---|")
    w("| Pod-0 custom PCB | The REV1 product | Senior | M4 Dec 11 | Demo on bench rigs in printed shells |")
    w("| C1 plate combiner | Baseline optic, lowest risk | Junior | M1 Nov 6 | None needed; it is the fallback |")
    w("| C2 look-around eyepiece | Brightest, outdoor-readable | Junior | M3 Nov 20 | Drop; C1 covers glasses |")
    w("| C3 behind-visor reflector | Helmet market | Junior | M3 Nov 20 | Show as rendering |")
    w("| C4 micro-OLED on FPGA | High-res path, open FPGA | Senior | M3 Nov 20 | Show board and rendering; datasheet NDA may rule it out |")
    w("| C5 Linux puck + Monado | Spatial XR teaser | Senior, then pair | M3 Nov 20 | Show the OpenXR sample only |")
    w("")
    w("## Milestones\n")
    w("| Milestone | Due | Done means |\n|---|---|---|")
    for key, title, due, done in MILESTONES:
        w(f"| {title} | {fmt(due)} | {done} |")
    w("")
    w("## Kanban policies\n")
    w("- **Flow, not sprints.** Cards move Backlog, Ready, In progress, Review, Done. Blocked is a flag, not a parking lot.")
    w("- **WIP limits:** 2 in progress per engineer, 3 in Review. Finish before starting: if you are at your limit, help clear Review.")
    w("- **Pull order:** Expedite, then Fixed date (earliest target first), then Standard (top of Ready first), then Intangible.")
    w("- **Expedite** is for long-lead orders and anything blocking the other engineer. At most one at a time.")
    w("- **Definition of done:** acceptance criteria ticked, files in the repo, the other engineer has seen it work.")
    w("- **Cadence:** async standup daily (what moved, what is blocked). Friday: 30-minute demo of whatever moved to Done, then replenish Ready. Milestone dates are the review points.")
    w("- **Metrics:** cycle time per card and throughput per week, read at the Friday replenishment. If cycle time grows, cut WIP before adding cards.")
    w("- **Fairness:** each engineer owns whole deliverables. Reviews go both ways: the junior reviews the senior's schematic and layout too.")
    w("- **Holidays:** Thanksgiving Nov 26-27 and Dec 21-Jan 1 are unplanned. Slips land there, not in nights and weekends.\n")
    w("## Cards\n")
    w("| Card | Owner | Track | Start | Target | Size | Class | Needs |\n|---|---|---|---|---|---|---|---|")
    for t in TASKS:
        tid, title, owner, track, ms, start, target, size, cos, deps = t[:10]
        needs = ", ".join(ref(d) for d in deps) or "-"
        w(f"| {ref(tid)} {title} | {OWNERS[owner]} | {TRACKS[track][0]} | {fmt(start)} | {fmt(target)} | {size} | {CLASSES[cos][0]} | {needs} |")
    w("")
    w("## Risks\n")
    w("- **Pod-0 parts lead time.** The full USB-C receptacle is 20 weeks and the Orient OLED 8 weeks. Mitigated by T1 on day one and the Winstar alternate.")
    w("- **Board respin.** There is no time for rev B before CES. Rev A must have test pads and bodge room; bench rigs in printed shells are the fallback demo.")
    w("- **Micro-OLED datasheets under NDA.** That would make C4 not open hardware. The C4 gate answers this before any money is spent.")
    w("- **Senior is the bottleneck on Pod-0.** The junior reviews schematic and layout and pairs on firmware so knowledge is not in one head.")
    w("- **Prices are snapshots.** DigiKey and Mouser blocked automated checks; the live cart in T2 updates them.\n")
    (HERE / "PLAN.md").write_text("\n".join(out))


# ---------------------------------------------------------------- GitHub sync


def gh(*args, json_out=False, check=True):
    r = subprocess.run(["gh", *args], capture_output=True, text=True)
    if check and r.returncode != 0:
        sys.exit(f"gh {' '.join(args)}\n{r.stderr}")
    return json.loads(r.stdout) if json_out else r.stdout.strip()


def graphql(query, **variables):
    args = ["api", "graphql", "-f", f"query={query}"]
    for k, v in variables.items():
        args += ["-F" if isinstance(v, int) else "-f", f"{k}={v}"]
    return json.loads(gh(*args))


def sync_labels():
    want = {"hiroglass": ("1d76db", "Hiroglass REV1")}
    for owner in OWNERS.values():
        want[f"owner:{owner.lower()}"] = ("ededed", f"{owner} engineer owns this")
    for key, (name, color) in TRACKS.items():
        want[f"track:{key}"] = (color, name)
    for key, (name, color, desc) in CLASSES.items():
        want[f"cos:{key}"] = (color, f"{name}: {desc}")
    for name, (color, desc) in want.items():
        gh("label", "create", name, "--repo", REPO, "--color", color,
           "--description", desc[:100], "--force")


def sync_milestones():
    have = {m["title"]: m["number"] for m in
            gh("api", f"repos/{REPO}/milestones?state=all&per_page=100", json_out=True)}
    for key, title, due, done in MILESTONES:
        fields = ["-f", f"title={title}", "-f", f"due_on={due}T23:59:00Z",
                  "-f", f"description={done}"]
        if title in have:
            gh("api", "-X", "PATCH", f"repos/{REPO}/milestones/{have[title]}", *fields)
        else:
            gh("api", "-X", "POST", f"repos/{REPO}/milestones", *fields)


def issue_body(t, numbers):
    tid, title, owner, track, ms, start, target, size, cos, deps, why, accept = t
    deps_txt = ", ".join(f"#{numbers[d]}" if d in numbers else f"T{d}" for d in deps) or "none"
    lines = [
        why, "",
        "## Acceptance criteria", "",
        *[f"- [ ] {a}" for a in accept], "",
        "## Plan", "",
        f"- **Owner:** {OWNERS[owner]} ({', '.join('@' + p for p in PEOPLE[owner])})",
        f"- **Track:** {TRACKS[track][0]}",
        f"- **Start / target:** {fmt(start)} to {fmt(target)}",
        f"- **Milestone:** {MS[ms][1]} (due {fmt(MS[ms][2])})",
        f"- **Class of service:** {CLASSES[cos][0]}",
        f"- **Needs:** {deps_txt}", "",
        f"Part of #{TRACKING_ISSUE}. Plan and policies: [PLAN.md]({TREE}/plan/PLAN.md) · "
        f"[BOMs]({TREE}/bom/README.md) · [schematics]({TREE}/schematics/README.md) · "
        f"[report]({TREE}/reports/Open%20hardware%20XR%20wearable%20approaches.md)",
        "", f"<!-- roadmap:T{tid} -->",
    ]
    return "\n".join(lines)


def sync_issues():
    existing = gh("issue", "list", "--repo", REPO, "--state", "all", "--label", "hiroglass",
                  "--limit", "200", "--json", "number,title,body", json_out=True)
    numbers = {}
    for t in TASKS:
        tag = f"<!-- roadmap:T{t[0]} -->"
        for i in existing:
            if tag in (i["body"] or "") or i["title"] == t[1]:
                numbers[t[0]] = i["number"]
    # Two passes so bodies can link dependencies by issue number.
    for t in TASKS:
        tid, title, owner, track, ms, *_ = t
        labels = ["hiroglass", f"owner:{OWNERS[owner].lower()}", f"track:{track}", f"cos:{t[8]}"]
        if tid not in numbers:
            url = gh("issue", "create", "--repo", REPO, "--title", title,
                     "--body", issue_body(t, numbers), "--milestone", MS[ms][1],
                     "--label", ",".join(labels), "--assignee", ",".join(PEOPLE[owner]))
            numbers[tid] = int(url.rstrip("/").split("/")[-1])
            print("created", url)
    for t in TASKS:
        tid, title, owner, track, ms, *_ = t
        labels = ["hiroglass", f"owner:{OWNERS[owner].lower()}", f"track:{track}", f"cos:{t[8]}"]
        gh("issue", "edit", str(numbers[tid]), "--repo", REPO, "--title", title,
           "--body", issue_body(t, numbers), "--milestone", MS[ms][1],
           "--add-label", ",".join(labels), "--add-assignee", ",".join(PEOPLE[owner]))
    return numbers


def update_readme(num):
    gh("project", "edit", num, "--owner", ORG, "--readme",
       (HERE / "PLAN.md").read_text()[:20000],
       "--description", "Hiroglass REV1 open-hardware HUD: kickoff Oct 16 2026, demo freeze Dec 18, CES Jan 6-9 2027")


def sync_project(numbers):
    projects = gh("project", "list", "--owner", ORG, "--format", "json", "--limit", "100",
                  json_out=True)["projects"]
    proj = next((p for p in projects if p["title"] == PROJECT_TITLE), None)
    if not proj:
        proj = gh("project", "create", "--owner", ORG, "--title", PROJECT_TITLE,
                  "--format", "json", json_out=True)
        gh("project", "link", str(proj["number"]), "--owner", ORG, "--repo", REPO)
    num, pid = str(proj["number"]), proj["id"]
    update_readme(num)

    fields = {f["name"]: f for f in
              gh("project", "field-list", num, "--owner", ORG, "--format", "json",
                 "--limit", "50", json_out=True)["fields"]}
    for name, options in FIELDS.items():
        if name not in fields:
            gh("project", "field-create", num, "--owner", ORG, "--name", name,
               "--data-type", "SINGLE_SELECT", "--single-select-options", ",".join(options))
    for name in ("Start date", "Target date"):
        if name not in fields:
            gh("project", "field-create", num, "--owner", ORG, "--name", name, "--data-type", "DATE")

    status = fields.get("Status") or {}
    if [o["name"] for o in status.get("options", [])] != [s[0] for s in STATUSES]:
        opts = ", ".join(f'{{name: "{n}", color: {c}, description: "{d}"}}' for n, c, d in STATUSES)
        graphql(f"""mutation($id: ID!) {{ updateProjectV2Field(input: {{fieldId: $id,
                 singleSelectOptions: [{opts}]}}) {{ projectV2Field {{ ... on ProjectV2SingleSelectField {{ id }} }} }} }}""",
                id=status["id"])

    fields = {f["name"]: f for f in
              gh("project", "field-list", num, "--owner", ORG, "--format", "json",
                 "--limit", "50", json_out=True)["fields"]}

    def option(field, name):
        return next(o["id"] for o in fields[field]["options"] if o["name"] == name)

    def set_select(item, field, value):
        gh("project", "item-edit", "--id", item, "--project-id", pid,
           "--field-id", fields[field]["id"], "--single-select-option-id", option(field, value))

    items = {i["content"].get("number"): i["id"] for i in
             gh("project", "item-list", num, "--owner", ORG, "--format", "json",
                "--limit", "200", json_out=True)["items"]}
    for t in TASKS:
        tid, title, owner, track, ms, start, target, size, cos, deps = t[:10]
        n = numbers[tid]
        item = items.get(n)
        fresh = item is None
        if fresh:
            item = gh("project", "item-add", num, "--owner", ORG, "--url",
                      f"https://github.com/{REPO}/issues/{n}", "--format", "json",
                      json_out=True)["id"]
        set_select(item, "Owner", OWNERS[owner])
        set_select(item, "Track", TRACKS[track][0])
        set_select(item, "Size", size)
        set_select(item, "Class of service", CLASSES[cos][0])
        for fname, d in (("Start date", start), ("Target date", target)):
            gh("project", "item-edit", "--id", item, "--project-id", pid,
               "--field-id", fields[fname]["id"], "--date", d)
        if fresh:  # never move a card someone has already moved
            set_select(item, "Status", "Ready" if start == START.isoformat() else "Backlog")
        print("board", f"#{n}", title)
    return num, proj.get("url") or f"https://github.com/orgs/{ORG}/projects/{num}"


def main():
    if "--github" not in sys.argv:
        write_plan()
        print("wrote", HERE / "PLAN.md")
        return
    sync_labels()
    sync_milestones()
    write_plan()
    numbers = sync_issues()
    if "project" not in gh("auth", "status", check=False) + subprocess.run(
            ["gh", "auth", "status"], capture_output=True, text=True).stderr:
        write_plan(numbers)
        sys.exit("issues synced; board skipped: run  gh auth refresh -s project  and re-run")
    num, url = sync_project(numbers)
    write_plan(numbers, url)
    update_readme(num)  # again, now with issue numbers and the board link
    print("project", url)


if __name__ == "__main__":
    main()
