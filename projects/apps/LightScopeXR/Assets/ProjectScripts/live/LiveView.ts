// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// LiveView.ts — Scout's picture in the air: frames over Wi-Fi, the shutter
// over Bluetooth.
//
// Spectacles have no mDNS, so the lens asks Scout for its address over its
// BLE console (`url?` -> `url http://<ip>/ ws://<ip>/ws`). Frames then come
// over a WebSocket to that IP: Scout answers each message with one 320x240
// JPEG. One request at a time gets ~2.7 fps on the glasses (a round trip per
// frame); four in flight got 8-9 fps with BLE live (LOG.md, LanProbe runs 4
// and 5). Each JPEG blob goes through RemoteMediaModule into a texture; if
// the decoder falls behind, frames are dropped rather than queued.
//
// SHOOT SCOUT sends `shoot` over BLE: Scout takes a dithered frame into its
// roll and answers `ok <n>`, and the Roll pulls it. Frame requests pause
// while it shoots, so the shot doesn't fight the stream for the camera.
// SHOOT α7 sends `shoot` to AlphaFriend, which fires the α7R III over the
// camera's Wi-Fi (switching Continuous drive to Single if it has to), then
// `pv`: AlphaFriend streams the shot's postview through TJpgDec and answers
// with a 198x132 dithered frame in base64, which joins the Roll. The line
// above the picture is the Sony's state from `cam?` and `cam-evt`.
// α7 shows `lvj` frames by default: the same liveview frame as a 160x106
// colour JPEG (~1.3 KB, re-encoded on AlphaFriend), so it looks like Scout's
// picture. A pinch on the picture switches to the 1-bit `lv` and back (kept).
// The SCOUT | α7 switch picks what the picture shows (kept for next launch). The 1-bit view loops `lv`
// on AlphaFriend: one liveview frame from the camera's stream, decoded at
// 1/4 and dithered to 160x106 (~2.1 KB, ~1 fps over BLE), shown in a 3:2
// quad where Scout's picture was. Scout's socket is let go meanwhile.
//
// Every shot also goes to RealReel in colour: a Scout shot as the Live
// frame that was up when SHOOT SCOUT went; an α7 shot (from the lens or
// AlphaFriend's A button, which the lens notices through `shots?`) as
// AlphaFriend's `pvj` colour thumbnail, before the dithered `pv` for the Roll.
//
// Each shot plays its camera's shutter (ui/Sound.ts, sounds in Assets/Sounds,
// credits in README.md).
//
// If the socket drops or stalls, it reconnects; after a few failures it asks
// Scout for its address again (its DHCP lease may have moved).

require("LensStudio:InternetModule");

import { Links } from "../ble/Links";
import { GoggleLink } from "../ble/GoggleLink";
import { b64ToBytes, makeQuad, makeTappable, makeTexture, setQuadTexture, unpack, Z_BACK, Z_IMAGE } from "../ui/Bitmap";
import { placeInFront } from "../ui/Place";
import { Raster, Rgb } from "../ui/Raster";
import { RollView } from "../roll/RollView";
import { RealReel } from "../reel/RealReel";
import { playOnce, setUiSounds, uiFail } from "../ui/Sound";

const AMBER: Rgb = [242, 163, 58];
const rgba = (c: Rgb) => new vec4(c[0] / 255, c[1] / 255, c[2] / 255, 1);
const IN_FLIGHT = 4;              // frame requests outstanding at once
const MAX_DECODING = 2;           // JPEGs being turned into textures at once
const STALL_MS = 3000;            // no frame this long: reconnect
const FAILS_BEFORE_ASK = 3;       // reconnects before asking url? again
const SHOOT_MS = 10000;
const SONY_SHOOT_MS = 25000;      // AF in AF-S can hold a shot ~15 s before it fails
const SONY_POLL_S = 15;
const PV_MS = 30000;
const LV_MS = 6000;
const LV_PAPER = [235, 222, 196, 255];        // warm white: the liveview reads as a picture, not a roll frame
const LV_INK = [0, 0, 0, 255];
const VIEW_KEY = "lightscope.view";
const A7_LOOK_KEY = "lightscope.a7look";              // fetch + decode on AlphaFriend (~2 s) and ~4.4 KB of base64 over BLE

@component
export class LiveView extends BaseScriptComponent {

