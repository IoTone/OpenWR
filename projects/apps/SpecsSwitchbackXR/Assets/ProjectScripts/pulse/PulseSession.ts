// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// PulseSession.ts — G1: heart rate first, on the glasses and on goggl1.
//
// The glasses read the strap themselves (HrStrap) and hold goggl1's console
// (GoggleLink). Once a second they show bpm, zone, elapsed time and cadence
// here, and send the same to goggl1:
//
//     secs bpm zone cad kind sess-hr!        (weather-hud 0.8, 16-pulse.fs)
//
// goggl1 switches to its Pulse screen on the first line and back by itself
// 30 s after the lines stop, so closing the lens never strands it.
//
// Glass palette (black is clear glass): Fog for the number, Moss for labels,
// Alpenglow for the zone. See docs/specs-switchbackxr.html, section 3.
//
// Setup (scene "PulseFrame", built over the Lens Studio MCP): a SIK
// ContainerFrame placed in the world, which the wearer can grab and move;
// inside it a ZoneRing gauge, the bpm in its middle, the zone label in the
// gauge's opening, a row of time and cadence, and a status line. Layout in
// cm matches tools/pulse_preview.py, which draws it off the glasses.
//
// demoMode fakes the strap (a sweep through all five zones) so the view can
// be checked in the Lens Studio preview, where there is no Bluetooth.

import { GoggleLink } from "../ble/GoggleLink";
import { HrStrap, hrZone } from "../ble/HrStrap";
import { Links } from "../ble/Links";
import { placeInFront } from "../ui/Place";
import { ZoneRing } from "./ZoneRing";

const FOG = new vec4(0.953, 0.957, 0.945, 1);
const MOSS = new vec4(0.498, 0.784, 0.639, 1);
const GLOW = new vec4(0.941, 0.541, 0.384, 1);
const DIM = new vec4(0.35, 0.35, 0.35, 1);

@component
export class PulseSession extends BaseScriptComponent {

  @input
  @hint("The lens's Bluetooth links (strap, goggl1, maplet)")
  links: Links;

  @input
  @hint("Your max heart rate; zones are 60/70/80/90 % of it, as on the phone")
  maxHr: number = 185;

  @input bpmText: Text;
  @input labelText: Text;
  @input rowText: Text;
  @input statusText: Text;

  @input
  @allowUndefined
  @hint("The zone gauge")
  ring: ZoneRing;

  @input
  @hint("Fake the strap (sweeps 60-180 bpm) for the editor preview; no Bluetooth")
  demoMode: boolean = false;

  @input
  @allowUndefined
  @hint("The ContainerFrame to place in front of the wearer at start")
  frame: SceneObject;

  @input
  @allowUndefined
  @hint("The camera (Camera Object)")
  camera: SceneObject;

  @input
  @hint("How far in front of the eyes the panel starts, cm. It scales with distance, so it looks the same size; nearer than ~60 cm is hard to focus on")
  distance: number = 85;

  @input
  @hint("How far below eye level the panel starts, cm (at 45 cm; scales with distance)")
  drop: number = 6;

  private get strap(): HrStrap { return this.links.strap; }
  private get goggle(): GoggleLink { return this.links.goggle; }
  private startedAt = 0;
  private shownZone = -1;
  private sent = 0;
  private sendErrors = 0;
  private ticks = 0;

  onAwake() {
    // other components (the ring) are not awake yet: draw from OnStart
    this.createEvent("OnStartEvent").bind(() => {
      this.paint(this.bpmText, FOG, "--");
      this.paint(this.labelText, MOSS, "BPM");
      this.paint(this.rowText, FOG, "");
      this.drawRing(0);
      this.links.onChange(() => this.drawStatus());
      this.drawStatus();
      this.after(() => this.place(), 0.3);  // once head tracking has a pose
      this.after(() => this.logSizes(), 2);   // text meshes exist after a frame or two
      this.tick();
    });
    this.createEvent("OnDestroyEvent").bind(() => {
      if (this.goggle.idle) this.goggle.send("pulse-off").catch(() => {});
    });
  }

