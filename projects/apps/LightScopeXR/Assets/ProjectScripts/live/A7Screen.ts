// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// A7Screen.ts — the Sony's own rear screen, over its liveview on the glasses.
//
// Laid out like the α7R III's display (photo IMG_0762): exposure mode large
// at top left, then the card, aspect and quality across the top and the
// battery at top right; drive and focus mode down the left; white balance on
// the right; shutter speed, aperture, EV and ISO along the bottom, with
// third-lines and a focus bracket in the middle. White text with a shadow,
// yellow for a warning (NO CARD), amber for the setting being changed.
//
// A pinch on a setting (the bottom four, white balance, drive, self-timer,
// focus mode) selects it, and − / + beside the picture step it one notch
// among the values the body offers right now (AlphaFriend's `cam-step`). AF
// on the left is the shutter's half-press (`cam-af`): the bracket turns green
// when the camera focused, red when it could not.
//
// The remote can only change what the body lists a setter for, with values
// (`evt can`). On the α7R III in S mode that is shutter, ISO, white balance,
// drive and self-timer: aperture needs the dial on A or M, EV needs the EV
// dial at 0, and this body has no remote focus-mode or touch-AF at all
// (checked 2026-11-05). A setting it won't take is dim; a pinch on it says
// what to do on the camera. The same layout serves the α6000-series bodies:
// everything comes from their getEvent.

import { makeQuad, makeTappable, makeTexture, Z_IMAGE } from "../ui/Bitmap";
import { uiFail } from "../ui/Sound";

const WHITE = new vec4(0.96, 0.96, 0.94, 1);
const DIM = new vec4(0.6, 0.6, 0.58, 1);
const YELLOW = new vec4(1.0, 0.82, 0.2, 1);
const AMBER = new vec4(0.949, 0.639, 0.227, 1);
const Z_OVER = Z_IMAGE + 0.05;            // over the picture
const GREEN = new vec4(0.35, 0.9, 0.45, 1);
const RED = new vec4(1.0, 0.3, 0.25, 1);
const BOTTOM = ["ss", "f", "ev", "iso"];
const PICKABLE = ["ss", "f", "ev", "iso", "wb", "drive", "timer", "focus"];
// AlphaFriend's cam-step numbering
const STEP_INDEX: { [k: string]: number } = { ss: 0, f: 1, iso: 2, ev: 3, wb: 4, drive: 5, timer: 6, focus: 7 };

export type A7State = { [k: string]: string };

export class A7Screen {
  private root: SceneObject;
  private t: { [k: string]: Text } = {};
  private selected = "";
  private can: string[] = [];
  private stepButtons: SceneObject[] = [];
  private bracket: SceneObject[] = [];
  private bracketTex: { [k: string]: Texture } = {};
  private mode = "";
  private evstep = 1;

