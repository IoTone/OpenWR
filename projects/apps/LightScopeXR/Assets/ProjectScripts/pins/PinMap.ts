// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// PinMap.ts — the pin board's plate, drawn in pixels.
//
// Pure drawing, no Lens Studio: tools/pin_preview.js runs it under Node to
// check the look off the glasses. Map pixels: x right, y down (map up is
// north, or the way you faced at lens start), 5 m a pixel.

import { Raster, Rgb } from "../ui/Raster";

export const MOSS: Rgb = [127, 200, 163];
export const GLACIER: Rgb = [120, 183, 212];
export const ALPEN: Rgb = [240, 138, 98];
export const FOG: Rgb = [243, 244, 241];

export const MAP_W = 640, MAP_H = 300;
export const M_PER_PX = 5;            // 3.2 x 1.5 km

// kind: 0 waypoint, 1 destination, 2 photo, 3 breadcrumb (goggl1's pin! kinds)
export interface MapPin { name: string; x: number; y: number; kind: number; }

export interface MapState { you: number[]; track: number[][]; pins: MapPin[]; dest: number; }

// The design page's terrain: a tilted slope with sixteen bumps, in metres.
export function terrain(w: number, h: number, seed: number): (x: number, y: number) => number {
  let s = seed >>> 0;
  const r = () => ((s = (Math.imul(s, 1664525) + 1013904223) >>> 0) / 4294967296);
  const bumps: number[][] = [];
  for (let i = 0; i < 16; i++) bumps.push([r() * w * 1.2 - w * .1, r() * h * 1.2 - h * .1, (0.06 + r() * .1) * w, (r() * 2 - 0.8) * 40]);
  return (x, y) => {
    let v = 1.6 * (h - y) / h * 100 + 1.1 * x / w * 120;
    for (const b of bumps) v += b[3] * Math.exp(-((x - b[0]) ** 2 + (y - b[1]) ** 2) / (2 * b[2] * b[2]));
    return v;
  };
}

// Contour lines every 12 m, every fifth brighter; a faint wash and a border.
// The terrain is smooth, so it is sampled every 4 px and interpolated: about
// 12 000 evaluations instead of 192 000.
export function contours(z: (x: number, y: number) => number): Raster {
  const w = MAP_W, h = MAP_H, step = 12, g = 4, out = new Raster(w, h);
  const gw = Math.ceil(w / g) + 1, gh = Math.ceil(h / g) + 1, grid = new Float32Array(gw * gh);
  for (let j = 0; j < gh; j++) for (let i = 0; i < gw; i++) grid[j * gw + i] = z(i * g, j * g);
  const band = new Int32Array(w * h);
  for (let y = 0; y < h; y++) {
    const j = Math.floor(y / g), fy = y / g - j;
    for (let x = 0; x < w; x++) {
      const i = Math.floor(x / g), fx = x / g - i, o = j * gw + i;
      const v = (grid[o] * (1 - fx) + grid[o + 1] * fx) * (1 - fy) + (grid[o + gw] * (1 - fx) + grid[o + gw + 1] * fx) * fy;
      band[y * w + x] = Math.floor(v / step);
    }
  }
  out.rect(0, 0, w, h, MOSS, 0.06);
  for (let y = 0; y < h - 1; y++) for (let x = 0; x < w - 1; x++) {
    const b = band[y * w + x], br = band[y * w + x + 1], bd = band[(y + 1) * w + x];
    if (b !== br || b !== bd) out.plot(x, y, MOSS, Math.max(b, br, bd) % 5 === 0 ? 0.9 : 0.43);
  }
  out.frame(0, 0, w, h, 2, MOSS, 0.8);
  return out;
}

// Base, scale bar, track solid, leg to the destination dashed, pins, you.
export function drawMap(c: Raster, base: Raster, s: MapState) {
  c.copyFrom(base);
  c.rect(18, 38, 500 / M_PER_PX, 2, MOSS);
  c.path(s.track.concat([s.you]), 6, ALPEN);
  const d = s.pins[s.dest];
  c.path([s.you, [d.x, d.y]], 4, ALPEN, [12, 10]);
  s.pins.forEach((p, i) => {
    if (p.kind === 3) { c.disc(p.x, p.y, 4, GLACIER); return; }
    if (i === s.dest) {                          // destination: a flag
      c.rect(p.x - 2, p.y - 40, 4, 40, ALPEN);
      c.rect(p.x + 2, p.y - 40, 24, 14, ALPEN);
    } else {
      c.tri([p.x - 11, p.y - 26], [p.x + 11, p.y - 26], [p.x, p.y - 6], GLACIER);
      c.rect(p.x - 1.5, p.y - 8, 3, 8, GLACIER);
    }
  });
  c.ring(s.you[0], s.you[1], 15, 3, FOG);
  c.disc(s.you[0], s.you[1], 6, FOG);
}

// A chip: a pill outline, or filled when on.
export function pill(w: number, h: number, on: boolean): Raster {
  const r = new Raster(w, h), rad = h / 2 - 1;
  for (let y = 0; y < h; y++) for (let x = 0; x < w; x++) {
    const cx = Math.min(Math.max(x, rad + 1), w - rad - 1), d = Math.hypot(x - cx, y - h / 2);
    if (on ? d <= rad : Math.abs(d - rad + 1) <= 1.1) r.plot(x, y, MOSS);
  }
  return r;
}
