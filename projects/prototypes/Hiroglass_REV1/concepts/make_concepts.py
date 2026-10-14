#!/usr/bin/env python3
"""Generate the Hiroglass REV1 concept sheets as SVG.

One sheet per approach in the research report, plus the recommended Pod-0
system. Every sheet shares one layout: a header, a technical section on the
left, the device in context on the right, and a spec strip along the bottom.

    python3 make_concepts.py        # writes *.svg next to this file
"""
import math
import os

W, H = 1200, 760
OUT = os.path.dirname(os.path.abspath(__file__))

INK = "#1b232c"
MUTED = "#5a6672"
RULE = "#c9d1d8"
BG = "#f7f9fb"
PANEL = "#ffffff"
HOUSING = "#2b333b"
GLASS = "#9fd3e6"
LIGHT = "#e8572a"     # display light
WORLD = "#4f7fa8"     # light from the world
ACCENT = "#1f6f8b"
TINT = "#6b7177"
SANS = "IBM Plex Sans, Helvetica Neue, Helvetica, Arial, sans-serif"
MONO = "IBM Plex Mono, Menlo, Consolas, monospace"


def esc(s):
    return s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


class Sheet:
    def __init__(self, num, title, subtitle):
        self.parts = []
        self.num, self.title, self.subtitle = num, title, subtitle

    def add(self, s):
        self.parts.append(s)

    # primitives -----------------------------------------------------------
    def text(self, x, y, s, size=13, anchor="start", color=INK, weight=400,
             font=SANS, italic=False):
        style = ' font-style="italic"' if italic else ""
        self.add(f'<text x="{x:.1f}" y="{y:.1f}" font-family="{font}" '
                 f'font-size="{size}" font-weight="{weight}" fill="{color}" '
                 f'text-anchor="{anchor}"{style}>{esc(s)}</text>')

    def lines(self, x, y, rows, size=13, gap=17, **kw):
        for i, r in enumerate(rows):
            self.text(x, y + i * gap, r, size=size, **kw)

    def callout(self, x1, y1, x2, y2, rows, anchor="start", size=12.5):
        """Leader from a point on the drawing (x1,y1) to a label at (x2,y2)."""
        self.add(f'<circle cx="{x1}" cy="{y1}" r="2.6" fill="{INK}"/>')
        self.add(f'<polyline points="{x1},{y1} {x2},{y2}" fill="none" '
                 f'stroke="{MUTED}" stroke-width="1"/>')
        dx = 6 if anchor == "start" else -6
        self.lines(x2 + dx, y2 + 4, rows, size=size, anchor=anchor, gap=15)

    def ray(self, pts, color=LIGHT, dashed=False, width=2, arrow=True, opacity=1):
        p = " ".join(f"{x:.1f},{y:.1f}" for x, y in pts)
        dash = ' stroke-dasharray="6 5"' if dashed else ""
        mk = ' marker-end="url(#arrow-light)"' if arrow and color == LIGHT else (
             ' marker-end="url(#arrow-world)"' if arrow else "")
        self.add(f'<polyline points="{p}" fill="none" stroke="{color}" '
                 f'stroke-width="{width}" stroke-linejoin="round" '
                 f'opacity="{opacity}"{dash}{mk}/>')

    def box(self, x, y, w, h, rows=(), fill=PANEL, stroke=INK, color=INK,
            size=13, rx=6, dashed=False, title=None):
        dash = ' stroke-dasharray="5 4"' if dashed else ""
        self.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" '
                 f'fill="{fill}" stroke="{stroke}" stroke-width="1.4"{dash}/>')
        ty = y + 21
        if title:
            self.text(x + 12, ty, title, size=size + 1, weight=600, color=color)
            ty += 18
        self.lines(x + 12, ty, rows, size=size - 1, color=color if title is None
                   else (MUTED if fill == PANEL else color), gap=16)

    def arrow(self, x1, y1, x2, y2, label=None, color=INK, both=False,
              lx=None, ly=None, anchor="middle"):
        start = ' marker-start="url(#arrow-ink-rev)"' if both else ""
        self.add(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" '
                 f'stroke-width="1.5" marker-end="url(#arrow-ink)"{start}/>')
        if label:
            lx = (x1 + x2) / 2 if lx is None else lx
            ly = (y1 + y2) / 2 - 7 if ly is None else ly
            self.text(lx, ly, label, size=11.5, anchor=anchor, color=MUTED,
                      font=MONO)

    def dim(self, x1, y1, x2, y2, label, off=14):
        """Dimension line with end ticks and a centred label above it."""
        self.add(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{MUTED}" '
                 f'stroke-width="1" marker-end="url(#tick)" marker-start="url(#tick)"/>')
        self.text((x1 + x2) / 2, (y1 + y2) / 2 - off + 10, label, size=11.5,
                  anchor="middle", color=MUTED, font=MONO)

    def eye(self, cx, cy, r=66, facing=0):
        """Eye in section. facing: 0 looks toward +x, -90 looks toward -y."""
        g = [f'<g transform="rotate({facing} {cx} {cy})">',
             f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="#fbfcfd" stroke="{INK}" stroke-width="1.6"/>']
        a = math.radians(38)
        x0, y0 = cx + r * math.cos(a), cy - r * math.sin(a)
        x1, y1 = cx + r * math.cos(a), cy + r * math.sin(a)
        g.append(f'<path d="M{x0:.1f},{y0:.1f} Q{cx + r * 1.42:.1f},{cy} {x1:.1f},{y1:.1f}" '
                 f'fill="#eef6f9" stroke="{INK}" stroke-width="1.4"/>')
        g.append(f'<ellipse cx="{cx + r * 0.72:.1f}" cy="{cy}" rx="{r * 0.1:.1f}" '
                 f'ry="{r * 0.32:.1f}" fill="{GLASS}" stroke="{INK}" stroke-width="1.2"/>')
        g.append(f'<line x1="{cx + r * 0.84:.1f}" y1="{cy - r * 0.42:.1f}" '
                 f'x2="{cx + r * 0.84:.1f}" y2="{cy - r * 0.16:.1f}" stroke="{INK}" stroke-width="3"/>')
        g.append(f'<line x1="{cx + r * 0.84:.1f}" y1="{cy + r * 0.16:.1f}" '
                 f'x2="{cx + r * 0.84:.1f}" y2="{cy + r * 0.42:.1f}" stroke="{INK}" stroke-width="3"/>')
        g.append(f'<circle cx="{cx - r * 0.86:.1f}" cy="{cy + r * 0.12:.1f}" r="{r * 0.09:.1f}" fill="{RULE}"/>')
        g.append("</g>")
        self.add("".join(g))

    def vlens(self, x, y0, y1, bulge=10, fill=GLASS, opacity=0.55):
        """Biconvex lens standing vertically."""
        my = (y0 + y1) / 2
        self.add(f'<path d="M{x},{y0} Q{x + bulge * 2},{my} {x},{y1} Q{x - bulge * 2},{my} {x},{y0} Z" '
                 f'fill="{fill}" fill-opacity="{opacity}" stroke="{INK}" stroke-width="1.3"/>')

    def hlens(self, x0, x1, y, bulge=9, opacity=0.55):
        mx = (x0 + x1) / 2
        self.add(f'<path d="M{x0},{y} Q{mx},{y - bulge * 2} {x1},{y} Q{mx},{y + bulge * 2} {x0},{y} Z" '
                 f'fill="{GLASS}" fill-opacity="{opacity}" stroke="{INK}" stroke-width="1.3"/>')

    def plate(self, x0, y0, x1, y1, width=5, color=GLASS):
        self.add(f'<line x1="{x0}" y1="{y0}" x2="{x1}" y2="{y1}" stroke="{color}" '
                 f'stroke-width="{width}" stroke-linecap="round"/>')
        self.add(f'<line x1="{x0}" y1="{y0}" x2="{x1}" y2="{y1}" stroke="{INK}" '
                 f'stroke-width="1"/>')

    def panel(self, x, y, w, h, label):
        self.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="4" fill="{PANEL}" '
                 f'stroke="{RULE}" stroke-width="1"/>')
        self.text(x + 16, y + 24, label.upper(), size=11, color=MUTED, font=MONO,
                  weight=500)

    # sheet scaffolding ----------------------------------------------------
    def specs(self, cells):
        x0, y0, w, h = 30, 580, (W - 60) / len(cells), 120
        self.add(f'<rect x="30" y="{y0}" width="{W - 60}" height="{h}" rx="4" '
                 f'fill="{PANEL}" stroke="{RULE}"/>')
        for i, (k, v) in enumerate(cells):
            x = x0 + i * w
            if i:
                self.add(f'<line x1="{x}" y1="{y0 + 16}" x2="{x}" y2="{y0 + h - 16}" stroke="{RULE}"/>')
            self.text(x + 18, y0 + 30, k.upper(), size=10.5, color=MUTED, font=MONO,
                      weight=500)
            rows = v if isinstance(v, (list, tuple)) else [v]
            self.lines(x + 18, y0 + 60, rows[:1], size=19, weight=600)
            self.lines(x + 18, y0 + 84, rows[1:], size=12.5, color=MUTED, gap=16)

    def svg(self):
        head = [
            f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" '
            f'width="{W}" height="{H}" role="img" aria-label="Hiroglass REV1 concept {self.num}: {esc(self.title)}">',
            "<defs>",
            f'<marker id="arrow-light" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="{LIGHT}"/></marker>',
            f'<marker id="arrow-world" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path d="M0,0 L10,5 L0,10 z" fill="{WORLD}"/></marker>',
            f'<marker id="arrow-ink" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="7" markerHeight="7" orient="auto"><path d="M0,0 L10,5 L0,10 z" fill="{INK}"/></marker>',
            f'<marker id="arrow-ink-rev" viewBox="0 0 10 10" refX="2" refY="5" markerWidth="7" markerHeight="7" orient="auto"><path d="M10,0 L0,5 L10,10 z" fill="{INK}"/></marker>',
            f'<marker id="tick" viewBox="0 0 10 10" refX="5" refY="5" markerWidth="8" markerHeight="8" orient="auto"><path d="M5,0 L5,10" stroke="{MUTED}" stroke-width="1.5"/></marker>',
            f'<pattern id="hatch" width="6" height="6" patternUnits="userSpaceOnUse" patternTransform="rotate(45)"><line x1="0" y1="0" x2="0" y2="6" stroke="{MUTED}" stroke-width="1"/></pattern>',
            "</defs>",
            f'<rect width="{W}" height="{H}" fill="{BG}"/>',
        ]
        hdr = []
        s = Sheet.__new__(Sheet)
        s.parts = hdr
        s.text(30, 38, f"HIROGLASS REV1 · CONCEPT {self.num}", size=12, color=ACCENT,
               font=MONO, weight=500)
        s.text(30, 70, self.title, size=28, weight=600)
        s.text(W - 30, 70, self.subtitle, size=14, anchor="end", color=MUTED)
        s.text(30, H - 22, "OpenWR · concept sketch, not to scale · see reports/"
               "Open hardware XR wearable approaches.md", size=11, color=MUTED, font=MONO)
        legend_x = W - 30
        s.text(legend_x, H - 22, "display light", size=11, anchor="end", color=MUTED, font=MONO)
        s.add(f'<line x1="{legend_x - 132}" y1="{H - 26}" x2="{legend_x - 102}" y2="{H - 26}" stroke="{LIGHT}" stroke-width="2"/>')
        s.text(legend_x - 145, H - 22, "world light", size=11, anchor="end", color=MUTED, font=MONO)
        s.add(f'<line x1="{legend_x - 268}" y1="{H - 26}" x2="{legend_x - 238}" y2="{H - 26}" stroke="{WORLD}" stroke-width="2" stroke-dasharray="6 5"/>')
        return "\n".join(head + hdr + self.parts + ["</svg>"]) + "\n"

    def save(self, name):
        with open(os.path.join(OUT, name), "w") as f:
            f.write(self.svg())


