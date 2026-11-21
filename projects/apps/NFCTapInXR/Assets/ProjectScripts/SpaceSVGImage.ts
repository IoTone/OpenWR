// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// SpaceSVGImage.ts — Drop-in component to render an SVG into the scene.
//
// Setup in Lens Studio:
//   1. Create a SceneObject (e.g. "MyMermaid") and attach this script.
//   2. Assign an unlit material (e.g., LegitUnlit) to the "material" input.
//   3. Either:
//        a) Assign a Text component (svgTextSource) whose .text field holds the
//           raw SVG XML. Disable the SceneObject hosting that Text so it does
//           not render — we only read its .text property as a data carrier.
//           This works well for pasting an entire <svg>...</svg> document into
//           the Inspector (e.g. mermaid-diagram.svg.txt contents).
//        b) -OR- set svgInline directly in the Inspector (small snippets only).
//   4. The script parses the SVG on onAwake() and tessellates it into meshes
//      under this SceneObject.
//
// The component is fail-soft: parse errors are logged via print() and do not
// throw out of onAwake(). Call reload() at runtime to re-read the source after
// changing the Text contents programmatically.

import { SVGXMLParser, SpaceSVGMeshBackend } from './SpaceSVG';

@component
export default class SpaceSVGImage extends BaseScriptComponent {

  @input
  @hint('Unlit material for mesh rendering (e.g., LegitUnlit)')
  material!: Material;

  @input
  @allowUndefined
  @hint('Text component whose .text holds the raw SVG XML. Disable its SceneObject so it does not render — we read .text only.')
  svgTextSource: Text;

  @input
  @hint('Inline SVG XML (used when svgTextSource is empty or unassigned).')
  svgInline: string = '';

  @input
  @hint('World-space width of the rendered SVG')
  worldWidth: number = 20;

  @input
  @hint('World-space height of the rendered SVG')
  worldHeight: number = 20;

  @input
  @hint('Multiplier on stroke widths. 1 = faithful to SVG (often sub-pixel for large viewBoxes). Try 2–6 for visibility; values >8 may distort sharp corners.')
  strokeScale: number = 1.0;

  @input
  @hint('Comma-separated list of <g> id values whose entire subtree should be skipped. Useful for 3D-exported SVGs that include interior fold lines (e.g. "MeshIntersection,Crease,Material,Pupils,Boundary"). Match is exact and case-sensitive.')
  skipGroupIds: string = '';

  @input
  @hint('Remap near-black stroke colors to this RGB on AR. Spectacles is additive-only — true black emits no light and looks identical to occluded pink. White (1,1,1) gives high-contrast outlines; (0.2,0.1,0.15) reads as "dark" against pink fill. Set the alpha to 0 to leave black strokes alone.')
  darkStrokeColor: vec4 = new vec4(1, 1, 1, 1);

  @input
  @hint('Threshold for "near-black": if a stroke color\'s max RGB channel is at or below this, it gets remapped via darkStrokeColor.')
  darkStrokeThreshold: number = 0.1;

  @input
  @hint('Render automatically on Awake')
  autoRender: boolean = true;

  private parser = new SVGXMLParser();
  private backend = new SpaceSVGMeshBackend();
  private meshObjects: SceneObject[] = [];

  onAwake() {
    if (!this.material) {
      print('[SpaceSVGImage] ERROR: No material assigned.');
      return;
    }
    if (this.autoRender) {
      this.reload();
    }
  }

  /** Re-read the SVG source and re-render. Call after editing svgTextSource.text at runtime. */
  reload(): void {
    const svg = this.resolveSVG();
    if (!svg) {
      print('[SpaceSVGImage] No SVG source: assign svgTextSource (a Text component) or svgInline.');
      return;
    }
    this.render(svg);
  }

  /** Render an SVG string explicitly. Replaces any previously rendered content. */
  render(svgString: string): void {
    this.clear();
    try {
      const tree = this.parser.parse(svgString);
      const safeScale = (isFinite(this.strokeScale) && this.strokeScale > 0) ? this.strokeScale : 1;
      const skipIds = this.skipGroupIds
        ? this.skipGroupIds.split(',').map(s => s.trim()).filter(s => s.length > 0)
        : [];
      const groups = this.backend.buildMeshes(tree, this.worldWidth, this.worldHeight, { strokeScale: safeScale, skipGroupIds: skipIds });
      print(`[SpaceSVGImage] Rendered ${groups.length} mesh groups`);
      for (let i = 0; i < groups.length; i++) {
        this.remapDarkColor(groups[i]);
        this.createMesh(groups[i], i);
      }
    } catch (e: any) {
      print(`[SpaceSVGImage] Error: ${e.message || e}`);
    }
  }

  /** Destroy all mesh SceneObjects created by this component. */
  clear(): void {
    for (const obj of this.meshObjects) {
      obj.destroy();
    }
    this.meshObjects = [];
  }

  private remapDarkColor(group: { color: number[] }): void {
    if (!this.darkStrokeColor || this.darkStrokeColor.w <= 0) return;
    const maxRGB = Math.max(group.color[0], group.color[1], group.color[2]);
    if (maxRGB <= this.darkStrokeThreshold) {
      group.color[0] = this.darkStrokeColor.x;
      group.color[1] = this.darkStrokeColor.y;
      group.color[2] = this.darkStrokeColor.z;
      // Preserve any alpha from the original stroke
    }
  }

  private resolveSVG(): string {
    if (this.svgTextSource) {
      const t = this.svgTextSource.text;
      if (t && t.length > 0) return t;
    }
    if (this.svgInline && this.svgInline.length > 0) return this.svgInline;
    return '';
  }

  private createMesh(group: { vertices: number[]; indices: number[]; color: number[] }, index: number): void {
    const builder = new MeshBuilder([
      { name: 'position', components: 3 },
    ]);
    builder.topology = MeshTopology.Triangles;
    builder.indexType = MeshIndexType.UInt16;

    builder.appendVerticesInterleaved(group.vertices);
    builder.appendIndices(group.indices);

    if (!builder.isValid()) {
      print(`[SpaceSVGImage] Warning: invalid mesh group ${index}, skipping`);
      return;
    }

    builder.updateMesh();
    const mesh = builder.getMesh();

    const obj = global.scene.createSceneObject(`SpaceSVGImage_${index}`);
    obj.setParent(this.getSceneObject());
    // Tiny +z step just to break depth-fighting ties — setRenderOrder does
    // the actual ordering. Keep this small enough that successive groups
    // don't visibly hover above the canvas plane.
    obj.getTransform().setLocalPosition(new vec3(0, 0, index * 0.005));

    const visual = obj.createComponent('Component.RenderMeshVisual') as RenderMeshVisual;
    visual.mesh = mesh;
    visual.mainMaterial = this.material;
    try { visual.setRenderOrder(index); } catch (e) { /* older runtimes — ignore */ }

    const overrides = visual.mainPassOverrides as any;
    overrides.baseColor = new vec4(
      group.color[0], group.color[1], group.color[2], group.color[3]
    );

    print(`[SpaceSVGImage] Group ${index}: color=[${group.color[0].toFixed(2)},${group.color[1].toFixed(2)},${group.color[2].toFixed(2)},${group.color[3].toFixed(2)}] verts=${group.vertices.length / 3} tris=${group.indices.length / 3} z=${(index * 0.1).toFixed(2)}`);

    this.meshObjects.push(obj);
  }
}
