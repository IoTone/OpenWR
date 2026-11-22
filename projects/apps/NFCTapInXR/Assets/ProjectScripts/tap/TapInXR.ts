// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// TapInXR.ts — NFC taps become things in the air.
//
// The lens holds one Bluetooth link, to the NFCTapIn reader (an M5 ATOM
// Matrix with a PN532; see TapLink.ts). Each "TAP <id>" plays one of four
// scenes in front of the wearer:
//
//   0 CARD     a trading card that can be pinched and held
//   1 SIGN-IN  "Signing in as Iron Man", then a secured-session confirmation
//   2 MODEL    a 3D asset materializing in space
//   3 FARE     a transit fare taken from a card balance that counts down
//
// Until a tag is tapped the stage shows the reader link itself, step by step:
// searching (with progress), connecting, opening the tap channel, ready. If
// the reader is not found, or Bluetooth refuses, it says so, says what to
// check, and offers RETRY. The link never holds the lens up: the chips and
// scenes work the whole time (see TapLink.ts for how the scan is kept light).
//
// Which tag plays which scene: list your tag IDs in TapTags.ts (or set the
// four tag inputs). Scenes left without a tag learn: the first unknown tags
// tapped are bound to them in order and remembered. The chips under the stage play a scene without a
// tag; RESET forgets the learned tags and restores the fare balance.
//
// Everything is drawn in code (Paint.ts), so the scenes need no image
// assets. Layout is in cm at 45 cm and scaled by Place.ts.

import { makeQuad, makeTappable } from "../ui/Bitmap";
import { placeInFront } from "../ui/Place";
import { Paint, RGBA } from "./Paint";
import { TapLink } from "./TapLink";
import { TAG_SCENES, normalizeTagId } from "./TapTags";
import { Interactable } from "SpectaclesInteractionKit.lspkg/Components/Interaction/Interactable/Interactable";
import { InteractableManipulation } from "SpectaclesInteractionKit.lspkg/Components/Interaction/InteractableManipulation/InteractableManipulation";

// Glass palette: light colours only, because black is clear on the glasses.
const FOG: RGBA = [226, 234, 238, 255];
const GLACIER: RGBA = [120, 183, 212, 255];
const HIVIS: RGBA = [255, 210, 63, 255];
const MOSS: RGBA = [126, 214, 132, 255];
const ALPENGLOW: RGBA = [255, 128, 112, 255];

const v4 = (c: RGBA, a = 1) => new vec4(c[0] / 255, c[1] / 255, c[2] / 255, (c[3] / 255) * a);
const clamp01 = (x: number) => (x < 0 ? 0 : x > 1 ? 1 : x);
const span = (t: number, a: number, b: number) => clamp01((t - a) / (b - a));
const easeOut = (x: number) => 1 - Math.pow(1 - clamp01(x), 3);
const easeBack = (x: number) => { const c = 1.70158, y = clamp01(x) - 1; return 1 + (c + 1) * y * y * y + c * y * y; };
const DEG = Math.PI / 180;

const SCENE_NAMES = ["TRADING CARD", "SIGN-IN", "3D MODEL", "FARE"];
const CHIP_NAMES = ["CARD", "SIGN-IN", "MODEL", "FARE"];
const TAGS_KEY = "nfctapin.tags";
const BALANCE_KEY = "nfctapin.balance";
const START_CENTS = 2000;
const FARE_CENTS = 275;

interface Scene {
  root: SceneObject;
  start(id: string): void;
  update(t: number): void;      // t = seconds since start
}

@component
export class TapInXR extends BaseScriptComponent {

  @input
  @hint("Bluetooth Central Module asset")
  bluetoothModule: Bluetooth.BluetoothCentralModule;

  @input
  @hint("Unlit material with a base texture")
  imageMaterial: Material;

  @input
  @hint("A Text to copy the font from; it is hidden")
  textTemplate: Text;

  @input camera: SceneObject;

  @input
  @hint("The reader's advertised name")
  deviceName: string = "NFCTapIn";

  @input distance: number = 85;
  @input drop: number = 0;

  @input
  @hint("Extra tag ID(s) for the trading card, comma-separated; see TapTags.ts")
  cardTag: string = "";
  @input
  @hint("Extra tag ID(s) for the sign-in; see TapTags.ts")
  signInTag: string = "";
  @input
  @hint("Extra tag ID(s) for the 3D model; see TapTags.ts")
  modelTag: string = "";
  @input
  @hint("Extra tag ID(s) for the fare; see TapTags.ts")
  fareTag: string = "";

  @input
  @allowUndefined
  @hint("Optional 3D model to materialize; empty = a built-in crystal")
  modelPrefab: ObjectPrefab;

  @input
  @hint("Size the model prefab is scaled by")
  modelScale: number = 1;

  @input
  @hint("In the editor preview (no Bluetooth), play the scenes in turn")
  previewDemo: boolean = true;