LEFT = (30, 90, 580, 470)
RIGHT = (630, 90, 540, 470)


def glasses_front(s, cx, cy, sc=1.0, temples=True):
    """Front view of a pair of glasses; returns lens centres."""
    lx, rx = cx - 88 * sc, cx + 88 * sc
    for x in (lx, rx):
        s.add(f'<rect x="{x - 72 * sc:.1f}" y="{cy - 48 * sc:.1f}" width="{144 * sc:.1f}" '
              f'height="{96 * sc:.1f}" rx="{40 * sc:.1f}" fill="{GLASS}" fill-opacity="0.18" '
              f'stroke="{INK}" stroke-width="{3.2 * sc:.1f}"/>')
    s.add(f'<path d="M{cx - 16 * sc:.1f},{cy - 26 * sc:.1f} Q{cx},{cy - 40 * sc:.1f} '
          f'{cx + 16 * sc:.1f},{cy - 26 * sc:.1f}" fill="none" stroke="{INK}" stroke-width="{3.2 * sc:.1f}"/>')
    if temples:
        s.add(f'<line x1="{lx - 72 * sc:.1f}" y1="{cy - 30 * sc:.1f}" x2="{lx - 112 * sc:.1f}" '
              f'y2="{cy - 34 * sc:.1f}" stroke="{INK}" stroke-width="{4 * sc:.1f}" stroke-linecap="round"/>')
        s.add(f'<line x1="{rx + 72 * sc:.1f}" y1="{cy - 30 * sc:.1f}" x2="{rx + 112 * sc:.1f}" '
              f'y2="{cy - 34 * sc:.1f}" stroke="{INK}" stroke-width="{4 * sc:.1f}" stroke-linecap="round"/>')
    return lx, rx


