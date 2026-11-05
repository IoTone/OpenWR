// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Viewfinder.ts — shoot with the glasses, print on the maplet.
//
// CAMERA on the Roll panel opens a target: an amber frame at the maplet's
// aspect, 60 cm ahead (in front of the panels, so they do not take the
// pinch), locked to the head like a viewfinder. Like the panels it is laid
// out in cm at 45 cm and scaled by distance/45. SHOOT takes the next camera
// frame, crops exactly what sits inside the target (its corners projected
// into the camera image) and turns it into a 1-bit frame for the maplet's
// panel (snap/Dither.ts). The shot shows in the target for a moment and
// joins the Roll strip, selected and not sent.
//
// SEND on the Roll panel puts the selected frame on the maplet: `epd-hex!`
// lines into the canvas, then `epd-show`; a new shot is also kept on the
// card with `roll-save`.
//
// The camera in a Bluetooth lens: Spectacles refuse a connectivity module
// (BluetoothCentralModule) with a sensitive sensor (CameraModule) on their own.
// Declaring InternetModule as well makes it a Transparent Permission lens,
// which asks once at launch (or not at all with Extended Permissions on).
// See developers.snap.com/spectacles/permission-privacy/transparent-permission.
// The lens never uses the internet; the require only declares it.
require("LensStudio:InternetModule");

import { Links } from "../ble/Links";
import { makeQuad, makeTappable, makeTexture, unpack, Z_BACK, Z_IMAGE } from "../ui/Bitmap";
import { Raster, Rgb } from "../ui/Raster";
import { RollView } from "../roll/RollView";
import { DESIGN_DISTANCE } from "../ui/Place";
import { Source, toFrame, toHex } from "./Dither";
import { playOnce } from "../ui/Sound";

const AMBER: Rgb = [242, 163, 58];
const PAPER = [160, 108, 38, 255];
const INK = [0, 0, 0, 255];
const rgba = (c: Rgb) => new vec4(c[0] / 255, c[1] / 255, c[2] / 255, 1);
const SHOW_SECONDS = 5;                 // a shot stays in the target, then joins the Roll
const SKIP_FRAMES = 2;                  // camera frames to let pass after the pinch
const LINE_BYTES = 78;                 // as tools/push_map.py: 36 lines for 2808 B

@component
export class Viewfinder extends BaseScriptComponent {

  @input links: Links;
  @input roll: RollView;

  @input
  @hint("Unlit material with a base texture")
  imageMaterial: Material;

  @input
  @hint("A Text to copy for chips and labels (mono)")
  monoText: Text;

  @input
  @hint("The scene camera (it follows the head)")
  camera: SceneObject;

  @input
  @allowUndefined
  @hint("Shutter sound for a shot with the glasses")
  shutter: AudioTrackAsset;

  @input
  @hint("How far ahead the target floats, cm (nearer than the panels)")
  distance: number = 60;

  @input
  @hint("Target width, cm, at that distance")
  width: number = 26;

