"""Blender renders of the five SubverseReader variants (front and back in one frame).

    /Applications/Blender.app/Contents/MacOS/Blender -b -P make_renders.py -- [SR-1 ...] [--samples N]

Writes out/<variant>.png. Every model is built from the numbers in VARIANTS,
in centimetres, then scaled to metres for lighting. Screen content is real
Blender text at the panel's active area, so the page proportions are right.
"""
import math
import os
import sys

import bmesh
import bpy
from mathutils import Vector

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out")
FONTS = "/System/Library/Fonts/Supplemental/"

# --------------------------------------------------------------------------- content
ZINE = {
    "basement": ("How to Run a Basement Show",
                 "Sweep the floor before anyone sees it. Tape the setlist to the water heater. "
                 "Somebody has to stand at the door with a coffee can and a smile, and it should be "
                 "the person who knows every band's mother by name. Count the room twice: once for "
                 "the landlord, once for the joy of it.\n\nThe PA is two speakers and a promise. "
                 "Point them at the wall, not the neighbours. When the bass player blows a fuse, "
                 "the show is not over; it is unplugged. Pass the hat, then pass the mop."),
    "loadin": ("Notes From the Load-In",
               "Four in the afternoon and the venue smells like last night. We carry the cabs up "
               "the alley stairs because the freight lift has been broken since the first tour, "
               "and the stage manager writes our names on gaffer tape in silver pen.\n\n"
               "Every pass on every lanyard is a small permission: this hallway, that door, the "
               "green room with the sad fruit plate. The zine table goes by the bar, where the "
               "light is good and the floor is sticky. We sell out of issue six before doors.\n\n"
               "Soundcheck is the only quiet hour. The drummer reads, the singer sleeps on a "
               "road case, and the merch kid counts shirts by size in a whisper that sounds like "
               "a prayer. Then the house lights drop, and nobody reads anything for two hours."),
    "copier": ("Photocopier Love Letters",
               "The copy shop on Fifth stays open until two. We pay in quarters and make the "
               "toner do things it was never sold to do: grey skies out of black, a whole crowd "
               "out of a single blurred hand.\n\nFold, staple, trim. A zine is a letter written "
               "to whoever finds it."),
    "drive": ("The Long Drive Home",
              "After the last show of the run we drive through the night with the windows down "
              "to stay awake. The van has one working speaker and a tape deck that eats every "
              "third cassette, so we take turns reading aloud instead: liner notes, gas station "
              "maps, the back of a cereal box, and finally the zine somebody handed us at the "
              "merch table in a city none of us can name anymore.\n\n"
              "It is twelve pages of xeroxed photographs and handwriting about a skate park "
              "that was paved over. Nobody says much after it. The sky goes from black to the "
              "colour of a bruise to the colour of a peach, and the bassist, who has not slept, "
              "says that is the whole point of the thing: someone made it, someone carried it, "
              "someone else read it out loud on a highway at five in the morning.\n\n"
              "We get home at nine. The first thing I do is staple my own."),
    "nightbus": ("Reading on the Night Bus",
                 "The 4 a.m. bus has three passengers and one working light, and the light is "
                 "over the driver. So the pass glows a little, warm and low, enough for the page "
                 "and not enough to wake the man asleep against the window.\n\nWe wrote this "
                 "issue for the ride home: short pieces, one per stop, each finished before the "
                 "doors open again."),
    "quiet": ("No Signal",
              "There is no Wi-Fi in this thing. Not off; absent. It cannot be asked where it "
              "is, what you read, or for how long. It takes pages over a short Bluetooth hop "
              "or a USB cable, and then it keeps quiet.\n\nThat makes it welcome in places "
              "radios are not: the reading room, the ward, the flight, the classroom. It "
              "also makes the battery last a very long time."),
    "pan": ("Flyer Wall, Zoomed",
            "This page is a scan of the wall outside the Corner Bar: forty flyers stapled over "
            "each other since 1994. On a small screen it is mush, so the pass keeps it at twice "
            "the panel's resolution and the thumbstick moves a window around it. Push and the "
            "view slides; the little map in the corner shows where you are.\n\nZines are "
            "made for paper, and paper does not reflow. Panning keeps the layout the maker "
            "chose instead of tearing it into columns."),
    "stage": ("Stage Plot / Set One",
              "1. Doors (instrumental, cue lights on the snare hit)\n2. Basement Saint\n"
              "3. Night Bus to Nowhere\n4. Copy Shop  (capo 2)\n5. Long Drive Home\n"
              "6. Encore: whatever the room wants\n\nNotes\n- Bass DI stage left, two wedges\n"
              "- House kills haze after song 3\n- Merch closes at 11:40\n- Load-out by the alley door"),
}

MORE = ("\n\nWe print a hundred copies and leave them in laundromats, record shops and the "
        "back booth of the diner where the night cook draws comics on the order tickets. "
        "Nobody tracks where they go. That is the deal: you make the thing, you let it walk "
        "away, and sometimes it comes back years later with somebody else's handwriting in "
        "the margins and a ticket stub for a show you also played.\n\nThis issue was laid out "
        "on a borrowed laptop and proofed on the bus. Mistakes are ours. Copy it, pass it on, "
        "read it on whatever you have: paper, a screen, or a little grey pass on a lanyard "
        "that holds the page for a week without asking for a charger.")

