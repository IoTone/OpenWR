// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// GoggleLink.ts — the ok> console of a goggl1 or maplet, from the glasses.
//
// Nordic UART: write lines to RX, read replies from TX notifications until
// the "ok>" prompt, one line in flight at a time (there is one Forth
// interpreter on the device); lines from several views queue behind each
// other. Writes go in 20-byte pieces because the lens
// cannot raise the MTU, and with response because on the glasses that was
// the faster mode (median 100 ms against 149 ms). No bonding: the console
// is an open service. The link is held for the whole lens and retried.

import { BleHub, findChar } from "./BleHub";

const NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";
const REPLY_MS = 3000;

export type LinkState = "searching" | "connecting" | "live" | "lost";

export class GoggleLink {
  state: LinkState = "searching";
  lastRttMs = 0;

  private rx: Bluetooth.BluetoothGattCharacteristic = null;
  private buf = "";
  private waiter: ((reply: string) => void) | null = null;
  private busy = false;
  private queue: Promise<unknown> = Promise.resolve();
  private queued = 0;
  private seen = false;
  private stopped = false;

  constructor(
    public readonly deviceName: string,
    private hub: BleHub,
    private onChange: () => void,
    private retry: (fn: () => void, s: number) => void,
    private timer: (fn: () => void, s: number) => { cancel: () => void },
  ) {}

  get live(): boolean { return this.state === "live"; }
  // nothing in flight or waiting: a periodic sender (Pulse) only sends then
  get idle(): boolean { return this.live && !this.busy && this.queued === 0; }

  start() {
    this.stopped = false;
    this.attempt();
  }

  stop() {
    this.stopped = true;
  }

  // Send one console line and resolve with what the device printed before
  // the prompt. Lines queue: each waits for the one before to answer.
  // Rejects if the link is down or the device does not answer in time
  // (default 3 s; roll-get of a 4.7 KB frame needs about 30).
  send(line: string, timeoutMs = REPLY_MS): Promise<string> {
    if (!this.live) return Promise.reject(new Error(this.deviceName + " not connected"));
    this.queued++;
    const run = () => {
      this.queued--;
      return this.sendNow(line, timeoutMs);
    };
    const p = this.queue.then(run, run);
    this.queue = p.catch(() => null);
    return p;
  }

  private sendNow(line: string, timeoutMs: number): Promise<string> {
    if (!this.live) return Promise.reject(new Error(this.deviceName + " not connected"));
    this.busy = true;
    const t0 = Date.now();
    return new Promise<string>((resolve, reject) => {
      this.buf = "";
      const t = this.timer(() => {
        this.waiter = null;
        this.busy = false;
        reject(new Error("no reply to '" + line + "'"));
      }, timeoutMs / 1000);
      this.waiter = (reply) => {
        t.cancel();
        this.busy = false;
        this.lastRttMs = Date.now() - t0;
        // the console echoes the line first
        resolve(reply.startsWith(line) ? reply.substring(line.length).trim() : reply.trim());
      };
      this.write(line + "\r").catch((e) => {
        t.cancel();
        this.waiter = null;
        this.busy = false;
        reject(e);
      });
    });
  }

  private set(s: LinkState) {
    this.state = s;
    this.onChange();
  }

  private async attempt() {
    if (this.stopped) return;
    this.set(this.seen ? "lost" : "searching");
    const r = await this.hub.find((x) => x.deviceName === this.deviceName, 20);
    if (!r) {
      this.retry(() => this.attempt(), 5);
      return;
    }
    try {
      this.set("connecting");
      const gatt = await this.hub.connect(r);
      gatt.onConnectionStateChangedEvent.add((e) => {
        if (e.state === Bluetooth.ConnectionState.Disconnected && !this.stopped) {
          print("[" + this.deviceName + "] link dropped, retrying");
          this.rx = null;
          this.busy = false;
          this.waiter = null;
          this.retry(() => this.attempt(), 3);
          this.set("lost");
        }
      });
      const rx = findChar(gatt, NUS_SERVICE, NUS_RX);
      const tx = findChar(gatt, NUS_SERVICE, NUS_TX);
      if (!rx || !tx) throw new Error(this.deviceName + " has no console service");
      await tx.registerNotifications((v: Uint8Array) => this.onTx(v));
      this.rx = rx;
      this.seen = true;
      print("[" + this.deviceName + "] live");
      this.set("live");
    } catch (e) {
      print("[" + this.deviceName + "] " + e);
      this.retry(() => this.attempt(), 5);
    }
  }

  private onTx(v: Uint8Array) {
    for (let i = 0; i < v.length; i++) this.buf += String.fromCharCode(v[i]);
    const at = this.buf.lastIndexOf("ok>");
    if (at >= 0 && this.waiter) {
      const w = this.waiter;
      this.waiter = null;
      const body = this.buf.substring(0, at).replace(/\r/g, "");
      this.buf = "";
      w(body);
    } else if (!this.waiter && this.buf.length > 16384) {
      this.buf = "";                         // unsolicited chatter: drop it
    }
  }

  private async write(s: string) {
    const bytes = new Uint8Array(s.length);
    for (let i = 0; i < s.length; i++) bytes[i] = s.charCodeAt(i) & 0xff;
    for (let i = 0; i < bytes.length; i += 20) {
      if (!this.rx) throw new Error(this.deviceName + " link dropped");
      await this.rx.writeValue(bytes.slice(i, i + 20));
    }
  }
}