def puck(s, x, y, rows=("LiPo + BQ24074",)):
    s.add(f'<rect x="{x}" y="{y}" width="128" height="58" rx="22" fill="{HOUSING}"/>')
    s.add(f'<rect x="{x + 54}" y="{y - 4}" width="20" height="6" rx="2" fill="{MUTED}"/>')
    s.text(x + 64, y + 26, "PUCK", size=11, anchor="middle", color="#ffffff", font=MONO,
           weight=500)
    s.lines(x + 64, y + 43, rows, size=10.5, anchor="middle", color="#c9d1d8", gap=13)


# 1 -----------------------------------------------------------------------
def concept_1():
    s = Sheet(1, "Plate-combiner pod", "goggl1 electronics · one lens · tilted partial mirror")
    s.panel(*LEFT, "Side section · optical path")
    ex, ey = 150, 360
    s.eye(ex, ey)
    # prescription glasses lens (optional)
    s.add(f'<path d="M262,262 Q252,360 262,458 L270,458 Q262,360 270,262 Z" fill="{GLASS}" '
          f'fill-opacity="0.35" stroke="{MUTED}" stroke-width="1.1"/>')
    # world rays pass through plate
    for y in (345, 360, 375):
        s.ray([(590, y), (226, y)], color=WORLD, dashed=True, width=1.6)
    # housing
    s.add(f'<path d="M318,168 h88 a6,6 0 0 1 6,6 v122 h-12 v-108 h-64 v108 h-24 v-122 a6,6 0 0 1 6,-6 Z" fill="{HOUSING}"/>')
    s.add(f'<rect x="336" y="190" width="56" height="10" fill="#3d7a52"/>')  # PCB
    s.add(f'<rect x="336" y="200" width="56" height="9" fill="#11161b"/>')    # OLED
    s.add(f'<line x1="408" y1="296" x2="408" y2="402" stroke="{HOUSING}" stroke-width="5"/>')
    s.hlens(338, 390, 262, bulge=7)
    s.plate(328, 392, 396, 324, width=6)
    # display rays: down, then fold toward the eye
    for x in (348, 364, 380):
        y_hit = 720 - x
        s.ray([(x, 209), (x, y_hit), (226, y_hit)], width=2.2)
    # focus screw
    s.add(f'<rect x="416" y="196" width="8" height="40" rx="2" fill="{MUTED}"/>')
    s.arrow(438, 200, 438, 236, both=True)
    s.callout(392, 205, 470, 150, ["0.96″ SSD1306 OLED", "128×64, multi-vendor"])
    s.callout(390, 262, 470, 268, ["PMMA asphere", "f ≈ 15–20 mm"])
    s.callout(442, 218, 470, 212, ["M2 focus screw, ±2 mm"])
    s.callout(380, 336, 470, 312, ["wedged plate", "30R / 70T, 45°"])
    s.callout(266, 300, 248, 160, ["your glasses", "(optional)"], anchor="end")
    s.dim(222, 470, 360, 470, "eye relief 20–30 mm")
    # virtual image ghost
    s.text(520, 420, "12:34", size=30, anchor="middle", color=LIGHT, weight=600, font=MONO)
    s.text(520, 444, "virtual image ≈ 2 m", size=11.5, anchor="middle", color=MUTED, font=MONO)
    s.lines(50, 520, ["Display light folds off the plate into the eye. The world",
                      "passes straight through, so the HUD overlays the scene."],
            size=12.5, color=MUTED)

    s.panel(*RIGHT, "On the wearer · front view")
    lx, rx = glasses_front(s, 900, 250)
    # pod on the wearer's right temple (viewer's left)
    s.add(f'<rect x="{lx - 92}" y="186" width="66" height="44" rx="9" fill="{HOUSING}"/>')
    s.add(f'<rect x="{lx - 46}" y="226" width="40" height="44" fill="{GLASS}" '
          f'fill-opacity="0.6" stroke="{INK}" stroke-width="1"/>')
    s.add(f'<path d="M{lx - 92},208 C{lx - 150},214 {lx - 150},300 {lx - 136},420" '
          f'fill="none" stroke="{HOUSING}" stroke-width="3"/>')
    s.callout(lx - 60, 198, lx - 40, 140, ["pod on the dock", "magnets + dovetail"])
    s.callout(lx - 18, 250, lx + 40, 330, ["combiner sits over", "the upper outer lens"])
    puck(s, lx - 200, 424)
    s.box(880, 380, 270, 150, title="Pod electronics",
          rows=["ESP32-C3-MINI-1 · BLE", "BMI270 IMU · VEML7700 light",
                "Forth console + signed OTA", "Switchback lib/goggle as-is"])

    s.specs([("Prototype BOM", ["$75–215", "≈ $24 silicon at qty 1"]),
             ("Outdoors", ["Weak in snow", "good behind the tint"]),
             ("Single-supplier", ["None", "plates, lenses, SSD1306"]),
             ("Path to binocular", ["Low", "small FOV, BLE only"]),
             ("Team effort", ["1–2 months", "one optomech person"])])
    s.save("concept-1-plate-combiner.svg")