# --------------------------------------------------------------------------- variants
# Dimensions in cm. aa = panel active area (w, h) as mounted (portrait unless noted).
VARIANTS = {
    "SR-1": dict(name="Laminate", band="GUEST", sd=False, aa=(6.36, 8.48), T=1.25, W=8.6,
                 body=(0.85, 0.30, 0.07), band_col=(0.02, 0.02, 0.02), band_txt=(1, 1, 1),
                 strap=(0.03, 0.03, 0.03), controls="three", ctrl_h=1.5, zine="basement",
                 body_size=0.27, gray=False, pocket=False, clip=False, lanyard=True,
                 spec="ESP32-C5 · 4.2in 400x300"),
    "SR-2": dict(name="All Access", band="ALL ACCESS", aa=(8.82, 11.88), T=1.55, W=10.9,
                 body=(0.035, 0.035, 0.04), band_col=(0.98, 0.72, 0.02), band_txt=(0, 0, 0),
                 strap=(0.95, 0.68, 0.02), controls="dpad", ctrl_h=1.9, zine="loadin",
                 body_size=0.25, gray=False, pocket=True, clip=False, lanyard=True,
                 card=(0.02, 0.32, 0.36), card_txt="ZINE CLUB",
                 spec="ESP32-S3 · 5.83in 648x480"),
    "SR-3": dict(name="Gray Pass", band="PHOTO PIT", aa=(5.83, 10.37), T=1.45, W=7.8,
                 body=(0.02, 0.33, 0.36), band_col=(0.92, 0.92, 0.9), band_txt=(0.02, 0.02, 0.02),
                 strap=(0.25, 0.25, 0.27), controls="side3", ctrl_h=0.7, zine="copier",
                 body_size=0.24, gray=True, pocket=False, clip=False, lanyard=True,
                 spec="M5Stack PaperDIY S3 · 4.7in 960x540 16-grey"),
    "SR-4": dict(name="Headliner", band="BACKSTAGE", aa=(9.06, 12.24), T=1.35, W=10.9,
                 body=(0.80, 0.76, 0.66), band_col=(0.62, 0.04, 0.04), band_txt=(1, 1, 1),
                 strap=(0.62, 0.04, 0.04), controls="rockers", ctrl_h=1.1, zine="drive",
                 body_size=0.235, gray=True, pocket=True, clip=False, lanyard=True,
                 card=(0.03, 0.03, 0.03), card_txt="ADMIT ONE",
                 spec="ESP32-S3 + epdiy · 6in 1448x1072 300ppi"),
    "SR-5": dict(name="Crew Clip", band="CREW", aa=(8.48, 6.36), T=1.6, W=10.4,
                 body=(0.18, 0.20, 0.12), band_col=(1.0, 0.33, 0.02), band_txt=(0, 0, 0),
                 strap=None, controls="wheel", ctrl_h=1.4, zine="stage",
                 body_size=0.30, gray=False, pocket=False, clip=True, lanyard=False,
                 spec="ESP32-C5 · 4.2in 400x300 landscape"),
    "SR-6": dict(name="Night Shift", band="AFTER HOURS", aa=(6.36, 8.48), T=1.45, W=8.8,
                 body=(0.015, 0.025, 0.07), band_col=(0.95, 0.55, 0.12), band_txt=(0, 0, 0),
                 strap=(0.02, 0.04, 0.12), controls="four", ctrl_h=1.5, zine="nightbus",
                 body_size=0.27, gray=False, pocket=False, clip=False, lanyard=True, lit=True,
                 spec="ESP32-C5 · 4.2in 400x300 front light"),
    "SR-7": dict(name="Poster", band="VIP", aa=(8.82, 11.88), T=1.55, W=10.9,
                 body=(0.50, 0.035, 0.03), band_col=(0.02, 0.02, 0.02), band_txt=(0.95, 0.75, 0.05),
                 strap=(0.02, 0.02, 0.02), controls="dpad", ctrl_h=1.9, zine="poster",
                 body_size=0.25, gray=False, pocket=True, clip=False, lanyard=True, color=True,
                 pocket_item="zine", card=(0.88, 0.84, 0.70), card_txt="MINI ZINE #7",
                 spec="ESP32-S3 · 5.83in 648x480 4-colour"),
    "SR-8": dict(name="Quiet Pass", band="OFFLINE", aa=(6.36, 8.48), T=1.3, W=8.6,
                 body=(0.06, 0.03, 0.11), band_col=(0.93, 0.93, 0.9), band_txt=(0.02, 0.02, 0.02),
                 strap=(0.06, 0.03, 0.11), controls="three", ctrl_h=1.5, zine="quiet",
                 body_size=0.27, gray=False, pocket=False, clip=False, lanyard=True,
                 spec="nRF52840 · no Wi-Fi · 4.2in 400x300"),
    "SR-9": dict(name="Roadie", band="ROADIE", aa=(6.36, 8.48), T=1.45, W=8.8,
                 body=(0.025, 0.19, 0.07), band_col=(0.02, 0.02, 0.02), band_txt=(0.95, 0.95, 0.9),
                 strap=(0.02, 0.02, 0.02), controls="stick", ctrl_h=2.6, zine="pan",
                 body_size=0.27, gray=False, pocket=False, clip=False, lanyard=True, pan=True,
                 spec="ESP32-C5 · 4.2in 400x300 · thumbstick"),
}