  @input links: Links;

  @input
  @allowUndefined
  @hint("The Roll: refreshed after Scout shoots")
  roll: RollView;

  @input
  @hint("Unlit material with a base texture")
  imageMaterial: Material;

  @input
  @hint("A Text to copy for chips (mono)")
  monoText: Text;

  @input captionText: Text;

  @input
  @allowUndefined
  @hint("RealReel: colour copies of the shots")
  reel: RealReel;

  @input
  @allowUndefined
  @hint("A Text above the picture for the Sony's state")
  sonyText: Text;

  @input
  @allowUndefined
  @hint("Shutter sound for an α7 shot")
  alphaShutter: AudioTrackAsset;

  @input
  @allowUndefined
  @hint("Shutter sound for a Scout shot")
  scoutShutter: AudioTrackAsset;

  @input
  @allowUndefined
  @hint("Click for every button and switch (all panels)")
  uiClickSound: AudioTrackAsset;

  @input
  @allowUndefined
  @hint("When a shot or send fails (all panels)")
  uiFailSound: AudioTrackAsset;

  @input
  @allowUndefined
  frame: SceneObject;

  @input
  @allowUndefined
  camera: SceneObject;

  @input distance: number = 85;
  @input
  @hint("cm below eye level at 45 cm; negative is above")
  drop: number = -14;
  @input
  @hint("Degrees to the side; negative is left")
  yaw: number = 0;

  @input
  @hint("Picture width, cm (at 45 cm)")
  width: number = 16;

  private internet = require("LensStudio:InternetModule") as InternetModule;
  private media = require("LensStudio:RemoteMediaModule") as RemoteMediaModule;

  private picture: SceneObject;
  private viewLabels: Text[] = [];
  private exposure = "";          // the α7's settings, from the last cam-evt
  private wsUrl = "";
  private asking = false;
  private on = true;
  private view: "scout" | "a7" = "scout";
  private a7Quad: SceneObject;
  private a7Colour = true;        // lvj (colour) or lv (1-bit)
  private a7Run = 0;              // bumps to stop a running lv loop
  private a7Shown = 0;
  private a7At = 0;
  private ws: WebSocket = null;
  private wsSerial = 0;
  private fails = 0;
  private decoding = 0;
  private holding = false;        // shooting: don't ask for more frames
  private owed = 0;               // requests held back while shooting
  private lastFrameAt = 0;
  private shown = 0;
  private dropped = 0;
  private fpsAt = 0;
  private fps = 0;
  private note = "";
  private sonyBusy = false;
  private knownShots = -1;        // AlphaFriend's shot counter, as last seen
  private lastScoutTex: Texture = null;
  private sonyAsked = false;

  onAwake() {
    setUiSounds(this.getSceneObject(), this.uiClickSound, this.uiFailSound);
    this.createEvent("OnStartEvent").bind(() => {
      this.build();
      this.a7Colour = stored(A7_LOOK_KEY) !== "bits";
      if (stored(VIEW_KEY) === "a7") this.setView("a7");
      this.links.after(() => {
        if (this.frame && this.camera) placeInFront(this.frame, this.camera, this.distance, this.drop, this.yaw);
      }, 0.3);
      this.links.onChange(() => this.linkChanged());
      this.linkChanged();
      this.tick();
      this.sonyPoll();
      this.buttonPoll();
    });
    this.createEvent("OnDestroyEvent").bind(() => this.close(true));
  }

  // ---- the panel ----------------------------------------------------------