# 2 -----------------------------------------------------------------------
def concept_2():
    s = Sheet(2, "Look-around eyepiece pod", "the sunlight answer · glance, don't overlay")
    s.panel(*LEFT, "Top view · where the eyepiece sits")
    ex, ey = 230, 450
    R = 280
    def pt(deg, r):
        a = math.radians(deg)
        return ex + r * math.sin(a), ey - r * math.cos(a)
    # primary gaze kept clear (±8°)
    a0, a1 = pt(-8, R), pt(8, R)
    s.add(f'<path d="M{ex},{ey} L{a0[0]:.1f},{a0[1]:.1f} L{a1[0]:.1f},{a1[1]:.1f} Z" fill="{RULE}" fill-opacity="0.45"/>')
    b0, b1 = pt(8.7, R), pt(23.7, R)
    s.add(f'<path d="M{ex},{ey} L{b0[0]:.1f},{b0[1]:.1f} L{b1[0]:.1f},{b1[1]:.1f} Z" fill="{LIGHT}" fill-opacity="0.16"/>')
    s.ray([(ex, ey - 70), (ex, 130)], color=WORLD, dashed=True, width=1.6)
    s.eye(ex, ey, r=44, facing=-90)
    # glasses front and temple
    s.add(f'<path d="M120,372 Q230,356 340,372" fill="none" stroke="{INK}" stroke-width="4"/>')
    s.add(f'<path d="M340,372 L362,382 L392,548" fill="none" stroke="{INK}" stroke-width="4" stroke-linejoin="round"/>')
    # eyepiece on its arm
    c = pt(16, 196)
    s.add(f'<line x1="358" y1="378" x2="{c[0] + 30:.1f}" y2="{c[1] + 10:.1f}" stroke="{HOUSING}" stroke-width="6"/>')
    s.add(f'<g transform="rotate(16 {c[0]:.1f} {c[1]:.1f})">'
          f'<rect x="{c[0] - 17:.1f}" y="{c[1] - 40:.1f}" width="34" height="80" rx="7" fill="{HOUSING}"/>'
          f'<rect x="{c[0] - 12:.1f}" y="{c[1] - 34:.1f}" width="24" height="5" fill="#11161b"/>'
          f'<ellipse cx="{c[0]:.1f}" cy="{c[1] + 36:.1f}" rx="13" ry="4" fill="{GLASS}" stroke="{INK}"/></g>')
    s.lines(a0[0] - 4, a0[1] - 22, ["primary gaze", "±8° stays clear"], size=11.5, anchor="end",
            color=MUTED, font=MONO, gap=14)
    s.lines(b1[0] - 4, b1[1] - 26, ["8.7°–23.7° toward the ear", "(arXiv 2505.09047)"],
            size=11.5, color=LIGHT, font=MONO, gap=14)
    s.callout(c[0] + 14, c[1], 430, 300, ["opaque eyepiece", "outside your glasses"])
    s.callout(392, 540, 430, 520, ["temple dock,", "either side"])
    s.text(120, 360, "nose side", size=11, color=MUTED, font=MONO)
    s.text(420, 490, "ear side", size=11, color=MUTED, font=MONO)

    s.panel(*RIGHT, "Eyepiece section · what you see")
    s.eye(700, 220, r=40)
    s.add(f'<rect x="790" y="174" width="240" height="92" rx="10" fill="{HOUSING}"/>')
    s.add(f'<rect x="1010" y="186" width="10" height="68" fill="#11161b"/>')
    s.add(f'<rect x="1020" y="186" width="6" height="68" fill="#3d7a52"/>')
    s.vlens(812, 184, 256, bulge=8)
    for y, ty in ((196, 206), (220, 220), (244, 234)):
        s.ray([(1008, y), (826, ty), (752, ty)], width=2.2)
    s.add(f'<rect x="1030" y="200" width="16" height="40" rx="3" fill="{MUTED}"/>')
    s.callout(1040, 204, 1060, 150, ["0.96″ OLED", "or 1.1″ AMOLED"])
    s.callout(812, 190, 820, 150, ["asphere, ±2 mm focus"])
    s.text(900, 300, "no combiner: ~100% of panel light reaches the eye",
           size=12, anchor="middle", color=LIGHT, font=MONO)
    # glance view mock-up
    s.add(f'<rect x="660" y="330" width="480" height="200" rx="8" fill="#dfe7ee"/>')
    s.add(f'<path d="M660,470 L770,400 L860,450 L980,380 L1140,460 L1140,530 L660,530 Z" fill="#ffffff"/>')
    s.add(f'<path d="M660,470 L770,400 L860,450 L980,380 L1140,460" fill="none" stroke="{RULE}" stroke-width="2"/>')
    s.add(f'<rect x="1000" y="344" width="124" height="74" rx="8" fill="#0d1216"/>')
    s.text(1062, 375, "24 km/h", size=17, anchor="middle", color="#7cf0a6", font=MONO, weight=500)
    s.text(1062, 400, "▲ 1,240 m", size=13, anchor="middle", color="#7cf0a6", font=MONO)
    s.text(676, 354, "WEARER'S VIEW · glance right to read", size=11, color=MUTED, font=MONO)

    s.specs([("Prototype BOM", ["$70–190", "plate removed from #1"]),
             ("Outdoors", ["Strong", "full panel brightness"]),
             ("Single-supplier", ["None", "same parts as #1"]),
             ("Path to binocular", ["Low", "proves pod, dock, calibration"]),
             ("Team effort", ["1–2 months", "no ghosting to fight"])])
    s.save("concept-2-look-around.svg")