# --------------------------------------------------------------------------- helpers
_fonts = {}


def font(name):
    if name not in _fonts:
        _fonts[name] = bpy.data.fonts.load(os.path.join(FONTS, name), check_existing=True)
    return _fonts[name]


_mats = {}


def mat(key, color, rough=0.5, metal=0.0, emit=0.0):
    k = (key, tuple(color), rough, metal)
    if k in _mats:
        return _mats[k]
    m = bpy.data.materials.new(key)
    m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = (*color, 1)
    b.inputs["Roughness"].default_value = rough
    b.inputs["Metallic"].default_value = metal
    _mats[k] = m
    return m


def eink_mat():
    return mat("eink", (0.70, 0.69, 0.64), rough=0.9)


def lit_mat():
    """Front-lit e-paper: the light guide makes the white warm and faintly glowing."""
    m = mat("eink_lit", (0.74, 0.70, 0.60), rough=0.9)
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Emission Color"].default_value = (1.0, 0.78, 0.50, 1)
    b.inputs["Emission Strength"].default_value = 1.6
    return m


def ink_mat():
    return mat("ink", (0.035, 0.035, 0.04), rough=0.8)


def photo_mat(seed):
    """Procedural grey 'photograph' for the 16-grey panels."""
    m = bpy.data.materials.new(f"photo{seed}")
    m.use_nodes = True
    nt = m.node_tree
    b = nt.nodes["Principled BSDF"]
    b.inputs["Roughness"].default_value = 0.9
    tc = nt.nodes.new("ShaderNodeTexCoord")
    noise = nt.nodes.new("ShaderNodeTexNoise")
    noise.inputs["Scale"].default_value = 3.0
    noise.inputs["Detail"].default_value = 8.0
    noise.noise_dimensions = "4D"
    next(i for i in noise.inputs if i.name == "W").default_value = seed
    grad = nt.nodes.new("ShaderNodeSeparateXYZ")
    mix = nt.nodes.new("ShaderNodeMath")
    mix.operation = "MULTIPLY_ADD"
    mix.inputs[1].default_value = 1.1
    ramp = nt.nodes.new("ShaderNodeValToRGB")
    ramp.color_ramp.elements[0].position = 0.55
    ramp.color_ramp.elements[0].color = (0.03, 0.03, 0.035, 1)
    ramp.color_ramp.elements[1].position = 1.25
    ramp.color_ramp.elements[1].color = (0.66, 0.65, 0.60, 1)
    nt.links.new(tc.outputs["Generated"], noise.inputs["Vector"])
    nt.links.new(tc.outputs["Generated"], grad.inputs[0])
    nt.links.new(noise.outputs["Fac"], mix.inputs[0])
    nt.links.new(grad.outputs["Z"], mix.inputs[2])
    nt.links.new(mix.outputs[0], ramp.inputs["Fac"])
    nt.links.new(ramp.outputs["Color"], b.inputs["Base Color"])
    return m


def link(obj, parent):
    bpy.context.scene.collection.objects.link(obj)
    obj.parent = parent
    return obj


def box(name, size, loc, parent, material=None, bevel=0.0, segs=4):
    me = bpy.data.meshes.new(name)
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co = Vector((v.co.x * size[0], v.co.y * size[1], v.co.z * size[2]))
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    ob.location = loc
    link(ob, parent)
    if material:
        me.materials.append(material)
    if bevel > 0:
        md = ob.modifiers.new("bevel", "BEVEL")
        md.width = bevel
        md.segments = segs
        md.limit_method = "ANGLE"
    return ob


def cyl(name, r, depth, loc, parent, material=None, axis="Y", verts=48, bevel=0.0):
    me = bpy.data.meshes.new(name)
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=True, segments=verts, radius1=r, radius2=r, depth=depth)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    ob.location = loc
    if axis == "Y":
        ob.rotation_euler = (math.radians(90), 0, 0)
    elif axis == "X":
        ob.rotation_euler = (0, math.radians(90), 0)
    link(ob, parent)
    if material:
        me.materials.append(material)
    if bevel > 0:
        md = ob.modifiers.new("bevel", "BEVEL")
        md.width = bevel
        md.segments = 3
        md.limit_method = "ANGLE"
    return ob


def cut(target, cutter):
    cutter.hide_render = True
    cutter.display_type = "WIRE"
    md = target.modifiers.new("cut", "BOOLEAN")
    md.operation = "DIFFERENCE"
    md.object = cutter


def text(body, loc, parent, size, fnt="Georgia.ttf", material=None, width=0.0, height=0.0,
         align="LEFT", valign="TOP", spacing=1.0, back=False):
    cu = bpy.data.curves.new("txt", "FONT")
    cu.body = body
    cu.font = font(fnt)
    cu.size = size
    cu.space_line = spacing
    cu.align_x = align
    cu.align_y = valign
    if width:
        cu.text_boxes[0].width = width
        if height:
            cu.text_boxes[0].height = height
            cu.overflow = "TRUNCATE"
    ob = bpy.data.objects.new("txt", cu)
    ob.location = loc
    ob.rotation_euler = (math.radians(90), 0, math.radians(180) if back else 0)
    link(ob, parent)
    cu.materials.append(material or ink_mat())
    return ob


