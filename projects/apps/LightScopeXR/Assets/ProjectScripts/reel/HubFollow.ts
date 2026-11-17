// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// HubFollow.ts — the glasses as one of ReelHub's cameras, and its reel on
// the RealReel panel.
//
// ReelHub (the Waveshare C6 touch screen, reelhub_wavec6touch_espidforth)
// fires every camera at once and keeps the colour reel. Over its BLE
// console it prints, unasked:
//
//   fire <n>                          it shot: the glasses take theirs, a
//                                     colour frame from their camera, JPEG,
//                                     posted to the hub's reel as SPECS
//   show <file> <camera> <i>/<count>  the shot on its screen changed (a swipe,
//                                     a new shot): the RealReel shows it too,
//                                     fetched over Wi-Fi from
//                                     http://<hub>/reel/<file>.jpg
//
// BLE carries the events, Wi-Fi the pictures (the hub's address from url?).
// Without the hub nothing here runs, and the reel works as before.

import { Links } from "../ble/Links";
import { RealReel } from "./RealReel";
import { glassesCamera } from "../snap/Camera";
import { playOnce, uiFail } from "../ui/Sound";

require("LensStudio:InternetModule");

const EVENTS = /^(fire|show) /;

@component
export class HubFollow extends BaseScriptComponent {

  @input links: Links;
  @input reel: RealReel;

  @input
  @allowUndefined
  @hint("Shutter sound when the hub fires the glasses")
  shutter: AudioTrackAsset;

  @input
  @hint("Take the glasses' own shot when the hub fires")
  shootGlasses: boolean = true;

  private internet = require("LensStudio:InternetModule") as InternetModule;
  private media = require("LensStudio:RemoteMediaModule") as RemoteMediaModule;
  private base = "";               // http://<hub ip>/
  private wasLive = false;
  private loading = "";
  private want = "";               // the newest show while one loads
  private cam: Texture = null;
  private firing = false;

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => {
      const h = this.links.reelHub;
      if (!h) return;
      h.listen(EVENTS, (line) => this.heard(line));
      this.links.onChange(() => this.changed());
    });
  }

  private changed() {
    const h = this.links.reelHub;
    const live = !!h && h.live;
    if (live === this.wasLive) return;
    this.wasLive = live;
    if (!live) { print("[hub] link down"); return; }
    print("[hub] live");
    if (this.shootGlasses && !this.cam) {
      try { this.cam = glassesCamera(); } catch (e) { print("[hub] camera: " + e); }
    }
    h.send("url?").then((r) => {
      const m = /url (http:\/\/\S+)/.exec(r);
      this.base = m ? (m[1].endsWith("/") ? m[1] : m[1] + "/") : "";
      print("[hub] at " + (this.base || "no wifi (" + r.trim() + ")"));
      return h.send("show?");                       // its answer arrives as a show event
    }).catch((e) => print("[hub] " + e));
  }

  private heard(line: string) {
    const f = /^fire (\d+)/.exec(line);
    if (f) { this.fire(+f[1]); return; }
    const s = /^show (\d+) (\S+) (\d+)\/(\d+)/.exec(line);
    if (s) this.show(s[1], s[2]);
  }

  // ---- the hub's reel on the RealReel ------------------------------------

  private show(file: string, label: string) {
    const key = "hub:" + file;
    if (this.reel.has(key)) { this.reel.showFromHub(key, label, null, 0, 0, "", 0); return; }
    if (!this.base) return;
    if (this.loading) { this.want = file + " " + label; return; }
    this.loading = file;
    const t0 = Date.now(), url = this.base + "reel/" + file + ".jpg";
    this.media.loadResourceAsImageTexture(this.internet.makeResourceFromUrl(url), (tex) => {
      const s = (Date.now() - t0) / 1000;
      this.reel.showFromHub(key, label, tex, tex.getWidth(), tex.getHeight(), "HUB " + file, s);
      print("[hub] show " + file + " " + label + " " + tex.getWidth() + "x" + tex.getHeight() + " in " + s.toFixed(1) + " s");
      this.next();
    }, (err) => {
      print("[hub] " + url + ": " + err);
      this.next();
    });
  }

  private next() {
    this.loading = "";
    if (!this.want) return;
    const [file, label] = this.want.split(" ");
    this.want = "";
    this.show(file, label);
  }

  // ---- the glasses' own shot ---------------------------------------------

  private fire(n: number) {
    if (!this.shootGlasses || this.firing) return;
    if (!this.cam) { print("[hub] fire " + n + ": no camera"); return; }
    this.firing = true;
    playOnce(this.getSceneObject(), this.shutter);
    const t0 = Date.now();
    const provider = this.cam.control as CameraTextureProvider;
    let skip = 1;
    let reg: EventRegistration = null;
    reg = provider.onNewFrame.add(() => {
      if (skip-- > 0) return;
      provider.onNewFrame.remove(reg);
      Base64.encodeTextureAsync(this.cam, (b64) => this.post(n, b64, t0), () => {
        print("[hub] fire " + n + ": encode failed");
        uiFail();
        this.firing = false;
      }, CompressionQuality.IntermediateQuality, EncodingType.Jpg);
    });
  }

  private post(n: number, b64: string, t0: number) {
    if (!this.base) { print("[hub] fire " + n + ": hub has no address"); this.firing = false; return; }
    const req = new Request(this.base + "reel/add?label=SPECS", { method: "POST", body: b64, headers: { "Content-Type": "text/plain" } });
    this.internet.fetch(req).then((r) => r.text()).then((t) => {
      print("[hub] fire " + n + ": glasses " + Math.round(b64.length * 3 / 4 / 1024) + " KB, " + t.trim() + " in " + (Date.now() - t0) + " ms");
    }).catch((e) => {
      print("[hub] fire " + n + ": upload " + e);
      uiFail();
    }).then(() => { this.firing = false; });
  }
}
