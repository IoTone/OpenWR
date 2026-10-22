"""
pulse_preview.py — the Pulse panel off the glasses, with fake data.

    python3 tools/pulse_preview.py docs/images/pulse-preview.png

Draws ZoneRing's real triangles (via tools/ring_geometry.js) and the panel's
text in the app's fonts, light on black as the additive display shows it,
for a sweep of heart rates. Layout numbers are the lens's, in cm; keep them
in step with the scene (PulseFrame). Requires Pillow and Node.
"""
import glob, json, os, subprocess, sys
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
FONTS = os.path.join(HERE, '..', 'Assets', 'Fonts')
TS = sorted(glob.glob(os.path.expanduser('~/.npm/_npx/*/node_modules/typescript/lib/typescript.js')))[0]

# --- the lens layout, cm, origin at the frame centre, y up -------------
FRAME = (20.0, 15.0)                 # ContainerFrame inner size
RING = dict(radius=5.2, thickness=0.45, extra=0.35, cx=0.0, cy=0.4)
BPM = dict(x=0.0, y=0.3, cap=3.0)    # centred in the ring
ZONE = dict(x=0.0, y=-4.2, cap=0.55)  # in the ring's opening
ROW = dict(x=0.0, y=-6.0, cap=0.6)
STATUS = dict(x=0.0, y=6.6, cap=0.42)
MAX_HR = 185

FOG, MOSS, GLOW, DIM = (243, 244, 241), (127, 200, 163), (240, 138, 98), (90, 90, 90)
PX = 36                              # pixels per cm


def zone_of(bpm):
    if bpm <= 0: return 0
    p = bpm / MAX_HR
    return 5 if p >= .9 else 4 if p >= .8 else 3 if p >= .7 else 2 if p >= .6 else 1


def font(name, cap_cm, weight=None):
    f = ImageFont.truetype(os.path.join(FONTS, name), max(8, int(cap_cm * PX / 0.72)))
    if weight:
        try: f.set_variation_by_name(weight)
        except Exception: pass
    return f


def panel(bpm, secs, cad, geom):
    W, H = int(FRAME[0] * PX), int(FRAME[1] * PX)
    im = Image.new('RGB', (W, H), (0, 0, 0))
    d = ImageDraw.Draw(im)
    X = lambda x: W / 2 + x * PX
    Y = lambda y: H / 2 - y * PX
    d.rounded_rectangle([1, 1, W - 2, H - 2], radius=int(0.8 * PX), outline=(60, 72, 66), width=2)
    z = zone_of(bpm)
    for s in geom[str(z)]:
        v, idx, c = s['vertices'], s['indices'], s['color']
        col = tuple(int(255 * c[k]) for k in range(3))
        P = lambda k: (X(RING['cx'] + v[3 * k]), Y(RING['cy'] + v[3 * k + 1]))
        for t in range(0, len(idx), 3):
            d.polygon([P(idx[t]), P(idx[t + 1]), P(idx[t + 2])], fill=col)
    big = font('BricolageGrotesque.ttf', BPM['cap'], 'ExtraBold')
    d.text((X(BPM['x']), Y(BPM['y'])), str(bpm) if bpm > 0 else '--', font=big, fill=FOG if bpm > 0 else DIM, anchor='mm')
    mono = lambda cap: font('IBMPlexMono-Medium.ttf', cap)
    label = 'BPM · Z%d' % z if z else 'BPM'
    d.text((X(ZONE['x']), Y(ZONE['y'])), label, font=mono(ZONE['cap']), fill=MOSS, anchor='mm')
    row = '%02d:%02d' % (secs // 60, secs % 60) + ('   %d spm' % cad if cad > 0 else '') + '   max %d' % MAX_HR
    d.text((X(ROW['x']), Y(ROW['y'])), row, font=mono(ROW['cap']), fill=FOG, anchor='mm')
    d.text((X(STATUS['x']), Y(STATUS['y'])), '• Rhythm 24 5248 · 85%     • goggl1 · 104 ms', font=mono(STATUS['cap']), fill=MOSS, anchor='mm')
    return im


def main(out):
    zones = ['0', '1', '2', '3', '4', '5']
    geom = json.loads(subprocess.check_output(
        ['node', os.path.join(HERE, 'ring_geometry.js'), TS,
         str(RING['radius']), str(RING['thickness']), str(RING['extra'])] + zones))
    fake = [(0, 0, 0), (72, 95, 0), (118, 412, 152), (148, 1934, 172), (171, 2710, 181)]
    tiles = [panel(b, s, c, geom) for b, s, c in fake]
    pad = 12
    sheet = Image.new('RGB', (sum(t.width for t in tiles) + pad * (len(tiles) + 1), tiles[0].height + 2 * pad), (24, 24, 24))
    x = pad
    for t in tiles:
        sheet.paste(t, (x, pad)); x += t.width + pad
    sheet.save(out)
    print('wrote', out)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'pulse-preview.png')