  constructor(
    parent: SceneObject,
    private font: Font,
    material: Material,
    private w: number,
    private h: number,
    y0: number,
    private onStep: (which: number, dir: number) => void,
    private onAf: () => void,
    private say: (s: string) => void,
  ) {
    this.root = global.scene.createSceneObject("A7Screen");
    this.root.setParent(parent);
    this.root.getTransform().setLocalPosition(new vec3(0, y0, 0));

    // third-lines and the focus bracket as thin strips: a Raster starts
    // opaque black, and a full-size overlay would hide the picture under it
    const grey = makeTexture(1, 1, new Uint8Array([90, 90, 88, 255]));
    const cyan = makeTexture(1, 1, new Uint8Array([140, 220, 230, 255]));
    const line = (x: number, y: number, lw: number, lh: number, tex: Texture): SceneObject => {
      const q = makeQuad(this.root, "Grid", lw, lh, material, tex, 11);
      q.getTransform().setLocalPosition(new vec3(x, y, Z_OVER - 0.02));
      return q;
    };
    const px = (c: vec4) => makeTexture(1, 1, new Uint8Array([c.x * 255, c.y * 255, c.z * 255, 255]));
    this.bracketTex = { idle: cyan, Focused: px(GREEN), Failed: px(RED) };
    for (const k of [-1, 1]) {
      line(k * w / 6, 0, 0.03, h, grey);
      line(0, k * h / 6, w, 0.03, grey);
    }
    const bw = 0.55, bh = 0.7, t = 0.06, arm = 0.25;
    for (const sx of [-1, 1]) {
      this.bracket.push(line(sx * bw, 0, t, bh * 2, cyan));
      for (const sy of [-1, 1]) this.bracket.push(line(sx * (bw - arm / 2), sy * bh, arm, t, cyan));
    }

    const top = h / 2 - 0.75, left = -w / 2 + 0.5, right = w / 2 - 0.5, bottom = -h / 2 + 0.75;
    this.text("mode", 44, new vec3(left + 0.5, top - 0.5, 0), HorizontalAlignment.Center);
    this.tapArea("mode", new vec3(left + 0.5, top - 0.5, 0), 1.6, 1.6, () => this.say(this.why("mode")));
    this.text("card", 17, new vec3(-w / 2 + 2.4, top, 0), HorizontalAlignment.Left);
    this.text("aspect", 17, new vec3(-1.2, top, 0), HorizontalAlignment.Center);
    this.text("quality", 17, new vec3(1.6, top, 0), HorizontalAlignment.Center);
    this.text("bat", 17, new vec3(right, top, 0), HorizontalAlignment.Right);
    this.text("drive", 16, new vec3(left, 1.9, 0), HorizontalAlignment.Left);
    this.text("timer", 16, new vec3(left, 0.9, 0), HorizontalAlignment.Left);
    this.text("focus", 16, new vec3(left, -0.4, 0), HorizontalAlignment.Left);
    this.text("af", 20, new vec3(left, -1.7, 0), HorizontalAlignment.Left).text = "[ AF ]";
    this.text("wb", 16, new vec3(right, 0.5, 0), HorizontalAlignment.Right);
    // side settings: tap areas reach in from the edge
    for (const [k, y] of [["drive", 1.9], ["timer", 0.9], ["focus", -0.4]] as [string, number][])
      this.tapArea(k, new vec3(left + 1.4, y, 0), 2.8, 0.9, () => this.pick(k));
    this.tapArea("af", new vec3(left + 1.2, -1.7, 0), 2.4, 1.1, () => this.af());
    this.tapArea("wb", new vec3(right - 1.4, 0.5, 0), 2.8, 0.9, () => this.pick("wb"));
    const xs = [-w * 0.3, -w * 0.1, w * 0.1, w * 0.3];
    BOTTOM.forEach((k, i) => {
      this.text(k, 22, new vec3(xs[i], bottom, 0), HorizontalAlignment.Center);
      this.tapArea(k, new vec3(xs[i], bottom, 0), w * 0.19, 1.4, () => this.pick(k));
    });
    // − and + just outside the picture, level with the bottom row
    for (const [s, dir, x] of [["−", -1, -w / 2 - 1.3], ["+", 1, w / 2 + 1.3]] as [string, number, number][]) {
      const t = this.text("step" + dir, 34, new vec3(x, bottom, 0), HorizontalAlignment.Center);
      t.text = s;
      t.textFill.color = AMBER;
      const o = t.getSceneObject();
      makeTappable(o, 2.2, 2.2, () => this.step(dir));
      this.stepButtons.push(o);
    }
    this.showStep(false);
  }

  private tapArea(k: string, at: vec3, w: number, h: number, act: () => void) {
    const hit = global.scene.createSceneObject("Pick " + k);
    hit.setParent(this.root);
    hit.getTransform().setLocalPosition(new vec3(at.x, at.y, Z_OVER + 0.1));
    makeTappable(hit, w, h, act);
  }

