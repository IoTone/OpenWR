"""Convert Elecrow's CrowPanel STEP model to an STL of the board alone, for the fit check.

    uv venv -p 3.12 /tmp/occ && uv pip install -p /tmp/occ/bin/python cadquery-ocp
    curl -L -o cp42.stp "https://raw.githubusercontent.com/Elecrow-RD/CrowPanel-ESP32-4.2-E-paper-HMI-Display-with-400-300/HEAD/3D%20file/CrowPanel%20ESP32%204.2%E2%80%9D%20E-paper%20HMI%20Display.stp"
    /tmp/occ/bin/python step_to_stl.py cp42.stp REFDIR/cp42_board.stl
    Blender -b -P make_enclosure.py -- SR-15 --ref REFDIR

Elecrow's STEP includes the acrylic case it ships in (three plates and six posts); this drops
those and keeps the PCB, panel and parts. Elecrow's repo carries no license, so the STEP and
the STL stay out of this repo.
"""
import sys
from OCP.STEPControl import STEPControl_Reader
from OCP.TopExp import TopExp_Explorer
from OCP.TopAbs import TopAbs_SOLID
from OCP.Bnd import Bnd_Box
from OCP.BRepBndLib import BRepBndLib
from OCP.BRepMesh import BRepMesh_IncrementalMesh
from OCP.StlAPI import StlAPI_Writer
from OCP.BRep import BRep_Builder
from OCP.TopoDS import TopoDS_Compound
src, out = sys.argv[1], sys.argv[2]
r = STEPControl_Reader(); r.ReadFile(src); r.TransferRoots(); shp = r.OneShape()
comp = TopoDS_Compound(); bb = BRep_Builder(); bb.MakeCompound(comp); kept = dropped = 0
e = TopExp_Explorer(shp, TopAbs_SOLID)
while e.More():
    s = e.Current(); b = Bnd_Box(); BRepBndLib.Add_s(s, b, True)
    a, c = b.CornerMin(), b.CornerMax(); dx, dy, dz = c.X()-a.X(), c.Y()-a.Y(), c.Z()-a.Z()
    case_slab = (abs(dx-107.3) < .2 or abs(dx-53.3) < .2) and (a.Y() > -0.05 or a.Y() < -6.0 or abs(dy-4.0) < .05)
    post = abs(dx-4.5) < .05 and abs(dy-4.7) < .05 and abs(dz-4.5) < .05
    if case_slab or post: dropped += 1
    else: bb.Add(comp, s); kept += 1
    e.Next()
BRepMesh_IncrementalMesh(comp, 0.08, False, 0.3, True); StlAPI_Writer().Write(comp, out)
print(out, "kept", kept, "dropped", dropped)
