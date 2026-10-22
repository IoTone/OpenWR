// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// RollView.ts — the maplet's roll, pulled straight from its card.
//
// When the maplet link is up: `roll-ls` lists the frames (0000.BIN ...),
// then `n roll-get` brings each back as hex lines, newest first. A frame is
// the panel it was shot on: 212x104 (2808 B) or 296x128 (4736 B), packed
// 1-bit, 1 = ink. On the glass, paper is Headlamp amber at half strength and
// ink is clear glass, so the photos and the minimap inset read without a
// glowing slab. The newest frame is large; the rest sit in a strip below,
// and a pinch on one brings it up. Over BLE a 4.7 KB frame takes several
// seconds, so they arrive one by one. A maplet on 0.3.2 or later sends
// `roll-b64` instead (a third fewer characters) and is asked for a short
// connection interval while the roll comes over.
//
// demoMode draws stand-in frames (a dithered ridge at dusk) for the editor
// preview, where there is no Bluetooth.

import { Links } from "../ble/Links";
import { b64ToBytes, frameSize, hexToBytes, makeQuad, makeTappable, makeTexture, unpack, Z_IMAGE } from "../ui/Bitmap";
import { placeInFront } from "../ui/Place";

const PAPER = [100, 66, 24, 255];      // Headlamp amber #F2A33A at about 40 %
const PAPER_SEL = [160, 108, 38, 255];  // the selected frame, about 65 %
const INK = [0, 0, 0, 255];           // clear glass
const AMBER = new vec4(0.949, 0.639, 0.227, 1);
const MAX_FRAMES = 6;

interface Frame { n: number; w: number; h: number; bits: Uint8Array; thumb: SceneObject | null; }

@component
export class RollView extends BaseScriptComponent {

  @input links: Links;

  @input
  @hint("Unlit material with a base texture")
  imageMaterial: Material;

  @input titleText: Text;
  @input captionText: Text;

  @input
  @hint("Stand-in frames for the editor preview")
  demoMode: boolean = false;

  @input
  @allowUndefined
  frame: SceneObject;

  @input
  @allowUndefined
  camera: SceneObject;

  @input distance: number = 85;
  @input drop: number = 6;
  @input
  @hint("Degrees to the side; negative is left")
  yaw: number = 32;

  @input
  @hint("Width of the large frame, cm (at 45 cm)")
  bigWidth: number = 16;

