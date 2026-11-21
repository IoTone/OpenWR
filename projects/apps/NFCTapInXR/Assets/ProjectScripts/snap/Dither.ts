// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Dither.ts — a camera crop to a 1-bit maplet frame.
//
// Pure pixels, no Lens Studio: tools/snap_preview.js runs it under Node on a
// photo. The target's four corners are given in source pixels (top-left
// origin), so a target seen a little askew still fills the frame straight.
// Grey (Rec. 601), levels stretched between the 2nd and 98th percentiles,
// Floyd–Steinberg, packed MSB first in rows of ceil(w/8) bytes, 1 = ink:
// the maplet's canvas format, the same as its roll frames.

// Source RGBA rows; top-down unless bottomUp (Lens getPixels starts at the bottom).
export interface Source { rgba: Uint8Array; w: number; h: number; bottomUp: boolean; }

// Corners in source pixels: top-left, top-right, bottom-right, bottom-left.
export function sampleGrey(src: Source, corners: number[][], w: number, h: number): Float32Array {
  const [tl, tr, br, bl] = corners, out = new Float32Array(w * h);
  const at = (x: number, y: number) => {
    x = Math.min(src.w - 1, Math.max(0, x)); y = Math.min(src.h - 1, Math.max(0, y));
    const row = src.bottomUp ? src.h - 1 - Math.floor(y) : Math.floor(y), o = (row * src.w + Math.floor(x)) * 4;
    return 0.299 * src.rgba[o] + 0.587 * src.rgba[o + 1] + 0.114 * src.rgba[o + 2];
  };
  for (let j = 0; j < h; j++) {
    for (let i = 0; i < w; i++) {
      let sum = 0;
      for (let s = 0; s < 4; s++) {                 // 2 x 2 supersampling
        const u = (i + 0.25 + 0.5 * (s & 1)) / w, v = (j + 0.25 + 0.5 * (s >> 1)) / h;
        const x = (tl[0] * (1 - u) + tr[0] * u) * (1 - v) + (bl[0] * (1 - u) + br[0] * u) * v;
        const y = (tl[1] * (1 - u) + tr[1] * u) * (1 - v) + (bl[1] * (1 - u) + br[1] * u) * v;
        sum += at(x, y);
      }
      out[j * w + i] = sum / 4 / 255;
    }
  }
  return out;
}

// Stretch so the 2nd percentile is black and the 98th white.
export function levels(g: Float32Array): void {
  const hist = new Uint32Array(256);
  for (let i = 0; i < g.length; i++) hist[Math.min(255, Math.max(0, Math.round(g[i] * 255)))]++;
  const pick = (f: number) => { let n = 0; for (let k = 0; k < 256; k++) { n += hist[k]; if (n >= g.length * f) return k / 255; } return 1; };
  const lo = pick(0.02), hi = Math.max(lo + 0.05, pick(0.98));
  for (let i = 0; i < g.length; i++) g[i] = Math.min(1, Math.max(0, (g[i] - lo) / (hi - lo)));
}

// Floyd–Steinberg to packed bits, 1 = ink (dark).
export function ditherPack(g: Float32Array, w: number, h: number): Uint8Array {
  const lum = Float32Array.from(g), stride = (w + 7) >> 3, out = new Uint8Array(stride * h);
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const i = y * w + x, v = lum[i], on = v > 0.5 ? 1 : 0, err = v - on;
      if (x + 1 < w) lum[i + 1] += err * 7 / 16;
      if (y + 1 < h) {
        if (x > 0) lum[i + w - 1] += err * 3 / 16;
        lum[i + w] += err * 5 / 16;
        if (x + 1 < w) lum[i + w + 1] += err / 16;
      }
      if (!on) out[y * stride + (x >> 3)] |= 0x80 >> (x & 7);
    }
  }
  return out;
}

export function toFrame(src: Source, corners: number[][], w: number, h: number): Uint8Array {
  const g = sampleGrey(src, corners, w, h);
  levels(g);
  return ditherPack(g, w, h);
}

export function toHex(b: Uint8Array, from: number, to: number): string {
  let s = "";
  for (let i = from; i < to && i < b.length; i++) s += (b[i] < 16 ? "0" : "") + b[i].toString(16);
  return s;
}
