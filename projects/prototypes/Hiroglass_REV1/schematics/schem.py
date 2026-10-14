"""Tiny SVG schematic library for the Hiroglass REV1 sheets.

Two drawing styles share one sheet frame:

* Net-label schematics (`Sheet.ic`, `Sheet.two_pin`): every pin ends in a
  stub with a net label, the way KiCad global labels work, so nothing has to
  be routed and the netlist is the drawing. `Sheet.netlist()` returns it, and
  `Sheet.check()` reports nets with only one connection.
* Harness diagrams (`Sheet.board`, `Sheet.wire`): off-the-shelf boards drawn
  as pin headers, joined by coloured wires, for the bench kits.

Nothing here knows about any particular part; the sheets live in
make_schematics.py.
"""
from collections import defaultdict

INK = "#1b232c"
MUTED = "#5a6672"
RULE = "#c9d1d8"
BG = "#ffffff"
PAPER = "#f7f9fb"
BOX = "#fffdf5"
ACCENT = "#1f6f8b"
POWER = "#b5371f"
GROUND = "#1b232c"
SANS = "IBM Plex Sans, Helvetica Neue, Helvetica, Arial, sans-serif"
MONO = "IBM Plex Mono, Menlo, Consolas, monospace"

PIN = 34        # pin stub length
PITCH = 22      # pin spacing

WIRE_COLORS = {
    "red": "#d62828", "black": "#1b232c", "blue": "#1f6fd1", "yellow": "#d9a400",
    "green": "#2a9d4b", "orange": "#e76f1c", "purple": "#7b3fbf", "white": "#9aa5b1",
    "brown": "#8a5a2b", "grey": "#6b7280",
}


def esc(s):
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def is_power(net):
    n = net.upper()
    return n.startswith(("+", "V")) or n in ("3V3", "5V", "VBUS", "VBAT", "VSYS")


def is_ground(net):
    return net.upper() in ("GND", "AGND", "PGND", "SHIELD")


