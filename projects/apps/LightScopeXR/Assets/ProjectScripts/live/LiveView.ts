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
// camera's Wi-Fi (switching Continuous drive to Single if it has to). The
// line above the picture is the Sony's state from `cam?` and `cam-evt`.
// LIVE pauses and resumes the stream.
//
// If the socket drops or stalls, it reconnects; after a few failures it asks
// Scout for its address again (its DHCP lease may have moved).

require("LensStudio:InternetModule");

import { Links } from "../ble/Links";
import { makeQuad, makeTappable, makeTexture, setQuadTexture, Z_BACK, Z_IMAGE } from "../ui/Bitmap";
import { placeInFront } from "../ui/Place";
import { Raster, Rgb } from "../ui/Raster";
import { RollView } from "../roll/RollView";

const AMBER: Rgb = [242, 163, 58];
const rgba = (c: Rgb) => new vec4(c[0] / 255, c[1] / 255, c[2] / 255, 1);
const IN_FLIGHT = 4;              // frame requests outstanding at once
const MAX_DECODING = 2;           // JPEGs being turned into textures at once
const STALL_MS = 3000;            // no frame this long: reconnect
const FAILS_BEFORE_ASK = 3;       // reconnects before asking url? again
const SHOOT_MS = 10000;
const SONY_SHOOT_MS = 25000;      // AF in AF-S can hold a shot ~15 s before it fails
const SONY_POLL_S = 15;

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
  @hint("A Text above the picture for the Sony's state")
  sonyText: Text;

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
  private liveChip: Text;
  private wsUrl = "";
  private asking = false;
  private on = true;
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
  private sonyAsked = false;

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => {
      this.build();
      this.links.after(() => {
        if (this.frame && this.camera) placeInFront(this.frame, this.camera, this.distance, this.drop, this.yaw);
      }, 0.3);
      this.links.onChange(() => this.linkChanged());
      this.linkChanged();
      this.tick();
      this.sonyPoll();
    });
    this.createEvent("OnDestroyEvent").bind(() => this.close());
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
    const cy = y0 - h / 2 - 1.6;
    this.chip(root, "SHOOT SCOUT", 6.0, new vec3(-5.0, cy, 0), () => this.shoot());
    this.chip(root, "SHOOT α7", 5.0, new vec3(0.9, cy, 0), () => this.shootSony());
    this.liveChip = this.chip(root, "LIVE", 3.6, new vec3(6.2, cy, 0), () => this.toggle());
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
      this.links.after(() => this.askUrl(), 5);
    });
  }

  private shoot() {
    const s = this.links.scout;
    if (this.holding) return;
    if (!s || !s.live) { this.say("SHOOT · SCOUT NOT CONNECTED"); return; }
    this.holding = true;
    this.say("SHOOTING");
    const t0 = Date.now();
    s.send("shoot", SHOOT_MS).then((r) => {
      const m = /ok\s+(\d+)/.exec(r);
      this.note = m ? "SHOT " + ("000" + m[1]).slice(-4) + " ON SCOUT" : "SHOOT: " + r.trim().toUpperCase();
      print("[live] shoot " + r.trim() + " in " + (Date.now() - t0) + " ms");
      if (m && this.roll) this.roll.refresh();
    }, (e) => {
      this.note = "SHOOT FAILED";
      print("[live] shoot: " + e);
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
    if (!a || !a.live) { this.note = "α7 · ALPHAFRIEND NOT CONNECTED"; this.links.after(() => { this.note = ""; }, 4); return; }
    this.sonyBusy = true;
    this.sony("α7R III · SHOOTING");
    const t0 = Date.now();
    a.send("shoot", SONY_SHOOT_MS).then((r) => {
      const ms = Date.now() - t0, txt = r.trim();
      print("[live] α7 shoot: " + txt.replace(/\n/g, " | ") + " in " + ms + " ms");
      const err = /err\s+(-?\d+)/.exec(txt);
      this.note = err ? "α7 SHOT FAILED (" + err[1] + (err[1] === "40400" ? ", FOCUS?" : "") + ")"
        : /\bok\b/.test(txt) ? "α7 SHOT · " + (ms / 1000).toFixed(1) + " S" : "α7: " + txt.toUpperCase();
    }, (e) => {
      this.note = "α7 SHOOT: NO ANSWER";
      print("[live] α7 shoot: " + e);
    }).then(() => {
      this.sonyBusy = false;
      this.links.after(() => { this.note = ""; }, 5);
      this.sonyRead();
    });
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
      const parts = [c[2] === "ILCE-7RM3" ? "α7R III" : c[2], modeShort(v.mode), v.ss, v.f ? "f/" + v.f : "", v.iso ? "ISO " + v.iso : "", ev];
      this.sony(parts.filter((x) => x).join(" · "));
    } catch (e) {
      print("[live] α7 read: " + e);
    }
  }

  private sony(s: string) {
    if (this.sonyText) {
      this.sonyText.text = s;
      this.sonyText.textFill.color = rgba(AMBER);
    }
  }

  // ---- frames over Wi-Fi --------------------------------------------------

  private toggle() {
    this.on = !this.on;
    this.liveChip.text = this.on ? "LIVE" : "PAUSED";
    if (this.on) {
      if (this.wsUrl) this.open(); else this.askUrl();
    } else {
      this.close();
      this.say("PAUSED");
    }
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
      if (typeof e.data === "string") return;
      this.lastFrameAt = Date.now();
      this.request();                                   // ask for the next while this one decodes
      if (this.decoding >= MAX_DECODING) { this.dropped++; return; }
      this.decoding++;
      this.media.loadResourceAsImageTexture(this.internet.makeResourceFromBlob(e.data as Blob), (t) => {
        this.decoding--;
        if (serial !== this.wsSerial) return;
        setQuadTexture(this.picture, t);
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

  private close() {
    this.wsSerial++;
    const ws = this.ws;
    this.ws = null;
    if (ws) try { ws.close(); } catch (e) { /* already closed */ }
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
    if (this.ws && this.on && !this.holding) {
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
