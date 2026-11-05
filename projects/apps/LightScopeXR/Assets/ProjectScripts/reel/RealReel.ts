// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// RealReel.ts — the shots as they were: colour thumbnails, before the 1-bit
// treatment the Roll gets.
//
// A Scout shot shows the colour frame that was on the Live panel when
// SHOOT SCOUT went (Scout's own 320x240 JPEG, already on the glasses). An
// α7 shot arrives from AlphaFriend: `pvj` downloads the camera's postview
// again and re-encodes it at 1/8 as a ~200x135 colour JPEG (AlphaFriend has
// no room to keep a colour copy), then base64 over BLE. That takes seconds,
// so each shot on its way has a progress row: what it is waiting on, how
// long so far, and a bar against how long that step took last time.
//
// Newest shot large on top, the rest in a strip below; a pinch on a thumb
// brings it up and writes its details in the caption: where it came from,
// when, its size, how long it took to get here and (for the α7) the exposure.

import { makeQuad, makeTappable, makeTexture, setQuadTexture, Z_BACK, Z_IMAGE } from "../ui/Bitmap";
import { placeInFront } from "../ui/Place";
import { Links } from "../ble/Links";

const AMBER = new vec4(0.949, 0.639, 0.227, 1);
const MAX_SHOTS = 6;
const BIG_Y = 2.2;              // clear of the thumb strip (a selected thumb grows 12 %)

interface Shot { tex: Texture; w: number; h: number; label: string; info: string; meta: string; took: number; at: string; thumb: SceneObject; }

export class ReelJob {
  private t0 = Date.now();
  private stageAt = Date.now();
  private expect = 3;
  stageName = "";
  meta = "";                       // the exposure etc., shown with the shot
  constructor(private reel: RealReel, readonly label: string) {}

  // A step that should take about expectSec (the reel remembers how long
  // each named step took, and uses that next time).
  stage(name: string, expectSec: number) {
    this.stageName = name;
    this.stageAt = Date.now();
    this.expect = this.reel.learned(name, expectSec);
    this.reel.redraw();
  }

  elapsed(): number { return (Date.now() - this.t0) / 1000; }
  fraction(): number { return Math.min(0.97, (Date.now() - this.stageAt) / 1000 / this.expect); }

  done(tex: Texture, w: number, h: number, info: string) {
    this.reel.learn(this.stageName, (Date.now() - this.stageAt) / 1000);
    this.reel.finish(this, tex, w, h, info);
  }

  fail(why: string) {
    this.reel.abandon(this, why);
  }

  // A step finished without a picture yet: remember its time, go on.
  stepDone() {
    this.reel.learn(this.stageName, (Date.now() - this.stageAt) / 1000);
  }
}

@component
export class RealReel extends BaseScriptComponent {

  @input links: Links;

  @input
  @hint("Unlit material with a base texture")
  imageMaterial: Material;

  @input titleText: Text;
  @input captionText: Text;

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
  yaw: number = -32;

  @input
  @hint("Width of the large picture, cm (at 45 cm)")
  bigWidth: number = 16;