def plane(name, w, h, loc, parent, material, back=False):
    """Plane in the XZ plane facing -Y (front) or +Y (back)."""
    return box(name, (w, 0.002, h), loc, parent, material)


def ribbon(name, pts, width, thick, parent, material):
    me = bpy.data.meshes.new(name)
    bm = bmesh.new()
    rows = []
    for i, p in enumerate(pts):
        t = (pts[min(i + 1, len(pts) - 1)] - pts[max(i - 1, 0)]).normalized()
        side = t.cross(Vector((0, 1, 0))).normalized() * (width / 2)
        rows.append((bm.verts.new(p - side), bm.verts.new(p + side)))
    for a, b in zip(rows, rows[1:]):
        bm.faces.new((a[0], a[1], b[1], b[0]))
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    link(ob, parent)
    me.materials.append(material)
    md = ob.modifiers.new("solid", "SOLIDIFY")
    md.thickness = thick
    for poly in me.polygons:
        poly.use_smooth = True
    return ob


def qbez(p0, p1, p2, n=48):
    return [p0 * (1 - t) ** 2 + p1 * 2 * (1 - t) * t + p2 * t * t
            for t in (i / (n - 1) for i in range(n))]


def knurl_wheel(name, r, depth, loc, parent, material, teeth=36):
    me = bpy.data.meshes.new(name)
    bm = bmesh.new()
    ring = []
    for i in range(teeth * 2):
        a = math.pi * 2 * i / (teeth * 2)
        rr = r if i % 2 == 0 else r * 0.93
        ring.append(bm.verts.new((rr * math.cos(a), rr * math.sin(a), -depth / 2)))
    f = bm.faces.new(ring)
    ext = bmesh.ops.extrude_face_region(bm, geom=[f])
    for v in [e for e in ext["geom"] if isinstance(e, bmesh.types.BMVert)]:
        v.co.z += depth
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    ob.location = loc
    ob.rotation_euler = (math.radians(90), 0, 0)
    link(ob, parent)
    me.materials.append(material)
    return ob


# --------------------------------------------------------------------------- device
def layout(v):
    """Vertical stack from the top: slot, band, screen, controls. Returns H and z centres."""
    slot_h = 0.45 if v["lanyard"] else 0.0
    top = 0.55
    band_h = 1.05 if v["W"] > 9 else 0.95
    scr_h = v["aa"][1] + 0.55
    gaps = 0.28
    H = top + slot_h + (gaps if slot_h else 0) + band_h + gaps + scr_h + gaps + v["ctrl_h"] + 0.45
    z = H / 2 - top
    z_slot = z - slot_h / 2
    z -= slot_h + (gaps if slot_h else 0)
    z_band = z - band_h / 2
    z -= band_h + gaps
    z_scr = z - scr_h / 2
    z -= scr_h + gaps
    z_ctrl = z - v["ctrl_h"] / 2
    return H, dict(slot=z_slot, slot_h=slot_h, band=z_band, band_h=band_h, scr=z_scr,
                   scr_h=scr_h, ctrl=z_ctrl)