  private build() {
    const root = this.getSceneObject();
    const w = this.width, h = w * 3 / 4, y0 = 2;          // picture centre: room below for chips and caption
    const r = new Raster(Math.round(w * 16), Math.round(h * 16));
    r.frame(0, 0, r.w, r.h, 2, AMBER, 0.6);
    makeQuad(root, "LiveFrame", w + 0.4, h + 0.4, this.imageMaterial, r.texture(), 9)
      .getTransform().setLocalPosition(new vec3(0, y0, Z_BACK));
    this.picture = makeQuad(root, "Picture", w, h, this.imageMaterial, makeTexture(1, 1, new Uint8Array([0, 0, 0, 255])));
    this.picture.getTransform().setLocalPosition(new vec3(0, y0, Z_IMAGE));
    this.a7Quad = makeQuad(root, "A7", w, w * 2 / 3, this.imageMaterial, makeTexture(1, 1, new Uint8Array([0, 0, 0, 255])));
    this.a7Quad.getTransform().setLocalPosition(new vec3(0, y0, Z_IMAGE));
    this.a7Quad.enabled = false;
    makeTappable(this.a7Quad, w, w * 2 / 3, () => {
      this.a7Colour = !this.a7Colour;
      store(A7_LOOK_KEY, this.a7Colour ? "colour" : "bits");
      this.note = this.a7Colour ? "α7 · COLOUR" : "α7 · 1-BIT";
      this.links.after(() => { if (/^α7 · (COLOUR|1-BIT)$/.test(this.note)) this.note = ""; }, 2);
    });
    const cy = y0 - h / 2 - 1.6;
    this.chip(root, "SHOOT SCOUT", 5.6, new vec3(-5.4, cy, 0), () => this.shoot());
    this.chip(root, "SHOOT α7", 4.6, new vec3(0.0, cy, 0), () => this.shootSony());
    this.viewSwitch(root, 5.6, 1.6, new vec3(5.4, cy, 0));
    if (this.captionText) this.captionText.getSceneObject().getTransform().setLocalPosition(new vec3(0, y0 - h / 2 - 3.6, 0.2));
    if (this.sonyText) this.sonyText.getSceneObject().getTransform().setLocalPosition(new vec3(0, y0 + h / 2 + 1.0, 0.2));
    this.say("WAITING FOR SCOUT");
    this.sony("α7R III · WAITING FOR ALPHAFRIEND");
  }

  private chip(parent: SceneObject, s: string, w: number, at: vec3, act: () => void, h = 1.6): Text {
    const r = new Raster(Math.round(w * 40), Math.round(h * 40)), rad = r.h / 2 - 1;
    for (let y = 0; y < r.h; y++) for (let x = 0; x < r.w; x++) {
      const cx = Math.min(Math.max(x, rad + 1), r.w - rad - 1), d = Math.hypot(x - cx, y - r.h / 2);
      if (Math.abs(d - rad + 1) <= 1.1) r.plot(x, y, AMBER);
    }
    const q = makeQuad(parent, "Chip " + s, w, h, this.imageMaterial, r.texture(), 9);
    q.getTransform().setLocalPosition(new vec3(at.x, at.y, Z_BACK));
    makeTappable(q, w, h, act);
    const o = global.scene.createSceneObject("Label " + s);
    o.setParent(parent);
    o.getTransform().setLocalPosition(new vec3(at.x, at.y, Z_IMAGE));
    const t = o.createComponent("Component.Text") as Text;
    t.font = this.monoText.font;
    t.size = 18;
    t.text = s;
    t.horizontalAlignment = HorizontalAlignment.Center;
    t.verticalAlignment = VerticalAlignment.Center;
    t.textFill.color = rgba(AMBER);
    t.setRenderOrder(12);
    return t;
  }

  // SCOUT | α7: a pill, the side in view filled to its rounded end and cut
  // straight at the middle (half a jellybean), with dark text on it.
  private viewSwitch(parent: SceneObject, w: number, h: number, at: vec3) {
    this.viewTex = [this.pillTexture(w, h, -1), this.pillTexture(w, h, 1)];
    this.viewPill = makeQuad(parent, "Chip VIEW", w, h, this.imageMaterial, this.viewTex[0], 9);
    this.viewPill.getTransform().setLocalPosition(new vec3(at.x, at.y, Z_BACK));
    const sides: ["scout" | "a7", string, number][] = [["scout", "SCOUT", -w / 4], ["a7", "α7", w / 4]];
    for (const [v, s, dx] of sides) {
      const hit = global.scene.createSceneObject("View " + s);       // a tap area, nothing drawn
      hit.setParent(parent);
      hit.getTransform().setLocalPosition(new vec3(at.x + dx, at.y, Z_BACK));
      makeTappable(hit, w / 2, h, () => this.setView(v));
      const o = global.scene.createSceneObject("Label VIEW " + s);
      o.setParent(parent);
      o.getTransform().setLocalPosition(new vec3(at.x + dx, at.y, Z_IMAGE));
      const t = o.createComponent("Component.Text") as Text;
      t.font = this.monoText.font;
      t.size = 18;
      t.text = s;
      t.horizontalAlignment = HorizontalAlignment.Center;
      t.verticalAlignment = VerticalAlignment.Center;
      t.setRenderOrder(12);
      this.viewLabels.push(t);
    }
    this.paintSwitch();
  }

