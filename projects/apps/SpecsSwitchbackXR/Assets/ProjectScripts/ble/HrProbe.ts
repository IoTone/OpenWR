// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// HrProbe.ts — can Spectacles read the heart-rate strap directly, and hold
// it alongside a maplet/goggl1 link?
//
// The Rhythm 24 serves the standard Heart Rate service (0x180D / 0x2A37),
// unencrypted, but advertises no service UUIDs, so we match by name the way
// Switchback's looksLikeHrStrap() does. The strap must be worn and woken
// (button) to advertise, and it hides while another central holds it, so
// stop Switchback's strap link first.
//
// Run after NusProbe (startDelay) so both links are up at once: if NusProbe
// never logs "link state 0" while this streams, two links work.
//
// Setup in Lens Studio: add to a SceneObject, drag in the Bluetooth Central
// Module, optionally a Text. Every line is printed with a [hr] prefix.

const HR_SERVICE = "180d";
const HR_MEASUREMENT = "2a37";
const BATTERY_SERVICE = "180f";
const BATTERY_LEVEL = "2a19";

// "0x2A37", "2a37" and "00002a37-0000-1000-8000-00805f9b34fb" all name 2a37.
function short16(uuid: string): string {
  const u = uuid.toLowerCase().replace(/^0x/, "");
  return u.length === 36 && u.endsWith("-0000-1000-8000-00805f9b34fb") ? u.substring(4, 8) : u;
}

export class HrSample {
  constructor(public bpm: number, public contact: boolean | null, public rrMs: number[]) {}
}

// Heart Rate Measurement (0x2A37): flags, 8- or 16-bit bpm, optional energy, RR in 1/1024 s.
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

export function looksLikeHrStrap(name: string): boolean {
  const n = (name || "").toLowerCase();
  return n.startsWith("rhythm") || n.startsWith("r+") || n.startsWith("polar") || n.startsWith("hrm")
    || n.startsWith("tickr") || n.indexOf("heart") >= 0;
}

@component
export class HrProbe extends BaseScriptComponent {

  @input
  @hint("Bluetooth Central Module asset")
  bluetoothModule: Bluetooth.BluetoothCentralModule;

  @input
  @hint("Seconds to wait before scanning (let NusProbe finish first)")
  startDelay: number = 9;

  @input
  @hint("Seconds to stream heart rate")
  seconds: number = 60;

  @input
  @allowUndefined
  @hint("Optional Text to show results in the glasses")
  output: Text;

  private lines: string[] = [];
  private count = 0;
  private lo = 999;
  private hi = 0;
  private last: HrSample = null;

  onAwake() {
    const ev = this.createEvent("DelayedCallbackEvent");
    ev.bind(() => { this.run(); });
    ev.reset(this.startDelay);
  }

  private log(s: string) {
    print("[hr] " + s);
    this.lines.push(s);
    if (this.lines.length > 10) this.lines.shift();
    if (this.output) this.output.text = this.lines.join("\n");
  }

  private async run() {
    const t0 = Date.now();
    try {
      this.log("scanning for a strap");
      const settings = new Bluetooth.ScanSettings();
      settings.timeoutSeconds = 30;
      settings.scanMode = Bluetooth.ScanMode.LowLatency;
      const found = await this.bluetoothModule.startScan([new Bluetooth.ScanFilter()], settings,
        (r: Bluetooth.ScanResult) => looksLikeHrStrap(r.deviceName));
      if (!found) throw new Error("scan found nothing (strap worn and awake? Switchback let go?)");
      this.log("found " + found.deviceName + " rssi " + found.rssi + " (" + (Date.now() - t0) + " ms)");

      const t1 = Date.now();
      const gatt = await this.bluetoothModule.connectGatt(found.deviceAddress);
      this.log("connected in " + (Date.now() - t1) + " ms");
      gatt.onConnectionStateChangedEvent.add((e) => this.log("strap link state " + e.state));

      let hrm: Bluetooth.BluetoothGattCharacteristic = null;
      let batt: Bluetooth.BluetoothGattCharacteristic = null;
      for (const s of gatt.getServices()) {
        for (const c of s.getCharacteristics()) {
          if (short16(s.uuid) === HR_SERVICE && short16(c.uuid) === HR_MEASUREMENT) hrm = c;
          if (short16(s.uuid) === BATTERY_SERVICE && short16(c.uuid) === BATTERY_LEVEL) batt = c;
        }
      }
      if (!hrm) throw new Error("no Heart Rate service");
      if (batt) {
        try {
          const v = await batt.readValue();
          if (v && v.length) this.log("battery " + v[0] + "%");
        } catch (e) {
          this.log("battery read failed: " + e);
        }
      }

      const t2 = Date.now();
      await hrm.registerNotifications((v: Uint8Array) => this.onHr(v));
      this.log("subscribed without bonding in " + (Date.now() - t2) + " ms");

      const tick = this.createEvent("DelayedCallbackEvent");
      let left = this.seconds;
      tick.bind(() => {
        left -= 5;
        const s = this.last;
        this.log((s ? s.bpm + " bpm" + (s.contact === false ? " (no contact)" : "") : "no reading")
          + "  n=" + this.count + "  " + this.lo + "-" + this.hi);
        if (left === 0) this.finish(gatt, hrm);
        tick.reset(5);
      });
      tick.reset(5);
    } catch (e) {
      this.log("FAIL: " + e);
    }
  }

  private onHr(v: Uint8Array) {
    const s = parseHeartRate(v);
    if (!s) return;
    this.last = s;
    this.count++;
    if (s.bpm > 0) {
      this.lo = Math.min(this.lo, s.bpm);
      this.hi = Math.max(this.hi, s.bpm);
    }
  }

  // Report and keep streaming: a session holds the strap until the lens
  // closes, and an early teardown is the path SkywriterBLE saw freeze.
  private async finish(gatt: Bluetooth.BluetoothGatt, hrm: Bluetooth.BluetoothGattCharacteristic) {
    const rate = (this.count / this.seconds).toFixed(1);
    if (this.count > 0) this.log("PASS: " + this.count + " readings, " + rate + "/s, " + this.lo + "-" + this.hi + " bpm");
    else this.log("FAIL: subscribed but no readings in " + this.seconds + " s");
  }
}
