// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// PinView.ts — where you are, where you are going.
//
// The map floats as a tilted plate: contours, your track solid, the leg to
// the destination dashed, pins on top. Each named pin also stands in the room
// as a flag at its bearing from you, with its distance and climb, so the
// destination hangs over the ridge it sits behind. Map up is the way you
// faced when the lens started (the glasses have no compass in a BLE lens).
//
// Position will come from the phone through goggl1 (`here!`, phase G2). Until
// then this is a demo: a contour plate, three pins, and "you" walking toward
// the destination at 1.4 m/s. Pin now drops a breadcrumb where you are; the
// interval chip pins every 1, 5 or 15 minutes; Go to cycles the destination.

import { makeQuad, makeTappable, makeTexture, Z_BACK, Z_IMAGE } from "../ui/Bitmap";
import { placeInFront, DESIGN_DISTANCE } from "../ui/Place";
import { Raster, Rgb } from "../ui/Raster";
import { ALPEN, contours, drawMap, FOG, GLACIER, M_PER_PX, MAP_H, MAP_W, MapPin, MOSS, pill, terrain } from "./PinMap";

const rgba = (c: Rgb) => new vec4(c[0] / 255, c[1] / 255, c[2] / 255, 1);
const WALK = 1.4;                     // m/s, demo
const INTERVALS = [0, 1, 5, 15];      // minutes; 0 = off

interface Pin extends MapPin { flag: SceneObject | null; sub: Text | null; }
interface Chip { text: Text; quad: SceneObject; w: number; on: Texture; off: Texture; }

@component
export class PinView extends BaseScriptComponent {

  @input
  @hint("Unlit material with a base texture")
  imageMaterial: Material;

  @input
  @hint("A Text to copy for pin names (display face)")
  nameText: Text;

  @input
  @hint("A Text to copy for chips and small labels (mono)")
  monoText: Text;

  @input titleText: Text;
  @input captionText: Text;

  @input
  @allowUndefined
  frame: SceneObject;

  @input
  @allowUndefined
  camera: SceneObject;

  @input distance: number = 85;
  @input
  @hint("cm below eye level at 45 cm; below Pulse")
  drop: number = 22;
  @input yaw: number = 0;

  @input
  @hint("How far away the flags in the room stand, cm")
  pinDistance: number = 400;

  @input
  @hint("Plate width, cm (at 45 cm)")
  plateWidth: number = 19.2;