  // side -1: the left half filled, 1: the right
  private pillTexture(w: number, h: number, side: number): Texture {
    const r = new Raster(Math.round(w * 40), Math.round(h * 40)), rad = r.h / 2 - 1, mid = r.w / 2;
    for (let y = 0; y < r.h; y++) for (let x = 0; x < r.w; x++) {
      const cx = Math.min(Math.max(x, rad + 1), r.w - rad - 1), d = Math.hypot(x - cx, y - r.h / 2);
      const inside = d <= rad - 1.5, edge = Math.abs(d - rad + 1) <= 1.1;
      const filled = inside && (side < 0 ? x < mid - 1 : x > mid + 1);
      const cut = inside && Math.abs(x - mid) <= 1;
      if (edge || filled || cut) r.plot(x, y, AMBER);
    }
    return r.texture();
  }

  private viewPill: SceneObject;
  private viewTex: Texture[] = [];

  private paintSwitch() {
    const a7 = this.view === "a7";
    setQuadTexture(this.viewPill, this.viewTex[a7 ? 1 : 0]);
    this.viewLabels[0].textFill.color = a7 ? rgba(AMBER) : new vec4(0, 0, 0, 1);
    this.viewLabels[1].textFill.color = a7 ? new vec4(0, 0, 0, 1) : rgba(AMBER);
  }

  // ---- Scout over BLE -----------------------------------------------------

  private linkChanged() {
    const s = this.links.scout;
    if (!s) { this.say("NO SCOUT LINK (scoutName empty)"); return; }
    if (s.live && !this.wsUrl) this.askUrl();
    if (!s.live && !this.ws) this.say("WAITING FOR SCOUT");
  }

  private askUrl() {
    const s = this.links.scout;
    if (this.asking || !s || !s.live) return;
    this.asking = true;
    this.say("ASKING SCOUT FOR ITS ADDRESS");
    s.send("url?").then((r) => {
      this.asking = false;
      const m = /ws:\/\/\S+/.exec(r);
      if (!m) {
        this.say("SCOUT IS NOT ON WI-FI");
        print("[live] url?: " + r.trim());
        this.links.after(() => this.askUrl(), 10);
        return;
      }
      this.wsUrl = m[0];
      this.fails = 0;
      print("[live] scout at " + this.wsUrl);
      if (this.on) this.open();
    }, (e) => {
      this.asking = false;
      print("[live] url?: " + e);
      this.say("SCOUT NOT ANSWERING");
      this.links.after(() => this.askUrl(), 5);
    });
  }

  private shoot() {
    const s = this.links.scout;
    if (this.holding) return;
    if (!s || !s.live) { this.say("SHOOT · SCOUT NOT CONNECTED"); uiFail(); return; }
    this.holding = true;
    this.say("SHOOTING");
    const t0 = Date.now();
    const colour = this.lastScoutTex;                    // what was on the panel when SHOOT went
    const job = this.reel ? this.reel.begin("SCOUT") : null;
    if (job) job.stage("SHOOTING", 1.5);
    s.send("shoot", SHOOT_MS).then((r) => {
      const m = /ok\s+(\d+)/.exec(r);
      if (m) playOnce(this.getSceneObject(), this.scoutShutter);
      if (job) {
        if (m && colour) job.done(colour, 320, 240, "FRAME " + ("000" + m[1]).slice(-4));
        else job.fail(m ? "NO LIVE FRAME TO KEEP" : "SHOT FAILED");
      }
      this.note = m ? "SHOT " + ("000" + m[1]).slice(-4) + " ON SCOUT" : "SHOOT: " + r.trim().toUpperCase();
      if (!m) uiFail();
      print("[live] shoot " + r.trim() + " in " + (Date.now() - t0) + " ms");
      if (m && this.roll) this.roll.refresh();
    }, (e) => {
      this.note = "SHOOT FAILED";
      uiFail();
      print("[live] shoot: " + e);
      if (job) job.fail("NO ANSWER");
    }).then(() => {
      this.holding = false;
      this.links.after(() => { this.note = ""; }, 4);
      this.release();
    });
  }

