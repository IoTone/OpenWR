// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Paint.ts — a small pixel painter for procedural textures.
//
// The tap scenes have no image assets: cards, rings, check marks and the
// creature are painted here into RGBA buffers. Coordinates are pixels from
// the top left; toTexture() flips the rows, because procedural textures start
// at the bottom left. Edges are anti-aliased by coverage. A fresh canvas is
// fully transparent, which the glasses show as clear.

export type RGBA = [number, number, number, number];

export class Paint {
  readonly px: Uint8Array;

  constructor(public readonly w: number, public readonly h: number) {
    this.px = new Uint8Array(w * h * 4);
  }

  // Blend colour c over pixel (x, y) with extra coverage k (0..1).
  private put(x: number, y: number, c: RGBA, k: number) {
    if (x < 0 || y < 0 || x >= this.w || y >= this.h || k <= 0) return;
    const o = (y * this.w + x) * 4, p = this.px;
    const a = (c[3] / 255) * (k > 1 ? 1 : k), da = p[o + 3] / 255, out = a + da * (1 - a);
    if (out <= 0) return;
    p[o] = (c[0] * a + p[o] * da * (1 - a)) / out;
    p[o + 1] = (c[1] * a + p[o + 1] * da * (1 - a)) / out;
    p[o + 2] = (c[2] * a + p[o + 2] * da * (1 - a)) / out;
    p[o + 3] = out * 255;
  }

  rect(x: number, y: number, w: number, h: number, c: RGBA) {
    for (let j = Math.floor(y); j < y + h; j++) for (let i = Math.floor(x); i < x + w; i++) this.put(i, j, c, 1);
  }

  // Rounded rectangle; c2 gives a top-to-bottom gradient.
  roundRect(x: number, y: number, w: number, h: number, r: number, c: RGBA, c2?: RGBA) {
    const cx = x + w / 2, cy = y + h / 2, hx = w / 2 - r, hy = h / 2 - r;
    for (let j = Math.floor(y); j < y + h; j++) {
      const t = c2 ? (j - y) / h : 0;
      const col: RGBA = c2 ? [c[0] + (c2[0] - c[0]) * t, c[1] + (c2[1] - c[1]) * t, c[2] + (c2[2] - c[2]) * t, c[3] + (c2[3] - c[3]) * t] : c;
      for (let i = Math.floor(x); i < x + w; i++) {
        const dx = Math.max(Math.abs(i + 0.5 - cx) - hx, 0), dy = Math.max(Math.abs(j + 0.5 - cy) - hy, 0);
        this.put(i, j, col, r + 0.5 - Math.sqrt(dx * dx + dy * dy));
      }
    }
  }

  disc(cx: number, cy: number, r: number, c: RGBA) {
    for (let j = Math.floor(cy - r - 1); j <= cy + r + 1; j++)
      for (let i = Math.floor(cx - r - 1); i <= cx + r + 1; i++) {
        const dx = i + 0.5 - cx, dy = j + 0.5 - cy;
        this.put(i, j, c, r + 0.5 - Math.sqrt(dx * dx + dy * dy));
      }
  }

  // Ring of radius r and thickness t; a0..a1 (degrees, clockwise from the top) cuts an arc.
  ring(cx: number, cy: number, r: number, t: number, c: RGBA, a0 = 0, a1 = 360) {
    const full = a1 - a0 >= 360;
    for (let j = Math.floor(cy - r - t); j <= cy + r + t; j++)
      for (let i = Math.floor(cx - r - t); i <= cx + r + t; i++) {
        const dx = i + 0.5 - cx, dy = j + 0.5 - cy, d = Math.sqrt(dx * dx + dy * dy);
        const k = t / 2 + 0.5 - Math.abs(d - r);
        if (k <= 0) continue;
        if (!full) {
          let a = Math.atan2(dx, -dy) * 180 / Math.PI;
          if (a < 0) a += 360;
          if (a < a0 || a > a1) continue;
        }
        this.put(i, j, c, k);
      }
  }

  // A thick line with round ends.
  line(x0: number, y0: number, x1: number, y1: number, t: number, c: RGBA) {
    const minX = Math.floor(Math.min(x0, x1) - t), maxX = Math.ceil(Math.max(x0, x1) + t);
    const minY = Math.floor(Math.min(y0, y1) - t), maxY = Math.ceil(Math.max(y0, y1) + t);
    const vx = x1 - x0, vy = y1 - y0, len2 = vx * vx + vy * vy || 1;
    for (let j = minY; j <= maxY; j++)
      for (let i = minX; i <= maxX; i++) {
        let u = ((i + 0.5 - x0) * vx + (j + 0.5 - y0) * vy) / len2;
        u = u < 0 ? 0 : u > 1 ? 1 : u;
        const dx = i + 0.5 - (x0 + vx * u), dy = j + 0.5 - (y0 + vy * u);
        this.put(i, j, c, t / 2 + 0.5 - Math.sqrt(dx * dx + dy * dy));
      }
  }

  // Pixel art: one character per pixel, looked up in the palette; "." is clear.
  sprite(rows: string[], palette: { [ch: string]: RGBA }, x: number, y: number, scale: number) {
    for (let j = 0; j < rows.length; j++)
      for (let i = 0; i < rows[j].length; i++) {
        const c = palette[rows[j][i]];
        if (c) this.rect(x + i * scale, y + j * scale, scale, scale, c);
      }
  }

  toTexture(): Texture {
    const w = this.w, h = this.h, out = new Uint8Array(w * h * 4);
    for (let y = 0; y < h; y++) out.set(this.px.subarray(y * w * 4, (y + 1) * w * 4), (h - 1 - y) * w * 4);
    const tex = ProceduralTextureProvider.createWithFormat(w, h, TextureFormat.RGBA8Unorm);
    (tex.control as ProceduralTextureProvider).setPixels(0, 0, w, h, out);
    return tex;
  }
}
