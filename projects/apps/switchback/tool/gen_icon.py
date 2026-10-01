#!/usr/bin/env python3
"""Generate the Switchback app icon: a trail switchback on a topo map.

The mark is two bold strokes of trail. The first climbs from lower-left to a
turn point; the second climbs from that turn back up and over it, forming a sharp
zigzag — a hairpin on a topo map. Round caps, an Alpenglow dot at the turn.

Palette: Evergreen #1F4D3A on Fog #F3F4F1, Alpenglow #E0683F accent.

Rendered at 4× and downsampled so the strokes are smooth without a vector
rasteriser. Nothing in the mark is thinner than a stroke, so it still reads
at 16 px.

    python3 tool/gen_icon.py            # writes assets/images/*.png
"""
import pathlib

from PIL import Image, ImageDraw

SS = 4                      # supersample factor
OUT = 1024                  # master icon edge
S = OUT * SS

FOG = (243, 244, 241)       # #F3F4F1
EVERGREEN = (31, 77, 58)    # #1F4D3A
ALPENGLOW = (224, 104, 63)  # #E0683F

# Geometry, as fractions of the canvas edge. The mark stays inside ~80% of
# the canvas so rounded-corner and maskable masks never clip it.
LOWER_LEFT = (0.18, 0.80)
TURN = (0.78, 0.62)
UPPER_END = (0.40, 0.20)  # back over the turn: a true hairpin
STROKE = 0.155              # stroke width
DOT = 0.07                  # accent dot radius


def px(p):
    return (p[0] * S, p[1] * S)


def rounded_square(size, radius, fill):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(img).rounded_rectangle([0, 0, size - 1, size - 1],
                                          radius=radius, fill=fill)
    return img


def stroke(d, a, b, width, fill):
    """Line with round caps."""
    d.line([px(a), px(b)], fill=fill, width=int(width * S))
    r = width * S / 2
    for p in (px(a), px(b)):
        d.ellipse([p[0] - r, p[1] - r, p[0] + r, p[1] + r], fill=fill)


def mark(canvas):
    d = ImageDraw.Draw(canvas)
    stroke(d, LOWER_LEFT, TURN, STROKE, EVERGREEN)
    stroke(d, TURN, UPPER_END, STROKE, EVERGREEN)
    r = DOT * S
    c = px(TURN)
    d.ellipse([c[0] - r, c[1] - r, c[0] + r, c[1] + r], fill=ALPENGLOW)
    return canvas


def main():
    assets = pathlib.Path(__file__).resolve().parent.parent / "assets" / "images"
    assets.mkdir(parents=True, exist_ok=True)

    # Icon: Fog rounded square with the mark.
    icon = mark(rounded_square(S, int(S * 0.22), FOG + (255,)))
    icon_1024 = icon.resize((OUT, OUT), Image.LANCZOS)
    icon_1024.save(assets / "switchback-icon-1024.png")
    icon.resize((512, 512), Image.LANCZOS).save(assets / "switchback-icon-512.png")
    icon.resize((192, 192), Image.LANCZOS).save(assets / "icon-192.png")

    # Splash: the same mark on transparency, for flutter_native_splash.
    splash = mark(Image.new("RGBA", (S, S), (0, 0, 0, 0)))
    splash.resize((OUT, OUT), Image.LANCZOS).save(assets / "switchback-splash.png")

    print("wrote:", ", ".join(sorted(
        p.name for p in assets.glob("*.png") if p.is_file())))


if __name__ == "__main__":
    main()
