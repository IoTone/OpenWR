"""Printable shells for SR-15 Dial Pass and SR-16 Setlist.

    /Applications/Blender.app/Contents/MacOS/Blender -b -P make_enclosure.py -- [SR-15] [SR-16] [--ref DIR]

Writes enclosure/<variant>/:
    <variant>-front.stl, <variant>-back.stl, <variant>-keycap.stl ...  print-oriented, millimetres
    <variant>.blend        shells + reference electronics, assembled
    <variant>-fit.png      cut-away preview
    <variant>-report.txt   sizes, volumes, manifold check, interference check

Frame: X across the pass (right +), Y up the pass (top +), Z out of the screen (front +).
Every dimension is in millimetres and lives in the variant dicts below; the ones marked
MEASURE are from listings or estimates and must be checked against parts in hand before the
final print. --ref DIR adds board models converted from Elecrow's STEP files (cp42_board.stl)
for the fit check; without it the check uses the box models defined here.
"""
import math
import os
import sys

import bmesh
import bpy
from mathutils import Matrix, Vector

HERE = os.path.dirname(os.path.abspath(__file__))
EPS = 0.02   # cutters overshoot faces by at least this

# ----------------------------------------------------------------------------------- SR-15
# Elecrow CrowPanel ESP32 4.2in (DIE07300S). Positions are from Elecrow's Eagle board and STEP
# model (repo Elecrow-RD/CrowPanel-ESP32-4.2-E-paper-HMI-Display-with-400-300), rotated to
# portrait with the dial on the bottom edge: shell X = -STEP z, Y = -STEP x, Z = STEP y.
SR15 = dict(
    key="SR-15", name="Dial Pass",
    board=(87.0, 107.0, 3.0),            # PCB W x H, corner radius
    pcb_z=(-2.1, -0.5),                  # PCB back / front
    glass=(-38.5, 38.5, -45.5, 45.5),    # 4.2in glass X0 X1 Y0 Y1
    glass_front=1.2,                     # front of glass (STEP max, incl. tolerance)
    fpc_fold=(-40.5, -38.5),             # FPC wraps the left glass edge through the PCB slot
    # Active area 63.6 x 84.8. MEASURE: assumed borders 3.3 on the right, 10.1 on the FPC (left)
    # side, 3.1 top and bottom (Good Display 4.2in layout); check against a powered panel.
    aa=(-38.5 + 10.1, -38.5 + 10.1 + 63.6, -42.4, 42.4),
    clr=0.4,                             # board-to-wall clearance
    wall=3.0, corner_r=2.5,
    bezel=1.5,                           # front wall over the glass
    back_inner=-9.0,                     # back interior (BOOT/RESET tops at -8.6)
    back_wall=1.6,
    screw=(44.6, 54.6),                  # +/- X, +/- Y screw axes (outside the board corners)
)

# ----------------------------------------------------------------------------------- SR-16
# Bare Elecrow 3.7in panel (DIE04037S) on Seeed's ePaper Driver Board for XIAO V2 with a XIAO
# ESP32-C5, three B3F-1000 keys on the right edge, 603450 cell. Nothing here comes from a
# drawing: MEASURE every part before the final print.
SR16 = dict(
    key="SR-16", name="Setlist",
    glass=(53.0, 93.0, 1.0),             # MEASURE: listing 53 x 93 x 1 (Elecrow STEP 53 x 93.8 incl. FPC)
    # Active area 47.04 x 81.54. MEASURE: borders assumed 2.98 left/right, 3.0 top, 8.46 FPC (bottom).
    aa_border=(2.98, 2.98, 3.0, 8.46),   # left, right, top, bottom
    cell=(34.0, 50.0, 6.0),              # 603450 W x H x T (MEASURE: pouch cells swell to ~6.3)
    drv=(43.0, 26.0),                    # MEASURE: RobotShop lists 43 x 26 x 8 mm
    drv_stack=12.5,                      # MEASURE: driver board + XIAO socket + XIAO + USB-C
    usb_z_from_back=1.8,                 # MEASURE: USB-C centre above the back of the stack
    wall=1.6, corner_r=3.0, bezel=1.3,
    key_zone=7.0,                        # right margin for the key column
    key_pitch=14.0,
    back_wall=1.4,
    band_top=11.0,                       # lanyard band above the panel
    band_bottom=6.0,                     # below the panel (USB-C, label)
)

# --------------------------------------------------------------------------- mesh helpers

def obj_from_bm(name, bm, coll=None):
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    (coll or bpy.context.scene.collection).objects.link(ob)
    return ob