  // ---- the α7R III through AlphaFriend ------------------------------------

  private shootSony() {
    const a = this.links.alpha;
    if (this.sonyBusy) return;
    if (!a || !a.live) { this.note = "α7 · ALPHAFRIEND NOT CONNECTED"; uiFail(); this.links.after(() => { this.note = ""; }, 4); return; }
    this.sonyBusy = true;
    this.sony("α7R III · SHOOTING");
    const t0 = Date.now();
    a.send("shoot", SONY_SHOOT_MS).then(async (r) => {
      const ms = Date.now() - t0, txt = r.trim();
      print("[live] α7 shoot: " + txt.replace(/\n/g, " | ") + " in " + ms + " ms");
      const err = /err\s+(-?\d+)/.exec(txt);
      if (err) { uiFail(); this.note = "α7 SHOT FAILED (" + err[1] + (err[1] === "40400" ? ", FOCUS?" : "") + ")"; return; }
      if (!/\bok\b/.test(txt)) { uiFail(); this.note = "α7: " + txt.toUpperCase(); return; }
      playOnce(this.getSceneObject(), this.alphaShutter);
      this.knownShots = await this.shotsQ(a, this.knownShots);
      await this.afterSonyShot(a, "α7 · LENS");
    }, (e) => {
      this.note = "α7 SHOOT: NO ANSWER";
      uiFail();
      print("[live] α7 shoot: " + e);
    }).then(() => {
      this.sonyBusy = false;
      this.links.after(() => { this.note = ""; }, 5);
      this.sonyRead();
    });
  }

  // A shot is on the camera: colour thumbnail to RealReel first, then the
  // dithered postview to the Roll.
  private async afterSonyShot(a: GoggleLink, label: string) {
    this.note = "α7 SHOT · FETCHING THE PICTURES";
    this.say(this.note);
    await this.colourThumb(a, label + " #" + this.knownShots);
    await this.postview(a);
  }

  private async shotsQ(a: GoggleLink, fallback: number): Promise<number> {
    try {
      const m = /shots\s+(\d+)/.exec(await a.send("shots?"));
      return m ? +m[1] : fallback;
    } catch (e) {
      return fallback;
    }
  }

  // `pvj` -> "pvj <bytes> <w> <h>" then base64 lines of a colour JPEG.
  private async colourThumb(a: GoggleLink, label: string) {
    if (!this.reel) return;
    const job = this.reel.begin(label);
    job.meta = this.exposure;
    job.stage("ALPHAFRIEND FETCHING + ENCODING", 3.5);
    try {
      const r = await a.send("pvj", PV_MS);
      const head = /pvj\s+(\d+)\s+(\d+)\s+(\d+)/.exec(r);
      if (!head) { job.fail("PVJ: " + r.trim().toUpperCase().slice(0, 24)); return; }
      job.stepDone();
      job.stage("DECODING", 0.3);
      let b64 = "";
      for (const l of r.split("\n")) {
        const t = l.trim();
        if (/^[A-Za-z0-9+/]+=*$/.test(t)) b64 += t;
      }
      const w = +head[2], h = +head[3], bytes = +head[1];
      await new Promise<void>((done) => Base64.decodeTextureAsync(b64, (tex) => {
        job.done(tex, w, h, (bytes / 1024).toFixed(1) + " KB");
        done();
      }, () => { job.fail("JPEG DID NOT DECODE"); done(); }));
    } catch (e) {
      job.fail("NO ANSWER");
      print("[live] pvj: " + e);
    }
  }

  // `pv` -> "pv <w> <h>" then base64 lines: a 1-bit frame for the Roll.
  private async postview(a: GoggleLink) {
    const t0 = Date.now();
    try {
      const r = await a.send("pv", PV_MS);
      const head = /pv\s+(\d+)\s+(\d+)/.exec(r);
      if (!head) { uiFail(); this.note = "α7 POSTVIEW: " + r.trim().toUpperCase().slice(0, 30); print("[live] pv: " + r.trim()); return; }
      const w = +head[1], h = +head[2];
      let text = "";
      for (const l of r.split("\n")) {
        const t = l.trim();
        if (/^[A-Za-z0-9+/]+=*$/.test(t)) text += t;
      }
      const bits = b64ToBytes(text);
      if (bits.length !== ((w + 7) >> 3) * h) { this.note = "α7 POSTVIEW CUT SHORT"; print("[live] pv: " + bits.length + " B for " + w + "x" + h); return; }
      print("[live] pv " + w + "x" + h + " in " + (Date.now() - t0) + " ms");
      if (this.roll) this.roll.addShot(w, h, bits);
      this.note = "α7 SHOT · IN THE ROLL";
    } catch (e) {
      this.note = "α7 POSTVIEW: NO ANSWER";
      uiFail();
      print("[live] pv: " + e);
    }
  }