def build_device(key, v, root):
    W, T = v["W"], v["T"]
    H, L = layout(v)
    yf, yb = -T / 2, T / 2
    body_m = mat("body" + key, v["body"], rough=0.42)
    rubber = mat("rubber", (0.015, 0.015, 0.017), rough=0.75)
    metal = mat("metal", (0.8, 0.8, 0.82), rough=0.25, metal=1.0)

    body = box("body", (W, T, H), (0, 0, 0), root, body_m, bevel=0.32, segs=10)

    # screen recess and panel
    aw, ah = v["aa"]
    rec = box("rec_cut", (aw + 0.55, 0.5, L["scr_h"]), (0, yf - 0.25 + 0.13, L["scr"]), root)
    cut(body, rec)
    ys = yf + 0.13 - 0.004
    box("panel", (aw + 0.5, 0.004, L["scr_h"] - 0.05), (0, ys + 0.002, L["scr"]), root,
        lit_mat() if v.get("lit") else eink_mat())
    screen_content(v, (0, ys - 0.0005, L["scr"]), root)

    # laminate band
    bw = W - 0.9
    zb = L["band"]
    box("band", (bw, 0.01, L["band_h"]), (0, yf - 0.002, zb), root,
        mat("band" + key, v["band_col"], rough=0.35), bevel=0.0)
    tm = mat("bandtxt" + key, v["band_txt"], rough=0.4)
    text("SUBVERSE", (-bw / 2 + 0.3, yf - 0.009, zb), root, L["band_h"] * 0.72,
         "DIN Condensed Bold.ttf", tm, valign="CENTER")
    text(v["band"], (bw / 2 - 0.3, yf - 0.009, zb + 0.02), root, L["band_h"] * 0.42,
         "DIN Alternate Bold.ttf", tm, align="RIGHT", valign="CENTER")

    # lanyard slot, ring, swivel, strap
    if v["lanyard"]:
        R = max(0.75, T / 2 + 0.25)
        sag = R - math.sqrt(R * R - (T / 2) ** 2)
        sh = sag + 0.16
        slot = box("slot_cut", (1.8, T + 1, sh), (0, 0, L["slot"]), root, bevel=0.08)
        cut(body, slot)
        cz = L["slot"] - sh / 2 + 0.07 + R
        bpy.ops.mesh.primitive_torus_add(major_radius=R, minor_radius=0.055,
                                         major_segments=64, minor_segments=12)
        tor = bpy.context.active_object
        tor.location = (0, 0, cz)
        tor.rotation_euler = (0, math.radians(90), 0)
        tor.parent = root
        tor.data.materials.append(metal)
        for p in tor.data.polygons:
            p.use_smooth = True
        ztop = cz + R
        sw = cyl("swivel", 0.2, 1.0, (0, 0, ztop + 0.45), root, metal, axis="Z", bevel=0.05)
        for p in sw.data.polygons:
            p.use_smooth = True
        sm = mat("strap" + key, v["strap"], rough=0.8)
        box("crimp", (1.7, 0.32, 0.8), (0, 0, ztop + 1.2), root,
            mat("crimp", (0.05, 0.05, 0.05), rough=0.4, metal=0.6), bevel=0.06)
        for sgn in (-1, 1):
            p0 = Vector((sgn * 0.42, 0.0, ztop + 1.2))
            pts = qbez(p0, p0 + Vector((sgn * 0.6, 0.5, 7)), p0 + Vector((sgn * 4.5, 5.0, 30)))
            ribbon("strap", pts, 1.5, 0.08, root, sm)

    # controls
    controls(v, W, T, H, L, body, root, rubber, body_m)

    # microSD slot, left edge near the bottom
    if v.get("sd", True):
        sd = box("sd_cut", (0.7, 0.2, 1.3), (-W / 2, 0, -H / 2 + 2.6), root, bevel=0.05)
        cut(body, sd)

    # USB-C port, bottom edge
    usb = box("usb_cut", (0.92, 0.34, 0.8), (0, 0, -H / 2), root, bevel=0.15)
    cut(body, usb)

    # back: label, pocket, clip
    lab_z = L["band"] if not v["clip"] else H / 2 - 0.85
    lab_w = min(6.2, W - 1.6)
    box("label", (lab_w, 0.004, 1.3), (0, yb + 0.002, lab_z), root,
        mat("label", (0.92, 0.91, 0.87), rough=0.6))
    text(f"SubverseReader {key} · {v['name']}\n{v['spec']}\nopen hardware · CERN-OHL-S",
         (lab_w / 2 - 0.25, yb + 0.006, lab_z + 0.38), root, 0.27, "Courier New Bold.ttf",
         ink_mat(), back=True, spacing=1.05)
    if v["pocket"]:
        pocket(key, v, W, T, H, L, root, body_m)
    if v["clip"]:
        cm = mat("clipblk", (0.02, 0.02, 0.022), rough=0.35, metal=0.3)
        zt = H / 2 - 2.3
        box("clipblock", (2.8, 0.55, 1.3), (0, yb + 0.25, zt), root, cm, bevel=0.12)
        blade = box("clipblade", (2.6, 0.2, H * 0.7), (0, yb + 0.62, zt - H * 0.33), root, cm,
                    bevel=0.08)
        blade.rotation_euler = (math.radians(-2.5), 0, 0)
        box("cliptip", (2.6, 0.22, 0.5), (0, yb + 0.52, zt - H * 0.67), root, cm, bevel=0.1)
    return H


