// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// TapLink.ts — the NFCTapIn reader's tap stream, from the glasses.
//
// The reader (projects/prototypes/NFCTapInXR_AtomMatrix_proto1) is a Nordic
// UART peripheral with no bonding. It notifies one line per event on TX, in
// 20-byte pieces: "TAP <id hex>\n" for a tag, "ERR read\n" for a bad read.
//
// The connection is staged so the lens can show where it is and never waits
// on it: searching -> connecting -> subscribing -> live. Every step is a
// promise with its own time limit; nothing here loops or blocks, and the
// first scan starts only after the caller's delay, once the UI is drawn.
//
// Scanning is the expensive part on the glasses (a scan with a script
// predicate calls into the lens for every advertisement nearby), so:
//   - the scan carries a native name filter, so only the reader's own
//     advertisements reach the predicate;
//   - a search is a fixed number of short scans, then it stops and reports
//     "notfound" instead of scanning forever. retry() starts a new search.
// Anything that is not a plain timeout (Bluetooth off, permission denied, a
// connect that fails) ends in "failed" with the platform's message in `error`.
// A link that drops after being live searches again once, on its own.

import { findChar } from "../ble/BleHub";

const NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";

const SCAN_SECONDS = 7;
const SCAN_TRIES = 2;
const SCAN_GAP_SECONDS = 1;
const CONNECT_SECONDS = 12;
const SUBSCRIBE_SECONDS = 8;

export type TapLinkState = "idle" | "searching" | "connecting" | "subscribing" | "live" | "lost" | "notfound" | "failed";

export class TapLink {
  state: TapLinkState = "idle";
  error = "";                 // why, when state is "failed" (and the last scan complaint, when "notfound")
  attempt = 0;                // which scan of the search is running, 1-based
  readonly tries = SCAN_TRIES;

  private buf = "";
  private gen = 0;            // bumped by start/retry/stop: results from an older run are dropped
  private searchStarted = 0;

  constructor(
    public readonly deviceName: string,
    private module: Bluetooth.BluetoothCentralModule,
    private onChange: () => void,
    private onTap: (id: string) => void,
    private onBadRead: () => void,
    private later: (fn: () => void, seconds: number) => void,
    private now: () => number,
  ) {}

  get live(): boolean { return this.state === "live"; }
  get busy(): boolean { return this.state === "searching" || this.state === "connecting" || this.state === "subscribing" || this.state === "lost"; }

  // 0..1 through the current search, for a progress bar.
  get searchProgress(): number {
    const total = SCAN_TRIES * SCAN_SECONDS + (SCAN_TRIES - 1) * SCAN_GAP_SECONDS;
    const p = (this.now() - this.searchStarted) / total;
    return p < 0 ? 0 : p > 1 ? 1 : p;
  }

  // Begin the first search after delaySeconds, so the caller's UI is up first.
  start(delaySeconds = 0) {
    const g = ++this.gen;
    this.later(() => { if (g === this.gen) this.search(g, false); }, delaySeconds);
  }

  // A new search after "notfound" or "failed" (ignored while one is running or the link is live).
  retry() {
    if (this.busy || this.live) return;
    this.search(++this.gen, false);
  }

  stop() {
    this.gen++;
    this.module.stopScan().catch(() => null);
  }

  // For the editor preview, which has no Bluetooth: show a state as if the link were in it.
  simulate(state: TapLinkState, error = "", attempt = 1) {
    if (state === "searching" && this.state !== "searching") this.searchStarted = this.now();
    this.error = error;
    this.attempt = attempt;
    this.set(state);
  }

  private set(s: TapLinkState) {
    this.state = s;
    this.onChange();
  }

  private fail(g: number, message: string) {
    if (g !== this.gen) return;
    this.error = message;
    print("[" + this.deviceName + "] failed: " + message);
    this.set("failed");
  }

  private pause(seconds: number): Promise<void> {
    return new Promise<void>((r) => this.later(r, seconds));
  }

  private limit<T>(p: Promise<T>, seconds: number, what: string): Promise<T> {
    return new Promise<T>((resolve, reject) => {
      let done = false;
      this.later(() => { if (!done) { done = true; reject(new Error(what + " took longer than " + seconds + " s")); } }, seconds);
      p.then((v) => { if (!done) { done = true; resolve(v); } }, (e) => { if (!done) { done = true; reject(e); } });
    });
  }

  private async search(g: number, afterDrop: boolean) {
    this.error = "";
    this.searchStarted = this.now();
    let found: Bluetooth.ScanResult | null = null;
    for (let i = 1; i <= SCAN_TRIES && !found; i++) {
      if (g !== this.gen) return;
      this.attempt = i;
      this.set(afterDrop ? "lost" : "searching");
      const filter = new Bluetooth.ScanFilter();
      filter.deviceName = this.deviceName;          // filtered natively: the predicate sees only the reader
      const settings = new Bluetooth.ScanSettings();
      settings.timeoutSeconds = SCAN_SECONDS;
      settings.scanMode = Bluetooth.ScanMode.Balanced;
      try {
        found = (await this.module.startScan([filter], settings, (r) => r.deviceName === this.deviceName)) || null;
      } catch (e) {
        const msg = "" + e;
        // A scan that just ran out of time is "not there"; anything else is a real fault.
        if (/PERMISSION|DENIED|UNAUTHORI|DISABLED|POWERED|UNSUPPORTED|NOT_AVAILABLE/i.test(msg)) {
          this.fail(g, "Bluetooth scan refused: " + msg);
          return;
        }
        this.error = msg;
        print("[" + this.deviceName + "] scan " + i + ": " + msg);
      }
      if (g !== this.gen) return;
      if (!found && i < SCAN_TRIES) await this.pause(SCAN_GAP_SECONDS);
    }
    if (g !== this.gen) return;
    if (!found) {
      print("[" + this.deviceName + "] not found after " + SCAN_TRIES + " scans");
      this.set("notfound");
      return;
    }
    await this.pause(0.4);                           // a matched scan is still winding down when it resolves
    if (g !== this.gen) return;

    try {
      this.set("connecting");
      const gatt = await this.limit(this.module.connectGatt(found.deviceAddress), CONNECT_SECONDS, "connecting");
      if (g !== this.gen) return;
      gatt.onConnectionStateChangedEvent.add((e) => {
        if (e.state !== Bluetooth.ConnectionState.Disconnected || g !== this.gen) return;
        print("[" + this.deviceName + "] link dropped, searching again");
        this.buf = "";
        const g2 = ++this.gen;
        this.set("lost");
        this.later(() => { if (g2 === this.gen) this.search(g2, true); }, 2);
      });
      this.set("subscribing");
      const tx = findChar(gatt, NUS_SERVICE, NUS_TX);
      if (!tx || !findChar(gatt, NUS_SERVICE, NUS_RX)) throw new Error("it has no tap (UART) service; is this the right firmware?");
      await this.limit(tx.registerNotifications((v: Uint8Array) => this.onTx(v)), SUBSCRIBE_SECONDS, "subscribing");
      if (g !== this.gen) return;
      print("[" + this.deviceName + "] live");
      this.set("live");
    } catch (e) {
      this.fail(g, "" + (e && (e as Error).message ? (e as Error).message : e));
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