def rrect_pts(x0, x1, y0, y1, r, seg=10):
    r = max(0.0, min(r, (x1 - x0) / 2 - 1e-4, (y1 - y0) / 2 - 1e-4))
    if r <= 1e-3:
        return [(x0, y0), (x1, y0), (x1, y1), (x0, y1)]
    pts = []
    for cx, cy, a0 in ((x1 - r, y0 + r, -90), (x1 - r, y1 - r, 0), (x0 + r, y1 - r, 90), (x0 + r, y0 + r, 180)):
        for i in range(seg + 1):
            a = math.radians(a0 + 90 * i / seg)
            pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


def prism_bm(pts, z0, z1):
    """Extrude a CCW polygon in XY from z0 to z1."""
    bm = bmesh.new()
    lo = [bm.verts.new((x, y, z0)) for x, y in pts]
    hi = [bm.verts.new((x, y, z1)) for x, y in pts]
    n = len(pts)
    bm.faces.new(list(reversed(lo)))
    bm.faces.new(hi)
    for i in range(n):
        j = (i + 1) % n
        bm.faces.new((lo[i], lo[j], hi[j], hi[i]))
    return bm


def rrect(name, x0, x1, y0, y1, z0, z1, r=0.0, seg=10):
    return obj_from_bm(name, prism_bm(rrect_pts(x0, x1, y0, y1, r, seg), z0, z1))


def box(name, x0, x1, y0, y1, z0, z1):
    return rrect(name, x0, x1, y0, y1, z0, z1, 0.0)


def cyl(name, cx, cy, r, z0, z1, seg=40):
    pts = [(cx + r * math.cos(2 * math.pi * i / seg), cy + r * math.sin(2 * math.pi * i / seg)) for i in range(seg)]
    return obj_from_bm(name, prism_bm(pts, z0, z1))


def along(ob, axis):
    """Re-orient a prism built along Z so it runs along X or Y (through its own origin)."""
    if axis == "X":
        m = Matrix.Rotation(math.radians(90), 4, "Y")
    elif axis == "Y":
        m = Matrix.Rotation(math.radians(-90), 4, "X")
    else:
        return ob
    ob.data.transform(m)
    return ob


def edge_cut(name, axis, c1, c2, w, h, a0, a1, r=0.0):
    """A rounded-rect hole through a side wall. axis 'Y' (top/bottom wall): rectangle w (X) by
    h (Z) centred at X=c1, Z=c2, spanning Y a0..a1. axis 'X' (left/right wall): w (Y) by h (Z)
    centred at Y=c1, Z=c2, spanning X a0..a1."""
    if axis == "Y":
        ob = rrect(name, c1 - w / 2, c1 + w / 2, -h / 2, h / 2, a0, a1, r)
        ob.data.transform(Matrix(((1, 0, 0, 0), (0, 0, 1, 0), (0, 1, 0, 0), (0, 0, 0, 1))))  # local z->Y, y->Z
        flip(ob)                                                  # that swap mirrors: fix the normals
        ob.data.transform(Matrix.Translation((0, 0, c2)))
    else:
        ob = rrect(name, a0, a1, c1 - w / 2, c1 + w / 2, -h / 2, h / 2, 0.0)
        if r:
            ob2 = rrect(name, c1 - w / 2, c1 + w / 2, -h / 2, h / 2, a0, a1, r)
            bpy.data.objects.remove(ob)
            ob = ob2
            ob.data.transform(Matrix(((0, 0, 1, 0), (1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 0, 1))))  # z->X, x->Y, y->Z
        ob.data.transform(Matrix.Translation((0, 0, c2)))
    return ob


