// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Raster.ts — a small software canvas for textures drawn on the glasses.
//
// Lens Studio has no 2D canvas, so plates like the pin board's map are drawn
// here into RGBA pixels and handed to a procedural texture. Colours are light
// on the additive display: black is clear glass, and "dim" means a darker
// colour, not transparency, so strokes are added (max) rather than blended.
// Rows are kept top-down here and flipped when the texture is made. Only
// texture() and update() touch Lens Studio, so the drawing runs under Node too
// (tools/pin_preview.js).

export type Rgb = [number, number, number];

export class Raster {
  readonly px: Uint8Array;

  constructor(readonly w: number, readonly h: number) {
    this.px = new Uint8Array(w * h * 4);
    for (let i = 3; i < this.px.length; i += 4) this.px[i] = 255;
  }

  copyFrom(r: Raster) {
    this.px.set(r.px);
  }

  // Light at (x, y): the brighter of what is there and c * k.
  plot(x: number, y: number, c: Rgb, k = 1) {
    x = Math.round(x); y = Math.round(y);
    if (x < 0 || y < 0 || x >= this.w || y >= this.h) return;
    const o = (y * this.w + x) * 4;
    for (let j = 0; j < 3; j++) this.px[o + j] = Math.max(this.px[o + j], Math.round(c[j] * k));
  }

  // Replace whatever is there; used to cut clear glass under a mark.
  put(x: number, y: number, c: Rgb) {
    x = Math.round(x); y = Math.round(y);
    if (x < 0 || y < 0 || x >= this.w || y >= this.h) return;
    const o = (y * this.w + x) * 4;
    this.px[o] = c[0]; this.px[o + 1] = c[1]; this.px[o + 2] = c[2];
  }

  disc(cx: number, cy: number, r: number, c: Rgb, k = 1) {
    for (let y = Math.floor(cy - r); y <= Math.ceil(cy + r); y++)
      for (let x = Math.floor(cx - r); x <= Math.ceil(cx + r); x++)
        if ((x - cx) * (x - cx) + (y - cy) * (y - cy) <= r * r) this.plot(x, y, c, k);
  }

  ring(cx: number, cy: number, r: number, width: number, c: Rgb) {
    const lo = r - width / 2, hi = r + width / 2;
    for (let y = Math.floor(cy - hi); y <= Math.ceil(cy + hi); y++)
      for (let x = Math.floor(cx - hi); x <= Math.ceil(cx + hi); x++) {
        const d = Math.sqrt((x - cx) * (x - cx) + (y - cy) * (y - cy));
        if (d >= lo && d <= hi) this.plot(x, y, c);
      }
  }

  rect(x: number, y: number, w: number, h: number, c: Rgb, k = 1) {
    for (let j = 0; j < h; j++) for (let i = 0; i < w; i++) this.plot(x + i, y + j, c, k);
  }

  frame(x: number, y: number, w: number, h: number, width: number, c: Rgb, k = 1) {
    this.rect(x, y, w, width, c, k); this.rect(x, y + h - width, w, width, c, k);
    this.rect(x, y, width, h, c, k); this.rect(x + w - width, y, width, h, c, k);
  }

  // A round-capped stroke; dash = [on, off] in pixels along the path.
  path(pts: number[][], width: number, c: Rgb, dash: number[] | null = null) {
    let along = 0;
    for (let i = 1; i < pts.length; i++) {
      const [x0, y0] = pts[i - 1], [x1, y1] = pts[i];
      const len = Math.hypot(x1 - x0, y1 - y0), n = Math.max(1, Math.ceil(len));
      for (let s = 0; s <= n; s++) {
        const d = along + len * s / n;
        if (dash && d % (dash[0] + dash[1]) > dash[0]) continue;
        this.disc(x0 + (x1 - x0) * s / n, y0 + (y1 - y0) * s / n, width / 2, c);
      }
      along += len;
    }
  }

  // A filled triangle (pin heads).
  tri(a: number[], b: number[], c: number[], col: Rgb) {
    const x0 = Math.floor(Math.min(a[0], b[0], c[0])), x1 = Math.ceil(Math.max(a[0], b[0], c[0]));
    const y0 = Math.floor(Math.min(a[1], b[1], c[1])), y1 = Math.ceil(Math.max(a[1], b[1], c[1]));
    const side = (p: number[], q: number[], x: number, y: number) => (q[0] - p[0]) * (y - p[1]) - (q[1] - p[1]) * (x - p[0]);
    for (let y = y0; y <= y1; y++) for (let x = x0; x <= x1; x++) {
      const s1 = side(a, b, x, y), s2 = side(b, c, x, y), s3 = side(c, a, x, y);
      if ((s1 >= 0 && s2 >= 0 && s3 >= 0) || (s1 <= 0 && s2 <= 0 && s3 <= 0)) this.plot(x, y, col);
    }
  }

  // Bottom row first, as procedural textures expect.
  private flipped(): Uint8Array {
    const out = new Uint8Array(this.px.length), row = this.w * 4;
    for (let y = 0; y < this.h; y++) out.set(this.px.subarray(y * row, (y + 1) * row), (this.h - 1 - y) * row);
    return out;
  }

  texture(): Texture {
    const tex = ProceduralTextureProvider.createWithFormat(this.w, this.h, TextureFormat.RGBA8Unorm);
    (tex.control as ProceduralTextureProvider).setPixels(0, 0, this.w, this.h, this.flipped());
    return tex;
  }

  // Redraw into a texture made by texture(), without making a new one.
  update(tex: Texture) {
    (tex.control as ProceduralTextureProvider).setPixels(0, 0, this.w, this.h, this.flipped());
  }
}