  private link: TapLink;
  private stage: SceneObject;
  private scenes: Scene[] = [];
  private idle: Scene;
  private current: Scene;
  private startedAt = 0;
  private caption: Text;
  private captionHold = 0;          // getTime() until which a tap/bad-read caption stays
  private chips: SceneObject[] = [];
  private fixed: string[][] = [];   // configured tags per scene (TapTags.ts + inputs)
  private learned: string[] = ["", "", "", ""];
  private cents = START_CENTS;

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => this.begin());
  }

  private begin() {
    this.textTemplate.getSceneObject().enabled = false;
    this.fixed = [this.cardTag, this.signInTag, this.modelTag, this.fareTag].map((input, i) =>
      (TAG_SCENES[i] || []).concat((input || "").split(",")).map(normalizeTagId).filter((t) => t.length > 0));
    this.loadState();

    this.stage = global.scene.createSceneObject("TapStage");
    this.stage.setParent(this.getSceneObject());
    this.buildChrome();
    this.idle = this.buildIdle();
    this.scenes = [this.buildCard(), this.buildSignIn(), this.buildModel(), this.buildFare()];
    this.show(this.idle, "");

    this.after(() => placeInFront(this.stage, this.camera, this.distance, this.drop), 0.3);
    this.createEvent("UpdateEvent").bind(() => this.tick());

    this.link = new TapLink(this.deviceName, this.bluetoothModule, () => this.onLink(), (id) => this.onTap(id), () => this.onBadRead(),
      (fn, s) => this.after(fn, s), () => getTime());
    this.createEvent("OnDestroyEvent").bind(() => this.link.stop());
    const editor = global.deviceInfoSystem.isEditor();
    // The first scan waits until the stage has been drawn and placed.
    if (!editor) this.link.start(1.2);
    else if (this.previewDemo) this.previewScript();
    this.onLink();
    print("[tapin] ready; tags " + this.fixed.map((f, i) => SCENE_NAMES[i] + "=" + (f.join("/") || this.learned[i] || "learn")).join(" "));
  }

  // The editor preview has no Bluetooth. Act the link out instead, so every
  // state can be seen: a search that fails, a retry that connects, then the
  // four scenes in turn.
  private previewScript() {
    const L = this.link, at = (s: number, fn: () => void) => this.after(fn, s);
    at(0.5, () => L.simulate("searching", "", 1));
    at(3.0, () => L.simulate("searching", "", 2));
    at(5.5, () => L.simulate("notfound"));
    at(9.5, () => L.simulate("failed", "Bluetooth scan refused: PERMISSION_DENIED"));
    at(13.0, () => L.simulate("searching", "", 1));
    at(15.0, () => L.simulate("connecting"));
    at(16.2, () => L.simulate("subscribing"));
    at(17.2, () => L.simulate("live"));
    let n = 0;
    const next = () => {
      print("[tapin] preview: " + SCENE_NAMES[n % 4]);
      this.play(n % 4, "PREVIEW");
      n++;
      this.after(next, 5);
    };
    at(21, next);
  }

  // The link changed state: update the caption, and come back to the link
  // view when it needs attention and nothing is playing.
  private onLink() {
    if (!this.caption) return;
    const st = this.link.state;
    print("[tapin] reader: " + st + (this.link.error ? " (" + this.link.error + ")" : ""));
    if (st === "notfound" || st === "failed") this.captionHold = 0;   // an error replaces a held caption
    this.linkCaption();
  }

  private retryLink() {
    if (global.deviceInfoSystem.isEditor()) {
      this.link.simulate("searching", "", 1);
      this.after(() => this.link.simulate("connecting"), 2);
      this.after(() => this.link.simulate("subscribing"), 3);
      this.after(() => this.link.simulate("live"), 4);
    } else {
      this.link.retry();
    }
    this.show(this.idle, "");
    this.chips.forEach((c) => this.tint(c, 0.45));
  }

  // ---- taps -----------------------------------------------------------------

  // The scene a tag plays: a configured tag (TapTags.ts) first, then one
  // learned earlier. -1 = unknown.
  private sceneOf(id: string): number {
    for (let i = 0; i < 4; i++) if (this.fixed[i].indexOf(id) >= 0) return i;
    for (let i = 0; i < 4; i++) if (!this.fixed[i].length && this.learned[i] === id) return i;
    return -1;
  }

  private onTap(raw: string) {
    const id = normalizeTagId(raw);
    print("[tapin] tap " + id);
    let i = this.sceneOf(id);
    if (i < 0) {
      // Learn into the first scene that has no configured and no learned tag.
      for (let k = 0; k < 4 && i < 0; k++) if (!this.fixed[k].length && !this.learned[k]) i = k;
      if (i < 0) {
        const all = this.fixed.every((f) => f.length > 0);
        this.say("UNKNOWN TAG " + id + (all ? " : ADD IT TO TapTags.ts" : " : PINCH RESET TO RE-LEARN"), ALPENGLOW, 4);
        print("[tapin] unknown tag " + id + "; add it to a scene in TapTags.ts");
        return;
      }
      this.learned[i] = id;
      this.saveState();
      print("[tapin] learned " + id + " as " + SCENE_NAMES[i]);
    }
    this.play(i, id);
  }

  private onBadRead() {
    this.say("BAD READ : TAP AGAIN", HIVIS, 1.5);
  }

  private play(i: number, id: string) {
    this.say((id ? "TAG " + id + " : " : "") + SCENE_NAMES[i], FOG, 4);
    this.show(this.scenes[i], id);
    this.chips.forEach((c, k) => this.tint(c, k === i ? 1 : 0.45));
  }

  private show(s: Scene, id: string) {
    for (const x of this.scenes) x.root.enabled = false;
    this.idle.root.enabled = false;
    s.root.enabled = true;
    this.current = s;
    this.startedAt = getTime();
    s.start(id);
    s.update(0);
  }

  private tick() {
    if (this.current) this.current.update(getTime() - this.startedAt);
    if (this.captionHold && getTime() > this.captionHold) {
      this.captionHold = 0;
      this.linkCaption();
    }
  }

  private reset() {
    this.learned = ["", "", "", ""];
    this.cents = START_CENTS;
    this.saveState();
    this.chips.forEach((c) => this.tint(c, 0.45));
    this.show(this.idle, "");
    this.say("TAGS FORGOTTEN : BALANCE RESTORED", HIVIS, 3);
  }

  // ---- caption --------------------------------------------------------------

  private linkCaption() {
    if (this.captionHold || !this.caption || !this.link) return;
    const name = this.deviceName.toUpperCase();
    switch (this.link.state) {
      case "live": this.setCaption("READER CONNECTED : TAP A TAG", MOSS); break;
      case "searching": this.setCaption("SEARCHING FOR " + name, GLACIER); break;
      case "connecting": this.setCaption("CONNECTING TO " + name, GLACIER); break;
      case "subscribing": this.setCaption("OPENING THE TAP CHANNEL", GLACIER); break;
      case "lost": this.setCaption("READER LOST : SEARCHING AGAIN", HIVIS); break;
      case "notfound": this.setCaption("READER NOT FOUND : PINCH RETRY", ALPENGLOW); break;
      case "failed": this.setCaption("READER ERROR : PINCH RETRY", ALPENGLOW); break;
      default: this.setCaption("STARTING BLUETOOTH", GLACIER);
    }
  }

  private say(s: string, c: RGBA, seconds: number) {
    this.setCaption(s, c);
    this.captionHold = getTime() + seconds;
  }

  private setCaption(s: string, c: RGBA) {
    this.caption.text = s;
    this.caption.textFill.color = v4(c);
  }

  // ---- state ----------------------------------------------------------------

  private loadState() {
    try {
      const store = global.persistentStorageSystem.store;
      const tags = store.getString(TAGS_KEY);
      if (tags) tags.split(",").forEach((t, i) => { if (i < 4) this.learned[i] = t; });
      if (store.has(BALANCE_KEY)) this.cents = store.getInt(BALANCE_KEY);
    } catch (e) {
      print("[tapin] no saved state: " + e);
    }
  }

  private saveState() {
    try {
      const store = global.persistentStorageSystem.store;
      store.putString(TAGS_KEY, this.learned.join(","));
      store.putInt(BALANCE_KEY, this.cents);
    } catch (e) {
      print("[tapin] could not save: " + e);
    }
  }

  // ---- building blocks ------------------------------------------------------

  private after(fn: () => void, seconds: number) {
    const ev = this.createEvent("DelayedCallbackEvent");
    ev.bind(() => { this.removeEvent(ev); fn(); });
    ev.reset(seconds);
  }

  private node(parent: SceneObject, name: string, x = 0, y = 0, z = 0): SceneObject {
    const o = global.scene.createSceneObject(name);
    o.setParent(parent);
    o.getTransform().setLocalPosition(new vec3(x, y, z));
    return o;
  }

  // Centred text (a script-made Text has a 15 cm box; centring keeps it on its object).
  private text(parent: SceneObject, s: string, size: number, c: RGBA, x: number, y: number, z = 0.6): Text {
    const t = this.node(parent, "T:" + s.substring(0, 12), x, y, z).createComponent("Component.Text") as Text;
    t.font = this.textTemplate.font;
    t.size = size;
    t.text = s;
    t.horizontalAlignment = HorizontalAlignment.Center;
    t.verticalAlignment = VerticalAlignment.Center;
    t.textFill.color = v4(c);
    t.depthTest = false;
    t.setRenderOrder(30);
    return t;
  }

  private quad(parent: SceneObject, name: string, w: number, h: number, tex: Texture, x: number, y: number, z = 0.4, order = 10): SceneObject {
    const q = makeQuad(parent, name, w, h, this.imageMaterial, tex, order);
    q.getTransform().setLocalPosition(new vec3(x, y, z));
    return q;
  }

  // Brightness and opacity of a quad together (the material is premultiplied).
  private tint(q: SceneObject, a: number) {
    const vis = q.getComponent("Component.RenderMeshVisual") as RenderMeshVisual;
    vis.mainMaterial.mainPass.baseColor = new vec4(a, a, a, a);
  }

  private fade(t: Text, c: RGBA, a: number) {
    t.textFill.color = v4(c, clamp01(a));
  }

  private ringTex(size: number, thick: number, c: RGBA, a0 = 0, a1 = 360): Texture {
    const p = new Paint(size, size);
    p.ring(size / 2, size / 2, size / 2 - thick, thick, c, a0, a1);
    return p.toTexture();
  }

  private checkTex(c: RGBA): Texture {
    const p = new Paint(128, 128);
    p.line(30, 68, 54, 92, 14, c);
    p.line(54, 92, 100, 38, 14, c);
    return p.toTexture();
  }

  // ---- the frame around the scenes: title, caption, chips ----------------------

  private buildChrome() {
    this.text(this.stage, "NFC TAP-IN", 16, GLACIER, 0, 7.9);
    this.caption = this.text(this.stage, "", 16, GLACIER, 0, -7.4);

    const w = 3.3, h = 1.4, gap = 0.35;
    const p = new Paint(132, 56);
    p.roundRect(0, 0, 132, 56, 26, [120, 183, 212, 255]);
    p.roundRect(3, 3, 126, 50, 23, [30, 62, 78, 255]);
    const chipTex = p.toTexture();
    const names = CHIP_NAMES.concat(["RESET"]);
    const total = names.length * w + (names.length - 1) * gap;
    names.forEach((name, i) => {
      const x = -total / 2 + w / 2 + i * (w + gap);
      const q = this.quad(this.stage, "Chip" + name, w, h, chipTex, x, -9.0);
      this.text(q, name, 12, i < 4 ? FOG : HIVIS, 0, 0, 0.2);
      makeTappable(q, w, h, () => (i < 4 ? this.play(i, "") : this.reset()));
      this.tint(q, i < 4 ? 0.45 : 0.7);
      if (i < 4) this.chips.push(q);
    });
  }

  // ---- idle: the reader link, and waiting for a tag -------------------------------

  // Long platform messages are cut to fit one line of the stage.
  private fit(s: string, max: number): string {
    s = s.replace(/\s+/g, " ").trim().toUpperCase();
    return s.length > max ? s.substring(0, max - 3) + "..." : s;
  }

  private buildIdle(): Scene {
    const root = this.node(this.stage, "Idle");
    const cy = 2.0;
    // ready: a steady ring that sends out a wave
    const ring = this.quad(root, "IdleRing", 6, 6, this.ringTex(256, 10, GLACIER), 0, cy);
    const wave = this.quad(root, "IdleWave", 6, 6, this.ringTex(256, 6, GLACIER), 0, cy, 0.3, 9);
    // working: a turning arc on a faint track
    const track = this.quad(root, "LinkTrack", 6, 6, this.ringTex(256, 10, [120, 183, 212, 70]), 0, cy, 0.3, 9);
    const arc = this.quad(root, "LinkArc", 6, 6, this.ringTex(256, 10, GLACIER, 0, 110), 0, cy);
    // trouble: a warm ring
    const alarm = this.quad(root, "LinkAlarm", 6, 6, this.ringTex(256, 10, ALPENGLOW), 0, cy);
    const centre = this.text(root, "", 30, FOG, 0, cy);
    const title = this.text(root, "", 17, FOG, 0, -2.3);
    const line1 = this.text(root, "", 13, GLACIER, 0, -3.5);
    const line2 = this.text(root, "", 13, GLACIER, 0, -4.5);
    // search progress
    const BAR = 7;
    const dot = (c: RGBA) => { const p = new Paint(4, 4); p.rect(0, 0, 4, 4, c); return p.toTexture(); };
    const barBack = this.quad(root, "BarBack", BAR, 0.22, dot([120, 183, 212, 80]), 0, -5.5, 0.3, 9);
    const barFill = this.quad(root, "BarFill", BAR, 0.22, dot(GLACIER), 0, -5.5);
    // retry
    const cp = new Paint(176, 64);
    cp.roundRect(0, 0, 176, 64, 30, ALPENGLOW);
    cp.roundRect(4, 4, 168, 56, 26, [86, 44, 40, 255]);
    const retry = this.quad(root, "Retry", 4.4, 1.6, cp.toTexture(), 0, -6.0);
    this.text(retry, "RETRY", 14, FOG, 0, 0, 0.2);
    makeTappable(retry, 4.4, 1.6, () => this.retryLink());

    const name = this.deviceName.toUpperCase();
    let shown = "";
    const setTexts = (st: string) => {
      const L = this.link;
      const key = st + "/" + (L ? L.attempt + "/" + L.error : "");
      if (key === shown) return;
      shown = key;
      const texts = (c: string, t: string, a: string, b: string, tc: RGBA, lc: RGBA) => {
        centre.text = c; title.text = t; line1.text = a; line2.text = b;
        centre.textFill.color = v4(tc); title.textFill.color = v4(tc);
        line1.textFill.color = v4(lc); line2.textFill.color = v4(lc);
      };
      switch (st) {
        case "live": texts("TAP", "READER CONNECTED", "HOLD A TAG ON THE READER", "", FOG, GLACIER); title.textFill.color = v4(MOSS); break;
        case "searching": texts("", "LOOKING FOR " + name, "SCAN " + L.attempt + " OF " + L.tries, "", FOG, GLACIER); break;
        case "connecting": texts("", "FOUND " + name, "CONNECTING", "", FOG, GLACIER); break;
        case "subscribing": texts("", "CONNECTED", "OPENING THE TAP CHANNEL", "", FOG, GLACIER); break;
        case "lost": texts("", "READER LOST", "SEARCHING AGAIN, SCAN " + L.attempt + " OF " + L.tries, "", HIVIS, GLACIER); break;
        case "notfound": texts("!", name + " NOT FOUND", "IS THE READER ON AND BLINKING BLUE?", "SOLID GREEN: ANOTHER DEVICE HOLDS IT", ALPENGLOW, FOG); break;
        case "failed": texts("!", "COULD NOT REACH THE READER", this.fit(L.error || "unknown error", 44), "CHECK BLUETOOTH, THEN RETRY", ALPENGLOW, FOG); break;
        default: texts("", "STARTING BLUETOOTH", "", "", FOG, GLACIER);
      }
    };

    return {
      root,
      start: () => { shown = ""; },
      update: (t) => {
        const L = this.link, st = L ? L.state : "idle";
        setTexts(st);
        const ready = st === "live", bad = st === "notfound" || st === "failed", working = !ready && !bad;
        ring.enabled = wave.enabled = ready;
        track.enabled = arc.enabled = working;
        alarm.enabled = retry.enabled = bad;
        const scanning = st === "searching" || st === "lost";
        barBack.enabled = barFill.enabled = scanning;
        if (ready) {
          const k = (t * 0.6) % 1, s = 1 + k * 0.7;
          wave.getTransform().setLocalScale(new vec3(s, s, 1));
          this.tint(wave, 0.8 * (1 - k));
          this.tint(ring, 0.75 + 0.25 * Math.sin(t * 2.4));
        } else if (working) {
          arc.getTransform().setLocalRotation(quat.angleAxis(-t * 220 * DEG, vec3.forward()));
          if (scanning) {
            const p = Math.max(0.02, L.searchProgress);
            barFill.getTransform().setLocalScale(new vec3(p, 1, 1));
            barFill.getTransform().setLocalPosition(new vec3(-BAR / 2 + (BAR * p) / 2, -5.5, 0.4));
          }
        } else {
          this.tint(alarm, 0.7 + 0.3 * Math.sin(t * 3.2));
        }
      },
    };
  }

  // ---- 0: the trading card ------------------------------------------------------

  private buildCard(): Scene {
    const root = this.node(this.stage, "CardScene");
    const holder = this.node(root, "CardHolder", 0, 0.6);     // what the hand moves
    const inner = this.node(holder, "CardInner");             // what the idle motion moves
    const W = 8, H = 11.2, PW = 320, PH = 448, cm = H / PH;
    const yOf = (py: number) => H / 2 - py * cm;

    const p = new Paint(PW, PH);
    p.roundRect(0, 0, PW, PH, 24, HIVIS, [255, 170, 60, 255]);
    p.roundRect(10, 10, PW - 20, PH - 20, 16, [52, 120, 146, 255], [30, 78, 110, 255]);
    p.roundRect(22, 20, PW - 44, 42, 10, [255, 255, 255, 46]);                       // name band
    p.roundRect(24, 72, PW - 48, 190, 12, [168, 226, 240, 255], [104, 164, 214, 255]);   // art window
    p.disc(238, 112, 20, [255, 244, 196, 255]);                                       // a pale sun
    p.roundRect(24, 232, PW - 48, 30, 8, [96, 176, 120, 255], [70, 140, 96, 255]);    // ground
    p.sprite(CREATURE, CREATURE_COLOURS, 80, 86, 10);
    p.roundRect(22, 282, PW - 44, 58, 10, [255, 255, 255, 40]);                       // move 1
    p.roundRect(22, 350, PW - 44, 58, 10, [255, 255, 255, 40]);                       // move 2
    const card = this.quad(inner, "Card", W, H, p.toTexture(), 0, 0);

    this.text(inner, "GLIMMERPUP", 17, FOG, -1.25, yOf(41));
    this.text(inner, "HP 70", 14, HIVIS, 2.75, yOf(41));
    this.text(inner, "AURORA TYPE / LV 12", 10, FOG, 0, yOf(271));
    this.text(inner, "PRISM BARK      30", 13, FOG, 0, yOf(311));
    this.text(inner, "TAIL LANTERN    50", 13, FOG, 0, yOf(379));
    this.text(inner, "NFC TAP-IN SERIES  001/004", 8, GLACIER, 0, yOf(424));
    const hint = this.text(root, "PINCH THE CARD TO HOLD IT", 13, GLACIER, 0, -6.2);

    // Pinch and hold: the interaction kit moves the holder.
    try {
      const col = holder.createComponent("Physics.ColliderComponent") as ColliderComponent;
      const shape = Shape.createBoxShape();
      shape.size = new vec3(W, H, 0.6);
      col.shape = shape;
      holder.createComponent(Interactable.getTypeName());
      holder.createComponent(InteractableManipulation.getTypeName());
    } catch (e) {
      print("[tapin] card is not holdable: " + e);
    }

    return {
      root,
      start: () => {
        const tr = holder.getTransform();
        tr.setLocalPosition(new vec3(0, 0.6, 0));
        tr.setLocalRotation(quat.quatIdentity());
        tr.setLocalScale(new vec3(1, 1, 1));
      },
      update: (t) => {
        const a = easeBack(span(t, 0, 0.8));
        const tr = inner.getTransform();
        // Arrive edge-on and turn to face; then a slow idle sway.
        const turn = (1 - easeOut(span(t, 0, 0.9))) * 80 + (t > 0.9 ? Math.sin((t - 0.9) * 0.9) * 7 : 0);
        tr.setLocalRotation(quat.angleAxis(turn * DEG, vec3.up()));
        tr.setLocalScale(new vec3(a, a, a));
        tr.setLocalPosition(new vec3(0, (1 - easeOut(span(t, 0, 0.8))) * -3 + (t > 0.9 ? Math.sin((t - 0.9) * 1.7) * 0.18 : 0), 0));
        this.tint(card, 0.6 + 0.4 * span(t, 0, 0.6));
        this.fade(hint, GLACIER, span(t, 1.2, 1.8));
      },
    };
  }

  // ---- 1: signing in --------------------------------------------------------------

  private buildSignIn(): Scene {
    const root = this.node(this.stage, "SignInScene");
    const cy = 2.3;
    const spinner = this.quad(root, "Spinner", 6.4, 6.4, this.ringTex(256, 12, GLACIER, 0, 250), 0, cy);
    const track = this.quad(root, "Track", 6.4, 6.4, this.ringTex(256, 12, [120, 183, 212, 70]), 0, cy, 0.3, 9);
    const done = this.quad(root, "Done", 6.4, 6.4, this.ringTex(256, 14, MOSS), 0, cy, 0.45, 11);
    const glow = this.quad(root, "Glow", 6.4, 6.4, this.ringTex(256, 6, MOSS), 0, cy, 0.35, 9);
    const check = this.quad(root, "Check", 3.4, 3.4, this.checkTex(MOSS), 0, cy, 0.5, 12);
    const pct = this.text(root, "0%", 24, FOG, 0, cy);
    const top = this.text(root, "SIGNING IN AS", 13, GLACIER, 0, -2.2);
    this.text(root, "IRON MAN", 38, FOG, 0, -3.9);
    const status = this.text(root, "", 15, GLACIER, 0, -5.5);
    const session = this.text(root, "", 12, MOSS, 0, -6.4);
    let tag = "", wasSecured = false;
    const T_DONE = 2.3;

    return {
      root,
      start: (id) => {
        tag = id || "DEMO";
        wasSecured = false;
        top.text = "SIGNING IN AS";
        this.fade(top, GLACIER, 1);
        // a session label derived from the tag, so each tag looks like its own session
        let h = 0;
        for (let i = 0; i < tag.length; i++) h = (h * 31 + tag.charCodeAt(i)) & 0xffff;
        session.text = "SESSION " + ("0000" + h.toString(16).toUpperCase()).slice(-4) + " / ENCRYPTED END TO END";
      },
      update: (t) => {
        const secured = t >= T_DONE;
        spinner.enabled = track.enabled = pct.getSceneObject().enabled = !secured;
        done.enabled = check.enabled = secured;
        glow.enabled = secured && t < T_DONE + 1.6;
        if (!secured) {
          spinner.getTransform().setLocalRotation(quat.angleAxis(-t * 300 * DEG, vec3.forward()));
          pct.text = Math.round(easeOut(t / T_DONE) * 100) + "%";
          status.text = t < 0.75 ? "READING TAG " + tag : t < 1.5 ? "VERIFYING CREDENTIAL" : "EXCHANGING KEYS";
          this.fade(status, GLACIER, 1);
          this.fade(session, MOSS, 0);
        } else {
          if (!wasSecured) {
            wasSecured = true;
            top.text = "SIGNED IN AS";
            status.text = "SESSION SECURED";
          }
          const u = t - T_DONE;
          const pop = easeBack(span(u, 0, 0.45));
          done.getTransform().setLocalScale(new vec3(0.8 + 0.2 * pop, 0.8 + 0.2 * pop, 1));
          check.getTransform().setLocalScale(new vec3(pop, pop, 1));
          const g = span(u, 0, 1.6), gs = 1 + g * 0.9;
          glow.getTransform().setLocalScale(new vec3(gs, gs, 1));
          this.tint(glow, 0.9 * (1 - g));
          this.fade(top, MOSS, 1);
          this.fade(status, MOSS, 1);
          this.fade(session, MOSS, span(u, 0.5, 1.0));
        }
      },
    };
  }

  // ---- 2: a 3D asset materializing -------------------------------------------------

  private buildModel(): Scene {
    const root = this.node(this.stage, "ModelScene");
    const cy = 1.4;
    const pivot = this.node(root, "ModelPivot", 0, cy, 1.5);
    let label = "CRYSTAL.GLB / 12 FACES / BUILT IN";
    let made = false;
    if (this.modelPrefab) {
      try {
        const m = this.modelPrefab.instantiate(pivot);
        m.getTransform().setLocalScale(new vec3(this.modelScale, this.modelScale, this.modelScale));
        label = this.modelPrefab.name.toUpperCase();
        made = true;
      } catch (e) {
        print("[tapin] model prefab: " + e);
      }
    }
    if (!made) this.buildCrystal(pivot);

    // Scan rings: rings squashed into ellipses, so they read as hoops around the model.
    const ringTex = this.ringTex(256, 8, GLACIER);
    const hoops = [0, 1, 2].map((i) => this.quad(root, "Hoop" + i, 8.4, 2.3, ringTex, 0, cy, 0.5, 14));
    const pad = this.quad(root, "Pad", 8.4, 2.3, this.ringTex(256, 5, [120, 183, 212, 150]), 0, cy - 4.1, 0.3, 8);
    const status = this.text(root, "", 14, GLACIER, 0, -4.9);
    const detail = this.text(root, label, 12, GLACIER, 0, -5.9);
    const T_DONE = 2.2;

    return {
      root,
      start: () => {},
      update: (t) => {
        const grow = easeBack(span(t, 0.35, 1.9));
        const tr = pivot.getTransform();
        tr.setLocalScale(new vec3(grow, grow, grow));
        // spins fast while it forms, then settles to a slow turn
        const angle = 720 * easeOut(span(t, 0, T_DONE)) + Math.max(0, t - T_DONE) * 28 + t * 20;
        tr.setLocalRotation(quat.angleAxis(angle * DEG, vec3.up()));
        tr.setLocalPosition(new vec3(0, cy + (t > T_DONE ? Math.sin((t - T_DONE) * 1.4) * 0.2 : 0), 1.5));
        hoops.forEach((h, i) => {
          const k = span(t, i * 0.3, i * 0.3 + 1.5);
          h.enabled = k > 0 && k < 1;
          h.getTransform().setLocalPosition(new vec3(0, cy - 4 + k * 8, 0.5));
          const s = 0.55 + 0.45 * Math.sin(k * Math.PI);
          h.getTransform().setLocalScale(new vec3(s, s, 1));
          this.tint(h, Math.sin(k * Math.PI));
        });
        this.tint(pad, 0.5 + 0.3 * Math.sin(t * 3));
        const dots = "...".substring(0, 1 + (Math.floor(t * 4) % 3));
        status.text = t < T_DONE ? "MATERIALIZING" + dots : "ASSET LOADED";
        this.fade(status, t < T_DONE ? GLACIER : MOSS, 1);
        this.fade(detail, GLACIER, span(t, T_DONE, T_DONE + 0.5));
      },
    };
  }

  // A faceted crystal: a six-sided bipyramid, each face with its own baked shade.
  private buildCrystal(parent: SceneObject) {
    const shades = 8, pal = new Paint(shades, 1);
    for (let i = 0; i < shades; i++) {
      const k = i / (shades - 1);
      pal.rect(i, 0, 1, 1, [50 + 150 * k, 110 + 140 * k, 190 + 65 * k, 255]);
    }
    const top = new vec3(0, 3.1, 0), bottom = new vec3(0, -3.3, 0), girdle: vec3[] = [];
    for (let i = 0; i < 6; i++) girdle.push(new vec3(Math.cos(i * 60 * DEG) * 2.2, 0.7, Math.sin(i * 60 * DEG) * 2.2));
    const light = new vec3(0.45, 0.75, 0.5).normalize();
    const b = new MeshBuilder([{ name: "position", components: 3 }, { name: "texture0", components: 2 }]);
    b.topology = MeshTopology.Triangles;
    b.indexType = MeshIndexType.UInt16;
    let n = 0;
    const face = (p0: vec3, p1: vec3, p2: vec3) => {
      let normal = p1.sub(p0).cross(p2.sub(p0)).normalize();
      const centre = p0.add(p1).add(p2).uniformScale(1 / 3);
      if (normal.dot(centre) < 0) { const s = p1; p1 = p2; p2 = s; normal = normal.uniformScale(-1); }   // wind outwards
      const shade = Math.round((0.25 + 0.75 * Math.max(0, normal.dot(light))) * (shades - 1));
      const u = (shade + 0.5) / shades;
      for (const p of [p0, p1, p2]) b.appendVerticesInterleaved([p.x, p.y, p.z, u, 0.5]);
      b.appendIndices([n, n + 1, n + 2]);
      n += 3;
    };
    for (let i = 0; i < 6; i++) {
      const a = girdle[i], c = girdle[(i + 1) % 6];
      face(top, a, c);
      face(bottom, c, a);
    }
    b.updateMesh();
    const o = this.node(parent, "Crystal");
    const vis = o.createComponent("Component.RenderMeshVisual") as RenderMeshVisual;
    vis.mesh = b.getMesh();
    const mat = this.imageMaterial.clone();
    mat.mainPass.baseTex = pal.toTexture();
    vis.mainMaterial = mat;
    vis.setRenderOrder(12);
  }

  // ---- 3: the fare ---------------------------------------------------------------

  private buildFare(): Scene {
    const root = this.node(this.stage, "FareScene");
    const slide = this.node(root, "PassSlide", 0, 1.6);
    const W = 9.6, H = 6, PW = 384, PH = 240;
    const p = new Paint(PW, PH);
    p.roundRect(0, 0, PW, PH, 26, [70, 170, 220, 255], [40, 104, 190, 255]);
    p.roundRect(0, 186, PW, 18, 0, [255, 210, 63, 235]);                 // a route stripe
    for (let i = 0; i < 3; i++) p.ring(318, 62, 12 + i * 11, 5, [255, 255, 255, 220], 45, 135);   // contactless mark
    const pass = this.quad(slide, "Pass", W, H, p.toTexture(), 0, 0);
    this.text(slide, "METRO PASS", 15, FOG, -2.3, 2.0);
    this.text(slide, "BALANCE", 11, FOG, -3.0, 0.8);
    const amount = this.text(slide, "", 36, FOG, -0.4, -0.45);
    this.text(slide, "STORED VALUE", 9, FOG, 2.6, -2.45);
    const fare = this.text(root, "", 17, HIVIS, 0, -3.0);
    const gate = this.text(root, "", 15, MOSS, 0, -4.6);
    const tick = this.quad(root, "GateTick", 1.5, 1.5, this.checkTex(MOSS), -3.3, -4.6, 0.5, 12);
    const cross = this.quad(root, "GateCross", 1.5, 1.5, this.crossTex(), 0, -1.9, 0.5, 12);
    const money = (cents: number) => "$" + (cents / 100).toFixed(2);
    let from = 0, to = 0, ok = true;

    return {
      root,
      start: () => {
        from = this.cents;
        ok = this.cents >= FARE_CENTS;
        to = ok ? this.cents - FARE_CENTS : START_CENTS;      // not enough: the pass tops itself up
        this.cents = to;
        this.saveState();
        fare.text = ok ? "FARE  -" + money(FARE_CENTS) : "NOT ENOUGH FOR " + money(FARE_CENTS) + " FARE";
        gate.text = ok ? "GATE OPEN" : "AUTO TOP-UP  +" + money(START_CENTS - from);
        print("[tapin] fare: " + money(from) + " -> " + money(to));
      },
      update: (t) => {
        const arrive = easeOut(span(t, 0, 0.45));
        const tr = slide.getTransform();
        tr.setLocalPosition(new vec3((1 - arrive) * 7, 1.6, 0));
        tr.setLocalRotation(quat.angleAxis((1 - arrive) * -18 * DEG, vec3.forward()));
        this.tint(pass, arrive);
        const k = easeOut(span(t, 0.7, 1.7));
        const shown = Math.round(from + (to - from) * k);
        amount.text = money(shown);
        this.fade(amount, k > 0 && k < 1 ? HIVIS : FOG, arrive);
        const c = ok ? HIVIS : ALPENGLOW;
        this.fade(fare, c, span(t, 0.5, 0.8));
        fare.getSceneObject().getTransform().setLocalPosition(new vec3(0, -3.0 + (1 - easeOut(span(t, 0.5, 0.9))) * -0.8, 0.6));
        const g = span(t, 1.7, 2.0);
        this.fade(gate, ok ? MOSS : HIVIS, g);
        const pop = easeBack(span(t, 1.7, 2.1));
        tick.enabled = ok && g > 0;
        cross.enabled = !ok && t > 0.5 && t < 1.7;
        tick.getTransform().setLocalScale(new vec3(pop, pop, 1));
      },
    };
  }

  private crossTex(): Texture {
    const p = new Paint(128, 128);
    p.line(36, 36, 92, 92, 14, ALPENGLOW);
    p.line(92, 36, 36, 92, 14, ALPENGLOW);
    return p.toTexture();
  }
}

// The creature on the card: 16 x 16, one character per pixel.
const CREATURE: string[] = [
  "....oo....oo....",
  "...obbo..obbo...",
  "...obbboobbbo...",
  "..obbbbbbbbbbo..",
  "..obebbbbbbebo..",
  "..obpbbbbbbpbo..",
  "..obbbbllbbbbo..",
  "..obcbllllbcbo..",
  "...obbllllbbo..t",
  "...obbbllbbbo.tt",
  "..obbbbbbbbbbot.",
  "..obbbbbbbbbbo..",
  "..obbobbbbobbo..",
  "..obbo.oo.obbo..",
  "...oo......oo...",
  "................",
];

const CREATURE_COLOURS: { [ch: string]: RGBA } = {
  o: [46, 58, 92, 255],
  b: [255, 176, 96, 255],
  l: [255, 232, 184, 255],
  e: [255, 255, 255, 255],
  p: [40, 44, 70, 255],
  c: [255, 120, 120, 255],
  t: [255, 244, 130, 255],
};