# 3 -----------------------------------------------------------------------
def concept_3():
    s = Sheet(3, "Behind-the-visor reflector", "helmets and goggles · combiner behind the tint")
    s.panel(*LEFT, "Side section · inside a goggle")
    ex, ey = 170, 380
    # face line
    s.eye(ex, ey, r=60)
    # goggle lens (tinted), frame and foam
    s.add(f'<path d="M318,196 Q402,370 318,536 L334,536 Q418,370 334,196 Z" fill="{TINT}" fill-opacity="0.45" stroke="{INK}" stroke-width="1.2"/>')
    s.add(f'<path d="M200,184 L340,184 L340,204 L214,204 Z" fill="{HOUSING}"/>')
    s.add(f'<path d="M200,526 L340,526 L340,546 L214,546 Z" fill="{HOUSING}"/>')
    s.add(f'<rect x="190" y="176" width="22" height="36" rx="4" fill="url(#hatch)" stroke="{MUTED}"/>')
    s.add(f'<rect x="190" y="518" width="22" height="36" rx="4" fill="url(#hatch)" stroke="{MUTED}"/>')
    # HUD film patch on the inner surface
    s.add(f'<path d="M357,338 Q364,372 360,410" fill="none" stroke="{LIGHT}" stroke-width="5" stroke-linecap="round" opacity="0.8"/>')
    # world light: bright outside, dimmed inside
    for y in (360, 384):
        s.ray([(590, y), (412, y)], color=WORLD, dashed=True, width=2.4, arrow=False)
        s.ray([(400, y), (236, y)], color=WORLD, dashed=True, width=1.2, opacity=0.55)
    # pod at the brow
    s.add(f'<rect x="232" y="206" width="70" height="34" rx="6" fill="{HOUSING}"/>')
    s.add(f'<rect x="282" y="214" width="6" height="22" fill="#11161b"/>')
    for (x0, y0), (x1, y1) in (((290, 228), (358, 352)), ((290, 234), (361, 392))):
        s.ray([(x0, y0), (x1, y1), (236, (y1 + ey) / 2 - 2)], width=2.2)
    s.callout(270, 206, 360, 150, ["pod at the brow", "display + projection lens"])
    s.callout(360, 372, 450, 300, ["HUD film patch", "on the inner surface"])
    s.callout(380, 450, 420, 480, ["tint dims the world 5–10×,", "not the display"])
    s.text(500, 352, "bright", size=11, anchor="middle", color=WORLD, font=MONO)

    s.panel(*RIGHT, "On a goggle and a helmet")
    # ski goggle front view
    s.add('<g transform="translate(0,22)">')
    s.add(f'<path d="M700,170 Q900,120 1100,170 Q1130,230 1100,290 Q1010,320 935,296 Q900,282 865,296 Q790,320 700,290 Q670,230 700,170 Z" fill="{TINT}" fill-opacity="0.35" stroke="{INK}" stroke-width="5"/>')
    s.add(f'<path d="M650,215 Q672,212 690,214 M1110,214 Q1128,212 1150,215" stroke="{ACCENT}" stroke-width="16" stroke-linecap="round" fill="none"/>')
    s.add(f'<rect x="750" y="176" width="60" height="24" rx="5" fill="{HOUSING}"/>')
    s.add(f'<rect x="756" y="210" width="50" height="34" rx="3" fill="{LIGHT}" fill-opacity="0.35" stroke="{LIGHT}"/>')
    s.callout(806, 250, 870, 270, ["film patch, one per goggle family"])
    s.add("</g>")
    s.callout(780, 204, 830, 140, ["pod inside, under the frame"])
    # helmet side view
    s.add(f'<path d="M700,500 Q700,370 840,360 Q960,356 990,470 L990,500 Z" fill="#e7ecf0" stroke="{INK}" stroke-width="2"/>')
    s.add(f'<path d="M980,440 Q1060,450 1080,520 L1060,526 Q1040,470 976,462 Z" fill="{TINT}" fill-opacity="0.4" stroke="{INK}" stroke-width="1.4"/>')
    s.add(f'<rect x="958" y="424" width="34" height="18" rx="4" fill="{HOUSING}"/>')
    s.callout(975, 424, 1020, 384, ["breakaway brow mount"])
    s.callout(1050, 490, 1090, 540, ["visor or flip-down combiner"], anchor="end")

    s.specs([("Prototype BOM", ["$80–200", "+ $10–40 optics + film"]),
             ("Outdoors", ["Strong", "combiner behind the tint"]),
             ("Single-supplier", ["None", "but every goggle curves differently"]),
             ("Path to binocular", ["Modest", "wide visors host research combiners"]),
             ("Team effort", ["2–3 months", "+ per-goggle adapters"])])
    s.save("concept-3-visor-reflector.svg")


