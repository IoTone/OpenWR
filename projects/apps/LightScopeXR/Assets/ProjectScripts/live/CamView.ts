// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// CamView.ts — one LAN camera live, in its own window: pick it from the
// cameras ReelHub has found.
//
// ReelHub keeps the list (it broadcasts `who` on the LAN; Scout firmware
// 0.8.0+ answers with its name and address). Over the hub's BLE console
// `cams?` prints `cam <name> <url> seen <s> ago` per camera; the window asks
// every 15 s and shows one chip per camera, so a new camera (Scout,
// CameraX, the next one) appears without code. Pinch a chip to watch it;
// the choice is kept.
//
// Frames come as LiveView's do: a WebSocket to ws://<ip>/ws, any message in,
// one 320x240 JPEG out, two in flight. A socket with no frame for 3 s is
// abandoned, not closed (close() on a stalled socket froze the lens for
// ~27 s on the glasses, 2026-11-04), and opened again.

import { Links } from "../ble/Links";
import { makeQuad, makeTappable, makeTexture, setQuadTexture, Z_BACK, Z_IMAGE } from "../ui/Bitmap";
import { Raster, Rgb } from "../ui/Raster";
import { placeInFront } from "../ui/Place";

require("LensStudio:InternetModule");

const AMBER: Rgb = [242, 163, 58];
const rgba = (c: Rgb) => new vec4(c[0] / 255, c[1] / 255, c[2] / 255, 1);
const ASK_EVERY_S = 15;
const STALL_MS = 3000;
const IN_FLIGHT = 2;
const STORE_KEY = "lightscope.camview";

interface Cam { name: string; url: string; chip: SceneObject; label: Text; }

@component
export class CamView extends BaseScriptComponent {

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

  @input distance: number = 125;
  @input drop: number = 6;
  @input
  @hint("Degrees to the side; negative is left")
  yaw: number = 64;            // right of the Roll (+32)

  @input
  @hint("Width of the picture, cm (at 45 cm)")
  width: number = 16;

  private internet = require("LensStudio:InternetModule") as InternetModule;
  private media = require("LensStudio:RemoteMediaModule") as RemoteMediaModule;
  private picture: SceneObject;
  private cams: Cam[] = [];
  private chipTex: { on: Texture; off: Texture } = null;
  private picked = "";
  private ws: WebSocket = null;
  private wsSerial = 0;
  private wsUrl = "";
  private lastFrameAt = 0;
  private decoding = 0;
  private shown = 0;
  private fps = 0;
  private fpsAt = 0;
  private note = "";

  onAwake() {
    try { this.picked = global.persistentStorageSystem.store.getString(STORE_KEY) || ""; } catch (e) { /* first run */ }
    this.createEvent("OnStartEvent").bind(() => {
      const root = this.getSceneObject(), h = this.width * 3 / 4;
      this.picture = makeQuad(root, "Picture", this.width, h, this.imageMaterial, makeTexture(1, 1, new Uint8Array([0, 0, 0, 255])));
      this.picture.getTransform().setLocalPosition(new vec3(0, 1.0, Z_IMAGE));
      if (this.titleText) this.titleText.getSceneObject().getTransform().setLocalPosition(new vec3(0, 1.0 + h / 2 + 0.9, 0.2));
      if (this.captionText) this.captionText.getSceneObject().getTransform().setLocalPosition(new vec3(0, 1.0 - h / 2 - 3.3, 0.2));
      this.chipTex = { on: pill(4.4, 1.3, AMBER, true), off: pill(4.4, 1.3, AMBER, false) };
      this.paint(this.titleText, "CAMERAS");
      this.links.after(() => {
        if (this.frame && this.camera) placeInFront(this.frame, this.camera, this.distance, this.drop, this.yaw);
      }, 0.3);
      this.ask();
      this.tick();
    });
  }

  // ---- the hub's list -----------------------------------------------------

