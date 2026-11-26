"""Printable shells for SR-15 Dial Pass, SR-16 Setlist and SR-16N Setlist, no keys.

    /Applications/Blender.app/Contents/MacOS/Blender -b -P make_enclosure.py -- [SR-15] [SR-16] [SR-16N] [--ref DIR]

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
# ESP32-C5 (or a XIAO ESP32-S3: same footprint, same shell), three B3F-1000 keys on the right
# edge (PREV = D4, MENU = D7, NEXT = D9), 603450 cell plugged into the driver board's JST
# connector and switched by the driver board's own battery switch. Nothing here comes from a
# drawing: MEASURE every part before the final print.
SR16 = dict(
    key="SR-16", name="Setlist",
    # Rev D layout (2026-11-25; rev C plus Seeed's STEP model of the driver board and a cable
    # channel in place of a snug USB-C hole). Seen from the front:
    #   - the glass is turned so its ribbon tail is at the TOP, under the lanyard band
    #     (firmware: `2 epd-rot!`);
    #   - the tail folds once, in its own marked bending area at the glass edge, onto a 24-pin
    #     FFC coupler held flat against the back of the glass; a 24-pin jumper runs down over
    #     the cell to the driver board, so nothing levers on the panel's own tail;
    #   - the driver board + socketed XIAO sit behind the bottom of the glass, component side
    #     to the back, the XIAO's USB-C pointing at the BOTTOM edge but set back from it: a
    #     channel one plug wide, open to the back, runs from the edge to the port, so the port
    #     does not have to line up with a hole;
    #   - the back is stepped: a deep pod over the driver stack, a thin shelf over the cell.
    # From Good Display's GDEY037T03 drawing (Elecrow publishes the same one for DIE04037S):
    glass=(53.0, 92.99, 1.0),            # 53.00 +/-0.20 x 92.99 +/-0.20 x 1.00 +/-0.10
    # Active area 47.04 x 81.54: 2.98 from each long edge and the far end, 8.47 from the tail
    # end. With the tail at the top: left, right, top (tail), bottom.
    aa_border=(2.98, 2.98, 8.47, 2.98),
    tail_w=13.8,                         # tail neck 13.8 (12.5 at the contacts); it is 10.15 long
    tail_reach=4.0,                      # the coupler's mouth, below the glass's top edge: the tail's
                                         # bending area is 0.5-2.5 mm past the edge, so ~7.5 mm of it
                                         # lies on the back and ~3.5 goes into the connector
    cell=(50.0, 34.0, 6.0),              # 603450 lying across the pass (MEASURE: pouches swell to ~6.3)
    # Driver board, from Seeed's STEP model ("ePaper Driver Board for Seeed Studio XIAO v1.step"):
    # PCB 25.00 x 42.00 x 1.6; pins 1.5 below it; sockets 4.5 tall, centred across the board,
    # starting 1.45 mm from the XIAO end; JST 6.3 x 8.2 x 4.8 tall; ribbon connector 15.5 wide,
    # 2.0 tall, at the far end, centred; slide switch on the long edge, 0.8 mm past it.
    drv=(25.0, 42.0),
    drv_stack=27.0,                      # MEASURED 2026-11-25: tips of the driver board's pins to the top
                                         # of the XIAO ESP32-C5's USB-C, XIAO in its socket on long pins
    drv_pcb_z=3.1,                       # top face of the driver board above its pin tips: 1.5 tail + 1.6 board
    usb_over=2.5,                        # USB-C past the driver board's end, from the two board models: the
                                         # XIAO's edge is 0.24 past it, the USB-C 2.3 past that (4 by ruler)
    usb_recess=5.0,                      # the USB-C's mouth, inside the outer face of the bottom wall
    usb_channel=(14.0, 4.0),             # cable channel: width (a USB-C plug's grip is ~12.5 x 6.5), and how
                                         # far it reaches toward the glass past the plug, for a shorter stack
    usb_z_from_back=1.6,                 # USB-C centre below the top of the stack (receptacle 3.2 tall)
    usb_dx=0.0,                          # MEASURE: USB-C centre off the driver board's centreline
    coupler=(20.0, 12.0, 3.6),           # MEASURE: FFC coupler board (Adafruit 4524 class: two top-contact
                                         # 24-pin connectors, 18.3 mm long each), across x along the pass x thick
    wall=1.6, corner_r=3.0, bezel=1.3,
    keys=3,                              # 3 = PREV / MENU / NEXT column on the right edge, 0 = none
    key_zone=7.0,                        # right margin for the key column (dropped when keys=0)
    key_pitch=14.0,
    # The driver board's battery switch, from the STEP model: on the board's edge that faces the
    # RIGHT wall seen from the front, 29.7 mm from the board's XIAO end. The board has to stay on
    # the glass's centreline (the ribbon cannot step sideways), so the switch is ~14 mm inside
    # the wall: the opening is a tool window, not a slider.
    # wall "L" or "R" (seen from the front), y = centre above the board's USB end, w x h = window
    # along the wall x through the thickness. None = no opening.
    drv_switch=dict(wall="R", y=29.7, w=10.0, h=5.0),
    # Paperclip pinhole in the back over the XIAO's BOOT button: (x from the board centreline,
    # right + seen from the front; y above the driver board's USB end; hole diameter). From
    # Seeed's KiCad board for the XIAO ESP32-C5: BOOT (K2) is 19.59 mm from the XIAO's USB edge
    # and 4.42 mm off the centreline toward the D7-D10 row, which is the right-hand side seen
    # from the front when the components face the back. The XIAO's first pin is 2.96 mm from
    # that edge and the driver board's first socket 2.72 mm from its end (STEP), so the XIAO's
    # edge is 0.24 mm past the driver board's end.
    boot_pin=None,
    sd=None,                             # microSD breakout (SR-16SD)
    back_wall=1.4,
    band_top=14.0,                       # above the glass: the tail's fold, then the lanyard slot
    band_bottom=9.0,                     # below the glass
)

# ---------------------------------------------------------------------------------- SR-16N
# SR-16 without the key column: turned from the phone over Bluetooth, by the auto-turn timer and
# by the XIAO's own BOOT button through a paperclip pinhole in the back. The shell is narrower by
# key_zone.
SR16N = dict(SR16, key="SR-16N", name="Setlist, no keys", keys=0,
             boot_pin=(4.4, 19.35, 1.8))

# --------------------------------------------------------------------------------- SR-16SD
# SR-16N with a microSD card. The breakout (Adafruit 4682: 25.4 x 22.8 x 3.5 mm overall, 3 V
# only, SPI) stands on edge in the pod beside the driver board, on the left seen from the front,
# taped to a fin on the tray, so the card goes in through the BOTTOM edge next to the USB-C
# channel. Wires: 3V3, GND, SCK = D8 and MOSI = D10 (shared with the panel), MISO = D9, CS = D7.
#   pcb = (along the pass, through the thickness, board thickness). The card enters the end that
#         faces the bottom wall, an edge pcb[1] long, at its middle (MEASURE: which edge of the
#         breakout the card enters, and where along it)
#   socket = height of the card socket on the board; x = socket-side face of the board, from
#   the inside of the left wall; slot = (card slot length, width) in the bottom wall;
#   proud = how far the card sticks out past the breakout's edge (MEASURE)
SR16SD = dict(SR16N, key="SR-16SD", name="Setlist, card",
              sd=dict(pcb=(22.8, 25.4, 1.6), socket=1.9, x=8.5, slot=(12.5, 2.6), proud=2.5))

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
    K = v["key"]
    nkeys = v.get("keys", 3)
    assert nkeys in (0, 3), "keys must be 0 or 3"
    # X layout: wall | 0.3 | glass | 0.5 | key zone (only with keys) | wall
    ix0 = 0.0
    gx0 = ix0 + 0.3
    gx1 = gx0 + gw
    ix1 = gx1 + 0.5 + (v["key_zone"] if nkeys else 0.0)
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
    # Z: glass front at 0, bezel over it, everything else behind it.
    zf = 0.1
    ztop = zf + v["bezel"]
    zg = -gt                                   # glass back
    zst = zg - 0.3                             # foam, then the electronics
    zs = zg - 0.5                              # split plane just behind the glass
    # The pod: deep, over the driver stack. The stack stands on a printed tray (1.0 mm) so the
    # pin tips never touch the glass.
    tray_t = 1.0
    zpod = zst - tray_t - v["drv_stack"] - 0.5 - 0.2      # pod floor (inside)
    zbo = zpod - v["back_wall"]
    # The shelf: thin, over the cell (against the glass), the jumper (over the cell) and the coupler.
    cw, ch, ct = v["cell"]
    cplw, cplh, cplt = v["coupler"]
    shelf_depth = max(ct + 0.3 + 1.0, cplt + 0.6, 8.4 if nkeys else 0.0)
    zsh = zst - shelf_depth - 0.2              # shelf floor (inside)
    zsho = zsh - v["back_wall"]

    def profile(name, z0, z1):
        return rrect(name, ox0, ox1, oy0, oy1, z0, z1, cr)

    # lanyard slot high in the top band; the tail folds in the 5 mm under it
    slot_y = iy1 - 4.5
    bx = (ox0 + w + 2.0, ox1 - w - 2.0)
    by = (oy0 + w + 2.0, oy1 - w - 2.0)
    # driver board: on the glass's centreline, USB-C set back from the bottom edge
    dw, dh = v["drv"]
    yport = oy0 + v["usb_recess"]              # the USB-C's mouth
    d0y = yport + v["usb_over"]                # the board's USB end
    d1y = d0y + dh                             # its ribbon-connector end
    d0x, d1x = gcx - dw / 2, gcx + dw / 2
    usb_x = gcx + v["usb_dx"]
    usb_z = zst - tray_t - v["drv_stack"] + v["usb_z_from_back"]
    zpcb = zst - tray_t - v["drv_pcb_z"]       # the board's component face (it looks at the back)
    ystep = d1y + 1.2                          # the pod ends here (inside); the step wall is w thick
    # cell on the shelf, against the glass
    c0x, c1x = gcx - cw / 2, gcx + cw / 2
    c0y = ystep + w + 0.4
    c1y = c0y + ch
    # coupler flat on the back of the glass, just under the tail's fold
    q1y = gy1 - v["tail_reach"]
    q0y = q1y - cplh
    q0x, q1x = gcx - cplw / 2, gcx + cplw / 2
    assert c1y + 0.5 <= q0y, "cell and coupler collide: shorten the cell or lengthen the glass column"
    # keys on the right wall of the shelf
    kyc = (ystep + w + iy1 - v["band_top"]) / 2
    keys_y = [kyc + v["key_pitch"] * k for k in (1, 0, -1)] if nkeys else []   # PREV, MENU, NEXT
    kz = zsh + 4.0                              # key centre (6 mm switch on an 8 mm perf strip)
    perf_x1 = ix1 - 0.8 - 0.1 - 4.3            # perf face: wall - flange - gap - switch height
    perf_x0 = perf_x1 - 1.6

    sw = v.get("drv_switch")
    boot = v.get("boot_pin")
    chw, chx = v["usb_channel"]
    zch = usb_z + 3.25 + chx                   # the channel's roof (toward the glass)
    zstk = zst - tray_t                        # the tray's top: what the electronics in the pod stand on
    sd = v.get("sd")
    if sd:
        sdl, sdh, sdt = sd["pcb"]
        sdx0 = ix0 + sd["x"]                   # board: sdx0..sdx0+sdt, socket on the wall side of it
        sd0y, sd1y = iy0 + 0.3, iy0 + 0.3 + sdl
        sdz0, sdz1 = zstk - sdh, zstk          # standing on the tray
        cardx, cardz = sdx0 - 0.95, zstk - sdh / 2

    def common_cuts(ob):
        tl = [rrect("slot", gcx - 7.0, gcx + 7.0, slot_y - 1.5, slot_y + 1.5, zbo - 1, ztop + 1, 1.5),
              # cable channel: through the bottom wall and the back, from the edge to the port
              rrect("usbch", usb_x - chw / 2, usb_x + chw / 2, oy0 - 4, yport + 0.5, zbo - 1, zch, 2.0)]
        for y in keys_y:
            tl.append(along(cyl("khole", 0, 0, 2.0, 0, 1, 40), "X"))
            tl[-1].data.transform(Matrix.Diagonal((w + 2, 1, 1, 1)))
            tl[-1].data.transform(Matrix.Translation((ix1 - EPS - 0.0, y, kz)))
        if sw:                                  # window onto the driver board's battery switch
            a0, a1 = (ox0 - 1, ix0 + EPS) if sw["wall"] == "L" else (ix1 - EPS, ox1 + 1)
            tl.append(edge_cut("drvsw", "X", d0y + sw["y"], zpcb - 1.5, sw["w"], sw["h"], a0, a1, 1.5))
        cut(ob, *tl)

    a_l, a_r, a_t, a_b = v["aa_border"]
    ax0, ax1, ay0, ay1 = gx0 + a_l, gx1 - a_r, gy0 + a_b, gy1 - a_t

    front = profile(K + "-front", zs, ztop)
    cut(front,
        rrect("cav", ix0, ix1, iy0, iy1, zs - 1, zg + EPS, 1.0),
        rrect("cav_glass", gx0 - 0.0, gx1 + 0.3, gy0 - 0.3, gy1 + 0.3, zg - EPS, zf, 0.5),
        # room for the tail to leave the glass and turn back
        box("cav_tail", gcx - v["tail_w"] / 2 - 1.0, gcx + v["tail_w"] / 2 + 1.0, gy1, gy1 + 5.0, zg - EPS, zf))
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

    back = profile(K + "-back", zbo, zs)
    cut(back,
        rrect("cav_pod", ix0, ix1, iy0, ystep, zpod, zs + 1, 1.0),
        rrect("cav_shelf", ix0, ix1, ystep + w, iy1, zsh, zs + 1, 1.0),
        # the jumper's doorway through the step wall, level with the board's connector
        box("door", gcx - v["tail_w"] / 2 - 2.0, gcx + v["tail_w"] / 2 + 2.0, ystep - EPS, ystep + w + EPS,
            zsh, zs + 1),
        # the outside of the shelf: everything behind its floor comes off
        box("step", ox0 - 1, ox1 + 1, ystep + w, oy1 + 1, zbo - 1, zsho))
    feats = []
    for x in bx:
        feats.append(cyl("bboss", x, by[0], 2.8, zpod - EPS, zs, 40))
        feats.append(cyl("bboss", x, by[1], 2.8, zsh - EPS, zs, 40))
    feats.append(rrect("slotboss", gcx - 9.0, gcx + 9.0, slot_y - 3.5, slot_y + 3.5, zsh - EPS, zs, 2.0))
    # the cable channel's two sides and its roof, so a plug cannot wander into the pod
    for x0, x1 in ((usb_x - chw / 2 - 1.2, usb_x - chw / 2), (usb_x + chw / 2, usb_x + chw / 2 + 1.2)):
        feats.append(box("chside", x0, x1, iy0 - EPS, yport + 0.5, zpod - EPS, zch + 1.2))
    feats.append(box("chroof", usb_x - chw / 2 - 1.2, usb_x + chw / 2 + 1.2, iy0 - EPS, yport + 0.5, zch, zch + 1.2))
    if nkeys:                                   # perf-strip rail for the keys, glued to this rib
        feats.append(box("rail", perf_x0 - 1.0, perf_x0, kyc - 20, kyc + 20, zsh - EPS, zsh + 3.0))
    # cell fence (the cell lies against the glass; the jumper passes over it through the gap at x = centre)
    zrib = zst - ct + 1.0
    for rb in ((c0x - 1.0, c0x - 0.2, c0y + 4, c1y - 4), (c1x + 0.2, c1x + 1.0, c0y + 4, c1y - 4),
               (c0x + 4, gcx - 10, c1y + 0.2, c1y + 1.0), (gcx + 10, c1x - 4, c1y + 0.2, c1y + 1.0)):
        if rb[1] <= ix1 and rb[0] >= ix0:
            feats.append(box("rib", rb[0], rb[1], rb[2], rb[3], zsh - EPS, zrib))
    # coupler: two pads press it to the glass, a fence on three sides holds it against the ribbon's pull
    zq = zst - cplt - 0.2
    feats += [box("qpad", q0x + 1.0, q0x + 4.0, q0y + 2.0, q1y - 2.0, zsh - EPS, zq),
              box("qpad", q1x - 4.0, q1x - 1.0, q0y + 2.0, q1y - 2.0, zsh - EPS, zq),
              box("qfence", q0x - 1.0, q0x - 0.2, q0y, q1y, zsh - EPS, zst - 1.0),
              box("qfence", q1x + 0.2, q1x + 1.0, q0y, q1y, zsh - EPS, zst - 1.0)]
    add(back, *feats)
    keep(back, profile("trim", zbo - 1, zs + 1))
    for x in bx:
        for y in by:
            cut(back, cyl("clear", x, y, 1.15, zbo - 1, zs + 1, 24),
                cyl("cbore", x, y, 1.95, zbo - 1, zs - 4.0, 32))     # M2x6 tip stops 0.9 under the bezel face
    # laminate sticker recess on the shelf, 0.4 deep
    lam_y0 = ystep + w + 3.0
    lam_h = min(34.0, slot_y - 4.5 - lam_y0)
    cut(back, rrect("lam", gcx - 22, gcx + 22, lam_y0, lam_y0 + lam_h, zsho - 1, zsho + 0.4, 3.0))
    if boot:                                    # paperclip pinhole over the XIAO's BOOT button
        cut(back, cyl("bootpin", gcx + boot[0], d0y + boot[1], boot[2] / 2, zbo - 1, zpod + 1, 24))
    if sd:                                      # card slot in the bottom wall, with a thumbnail relief
        sl, sw_ = sd["slot"]
        cut(back, box("sdslot", cardx - sw_ / 2, cardx + sw_ / 2, oy0 - 1, iy0 + EPS, cardz - sl / 2, cardz + sl / 2),
            box("sdnail", cardx - 3.0, cardx + 3.0, oy0 - 1, oy0 + 0.8, cardz - 3.5, cardz + 3.5))
    common_cuts(back)

    # tray: stands the driver board's pin tips off the glass and locates the board's corners
    tx0, tx1, ty0, ty1 = ix0 + 0.3, ix1 - 0.3, iy0 + 0.3, ystep - 0.3
    tray = box(K + "-tray", tx0, tx1, ty0, ty1, zst - tray_t, zst)
    pegs = []
    for x0, x1 in ((d0x - 1.6, d0x - 0.2), (d1x + 0.2, d1x + 1.6)):
        for y0, y1 in ((d0y + 1.0, d0y + 5.0), (d1y - 5.0, d1y - 1.0)):
            pegs.append(box("peg", x0, x1, y0, y1, zpcb, zst - tray_t + EPS))
    # end stops at the board's corners, outside the XIAO and the ribbon connector: the far pair
    # is backed by the step wall and takes the push of a plug going in
    for x0, x1 in ((d0x, d0x + 2.5), (d1x - 2.5, d1x)):
        pegs.append(box("stop", x0, x1, d0y - 1.6, d0y - 0.2, zpcb, zst - tray_t + EPS))
        pegs.append(box("stop", x0, x1, d1y + 0.2, ty1, zpcb, zst - tray_t + EPS))
    if sd:                                      # fin the card breakout is taped to, and its foot
        fx = sdx0 + sdt + 0.5
        pegs.append(box("sdfin", fx, fx + 1.4, sd0y + 1.0, sd1y, sdz0 + 2.0, zst - tray_t + EPS))
        pegs.append(box("sdfoot", fx, fx + 4.0, sd0y + 1.0, sd0y + 2.4, zstk - 8.0, zst - tray_t + EPS))
    add(tray, *pegs)
    for x in bx:                                # clear of the two lower screw bosses
        cut(tray, cyl("tclear", x, by[0], 3.2, zst - tray_t - 1, zst + 1, 32))

    # key caps: flange d6.4 x 0.8 inside the wall, stem d3.6 through the d4.0 hole, 1.0 proud.
    caps = {}
    for n, y in zip(("prev", "menu", "next"), keys_y):
        c = cyl(f"{K}-keycap-{n}", 0, 0, 3.2, 0, 0.8, 40)
        add(c, cyl("st", 0, 0, 1.8, 0.8 - EPS, 0.8 + w + 1.0, 40))
        c.matrix_world = Matrix.Translation((ix1 - 0.8, y, kz)) @ Matrix.Rotation(math.radians(90), 4, "Y")
        caps["keycap-" + n] = c

    refs = [box("ref-glass", gx0, gx1, gy0, gy1, zg, 0.0),
            box("ref-driver-pcb", d0x, d1x, d0y, d1y, zpcb, zpcb + 1.6),
            box("ref-xiao", usb_x - 8.9, usb_x + 8.9, d0y - 0.24, d0y + 20.76, zstk - v["drv_stack"] + 3.2, zpcb - 4.5),
            box("ref-usb-c", usb_x - 4.5, usb_x + 4.5, d0y - v["usb_over"], d0y + 5.0,
                zstk - v["drv_stack"], zstk - v["drv_stack"] + 3.2),
            # a plug in the port: 12.4 x 6.5 grip, 20 long
            box("ref-usb-plug", usb_x - 6.2, usb_x + 6.2, yport - 20.0, yport, usb_z - 3.25, usb_z + 3.25),
            box("ref-drv-switch", d1x - 3.2, d1x + 0.8, d0y + 26.3, d0y + 33.1, zpcb - 1.42, zpcb),
            box("ref-drv-jst", d0x + 3.85, d0x + 10.14, d0y + 25.5, d0y + 33.7, zpcb - 4.8, zpcb),
            box("ref-drv-fpc", d0x + 4.76, d0x + 20.28, d0y + 36.2, d0y + 41.1, zpcb - 2.0, zpcb),
            box("ref-cell", c0x, c1x, c0y, c1y, zst - ct, zst),
            box("ref-coupler", q0x, q1x, q0y, q1y, zst - cplt, zst)]
    if sd:
        refs.append(box("ref-sd-board", sdx0 - sd["socket"], sdx0 + sdt, sd0y, sd1y, sdz0, sdz1))
        refs.append(box("ref-sd-card", cardx - 0.5, cardx + 0.5, sd0y - sd["proud"], sd0y + 12.5,
                        cardz - 5.5, cardz + 5.5))
    if nkeys:
        refs.append(box("ref-perf", perf_x0, perf_x1, kyc - 20, kyc + 20, zsh, zsh + 8.0))
    for y in keys_y:
        refs.append(box("ref-key", perf_x1, perf_x1 + 3.5, y - 3, y + 3, kz - 3, kz + 3))
    parts = dict(front=front, back=back, tray=tray, **caps)
    pod_t, shelf_t = ztop - zbo, ztop - zsho
    notes = ["Glass drops into the front shell face-down, taped to the bezel border with 0.2 mm "
             "double-sided tape, with its ribbon tail at the TOP (set `2 epd-rot!` on the pass).",
             "Tail: fold it once over the top edge of the glass, in its marked bending area (0.5 to 2.5 mm "
             "past the edge), so its last 7 mm lie on the back with the gold fingers facing the back.",
             f"Coupler ({cplw:.0f} x {cplh:.0f} mm, 24-pin 0.5 mm, top-contact connectors, e.g. Adafruit "
             f"4524): on the back of the glass on foam tape, connectors facing the back, its top edge "
             f"{v['tail_reach']:.0f} mm below the top of the glass, between the two fences; the back shell's "
             "pads press it down. The tail goes in its top connector, the jumper in its bottom one.",
             "Jumper: 24-pin 0.5 mm FFC, 60 mm, same-side contacts (type A), fingers facing the back at "
             "both ends. From the coupler it runs down OVER the cell, through the doorway in the step, into "
             "the driver board's (top-contact) connector. The run must be a straight extension: each "
             "conductor stays on its own side. Check that with a meter, tail pad to driver-board pad, "
             "before power: this ribbon carries the panel's high voltages.",
             "Tray: 1 mm plate on the back of the glass in the pod, pegs up. The driver board stands on it "
             "on its pin tips, between the pegs and the corner stops, component side to the back, USB-C "
             "toward the bottom edge.",
             f"USB-C: the port is {v['usb_recess']:.0f} mm inside the bottom edge, at the end of a channel "
             f"{chw:.0f} mm wide that is open to the back. Lay the plug in the channel and push. The channel "
             f"reaches {chx:.0f} mm further toward the glass than a plug needs, so a shorter stack still "
             "lines up.",
             f"Cell: 603450 lying across the shelf against the glass, plugged into the driver board's JST. "
             "The driver board's own switch disconnects it.",
             f"Thickness: {pod_t:.1f} mm over the pod (bottom {ystep + w - oy0:.0f} mm), {shelf_t:.1f} mm over "
             "the shelf. The pod is as deep as the measured 27 mm stack. Seeed's models put a XIAO on "
             "standard short pins at 14.4 mm, which would make the pod about 20 mm: set drv_stack and rerun.",
             "Antenna: the XIAO ESP32-C5's stick-on antenna (40 x 20 mm, 80 mm lead) goes on the inside of "
             + ("the pod's right wall (seen from the front), toward the back, clear of the switch window."
                if sd else "the pod's left wall (seen from the front), away from the cell and the board.")]
    if sd:
        notes.append(f"microSD: the breakout ({sdl:.1f} x {sdh:.1f} mm, e.g. Adafruit 4682) stands on edge on "
                     "the tray, left of the driver board seen from the front, socket toward the left wall, "
                     "card end against the bottom wall, stuck to the tray's fin with 0.5 mm foam tape. The "
                     f"card goes in through the slot in the bottom edge, {cardx - ox0:.1f} mm from the left "
                     "side. Wires to the driver board's pad row: 3V3, GND, SCK = D8, MOSI = D10, MISO = D9, "
                     "CS = D7.")
    if nkeys:
        notes.append("Keys: three B3F-1000 (PREV = D4, MENU = D7, NEXT = D9) on an 8 x 40 mm perfboard strip "
                     "glued to the rail on the shelf, plungers against the printed caps in the right wall.")
    else:
        notes.append("No keys: pages turn from the phone over Bluetooth, on the auto-turn timer, or with the "
                     "XIAO's BOOT button as a single next-page button.")
    if boot:
        notes.append(f"BOOT pinhole: d{boot[2]:.1f} mm in the back of the pod, {abs(boot[0]):.1f} mm "
                     f"{'right' if boot[0] >= 0 else 'left'} of the centreline seen from the front "
                     f"(mirrored when you look at the back) and {d0y + boot[1] - oy0:.1f} mm above the bottom "
                     "edge. Press with a paperclip.")
    if sw:
        notes.append(f"Battery-switch window: {sw['w']:.0f} x {sw['h']:.0f} mm in the "
                     f"{'left' if sw['wall'] == 'L' else 'right'} wall (seen from the front), centred "
                     f"{d0y + sw['y'] - oy0:.1f} mm above the bottom edge. The switch is about "
                     f"{(ix1 - d1x) if sw['wall'] == 'R' else (d0x - ix0):.0f} mm inside: work it with a toothpick.")
    measure = ["glass, aa_border, tail: from Good Display's GDEY037T03 drawing, not from the part",
               "cell: 603450 pouch, swells to about 6.3 mm",
               "drv, drv_pcb_z, drv_switch: from Seeed's STEP model of the driver board, not from the part",
               "drv_stack: 27 mm measured by ruler with the XIAO ESP32-C5 on long pins; the models give "
               "14.4 mm on standard pins. Re-measure for another XIAO",
               "usb_over: USB-C 2.5 mm past the board's end from the two board models (about 4 by ruler); "
               "the channel takes either",
               "coupler: FFC coupler board size, assumed 20 x 12 x 3.6, and that the tail reaches its mouth "
               "(tail_reach)"]
    if boot:
        measure.append("boot_pin: from Seeed's KiCad board for the XIAO and the driver board's STEP model")
    if sd:
        measure.append("sd: the breakout's size is Adafruit's overall figure; which edge the card enters, "
                       "where along it, how far the card stands proud and the socket height are assumed")
    measure.append("battery sense: the XIAO ESP32-C5 reads its own battery pads (GPIO6, enabled by GPIO26); "
                   "whether that sees a cell on the driver board's JST is unverified")
    layout = dict(outer=(ox0, ox1, oy0, oy1), z=(zbo, zsho, zs, ztop), ystep=ystep + w,
                  screws=[(x, y) for x in bx for y in by], lanyard=(gcx - 7.0, gcx + 7.0, slot_y - 1.5, slot_y + 1.5),
                  window=(ax0 - 0.4, ax1 + 0.4, ay0 - 0.4, ay1 + 0.4),
                  tray=(tx0, tx1, ty0, ty1, zst - tray_t, zst),
                  channel=(usb_x - chw / 2, usb_x + chw / 2, oy0, yport, zbo, zch),
                  jumper=(gcx - 6.25, gcx + 6.25, d1y, q0y + 3.0, zst - ct - 0.6, zst - ct - 0.3),
                  boot=(gcx + boot[0], d0y + boot[1]) if boot else None,
                  switch_win=((d0y + sw["y"], sw["w"], sw["wall"]) if sw else None),
                  sdslot=((cardx, cardz, sd["slot"][0], sd["slot"][1]) if sd else None))
    info = dict(layout=layout, outer=(ox1 - ox0, oy1 - oy0, ztop - zbo), z=(zbo, zs, ztop),
                screws="4x M2 pan head, thread-forming for plastic, through the back into the front bosses: "
                       "M2x6 at the top (shelf), M2x6 at the bottom down the pod's deep counterbores",
                notes=notes, measure=measure,
                print="PETG, 0.2 mm layers, 4 walls, 25% gyroid. Front shell face down (bezel on the bed). "
                      "Back shell back down: the pod sits on the bed and the shelf hangs "
                      f"{zsho - zbo:.1f} mm above it, so turn on supports touching the bed only (they come "
                      "off the flat shelf back cleanly). Tray plate down"
                      + ("; key caps flange down." if nkeys else "."))
    return parts, refs, info


# ---------------------------------------------------------------------------------- SR-16L
# SR-16N made long instead of thick. Nothing is stacked:
#   - the glass keeps its ribbon tail at the BOTTOM (no `epd-rot!`); the tail runs straight,
#     unfolded, into the driver board, which lies in a chin below the glass with its components
#     facing the FRONT (the tail's contacts face the front, the connector is top-contact);
#   - the XIAO is not plugged into the driver board. A bare XIAO (no pin headers) lies flat
#     behind the bottom of the glass, components to the back, USB-C out of the side wall, wired
#     to the driver board's two pad rows (CN1/CN2 on Seeed's schematic carry every XIAO pin):
#     5V, GND, 3V3, D0, D1, D2, D3, D8, D10;
#   - the cell stays on the driver board's socket and switch (its ETA9740 charges from the
#     XIAO's 5 V pin and boosts the cell back to it);
#   - the driver board's parts stand 4.8 mm in front of its face, so the chin has a printed cap
#     that stands proud of the bezel. No coupler, no jumper, no tray.
SR16L = dict(SR16N, key="SR-16L", name="Setlist, long", keys=0,
             aa_border=(2.98, 2.98, 2.98, 8.47),   # tail at the bottom: left, right, top, bottom
             band_top=9.0,
             tail_len=10.15, tail_in=3.5,          # the tail, and how much of it the connector takes
             tail_z=-0.5,                          # MEASURE: the tail's plane behind the glass front
             fpc_slot=1.0,                         # MEASURE: the connector's slot above the board's face
             xiao=(21.0, 17.8, 1.0),               # XIAO ESP32-C5 board: along its USB axis, across, thick
             xiao_usb=2.3,                         # its USB-C past the board's edge (KiCad)
             xiao_y=3.0,                           # XIAO's lower edge above the glass's lower edge
             usb_wall="R",                         # wall the USB-C comes out of, seen from the front
             boot_pin=None, drv_switch=None, sd=None)


def build_sr16_long(v, ref_dir):
    gw, gh, gt = v["glass"]
    w, cr = v["wall"], v["corner_r"]
    K = v["key"]
    dw, dh = v["drv"]
    gap = v["tail_len"] - v["tail_in"] - 0.86          # glass edge to the board's ribbon end
    chin = 3.5 + dh + gap                              # inside length of the chin
    ix0 = 0.0; gx0 = 0.3; gx1 = gx0 + gw; ix1 = gx1 + 0.5
    iy0 = 0.0; gy0 = chin; gy1 = gy0 + gh; iy1 = gy1 + 0.3 + v["band_top"]
    dx, dy = -(ix0 + ix1) / 2, -(iy0 + iy1) / 2
    ix0, ix1, gx0, gx1 = ix0 + dx, ix1 + dx, gx0 + dx, gx1 + dx
    iy0, iy1, gy0, gy1 = iy0 + dy, iy1 + dy, gy0 + dy, gy1 + dy
    ox0, ox1, oy0, oy1 = ix0 - w, ix1 + w, iy0 - w, iy1 + w
    gcx = (gx0 + gx1) / 2
    zf = 0.1; ztop = zf + v["bezel"]; zg = -gt; zst = zg - 0.3; zs = zg - 0.5
    cw, ch, ct = v["cell"]
    zfl = zst - (ct + 1.3) - 0.2                       # back floor (inside)
    zbo = zfl - v["back_wall"]
    # driver board, components to the front
    zpt = v["tail_z"] - v["fpc_slot"]                  # its component face
    zpb = zpt - 1.6                                    # its back
    d1y = gy0 - gap; d0y = d1y - dh
    d0x, d1x = gcx - dw / 2, gcx + dw / 2
    zci = zpt + 4.8 + 0.4                              # chin cap, inside
    zco = zci + 1.2
    # opening in the front frame for the cap, and the cap
    px0, px1, py0, py1 = ix0 + 5.0, ix1 - 5.0, iy0 + 1.5, gy0 - 2.5
    # XIAO flat behind the glass, components to the back
    xl, xw, xt = v["xiao"]
    right = v["usb_wall"] == "R"
    xe = (ix1 - 1.0) if right else (ix0 + 1.0)         # its USB edge, 1 mm inside the wall
    xa0, xa1 = (xe - xl, xe) if right else (xe, xe + xl)
    xb0 = gy0 + v["xiao_y"]; xb1 = xb0 + xw; xcy = (xb0 + xb1) / 2
    zxf = zst - 0.2; zxb = zxf - xt                    # XIAO board front, back
    usb_z = zxb - 1.6
    sgn = -1 if right else 1                           # from the USB edge into the board
    boot = (xe + sgn * 19.59, xcy + (4.4 if right else -4.4))
    rst = (xe + sgn * 19.59, xcy - (4.4 if right else -4.4))
    # cell above the XIAO
    c0x, c1x = gcx - cw / 2, gcx + cw / 2
    c0y = xb1 + 2.0; c1y = c0y + ch
    assert c1y + 3.0 <= gy1, "cell runs off the glass"
    slot_y = iy1 - 4.5
    bx = (ox0 + w + 2.0, ox1 - w - 2.0); by = (oy0 + w + 2.0, oy1 - w - 2.0)
    # the driver board's switch, seen from the front: left edge, 26.3-33.1 from the XIAO end
    swx, swy = d0x + 1.2, d0y + 29.7
    rests = [(d0x, d0x + 2.5, d1y - 2.5, d1y), (d1x - 2.5, d1x, d1y - 2.5, d1y),
             (d0x + 9.0, d0x + 16.0, d0y, d0y + 1.2)]  # bare corners of the board, and its end between the sockets

    def profile(name, z0, z1):
        return rrect(name, ox0, ox1, oy0, oy1, z0, z1, cr)

    def common_cuts(ob):
        a0, a1 = (ix1 - EPS, ox1 + 1) if right else (ox0 - 1, ix0 + EPS)
        r0, r1 = (ox1 - 1.0, ox1 + 1) if right else (ox0 - 1, ox0 + 1.0)
        cut(ob, rrect("slot", gcx - 7.0, gcx + 7.0, slot_y - 1.5, slot_y + 1.5, zbo - 1, ztop + 1, 1.5),
            edge_cut("usb", "X", xcy, usb_z, 9.8, 3.9, a0, a1, 1.6),
            edge_cut("usbr", "X", xcy, usb_z, 12.8, 7.0, r0, r1, 2.6))

    a_l, a_r, a_t, a_b = v["aa_border"]
    ax0, ax1, ay0, ay1 = gx0 + a_l, gx1 - a_r, gy0 + a_b, gy1 - a_t
    front = profile(K + "-front", zs, ztop)
    cut(front,
        rrect("cav", ix0, ix1, iy0, iy1, zs - 1, zg + EPS, 1.0),
        rrect("cav_glass", gx0, gx1 + 0.3, gy0 - 0.3, gy1 + 0.3, zg - EPS, zf, 0.5),
        rrect("cav_chin", ix0, ix1, iy0, gy0 - 1.3, zg - EPS, zf, 1.0),
        box("cav_tail", gcx - v["tail_w"] / 2 - 1.0, gcx + v["tail_w"] / 2 + 1.0, gy0 - 1.4, gy0 - 0.2, zg - EPS, zf))
    for x in bx:
        for y in by:
            add(front, cyl("boss", x, y, 1.8, zs, zf + EPS, 32))
    add(front, rrect("slotboss", gcx - 9.0, gcx + 9.0, slot_y - 3.5, slot_y + 3.5, zs, zf + EPS, 2.0))
    keep(front, profile("trim", zs - 1, ztop + 1))
    cut(front, frustum("window", ax0 - 0.4, ax1 + 0.4, ay0 - 0.4, ay1 + 0.4, 0.9, zf - EPS, ztop + EPS),
        rrect("capwin", px0, px1, py0, py1, zf - 1, ztop + 1, 2.0))
    for x in bx:
        for y in by:
            cut(front, cyl("pilot", x, y, 0.8, zs - 1, zf + 0.9, 24))
    common_cuts(front)

    # chin cap: a flange held under the front frame, a dome through its opening, rests for the board
    cap = rrect(K + "-chincap", ix0 + 0.4, ix1 - 0.4, iy0 + 0.3, gy0 - 1.6, zf - 1.0, zf, 1.0)
    add(cap, rrect("dome", px0 + 0.25, px1 - 0.25, py0 + 0.25, py1 - 0.25, zf - EPS, zco, 1.8))
    cut(cap, rrect("capcav", px0 + 1.45, px1 - 1.45, py0 + 1.45, py1 - 1.45, zf - 2, zci, 1.0),
        box("captail", gcx - 9.0, gcx + 9.0, py1 - 1.6, gy0, zf - 2, zf + EPS),
        rrect("swslot", swx - 2.0, swx + 2.0, swy - 5.5, swy + 5.5, zci - 1, zco + 1, 1.0))
    for x in bx:
        cut(cap, cyl("capboss", x, by[0], 2.3, zf - 2, zf + 1, 32))
    add(cap, *[box("rest", r[0], r[1], r[2], r[3], zpt, zci + EPS) for r in rests])

    back = profile(K + "-back", zbo, zs)
    cut(back, rrect("cav", ix0, ix1, iy0, iy1, zfl, zs + 1, 1.0))
    feats = []
    for x in bx:
        for y in by:
            feats.append(cyl("bboss", x, y, 2.8, zfl - EPS, zs, 40))
    feats.append(rrect("slotboss", gcx - 9.0, gcx + 9.0, slot_y - 3.5, slot_y + 3.5, zfl - EPS, zs, 2.0))
    # the driver board: posts under the three rests, a fence round its outline
    feats += [box("post", r[0], r[1], r[2], r[3], zfl - EPS, zpb) for r in rests]
    for x0, x1 in ((d0x - 1.6, d0x - 0.3), (d1x + 0.3, d1x + 1.6)):
        for y0, y1 in ((d0y + 1.0, d0y + 6.0), (d1y - 6.0, d1y - 1.0)):
            feats.append(box("fence", x0, x1, y0, y1, zfl - EPS, zpt - 0.2))
    feats.append(box("stop", d0x + 9.0, d0x + 16.0, d0y - 1.5, d0y - 0.3, zfl - EPS, zpt - 0.2))
    for x0, x1 in ((d0x, d0x + 3.5), (d1x - 3.5, d1x)):
        feats.append(box("stop", x0, x1, d1y + 0.3, d1y + 1.5, zfl - EPS, zpt - 0.2))
    # the XIAO: a fence on three sides; foam tape holds it to the glass, the wall hole takes the USB-C
    fx0, fx1 = (xa0 + 3.0, xa0 + 12.0)
    feats += [box("xfence", fx0, fx1, xb0 - 1.1, xb0 - 0.3, zfl - EPS, zxb + 0.4),
              box("xfence", fx0, fx1, xb1 + 0.3, xb1 + 1.1, zfl - EPS, zxb + 0.4)]
    ex = (xa0 - 1.1, xa0 - 0.3) if right else (xa1 + 0.3, xa1 + 1.1)
    feats.append(box("xfence", ex[0], ex[1], xcy - 4.0, xcy + 4.0, zfl - EPS, zxb + 0.4))
    # cell fence
    zrib = zst - ct + 1.0
    feats += [box("rib", c0x + 6, c1x - 6, c1y + 0.2, c1y + 1.0, zfl - EPS, zrib),
              box("rib", c0x - 1.0, c0x - 0.2, c0y + 6, c1y - 6, zfl - EPS, zrib),
              box("rib", c1x + 0.2, c1x + 1.0, c0y + 6, c1y - 6, zfl - EPS, zrib)]
    feats = [f for f in feats if True]
    add(back, *feats)
    keep(back, profile("trim", zbo - 1, zs + 1))
    for x in bx:
        for y in by:
            cut(back, cyl("clear", x, y, 1.15, zbo - 1, zs + 1, 24), cyl("cbore", x, y, 1.95, zbo - 1, zs - 4.0, 32))
    cut(back, cyl("bootpin", boot[0], boot[1], 0.9, zbo - 1, zfl + 1, 24),
        cyl("rstpin", rst[0], rst[1], 0.9, zbo - 1, zfl + 1, 24))
    common_cuts(back)

    refs = [box("ref-glass", gx0, gx1, gy0, gy1, zg, 0.0),
            box("ref-tail", gcx - v["tail_w"] / 2, gcx + v["tail_w"] / 2, d1y - 0.86 - v["tail_in"], gy0,
                v["tail_z"] - 0.15, v["tail_z"] + 0.15),
            box("ref-driver-pcb", d0x, d1x, d0y, d1y, zpb, zpt),
            box("ref-drv-socket", d0x + 19.27, d0x + 21.81, d0y + 1.45, d0y + 19.25, zpt, zpt + 4.5),
            box("ref-drv-socket", d0x + 3.41, d0x + 5.95, d0y + 1.45, d0y + 19.25, zpt, zpt + 4.5),
            box("ref-drv-switch", d0x - 0.8, d0x + 3.2, d0y + 26.3, d0y + 33.1, zpt, zpt + 1.42),
            box("ref-drv-jst", d0x + 14.86, d0x + 21.15, d0y + 25.5, d0y + 33.7, zpt, zpt + 4.8),
            box("ref-drv-fpc", d0x + 4.72, d0x + 20.24, d0y + 36.2, d0y + 41.1, zpt, zpt + 2.0),
            box("ref-xiao", xa0, xa1, xb0, xb1, zxb - 2.4, zxf),
            box("ref-usb-c", (xe - 5.0) if right else (xe - v["xiao_usb"]), (xe + v["xiao_usb"]) if right else (xe + 5.0),
                xcy - 4.5, xcy + 4.5, zxb - 3.2, zxb),
            box("ref-cell", c0x, c1x, c0y, c1y, zst - ct, zst)]
    parts = dict(front=front, back=back, chincap=cap)
    T, Tc = ztop - zbo, zco - zbo
    side = "right" if right else "left"
    notes = ["Glass drops into the front shell face-down, taped to the bezel border with 0.2 mm "
             "double-sided tape, with its ribbon tail at the BOTTOM (rotation 0, the firmware's default).",
             "Chin cap: drop it into the front shell from behind, dome through the opening, before the "
             "driver board. Its flange sits under the frame; nothing else holds it but the board behind it.",
             "Driver board: components toward the FRONT, ribbon connector toward the glass. Slide the "
             f"tail straight into the connector ({v['tail_in']:.1f} mm, no fold, no bend) and latch it, then "
             "lay the board on the cap's three rests. The back shell's posts and fence hold it there.",
             "XIAO: a bare XIAO ESP32-C5 with NO pin headers, on 0.2 mm foam tape on the back of the glass, "
             f"components to the back, USB-C through the {side} wall (seen from the front), "
             f"{xcy - oy0:.1f} mm above the bottom edge.",
             "Wires, 30 AWG, about 45 mm, XIAO pad to the same-named pad in the driver board's two pad "
             "rows: 5V, GND, 3V3, D0, D1, D2, D3, D8, D10. Solder them to the back of the driver board.",
             "Cell: 603450 lying across, behind the glass above the XIAO, plugged into the driver board's "
             "socket. Its lead goes round the board's edge into the cap.",
             "Battery switch: the driver board's own, through the slot in the chin cap, with a toothpick.",
             f"BOOT and RESET pinholes: d1.8 mm in the back, {abs(boot[0] - gcx):.1f} mm "
             f"{'left' if boot[0] < gcx else 'right'} of the centreline seen from the front, "
             f"{boot[1] - oy0:.1f} mm (BOOT) and {rst[1] - oy0:.1f} mm (RESET) above the bottom edge.",
             "Antenna: the stick-on antenna goes on the inside of the back, above the cell.",
             f"Thickness: {T:.1f} mm, and {Tc:.1f} mm at the chin, whose cap stands {zco - ztop:.1f} mm proud "
             "of the bezel."]
    measure = ["tail_z, fpc_slot: where the tail leaves the glass and how high the connector's slot is above "
               "the board. Together they set the board's depth; a wrong figure bends the tail",
               "tail_in: how far the tail goes into the connector, which sets the gap between glass and board",
               "xiao, xiao_usb: from Seeed's KiCad board; the USB-C hole has no channel, so check it",
               "the XIAO must be bare: one with headers soldered on does not fit this shell",
               "driver board: from Seeed's STEP model; which pads are CN1/CN2 is read from the schematic, "
               "not checked on the board",
               "cell: 603450 pouch, swells to about 6.3 mm"]
    marks = [("Battery switch slot, in the chin cap", "4 x 11 mm", (swx - 2.0, swx + 2.0, swy - 5.5, swy + 5.5)),
             ("USB-C opening, in the side wall", "9.8 x 3.9 mm",
              ((ox1 - 1.2, ox1 + 1.2) if right else (ox0 - 1.2, ox0 + 1.2)) + (xcy - 4.9, xcy + 4.9)),
             ("BOOT pinhole, through the back", "d1.8 mm", (boot[0] - 1, boot[0] + 1, boot[1] - 1, boot[1] + 1)),
             ("RESET pinhole, through the back", "d1.8 mm", (rst[0] - 1, rst[0] + 1, rst[1] - 1, rst[1] + 1))]
    layout = dict(outer=(ox0, ox1, oy0, oy1), z=(zbo, zbo, zs, ztop),
                  profile=[(ztop, oy1), (zbo, oy1), (zbo, oy0), (ztop, oy0), (ztop, py0), (zco, py0), (zco, py1),
                           (ztop, py1)],
                  screws=[(x, y) for x in bx for y in by], lanyard=(gcx - 7.0, gcx + 7.0, slot_y - 1.5, slot_y + 1.5),
                  window=(ax0 - 0.4, ax1 + 0.4, ay0 - 0.4, ay1 + 0.4), marks=marks,
                  cap=(px0, px1, py0, py1),
                  caption=f"{T:.1f} mm thick, {Tc:.1f} mm at the chin; {oy1 - oy0:.1f} mm tall")
    info = dict(layout=layout, outer=(ox1 - ox0, oy1 - oy0, Tc), z=(zbo, zs, ztop),
                screws="4x M2x6 pan head, thread-forming for plastic, through the back into the front bosses",
                notes=notes, measure=measure,
                print="PETG, 0.2 mm layers, 4 walls, 25% gyroid. Front shell face down, back shell back down, "
                      "chin cap dome down. No supports.")
    return parts, refs, info


# ------------------------------------------------------------------------------- outputs

def print_pose(ob, part):
    """Copy of `ob` lying on the bed (z=0) the way it should be printed."""
    c = ob.copy(); c.data = ob.data.copy()
    bpy.context.scene.collection.objects.link(c)
    if not part.startswith("keycap"):          # key caps are modelled flange-down already
        c.data.transform(c.matrix_world)
    c.matrix_world = Matrix.Identity(4)
    if part in ("front", "tray", "chincap"):               # face down: bezel face (or the tray's plate) on the bed
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


LAYOUT_PARTS = [  # ref name prefix, label, fill
    ("ref-glass", "E-paper glass", "#9db4c8"), ("ref-tail", "Glass ribbon tail, straight", "#e8b04a"), ("ref-cell", "Battery, 603450 LiPo", "#e0a84a"),
    ("ref-coupler", "FFC coupler board (ribbon extender)", "#c77dd1"),
    ("ref-driver-pcb", "ePaper driver board", "#4f9a63"), ("ref-xiao", "XIAO ESP32-C5", "#d96a5a"),
    ("ref-usb-c", "USB-C port", "#8a8a8a"), ("ref-drv-fpc", "Ribbon connector", "#e8e2c8"),
    ("ref-drv-jst", "Battery socket (JST)", "#f1efe6"), ("ref-drv-switch", "Battery switch", "#3b3b3b"),
    ("ref-drv-socket", "XIAO sockets (unused)", "#2a2a2a"),
    ("ref-sd-board", "microSD breakout, on edge", "#4a7fd0"), ("ref-sd-card", "microSD card", "#20304a"),
    ("ref-perf", "Key strip (perfboard)", "#b08b5a"), ("ref-key", "Key", "#555555"),
]

def layout_svg(path, v, refs, info):
    """Mechanical layout: the pass seen from the back with the back shell removed, and a side
    section, every component outlined to scale and numbered against a legend."""
    L = info["layout"]
    ox0, ox1, oy0, oy1 = L["outer"]
    zbo, zsho, zs, ztop = L["z"]
    K, M = 5.0, 14.0                             # px per mm, margin in mm
    W, H, T = ox1 - ox0, oy1 - oy0, ztop - zbo
    bxo, sxo = M, M + W + 22.0                   # left edge of the back view, of the side view
    def bx_(x): return (bxo + (ox1 - x)) * K     # back view: x mirrored, the way you look at the back
    def y_(y): return (M + 8 + (oy1 - y)) * K
    zfront = max([ztop] + [z for z, _ in (L.get("profile") or [])])
    def sx_(z): return (sxo + (zfront - z)) * K  # side view: front (glass) on the left
    out, items = [], []
    def rect(x0, x1, y0, y1, fill, stroke="#222", sw=1.0, dash="", op=1.0):
        xa, xb, ya, yb = min(x0, x1), max(x0, x1), min(y0, y1), max(y0, y1)
        out.append(f'<rect x="{xa:.1f}" y="{ya:.1f}" width="{xb - xa:.1f}" height="{yb - ya:.1f}" fill="{fill}" '
                   f'fill-opacity="{op}" stroke="{stroke}" stroke-width="{sw}"'
                   + (f' stroke-dasharray="{dash}"' if dash else "") + "/>")
    def text(x, y, s, size=12, anchor="start", weight="normal", fill="#111"):
        out.append(f'<text x="{x:.1f}" y="{y:.1f}" font-size="{size}" text-anchor="{anchor}" font-weight="{weight}" '
                   f'fill="{fill}">{s}</text>')
    def tag(x, y, n):
        out.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="8" fill="#fff" stroke="#111" stroke-width="1"/>')
        text(x, y + 4, str(n), 11, "middle", "bold")
    # shells
    rect(bx_(ox0), bx_(ox1), y_(oy0), y_(oy1), "#f4f4f1", "#111", 1.6)
    if L.get("ystep") is not None:
        rect(bx_(ox0), bx_(ox1), y_(oy0), y_(L["ystep"]), "#e2e2dc", "#111", 1.0, "6 3")
    prof = L.get("profile") or [(ztop, oy1), (zsho, oy1), (zsho, L["ystep"]), (zbo, L["ystep"]), (zbo, oy0), (ztop, oy0)]
    out.append('<path d="M' + " L".join(f"{sx_(z):.1f},{y_(y):.1f}" for z, y in prof)
               + ' Z" fill="#f4f4f1" stroke="#111" stroke-width="1.6"/>')
    if L.get("cap"):
        cp = L["cap"]; rect(bx_(cp[0]), bx_(cp[1]), y_(cp[2]), y_(cp[3]), "#e2e2dc", "#111", 1.0, "6 3")
    out.append(f'<line x1="{sx_(zs):.1f}" y1="{y_(oy0):.1f}" x2="{sx_(zs):.1f}" y2="{y_(oy1):.1f}" stroke="#111" '
               'stroke-width="0.8" stroke-dasharray="2 3"/>')
    a = L["window"]; rect(bx_(a[0]), bx_(a[1]), y_(a[2]), y_(a[3]), "none", "#3a6ea5", 1.0, "3 3")
    extra = []
    if L.get("tray"):
        t5 = L["tray"]; rect(bx_(t5[0]), bx_(t5[1]), y_(t5[2]), y_(t5[3]), "#d9c9a8", "#7a6a48", 0.8, "", 0.55)
        rect(sx_(t5[4]), sx_(t5[5]), y_(t5[2]), y_(t5[3]), "#d9c9a8", "#7a6a48", 0.8)
        c = L["channel"]; rect(bx_(c[0]), bx_(c[1]), y_(c[2]), y_(c[3]), "#ffffff", "#c0392b", 1.2, "4 2")
        rect(sx_(c[4]), sx_(c[5]), y_(c[2]), y_(c[3]), "#ffffff", "#c0392b", 1.2, "4 2")
        j = L["jumper"]; rect(bx_(j[0]), bx_(j[1]), y_(j[2]), y_(j[3]), "#f0d9a0", "#a07a20", 0.8, "", 0.8)
        rect(sx_(j[4]), sx_(j[5]), y_(j[2]), y_(j[3]), "#f0d9a0", "#a07a20", 0.8)
        extra = [("Tray (printed)", "#d9c9a8", t5[:4]), ("USB-C cable channel, open to the back", "#ffffff", c[:4]),
                 ("FFC jumper, 60 mm, over the battery", "#f0d9a0", j[:4])]
    n = 0
    for pref, label, fill in LAYOUT_PARTS:
        hits = [r for r in refs if r.name.startswith(pref)]
        if not hits:
            continue
        n += 1
        for i, r in enumerate(hits):
            co = [r.matrix_world @ vv.co for vv in r.data.vertices]
            x0, x1 = min(p.x for p in co), max(p.x for p in co)
            y0, y1 = min(p.y for p in co), max(p.y for p in co)
            z0, z1 = min(p.z for p in co), max(p.z for p in co)
            rect(bx_(x0), bx_(x1), y_(y0), y_(y1), fill, "#222", 0.9, "", 0.9)
            rect(sx_(z0), sx_(z1), y_(y0), y_(y1), fill, "#222", 0.9, "", 0.9)
            if i == 0:
                tag(bx_((x0 + x1) / 2), y_((y0 + y1) / 2), n)
                tag(sx_((z0 + z1) / 2), y_((y0 + y1) / 2), n)
                items.append((n, label, fill, f"{x1 - x0:.1f} x {y1 - y0:.1f} x {z1 - z0:.1f} mm"))
    for label, fill, b in extra:
        n += 1
        tag(bx_(b[1]) + 14 if "Tray" in label else bx_((b[0] + b[1]) / 2), y_(b[2]) - 12 if "Tray" in label else y_((b[2] + b[3]) / 2), n)
        items.append((n, label, fill, f"{b[1] - b[0]:.1f} x {b[3] - b[2]:.1f} mm"))
    n += 1
    for x, y in L["screws"]:
        out.append(f'<circle cx="{bx_(x):.1f}" cy="{y_(y):.1f}" r="{1.0 * K:.1f}" fill="#fff" stroke="#111" stroke-width="1.2"/>')
        out.append(f'<path d="M{bx_(x) - 4:.1f},{y_(y):.1f} h8 M{bx_(x):.1f},{y_(y) - 4:.1f} v8" stroke="#111" stroke-width="1"/>')
    tag(bx_(L["screws"][0][0]) - 16, y_(L["screws"][0][1]) - 14, n)
    items.append((n, "Screws, 4 x M2x6 thread-forming", "#ffffff", "through the back into the front"))
    n += 1
    a = L["lanyard"]; rect(bx_(a[0]), bx_(a[1]), y_(a[2]), y_(a[3]), "#ffffff", "#111", 1.2)
    tag(bx_(a[0]) + 16, y_((a[2] + a[3]) / 2), n); items.append((n, "Lanyard slot", "#ffffff", "14 x 3 mm"))
    for label, dims, b in L.get("marks", []):
        n += 1
        rect(bx_(b[0]), bx_(b[1]), y_(b[2]), y_(b[3]), "#ffffff", "#c0392b", 1.5)
        tag(bx_(b[0]) + 16, y_((b[2] + b[3]) / 2), n); items.append((n, label, "#ffffff", dims))
    if L.get("cap"):
        n += 1; cp = L["cap"]
        tag(bx_(cp[0]) - 12, y_(cp[3]) + 12, n)
        items.append((n, "Chin cap (printed), dashed", "#e2e2dc", f"{cp[1] - cp[0]:.1f} x {cp[3] - cp[2]:.1f} mm"))
    if L.get("boot"):
        n += 1
        out.append(f'<circle cx="{bx_(L["boot"][0]):.1f}" cy="{y_(L["boot"][1]):.1f}" r="4.5" fill="#fff" stroke="#c0392b" stroke-width="1.5"/>')
        tag(bx_(L["boot"][0]) + 18, y_(L["boot"][1]), n)
        items.append((n, "BOOT pinhole, through the back", "#ffffff", "d1.8 mm"))
    if L.get("switch_win"):
        n += 1
        yc, wl, wall = L["switch_win"]
        xw = ox1 if wall == "R" else ox0
        rect(bx_(xw) - 4, bx_(xw) + 4, y_(yc - wl / 2), y_(yc + wl / 2), "#ffffff", "#c0392b", 1.5)
        tag(bx_(xw) + (16 if wall == "R" else -16), y_(yc), n)
        items.append((n, "Window to the battery switch, in the side wall", "#ffffff", f"{wl:.0f} x 5 mm"))
    if L.get("sdslot"):
        n += 1
        cx, cz, sl, sw_ = L["sdslot"]
        rect(bx_(cx - sw_ / 2), bx_(cx + sw_ / 2), y_(oy0) - 4, y_(oy0) + 4, "#ffffff", "#c0392b", 1.5)
        rect(sx_(cz - sl / 2), sx_(cz + sl / 2), y_(oy0) - 4, y_(oy0) + 4, "#ffffff", "#c0392b", 1.5)
        tag(bx_(cx), y_(oy0) + 20, n); items.append((n, "Card slot, in the bottom edge", "#ffffff", f"{sl:.1f} x {sw_:.1f} mm"))
    # titles, dimensions, legend
    text(M * K, 24, f"{v['key']} {v['name']}: mechanical layout, rev D", 18, "start", "bold")
    text(bx_(ox1), y_(oy1) - 10, "BACK VIEW, back shell off", 12, "start", "bold")
    text(bx_(ox1), y_(oy1) - 26, "left and right are swapped against the front", 11, "start", "normal", "#555")
    text(sx_(ztop), y_(oy1) - 10, "SIDE SECTION, glass on the left", 12, "start", "bold")
    text((bx_(ox0) + bx_(ox1)) / 2, y_(oy0) + 44, f"{W:.1f} mm wide", 12, "middle")
    text(sx_(max(z for z, _ in prof) if L.get("profile") else ztop), y_(oy0) + 44,
         L.get("caption") or f"{T:.1f} mm at the pod, {ztop - zsho:.1f} mm above it; {H:.1f} mm tall", 12)
    if L.get("ystep") is not None:
        text(bx_(ox1) + 6, y_(L["ystep"]) - 6, "pod (dashed): below this line", 11, "start", "normal", "#444")
    lx, ly = (sxo + (zfront - zbo) + 16) * K, y_(oy1) + 6
    for num, label, fill, dims in items:
        out.append(f'<rect x="{lx:.1f}" y="{ly - 11:.1f}" width="14" height="14" fill="{fill}" stroke="#222" stroke-width="0.8"/>')
        text(lx + 22, ly, f"{num}", 12, "start", "bold"); text(lx + 44, ly, label, 12)
        text(lx + 44, ly + 14, dims, 11, "start", "normal", "#555")
        ly += 34
    text(lx, ly + 8, "Blue dashes: the panel's active area.", 11, "start", "normal", "#3a6ea5")
    text(lx, ly + 24, "Red: openings in the shell.", 11, "start", "normal", "#c0392b")
    text(lx, ly + 40, "Stand-in boxes, to scale; see the report for sources.", 11, "start", "normal", "#555")
    tw, th = lx + 330, max(y_(oy0) + 60, ly + 56)
    with open(path, "w") as f:
        f.write(f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {tw:.0f} {th:.0f}" width="{tw:.0f}" height="{th:.0f}" '
                'font-family="Helvetica, Arial, sans-serif">\n'
                f'<rect width="{tw:.0f}" height="{th:.0f}" fill="#ffffff"/>\n' + "\n".join(out) + "\n</svg>\n")


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
        if not n.startswith(("front", "back", "chincap", "tray")):
            continue
        for r in refs:
            ov = overlap(ob, r)
            flag = "" if ov < 0.5 else "   <-- CHECK"
            lines.append(f"  {n:6s} x {r.name:18s} {ov:8.2f}{flag}")
    lines += ["", "Assembly:"] + ["- " + n for n in info["notes"]]
    has_caps = any(n.startswith("keycap") for n in parts)
    if info.get("print"):
        lines += ["", "Print: " + info["print"], "",
                  "MEASURE before the final print: see the MEASURE notes at the top of make_enclosure.py."]
    else:
      lines += ["", "Print: PETG, 0.2 mm layers, 4 walls, 25% gyroid. Front shell face down (bezel on the bed),",
              "back shell back down; key caps flange down. No supports, except the USB-C and lanyard"
              if has_caps else "back shell back down. No supports, except the USB-C and lanyard",
              "slot bridges if your printer sags over 14 mm.", "",
              "MEASURE before the final print: see the MEASURE notes at the top of make_enclosure.py."]
    lines += ["- " + m for m in info.get("measure", [])]
    with open(os.path.join(out, f"{v['key']}-report.txt"), "w") as f:
        f.write("\n".join(lines) + "\n")
    print("\n".join(lines))
    coll = bpy.data.collections.new("reference electronics")
    s.collection.children.link(coll)
    for r in refs:
        for c in r.users_collection:
            c.objects.unlink(r)
        coll.objects.link(r)
    if info.get("layout"):
        layout_svg(os.path.join(out, f"{v['key']}-layout.svg"), v, refs, info)
    for r in refs:                              # things that plug in from outside were only there for
        if r.name in ("ref-usb-plug", "ref-sd-card"):   # the interference check: keep them out of sight
            r.hide_viewport = r.hide_render = True
    render_fit(os.path.join(out, f"{v['key']}-fit.png"), parts, refs, info)
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(out, f"{v['key']}.blend"))
    return bad


def main():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    ref_dir = None
    if "--ref" in argv:
        i = argv.index("--ref"); ref_dir = argv[i + 1]; del argv[i:i + 2]
    table = {"SR-15": (SR15, build_sr15), "SR-16": (SR16, build_sr16), "SR-16N": (SR16N, build_sr16),
             "SR-16SD": (SR16SD, build_sr16), "SR-16L": (SR16L, build_sr16_long)}
    todo = argv or list(table)
    unknown = [k for k in todo if k not in table]
    if unknown:
        sys.exit(f"unknown variant {', '.join(unknown)}; choose from {', '.join(table)}")
    bad = 0
    for k in todo:
        bad += run(*table[k], ref_dir)
    if bad:
        print(f"WARNING: {bad} non-manifold edges")


main()
