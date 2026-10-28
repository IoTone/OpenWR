#!/usr/bin/env python3
"""Generate the SpecsSwitchbackXR lens icon from the Switchback app's mark.

The phone app's icon (switchback/tool/gen_icon.py) is an Evergreen hairpin on
Fog with an Alpenglow dot at the turn. Lens icons are round, and the glasses
show light on clear glass, so here the mark is inverted: Fog strokes with a
soft glow on a deep Evergreen disc, faint topo contours behind, and a thin
Fog ring at the edge. The geometry is the phone app's, so the two read as one.

Rendered at 4x and downsampled, as the phone app's icon is.

    scratchpad/venv/bin/python tools/gen_icon.py      # writes icon.png (320 px)
"""
import math
import pathlib

from PIL import Image, ImageChops, ImageDraw, ImageFilter

SS = 4
OUT = 320                   # Lens Studio's project icon
S = OUT * SS

FOG = (243, 244, 241)       # #F3F4F1
EVERGREEN = (31, 77, 58)    # #1F4D3A
DEEP = (10, 30, 22)         # Evergreen at night, the disc's edge
ALPENGLOW = (224, 104, 63)  # #E0683F
GLACIER = (150, 205, 210)   # the glow under the strokes

# The phone app's geometry, pulled in a little so it sits inside the circle.
LOWER_LEFT = (0.18, 0.80)
TURN = (0.78, 0.62)
UPPER_END = (0.40, 0.20)
STROKE = 0.155
DOT = 0.07
INSET = 0.80                # the mark's scale about the centre
RING = 0.012                # ring width, of the edge


def px(p):
    return (((p[0] - 0.5) * INSET + 0.5) * S, ((p[1] - 0.5) * INSET + 0.5) * S)


def stroke(d, a, b, width, fill):
    """Line with round caps."""
    w = width * INSET * S
    d.line([px(a), px(b)], fill=fill, width=int(w))
    for p in (px(a), px(b)):
        d.ellipse([p[0] - w / 2, p[1] - w / 2, p[0] + w / 2, p[1] + w / 2], fill=fill)


def disc():
    """Deep Evergreen, lighter towards the upper left, with faint contours."""
    img = Image.new("RGB", (S, S))
    px_ = img.load()
    cx, cy = S * 0.38, S * 0.34
    for y in range(0, S, SS):
        for x in range(0, S, SS):
            t = min(1.0, math.hypot(x - cx, y - cy) / (S * 0.85))
            c = tuple(int(EVERGREEN[i] * (1 - t) + DEEP[i] * t) for i in range(3))
            for j in range(SS):
                for i in range(SS):
                    px_[x + i, y + j] = c
    # contours: a hill to the upper right, the trail climbing it
    d = ImageDraw.Draw(img)
    hx, hy = S * 0.70, S * 0.30
    for k in range(1, 12):
        r = S * 0.07 * k
        wob = [(hx + r * (1 + 0.08 * math.sin(3 * a + k)) * math.cos(a),
                hy + r * 0.78 * (1 + 0.08 * math.cos(2 * a - k)) * math.sin(a))
               for a in [i * 2 * math.pi / 180 for i in range(181)]]
        d.line(wob, fill=tuple(int(EVERGREEN[i] * 1.35) for i in range(3)), width=int(S * 0.004))
    return img


def main():
    out = pathlib.Path(__file__).resolve().parent.parent / "icon.png"

    img = disc()
    # glow: the mark in Glacier, blurred, added as light
    glow = Image.new("RGB", (S, S))
    stroke(ImageDraw.Draw(glow), LOWER_LEFT, TURN, STROKE * 1.25, GLACIER)
    stroke(ImageDraw.Draw(glow), TURN, UPPER_END, STROKE * 1.25, GLACIER)
    glow = glow.filter(ImageFilter.GaussianBlur(S * 0.035)).point(lambda v: int(v * 0.45))
    img = ImageChops.add(img, glow)

    d = ImageDraw.Draw(img)
    stroke(d, LOWER_LEFT, TURN, STROKE, FOG)
    stroke(d, TURN, UPPER_END, STROKE, FOG)
    r, c = DOT * INSET * S, px(TURN)
    d.ellipse([c[0] - r, c[1] - r, c[0] + r, c[1] + r], fill=ALPENGLOW)

    # round, with a thin Fog ring
    ring = RING * S
    d.ellipse([ring * 1.5, ring * 1.5, S - ring * 1.5, S - ring * 1.5], outline=FOG, width=int(ring))
    mask = Image.new("L", (S, S), 0)
    ImageDraw.Draw(mask).ellipse([0, 0, S - 1, S - 1], fill=255)
    icon = img.convert("RGBA")
    icon.putalpha(mask)
    icon.resize((OUT, OUT), Image.LANCZOS).save(out)
    print("wrote", out)


if __name__ == "__main__":
    main()
