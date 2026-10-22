"""
goggle_frames.py — the goggl1 screens SpecsSwitchbackXR drives, drawn by the
same ssd1309.c the goggle runs (through hud_mock.py's ctypes harness).

    python3 tools/goggle_frames.py docs/images

Writes goggle-<name>.png (4x, cyan on near black, as the other design
renders) and goggle-<name>.txt (the 128x64 bitmap as # and .). If it fits
here, it fits on the glass. Requires Pillow.
"""
import math, os, random, sys

HERE = os.path.dirname(os.path.abspath(__file__))
GOGGL1 = os.path.join(HERE, '..', '..', '..', 'prototypes', 'esp32c3_beetle_goggl1')
MAPLET = os.path.join(HERE, '..', '..', '..', 'prototypes', 'maplet_xiao_eink')

# Borrow hud_mock's harness (library build, primitives, png) without its
# fixture and theme drawing, which start at the fixture banner.
src = open(os.path.join(GOGGL1, 'tools', 'hud_mock.py')).read()
src = src.split('# ---------------- fixture')[0]
g = {'__file__': os.path.join(GOGGL1, 'tools', 'hud_mock.py')}
exec(compile(src, 'hud_mock.py', 'exec'), g)
clear, color, scale, pixel, line, hline, vline = (g[k] for k in
    ('clear', 'color', 'scale', 'pixel', 'line', 'hline', 'vline'))
frame, box, circle, disc, text, png, dump, getpix = (g[k] for k in
    ('frame', 'box', 'circle', 'disc', 'text', 'png', 'dump', 'getpix'))
from PIL import Image


def glyph(x, y, ch):
    text(x, y, ch)


def tri(p, q, r):
    """Filled triangle by scanlines (the goggle would get oled-tri)."""
    ys = [p[1], q[1], r[1]]
    for y in range(int(min(ys)), int(max(ys)) + 1):
        xs = []
        for a, b in ((p, q), (q, r), (r, p)):
            if (a[1] <= y < b[1]) or (b[1] <= y < a[1]):
                xs.append(a[0] + (y - a[1]) * (b[0] - a[0]) / (b[1] - a[1]))
        if len(xs) >= 2:
            x0, x1 = sorted(xs)[:2]
            hline(int(round(x0)), y, int(round(x1)) - int(round(x0)) + 1)


def strip(left, right):
    """The shared status strip, rows 56-63, as on every weather theme."""
    hline(0, 56, 128)
    text(0, 57, left)
    text(128 - 6 * len(right), 57, right)


# ---------------------------------------------------------------- Pulse
def pulse():
    """Heart rate first: bpm huge, the zone as five cells, time and cadence."""
    clear(); color(1); scale(1)
    bpm, zone = 148, 4
    scale(4); text(0, 2, "%d" % bpm); scale(1)
    text(76, 2, "bpm")
    # heart, 7x6
    for y, row in enumerate([".##.##.", "#######", "#######", ".#####.", "..###..", "...#..."]):
        for x, c in enumerate(row):
            if c == '#': pixel(76 + x, 13 + y)
    scale(2); text(98, 2, "Z%d" % zone); scale(1)
    text(98, 19, "of 5")
    # zone cells: 5 x 22 px, filled up to the zone, the current one taller
    for i in range(5):
        x = 2 + i * 25
        if i < zone - 1: box(x, 40, 22, 5)
        elif i == zone - 1: box(x, 36, 22, 9)
        else: frame(x, 40, 22, 5)
    text(0, 47, "32:14")
    text(128 - 6 * 7, 47, "172 spm")
    strip("Wildwood", "14:07")
    return "pulse"


# ---------------------------------------------------------------- Pin
def pin_arrow():
    """A destination pin: which way, how far, what it is called."""
    clear(); color(1); scale(1)
    cx, cy, r = 24, 27, 22
    circle(cx, cy, r)
    for k in range(12):                        # ticks every 30 degrees
        a = math.radians(k * 30)
        line(int(cx + (r - 3) * math.sin(a)), int(cy - (r - 3) * math.cos(a)),
             int(cx + r * math.sin(a)), int(cy - r * math.cos(a)))
    bearing = 38                               # relative to your heading
    a = math.radians(bearing)
    tip = (cx + 17 * math.sin(a), cy - 17 * math.cos(a))
    l = (cx + 8 * math.sin(a + 2.5), cy - 8 * math.cos(a + 2.5))
    rr = (cx + 8 * math.sin(a - 2.5), cy - 8 * math.cos(a - 2.5))
    tri(tip, l, (cx, cy)); tri(tip, rr, (cx, cy))
    disc(cx, cy, 2)
    text(52, 2, "PIN 2 OF 3")
    scale(2); text(52, 13, "1.4"); scale(1); text(90, 20, "km")
    text(52, 32, "Wildwood TH")
    text(52, 42, "+180m  ~22m")
    strip("pin 14:05", "14:07")
    return "pin"


# ---------------------------------------------------------------- Map
def terrain(w, h, seed=11):
    """A made-up ridge running NE with two drainages, enough for contours."""
    rnd = random.Random(seed)
    bumps = [(rnd.uniform(-10, w + 10), rnd.uniform(-10, h + 10), rnd.uniform(8, 20), rnd.uniform(-35, 45))
             for _ in range(16)]
    def z(x, y):
        v = 3.2 * (128 - x) * 0.0 + 1.6 * (60 - y) + 1.1 * x    # climbs to the NE
        for bx, by, s, a in bumps:
            v += a * math.exp(-((x - bx) ** 2 + (y - by) ** 2) / (2 * s * s))
        return v
    return z