def screen_content(v, origin, root):
    ox, oy, oz = origin
    aw, ah = v["aa"]
    m = 0.32
    x0, top = ox - aw / 2 + m, oz + ah / 2 - m
    tw = aw - 2 * m
    if v.get("color"):
        return poster_content(v, origin, root)
    title, body = ZINE[v["zine"]]
    if v["zine"] != "stage":
        body += MORE
    ink = ink_mat()
    hs = v["body_size"] * 0.78
    text(f"SUBVERSE — ISSUE 07", (x0, oy, top), root, hs, "DIN Alternate Bold.ttf", ink)
    text("14 / 52", (x0 + tw, oy, top), root, hs, "DIN Alternate Bold.ttf", ink, align="RIGHT")
    y = top - hs * 1.25
    box("rule", (tw, 0.001, 0.025), (ox, oy, y), root, ink)
    y -= 0.35
    ts = v["body_size"] * 1.9
    t = text(title, (x0, oy, y), root, ts, "Georgia Bold.ttf", ink, width=tw, spacing=0.95)
    nlines = 1 if len(title) * ts * 0.48 < tw else 2
    y -= ts * (1.05 * nlines) + 0.35
    bottom = oz - ah / 2 + m + 0.45
    if v["gray"]:
        ph = min(ah * 0.3, tw * 0.62)
        box("photo", (tw, 0.001, ph), (ox, oy, y - ph / 2), root, photo_mat(len(title)))
        y -= ph + 0.3
    if v["zine"] == "stage":
        fnt, al = "Courier New Bold.ttf", "LEFT"
    else:
        fnt, al = "Georgia.ttf", "JUSTIFY"
    text(body, (x0, oy, y), root, v["body_size"], fnt, ink, width=tw, height=y - bottom,
         align=al, spacing=1.12)
    if v.get("pan"):  # viewport map for thumbstick panning
        iw, ih = 1.5, 2.0
        ix, iz = ox + aw / 2 - m - iw / 2, oz - ah / 2 + m + 0.45 + ih / 2
        box("inset", (iw + 0.12, 0.001, ih + 0.12), (ix, oy - 0.0012, iz), root, eink_mat())
        for dx, dz, w, h in ((0, ih / 2, iw, 0.03), (0, -ih / 2, iw, 0.03),
                             (iw / 2, 0, 0.03, ih), (-iw / 2, 0, 0.03, ih)):
            box("insetrule", (w, 0.001, h), (ix + dx, oy - 0.0016, iz + dz), root, ink)
        for k in range(7):
            box("insetline", (iw * 0.8, 0.001, 0.025), (ix, oy - 0.0016, iz + ih / 2 - 0.3 - k * 0.22),
                root, ink)
        box("viewport", (iw * 0.5, 0.001, ih * 0.5), (ix - iw * 0.18, oy - 0.002, iz + ih * 0.15),
            root, mat("vp", (0.2, 0.2, 0.21), rough=0.9))
    # progress bar
    yb = oz - ah / 2 + m
    box("prog", (tw, 0.001, 0.02), (ox, oy, yb), root, ink)
    box("progfill", (tw * 0.27, 0.0012, 0.09), (x0 + tw * 0.135, oy - 0.0005, yb), root, ink)


def poster_content(v, origin, root):
    """4-colour (black, white, red, yellow) page: a zine cover, the thing colour is for."""
    ox, oy, oz = origin
    aw, ah = v["aa"]
    m = 0.32
    x0, top = ox - aw / 2 + m, oz + ah / 2 - m
    tw = aw - 2 * m
    ink = ink_mat()
    red = mat("epd_red", (0.50, 0.035, 0.025), rough=0.9)
    yel = mat("epd_yel", (0.80, 0.58, 0.02), rough=0.9)
    hs = v["body_size"] * 0.78
    text("SUBVERSE — ISSUE 07", (x0, oy, top), root, hs, "DIN Alternate Bold.ttf", ink)
    text("COVER", (x0 + tw, oy, top), root, hs, "DIN Alternate Bold.ttf", red, align="RIGHT")
    bh = ah * 0.56
    zb = top - 0.45 - bh / 2
    box("redblock", (tw, 0.001, bh), (ox, oy, zb), root, red)
    cyl("sun", tw * 0.24, 0.001, (ox + tw * 0.18, oy - 0.0008, zb + bh * 0.12), root, yel, verts=96)
    text("LOUD\nPAPER", (x0 + 0.3, oy - 0.0016, zb + bh / 2 - 0.35), root, tw * 0.25,
         "DIN Condensed Bold.ttf", ink, spacing=0.82)
    box("yband", (tw, 0.001, 0.75), (ox, oy, zb - bh / 2 - 0.45), root, yel)
    text("LIVE AT THE BASEMENT / FRI 11/14 / DOORS 8", (ox, oy - 0.0008, zb - bh / 2 - 0.45), root,
         0.3, "DIN Alternate Bold.ttf", ink, align="CENTER", valign="CENTER")
    y = zb - bh / 2 - 1.15
    body = ("Four bands, one basement, a photocopier that only prints in red and yellow. "
            "This issue is a poster first and a zine second: unfold it, pin it up, read the "
            "back on the bus. Covers, flyers and the pass itself are where colour earns its "
            "slow refresh; the stories inside stay black and white.")
    bottom = oz - ah / 2 + m + 0.1
    text(body, (x0, oy, y), root, v["body_size"] * 1.25, "Georgia.ttf", ink, width=tw,
         height=y - bottom, align="JUSTIFY", spacing=1.12)


