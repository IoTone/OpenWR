"""NFCTapInXR promo, picture: Blender 5.0 video-sequencer edit -> renders/frames/f_####.png.

  Blender -b --factory-startup --python render.py              # all 900 frames
  Blender -b --factory-startup --python render.py -- 50 340     # test stills only
Then run promo.py to add the music and SFX and write the MP4.
"""
import bpy, sys, os

HERE = os.path.dirname(os.path.abspath(__file__))
A = os.path.join(HERE, "assets")
OUT = os.path.join(HERE, "renders")
TEST = [int(x) for x in sys.argv[sys.argv.index("--") + 1:]] if "--" in sys.argv else []

FPS, W, H = 30, 1920, 1080
F = lambda s: int(round(s * FPS)) + 1          # seconds -> frame

sc = bpy.context.scene
sc.render.resolution_x, sc.render.resolution_y, sc.render.resolution_percentage = W, H, 100
sc.render.fps = FPS
sc.frame_start, sc.frame_end = 1, F(30) - 1
sc.view_settings.view_transform = "Standard"
sc.view_settings.look = "None"
sc.render.use_sequencer = True
sc.render.image_settings.file_format = "PNG"
sc.render.image_settings.color_mode = "RGB"
se = sc.sequence_editor_create()
S = se.strips

DIN = bpy.data.fonts.load("/System/Library/Fonts/Supplemental/DIN Alternate Bold.ttf")
SANS = bpy.data.fonts.load("/System/Library/Fonts/SFNS.ttf")
MONO = bpy.data.fonts.load("/System/Library/Fonts/SFNSMono.ttf")

CYAN = (0.31, 0.95, 1.0, 1)
WHITE = (0.95, 0.98, 1.0, 1)
SOFT = (0.72, 0.80, 0.90, 1)
PINK = (1.0, 0.31, 0.85, 1)

def fade(strip, start, end, n=6):
    strip.blend_alpha = 0.0; strip.keyframe_insert("blend_alpha", frame=start)
    strip.blend_alpha = 1.0; strip.keyframe_insert("blend_alpha", frame=start + n)
    strip.blend_alpha = 1.0; strip.keyframe_insert("blend_alpha", frame=end - n)
    strip.blend_alpha = 0.0; strip.keyframe_insert("blend_alpha", frame=end)

def image(name, path, ch, t0, t1, scale=1.0, dx=0, dy=0, fades=True):
    s = S.new_image(name=name, filepath=os.path.join(A, path), channel=ch, frame_start=F(t0), fit_method="ORIGINAL")
    s.frame_final_end = F(t1)
    s.blend_type = "ALPHA_OVER"
    s.transform.scale_x = s.transform.scale_y = scale
    s.transform.offset_x, s.transform.offset_y = dx, dy
    if fades: fade(s, F(t0), F(t1))
    return s

def movie(name, path, ch, t0):
    s = S.new_movie(name=name, filepath=os.path.join(A, path), channel=ch, frame_start=F(t0), fit_method="ORIGINAL")
    s.transform.offset_x = -240                 # 1440x1080 in the left box
    fade(s, s.frame_final_start, s.frame_final_end, 5)
    return s

def text(name, body, ch, t0, t1, x, y, size, font, color, align="LEFT", anchor_x="LEFT", wrap=0.21, fades=True):
    s = S.new_effect(name=name, type="TEXT", channel=ch, frame_start=F(t0), length=F(t1) - F(t0))
    s.text = body
    s.font = font
    s.font_size = size
    s.color = color
    s.location = (x, y)
    s.alignment_x = align
    s.anchor_x = anchor_x
    s.anchor_y = "TOP"
    s.wrap_width = wrap
    s.blend_type = "ALPHA_OVER"
    if fades: fade(s, F(t0), F(t1), 7)
    return s

# Background for the whole film.
bg = image("bg", "bg.png", 1, 0, 30, fades=False)

# ---- Title, 0 - 2.5 s
icon = image("icon_title", "icon.png", 2, 0, 2.5, scale=0.5, dy=150)
for f, sc_ in ((F(0), 0.38), (F(0.6), 0.52), (F(2.5), 0.55)):
    icon.transform.scale_x = icon.transform.scale_y = sc_
    icon.transform.keyframe_insert("scale_x", frame=f); icon.transform.keyframe_insert("scale_y", frame=f)
text("title", "NFCTapInXR", 3, 0.15, 2.5, 0.5, 0.36, 132, DIN, WHITE, "CENTER", "CENTER", 0.9)
text("tagline", "TAP A TAG.  SEE IT IN XR.", 4, 0.45, 2.5, 0.5, 0.215, 38, MONO, CYAN, "CENTER", "CENTER", 0.9)

