#!/usr/bin/env python3
"""
make_grid.py — elevation grid + track for the Pop-11 renderer.

    python3 make_grid.py TILEDIR NAME W H PAD   e.g.  make_grid.py scratch post 156 104 0.10

Reads Terrarium tiles terr_Z_X_Y.png from TILEDIR (zoom 14, Forest Park set),
densifies the demo loop, fits a W:H box around it with PAD margin, and writes
grid_NAME.txt (W H, then H rows of W elevations in decimetres) and
track_NAME.txt (one "x y" per line). Prints metres per pixel.
"""
import math, sys, glob, os, re
from PIL import Image
tiledir, name, cw, ch, pad = sys.argv[1], sys.argv[2], int(sys.argv[3]), int(sys.argv[4]), float(sys.argv[5])
tiles = {}
for f in glob.glob(os.path.join(tiledir, 'terr_*.png')):
    z, x, y = map(int, re.findall(r'terr_(\d+)_(\d+)_(\d+)', f)[0]); tiles[(x, y)] = Image.open(f).convert('RGB')
xs = sorted({x for x, _ in tiles}); ys = sorted({y for _, y in tiles}); x0t, y0t = xs[0], ys[0]
W, H = 256 * len(xs), 256 * len(ys)
mosaic = Image.new('RGB', (W, H))
for (x, y), im in tiles.items(): mosaic.paste(im, ((x - x0t) * 256, (y - y0t) * 256))
px = mosaic.load(); n = 2 ** z
def elev(x, y): r, g, b = px[x, y]; return (r * 256 + g + b / 256) - 32768
def topx(lat, lon):
    fx = (lon + 180) / 360 * n; fy = (1 - math.log(math.tan(math.radians(lat)) + 1 / math.cos(math.radians(lat))) / math.pi) / 2 * n
    return (fx - x0t) * 256, (fy - y0t) * 256
wps = [(45.5363, -122.7122), (45.5389, -122.7169), (45.5412, -122.7212), (45.5440, -122.7251), (45.5428, -122.7290),
       (45.5395, -122.7306), (45.5366, -122.7272), (45.5340, -122.7215), (45.5334, -122.7161), (45.5363, -122.7122)]
pts = []
for (a, b), (c, d) in zip(wps, wps[1:]):
    for t in range(20): u = t / 20; pts.append((a + (c - a) * u, b + (d - b) * u))
pts.append(wps[-1]); pp = [topx(*p) for p in pts]
minx = min(p[0] for p in pp); maxx = max(p[0] for p in pp); miny = min(p[1] for p in pp); maxy = max(p[1] for p in pp)
bw = (maxx - minx) * (1 + 2 * pad); bh = (maxy - miny) * (1 + 2 * pad)
if bw / bh < cw / ch: bw = bh * cw / ch
else: bh = bw * ch / cw
cx = (minx + maxx) / 2; cy = (miny + maxy) / 2; x0 = cx - bw / 2; y0 = cy - bh / 2
def sample(fx, fy):
    x = max(0, min(W - 2, fx)); y = max(0, min(H - 2, fy)); ix = int(x); iy = int(y); dx = x - ix; dy = y - iy
    return elev(ix, iy) * (1 - dx) * (1 - dy) + elev(ix + 1, iy) * dx * (1 - dy) + elev(ix, iy + 1) * (1 - dx) * dy + elev(ix + 1, iy + 1) * dx * dy
with open(f'grid_{name}.txt', 'w') as f:
    f.write(f"{cw} {ch}\n")
    for j in range(ch): f.write(' '.join(str(int(round(sample(x0 + (i + .5) * bw / cw, y0 + (j + .5) * bh / ch) * 10))) for i in range(cw)) + '\n')
with open(f'track_{name}.txt', 'w') as f:
    for fx, fy in pp: f.write(f"{int(round((fx - x0) / bw * cw))} {int(round((fy - y0) / bh * ch))}\n")
mpp = 156543.03 * math.cos(math.radians(45.54)) / n * (bw / cw)
print(f"{name}: {cw}x{ch} {mpp:.1f} m/px, 500 m = {500 / mpp:.0f} px")
