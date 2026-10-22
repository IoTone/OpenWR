// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// ZoneRing.ts — the heart-rate zone as a 240° gauge around the bpm.
//
// Five annular sectors built straight into meshes (2° steps, smooth at any
// size), opening at the bottom where the zone label sits. The geometry is
// built once; a zone change only recolours: zones below the current one in
// dim Alpenglow, the current one full Alpenglow and a little thicker, the
// rest a faint grey (black would be invisible on the additive display).
// ringSectors() is pure so it can be checked off the glasses
// (tools/ring_preview.js).

export interface Sector { vertices: number[]; indices: number[]; }

export const RING = { startDeg: 210, sweepDeg: 240, gapDeg: 5, stepDeg: 2 };

// Angles run clockwise from the start, measured like a clock face with 0° at
// the right and +90° at the top, so the opening is centred at the bottom.
export function ringSectors(radius: number, thickness: number, currentExtra: number, current: number): Sector[] {
  const out: Sector[] = [];
  const seg = (RING.sweepDeg - RING.gapDeg * 4) / 5;
  for (let i = 0; i < 5; i++) {
    const a0 = RING.startDeg - i * (seg + RING.gapDeg);
    const a1 = a0 - seg;
    const t = i === current - 1 ? thickness + currentExtra : thickness;
    const rIn = radius - t / 2, rOut = radius + t / 2;
    const steps = Math.max(2, Math.ceil(seg / RING.stepDeg));
    const vertices: number[] = [], indices: number[] = [];
    for (let k = 0; k <= steps; k++) {
      const a = (a0 + (a1 - a0) * k / steps) * Math.PI / 180;
      const c = Math.cos(a), s = Math.sin(a);
      vertices.push(rIn * c, rIn * s, 0, rOut * c, rOut * s, 0);
      if (k > 0) {
        const p = 2 * (k - 1);
        // counter-clockwise seen from +z (the viewer)
        indices.push(p, p + 2, p + 1, p + 1, p + 2, p + 3);
      }
    }
    out.push({ vertices, indices });
  }
  return out;
}

export function zoneColor(i: number, zone: number): [number, number, number, number] {
  if (i === zone - 1) return [0.941, 0.541, 0.384, 1];   // Alpenglow
  if (i < zone - 1) return [0.55, 0.30, 0.21, 1];        // Alpenglow, dimmed
  return [0.20, 0.20, 0.20, 1];                           // empty: faint, not black
}

@component
export class ZoneRing extends BaseScriptComponent {

  @input
  @hint("Unlit material (LegitUnlit); colour is set per sector")
  material: Material;

  @input
  @hint("Radius to the middle of the band, cm")
  radius: number = 5.2;

  @input
  @hint("Band thickness, cm")
  thickness: number = 0.45;

  @input
  @hint("Extra thickness of the current zone, cm")
  currentExtra: number = 0.35;

  private visuals: RenderMeshVisual[] = [];
  private zone = -1;

  onAwake() {
    this.build(0);
  }

  setZone(zone: number) {
    if (zone === this.zone) return;
    // thickness of the current sector changes with the zone: rebuild (5 small meshes)
    this.build(zone);
  }

  private build(zone: number) {
    this.zone = zone;
    for (const v of this.visuals) v.getSceneObject().destroy();
    this.visuals = [];
    const sectors = ringSectors(this.radius, this.thickness, this.currentExtra, zone);
    sectors.forEach((sec, i) => {
      const b = new MeshBuilder([{ name: "position", components: 3 }]);
      b.topology = MeshTopology.Triangles;
      b.indexType = MeshIndexType.UInt16;
      b.appendVerticesInterleaved(sec.vertices);
      b.appendIndices(sec.indices);
      b.updateMesh();
      const o = global.scene.createSceneObject("Zone" + (i + 1));
      o.setParent(this.getSceneObject());
      const vis = o.createComponent("Component.RenderMeshVisual") as RenderMeshVisual;
      vis.mesh = b.getMesh();
      vis.mainMaterial = this.material;
      const c = zoneColor(i, zone);
      (vis.mainPassOverrides as any).baseColor = new vec4(c[0], c[1], c[2], c[3]);
      this.visuals.push(vis);
    });
  }
}
