// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// RollView.ts — a camera's roll: the maplet's card, or Scout's flash.
//
// The roll comes from links.rollLink (the maplet when it is linked, else
// Scout, which speaks the same roll words). When that link is up: `roll-ls` lists the frames (0000.BIN ...),
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
// Shots from the glasses (snap/Viewfinder.ts) join the strip first, marked
// NEW until SEND puts them on the maplet; SEND works for any selected frame.
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

// n is the frame number on the maplet card; -1 for a shot not sent yet.
export interface Frame { n: number; w: number; h: number; bits: Uint8Array; thumb: SceneObject | null; }

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
      this.paint(this.titleText, "ROLL");
      this.paint(this.captionText, this.demoMode ? "DEMO · stand-in frames" : "waiting for a camera");
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
    const m = this.links.rollLink;
    if (!m || !m.live || this.fetching || this.fetched) return;
    this.paint(this.titleText, "ROLL · " + m.deviceName.toUpperCase());
    this.fetching = true;
    this.fetch().then(() => { this.fetched = true; }, (e) => {
      print("[roll] " + e);
      this.paint(this.captionText, m.deviceName + ": " + e);
    }).then(() => { this.fetching = false; });
  }

  // Fetch the roll again (a camera just shot into it).
  refresh() {
    if (this.fetching) return;
    this.fetched = false;
    this.maybeFetch();
  }

  private async fetch() {
    const m = this.links.rollLink;
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
      this.paint(this.captionText, "the roll is empty: shoot first");
      return;
    }
    // on a refresh only the frames not here yet, oldest first, each in front
    const have = new Set(this.frames.filter((f) => f.n >= 0).map((f) => f.n));
    const front = have.size > 0;
    let take = ns.filter((n) => !have.has(n)).slice(0, MAX_FRAMES);
    if (take.length === 0) { this.paint(this.captionText, this.describe()); return; }
    if (front) take = take.reverse();
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
      this.add({ n, w: size[0], h: size[1], bits, thumb: null }, front);
    }
    if (b64) await m.send("24 40 ble-conn!");        // 30-50 ms again: the roll is in
    if (front) this.select(0);
    this.paint(this.captionText, this.describe());
  }

  // A frame the glasses just shot: first in the strip, selected, not on the card yet.
  addShot(w: number, h: number, bits: Uint8Array) {
    this.add({ n: -1, w, h, bits, thumb: null }, true);
    this.select(0);
  }

  selectedFrame(): Frame | null {
    return this.frames[this.selected] || null;
  }

  // The maplet saved a shot as frame n.
  saved(f: Frame, n: number) {
    f.n = n;
    if (this.frames[this.selected] === f) this.paint(this.captionText, this.describe());
  }

  say(s: string) {
    this.paint(this.captionText, s);
  }

  // Lay out: the large frame centred near the top, the newest MAX_FRAMES as
  // thumbnails in a row below.
  private add(f: Frame, front = false) {
    if (front) this.frames.unshift(f); else this.frames.push(f);
    if (front && this.selected >= 0) this.selected++;
    const tw = this.thumbWidth(), th = tw * f.h / f.w;
    f.thumb = makeQuad(this.getSceneObject(), "Thumb" + f.n, tw, th, this.imageMaterial, makeTexture(f.w, f.h, unpack(f.bits, f.w, f.h, INK, PAPER)));
    makeTappable(f.thumb, tw, th, () => this.select(this.frames.indexOf(f)));
    this.layout();
    if (this.selected < 0) this.select(this.frames.indexOf(f));
    else this.paint(this.captionText, this.describe());
  }

  private thumbWidth(): number {
    return (this.bigWidth - 0.4 * (MAX_FRAMES - 1)) / MAX_FRAMES;
  }

  private layout() {
    const tw = this.thumbWidth();
    this.frames.forEach((g, k) => {
      if (!g.thumb) return;
      g.thumb.enabled = k < MAX_FRAMES;
      g.thumb.getTransform().setLocalPosition(new vec3(-this.bigWidth / 2 + tw / 2 + k * (tw + 0.4), -4.6, Z_IMAGE));
      g.thumb.getTransform().setLocalScale(k === this.selected ? new vec3(1.12, 1.12, 1) : vec3.one());
    });
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
    this.layout();
    this.paint(this.captionText, this.describe());
  }

  private wait(seconds: number): Promise<void> {
    return new Promise((r) => this.links.after(r, seconds));
  }

  private describe(): string {
    const f = this.frames[this.selected];
    if (!f) return "";
    const name = f.n < 0 ? "NEW SHOT · NOT SENT" : "FRAME " + ("000" + f.n).slice(-4);
    return name + " · " + f.w + "×" + f.h + " · " + this.frames.filter((g) => g.n >= 0).length + " ON THE CARD";
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