class Sheet:
    def __init__(self, title, subtitle, sheet_id, w=1600, h=1100, rev="A",
                 date="2026-10-14"):
        self.title, self.subtitle, self.sheet_id = title, subtitle, sheet_id
        self.w, self.h, self.rev, self.date = w, h, rev, date
        self.parts = []
        self.nets = defaultdict(list)      # net -> [(ref, pin)]
        self.pins = {}                     # (ref, pinname) -> (x, y)
        self.bom = []                      # (ref, value, mpn, note)
        self.noconnect = set()

    # low level ------------------------------------------------------------
    def add(self, s):
        self.parts.append(s)

    def text(self, x, y, s, size=12, anchor="start", color=INK, weight=400,
             font=SANS, rotate=None):
        rot = f' transform="rotate({rotate} {x:.1f} {y:.1f})"' if rotate else ""
        self.add(f'<text x="{x:.1f}" y="{y:.1f}" font-family="{font}" font-size="{size}" '
                 f'font-weight="{weight}" fill="{color}" text-anchor="{anchor}"{rot}>{esc(s)}</text>')

    def lines(self, x, y, rows, size=12, gap=16, **kw):
        for i, r in enumerate(rows):
            self.text(x, y + i * gap, r, size=size, **kw)

    def line(self, x1, y1, x2, y2, color=INK, width=1.4, dash=None):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" '
                 f'stroke="{color}" stroke-width="{width}"{d}/>')

    def rect(self, x, y, w, h, fill=BOX, stroke=INK, width=1.6, rx=2, dash=None):
        d = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(f'<rect x="{x:.1f}" y="{y:.1f}" width="{w:.1f}" height="{h:.1f}" rx="{rx}" '
                 f'fill="{fill}" stroke="{stroke}" stroke-width="{width}"{d}/>')

    def connect(self, ref, pin, net, x, y):
        if net in (None, "", "NC"):
            self.noconnect.add((ref, pin))
            return
        self.nets[net].append((ref, pin))
        self.pins[(ref, pin)] = (x, y)

    # net labels -----------------------------------------------------------
    def label(self, x, y, net, side):
        """Net flag at a stub end. side: 'left', 'right', 'up', 'down'."""
        if net in (None, "", "NC"):
            # no-connect cross
            self.line(x - 5, y - 5, x + 5, y + 5, color=MUTED)
            self.line(x - 5, y + 5, x + 5, y - 5, color=MUTED)
            return
        color = POWER if is_power(net) else (GROUND if is_ground(net) else ACCENT)
        if is_ground(net) and side in ("down", "up"):
            s = 1 if side == "down" else -1
            self.line(x, y, x, y + 8 * s, color=color)
            for i, half in enumerate((10, 6.5, 3)):
                yy = y + (8 + 4 * i) * s
                self.line(x - half, yy, x + half, yy, color=color, width=1.6)
            return
        tw = 7.2 * len(net) + 12
        h = 16
        if side == "right":
            pts = f"{x},{y} {x + 7},{y - h / 2} {x + 7 + tw},{y - h / 2} {x + 7 + tw},{y + h / 2} {x + 7},{y + h / 2}"
            tx, anchor = x + 10, "start"
        elif side == "left":
            pts = f"{x},{y} {x - 7},{y - h / 2} {x - 7 - tw},{y - h / 2} {x - 7 - tw},{y + h / 2} {x - 7},{y + h / 2}"
            tx, anchor = x - 10, "end"
        elif side == "up":
            self.line(x, y, x, y - 10, color=color)
            pts = f"{x - tw / 2},{y - 10} {x + tw / 2},{y - 10} {x + tw / 2},{y - 10 - h} {x - tw / 2},{y - 10 - h}"
            self.add(f'<polygon points="{pts}" fill="#ffffff" stroke="{color}" stroke-width="1.2"/>')
            self.text(x, y - 14, net, size=11.5, anchor="middle", color=color, font=MONO, weight=500)
            return
        else:
            self.line(x, y, x, y + 10, color=color)
            pts = f"{x - tw / 2},{y + 10} {x + tw / 2},{y + 10} {x + tw / 2},{y + 10 + h} {x - tw / 2},{y + 10 + h}"
            self.add(f'<polygon points="{pts}" fill="#ffffff" stroke="{color}" stroke-width="1.2"/>')
            self.text(x, y + 22, net, size=11.5, anchor="middle", color=color, font=MONO, weight=500)
            return
        self.add(f'<polygon points="{pts}" fill="#ffffff" stroke="{color}" stroke-width="1.2"/>')
        self.text(tx + (2 if side == "right" else -2), y + 4, net, size=11.5, anchor=anchor,
                  color=color, font=MONO, weight=500)

    # IC symbol ------------------------------------------------------------
    def ic(self, ref, value, x, y, left=(), right=(), width=170, mpn=None,
           note=None, top=None):
        """Box with pins. left/right: lists of (number, name, net) or None for a gap."""
        rows = max(len(left), len(right))
        h = (rows + 1) * PITCH
        self.rect(x, y, width, h)
        self.text(x, y - 24, ref, size=13, weight=600)
        self.text(x, y - 9, value, size=12, color=MUTED)
        if top:
            self.text(x + width / 2, y + h + 18, top, size=11, anchor="middle", color=MUTED,
                      font=MONO)
        for side, pins in (("left", left), ("right", right)):
            # long pin-number lists ("A4 A9 B4 B9") get a longer stub
            plen = max([PIN] + [6.3 * len(p[0]) + 12 for p in pins if p])
            for i, p in enumerate(pins):
                if p is None:
                    continue
                num, name, net = p
                py = y + (i + 1) * PITCH
                if side == "left":
                    x0, x1 = x - plen, x
                    self.line(x0, py, x1, py)
                    self.text(x1 + 6, py + 4, name, size=11.5, font=MONO)
                    self.text(x1 - 4, py - 4, num, size=10, anchor="end", color=MUTED, font=MONO)
                    self.label(x0, py, net, "left")
                    self.connect(ref, f"{num}:{name}", net, x0, py)
                else:
                    x0, x1 = x + width, x + width + plen
                    self.line(x0, py, x1, py)
                    self.text(x0 - 6, py + 4, name, size=11.5, anchor="end", font=MONO)
                    self.text(x0 + 4, py - 4, num, size=10, color=MUTED, font=MONO)
                    self.label(x1, py, net, "right")
                    self.connect(ref, f"{num}:{name}", net, x1, py)
        self.bom.append((ref, value, mpn or "", note or ""))
        return h

    # two-pin parts --------------------------------------------------------
    def two_pin(self, kind, ref, value, x, y, net_a, net_b, mpn=None, note=None,
                vertical=True):
        """Resistor, capacitor, LED, button, NTC, ferrite or diode.

        Vertical: pin A on top at (x, y), pin B at (x, y + 70).
        """
        L = 70
        if vertical:
            ax, ay, bx, by = x, y, x, y + L
            cx, cy = x, y + L / 2
        else:
            ax, ay, bx, by = x, y, x + L, y
            cx, cy = x + L / 2, y
        g = []
        rot = "" if vertical else f' transform="rotate(-90 {cx} {cy})"'
        # body drawn vertically around (cx, cy), then rotated
        if kind in ("R", "NTC", "FB"):
            fill = "#ffffff" if kind != "FB" else INK
            g.append(f'<rect x="{cx - 7}" y="{cy - 17}" width="14" height="34" fill="{fill}" stroke="{INK}" stroke-width="1.5"/>')
            g.append(f'<line x1="{cx}" y1="{cy - 35}" x2="{cx}" y2="{cy - 17}" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<line x1="{cx}" y1="{cy + 17}" x2="{cx}" y2="{cy + 35}" stroke="{INK}" stroke-width="1.4"/>')
            if kind == "NTC":
                g.append(f'<polyline points="{cx - 14},{cy + 16} {cx - 6},{cy + 16} {cx + 12},{cy - 16}" fill="none" stroke="{INK}" stroke-width="1.3"/>')
        elif kind == "C":
            g.append(f'<line x1="{cx}" y1="{cy - 35}" x2="{cx}" y2="{cy - 4}" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<line x1="{cx}" y1="{cy + 4}" x2="{cx}" y2="{cy + 35}" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<line x1="{cx - 13}" y1="{cy - 4}" x2="{cx + 13}" y2="{cy - 4}" stroke="{INK}" stroke-width="2.4"/>')
            g.append(f'<line x1="{cx - 13}" y1="{cy + 4}" x2="{cx + 13}" y2="{cy + 4}" stroke="{INK}" stroke-width="2.4"/>')
        elif kind in ("LED", "D", "TVS"):
            g.append(f'<line x1="{cx}" y1="{cy - 35}" x2="{cx}" y2="{cy - 9}" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<line x1="{cx}" y1="{cy + 9}" x2="{cx}" y2="{cy + 35}" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<polygon points="{cx - 10},{cy - 9} {cx + 10},{cy - 9} {cx},{cy + 9}" fill="#ffffff" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<line x1="{cx - 10}" y1="{cy + 9}" x2="{cx + 10}" y2="{cy + 9}" stroke="{INK}" stroke-width="1.8"/>')
            if kind == "LED":
                for d in (0, 7):
                    g.append(f'<line x1="{cx + 12}" y1="{cy - 4 + d}" x2="{cx + 20}" y2="{cy - 10 + d}" stroke="{INK}" stroke-width="1.1"/>')
        elif kind == "SW":
            g.append(f'<line x1="{cx}" y1="{cy - 35}" x2="{cx}" y2="{cy - 12}" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<line x1="{cx}" y1="{cy + 12}" x2="{cx}" y2="{cy + 35}" stroke="{INK}" stroke-width="1.4"/>')
            g.append(f'<circle cx="{cx}" cy="{cy - 12}" r="2.5" fill="{INK}"/><circle cx="{cx}" cy="{cy + 12}" r="2.5" fill="{INK}"/>')
            g.append(f'<line x1="{cx - 9}" y1="{cy - 14}" x2="{cx - 9}" y2="{cy + 14}" stroke="{INK}" stroke-width="1.8"/>')
            g.append(f'<line x1="{cx - 9}" y1="{cy}" x2="{cx - 18}" y2="{cy}" stroke="{INK}" stroke-width="1.4"/>')
        self.add(f'<g{rot}>' + "".join(g) + "</g>")
        if vertical:
            self.text(x + 18, cy - 2, ref, size=12, weight=600)
            self.text(x + 18, cy + 13, value, size=11, color=MUTED, font=MONO)
            self.label(ax, ay, net_a, "up")
            self.label(bx, by, net_b, "down")
        else:
            self.text(cx, y - 14, f"{ref}  {value}", size=11.5, anchor="middle")
            self.label(ax, ay, net_a, "left")
            self.label(bx, by, net_b, "right")
        self.connect(ref, "1", net_a, ax, ay)
        self.connect(ref, "2", net_b, bx, by)
        self.bom.append((ref, value, mpn or "", note or ""))

    # notes ----------------------------------------------------------------
    def note(self, x, y, w, title, rows, h=None):
        h = h or 30 + 16 * len(rows) + 8
        self.rect(x, y, w, h, fill="#eef5f8", stroke=ACCENT, width=1.1, rx=4)
        self.text(x + 12, y + 20, title, size=12.5, weight=600, color=ACCENT)
        self.lines(x + 12, y + 40, rows, size=11.5, gap=16)

    def frame_box(self, x, y, w, h, label):
        self.rect(x, y, w, h, fill="none", stroke=RULE, width=1.2, rx=6, dash="6 5")
        self.text(x + 12, y + 18, label.upper(), size=11, color=MUTED, font=MONO, weight=500)

    # harness diagrams ---------------------------------------------------------
    def board(self, name, x, y, pins_left=(), pins_right=(), width=170, sub=None,
              color="#253041"):
        """Off-the-shelf board as a dark block with header pins on each side."""
        rows = max(len(pins_left), len(pins_right))
        h = (rows + 1) * PITCH + 26
        self.add(f'<rect x="{x}" y="{y}" width="{width}" height="{h}" rx="8" fill="{color}"/>')
        self.text(x + width / 2, y + 20, name, size=13, anchor="middle", color="#ffffff",
                  weight=600)
        if sub:
            self.text(x + width / 2, y + h + 16, sub, size=11, anchor="middle", color=MUTED,
                      font=MONO)
        for side, pins in (("L", pins_left), ("R", pins_right)):
            for i, p in enumerate(pins):
                if not p:
                    continue
                py = y + 26 + (i + 1) * PITCH
                px = x if side == "L" else x + width
                self.add(f'<rect x="{px - 5}" y="{py - 5}" width="10" height="10" fill="#d4a017" stroke="#7a5c00"/>')
                tx = x + 12 if side == "L" else x + width - 12
                self.text(tx, py + 4, p, size=11.5, anchor="start" if side == "L" else "end",
                          color="#ffffff", font=MONO)
                self.pins[(name, p)] = (px, py)
        return h

    def wire(self, a, b, color="blue", label=None, via_x=None, path=None):
        """Wire between two board pins.

        Routed with one vertical run at via_x, or through explicit waypoints
        in `path` when the pins face away from each other.
        """
        (x1, y1), (x2, y2) = self.pins[a], self.pins[b]
        c = WIRE_COLORS.get(color, color)
        mx = via_x if via_x is not None else (x1 + x2) / 2
        pts = [(x1, y1)] + (list(path) if path else [(mx, y1), (mx, y2)]) + [(x2, y2)]
        pstr = " ".join(f"{x},{y}" for x, y in pts)
        self.add(f'<polyline points="{pstr}" fill="none" '
                 f'stroke="{c}" stroke-width="2.6" stroke-linejoin="round" opacity="0.92"/>')
        for x, y in ((x1, y1), (x2, y2)):
            self.add(f'<circle cx="{x}" cy="{y}" r="3.2" fill="{c}"/>')
        if label:
            lx, ly = pts[1][0], (pts[1][1] + pts[2][1]) / 2
            if pts[1][1] == pts[2][1]:
                lx, ly = (pts[1][0] + pts[2][0]) / 2, pts[1][1] - 4
            self.text(lx + 5, ly - 3, label, size=10.5, color=c, font=MONO)
        net = label or f"{a[1]}-{b[1]}"
        self.nets[net] += [a, b]

    # output ---------------------------------------------------------------
    def netlist(self):
        return {n: sorted(set(c)) for n, c in sorted(self.nets.items())}

    def check(self):
        """Nets with a single connection are almost always typos."""
        return [n for n, c in self.netlist().items()
                if len(c) < 2 and not n.startswith("TP_")]

    def svg(self):
        w, h = self.w, self.h
        out = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {w} {h}" width="{w}" '
               f'height="{h}" role="img" aria-label="{esc(self.title)}">',
               f'<rect width="{w}" height="{h}" fill="{PAPER}"/>',
               f'<rect x="16" y="16" width="{w - 32}" height="{h - 32}" fill="{BG}" stroke="{INK}" stroke-width="1.5"/>']
        # title block, bottom right
        tbw, tbh = 520, 92
        tx, ty = w - 16 - tbw, h - 16 - tbh
        out.append(f'<rect x="{tx}" y="{ty}" width="{tbw}" height="{tbh}" fill="#ffffff" stroke="{INK}" stroke-width="1.5"/>')
        out.append(f'<line x1="{tx}" y1="{ty + 50}" x2="{tx + tbw}" y2="{ty + 50}" stroke="{INK}"/>')
        for xx in (tx + 300, tx + 400):
            out.append(f'<line x1="{xx}" y1="{ty + 50}" x2="{xx}" y2="{ty + tbh}" stroke="{INK}"/>')
        hdr = Sheet.__new__(Sheet)
        hdr.parts = out
        hdr.text(tx + 12, ty + 22, self.title, size=16, weight=600)
        hdr.text(tx + 12, ty + 41, self.subtitle, size=11.5, color=MUTED)
        hdr.text(tx + 12, ty + 66, "OpenWR · Hiroglass REV1", size=10, color=MUTED, font=MONO)
        hdr.text(tx + 12, ty + 82, "CERN-OHL-P-2.0", size=10, color=MUTED, font=MONO)
        hdr.text(tx + 312, ty + 66, "SHEET", size=10, color=MUTED, font=MONO)
        hdr.text(tx + 312, ty + 84, self.sheet_id, size=13, weight=600, font=MONO)
        hdr.text(tx + 412, ty + 66, f"REV {self.rev}", size=10, color=MUTED, font=MONO)
        hdr.text(tx + 412, ty + 84, self.date, size=12, font=MONO)
        return "\n".join(out + self.parts + ["</svg>"]) + "\n"