  // what to do on the camera when the remote can't change k
  private why(k: string): string {
    const m = modeLetter(this.mode);
    switch (k) {
      case "mode": return "MODE: TURN THE MODE DIAL ON THE CAMERA";
      case "f": return m === "S" || m === "P" ? "APERTURE: TURN THE MODE DIAL TO A OR M" : "APERTURE IS SET ON THE CAMERA NOW";
      case "ss": return m === "A" || m === "P" ? "SHUTTER: TURN THE MODE DIAL TO S OR M" : "SHUTTER IS SET ON THE CAMERA NOW";
      case "ev": return "EV: SET THE EV DIAL TO 0 TO USE THE REMOTE";
      case "focus": return "FOCUS MODE: SET ON THE CAMERA (NOT OFFERED OVER THE REMOTE)";
      case "af": return "AF: NOT OFFERED OVER THE REMOTE NOW";
      default: return label(k) + " IS SET ON THE CAMERA NOW";
    }
  }

  private af() {
    if (this.can.length && this.can.indexOf("af") < 0) { this.say(this.why("af")); uiFail(); return; }
    this.focusShown("idle");
    this.t["af"].textFill.color = AMBER;
    this.onAf();
  }

  // cam-af's answer: Focused, Failed, or anything else
  focusShown(status: string) {
    const tex = this.bracketTex[status] || this.bracketTex.idle;
    for (const q of this.bracket) {
      const vis = q.getComponent("Component.RenderMeshVisual") as RenderMeshVisual;
      vis.mainMaterial.mainPass.baseTex = tex;
    }
    if (status !== "idle") this.t["af"].textFill.color = status === "Focused" ? GREEN : status === "Failed" ? RED : WHITE;
  }

  setVisible(on: boolean) {
    this.root.enabled = on;
    if (on) this.showStep(!!this.selected);
  }

  // cam-evt's fields: mode ss f iso ev evstep wb aspect quality drive card left bat focus can
  update(v: A7State) {
    this.can = (v.can || "").trim().split(/\s+/).filter((x) => x);
    this.mode = v.mode || "";
    this.evstep = +(v.evstep || 1);
    this.set("mode", modeLetter(v.mode));
    this.set("timer", v.timer && +v.timer > 0 ? "TIMER " + v.timer + "S" : "TIMER OFF");
    const noCard = /no\s*media/i.test(v.card || "");
    this.set("card", noCard ? "NO CARD" : v.left ? v.left : "", noCard ? YELLOW : WHITE);
    this.set("aspect", v.aspect || "");
    this.set("quality", (v.quality || "").toUpperCase());
    this.set("bat", v.bat ? v.bat + "%" : "");
    this.set("drive", driveShort(v.drive));
    this.set("focus", v.focus ? v.focus.toUpperCase() : "FOCUS ON BODY");
    this.set("wb", wbShort(v.wb));
    this.set("ss", v.ss || "");
    this.set("f", v.f ? "F" + v.f : "");
    this.set("ev", v.ev !== undefined ? "EV " + evText(+v.ev, +(v.evstep || 1)) : "");
    this.set("iso", v.iso ? "ISO " + v.iso : "");
    this.paintPicks();
  }

  // AlphaFriend's answer to a step: "step <name> <value>"
  stepped(name: string, value: string, evstep = 1) {
    if (name === "f") this.set("f", "F" + value);
    else if (name === "iso") this.set("iso", "ISO " + value);
    else if (name === "ev") this.set("ev", "EV " + evText(+value, evstep || this.evstep));
    else if (name === "wb") this.set("wb", wbShort(value));
    else if (name === "drive") this.set("drive", driveShort(value));
    else if (name === "timer") this.set("timer", +value > 0 ? "TIMER " + value + "S" : "TIMER OFF");
    else if (name === "focus") this.set("focus", value.toUpperCase());
    else this.set(name, value);
    this.paintPicks();
  }

  private pick(k: string) {
    if (this.selected === k) { this.selected = ""; this.showStep(false); this.paintPicks(); return; }
    if (this.can.length && this.can.indexOf(k) < 0) {
      this.say(this.why(k));
      uiFail();
      return;
    }
    this.selected = k;
    this.showStep(true);
    this.paintPicks();
  }

