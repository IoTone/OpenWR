// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// TapLink.ts — the NFCTapIn reader's tap stream, from the glasses.
//
// The reader (projects/prototypes/NFCTapInXR_AtomMatrix_proto1) is a Nordic
// UART peripheral with no bonding. It notifies one line per event on TX, in
// 20-byte pieces: "TAP <id hex>\n" for a tag, "ERR read\n" for a bad read,
// "pong\n" in answer to "ping". This link only listens: it finds the reader
// by name, subscribes, splits the notifications into lines and calls back.
// The link is held for the lens's life and retried after a drop.

import { BleHub, findChar } from "../ble/BleHub";

const NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

export type TapLinkState = "searching" | "connecting" | "live" | "lost";

export class TapLink {
  state: TapLinkState = "searching";

  private buf = "";
  private seen = false;
  private stopped = false;

  constructor(
    public readonly deviceName: string,
    private hub: BleHub,
    private onChange: () => void,
    private onTap: (id: string) => void,
    private onBadRead: () => void,
    private retry: (fn: () => void, s: number) => void,
  ) {}

  get live(): boolean { return this.state === "live"; }

  start() {
    this.stopped = false;
    this.attempt();
  }

  stop() {
    this.stopped = true;
  }

  private set(s: TapLinkState) {
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
          this.buf = "";
          this.retry(() => this.attempt(), 3);
          this.set("lost");
        }
      });
      const tx = findChar(gatt, NUS_SERVICE, NUS_TX);
      if (!tx || !findChar(gatt, NUS_SERVICE, NUS_RX)) throw new Error(this.deviceName + " has no UART service");
      await tx.registerNotifications((v: Uint8Array) => this.onTx(v));
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
    let nl = this.buf.indexOf("\n");
    while (nl >= 0) {
      const line = this.buf.substring(0, nl).replace(/\r/g, "").trim();
      this.buf = this.buf.substring(nl + 1);
      this.onLine(line);
      nl = this.buf.indexOf("\n");
    }
    if (this.buf.length > 256) this.buf = "";   // a line is at most ~25 bytes: this is noise
  }

  private onLine(line: string) {
    const tap = /^TAP ([0-9A-Fa-f]{8,20})$/.exec(line);
    if (tap) {
      print("[" + this.deviceName + "] tap " + tap[1]);
      this.onTap(tap[1].toUpperCase());
    } else if (line.indexOf("ERR") === 0) {
      print("[" + this.deviceName + "] " + line);
      this.onBadRead();
    }
  }
}