def controls(v, W, T, H, L, body, root, rubber, body_m):
    yf = -T / 2
    zc = L["ctrl"]
    lab = mat("ctrl_lab" + v["name"], v["band_txt"] if sum(v["body"]) < 0.4 else (0.03, 0.03, 0.03),
              rough=0.5)
    if sum(v["body"]) < 0.4:
        lab = mat("ctrl_lab_w", (0.85, 0.85, 0.82), rough=0.5)

    def front_key(x, z, r, label=None, raised=0.12, m=rubber):
        c = cyl("key", r, 0.3, (x, yf - raised + 0.15, z), root, m, bevel=0.05)
        for p in c.data.polygons:
            p.use_smooth = True
        if label:
            text(label, (x, yf - 0.004, z - r - 0.22), root, 0.2, "DIN Alternate Bold.ttf", lab,
                 align="CENTER", valign="CENTER")
        return c

    def side_key(sx, z, length, depth=0.5):
        k = box("sidekey", (0.3, depth, length), (sx * (W / 2 + 0.02), 0, z), root, rubber,
                bevel=0.09)
        return k

    c = v["controls"]
    if c == "three":
        for x, l in ((-2.2, "PREV"), (0, "MENU"), (2.2, "NEXT")):
            front_key(x, zc + 0.18, 0.42, l)
    elif c == "four":
        for x, l in ((-2.55, "PREV"), (-0.85, "MENU"), (0.85, "LIGHT"), (2.55, "NEXT")):
            front_key(x, zc + 0.18, 0.36, l)
    elif c == "stick":
        # PSP-style thumbstick (Adafruit 444) at the left thumb, two keys at the right
        sx, sz = -2.0, zc + 0.15
        ring = cyl("stickwell", 1.05, 0.2, (sx, yf + 0.02, sz), root,
                   mat("well", (0.02, 0.02, 0.022), rough=0.6), bevel=0.06)
        cap = cyl("stickcap", 0.78, 0.22, (sx + 0.12, yf - 0.16, sz + 0.1), root, rubber, bevel=0.08)
        for p in cap.data.polygons:
            p.use_smooth = True
        for k in range(3):
            cyl("grip", 0.55 - k * 0.17, 0.02, (sx + 0.12, yf - 0.275, sz + 0.1), root,
                mat("grip", (0.06, 0.06, 0.065), rough=0.9))
        text("PAN", (sx, yf - 0.004, sz - 1.3), root, 0.2, "DIN Alternate Bold.ttf", lab,
             align="CENTER", valign="CENTER")
        front_key(1.6, zc - 0.2, 0.45, "BACK")
        front_key(2.9, zc + 0.45, 0.45, "OK")
    elif c == "dpad":
        front_key(0, zc + 0.12, 0.72, None)
        front_key(0, zc + 0.12, 0.26, None, raised=0.2, m=mat("dpadc", v["band_col"], 0.4))
        for a in range(4):
            ang = a * math.pi / 2
            cone = cyl("arrow", 0.07, 0.05, (0.5 * math.cos(ang), yf - 0.2, zc + 0.12 + 0.5 * math.sin(ang)),
                       root, mat("arr", (0.3, 0.3, 0.3), 0.5), verts=3)
        front_key(-3.3, zc + 0.12, 0.55, "PREV")
        front_key(3.3, zc + 0.12, 0.55, "NEXT")
    elif c == "side3":
        for i, z in enumerate((L["scr"] + 2.4, L["scr"] + 0.9, L["scr"] - 0.6)):
            side_key(1, z, 1.2 if i != 1 else 0.7)
        front_key(0, zc, 0.22, None, raised=0.06)
    elif c == "rockers":
        for sx in (-1, 1):
            side_key(sx, L["scr"] + 0.8, 3.2, 0.55)
        for x, l in ((-1.6, "BACK"), (0, "MENU"), (1.6, "SYNC")):
            front_key(x, zc + 0.12, 0.26, None, raised=0.08)
    elif c == "wheel":
        slot = box("wheel_cut", (1.2, 0.75, 2.2), (W / 2, 0, L["scr"] + 0.5), root)
        cut(body, slot)
        knurl_wheel("wheel", 0.85, 0.55, (W / 2 - 0.45, 0, L["scr"] + 0.5), root, rubber)
        for x, l in ((-1.4, "BACK"), (1.4, "OK")):
            front_key(x, zc + 0.15, 0.4, l)


def pocket(key, v, W, T, H, L, root, body_m):
    yb = T / 2
    zine = v.get("pocket_item") == "zine"
    CW, CH = (7.4, 10.5) if zine else (8.56, 5.40)  # A7 folded mini-zine, or ID-1 card
    PW, PH = CW + 0.3, CH + 0.2
    zc = -H / 2 + 0.55 + (PH + 0.5) / 2
    shell = box("pocket", (PW + 0.7, 0.42, PH + 0.5), (0, yb + 0.17, zc), root, body_m, bevel=0.18,
                segs=6)
    cav = box("cav_cut", (PW, 0.16, PH + 2), (0, yb + 0.12, zc + 1.25), root)
    cut(shell, cav)
    win = box("win_cut", (PW - 1.6, 0.6, PH - 1.9), (0, yb + 0.45, zc - 0.25), root, bevel=0.2)
    cut(shell, win)
    notch = cyl("notch_cut", 1.1, 0.6, (0, yb + 0.45, zc + (PH + 0.5) / 2), root)
    cut(shell, notch)
    # the card
    cz = zc - (PH + 0.5) / 2 + 0.35 + CH / 2 + 1.1
    cm = mat("card" + key, v["card"], rough=0.3)
    card = box("card", (CW, 0.076, CH), (0, yb + 0.12, cz), root, cm, bevel=0.0)
    if not zine:
        box("ticket", (CW - 1.4, 0.03, CH + 0.6), (0.4, yb + 0.07, cz + 0.6), root,
            mat("paper", (0.88, 0.84, 0.70), rough=0.85))
    wt = mat("cardtxt", (0.95, 0.95, 0.92), rough=0.4)
    if zine:
        wz = zc - 0.25
        xl = CW / 2 - 1.25
        yt = yb + 0.12 + 0.04
        red = mat("zine_red", (0.55, 0.04, 0.03), rough=0.85)
        text("SUBVERSE", (xl, yt, wz + 2.4), root, 1.0, "DIN Condensed Bold.ttf", red, back=True,
             valign="CENTER")
        text(v["card_txt"], (xl, yt, wz + 1.2), root, 0.55, "DIN Alternate Bold.ttf", ink_mat(),
             back=True, valign="CENTER")
        text("fold, staple, pass it on", (xl, yt, wz - 2.6), root, 0.32, "Georgia Italic.ttf",
             ink_mat(), back=True, valign="CENTER")
        return
    yt = yb + 0.12 + 0.04
    wz = zc - 0.25  # centre of the window
    xl = CW / 2 - 1.15  # left edge of the window as seen from the back
    text("SUBVERSE", (xl, yt, wz + 0.75), root, 0.85, "DIN Condensed Bold.ttf", wt, back=True,
         valign="CENTER")
    text(v["card_txt"], (xl, yt, wz - 0.2), root, 0.5, "DIN Alternate Bold.ttf",
         mat("cardacc", v["band_col"], 0.4), back=True, valign="CENTER")
    text("MEMBER 0042  ·  EXP 12/27", (xl, yt, wz - 1.05), root, 0.3,
         "Courier New Bold.ttf", wt, back=True, valign="CENTER")