  private z: (x: number, y: number) => number;
  private base: Raster;
  private canvas: Raster;
  private plateTex: Texture;
  private you = [320, 150];
  private painted = [0, 0];
  private track: number[][] = [[40, 270], [90, 240], [130, 246], [180, 205], [230, 196], [270, 160], [320, 150]];
  private pins: Pin[] = [];
  private dest = 2;
  private every = 0;
  private lastAuto = 0;
  private chips: Chip[] = [];
  private anchor: vec3 = null;
  private ahead: vec3 = null;
  private world: SceneObject = null;

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => this.start());
  }

  private start() {
    const t0 = Date.now();
    this.z = terrain(MAP_W, MAP_H, 11);
    this.base = contours(this.z);
    this.canvas = new Raster(MAP_W, MAP_H);
    this.pins = [
      { name: "Creek", x: 180, y: 205, kind: 0, flag: null, sub: null },
      { name: "Saddle", x: 440, y: 108, kind: 0, flag: null, sub: null },
      { name: "Wildwood TH", x: 560, y: 62, kind: 1, flag: null, sub: null },
    ];
    this.buildPlate();
    this.buildChips();
    this.paintPlate();
    print("[pins] plate drawn in " + (Date.now() - t0) + " ms");

    this.after(() => {
      if (this.frame && this.camera) placeInFront(this.frame, this.camera, this.distance, this.drop, this.yaw);
    }, 0.3);
    this.after(() => this.buildWorld(), 0.4);
    this.createEvent("UpdateEvent").bind(() => this.faceFlags());
    this.loop();
  }

  // ---- plate ------------------------------------------------------------

  private buildPlate() {
    const root = this.getSceneObject();
    const w = this.plateWidth, h = w * MAP_H / MAP_W;
    const tilt = global.scene.createSceneObject("Tilt");
    tilt.setParent(root);
    tilt.getTransform().setLocalPosition(new vec3(0, 1.2, Z_IMAGE));
    tilt.getTransform().setLocalRotation(quat.angleAxis(-42 * Math.PI / 180, vec3.right()));
    this.plateTex = this.canvas.texture();
    makeQuad(tilt, "Plate", w, h, this.imageMaterial, this.plateTex);
    // 500 m scale bar, labelled on the plate itself
    const k = w / MAP_W;
    this.label(tilt, this.monoText, "500 m", 14, MOSS, -w / 2 + (18 + 50) * k, h / 2 - 22 * k, 0.05);
  }

  private paintPlate() {
    this.painted = this.you.slice();
    drawMap(this.canvas, this.base, { you: this.you, track: this.track, pins: this.pins, dest: this.dest });
    this.canvas.update(this.plateTex);
  }

  // ---- chips ------------------------------------------------------------

  private buildChips() {
    const root = this.getSceneObject();
    const defs: [number, () => void][] = [
      [3.6, () => this.pinNow(false)],
      [4.8, () => this.cycleEvery()],
      [7.6, () => this.cycleDest()],
    ];
    const gap = 0.8, total = defs.reduce((s, d) => s + d[0], 0) + gap * (defs.length - 1);
    let x = -total / 2;
    for (const [w, act] of defs) {
      const cx = x + w / 2, cy = -4.6, h = 1.15;
      const off = pill(Math.round(w * 40), Math.round(h * 40), false).texture();
      const on = pill(Math.round(w * 40), Math.round(h * 40), true).texture();
      const quad = makeQuad(root, "Chip", w, h, this.imageMaterial, off, 9);
      quad.getTransform().setLocalPosition(new vec3(cx, cy, Z_BACK));
      makeTappable(quad, w, h, act);
      const text = this.label(root, this.monoText, "", 15, MOSS, cx, cy - 0.2, Z_IMAGE);
      this.chips.push({ text, quad, w, on, off });
      x += w + gap;
    }
    this.paintChips();
  }

  private paintChips(flash = -1) {
    const n = INTERVALS[this.every];
    const labels = ["PIN NOW", n ? "EVERY " + n + " MIN" : "AUTO PIN OFF", "GO TO " + this.pins[this.dest].name.toUpperCase()];
    this.chips.forEach((c, i) => {
      const on = i === flash || (i === 1 && n > 0);
      const vis = c.quad.getComponent("Component.RenderMeshVisual") as RenderMeshVisual;
      vis.mainMaterial.mainPass.baseTex = on ? c.on : c.off;
      c.text.text = labels[i];
      c.text.textFill.color = on ? new vec4(0, 0, 0, 1) : rgba(MOSS);
    });
  }

  private pinNow(auto: boolean) {
    const now = new Date();
    this.pins.push({ name: hhmm(now), x: this.you[0], y: this.you[1], kind: 3, flag: null, sub: null });
    this.lastAuto = Date.now();
    print("[pins] " + (auto ? "auto " : "") + "pin at " + this.you.map((v) => v.toFixed(0)).join(",") + " " + hhmm(now));
    this.paintPlate();
    this.paintChips(auto ? -1 : 0);
    if (!auto) this.after(() => this.paintChips(), 0.6);
    this.paintText();
  }

  private cycleEvery() {
    this.every = (this.every + 1) % INTERVALS.length;
    this.lastAuto = Date.now();
    this.paintChips();
  }

  private cycleDest() {
    const named = this.pins.map((p, i) => i).filter((i) => this.pins[i].kind !== 3);
    this.dest = named[(named.indexOf(this.dest) + 1) % named.length];
    this.paintPlate();
    this.paintChips();
    this.paintText();
    this.paintFlags();
  }

  // ---- flags in the room ------------------------------------------------

  private buildWorld() {
    if (!this.camera) return;
    const cam = this.camera.getTransform(), f = cam.back;
    const flat = new vec3(f.x, 0, f.z);
    this.ahead = flat.length > 0.01 ? flat.normalize() : new vec3(0, 0, -1);
    this.anchor = cam.getWorldPosition();
    this.world = global.scene.createSceneObject("PinsInWorld");
    for (const p of this.pins) if (p.kind !== 3) this.makeFlag(p);
    this.paintFlags();
  }

  private makeFlag(p: Pin) {
    const o = global.scene.createSceneObject("Flag " + p.name);
    o.setParent(this.world);
    const k = this.pinDistance / DESIGN_DISTANCE;
    o.getTransform().setWorldScale(new vec3(k, k, k));
    const pole = makeQuad(o, "Pole", 0.12, 2.6, this.imageMaterial, makeTexture(1, 1, new Uint8Array([255, 255, 255, 255])));
    pole.getTransform().setLocalPosition(new vec3(0, -1.3, 0));
    const flag = makeQuad(o, "Cloth", 1.5, 0.9, this.imageMaterial, makeTexture(1, 1, new Uint8Array([255, 255, 255, 255])));
    flag.getTransform().setLocalPosition(new vec3(0.81, -0.45, 0));
    this.label(o, this.nameText, p.name, 26, FOG, 0, 1.6, 0);
    p.sub = this.label(o, this.monoText, "", 16, FOG, 0, 0.6, 0);
    p.flag = o;
  }

  private paintFlags() {
    if (!this.world) return;
    this.pins.forEach((p, i) => {
      if (!p.flag) return;
      const dx = (p.x - this.you[0]) * M_PER_PX, dy = (p.y - this.you[1]) * M_PER_PX;
      const bearing = Math.atan2(dx, -dy);                 // clockwise from map up
      const dir = quat.angleAxis(-bearing, vec3.up()).multiplyVec3(this.ahead);
      p.flag.getTransform().setWorldPosition(this.anchor.add(dir.uniformScale(this.pinDistance)).add(new vec3(0, 25, 0)));
      const col = rgba(i === this.dest ? ALPEN : GLACIER);
      for (const name of ["Pole", "Cloth"]) {
        const vis = findChild(p.flag, name).getComponent("Component.RenderMeshVisual") as RenderMeshVisual;
        vis.mainMaterial.mainPass.baseColor = col;
      }
      const climb = Math.round(this.z(p.x, p.y) - this.z(this.you[0], this.you[1]));
      p.sub.text = dist(Math.hypot(dx, dy)) + " · " + (climb >= 0 ? "+" : "-") + Math.abs(climb) + " m";
      p.sub.textFill.color = col;
    });
  }

  // Flags turn about the vertical to face you.
  private faceFlags() {
    if (!this.world || !this.camera) return;
    const eye = this.camera.getTransform().getWorldPosition();
    for (const p of this.pins) {
      if (!p.flag) continue;
      const tr = p.flag.getTransform(), d = tr.getWorldPosition().sub(eye);
      const flat = new vec3(d.x, 0, d.z);
      if (flat.length > 1) tr.setWorldRotation(quat.lookAt(flat.normalize().uniformScale(-1), vec3.up()));
    }
  }

  // ---- the clock --------------------------------------------------------

  private loop() {
    const ev = this.createEvent("DelayedCallbackEvent");
    let last = Date.now();
    ev.bind(() => {
      const now = Date.now(), dt = (now - last) / 1000;
      last = now;
      this.walk(dt);
      const n = INTERVALS[this.every];
      if (n > 0 && now - this.lastAuto >= n * 60000) this.pinNow(true);
      // a pixel is 5 m, about 4 s of walking: repaint only when you have moved
      if (Math.hypot(this.you[0] - this.painted[0], this.you[1] - this.painted[1]) >= 1) this.paintPlate();
      this.paintFlags();
      this.paintText();
      ev.reset(2);
    });
    ev.reset(2);
  }

  private walk(dt: number) {
    const d = this.pins[this.dest], dx = d.x - this.you[0], dy = d.y - this.you[1];
    const left = Math.hypot(dx, dy), step = WALK * dt / M_PER_PX;
    if (left < 2) return;
    const s = Math.min(1, step / left);
    this.you = [this.you[0] + dx * s, this.you[1] + dy * s];
    const t = this.track[this.track.length - 1];
    if (Math.hypot(this.you[0] - t[0], this.you[1] - t[1]) > 6) this.track.push(this.you.slice());
  }

  private paintText() {
    const named = this.pins.filter((p) => p.kind !== 3).length, crumbs = this.pins.length - named;
    this.paint(this.titleText, "YOU · " + hhmm(new Date()) + "    " + named + " PINS" + (crumbs ? " · " + crumbs + " CRUMBS" : ""), MOSS);
    const d = this.pins[this.dest];
    const left = Math.hypot(d.x - this.you[0], d.y - this.you[1]) * M_PER_PX;
    this.paint(this.captionText, "DEMO · " + d.name.toUpperCase() + " " + dist(left) + " · POSITION FROM THE PHONE IN G2", MOSS);
  }

  // ---- helpers ----------------------------------------------------------

  private label(parent: SceneObject, template: Text, s: string, size: number, c: Rgb, x: number, y: number, z: number): Text {
    const o = global.scene.createSceneObject("Label");
    o.setParent(parent);
    o.getTransform().setLocalPosition(new vec3(x, y, z));
    const t = o.createComponent("Component.Text") as Text;
    t.font = template.font;
    t.size = size;
    t.text = s;
    t.textFill.color = rgba(c);
    t.setRenderOrder(12);
    return t;
  }

  private paint(t: Text, s: string, c: Rgb) {
    if (!t) return;
    t.text = s;
    t.textFill.color = rgba(c);
  }

  private after(fn: () => void, seconds: number) {
    const ev = this.createEvent("DelayedCallbackEvent");
    ev.bind(() => { this.removeEvent(ev); fn(); });
    ev.reset(seconds);
  }
}

function findChild(o: SceneObject, name: string): SceneObject {
  for (let i = 0; i < o.getChildrenCount(); i++) if (o.getChild(i).name === name) return o.getChild(i);
  return null;
}

function hhmm(d: Date): string {
  return ("0" + d.getHours()).slice(-2) + ":" + ("0" + d.getMinutes()).slice(-2);
}

function dist(m: number): string {
  return m < 1000 ? Math.round(m / 10) * 10 + " m" : (m / 1000).toFixed(1) + " km";
}