  // once a second: the view, then goggl1
  private tick() {
    this.ticks++;
    const demo = this.demoMode ? this.fake() : null;
    const bpm = demo ? demo.bpm : this.strap.bpm;
    if (bpm > 0 && this.startedAt === 0) this.startedAt = Date.now();
    const secs = this.startedAt ? Math.floor((Date.now() - this.startedAt) / 1000) : 0;
    const zone = hrZone(bpm, this.maxHr);
    const cad = demo ? demo.cad : this.strap.cadence;

    this.paint(this.bpmText, bpm > 0 ? FOG : DIM, bpm > 0 ? String(bpm) : "--");
    const why = !demo && this.strap.state !== "live" ? "STRAP " + this.strap.state.toUpperCase()
      : !demo && bpm === 0 && this.strap.contact === false ? "NO CONTACT" : zone > 0 ? "BPM · Z" + zone : "BPM";
    this.paint(this.labelText, MOSS, why);
    let row = this.clock(secs);
    if (cad > 0) row += "   " + cad + " spm";
    row += "   max " + this.maxHr;
    this.paint(this.rowText, FOG, row);
    if (zone !== this.shownZone) this.drawRing(zone);

    if (demo && this.ticks % 5 === 0) print("[pulse] demo " + bpm + " bpm zone " + zone + " " + row);
    // only once there is a strap: an empty line would switch goggl1 to Pulse just to show --
    // and not while the Remote has goggl1 on a weather theme
    if (this.goggle.idle && (demo || this.strap.state === "live") && this.links.goggleView === "pulse") {
      // every 10th line also asks whether the goggle is showing Pulse
      const ask = this.sent % 10 === 0;
      const line = secs + " " + bpm + " " + zone + " " + Math.max(0, cad) + " " + (cad >= 0 ? 1 : 0) + " sess-hr!"
        + (ask ? " pulse-on? @ . depth ." : "");
      this.goggle.send(line).then((reply) => {
        this.sent++;
        if (ask || reply.indexOf("?") >= 0) print("[pulse] goggl1 <- " + line + "  -> '" + reply.replace(/\s+/g, " ").trim() + "'");
      }, (e) => {
        this.sendErrors++;
        print("[pulse] " + e);
      });
    }
    this.drawStatus();
    this.after(() => this.tick(), 1);
  }

  private clock(s: number): string {
    const two = (n: number) => (n < 10 ? "0" : "") + n;
    const h = Math.floor(s / 3600), m = Math.floor((s % 3600) / 60), r = s % 60;
    return h > 0 ? h + ":" + two(m) + ":" + two(r) : two(m) + ":" + two(r);
  }

  private drawStatus() {
    if (!this.statusText) return;
    const s = this.strap, g = this.goggle;
    if (this.demoMode) {
      this.paint(this.statusText, MOSS, "DEMO · no Bluetooth");
      return;
    }
    // Plex Mono has no filled circle; the bullet and a dash read as on/off
    const strap = s.state === "live" ? "• " + s.name + (s.battery >= 0 ? " · " + s.battery + "%" : "") : "- strap " + s.state;
    const gog = g.state === "live" ? "• " + g.deviceName + " · " + g.lastRttMs + " ms" : "- " + g.deviceName + " " + g.state;
    this.paint(this.statusText, MOSS, strap + "     " + gog);
  }

  private drawRing(zone: number) {
    this.shownZone = zone;
    if (this.ring) this.ring.setZone(zone);
  }

  // 60 -> 180 bpm and back over two minutes, cadence on when above 100
  private fake(): { bpm: number; cad: number } {
    const p = (this.ticks % 120) / 120;
    const bpm = Math.round(120 - 60 * Math.cos(p * 2 * Math.PI));
    return { bpm, cad: bpm > 100 ? 150 + Math.round((bpm - 100) / 3) : 0 };
  }

  // Text size is in points; the layout is in cm. Log what each line measures
  // so the sizes can be set to the cap heights in tools/pulse_preview.py.
  private place() {
    if (!this.frame || !this.camera) return;
    placeInFront(this.frame, this.camera, this.distance, this.drop);
    print("[pulse] frame placed " + this.distance + " cm ahead");
  }

  private logSizes() {
    let o = this.getSceneObject();
    while (o) {
      const tr = o.getTransform();
      print("[pulse] scale " + o.name + " local " + tr.getLocalScale().toString() + " world " + tr.getWorldScale().toString()
        + " pos " + tr.getWorldPosition().toString());
      o = o.getParent();
    }
    for (const [n, t] of [["bpm", this.bpmText], ["label", this.labelText], ["row", this.rowText], ["status", this.statusText]] as [string, Text][]) {
      if (!t) continue;
      try {
        const lo = t.localAabbMin(), hi = t.localAabbMax();
        print("[pulse] size " + n + " pt " + t.size + " -> " + (hi.x - lo.x).toFixed(2) + " x " + (hi.y - lo.y).toFixed(2) + " cm for '" + t.text + "'");
      } catch (e) {
        print("[pulse] size " + n + ": " + e);
      }
    }
  }

  private paint(t: Text, c: vec4, s: string) {
    if (!t) return;
    t.text = s;
    t.textFill.color = c;
  }

  private after(fn: () => void, seconds: number) {
    const ev = this.createEvent("DelayedCallbackEvent");
    ev.bind(() => { this.removeEvent(ev); fn(); });
    ev.reset(seconds);
  }
}