  // AlphaFriend's A button shoots without the lens: every 3 s ask `shots?`
  // and fetch the pictures of any shot we haven't seen.
  private async buttonPoll() {
    const a = this.links.alpha;
    if (a && a.live && a.idle && !this.sonyBusy) {
      const n = await this.shotsQ(a, this.knownShots);
      if (this.knownShots < 0) this.knownShots = n;
      else if (n > this.knownShots && !this.sonyBusy) {
        this.knownShots = n;
        print("[live] α7 button shot #" + n);
        playOnce(this.getSceneObject(), this.alphaShutter);
        this.sonyBusy = true;
        try { await this.afterSonyShot(a, "α7 · BUTTON"); } finally {
          this.sonyBusy = false;
          this.links.after(() => { this.note = ""; }, 5);
        }
      }
    } else if (!a || !a.live) {
      this.knownShots = -1;                    // AlphaFriend may reboot; take its count afresh
    }
    this.links.after(() => this.buttonPoll(), 3);
  }

  // On connect and every SONY_POLL_S: `cam?` and, when the camera is linked,
  // `cam-evt` for the exposure.
  private sonyPoll() {
    const a = this.links.alpha;
    if (a && a.live && !this.sonyBusy) this.sonyRead();
    else if (!a) this.sony("α7R III · NO ALPHAFRIEND LINK");
    else if (!a.live) { this.sony("α7R III · WAITING FOR ALPHAFRIEND"); this.sonyAsked = false; }
    this.links.after(() => this.sonyPoll(), this.sonyAsked ? SONY_POLL_S : 2);
  }

  private async sonyRead() {
    const a = this.links.alpha;
    if (!a || !a.live) return;
    this.sonyAsked = true;
    try {
      const c = /cam\s+(\S+)\s+(\S+)/.exec(await a.send("cam?"));
      if (!c || c[1] !== "ready") {
        this.sony("α7R III · " + (c ? "CAMERA " + c[1].toUpperCase() : "?") + " · JOIN ON ALPHAFRIEND");
        return;
      }
      const r = await a.send("cam-evt", 6000), v: { [k: string]: string } = {};
      for (const l of r.split("\n")) {
        const m = /^evt\s+(\S+)\s+(.+)$/.exec(l.trim());
        if (m) v[m[1]] = m[2].trim();
      }
      const ev = v.ev !== undefined && !isNaN(+v.ev) ? ((+v.ev >= 0 ? "+" : "") + (+v.ev / 3).toFixed(1) + " EV") : "";
      const parts = [modeShort(v.mode), v.ss, v.f ? "f/" + v.f : "", v.iso ? "ISO " + v.iso : "", ev].filter((x) => x);
      this.exposure = parts.join(" · ");
      this.sony([c[2] === "ILCE-7RM3" ? "α7R III" : c[2]].concat(parts).join(" · "));
    } catch (e) {
      print("[live] α7 read: " + e);
      this.sony("α7R III · ALPHAFRIEND NOT ANSWERING");
    }
  }

  private sony(s: string) {
    if (this.sonyText) {
      this.sonyText.text = s;
      this.sonyText.textFill.color = rgba(AMBER);
    }
  }

  // ---- frames over Wi-Fi --------------------------------------------------

  // SCOUT: Scout's stream over Wi-Fi. α7: AlphaFriend's `lv` loop over BLE.
  private setView(v: "scout" | "a7") {
    if (v === this.view && (v === "scout" || this.a7Quad.enabled)) return;
    this.view = v;
    store(VIEW_KEY, v);
    this.paintSwitch();
    this.a7Run++;                                        // any lv loop stops
    this.on = v === "scout";
    this.picture.enabled = v === "scout";
    this.a7Quad.enabled = v === "a7";
    if (v === "scout") {
      if (this.wsUrl) this.open(); else this.askUrl();
    } else {
      this.close(true);
      this.say("α7 · STARTING THE LIVEVIEW");
      this.a7Loop(this.a7Run);
    }
  }