# 4 -----------------------------------------------------------------------
def concept_4():
    s = Sheet(4, "Micro-OLED pod on an open FPGA", "Brilliant Labs Frame-class optics · experimental branch")
    s.panel(*LEFT, "Side section · birdbath")
    ex, ey = 150, 360
    s.eye(ex, ey)
    for y in (345, 375):
        s.ray([(590, y), (480, y)], color=WORLD, dashed=True, width=1.6, arrow=False)
        s.ray([(452, y), (226, y)], color=WORLD, dashed=True, width=1.2, opacity=0.55)
    s.add(f'<path d="M318,186 h92 v40 h-92 Z" fill="{HOUSING}"/>')
    s.add(f'<rect x="338" y="226" width="52" height="7" fill="#11161b"/>')
    s.add(f'<rect x="306" y="300" width="160" height="124" rx="10" fill="none" stroke="{HOUSING}" stroke-width="3"/>')
    s.plate(326, 326, 396, 396, width=6)
    s.add(f'<path d="M446,302 Q480,360 446,420" fill="none" stroke="{INK}" stroke-width="4"/>')
    for x in (350, 364, 378):
        s.ray([(x, 233), (x, x), (446 + (x - 364) * 0.05, x)], width=2, arrow=False)
        s.ray([(456, x), (226, x)], width=2.2)
    s.callout(330, 212, 300, 160, ["Sony 0.23″ 640×400 micro-OLED", "or SeeYA 0.49″ 1080p"], anchor="end")
    s.callout(330, 330, 260, 262, ["50/50 splitter"], anchor="end")
    s.callout(468, 330, 500, 300, ["curved mirror"])
    s.text(330, 470, "≈ 25% of panel light reaches the eye:", size=12, color=LIGHT, font=MONO)
    s.text(330, 488, "3,000 nits in → ~700 nits out", size=12, color=LIGHT, font=MONO)
    s.lines(50, 528, ["Same panel in a look-around layout keeps its full 3,000–4,000",
                      "nits, which is the better outdoor use of this hardware."],
            size=12.5, color=MUTED)

    s.panel(*RIGHT, "Signal chain")
    s.box(660, 130, 210, 66, title="Phone · Switchback", rows=["retained-mode primitives"])
    s.box(660, 250, 210, 86, title="nRF52840 / ESP32-C3",
          rows=["protocol, Forth or Lua", "panel config over SPI"])
    s.box(930, 250, 210, 86, title="iCE40 / ECP5 FPGA",
          rows=["Yosys + nextpnr, fully open", "frame buffer + timing"])
    s.box(930, 400, 210, 66, title="micro-OLED", rows=["single source, no datasheet"],
          fill="#fdf0eb", stroke=LIGHT)
    s.box(660, 400, 210, 66, title="RP2350 PIO", rows=["FPGA-free option, untested"],
          dashed=True)
    s.arrow(765, 198, 765, 248, "BLE", lx=790, ly=228, anchor="start")
    s.arrow(872, 293, 928, 293, "SPI")
    s.arrow(1035, 338, 1035, 398, "8-bit YCbCr", lx=1045, ly=372, anchor="start")
    s.add(f'<line x1="872" y1="433" x2="928" y2="433" stroke="{MUTED}" stroke-width="1.4" stroke-dasharray="5 4" marker-end="url(#arrow-ink)"/>')
    s.lines(660, 510, ["BLE moves ~46–63 KB/s; one 640×400 frame at 4 bpp is 128 KB.",
                       "So the glass renders primitives itself, as Frame and ActiveLook do."],
            size=12, color=MUTED)

    s.specs([("Prototype BOM", ["$200–450", "panel kits $184–415"]),
             ("Outdoors", ["Good look-around", "marginal through a birdbath"]),
             ("Single-supplier", ["Panel", "Sony / SeeYA"]),
             ("Path to binocular", ["Good", "same panel class ×2"]),
             ("Team effort", ["3–6 months", "RTL + board work"])])
    s.save("concept-4-microoled-fpga.svg")