  private frames: Frame[] = [];
  private big: SceneObject = null;
  private selected = -1;
  private fetching = false;
  private fetched = false;

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => {
      this.paint(this.titleText, "ROLL · MAPLET");
      this.paint(this.captionText, this.demoMode ? "DEMO · stand-in frames" : "waiting for the maplet");
      this.links.after(() => {
        if (this.frame && this.camera) placeInFront(this.frame, this.camera, this.distance, this.drop, this.yaw);
      }, 0.3);
      if (this.demoMode) {
        [3, 9, 14, 21].forEach((s, i) => this.add({ n: i, w: 212, h: 104, bits: demoFrame(s, 212, 104), thumb: null }));
        print("[roll] demo: " + this.frames.length + " frames, " + this.describe());
        return;
      }
      this.links.onChange(() => this.maybeFetch());
      this.maybeFetch();
    });
  }

  private maybeFetch() {
    const m = this.links.maplet;
    if (!m || !m.live || this.fetching || this.fetched) return;
    this.fetching = true;
    this.fetch().then(() => { this.fetched = true; }, (e) => {
      print("[roll] " + e);
      this.paint(this.captionText, "maplet: " + e);
    }).then(() => { this.fetching = false; });
  }

  private async fetch() {
    const m = this.links.maplet;
    this.paint(this.captionText, "listing the roll");
    const ls = await m.send("roll-ls", 8000);
    const ns: number[] = [];
    for (const l of ls.split("\n")) {
      const r = /^(\d{4})\.bin/i.exec(l.trim());
      if (r) ns.push(parseInt(r[1], 10));
    }
    ns.sort((a, b) => b - a);                  // newest first
    print("[roll] " + ns.length + " frames on the card");
    if (ns.length === 0) {
      this.paint(this.captionText, "the roll is empty: snap on the maplet");
      return;
    }
    const take = ns.slice(0, MAX_FRAMES);
    const v = /v(\d+)\.(\d+)\.(\d+)/.exec(await m.send("version"));
    const b64 = v !== null && +v[1] * 10000 + +v[2] * 100 + +v[3] >= 302;
    if (b64) {
      print("[roll] link before: " + (await m.send("ble-conn")).trim());
      await m.send("6 12 ble-conn!");
      await this.wait(0.5);
      print("[roll] link asked 7.5-15 ms: " + (await m.send("ble-conn")).trim());
    }
    for (let i = 0; i < take.length; i++) {
      const n = take[i];
      this.paint(this.captionText, "pulling frame " + (i + 1) + " of " + take.length);
      const t0 = Date.now();
      const r = await m.send(n + (b64 ? " roll-b64" : " roll-get"), 40000);
      let text = "";
      for (const l of r.split("\n")) {
        const s = l.trim();
        if (b64 ? /^[A-Za-z0-9+/]+=*$/.test(s) : /^[0-9a-fA-F]+$/.test(s) && s.length % 2 === 0) text += s;
      }
      const bits = b64 ? b64ToBytes(text) : hexToBytes(text);
      const size = frameSize(bits.length);
      if (!size) {
        print("[roll] frame " + n + ": " + bits.length + " bytes, unknown size");
        continue;
      }
      print("[roll] frame " + n + " " + size[0] + "x" + size[1] + " in " + (Date.now() - t0) + " ms" + (b64 ? " (base64)" : " (hex)"));
      this.add({ n, w: size[0], h: size[1], bits, thumb: null });
    }
    if (b64) await m.send("24 40 ble-conn!");        // 30-50 ms again: the roll is in
    this.paint(this.captionText, this.describe());
  }

  // Lay out: the large frame centred near the top, thumbnails in a row below.
  private add(f: Frame) {
    const i = this.frames.length;
    this.frames.push(f);
    const root = this.getSceneObject();
    const tw = this.bigWidth / 3.4, th = tw * f.h / f.w;
    const x = -this.bigWidth / 2 + tw / 2 + i * (tw + 0.5);
    f.thumb = makeQuad(root, "Thumb" + f.n, tw, th, this.imageMaterial, makeTexture(f.w, f.h, unpack(f.bits, f.w, f.h, INK, PAPER)));
    f.thumb.getTransform().setLocalPosition(new vec3(x, -4.6, Z_IMAGE));
    makeTappable(f.thumb, tw, th, () => this.select(i));
    if (this.selected < 0) this.select(i);
    else this.paint(this.captionText, this.describe());
  }

  private select(i: number) {
    const f = this.frames[i];
    if (!f) return;
    this.selected = i;
    const tex = makeTexture(f.w, f.h, unpack(f.bits, f.w, f.h, INK, PAPER_SEL));
    // frames differ in shape (212x104, 296x128): a fresh quad each time
    const bw = this.bigWidth, bh = bw * f.h / f.w;
    if (this.big) this.big.destroy();
    this.big = makeQuad(this.getSceneObject(), "Selected", bw, bh, this.imageMaterial, tex);
    this.big.getTransform().setLocalPosition(new vec3(0, 1.6, Z_IMAGE));
    this.frames.forEach((g, k) => {
      if (g.thumb) g.thumb.getTransform().setLocalScale(k === i ? new vec3(1.12, 1.12, 1) : vec3.one());
    });
    this.paint(this.captionText, this.describe());
  }

  private wait(seconds: number): Promise<void> {
    return new Promise((r) => this.links.after(r, seconds));
  }

  private describe(): string {
    const f = this.frames[this.selected];
    if (!f) return "";
    return "FRAME " + ("000" + f.n).slice(-4) + " · " + f.w + "×" + f.h + " · " + this.frames.length + " ON THE CARD";
  }

  private paint(t: Text, s: string) {
    if (!t) return;
    t.text = s;
    t.textFill.color = AMBER;
  }
}

// A dithered ridge at dusk, packed like a maplet frame (1 = ink).
export function demoFrame(seed: number, w: number, h: number): Uint8Array {
  const lum = new Float32Array(w * h);
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const hz = h * (0.45 + 0.12 * Math.sin(x / w * 5 + seed) + 0.05 * Math.sin(x / w * 17 + seed * 2));
      lum[y * w + x] = y < hz ? Math.exp(-(hz - y) / (h * 0.35)) * 0.95 : 0.1;
    }
  }
  const stride = (w + 7) >> 3, out = new Uint8Array(stride * h);
  for (let y = 0; y < h; y++) {
    for (let x = 0; x < w; x++) {
      const i = y * w + x, v = lum[i], on = v > 0.5 ? 1 : 0, err = v - on;
      if (x + 1 < w) lum[i + 1] += err * 7 / 16;
      if (y + 1 < h) {
        if (x > 0) lum[i + w - 1] += err * 3 / 16;
        lum[i + w] += err * 5 / 16;
        if (x + 1 < w) lum[i + w + 1] += err / 16;
      }
      if (!on) out[y * stride + (x >> 3)] |= 0x80 >> (x & 7);   // dark is ink
    }
  }
  return out;
}