  // One `lv` at a time while VIEW α7 is on; a shot goes first.
  private async a7Loop(run: number) {
    while (run === this.a7Run) {
      const a = this.links.alpha;
      if (!a || !a.live) { this.note = "α7 · WAITING FOR ALPHAFRIEND"; await this.pause(2); continue; }
      if (this.sonyBusy) { await this.pause(0.5); continue; }
      try {
        const colour = this.a7Colour;
        const r = await a.send(colour ? "lvj" : "lv", LV_MS);
        if (run !== this.a7Run) return;
        if (colour) {
          await this.showLvj(r);
          continue;
        }
        const head = /lv\s+(\d+)\s+(\d+)/.exec(r);
        if (!head) {
          this.note = "α7 LIVEVIEW: " + r.trim().toUpperCase().slice(0, 30);
          print("[live] lv: " + r.trim());
          await this.pause(2);
          continue;
        }
        const w = +head[1], h = +head[2];
        let text = "";
        for (const l of r.split("\n")) {
          const t = l.trim();
          if (/^[A-Za-z0-9+/]+=*$/.test(t)) text += t;
        }
        const bits = b64ToBytes(text);
        if (bits.length !== ((w + 7) >> 3) * h) { print("[live] lv: " + bits.length + " B for " + w + "x" + h); continue; }
        setQuadTexture(this.a7Quad, makeTexture(w, h, unpack(bits, w, h, LV_INK, LV_PAPER)));
        this.a7Shown++;
        if (/^α7 (LIVEVIEW|· WAITING)/.test(this.note)) this.note = "";   // a shot's note stays its 5 s
      } catch (e) {
        print("[live] lv: " + e);
        this.note = "α7 LIVEVIEW: NO ANSWER";
        await this.pause(1);
      }
    }
  }

  // "lvj <bytes> <w> <h>" then base64 lines of a colour JPEG
  private async showLvj(r: string) {
    const head = /lvj\s+(\d+)\s+(\d+)\s+(\d+)/.exec(r);
    if (!head) {
      this.note = "α7 LIVEVIEW: " + r.trim().toUpperCase().slice(0, 30);
      print("[live] lvj: " + r.trim());
      await this.pause(2);
      return;
    }
    let b64 = "";
    for (const l of r.split("\n")) {
      const t = l.trim();
      if (/^[A-Za-z0-9+/]+=*$/.test(t)) b64 += t;
    }
    await new Promise<void>((done) => Base64.decodeTextureAsync(b64, (tex) => {
      if (this.view === "a7") setQuadTexture(this.a7Quad, tex);
      this.a7Shown++;
      if (/^α7 (LIVEVIEW|· WAITING)/.test(this.note)) this.note = "";
      done();
    }, () => { print("[live] lvj: did not decode"); done(); }));
  }

  private pause(seconds: number): Promise<void> {
    return new Promise((r) => this.links.after(r, seconds));
  }

  private open() {
    if (this.ws || !this.on || !this.wsUrl) return;
    const serial = ++this.wsSerial;
    let ws: WebSocket;
    try {
      ws = this.internet.createWebSocket(this.wsUrl);
    } catch (e) {
      print("[live] createWebSocket: " + e);
      this.failed(serial);
      return;
    }
    this.ws = ws;
    ws.binaryType = "blob";
    this.owed = 0;
    this.lastFrameAt = Date.now();
    ws.onopen = () => {
      if (serial !== this.wsSerial) return;
      this.fails = 0;
      print("[live] streaming from " + this.wsUrl);
      for (let i = 0; i < IN_FLIGHT; i++) this.request();
    };
    ws.onerror = () => this.failed(serial);
    ws.onclose = () => this.failed(serial);
    ws.onmessage = (e) => {
      if (serial !== this.wsSerial) return;
      if (typeof e.data === "string") {             // "err no-frame": Scout's camera was busy; ask again
        this.links.after(() => { if (serial === this.wsSerial) this.request(); }, 0.2);
        return;
      }
      this.lastFrameAt = Date.now();
      this.request();                                   // ask for the next while this one decodes
      if (this.decoding >= MAX_DECODING) { this.dropped++; return; }
      this.decoding++;
      this.media.loadResourceAsImageTexture(this.internet.makeResourceFromBlob(e.data as Blob), (t) => {
        this.decoding--;
        if (serial !== this.wsSerial) return;
        setQuadTexture(this.picture, t);
        this.lastScoutTex = t;
        this.shown++;
      }, (m) => {
        this.decoding--;
        this.dropped++;
        print("[live] decode: " + m);
      });
    };
  }