def minimap():
    """The phone's map, cut to 128x56: contours, your track, the pins."""
    clear(); color(1); scale(1)
    W, H = 128, 56
    z = terrain(W, H)
    step = 25.0                                 # 25 m contours, every 4th bold
    band = [[int(z(x, y) // step) for x in range(W + 1)] for y in range(H + 1)]
    for y in range(H):
        for x in range(W):
            b, br, bd = band[y][x], band[y][x + 1], band[y + 1][x]
            if b != br or b != bd:
                hi = max(b, br, bd)
                if hi % 4 == 0 or (x + y) % 2 == 0:   # thin lines dotted, index lines solid
                    pixel(x, y)
    track = [(6, 52), (14, 47), (21, 48), (29, 42), (36, 40), (43, 33), (51, 32), (57, 27), (63, 28)]
    plan = [(63, 28), (72, 22), (82, 20), (92, 15), (103, 13), (114, 9)]
    def pts(path):
        for (x0, y0), (x1, y1) in zip(path, path[1:]):
            n = max(abs(x1 - x0), abs(y1 - y0))
            for i in range(n + 1):
                yield i, round(x0 + (x1 - x0) * i / n), round(y0 + (y1 - y0) * i / n)
    color(0)                                    # halos first, all of them
    for _, x, y in pts(track): box(x - 2, y - 2, 5, 5)
    for _, x, y in pts(plan): box(x - 1, y - 1, 4, 4)
    color(1)
    for _, x, y in pts(track): box(x - 1, y - 1, 2, 2)
    for i, x, y in pts(plan):                   # the planned leg: 2 px dashes
        if (i // 3) % 2 == 0: box(x, y, 2, 2)
    # waypoint pins: little inverted triangles on a stem
    for x, y in ((82, 20), (103, 13)):
        color(0); box(x - 4, y - 8, 9, 9); color(1)
        tri((x - 3, y - 7), (x + 4, y - 7), (x, y - 2)); vline(x, y - 2, 3)
    # destination flag
    x, y = 114, 9
    color(0); box(x - 2, y - 9, 10, 11); color(1)
    vline(x, y - 8, 9); box(x + 1, y - 8, 6, 4)
    # you: disc in a ring with a dark gap
    x, y = 63, 28
    color(0); disc(x, y, 6); color(1); circle(x, y, 5); disc(x, y, 2)
    # scale bar, top left
    color(0); box(0, 0, 30, 13); color(1)
    text(2, 1, "500m"); hline(2, 10, 24); vline(2, 8, 3); vline(25, 8, 3)
    strip("you  3 pins", "14:07")
    return "map"


# ---------------------------------------------------------------- Photo
def scene():
    """Stand-in for a roll frame: alpenglow behind a ridge, firs, the trail.

    The glass is emissive, so a frame reads best when the light sits in one
    band and the rest is silhouette."""
    from PIL import ImageDraw
    W, H = 512, 224
    im = Image.new('L', (W, H), 0)
    d = ImageDraw.Draw(im)
    for y in range(H):                          # glow just above the far ridge
        d.line([(0, y), (W, y)], fill=int(255 * math.exp(-max(0, 104 - y) / 70.0) ** 1.6))
    far = [(x, 100 + 14 * math.sin(x / 70.0) + 6 * math.sin(x / 23.0)) for x in range(0, W + 8, 8)]
    d.polygon(far + [(W, H), (0, H)], fill=30)
    d.line(far, fill=255, width=3)               # the ridge catches the light
    near = [(x, 140 + 20 * math.sin(x / 95.0 + 1) + 5 * math.sin(x / 17.0)) for x in range(0, W + 8, 8)]
    d.polygon(near + [(W, H), (0, H)], fill=8)
    # two firs framing the frame, the glow left open between them
    for x, base, h in ((40, 236, 230), (96, 232, 150), (470, 236, 220), (420, 230, 120)):
        w = h * 0.30
        for k in range(7):
            t0 = base - h + k * h / 8
            d.polygon([(x, t0), (x - w * (k + 2) / 8, t0 + h / 3.2), (x + w * (k + 2) / 8, t0 + h / 3.2)], fill=0)
    d.polygon([(220, H), (310, H), (266, 158), (258, 158)], fill=170)     # the trail, fading up
    d.polygon([(258, 158), (266, 158), (281, 143), (277, 143)], fill=110)
    return im


def photo():
    """A roll frame, dithered to the glass: 128x56 plus the strip."""
    clear(); color(1); scale(1)
    im = scene().resize((128, 56), Image.LANCZOS)
    # Floyd-Steinberg keeps the ridge and tree edges; ordered dither turned
    # the glow into a lattice at this size. Lit pixels are the light ones.
    bw = im.convert('1')
    px = bw.load()
    for y in range(56):
        for x in range(128):
            if px[x, y]:
                pixel(x, y)
    strip("roll 07", "14:12")
    return "photo"


out = sys.argv[1] if len(sys.argv) > 1 else '.'
os.makedirs(out, exist_ok=True)
for fn in (pulse, pin_arrow, minimap, photo):
    name = fn()
    rows, lit = dump(name)
    open(os.path.join(out, 'goggle-%s.txt' % name), 'w').write('\n'.join(rows) + '\n')
    png(os.path.join(out, 'goggle-%s.png' % name))
    print('== %s: %d lit px of 8192 (%d%%)' % (name, lit, lit * 100 // 8192))