  private step(dir: number) {
    if (this.selected) this.onStep(STEP_INDEX[this.selected], dir);
  }

  private showStep(on: boolean) {
    for (const o of this.stepButtons) o.enabled = on;
  }

  private paintPicks() {
    for (const k of PICKABLE) {
      const t = this.t[k];
      const big = BOTTOM.indexOf(k) >= 0 ? 22 : 16;
      t.textFill.color = k === this.selected ? AMBER : this.can.length && this.can.indexOf(k) < 0 ? DIM : WHITE;
      t.size = k === this.selected ? big + 4 : big;
    }
  }

  private set(k: string, s: string, c = WHITE) {
    const t = this.t[k];
    if (!t) return;
    t.text = s;
    if (PICKABLE.indexOf(k) < 0) t.textFill.color = c;
  }

  private text(k: string, size: number, at: vec3, align: HorizontalAlignment): Text {
    const o = global.scene.createSceneObject("Screen " + k);
    o.setParent(this.root);
    o.getTransform().setLocalPosition(new vec3(at.x, at.y, Z_OVER));
    const t = o.createComponent("Component.Text") as Text;
    t.font = this.font;
    t.size = size;
    t.text = "";
    t.horizontalAlignment = align;
    t.verticalAlignment = VerticalAlignment.Center;
    t.textFill.color = WHITE;
    t.dropshadowSettings.enabled = true;
    t.dropshadowSettings.fill.color = new vec4(0, 0, 0, 0.85);
    t.dropshadowSettings.offset = new vec2(0.06, -0.06);
    t.setRenderOrder(13);
    this.t[k] = t;
    return t;
  }
}

function label(k: string): string {
  return { ss: "SHUTTER SPEED", f: "APERTURE", ev: "EV", iso: "ISO", wb: "WHITE BALANCE", drive: "DRIVE", timer: "SELF-TIMER", focus: "FOCUS MODE" }[k] || k.toUpperCase();
}

function modeLetter(m: string): string {
  if (!m) return "";
  const k: { [k: string]: string } = {
    "Program Auto": "P", "Aperture": "A", "Shutter": "S", "Manual": "M",
    "Intelligent Auto": "AUTO", "Superior Auto": "AUTO", "Movie": "MOVIE",
  };
  return k[m] || m.toUpperCase();
}

function driveShort(d: string): string {
  if (!d) return "";
  if (/^single/i.test(d)) return "SINGLE";
  if (/spd|speed/i.test(d)) return "CONT HI+";
  if (/cont/i.test(d)) return "CONT";
  return d.toUpperCase();
}

function wbShort(w: string): string {
  if (!w) return "";
  if (/auto/i.test(w)) return "AWB";
  return w.replace(/^Fluorescent: /i, "FL ").replace(/^Color Temperature$/i, "K").toUpperCase();
}

// EV index in steps: stepIndex 1 is 1/3 EV, 2 is 1/2 EV
function evText(i: number, step: number): string {
  const ev = i / (step === 2 ? 2 : 3);
  if (Math.abs(ev) < 0.05) return "±0.0";
  return (ev > 0 ? "+" : "−") + Math.abs(ev).toFixed(1);
}

// "ILCE-7RM3" -> α7R III, "ILCE-6400" -> α6400
export function cameraName(model: string): string {
  const m = /^ILCE-(\d)(R|S|C)?M(\d)$/.exec(model || "");
  if (m) return "α" + m[1] + (m[2] || "") + " " + ["", "", "II", "III", "IV", "V"][+m[3]];
  const a6 = /^ILCE-(\d{4})$/.exec(model || "");
  if (a6) return "α" + a6[1];
  return model || "α";
}

// the short family name for chips: α7 or α6
export function cameraShort(model: string): string {
  const m = /^ILCE-(\d)/.exec(model || "");
  return m ? "α" + m[1] : "α7";
}