def flip(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bmesh.ops.reverse_faces(bm, faces=bm.faces)
    bm.to_mesh(ob.data)
    bm.free()


def frustum(name, x0, x1, y0, y1, grow, z0, z1, r=0.6):
    """Window cutter with a chamfer: x0..x1 / y0..y1 at z0, grown by `grow` at z1."""
    bm = bmesh.new()
    for g, z in ((0.0, z0), (grow, z1)):
        for x, y in rrect_pts(x0 - g, x1 + g, y0 - g, y1 + g, r + g, 8):
            bm.verts.new((x, y, z))
    bmesh.ops.convex_hull(bm, input=list(bm.verts))
    return obj_from_bm(name, bm)


def apply_bool(target, tool, op):
    m = target.modifiers.new("b", "BOOLEAN")
    m.operation = op
    m.solver = "EXACT"
    m.object = tool
    bpy.context.view_layer.objects.active = target
    bpy.ops.object.modifier_apply(modifier=m.name)
    bpy.data.objects.remove(tool, do_unlink=True)


def cut(target, *tools):
    for t in tools:
        apply_bool(target, t, "DIFFERENCE")


def add(target, *tools):
    for t in tools:
        apply_bool(target, t, "UNION")


def keep(target, tool):
    apply_bool(target, tool, "INTERSECT")


def mesh_stats(ob):
    bm = bmesh.new()
    bm.from_mesh(ob.data)
    bm.transform(ob.matrix_world)
    nm = sum(1 for e in bm.edges if not e.is_manifold)
    vol = bm.calc_volume(signed=False)
    xs = [v.co.x for v in bm.verts]; ys = [v.co.y for v in bm.verts]; zs = [v.co.z for v in bm.verts]
    bm.free()
    return dict(nonmanifold=nm, volume=vol, size=(max(xs) - min(xs), max(ys) - min(ys), max(zs) - min(zs)),
                lo=(min(xs), min(ys), min(zs)), hi=(max(xs), max(ys), max(zs)))


def overlap(a, b):
    """Volume of a ∩ b (mm^3), on copies."""
    ca = a.copy(); ca.data = a.data.copy(); bpy.context.scene.collection.objects.link(ca)
    cb = b.copy(); cb.data = b.data.copy(); bpy.context.scene.collection.objects.link(cb)
    keep(ca, cb)
    v = mesh_stats(ca)["volume"] if len(ca.data.vertices) else 0.0
    bpy.data.objects.remove(ca, do_unlink=True)
    return v


# --------------------------------------------------------------------------------- SR-15

def build_sr15(v, ref_dir):
    W, H, br = v["board"]
    hx, hy = W / 2 + v["clr"], H / 2 + v["clr"]            # cavity half-size
    ox, oy = hx + v["wall"], hy + v["wall"]                 # outer half-size
    zf = v["glass_front"] + 0.1                             # bezel inner face
    ztop = zf + v["bezel"]
    zs = v["pcb_z"][0]                                      # split plane = PCB back
    zbi = v["back_inner"]
    zbo = zbi - v["back_wall"]
    sx, sy = v["screw"]
    ear = dict(x=11.0, h=10.0)                              # lanyard ear above the top wall

    def profile(name, z0, z1):
        ob = rrect(name, -ox, ox, -oy, oy, z0, z1, v["corner_r"])
        add(ob, rrect(name + "e", -ear["x"], ear["x"], oy - 4, oy + ear["h"], z0, z1, 4.0))
        return ob

    def common_cuts(ob):
        """Holes through the side walls, applied to both halves."""
        tl = []
        # dial (TM_2024A): X +/-6.58, tip at Y -55.95, Z -4.3..-1.5. Slot + thumb scallop.
        tl.append(box("dial", -7.4, 7.4, -oy - 1, -hy + EPS, -4.9, -0.9))
        tl.append(cyl("scallop", 0, -oy - 10.0 + 2.4, 10.0, -6.4, 0.6, 72))
        # MENU / EXIT side keys: actuator X +/-21.5, Z -3.85, tip at Y -54.55.
        for s in (-1, 1):
            x = 21.5 * s
            tl.append(box("kpocket", x - 2.55, x + 2.55, -hy - 1.5, -hy + EPS, -6.4, -1.3))
            tl.append(box("khole", x - 1.7, x + 1.7, -oy - 1, -hy - 1.4, -3.85 - 1.2, -3.85 + 1.2))
        # USB-C: X -20.53, Z -3.21, mouth at the top board edge; plug-shell recess outside.
        tl.append(edge_cut("usb", "Y", -20.53, -3.21, 9.8, 3.9, hy - EPS, oy + 1, 1.6))
        tl.append(edge_cut("usbr", "Y", -20.53, -3.21, 12.8, 7.0, oy - 1.6, oy + 1, 2.6))
        # microSD (push-push) on the left edge: card at Y -25.55, Z ~-2.9.
        tl.append(edge_cut("sd", "X", -25.55, -2.9, 13.0, 2.6, -ox - 1, -hx + EPS, 0.8))
        tl.append(cyl("sdn", -ox - 4.0, -25.55, 5.5, -5.6, -0.2, 48))   # fingernail notch
        # battery plug (SH1.0 at top centre) needs room above the board edge: pocket in the ear.
        tl.append(box("plug", -5.0, 5.0, hy - EPS, oy + 2.0, -6.6, -0.9))
        # slide switch on the left wall, bottom: actuator slot + finger recess (MEASURE JS102011SAQN).
        tl.append(box("swslot", -ox - 1, -hx + EPS, -45.2, -40.2, -8.2, -6.2))
        tl.append(box("swrec", -ox - 1, -ox + 1.8, -47.2, -38.2, -9.2, -5.2))
        # lanyard slot through the ear.
        tl.append(rrect("slot", -7.0, 7.0, oy + 3.6, oy + 6.6, zbo - 1, ztop + 1, 1.5))
        cut(ob, *tl)

    # ---- front shell: bezel + walls from the PCB back up
    front = profile("SR-15-front", zs, ztop)
    cut(front,
        rrect("cav_pcb", -hx, hx, -hy, hy, zs - 1, v["pcb_z"][1], br + v["clr"]),
        rrect("cav_glass", v["glass"][0] - 0.3, v["glass"][1] + 0.3, v["glass"][2] - 0.3, v["glass"][3] + 0.3,
              v["pcb_z"][1] - EPS, zf, 0.5),
        box("cav_fpc", v["fpc_fold"][0] - 0.6, v["glass"][0], -11.0, 31.0, v["pcb_z"][1] - EPS, zf))
    # screw bosses (pilot for M2 thread-forming screws) in the corners outside the board.
    for x in (-sx, sx):
        for y in (-sy, sy):
            add(front, cyl("boss", x, y, 1.8, zs, zf + EPS, 32))
    keep(front, profile("trim", zs - 1, ztop + 1))
    a0, a1, b0, b1 = v["aa"]
    cut(front, frustum("window", a0 - 0.5, a1 + 0.5, b0 - 0.5, b1 + 0.5, 1.0, zf - EPS, ztop + EPS))
    for x in (-sx, sx):
        for y in (-sy, sy):
            cut(front, cyl("pilot", x, y, 0.8, zs - 1, zf + 1.0, 24))
    common_cuts(front)

    # ---- back shell
    back = profile("SR-15-back", zbo, zs)
    cut(back, rrect("cav", -hx, hx, -hy, hy, zbi, zs + 1, br + v["clr"]))
    feats = []
    for x in (-sx, sx):
        for y in (-sy, sy):
            feats.append(cyl("bboss", x, y, 2.8, zbi - EPS, zs, 40))
    for x in (-40, 40):                                     # PCB support pads (clear of parts)
        for y in (-50, 50):
            feats.append(box("pad", x - 1.5, x + 1.5, y - 1.5, y + 1.5, zbi - EPS, zs))
    # battery cell 50 x 34 x 6 on the back plate, X -25..25, Y -46..-12, located by low ribs.
    cx0, cx1, cy0, cy1 = -25.4, 25.4, -46.4, -11.6
    for rb in ((cx0 - 1.0, cx0, cy0, cy1), (cx1, cx1 + 1.0, cy0, cy1),
               (cx0 + 6, cx1 - 6, cy1, cy1 + 1.0), (cx0 + 6, cx1 - 6, cy0 - 1.0, cy0)):
        feats.append(box("rib", rb[0], rb[1], rb[2], rb[3], zbi - EPS, zbi + 2.0))
    # switch cradle: body 8.6 x 3.5 x 3.6 (MEASURE) against the left wall.
    feats.append(box("swc", -hx + 3.6, -hx + 4.6, -47.0, -38.4, zbi - EPS, zbi + 2.5))
    add(back, *feats)
    keep(back, profile("trim", zbo - 1, zs + 1))
    for x in (-sx, sx):
        for y in (-sy, sy):
            cut(back, cyl("clear", x, y, 1.15, zbo - 1, zs + 1, 24),
                cyl("cbore", x, y, 1.95, zbo - 1, zs - 3.0, 32))       # head pocket: M2x6 reaches 3 mm into the front boss
    # BOOT / RESET pinholes (K1/K2 at X 38, Y 16.3 and 30.0; tops at Z -8.6).
    cut(back, cyl("pin1", 38.0, 16.3, 1.1, zbo - 1, zbi + 1, 24), cyl("pin2", 38.0, 30.0, 1.1, zbo - 1, zbi + 1, 24))
    # laminate sticker recess on the back, 0.4 deep.
    cut(back, rrect("lam", -40, 40, -50, -14, zbo - 1, zbo + 0.4, 3.0))
    common_cuts(back)

    # ---- key plungers (MENU, EXIT): flange 4.6 x 4.6 x 0.8 in the wall pocket, stem 3.0 x 2.0.
    def plunger(name):
        p = box(name, -2.3, 2.3, -2.3, 2.3, 0, 0.8)
        add(p, box(name + "s", -1.5, 1.5, -1.0, 1.0, 0.8 - EPS, 0.8 + 1.5 + 0.8))
        return p
    caps = [plunger("SR-15-keycap-menu"), plunger("SR-15-keycap-exit")]
    # place them assembled: flange against the pocket's outer face, stem out through the wall.
    for p, x in zip(caps, (-21.5, 21.5)):
        p.matrix_world = Matrix.Translation((x, -hy - 0.65, -3.85)) @ Matrix.Rotation(math.radians(90), 4, "X")

    # ---- reference electronics
    refs = []
    if ref_dir and os.path.exists(os.path.join(ref_dir, "cp42_board.stl")):
        bpy.ops.wm.stl_import(filepath=os.path.join(ref_dir, "cp42_board.stl"))
        ob = bpy.context.selected_objects[0]
        ob.name = "ref-crowpanel"
        ob.data.transform(Matrix(((0, 0, -1, 0), (-1, 0, 0, 0), (0, 1, 0, 0), (0, 0, 0, 1))))
        refs.append(ob)
    else:
        refs.append(rrect("ref-pcb", -W / 2, W / 2, -H / 2, H / 2, *v["pcb_z"], br))
        refs.append(box("ref-glass", *v["glass"], v["pcb_z"][1], v["glass_front"]))
        refs.append(box("ref-dial", -6.58, 6.58, -55.95, -46.95, -4.3, -1.5))
    refs.append(box("ref-cell", -25, 25, -46, -12, zbi, zbi + 6.0))
    refs.append(box("ref-switch", -hx, -hx + 3.5, -47.0, -38.4, zbi + 0.0, zbi + 3.6))
    parts = dict(front=front, back=back)
    for p, n in zip(caps, ("keycap-menu", "keycap-exit")):
        parts[n] = p
    info = dict(outer=(2 * ox, 2 * oy + ear["h"] - 0.0, ztop - zbo), z=(zbo, zs, ztop),
                screws="4x M2x6 pan head, thread-forming for plastic (e.g. WN 1452 / PT style), "
                       "through the back into the front bosses",
                notes=["Board drops into the front shell face-down; the PCB border bears on the bezel step, "
                       "the glass floats 0.1 mm under the bezel: add 0.3 mm foam tape on the glass border if it rattles.",
                       "Battery: 603450 cell on the back plate between the ribs, foam tape; lead to the SH1.0 socket "
                       "through the slide switch in the left-wall cradle.",
                       "BOOT and RESET reach through the two pinholes on the back (upper right).",
                       "microSD on the left edge: push the card in with a fingernail or a pen tip through the notch."])
    return parts, refs, info


# --------------------------------------------------------------------------------- SR-16

def build_sr16(v, ref_dir):
    gw, gh, gt = v["glass"]
    w, cr = v["wall"], v["corner_r"]
    # X layout: wall | 0.3 | glass | 0.5 | key zone | wall
    ix0 = 0.0
    gx0 = ix0 + 0.3
    gx1 = gx0 + gw
    ix1 = gx1 + 0.5 + v["key_zone"]
    # Y layout: wall | band_bottom | glass | 0.3 | band_top | wall
    iy0 = 0.0
    gy0 = iy0 + v["band_bottom"]
    gy1 = gy0 + gh
    iy1 = gy1 + 0.3 + v["band_top"]
    # centre the outline on the origin
    dx, dy = -(ix0 + ix1) / 2, -(iy0 + iy1) / 2
    ix0, ix1, gx0, gx1 = ix0 + dx, ix1 + dx, gx0 + dx, gx1 + dx
    iy0, iy1, gy0, gy1 = iy0 + dy, iy1 + dy, gy0 + dy, gy1 + dy
    ox0, ox1, oy0, oy1 = ix0 - w, ix1 + w, iy0 - w, iy1 + w
    gcx = (gx0 + gx1) / 2
    # Z: glass front at 0, bezel over it, stack behind it.
    zf = 0.1
    ztop = zf + v["bezel"]
    zg = -gt                                   # glass back
    zst = zg - 0.3                             # foam, then the electronics
    depth = max(v["drv_stack"], v["cell"][2] + 0.3)
    zbi = zst - depth - 0.2
    zbo = zbi - v["back_wall"]
    zs = zg - 0.5                              # split plane just behind the glass

    def profile(name, z0, z1):
        return rrect(name, ox0, ox1, oy0, oy1, z0, z1, cr)

    # lanyard slot through the top band, centred on the glass column
    slot_y = (gy1 + 0.3 + iy1) / 2
    # bosses: four corners, M2 thread-forming, front boss from split to bezel.
    bx = (ox0 + w + 2.0, ox1 - w - 2.0)
    by = (oy0 + w + 2.0, oy1 - w - 2.0)
    # driver board: bottom of the glass column, USB-C facing down (MEASURE: orientation).
    dw, dh = v["drv"]
    d0x, d1x = gcx - dw / 2, gcx + dw / 2
    d0y, d1y = iy0 + 0.3, iy0 + 0.3 + dh
    usb_z = zbi + 0.2 + v["usb_z_from_back"]
    # cell above it
    cw, ch, ct = v["cell"]
    c0x, c1x = gcx - cw / 2, gcx + cw / 2
    c1y = gy1 - 1.0
    c0y = c1y - ch
    # keys on the right wall
    keys_y = [(gy0 + gy1) / 2 + v["key_pitch"] * k for k in (1, 0, -1)]     # PREV, MENU, NEXT
    kz = zbi + 4.0                              # key centre (6 mm switch on an 8 mm perf strip)
    perf_x1 = ix1 - 0.8 - 0.1 - 4.3            # perf face: wall - flange - gap - switch height
    perf_x0 = perf_x1 - 1.6

    def common_cuts(ob):
        tl = [rrect("slot", gcx - 7.0, gcx + 7.0, slot_y - 1.5, slot_y + 1.5, zbo - 1, ztop + 1, 1.5),
              edge_cut("usb", "Y", gcx, usb_z, 9.8, 3.9, oy0 - 1, iy0 + EPS, 1.6),
              edge_cut("usbr", "Y", gcx, usb_z, 12.8, 7.0, oy0 - 1, oy0 + 1.0, 2.6)]
        for y in keys_y:
            tl.append(along(cyl("khole", 0, 0, 2.0, 0, 1, 40), "X"))
            tl[-1].data.transform(Matrix.Diagonal((w + 2, 1, 1, 1)))
            tl[-1].data.transform(Matrix.Translation((ix1 - EPS - 0.0, y, kz)))
        # slide switch on the left wall (MEASURE JS102011SAQN), level with the cell
        sy = (c0y + c1y) / 2
        tl.append(box("swslot", ox0 - 1, ix0 + EPS, sy - 2.5, sy + 2.5, zbi + 0.8, zbi + 2.8))
        tl.append(box("swrec", ox0 - 1, ox0 + 0.6, sy - 4.5, sy + 4.5, zbi - 0.2, zbi + 3.8))
        cut(ob, *tl)

    a_l, a_r, a_t, a_b = v["aa_border"]
    ax0, ax1, ay0, ay1 = gx0 + a_l, gx1 - a_r, gy0 + a_b, gy1 - a_t

    front = profile("SR-16-front", zs, ztop)
    cut(front,
        rrect("cav", ix0, ix1, iy0, iy1, zs - 1, zg + EPS, 1.0),
        rrect("cav_glass", gx0 - 0.0, gx1 + 0.3, gy0 - 0.3, gy1 + 0.3, zg - EPS, zf, 0.5))
    for x in bx:
        for y in by:
            add(front, cyl("boss", x, y, 1.8, zs, zf + EPS, 32))
    add(front, rrect("slotboss", gcx - 9.0, gcx + 9.0, slot_y - 3.5, slot_y + 3.5, zs, zf + EPS, 2.0))
    keep(front, profile("trim", zs - 1, ztop + 1))
    cut(front, frustum("window", ax0 - 0.4, ax1 + 0.4, ay0 - 0.4, ay1 + 0.4, 0.9, zf - EPS, ztop + EPS))
    for x in bx:
        for y in by:
            cut(front, cyl("pilot", x, y, 0.8, zs - 1, zf + 0.9, 24))
    common_cuts(front)

    back = profile("SR-16-back", zbo, zs)
    cut(back, rrect("cav", ix0, ix1, iy0, iy1, zbi, zs + 1, 1.0))
    feats = []
    for x in bx:
        for y in by:
            feats.append(cyl("bboss", x, y, 2.8, zbi - EPS, zs, 40))
    feats.append(rrect("slotboss", gcx - 9.0, gcx + 9.0, slot_y - 3.5, slot_y + 3.5, zbi - EPS, zs, 2.0))
    # perf-strip rail for the keys (strip 1.6 thick, 8 mm tall, 40 long), glued to this rib
    feats.append(box("rail", perf_x0 - 1.0, perf_x0, keys_y[2] - 20, keys_y[0] + 20, zbi - EPS, zbi + 3.0))
    # cell ribs
    for rb in ((c0x - 1.0, c0x - 0.2, c0y, c1y), (c1x + 0.2, c1x + 1.0, c0y, c1y),
               (c0x + 5, c1x - 5, c0y - 1.0, c0y - 0.2)):
        feats.append(box("rib", rb[0], rb[1], rb[2], rb[3], zbi - EPS, zbi + 2.0))
    # switch cradle
    sy = (c0y + c1y) / 2
    feats.append(box("swc", ix0 + 3.6, ix0 + 4.6, sy - 4.3, sy + 4.3, zbi - EPS, zbi + 2.5))
    add(back, *feats)
    keep(back, profile("trim", zbo - 1, zs + 1))
    for x in bx:
        for y in by:
            cut(back, cyl("clear", x, y, 1.15, zbo - 1, zs + 1, 24),
                cyl("cbore", x, y, 1.95, zbo - 1, zs - 4.0, 32))     # M2x6 tip stops 0.9 under the bezel face
    cut(back, rrect("lam", gcx - 22, gcx + 22, oy0 + 6, oy0 + 40, zbo - 1, zbo + 0.4, 3.0))
    common_cuts(back)

    # key caps: flange d6.4 x 0.8 inside the wall, stem d3.6 through the d4.0 hole, 1.0 proud.
    caps = {}
    for n, y in zip(("prev", "menu", "next"), keys_y):
        c = cyl(f"SR-16-keycap-{n}", 0, 0, 3.2, 0, 0.8, 40)
        add(c, cyl("st", 0, 0, 1.8, 0.8 - EPS, 0.8 + w + 1.0, 40))
        c.matrix_world = Matrix.Translation((ix1 - 0.8, y, kz)) @ Matrix.Rotation(math.radians(90), 4, "Y")
        caps["keycap-" + n] = c

    refs = [box("ref-glass", gx0, gx1, gy0, gy1, zg, 0.0),
            box("ref-driver+xiao", d0x, d1x, d0y, d1y, zst - v["drv_stack"], zst),
            box("ref-cell", c0x, c1x, c0y, c1y, zbi, zbi + ct),
            box("ref-perf", perf_x0, perf_x1, keys_y[2] - 20, keys_y[0] + 20, zbi, zbi + 8.0),
            box("ref-switch", ix0, ix0 + 3.5, sy - 4.3, sy + 4.3, zbi, zbi + 3.6)]
    for y in keys_y:
        refs.append(box("ref-key", perf_x1, perf_x1 + 3.5, y - 3, y + 3, kz - 3, kz + 3))
    parts = dict(front=front, back=back, **caps)
    info = dict(outer=(ox1 - ox0, oy1 - oy0, ztop - zbo), z=(zbo, zs, ztop),
                screws="4x M2x6 pan head, thread-forming for plastic, through the back into the front bosses",
                notes=["Glass drops into the front shell face-down, taped to the bezel border with 0.2 mm "
                       "double-sided tape (it is not clamped from behind).",
                       "Driver board + XIAO at the bottom behind the glass, USB-C out of the bottom edge; "
                       "the panel FPC folds back onto the driver board's connector.",
                       "Keys: three B3F-1000 on an 8 x 40 mm perfboard strip glued to the rail, plungers "
                       "against the printed caps in the right wall.",
                       f"Driver stack assumed {v['drv_stack']} mm (XIAO in its socket). Soldering the XIAO flat "
                       "to the driver board instead brings it to about 6.5 mm: set drv_stack and rerun for a "
                       "~12 mm pass."])
    return parts, refs, info


# ------------------------------------------------------------------------------- outputs

def print_pose(ob, part):
    """Copy of `ob` lying on the bed (z=0) the way it should be printed."""
    c = ob.copy(); c.data = ob.data.copy()
    bpy.context.scene.collection.objects.link(c)
    if not part.startswith("keycap"):          # key caps are modelled flange-down already
        c.data.transform(c.matrix_world)
    c.matrix_world = Matrix.Identity(4)
    if part == "front":                         # face down: flip so the bezel face is on the bed
        c.data.transform(Matrix.Rotation(math.radians(180), 4, "X"))
    zs = [vv.co.z for vv in c.data.vertices]
    c.data.transform(Matrix.Translation((0, 0, -min(zs))))
    return c


def export_stl(ob, path):
    bpy.ops.object.select_all(action="DESELECT")
    ob.select_set(True)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.wm.stl_export(filepath=path, export_selected_objects=True, apply_modifiers=True,
                          global_scale=1.0, ascii_format=False)


def render_fit(path, parts, refs, info):
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "OBJECT"
    scene.display.shading.show_xray = False
    scene.display.shading.show_cavity = True
    scene.display.shading.cavity_type = "BOTH"
    scene.display.shading.show_shadows = True
    scene.display.shading.show_object_outline = True
    scene.display.light_direction = (0.45, -0.35, 0.82)
    scene.render.resolution_x, scene.render.resolution_y = 1600, 1000
    scene.render.film_transparent = False
    # exploded: back at -Z, front lifted, both turned to show the inside
    colors = dict(front=(0.12, 0.56, 0.52, 1), back=(0.18, 0.62, 0.58, 1))
    shown = []
    W = info["outer"][0]
    for n, ob in parts.items():
        c = ob.copy(); c.data = ob.data.copy(); scene.collection.objects.link(c)
        c.data.transform(c.matrix_world); c.matrix_world = Matrix.Identity(4)
        c.color = colors.get(n, (0.9, 0.5, 0.2, 1))
        if n == "front":
            c.data.transform(Matrix.Rotation(math.radians(180), 4, "Y"))   # inside up
            c.data.transform(Matrix.Translation((-W * 1.15, 0, 0)))
        shown.append(c)
    for r in refs:
        c = r.copy(); c.data = r.data.copy(); scene.collection.objects.link(c)
        c.color = (0.85, 0.2, 0.2, 1) if "cell" in r.name else (0.25, 0.3, 0.75, 1)
        c.data.transform(c.matrix_world); c.matrix_world = Matrix.Identity(4)
        c.data.transform(Matrix.Translation((W * 1.15, 0, 0)))
        shown.append(c)
    cam = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
    scene.collection.objects.link(cam)
    cam.data.type = "ORTHO"
    cam.data.ortho_scale = W * 3.75
    cam.location = (0, -200 * math.sin(math.radians(30)), 200 * math.cos(math.radians(30)))
    cam.rotation_euler = (math.radians(30), 0, 0)
    scene.render.resolution_y = int(1600 * (info["outer"][1] * 0.87 + 20) * 1.2 / (W * 3.75))
    scene.camera = cam
    for o in scene.objects:
        o.hide_render = not (o in shown or o == cam)
    scene.render.filepath = path
    bpy.ops.render.render(write_still=True)
    for o in shown + [cam]:
        bpy.data.objects.remove(o, do_unlink=True)
    for o in scene.objects:
        o.hide_render = False


def run(v, builder, ref_dir):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    s = bpy.context.scene
    s.unit_settings.system = "METRIC"
    s.unit_settings.scale_length = 0.001
    s.unit_settings.length_unit = "MILLIMETERS"
    out = os.path.join(HERE, v["key"])
    os.makedirs(out, exist_ok=True)
    parts, refs, info = builder(v, ref_dir)
    lines = [f"{v['key']} {v['name']} enclosure", "",
             "Outer size %.1f x %.1f x %.1f mm (W x H incl. lanyard ear x T)" % info["outer"],
             "Z: back %.2f, split %.2f, front %.2f" % info["z"], "Screws: " + info["screws"], ""]
    bad = 0
    for n, ob in parts.items():
        st = mesh_stats(ob)
        bad += st["nonmanifold"]
        pose = print_pose(ob, n)
        ps = mesh_stats(pose)
        path = os.path.join(out, f"{v['key']}-{n}.stl")
        export_stl(pose, path)
        bpy.data.objects.remove(pose, do_unlink=True)
        lines.append(f"{n:12s} {ps['size'][0]:6.1f} x {ps['size'][1]:6.1f} x {ps['size'][2]:5.1f} mm on the bed, "
                     f"{st['volume'] / 1000:5.2f} cm3 (~{st['volume'] / 1000 * 1.27:4.1f} g PETG solid), "
                     f"non-manifold edges {st['nonmanifold']}")
    src = ("Elecrow's STEP model (--ref)" if any(r.name == "ref-crowpanel" for r in refs)
           else "box stand-ins defined in this script")
    lines += ["", f"Interference (part vs reference electronics, mm3; 0 = clear). References: {src}."]
    for n, ob in parts.items():
        if not n.startswith(("front", "back")):
            continue
        for r in refs:
            ov = overlap(ob, r)
            flag = "" if ov < 0.5 else "   <-- CHECK"
            lines.append(f"  {n:6s} x {r.name:18s} {ov:8.2f}{flag}")
    lines += ["", "Assembly:"] + ["- " + n for n in info["notes"]]
    lines += ["", "Print: PETG, 0.2 mm layers, 4 walls, 25% gyroid. Front shell face down (bezel on the bed),",
              "back shell back down; key caps flange down. No supports, except the USB-C and lanyard",
              "slot bridges if your printer sags over 14 mm.", "",
              "MEASURE before the final print: see the MEASURE notes at the top of make_enclosure.py."]
    with open(os.path.join(out, f"{v['key']}-report.txt"), "w") as f:
        f.write("\n".join(lines) + "\n")
    print("\n".join(lines))
    coll = bpy.data.collections.new("reference electronics")
    s.collection.children.link(coll)
    for r in refs:
        for c in r.users_collection:
            c.objects.unlink(r)
        coll.objects.link(r)
    render_fit(os.path.join(out, f"{v['key']}-fit.png"), parts, refs, info)
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out, f"{v['key']}.blend"))
    return bad


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    ref_dir = None
    if "--ref" in argv:
        i = argv.index("--ref"); ref_dir = argv[i + 1]; del argv[i:i + 2]
    todo = argv or ["SR-15", "SR-16"]
    table = {"SR-15": (SR15, build_sr15), "SR-16": (SR16, build_sr16)}
    bad = 0
    for k in todo:
        bad += run(*table[k], ref_dir)
    if bad:
        print(f"WARNING: {bad} non-manifold edges")


main()
