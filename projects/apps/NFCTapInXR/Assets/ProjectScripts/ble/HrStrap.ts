// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// HrStrap.ts — the heart-rate strap, read by the glasses themselves.
//
// The standard Heart Rate service (0x180D / 0x2A37), unencrypted, so no
// bonding; the same parsing as Switchback's lib/goggle/hr_strap.dart. The
// Rhythm 24 advertises no service UUIDs, so straps are matched by name. It
// also serves Running Speed and Cadence (0x1814 / 0x2A53), which gives steps
// a minute on foot. The link is held for the whole lens and retried after a
// drop; it is never torn down mid-lens.

import { BleHub, findChar } from "./BleHub";

const HR_SERVICE = "180d";
const HR_MEASUREMENT = "2a37";
const BATTERY_SERVICE = "180f";
const BATTERY_LEVEL = "2a19";
const RSC_SERVICE = "1814";
const RSC_MEASUREMENT = "2a53";

const STALE_MS = 5000;

export class HrSample {
  constructor(public bpm: number, public contact: boolean | null, public rrMs: number[]) {}
}

// Heart Rate Measurement: flags, 8- or 16-bit bpm, optional energy, RR in 1/1024 s.
export function parseHeartRate(b: Uint8Array): HrSample | null {
  if (b.length < 2) return null;
  const f = b[0];
  let i: number;
  let bpm: number;
  if (f & 0x01) {
    if (b.length < 3) return null;
    bpm = b[1] | (b[2] << 8);
    i = 3;
  } else {
    bpm = b[1];
    i = 2;
  }
  const c = (f >> 1) & 3;
  const contact = c === 2 ? false : c === 3 ? true : null;
  if (f & 0x08) i += 2;
  const rr: number[] = [];
  if (f & 0x10) {
    for (; i + 1 < b.length; i += 2) rr.push(Math.round(((b[i] | (b[i + 1] << 8)) * 1000) / 1024));
  }
  return new HrSample(bpm, contact, rr);
}

// Running Speed and Cadence: speed in 1/256 m/s, cadence per minute.
export function parseRscCadence(b: Uint8Array): number {
  return b.length >= 4 ? b[3] : -1;
}

// Zone 1..5 by percent of max (60/70/80/90), 0 for no reading. As the phone app.
export function hrZone(bpm: number, maxHr: number): number {
  if (bpm <= 0 || maxHr <= 0) return 0;
  const p = bpm / maxHr;
  return p >= 0.9 ? 5 : p >= 0.8 ? 4 : p >= 0.7 ? 3 : p >= 0.6 ? 2 : 1;
}

export function looksLikeHrStrap(name: string): boolean {
  const n = (name || "").toLowerCase();
  return n.startsWith("rhythm") || n.startsWith("r+") || n.startsWith("polar") || n.startsWith("hrm")
    || n.startsWith("tickr") || n.indexOf("heart") >= 0;
}

export type StrapState = "searching" | "connecting" | "live" | "lost";

export class HrStrap {
  state: StrapState = "searching";
  name = "";
  battery = -1;
  contact: boolean | null = null;

  private lastBpm = 0;
  private bpmAt = 0;
  private stepCad = 0;
  private stepAt = 0;
  private hasRsc = false;
  private gatt: Bluetooth.BluetoothGatt = null;
  private stopped = false;

  constructor(private hub: BleHub, private onChange: () => void, private retry: (fn: () => void, s: number) => void) {}

  // Current bpm, 0 when stale or the strap reports no skin contact.
  get bpm(): number {
    if (Date.now() - this.bpmAt > STALE_MS || this.contact === false) return 0;
    return this.lastBpm;
  }

  // Steps a minute, 0 when stale; -1 when the strap has no running cadence.
  get cadence(): number {
    if (!this.hasRsc) return -1;
    return Date.now() - this.stepAt > STALE_MS ? 0 : this.stepCad;
  }

  start() {
    this.stopped = false;
    this.attempt();
  }

  stop() {
    this.stopped = true;
  }

  private set(s: StrapState) {
    this.state = s;
    this.onChange();
  }

  private async attempt() {
    if (this.stopped) return;
    this.set(this.gatt ? "lost" : "searching");
    const r = await this.hub.find((x) => looksLikeHrStrap(x.deviceName), 20);
    if (!r) {
      this.retry(() => this.attempt(), 5);
      return;
    }
    try {
      this.set("connecting");
      this.name = r.deviceName;
      const gatt = await this.hub.connect(r);
      this.gatt = gatt;
      gatt.onConnectionStateChangedEvent.add((e) => {
        if (e.state === Bluetooth.ConnectionState.Disconnected && !this.stopped) {
          print("[strap] link dropped, retrying");
          this.retry(() => this.attempt(), 3);
        }
      });
      const hrm = findChar(gatt, HR_SERVICE, HR_MEASUREMENT);
      if (!hrm) throw new Error("no Heart Rate service on " + r.deviceName);
      const batt = findChar(gatt, BATTERY_SERVICE, BATTERY_LEVEL);
      if (batt) {
        try {
          const v = await batt.readValue();
          if (v && v.length) this.battery = v[0];
        } catch (e) { /* battery is optional */ }
      }
      await hrm.registerNotifications((v: Uint8Array) => this.onHr(v));
      const rsc = findChar(gatt, RSC_SERVICE, RSC_MEASUREMENT);
      if (rsc) {
        try {
          await rsc.registerNotifications((v: Uint8Array) => this.onRsc(v));
          this.hasRsc = true;
        } catch (e) { /* cadence is optional */ }
      }
      print("[strap] live: " + r.deviceName + " battery " + this.battery);
      this.set("live");
    } catch (e) {
      print("[strap] " + e);
      this.retry(() => this.attempt(), 5);
    }
  }

  private onHr(v: Uint8Array) {
    const s = parseHeartRate(v);
    if (!s) return;
    this.contact = s.contact;
    this.lastBpm = s.bpm;
    this.bpmAt = Date.now();
  }

  private onRsc(v: Uint8Array) {
    const c = parseRscCadence(v);
    if (c < 0) return;
    this.stepCad = c;
    this.stepAt = Date.now();
  }
}
