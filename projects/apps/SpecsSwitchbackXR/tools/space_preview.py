"""
space_preview.py — the three panels side by side, off the glasses.

    python3 tools/space_preview.py docs/images/space-preview.png [frames.hex ...]

Remote (left), Pulse (centre), Roll (right), light on black as the additive
display shows them, with the lens's layout in cm. Roll frames are maplet
`roll-get` dumps (hex, one file each); without any, the Roll panel is left
out. Requires Pillow and Node (for the Pulse ring, via pulse_preview.py).
"""
import json, os, subprocess, sys
from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import pulse_preview as pp

GOGGLE = os.path.join(HERE, '..', 'Assets', 'Goggle')
PX = pp.PX
HIVIS, GLACIER, AMBER = (255, 210, 63), (120, 183, 212), (242, 163, 58)
PAPER, PAPER_SEL = (100, 66, 24), (160, 108, 38)


def mono(cap):
    return pp.font('IBMPlexMono-Medium.ttf', cap)


def frame_box(w_cm, h_cm):
    W, H = int(w_cm * PX), int(h_cm * PX)
    im = Image.new('RGB', (W, H), (0, 0, 0))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([1, 1, W - 2, H - 2], radius=int(0.8 * PX), outline=(60, 72, 66), width=2)
    return im, d, (lambda x: W / 2 + x * PX), (lambda y: H / 2 - y * PX)


def remote(selected='PULSE'):
    im, d, X, Y = frame_box(20, 15)
    w, h, gx, gy = 7.6, 3.8, 1.2, 1.9
    names = ['PULSE', 'STRIP', 'GLANCE', 'RIBBON']
    for i, n in enumerate(names):
        cx = (-1 if i % 2 == 0 else 1) * (w + gx) / 2
        cy = (1 if i < 2 else -1) * (h + gy) / 2 + 0.6
        on = n == selected
        if on:
            d.rectangle([X(cx - w / 2 - .25), Y(cy + h / 2 + .25), X(cx + w / 2 + .25), Y(cy - h / 2 - .25)], fill=HIVIS)
            d.rectangle([X(cx - w / 2), Y(cy + h / 2), X(cx + w / 2), Y(cy - h / 2)], fill=(0, 0, 0))
        card = Image.open(os.path.join(GOGGLE, 'goggle-%s.png' % n.lower())).convert('RGB')
        card = card.resize((int(w * PX), int(h * PX)), Image.LANCZOS)
        if not on:
            card = card.point(lambda v: int(v * 0.55))
        im.paste(card, (int(X(cx - w / 2)), int(Y(cy + h / 2))))
        d.text((X(cx), Y(cy - h / 2 - 0.6)), n, font=mono(0.5), fill=HIVIS if on else GLACIER, anchor='mm')
    d.text((X(0), Y(-6.4)), 'PINCH A CARD: GOGGL1 SHOWS IT', font=mono(0.42), fill=GLACIER, anchor='mm')
    return im


def unpack(b):
    w, h = {2808: (212, 104), 4736: (296, 128)}[len(b)]
    st = (w + 7) // 8
    return w, h, [[(b[y * st + x // 8] >> (7 - x % 8)) & 1 for x in range(w)] for y in range(h)]


def frame_img(b, paper, scale_w):
    w, h, bits = unpack(b)
    im = Image.new('RGB', (w, h))
    px = im.load()
    for y in range(h):
        for x in range(w):
            px[x, y] = (0, 0, 0) if bits[y][x] else paper
    return im.resize((int(scale_w * PX), int(scale_w * PX * h / w)), Image.NEAREST)


def roll(frames):
    im, d, X, Y = frame_box(20, 15)
    big = 16
    sel = frame_img(frames[0], PAPER_SEL, big)
    im.paste(sel, (int(X(-big / 2)), int(Y(1.6) - sel.height / 2)))
    tw = big / 3.4
    for i, b in enumerate(frames):
        t = frame_img(b, PAPER, tw * (1.12 if i == 0 else 1))
        x = -big / 2 + tw / 2 + i * (tw + 0.5)
        im.paste(t, (int(X(x) - t.width / 2), int(Y(-4.6) - t.height / 2)))
    d.text((X(0), Y(6.4)), 'ROLL · MAPLET', font=mono(0.5), fill=AMBER, anchor='mm')
    w, h, _ = unpack(frames[0])
    d.text((X(0), Y(-6.6)), 'FRAME 0002 · %d×%d · %d ON THE CARD' % (w, h, len(frames)), font=mono(0.42), fill=AMBER, anchor='mm')
    return im


def main(out, hexfiles):
    zones = ['0', '1', '2', '3', '4', '5']
    geom = json.loads(subprocess.check_output(['node', os.path.join(HERE, 'ring_geometry.js'), pp.TS,
                                               str(pp.RING['radius']), str(pp.RING['thickness']), str(pp.RING['extra'])] + zones))
    panels = [remote(), pp.panel(148, 1934, 172, geom)]
    if hexfiles:
        frames = [bytes.fromhex(open(f).read().strip()) for f in hexfiles]
        panels.append(roll(frames[::-1]))
    pad = 16
    sheet = Image.new('RGB', (sum(p.width for p in panels) + pad * (len(panels) + 1), panels[0].height + 2 * pad), (24, 24, 24))
    x = pad
    for p in panels:
        sheet.paste(p, (x, pad)); x += p.width + pad
    sheet.save(out)
    print('wrote', out)


if __name__ == '__main__':
    main(sys.argv[1] if len(sys.argv) > 1 else 'space-preview.png', sys.argv[2:])