# ---- Panel sections (left: media, right: caption)
PX = 1478 / W                                   # panel text left edge
SECTIONS = [
    # t0, t1, label, title, body
    (2.5, 4.9, "THE TAGS", "Four NFC tags", "Stickers and cards. The lens only needs each tag's UID."),
    (4.9, 6.5, "ATOM MATRIX + PN532", "The reader", "An M5 ATOM Matrix (ESP32) reads each tap and sends the UID over Bluetooth LE."),
    (6.5, 9.5, "SPECTACLES", "Connect + tap", "The lens finds the reader over BLE. Hold a tag to the reader."),
    (9.5, 13.5, "TAG 1 / 4", "3D model", "A model materializes in the room."),
    (13.5, 17.5, "TAG 2 / 4", "Transit fare", "$2.75 is taken and the card balance counts down."),
    (17.5, 22.0, "TAG 3 / 4", "Sign in", "“Signing in as Iron Man”, then the session is secured."),
    (22.0, 26.0, "TAG 4 / 4", "Trading card", "A collectible card you can pinch and hold."),
]
for i, (t0, t1, label, title, body) in enumerate(SECTIONS):
    text("lab%d" % i, label, 5, t0, t1, PX, 0.80, 30, MONO, PINK if label.startswith("TAG") else CYAN)
    text("tit%d" % i, title, 6, t0, t1, PX, 0.735, 70, DIN, WHITE)
    text("bod%d" % i, body, 7, t0, t1, PX, 0.62, 34, SANS, SOFT)

# The media for those sections.
image("cards", "cards.png", 2, 2.5, 4.9, scale=0.80, dx=-240)
image("wiring", "wiring.png", 2, 4.9, 6.5, scale=1.08, dx=-240)
for t0, path in ((6.5, "s1_tap.mp4"), (9.5, "s2_model.mp4"), (13.5, "s3_fare.mp4"), (17.5, "s4_signin.mp4"), (22.0, "s5_card.mp4")):
    movie(path, path, 2, t0)

# Divider and small brand while the panel is up.
image("divider", "divider.png", 8, 2.5, 26.0)
text("brand", "NFCTapInXR", 9, 2.5, 26.0, PX + 0.05, 0.115, 28, MONO, SOFT, wrap=0.3)
image("icon_small", "icon.png", 10, 2.5, 26.0, scale=0.13, dx=1478 + 24 - 960, dy=-540 + 100)

# ---- Outro, 26 - 30 s: name, a neon OPEN SOURCE, and where the code is
CODE_HOST = "github.com/IoTone/OpenWR"
CODE_PATH = "tree/dev/projects/apps/NFCTapInXR"
image("icon_out", "icon.png", 2, 26.0, 30.0, scale=0.34, dy=250)
text("o_title", "NFCTapInXR", 3, 26.15, 30.0, 0.5, 0.60, 104, DIN, WHITE, "CENTER", "CENTER", 0.9)

def neon(name, body, ch, t0, t1, y, size, core, glow):
    """Text with two blurred, unshifted shadows under it (the glow), and a
    neon-tube flicker as it lights."""
    strips = []
    for k, (blur, alpha) in enumerate(((1.0, 1.0), (0.45, 1.0))):
        s = text("%s_g%d" % (name, k), body, ch + k, t0, t1, 0.5, y, size, DIN, glow[:3] + (1.0,), "CENTER", "CENTER", 0.9, fades=False)
        s.use_shadow = True
        s.shadow_color = glow[:3] + (alpha,)
        s.shadow_offset = 0.0
        s.shadow_blur = blur
        strips.append(s)
    c = text(name, body, ch + 3, t0, t1, 0.5, y, size, DIN, core, "CENTER", "CENTER", 0.9, fades=False)
    c.use_outline = True
    c.outline_color = glow[:3] + (1.0,)
    c.outline_width = 0.04
    # Wide bloom: Blender's Glow effect on the pink layer, glow only, added
    # under the white-hot core.
    g = S.new_effect(name=name + "_bloom", type="GLOW", channel=ch + 2, frame_start=F(t0), length=F(t1) - F(t0), input1=strips[1])
    g.threshold = 0.05
    g.blur_radius = 36
    g.boost_factor = 2.2
    g.quality = 6
    g.use_only_boost = True
    g.blend_type = "ADD"
    strips += [g, c]
    f0 = F(t0)
    edit = bpy.context.preferences.edit
    for s in strips:                           # off, flick, off, flick, on ... fade out
        for df, a in ((0, 0), (2, 1), (4, 0.15), (7, 1), (9, 0.35), (12, 1), (F(t1) - f0 - 8, 1), (F(t1) - f0, 0)):
            edit.keyframe_new_interpolation_type = "CONSTANT" if df < 12 else "LINEAR"
            s.blend_alpha = a
            s.keyframe_insert("blend_alpha", frame=f0 + df)
    edit.keyframe_new_interpolation_type = "BEZIER"
    return strips

neon("o_open", "OPEN SOURCE", 4, 26.6, 30.0, 0.46, 78, (1.0, 0.86, 0.97, 1), (1.0, 0.31, 0.85))
text("o_url", CODE_HOST, 8, 27.0, 30.0, 0.5, 0.335, 46, MONO, CYAN, "CENTER", "CENTER", 0.9)
text("o_path", CODE_PATH, 9, 27.1, 30.0, 0.5, 0.268, 28, MONO, SOFT, "CENTER", "CENTER", 0.9)
text("o_line", "Snap Spectacles  \u00b7  Lens Studio  \u00b7  M5 ATOM Matrix  \u00b7  PN532", 10, 27.3, 30.0, 0.5, 0.15, 28, SANS, SOFT, "CENTER", "CENTER", 0.9)

bpy.ops.wm.save_as_mainfile(filepath=os.path.join(HERE, "NFCTapInXR_Promo.blend"))

if TEST:
    for f in TEST:
        sc.frame_set(f)
        sc.render.filepath = os.path.join(OUT, "test_%04d.png" % f)
        bpy.ops.render.render(write_still=True)
else:
    sc.render.filepath = os.path.join(OUT, "frames", "f_")
    bpy.ops.render.render(animation=True)