# 5 -----------------------------------------------------------------------
def concept_5():
    s = Sheet(5, "USB-C display on a Linux puck", "binocular through Monado / OpenXR")
    s.panel(*LEFT, "Top view · binocular head unit")
    l, r, ey = 220, 420, 470
    s.eye(l, ey, r=40, facing=-90)
    s.eye(r, ey, r=40, facing=-90)
    s.dim(l, 538, r, 538, "IPD 55–72 mm on the rail")
    # rail
    s.add(f'<rect x="120" y="292" width="400" height="20" rx="3" fill="{HOUSING}"/>')
    s.add(f'<path d="M120,292 l14,-10 h372 l14,10 Z" fill="{MUTED}"/>')
    for x in (l, r):
        s.add(f'<rect x="{x - 46}" y="318" width="92" height="76" rx="8" fill="#ffffff" stroke="{HOUSING}" stroke-width="3"/>')
        s.add(f'<line x1="{x - 30}" y1="384" x2="{x + 30}" y2="330" stroke="{GLASS}" stroke-width="5"/>')
        s.add(f'<path d="M{x - 34},322 Q{x},310 {x + 34},322" fill="none" stroke="{INK}" stroke-width="3"/>')
        s.arrow(x - 58, 356, x - 50, 356)
        s.arrow(x + 58, 356, x + 50, 356)
    s.add(f'<rect x="306" y="266" width="28" height="22" rx="3" fill="#3d7a52"/>')
    s.callout(320, 268, 360, 200, ["IMU with FIFO + sync pin,", "mounted on the optics"])
    # stereo camera bar
    s.add(f'<rect x="200" y="226" width="240" height="22" rx="11" fill="none" stroke="{INK}" stroke-width="1.6" stroke-dasharray="5 4"/>')
    for x in (232, 408):
        s.add(f'<circle cx="{x}" cy="237" r="7" fill="{INK}"/>')
    s.callout(436, 237, 476, 244, ["stereo camera bar", "add-on, 6DoF"])
    s.callout(r + 30, 336, 486, 330, ["birdbath per eye", "(borrowed, or", "dual ECX335)"])
    s.text(120, 146, "same dovetail rail as the REV1 pod", size=11.5, color=MUTED, font=MONO)

    s.panel(*RIGHT, "System")
    s.box(660, 130, 220, 100, title="Head unit",
          rows=["two displays, IMU, calibration", "USB-C tether, every pin routed"])
    s.box(930, 130, 220, 120, title="Puck · RK3588 / Pi 5",
          rows=["Monado (OpenXR runtime)", "Hiroglass driver", "Basalt SLAM for 6DoF"])
    s.box(930, 310, 220, 66, title="OpenXR apps", rows=["any engine, no vendor SDK"])
    s.box(660, 310, 220, 86, title="Phone · Switchback",
          rows=["settings, HUD widgets", "BLE sideband only"])
    s.add(f'<line x1="882" y1="160" x2="928" y2="160" stroke="{LIGHT}" stroke-width="2" marker-start="url(#arrow-light)"/>')
    s.text(905, 152, "DP alt", size=11, anchor="middle", color=LIGHT, font=MONO)
    s.arrow(882, 200, 928, 200, "USB", lx=905, ly=218)
    s.arrow(1040, 252, 1040, 308)
    s.add(f'<line x1="770" y1="232" x2="770" y2="308" stroke="{MUTED}" stroke-width="1.4" stroke-dasharray="5 4"/>')
    s.text(780, 276, "BLE", size=11, color=MUTED, font=MONO)
    s.box(660, 430, 490, 100, title="Fastest start: borrow the displays",
          rows=["Xreal, Viture, Rokid and RayNeo glasses already run on Linux through",
                "wheaney's XRLinuxDriver (GPL-3.0); Monado 25.1 does 3DoF on Xreal.",
                "Write the 3DoF Monado driver for the monocular pod first."],
          fill="#eaf3f6", stroke=ACCENT)

    s.specs([("Prototype BOM", ["$150–450", "+ puck SoC"]),
             ("Outdoors", ["Marginal", "birdbath class"]),
             ("Single-supplier", ["Panel or vendor", "several glasses brands work"]),
             ("Path to binocular", ["Direct", "this is the path"]),
             ("Team effort", ["3–6 months", "mostly software"])])
    s.save("concept-5-usbc-monado.svg")