  private ask() {
    const h = this.links.reelHub;
    if (h && h.live) {
      h.send("cams?").then((r) => this.list(r)).catch((e) => print("[cams] " + e));
    }
    this.links.after(() => this.ask(), ASK_EVERY_S);
  }

  private list(reply: string) {
    const found: { name: string; url: string }[] = [];
    for (const line of reply.split("\n")) {
      const m = /^cam (\S+) (http:\/\/\S+)/.exec(line.trim());
      if (m) found.push({ name: m[1], url: m[2].replace(/\/$/, "") });
    }
    let changed = false;
    for (const f of found) {
      const c = this.cams.find((x) => x.name === f.name);
      if (!c) { this.cams.push({ name: f.name, url: f.url, chip: null, label: null }); changed = true; }
      else if (c.url !== f.url) {
        c.url = f.url;
        if (c.name === this.picked) { print("[cams] " + c.name + " moved to " + c.url); this.stream(); }
      }
    }
    if (changed) { this.layout(); print("[cams] " + this.cams.map((c) => c.name + " " + c.url).join(", ")); }
    if (!this.ws && this.cams.length) {
      if (!this.cams.some((c) => c.name === this.picked)) this.picked = this.cams[0].name;
      this.stream();
    }
  }

  private layout() {
    const root = this.getSceneObject(), h = this.width * 3 / 4, y = 1.0 - h / 2 - 1.4, gap = 4.8;
    this.cams.forEach((c, i) => {
      const x = (i - (this.cams.length - 1) / 2) * gap;
      if (!c.chip) {
        c.chip = makeQuad(root, "Cam " + c.name, 4.4, 1.3, this.imageMaterial, this.chipTex.off, 9);
        makeTappable(c.chip, 4.4, 1.3, () => this.pick(c.name));
        c.label = this.text(root, c.name.toUpperCase(), 15);
      }
      c.chip.getTransform().setLocalPosition(new vec3(x, y, Z_BACK));
      c.label.getSceneObject().getTransform().setLocalPosition(new vec3(x, y, Z_IMAGE));
    });
    this.marks();
  }

  private marks() {
    for (const c of this.cams) {
      if (!c.chip) continue;
      const on = c.name === this.picked;
      setQuadTexture(c.chip, on ? this.chipTex.on : this.chipTex.off);
      c.label.textFill.color = on ? new vec4(0, 0, 0, 1) : rgba(AMBER);
    }
  }

  private pick(name: string) {
    if (name === this.picked && this.ws) return;
    this.picked = name;
    try { global.persistentStorageSystem.store.putString(STORE_KEY, name); } catch (e) { /* not kept */ }
    this.marks();
    this.stream();
  }

  // ---- frames -------------------------------------------------------------

  private stream() {
    this.abandon();
    const c = this.cams.find((x) => x.name === this.picked);
    if (!c) return;
    this.marks();
    this.paint(this.titleText, c.name.toUpperCase());
    this.wsUrl = c.url.replace(/^http:/, "ws:") + "/ws";
    this.shown = 0; this.fps = 0; this.fpsAt = Date.now();
    this.open();
  }

