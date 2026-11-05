// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Links.ts — every Bluetooth link the lens holds, in one place.
//
// LightScopeXR's cameras: AlphaFriend (the StickC that drives the α7R III
// over its Wi-Fi) and Scout (the ESP32 camera; its shutter and roll over
// BLE, its address on the LAN for Live). Each speaks the same ok> console
// (GoggleLink). The older devices are still here, off unless named: goggl1,
// the maplet (Roll and Snap's SEND), the heart-rate strap (Pulse).
// Views take this component as an input and share the links: one scanner
// (a missing device costs every other one a 20 s scan per retry, so only
// name what is in use), one console queue per device. Each link is held for
// the lens's life and retried after a drop.

import { BleHub } from "./BleHub";
import { GoggleLink } from "./GoggleLink";
import { HrStrap } from "./HrStrap";

@component
export class Links extends BaseScriptComponent {

  @input
  @hint("Bluetooth Central Module asset")
  bluetoothModule: Bluetooth.BluetoothCentralModule;

  @input
  @hint("AlphaFriend's advertised name; empty to leave it alone")
  alphaName: string = "AlphaFriend";

  @input
  @hint("Scout's advertised name; empty to leave it alone")
  scoutName: string = "Scout";

  @input
  @hint("goggl1's advertised name; empty to leave it alone")
  goggleName: string = "";

  @input
  @hint("The maplet's advertised name; empty to leave it alone")
  mapletName: string = "";

  @input
  @hint("Read the heart-rate strap")
  useStrap: boolean = false;

  @input
  @hint("Open no links (the editor preview has no Bluetooth)")
  offline: boolean = false;

  hub: BleHub;
  strap: HrStrap;
  goggle: GoggleLink;
  maplet: GoggleLink | null = null;
  scout: GoggleLink | null = null;
  alpha: GoggleLink | null = null;

  // What goggl1 should show: Pulse while the strap streams, or a weather
  // theme picked on the Remote. Pulse only sends sess-hr! while this is "pulse".
  goggleView: "pulse" | "weather" = "pulse";

  private listeners: (() => void)[] = [];

  onAwake() {
    this.hub = new BleHub(this.bluetoothModule, (ms) => new Promise<void>((r) => this.after(r, ms / 1000)));
    const changed = () => this.listeners.forEach((f) => f());
    const retry = (fn: () => void, s: number) => this.after(fn, s);
    const timer = (fn: () => void, s: number) => this.cancellable(fn, s);
    this.strap = new HrStrap(this.hub, changed, retry);
    this.goggle = new GoggleLink(this.goggleName, this.hub, changed, retry, timer);   // Pulse and Remote expect one
    if (this.alphaName) this.alpha = new GoggleLink(this.alphaName, this.hub, changed, retry, timer);
    if (this.mapletName) this.maplet = new GoggleLink(this.mapletName, this.hub, changed, retry, timer);
    if (this.scoutName) this.scout = new GoggleLink(this.scoutName, this.hub, changed, retry, timer);

    this.onChange(() => this.sendClock());
    this.createEvent("OnStartEvent").bind(() => {
      if (this.offline) return;
      if (this.alpha) this.alpha.start();
      if (this.scout) this.scout.start();
      if (this.useStrap) this.strap.start();
      if (this.goggleName) this.goggle.start();
      if (this.maplet) this.maplet.start();
    });
    this.createEvent("OnDestroyEvent").bind(() => {
      this.strap.stop();
      this.goggle.stop();
      if (this.maplet) this.maplet.stop();
      if (this.scout) this.scout.stop();
      if (this.alpha) this.alpha.stop();
    });
  }

  // goggl1 has no clock without WiFi: give it the glasses' time on each
  // connect. clock! arrived in firmware 0.6.1; older goggles are left alone.
  private clockSent = false;

  private sendClock() {
    if (!this.goggle.live) { this.clockSent = false; return; }
    if (this.clockSent) return;
    this.clockSent = true;
    this.goggle.send("version").then((r) => {
      const v = /v(\d+)\.(\d+)\.(\d+)/.exec(r);
      const n = v ? +v[1] * 10000 + +v[2] * 100 + +v[3] : 0;
      if (n < 601) { print("[goggl1] " + (v ? v[0] : "no version") + ": no clock!, the clock stays --:--"); return; }
      const now = new Date(), unix = Math.floor(now.getTime() / 1000), off = -now.getTimezoneOffset() * 60;
      return this.goggle.send(unix + " " + off + " clock! hud-now").then(() => print("[goggl1] clock set " + now.toTimeString().slice(0, 5)));
    }).catch((e) => { this.clockSent = false; print("[goggl1] clock: " + e); });
  }

  // Where the roll lives: the maplet's card when it is linked, else Scout's flash.
  get rollLink(): GoggleLink | null {
    if (this.maplet && this.maplet.live) return this.maplet;
    return this.scout || this.maplet;
  }

  // Called whenever any link changes state.
  onChange(f: () => void) {
    this.listeners.push(f);
  }

  after(fn: () => void, seconds: number) {
    const ev = this.createEvent("DelayedCallbackEvent");
    ev.bind(() => { this.removeEvent(ev); fn(); });
    ev.reset(seconds);
  }

  private cancellable(fn: () => void, seconds: number): { cancel: () => void } {
    const ev = this.createEvent("DelayedCallbackEvent");
    let live = true;
    ev.bind(() => { this.removeEvent(ev); if (live) fn(); });
    ev.reset(seconds);
    return { cancel: () => { live = false; } };
  }
}