# --------------------------------------------------------------------------- scene
def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    _fonts.clear()
    _mats.clear()


def render(key, samples):
    reset()
    v = VARIANTS[key]
    sc = bpy.context.scene
    world_root = bpy.data.objects.new("world_root", None)
    sc.collection.objects.link(world_root)
    world_root.scale = (0.01, 0.01, 0.01)

    H, _ = layout(v)
    W = v["W"]
    off = W * 0.62 + 1.6
    for side, yaw in ((-1, -22), (1, 180 + 24)):
        r = bpy.data.objects.new(f"root{side}", None)
        sc.collection.objects.link(r)
        r.parent = world_root
        r.location = (side * off, 0, 0)
        r.rotation_euler = (math.radians(4), 0, math.radians(yaw))
        build_device(key, v, r)

    # backdrop
    bpy.ops.mesh.primitive_plane_add(size=6, location=(0, 0.45, 0), rotation=(math.radians(90), 0, 0))
    bd = bpy.context.active_object
    bd.data.materials.append(mat("backdrop", (0.16, 0.155, 0.15), rough=0.9))

    # camera: frame both views, a little room above for the strap
    frame_w = (2 * off + W * 1.6) * 0.01
    focal = 70
    dist = frame_w * focal / 36
    cam_d = bpy.data.cameras.new("cam")
    cam_d.lens = focal
    cam_d.sensor_fit = "HORIZONTAL"
    cam = bpy.data.objects.new("cam", cam_d)
    sc.collection.objects.link(cam)
    target = Vector((0, 0, H * 0.07 * 0.01))
    cam.location = target + Vector((0, -dist, dist * 0.16))
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()
    sc.camera = cam

    def area(name, loc, size, energy, color=(1, 1, 1)):
        ld = bpy.data.lights.new(name, "AREA")
        ld.size = size
        ld.energy = energy
        ld.color = color
        lo = bpy.data.objects.new(name, ld)
        lo.location = loc
        lo.rotation_euler = (Vector((0, 0, 0)) - Vector(loc)).to_track_quat("-Z", "Y").to_euler()
        sc.collection.objects.link(lo)

    k = 0.3 if v.get("lit") else 1.0  # the front-lit variant is shown at night
    area("key", (-0.55, -0.75, 0.7), 0.7, 28 * k, (1.0, 0.96, 0.9))
    area("fill", (0.8, -0.6, 0.15), 0.9, 9 * k, (0.9, 0.95, 1.0))
    area("rim", (0.1, 0.3, 0.9), 0.5, 14 * k)

    world = bpy.data.worlds.new("w")
    world.use_nodes = True
    world.node_tree.nodes["Background"].inputs["Color"].default_value = (0.55, 0.55, 0.57, 1)
    world.node_tree.nodes["Background"].inputs["Strength"].default_value = 0.12
    sc.world = world

    sc.render.engine = "CYCLES"
    prefs = bpy.context.preferences.addons["cycles"].preferences
    try:
        prefs.compute_device_type = "METAL"
        prefs.get_devices()
        for d in prefs.devices:
            d.use = True
        sc.cycles.device = "GPU"
    except Exception as e:  # noqa: BLE001
        print("GPU unavailable, CPU render:", e)
    sc.cycles.samples = samples
    sc.cycles.use_denoising = True
    sc.render.resolution_x = 1800
    sc.render.resolution_y = 1200
    sc.render.film_transparent = False
    try:
        sc.view_settings.look = "AgX - Medium High Contrast"
    except TypeError:
        pass
    os.makedirs(OUT, exist_ok=True)
    sc.render.filepath = os.path.join(OUT, f"{key}.png")
    bpy.ops.render.render(write_still=True)
    print("wrote", sc.render.filepath)


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    samples = 96
    if "--samples" in argv:
        i = argv.index("--samples")
        samples = int(argv[i + 1])
        del argv[i:i + 2]
    keys = argv or list(VARIANTS)
    for k in keys:
        render(k, samples)


main()