  private open() {
    if (this.ws || !this.wsUrl) return;
    const serial = ++this.wsSerial;
    let ws: WebSocket;
    try {
      ws = this.internet.createWebSocket(this.wsUrl);
    } catch (e) {
      print("[cams] createWebSocket: " + e);
      this.links.after(() => { if (serial === this.wsSerial) this.open(); }, 2);
      return;
    }
    this.ws = ws;
    ws.binaryType = "blob";
    this.lastFrameAt = Date.now();
    this.note = "CONNECTING";
    ws.onopen = () => {
      if (serial !== this.wsSerial) return;
      this.note = "";
      print("[cams] streaming " + this.picked + " from " + this.wsUrl);
      for (let i = 0; i < IN_FLIGHT; i++) this.request();
    };
    ws.onerror = () => this.lost(serial);
    ws.onclose = () => this.lost(serial);
    ws.onmessage = (e) => {
      if (serial !== this.wsSerial) return;
      if (typeof e.data === "string") {                     // the camera was busy: ask again
        this.links.after(() => { if (serial === this.wsSerial) this.request(); }, 0.2);
        return;
      }
      this.lastFrameAt = Date.now();
      this.request();
      if (this.decoding >= 2) return;
      this.decoding++;
      this.media.loadResourceAsImageTexture(this.internet.makeResourceFromBlob(e.data as Blob), (t) => {
        this.decoding--;
        if (serial !== this.wsSerial) return;
        setQuadTexture(this.picture, t);
        this.shown++;
      }, () => { this.decoding--; });
    };
  }

  private request() {
    try { if (this.ws) this.ws.send("f"); } catch (e) { /* closing; lost() reopens */ }
  }

  // abandon, never close() a socket that may be stalled (see the header)
  private abandon() {
    this.wsSerial++;
    const ws = this.ws;
    this.ws = null;
    if (!ws) return;
    ws.onmessage = () => {};
    ws.onerror = () => {};
    ws.onclose = () => {};
    if (Date.now() - this.lastFrameAt < 1000) { try { ws.close(); } catch (e) { /* gone */ } }
  }

  private lost(serial: number) {
    if (serial !== this.wsSerial) return;
    this.abandon();
    this.note = "RECONNECTING";
    this.links.after(() => this.open(), 2);
  }

  private tick() {
    const now = Date.now();
    if (this.ws && now - this.lastFrameAt > STALL_MS) {
      print("[cams] " + this.picked + ": no frame in " + STALL_MS + " ms");
      this.lost(this.wsSerial);
    }
    if (now - this.fpsAt >= 2000) { this.fps = this.shown * 1000 / (now - this.fpsAt); this.shown = 0; this.fpsAt = now; }
    const h = this.links.reelHub;
    let s: string;
    if (!this.cams.length) s = h && h.live ? "ASKING REELHUB FOR CAMERAS" : "WAITING FOR REELHUB";
    else {
      const c = this.cams.find((x) => x.name === this.picked);
      s = (c ? c.name + " · " + c.url.replace("http://", "") : "PICK A CAMERA") + " · " + (this.note || this.fps.toFixed(1) + " FPS");
    }
    this.paint(this.captionText, s);
    this.links.after(() => this.tick(), 1);
  }

  // ---- bits ---------------------------------------------------------------

  private text(parent: SceneObject, s: string, size: number): Text {
    const o = global.scene.createSceneObject("Label");
    o.setParent(parent);
    const t = o.createComponent("Component.Text") as Text;
    if (this.captionText) t.font = this.captionText.font;
    t.size = size;
    t.text = s;
    t.horizontalAlignment = HorizontalAlignment.Center;      // centred: see lens-text-alignment
    t.verticalAlignment = VerticalAlignment.Center;
    t.textFill.color = rgba(AMBER);
    t.setRenderOrder(12);
    return t;
  }

  private paint(t: Text, s: string) {
    if (!t) return;
    t.text = s.toUpperCase();
    t.textFill.color = rgba(AMBER);
  }
}

// a rounded pill, filled or outlined
function pill(w: number, h: number, c: Rgb, filled: boolean): Texture {
  const r = new Raster(Math.round(w * 40), Math.round(h * 40)), rad = r.h / 2 - 1;
  for (let y = 0; y < r.h; y++) for (let x = 0; x < r.w; x++) {
    const cx = Math.min(Math.max(x, rad + 1), r.w - rad - 1), d = Math.hypot(x - cx, y - r.h / 2);
    if (filled ? d <= rad : Math.abs(d - rad + 1) <= 1.1) r.plot(x, y, c);
  }
  return r.texture();
}