  private open = false;
  private shooting = false;
  private sending = false;
  private target: SceneObject;
  private preview: SceneObject = null;
  private status: Text;
  private camChip: Text;
  private camTex: Texture = null;
  private lastShot: Uint8Array = null;
  private cameraModule = require("LensStudio:CameraModule") as CameraModule;
  private frameW = 212;
  private frameH = 104;

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => this.build());
    this.createEvent("LateUpdateEvent").bind(() => this.follow());
  }

  // ---- the target and its chips -----------------------------------------

  private build() {
    // SEND and CAMERA, top right of the Roll panel
    const rollRoot = this.roll.getSceneObject();
    this.chip(rollRoot, "SEND", 2.8, new vec3(4.0, 6.4, 0), () => this.sendSelected());
    this.camChip = this.chip(rollRoot, "CAMERA", 3.4, new vec3(7.6, 6.4, 0), () => this.toggle());

    this.target = global.scene.createSceneObject("Viewfinder");
    const w = this.designWidth(), h = w * this.frameH / this.frameW;
    const r = new Raster(Math.round(w * 16), Math.round(h * 16));
    r.frame(0, 0, r.w, r.h, 3, AMBER, 0.9);
    const tick = Math.round(r.h * 0.18);           // bright corners
    for (const [x, y] of [[0, 0], [r.w - tick, 0], [0, r.h - 7], [r.w - tick, r.h - 7]]) r.rect(x, y, tick, 7, AMBER);
    for (const [x, y] of [[0, 0], [r.w - 7, 0], [0, r.h - tick], [r.w - 7, r.h - tick]]) r.rect(x, y, 7, tick, AMBER);
    makeQuad(this.target, "Frame", w, h, this.imageMaterial, r.texture(), 9);
    this.status = this.label(this.target, "", 15, new vec3(0, h / 2 + 1.4, Z_IMAGE));
    // big enough to pinch while the target moves with the head
    this.chip(this.target, "SHOOT", 6.0, new vec3(-2.7, -h / 2 - 2.4, 0), () => this.shoot(), 2.0, 26);
    this.chip(this.target, "DONE", 4.6, new vec3(3.4, -h / 2 - 2.4, 0), () => this.toggle(), 2.0, 26);
    this.target.enabled = false;
  }

  private chip(parent: SceneObject, s: string, w: number, at: vec3, act: () => void, h = 1.2, size = 15): Text {
    const r = new Raster(Math.round(w * 40), Math.round(h * 40)), rad = r.h / 2 - 1;
    for (let y = 0; y < r.h; y++) for (let x = 0; x < r.w; x++) {
      const cx = Math.min(Math.max(x, rad + 1), r.w - rad - 1), d = Math.hypot(x - cx, y - r.h / 2);
      if (Math.abs(d - rad + 1) <= 1.1) r.plot(x, y, AMBER);
    }
    const q = makeQuad(parent, "Chip " + s, w, h, this.imageMaterial, r.texture(), 9);
    q.getTransform().setLocalPosition(new vec3(at.x, at.y, Z_BACK));
    makeTappable(q, w, h, act);
    return this.label(parent, s, size, new vec3(at.x, at.y, Z_IMAGE));
  }

  private label(parent: SceneObject, s: string, size: number, at: vec3): Text {
    const o = global.scene.createSceneObject("Label");
    o.setParent(parent);
    o.getTransform().setLocalPosition(at);
    const t = o.createComponent("Component.Text") as Text;
    t.font = this.monoText.font;
    t.size = size;
    t.text = s;
    t.horizontalAlignment = HorizontalAlignment.Center;
    t.verticalAlignment = VerticalAlignment.Center;   // centred on its object, so in its chip
    t.textFill.color = rgba(AMBER);
    t.setRenderOrder(12);
    return t;
  }

  private toggle() {
    if (this.shooting) return;
    this.open = !this.open;
    this.target.enabled = this.open;
    this.camChip.text = this.open ? "CAMERA ON" : "CAMERA";
    if (!this.open) return;
    this.clearPreview();
    this.follow();
    if (!this.camTex) this.startCamera();
    this.sizeFromMaplet();
  }

  // The target's width in layout cm (at 45 cm); the root is scaled up to it.
  private designWidth(): number {
    return this.width * DESIGN_DISTANCE / this.distance;
  }

  // Locked ahead along the gaze, after the camera has moved this frame. It
  // holds still while a shot is taken and shown.
  private follow() {
    if (!this.open || this.shooting || !this.camera) return;
    const cam = this.camera.getTransform(), tr = this.target.getTransform(), k = this.distance / DESIGN_DISTANCE;
    tr.setWorldScale(new vec3(k, k, k));
    tr.setWorldPosition(cam.getWorldPosition().add(cam.back.uniformScale(this.distance)));
    tr.setWorldRotation(cam.getWorldRotation());
  }

  // ---- camera -----------------------------------------------------------

  private startCamera() {
    try {
      const req = CameraModule.createCameraRequest();
      req.cameraId = CameraModule.CameraId.Default_Color;
      req.imageSmallerDimension = 480;
      this.camTex = this.cameraModule.requestCamera(req);
      this.say("AIM, THEN SHOOT");
      print("[snap] camera requested");
    } catch (e) {
      this.say("NO CAMERA: " + e);
      print("[snap] camera: " + e);
    }
  }

  // Shots are made at the maplet's panel size (212x104 until it says otherwise).
  private sizeFromMaplet(): Promise<void> {
    const m = this.links.maplet;
    if (!m || !m.live) return Promise.resolve();
    return m.send("epd-size@ swap . .").then((r) => {
      const v = /(\d+)\s+(\d+)/.exec(r);
      if (v) { this.frameW = +v[1]; this.frameH = +v[2]; }
      print("[snap] maplet panel " + this.frameW + "x" + this.frameH);
    }, (e) => print("[snap] epd-size@: " + e));
  }

  // The first frame event after the pinch can still hold the previous image,
  // so a few frames pass first, and a shot identical to the last one is
  // taken again.
  private shoot() {
    if (this.shooting || !this.camTex) return;
    this.shooting = true;                          // the target holds still
    playOnce(this.getSceneObject(), this.shutter);
    this.clearPreview();
    this.say("SHOOTING");
    this.grab(SKIP_FRAMES, 5);
  }

  private grab(skip: number, tries: number) {
    const provider = this.camTex.control as CameraTextureProvider;
    let reg: EventRegistration = null;
    reg = provider.onNewFrame.add(() => {
      if (skip-- > 0) return;
      provider.onNewFrame.remove(reg);
      try {
        const bits = this.capture();
        if (this.lastShot && same(bits, this.lastShot) && tries > 1) {
          print("[snap] stale frame, again");
          this.grab(1, tries - 1);
          return;
        }
        this.lastShot = bits;
        this.showPreview(bits);
        this.say("SHOT · SEND IT FROM THE ROLL");
        this.links.after(() => {                   // time to look at it, then into the Roll
          this.roll.addShot(this.frameW, this.frameH, bits);
          this.clearPreview();
          this.shooting = false;
          if (this.open) this.say("AIM, THEN SHOOT");
        }, SHOW_SECONDS);
      } catch (e) {
        print("[snap] capture: " + e);
        this.say("CAPTURE FAILED");
        this.shooting = false;
      }
    });
  }

  private capture(): Uint8Array {
    const t0 = Date.now();
    const copy = ProceduralTextureProvider.createFromTexture(this.camTex);
    const cw = copy.getWidth(), ch = copy.getHeight();
    // the target's corners, world -> device reference -> camera image pixels
    const dev = global.deviceInfoSystem.getTrackingCameraForId(CameraModule.CameraId.Default_Color);
    const inv = this.camera.getTransform().getInvertedWorldTransform();
    const tr = this.target.getTransform().getWorldTransform();
    const w = this.designWidth() / 2, h = w * this.frameH / this.frameW;
    const corners = [[-w, h], [w, h], [w, -h], [-w, -h]].map(([x, y]) => {
      const uv = dev.project(inv.multiplyPoint(tr.multiplyPoint(new vec3(x, y, 0))));
      return [uv.x * cw, uv.y * ch];
    });
    // read only the box around them
    const x0 = clamp(Math.floor(Math.min(...corners.map((c) => c[0]))), 0, cw - 1);
    const x1 = clamp(Math.ceil(Math.max(...corners.map((c) => c[0]))), x0 + 1, cw);
    const y0 = clamp(Math.floor(Math.min(...corners.map((c) => c[1]))), 0, ch - 1);
    const y1 = clamp(Math.ceil(Math.max(...corners.map((c) => c[1]))), y0 + 1, ch);
    const bw = x1 - x0, bh = y1 - y0, px = new Uint8Array(bw * bh * 4);
    (copy.control as ProceduralTextureProvider).getPixels(x0, ch - y1, bw, bh, px);   // bottom-left origin
    const src: Source = { rgba: px, w: bw, h: bh, bottomUp: true };
    const bits = toFrame(src, corners.map(([x, y]) => [x - x0, y - y0]), this.frameW, this.frameH);
    print("[snap] camera " + cw + "x" + ch + ", target box " + bw + "x" + bh + " at " + x0 + "," + y0 + " -> " + this.frameW + "x" + this.frameH + " in " + (Date.now() - t0) + " ms");
    return bits;
  }

  private showPreview(bits: Uint8Array) {
    this.clearPreview();
    const w = this.designWidth(), h = w * this.frameH / this.frameW;
    const tex = makeTexture(this.frameW, this.frameH, unpack(bits, this.frameW, this.frameH, INK, PAPER));
    this.preview = makeQuad(this.target, "Preview", w * 0.96, h * 0.96, this.imageMaterial, tex);
    this.preview.getTransform().setLocalPosition(new vec3(0, 0, Z_IMAGE));
  }

  private clearPreview() {
    if (this.preview) { this.preview.destroy(); this.preview = null; }
  }

  // ---- to the maplet ----------------------------------------------------

  // SEND: the Roll's selected frame onto the maplet's panel; progress in the
  // Roll caption. Any frame can go back up; a new shot is also saved.
  private async sendSelected() {
    const f = this.roll.selectedFrame(), m = this.links.maplet;
    if (this.sending || !f) return;
    if (!m || !m.live) { this.roll.say("SEND · MAPLET NOT CONNECTED"); return; }
    this.sending = true;
    const t0 = Date.now();
    try {
      await this.sizeFromMaplet();
      if (f.w !== this.frameW || f.h !== this.frameH) throw new Error(f.w + "×" + f.h + " FRAME, " + this.frameW + "×" + this.frameH + " PANEL");
      await m.send("epd-clear 0 epd-cursor!");
      const lines = Math.ceil(f.bits.length / LINE_BYTES);
      for (let i = 0; i < lines; i++) {
        this.roll.say("SENDING " + (i + 1) + "/" + lines);
        const r = await m.send('s" ' + toHex(f.bits, i * LINE_BYTES, (i + 1) * LINE_BYTES) + '" epd-hex!', 5000);
        if (/\?/.test(r)) throw new Error("epd-hex! " + r.trim());
      }
      const sent = Date.now() - t0;
      this.roll.say("PRINTING ON THE MAPLET");
      await m.send("epd-show", 15000);
      let n = f.n;
      if (n < 0) {
        n = parseInt((await m.send("roll-save .", 5000)).trim(), 10);
        if (n >= 0) this.roll.saved(f, n);
      }
      print("[snap] sent " + f.bits.length + " B in " + sent + " ms, shown" + (f.n === n ? "" : " and saved as " + n) + " in " + (Date.now() - t0) + " ms");
      this.roll.say(n >= 0 ? "ON THE MAPLET · FRAME " + ("000" + n).slice(-4) : "ON THE MAPLET · NOT SAVED (NO CARD?)");
    } catch (e) {
      print("[snap] send: " + e);
      this.roll.say("SEND FAILED: " + e);
    }
    this.sending = false;
  }

  private say(s: string) {
    if (this.status) this.status.text = s;
  }
}

function clamp(v: number, lo: number, hi: number): number {
  return Math.min(hi, Math.max(lo, v));
}

function same(a: Uint8Array, b: Uint8Array): boolean {
  if (a.length !== b.length) return false;
  for (let i = 0; i < a.length; i++) if (a[i] !== b[i]) return false;
  return true;
}