# 0 -----------------------------------------------------------------------
def adapter(s, x, y, kind):
    s.add(f'<rect x="{x}" y="{y}" width="150" height="150" rx="6" fill="#ffffff" stroke="{RULE}"/>')
    cx = x + 75
    # dovetail receiver on every adapter
    s.add(f'<path d="M{cx - 26},{y + 34} h52 l-8,14 h-36 Z" fill="{MUTED}"/>')
    s.add(f'<circle cx="{cx - 14}" cy="{y + 41}" r="4" fill="#c0392b"/><circle cx="{cx + 14}" cy="{y + 41}" r="4" fill="#c0392b"/>')
    if kind == "temple":
        s.add(f'<rect x="{x + 18}" y="{y + 84}" width="114" height="12" rx="6" fill="{INK}"/>')
        s.add(f'<path d="M{cx - 22},{y + 48} v28 a14,14 0 0 0 0,28 h44 a14,14 0 0 0 0,-28 v-28" fill="none" stroke="{ACCENT}" stroke-width="5"/>')
        label = ["Temple clamp", "glasses, sunglasses"]
    elif kind == "goggle":
        s.add(f'<path d="M{x + 14},{y + 116} Q{cx},{y + 70} {x + 136},{y + 116}" fill="none" stroke="{INK}" stroke-width="9"/>')
        s.add(f'<path d="M{cx - 18},{y + 48} v34 q0,14 14,10 l10,-2" fill="none" stroke="{ACCENT}" stroke-width="5"/>')
        label = ["Goggle-frame clip", "ski, moto, construction"]
    else:
        s.add(f'<path d="M{x + 14},{y + 128} Q{x + 20},{y + 66} {cx + 40},{y + 64}" fill="none" stroke="{INK}" stroke-width="9"/>')
        s.add(f'<rect x="{cx - 20}" y="{y + 50}" width="40" height="16" rx="3" fill="{ACCENT}"/>')
        s.text(cx, y + 140, "releases on impact", size=10.5, anchor="middle", color="#c0392b", font=MONO)
        label = ["Breakaway mount", "helmet brow, hat brim"]
    s.lines(cx, y + 176, label, size=12.5, anchor="middle", gap=16)


def concept_0():
    s = Sheet(0, "Pod-0: the recommended first build", "one pod, two cartridges, three adapters, one puck")
    s.panel(*LEFT, "Exploded view")
    # pod body
    s.add(f'<rect x="200" y="140" width="220" height="92" rx="16" fill="{HOUSING}"/>')
    s.add(f'<rect x="402" y="174" width="18" height="24" rx="4" fill="{MUTED}"/>')
    s.lines(220, 172, ["POD", "ESP32-C3 · SSD1306 0.96″", "BMI270 · VEML7700"], size=12,
            color="#ffffff", gap=18, font=MONO)
    s.callout(420, 186, 470, 160, ["USB-C, every pin", "routed for DP alt later"])
    # cartridges
    s.add(f'<rect x="150" y="300" width="130" height="96" rx="10" fill="#ffffff" stroke="{INK}" stroke-width="1.6"/>')
    s.hlens(176, 254, 328, bulge=6)
    s.plate(176, 384, 254, 344, width=6)
    s.text(215, 418, "Cartridge A", size=13, anchor="middle", weight=600)
    s.text(215, 434, "plate combiner, see-through", size=11.5, anchor="middle", color=MUTED)
    s.add(f'<rect x="340" y="300" width="130" height="96" rx="10" fill="#ffffff" stroke="{INK}" stroke-width="1.6"/>')
    s.add(f'<rect x="380" y="316" width="50" height="66" rx="8" fill="{HOUSING}"/>')
    s.add(f'<ellipse cx="405" cy="382" rx="18" ry="5" fill="{GLASS}" stroke="{INK}"/>')
    s.text(405, 418, "Cartridge B", size=13, anchor="middle", weight=600)
    s.text(405, 434, "look-around, for sun", size=11.5, anchor="middle", color=MUTED)
    s.text(310, 352, "or", size=13, anchor="middle", color=MUTED, italic=True)
    s.arrow(215, 296, 260, 236)
    s.arrow(405, 296, 360, 236)
    s.text(310, 466, "snap in · M2 focus slide ±2 mm", size=11, anchor="middle", color=MUTED, font=MONO)
    # dock
    s.add(f'<path d="M240,486 h140 l-18,28 h-104 Z" fill="{MUTED}"/>')
    for x in (270, 310, 350):
        s.add(f'<circle cx="{x}" cy="{498}" r="7" fill="#c0392b"/>')
    s.text(310, 540, "Dock: keyed dovetail locates, N52 magnets hold", size=12, anchor="middle")
    s.add(f'<line x1="310" y1="472" x2="310" y2="484" stroke="{MUTED}" stroke-width="1.2" stroke-dasharray="4 4"/>')
    s.lines(60, 150, ["≤ 25–30 g", "on the head"], size=15, weight=600, gap=20)
    s.lines(60, 220, ["mirror-mount", "for either eye"], size=12.5, color=MUTED)

    s.panel(*RIGHT, "Adapters and puck")
    adapter(s, 650, 130, "temple")
    adapter(s, 820, 130, "goggle")
    adapter(s, 990, 130, "helmet")
    s.add(f'<path d="M660,420 C720,440 760,470 800,476" fill="none" stroke="{HOUSING}" stroke-width="3"/>')
    puck(s, 800, 448, rows=("LiPo + BQ24074",))
    s.text(660, 444, "to pod", size=11, color=MUTED, font=MONO)
    s.lines(950, 462, ["Battery stays warm in a pocket:", "-10 °C can halve capacity.",
                       "NTC blocks charging below 0 °C."], size=12, color=MUTED, gap=16)
    s.text(650, 404, "Parametric CadQuery / OpenSCAD: print a fit for your own eyewear.",
           size=12, color=MUTED)

    s.specs([("Weeks 1–6", ["Pod-0", "bench optics, fit on 3 mounts"]),
             ("Months 2–4", ["Pod-1", "custom PCB, field adapters"]),
             ("Month 3 on", ["Branch", "concepts 4 and 5"]),
             ("Brightness test", ["First", "OLEDs vs. tint vs. snow"]),
             ("Licenses", ["CERN-OHL-P", "MIT / Apache-2.0 code"])])
    s.save("concept-0-pod0-system.svg")


if __name__ == "__main__":
    for f in (concept_0, concept_1, concept_2, concept_3, concept_4, concept_5):
        f()
    print("wrote", sorted(n for n in os.listdir(OUT) if n.endswith(".svg")))