  private shots: Shot[] = [];
  private selected = -1;
  private big: SceneObject = null;
  private bar: SceneObject = null;
  private barBack: SceneObject = null;
  private jobs: ReelJob[] = [];
  private took: { [stage: string]: number } = {};
  private note = "";

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => {
      this.paint(this.titleText, "REALREEL");
      this.paint(this.captionText, "COLOUR SHOTS LAND HERE");
      const root = this.getSceneObject();
      this.big = makeQuad(root, "Big", this.bigWidth, this.bigWidth * 2 / 3, this.imageMaterial, makeTexture(1, 1, new Uint8Array([0, 0, 0, 255])));
      this.big.getTransform().setLocalPosition(new vec3(0, BIG_Y, Z_IMAGE));
      if (this.titleText) this.titleText.getSceneObject().getTransform().setLocalPosition(new vec3(0, BIG_Y + this.bigWidth / 3 + 0.9, 0.2));
      const amber = makeTexture(1, 1, new Uint8Array([242, 163, 58, 255]));
      const dim = makeTexture(1, 1, new Uint8Array([70, 46, 16, 255]));
      this.barBack = makeQuad(root, "BarBack", this.bigWidth, 0.35, this.imageMaterial, dim, 9);
      this.barBack.getTransform().setLocalPosition(new vec3(0, -5.9, Z_BACK));
      this.bar = makeQuad(root, "Bar", this.bigWidth, 0.35, this.imageMaterial, amber, 10);
      this.bar.getTransform().setLocalPosition(new vec3(0, -5.9, Z_IMAGE));
      this.showBar(false);
      this.links.after(() => {
        if (this.frame && this.camera) placeInFront(this.frame, this.camera, this.distance, this.drop, this.yaw);
      }, 0.3);
      this.tick();
    });
  }

  // ---- for the cameras ----------------------------------------------------

  begin(label: string): ReelJob {
    const j = new ReelJob(this, label);
    this.jobs.push(j);
    this.redraw();
    return j;
  }

  learned(stage: string, fallback: number): number {
    return this.took[stage] || fallback;
  }

  learn(stage: string, seconds: number) {
    if (!stage || seconds <= 0) return;
    const was = this.took[stage];
    this.took[stage] = was ? was * 0.6 + seconds * 0.4 : seconds;
  }

  finish(job: ReelJob, tex: Texture, w: number, h: number, info: string) {
    this.jobs = this.jobs.filter((j) => j !== job);
    this.add({ tex, w, h, label: job.label, info, meta: job.meta, took: job.elapsed(), at: clock(), thumb: null });
    this.note = "";                                    // the caption shows the new shot's details
    print("[reel] " + job.label + " · " + info + " · " + job.elapsed().toFixed(1) + " S");
    this.redraw();
  }

  abandon(job: ReelJob, why: string) {
    this.jobs = this.jobs.filter((j) => j !== job);
    this.note = job.label + " · " + why;
    print("[reel] " + this.note);
    this.redraw();
  }

  // ---- layout -------------------------------------------------------------

  private add(s: Shot) {
    this.shots.unshift(s);
    if (this.shots.length > MAX_SHOTS * 2) {
      const old = this.shots.pop();
      if (old.thumb) old.thumb.enabled = false;
    }
    const tw = this.thumbWidth(), th = tw * 2 / 3;
    s.thumb = makeQuad(this.getSceneObject(), "Thumb", tw, th, this.imageMaterial, s.tex);
    makeTappable(s.thumb, tw, th, () => { this.note = ""; this.select(this.shots.indexOf(s)); });
    this.select(0);
  }

  private thumbWidth(): number {
    return (this.bigWidth - 0.4 * (MAX_SHOTS - 1)) / MAX_SHOTS;
  }

  private select(i: number) {
    const s = this.shots[i];
    if (!s) return;
    this.selected = i;
    setQuadTexture(this.big, s.tex);
    // fit the shot's aspect into the 3:2 box by scaling the quad
    const box = 3 / 2, a = s.w / s.h;
    this.big.getTransform().setLocalScale(a > box ? new vec3(1, box / a, 1) : new vec3(a / box, 1, 1));
    const tw = this.thumbWidth();
    this.shots.forEach((g, k) => {
      if (!g.thumb) return;
      g.thumb.enabled = k < MAX_SHOTS;
      g.thumb.getTransform().setLocalPosition(new vec3(-this.bigWidth / 2 + tw / 2 + k * (tw + 0.4), -4.4, Z_IMAGE));
      g.thumb.getTransform().setLocalScale(k === i ? new vec3(1.12, 1.12, 1) : vec3.one());
    });
    this.redraw();
  }

  redraw() {
    const j = this.jobs[0];
    this.showBar(!!j);
    if (j) {
      const more = this.jobs.length > 1 ? " (+" + (this.jobs.length - 1) + ")" : "";
      this.paint(this.captionText, j.label + " · " + j.stageName + " · " + j.elapsed().toFixed(1) + " S" + more);
      const f = j.fraction();
      this.bar.getTransform().setLocalScale(new vec3(Math.max(0.02, f), 1, 1));
      this.bar.getTransform().setLocalPosition(new vec3(-this.bigWidth / 2 * (1 - f), -5.9, Z_IMAGE));
    } else if (this.note) {
      this.paint(this.captionText, this.note);
    } else if (this.selected >= 0) {
      const s = this.shots[this.selected];
      this.paint(this.captionText, details(s));
    }
  }

  private showBar(on: boolean) {
    if (this.bar) this.bar.enabled = on;
    if (this.barBack) this.barBack.enabled = on;
  }

  // the bar moves while a job runs
  private tick() {
    if (this.jobs.length) this.redraw();
    this.links.after(() => this.tick(), 0.1);
  }

  private paint(t: Text, s: string) {
    if (!t) return;
    t.text = s.toUpperCase();
    t.textFill.color = AMBER;
  }
}

function clock(): string {
  const d = new Date(), two = (n: number) => ("0" + n).slice(-2);
  return two(d.getHours()) + ":" + two(d.getMinutes()) + ":" + two(d.getSeconds());
}

// two lines: what and when; size, transfer and exposure
function details(s: Shot): string {
  const second = [s.w + "×" + s.h, s.info, s.took.toFixed(1) + " S", s.meta].filter((x) => x);
  return s.label + " · " + s.at + "\n" + second.join(" · ");
}