  private request() {
    if (this.holding) { this.owed++; return; }
    try { this.ws.send("f"); } catch (e) { /* closing; failed() reconnects */ }
  }

  private release() {
    const n = this.owed;
    this.owed = 0;
    if (this.ws) for (let i = 0; i < n; i++) this.request();
  }

  // Drop the socket. WebSocket.close() on a stalled connection held the
  // lens's script for ~27 s on the glasses (2026-11-04), freezing every
  // panel, so a dead socket is only abandoned: its handlers go quiet (serial)
  // and the glasses' stack times it out. close() is called only when the
  // socket is still answering (switching to α7, lens closing).
  private close(graceful = false) {
    this.wsSerial++;
    const ws = this.ws;
    this.ws = null;
    if (!ws) return;
    ws.onmessage = () => {};
    ws.onerror = () => {};
    ws.onclose = () => {};
    if (graceful && this.lastFrameAt && Date.now() - this.lastFrameAt < 1000) {
      try { ws.close(); } catch (e) { /* already closed */ }
    }
  }

  // The socket closed, errored or stalled: try again shortly, and after a few
  // misses ask Scout where it is now.
  private failed(serial: number) {
    if (serial !== this.wsSerial) return;
    this.close();
    if (!this.on) return;
    this.fails++;
    print("[live] socket lost (" + this.fails + ")");
    if (this.fails >= FAILS_BEFORE_ASK) {
      this.wsUrl = "";
      this.fails = 0;
      this.say("LOOKING FOR SCOUT ON WI-FI");
      this.askUrl();
      return;
    }
    this.say("RECONNECTING");
    this.links.after(() => this.open(), 2);
  }

  // Once a second: the caption, and the stall check.
  private tick() {
    const now = Date.now();
    if (this.ws && !this.holding && now - this.lastFrameAt > STALL_MS) {
      print("[live] no frame in " + STALL_MS + " ms");
      this.failed(this.wsSerial);
    }
    if (this.fpsAt) this.fps = this.shown * 1000 / Math.max(1, now - this.fpsAt);
    this.fpsAt = now;
    this.shown = 0;
    if (this.view === "a7") {
      const fps = this.a7Shown * 1000 / Math.max(1, now - (this.a7At || now - 1000));
      this.a7At = now;
      this.a7Shown = 0;
      this.say(this.note || "α7 LIVE " + (this.a7Colour ? "COLOUR " : "1-BIT ") + fps.toFixed(1) + " FPS · PINCH TO SWITCH");
    } else if (this.ws && this.on && !this.holding) {
      const ble = this.links.scout && this.links.scout.live ? "" : " · BLE DOWN";
      this.say(this.note || ("LIVE " + this.fps.toFixed(1) + " FPS" + (this.dropped ? " · " + this.dropped + " DROPPED" : "") + ble));
      this.dropped = 0;
    } else if (this.note) {
      this.say(this.note);
    }
    this.links.after(() => this.tick(), 1);
  }

  private say(s: string) {
    if (this.captionText) {
      this.captionText.text = s;
      this.captionText.textFill.color = rgba(AMBER);
    }
  }
}

function modeShort(m: string): string {
  if (!m) return "";
  const k: { [k: string]: string } = { "Program Auto": "P", "Aperture": "A", "Shutter": "S", "Manual": "M", "Intelligent Auto": "AUTO" };
  return k[m] || m.toUpperCase();
}

function stored(key: string): string {
  try { return global.persistentStorageSystem.store.getString(key); } catch (e) { return ""; }
}

function store(key: string, v: string) {
  try { global.persistentStorageSystem.store.putString(key, v); } catch (e) { /* not kept */ }
}
