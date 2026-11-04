// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// LanProbe.ts — can a lens with Bluetooth also pull frames from a camera
// on the LAN over plain http/ws, by IP?
//
// Scout (projects/prototypes/scout_m5camera_espidforth, v0.5.0) serves its
// JPEGs at http://<ip>/frame.jpg, /stream, /ws and a test page at /, and
// answers `url?` over its BLE console. Spectacles have no mDNS, so the lens
// learns the IP over BLE. What we don't know is which network calls a lens
// may make while it also uses Bluetooth, and whether the order matters
// (SwitchbackXR's PERMISSION_DENIED may have been order, not a rule).
//
// order:
//   "ble-first"  BLE scan + connect, url? over BLE, then every net step,
//                then a BLE line again (is BLE still alive?)
//   "net-first"  every net step against `scoutUrl`, then BLE scan + url?,
//                then the net steps again
//   "net-only"   net steps against `scoutUrl`, no Bluetooth at all
//
// Net steps, each logged PASS/FAIL with the error text:
//   fetch    InternetModule.fetch(/info), then /frame.jpg x frames: ms, fps
//   media    RemoteMediaModule.loadResourceAsImageTexture(makeResourceFromUrl) x frames
//   ws       createWebSocket(/ws): send "f", JPEG blob back -> texture, for wsSeconds
//   webview  createWebView, loadUrl(/): the page itself loops /frame.jpg and
//            shows its FPS; Scout's `web?` counts the hits
//
// Setup in Lens Studio:
//   1. Add this script to a SceneObject; drag the Bluetooth Central Module
//      asset into bluetoothModule
//   2. Optional: Image components for frameImage / webImage, a Text for output
//   3. Scout on the same Wi-Fi as the glasses; for net-first set scoutUrl
//      (Scout prints it on its console: `url?`)
//   4. Send to Spectacles; read the Logger ([lan] lines) and Scout's `web?`

@component
export class LanProbe extends BaseScriptComponent {

  @input
  @hint("Bluetooth Central Module asset")
  bluetoothModule: Bluetooth.BluetoothCentralModule;

  @input
  @hint("ble-first | net-first | net-only")
  order: string = "ble-first";

  @input
  @hint("Scout's base URL for net-first / net-only, e.g. http://192.168.1.50/")
  scoutUrl: string = "";

  @input
  @hint("Advertised BLE name")
  deviceName: string = "Scout";

  @input
  @hint("Frames to time per step")
  frames: number = 20;

  @input
  @hint("Seconds to run the WebSocket loop")
  wsSeconds: number = 10;

  @input
  @hint("Seconds to wait before starting")
  startDelay: number = 1;

  @input
  @allowUndefined
  frameImage: Image;

  @input
  @allowUndefined
  webImage: Image;

  @input
  @allowUndefined
  output: Text;

  private internet = require("LensStudio:InternetModule") as InternetModule;
  private media = require("LensStudio:RemoteMediaModule") as RemoteMediaModule;
  private lines: string[] = [];
  private rx: Bluetooth.BluetoothGattCharacteristic = null;
  private rxBuf = "";
  private onPrompt: (() => void) | null = null;
  private webview: WebPageTextureProvider = null;

  onAwake() {
    if (this.frameImage) this.frameImage.mainMaterial = this.frameImage.mainMaterial.clone();
    if (this.webImage) this.webImage.mainMaterial = this.webImage.mainMaterial.clone();
    this.after(Math.max(0.01, this.startDelay), () => { this.run(); });
  }

  private log(s: string) {
    print("[lan] " + s);
    this.lines.push(s);
    if (this.lines.length > 16) this.lines.shift();
    if (this.output) this.output.text = this.lines.join("\n");
  }

  private after(s: number, fn: () => void) {
    const ev = this.createEvent("DelayedCallbackEvent");
    ev.bind(fn);
    ev.reset(s);
  }

  private wait(ms: number): Promise<void> {
    return new Promise((r) => this.after(ms / 1000, () => r()));
  }

  private async run() {
    this.log("order " + this.order + ", device " + (global.deviceInfoSystem.isSpectacles() ? "Spectacles" : "editor"));
    try {
      if (this.order === "ble-first") {
        const url = await this.bleUrl();
        await this.netSteps(url);
        await this.bleCheck();
      } else if (this.order === "net-first") {
        await this.netSteps(this.base(this.scoutUrl));
        const url = await this.bleUrl();
        await this.netSteps(url);
        await this.bleCheck();
      } else {
        await this.netSteps(this.base(this.scoutUrl));
      }
      this.log("done");
    } catch (e) {
      this.log("FAIL " + e);
    }
  }

  private base(u: string): string {
    if (!u) throw new Error("scoutUrl is empty");
    return u.endsWith("/") ? u : u + "/";
  }

  // ---- BLE: scan, connect, ask Scout for its URL ----

  private async bleUrl(): Promise<string> {
    const t0 = Date.now();
    this.log("ble: scanning for " + this.deviceName);
    const st = new Bluetooth.ScanSettings();
    st.timeoutSeconds = 30;
    st.scanMode = Bluetooth.ScanMode.LowLatency;
    let found: Bluetooth.ScanResult = null;
    try {
      found = await this.bluetoothModule.startScan([new Bluetooth.ScanFilter()], st,
        (r: Bluetooth.ScanResult) => r.deviceName === this.deviceName);
    } catch (e) {
      throw new Error("ble scan: " + e);
    }
    if (!found) throw new Error("ble scan found nothing");
    const gatt = await this.bluetoothModule.connectGatt(found.deviceAddress);
    gatt.onConnectionStateChangedEvent.add((e) => this.log("ble: link state " + e.state));
    let tx: Bluetooth.BluetoothGattCharacteristic = null;
    for (const s of gatt.getServices())
      for (const c of s.getCharacteristics()) {
        const u = c.uuid.toLowerCase();
        if (u === "6e400002-b5a3-f393-e0a9-e50e24dcca9e") this.rx = c;
        if (u === "6e400003-b5a3-f393-e0a9-e50e24dcca9e") tx = c;
      }
    if (!this.rx || !tx) throw new Error("no NUS on " + this.deviceName);
    await tx.registerNotifications((v: Uint8Array) => this.onTx(v));
    this.log("ble: PASS live in " + (Date.now() - t0) + " ms");
    const reply = await this.query("url?");
    const m = reply.match(/url (http:\/\/\S+)/);
    if (!m) throw new Error("Scout has no URL yet: " + reply.replace(/\s+/g, " ").trim());
    this.log("ble: url " + m[1]);
    return m[1];
  }

  private async bleCheck() {
    if (!this.rx) return;
    try {
      const t = Date.now();
      const r = await this.query("web?");
      this.log("ble: PASS still live (" + (Date.now() - t) + " ms) " + (r.match(/web .*/) || [""])[0]);
    } catch (e) {
      this.log("ble: FAIL after net steps: " + e);
    }
  }

  private onTx(v: Uint8Array) {
    for (let i = 0; i < v.length; i++) this.rxBuf += String.fromCharCode(v[i]);
    if (this.rxBuf.indexOf("ok>") >= 0 && this.onPrompt) {
      const done = this.onPrompt;
      this.onPrompt = null;
      done();
    }
  }

  private query(line: string): Promise<string> {
    return new Promise<string>((resolve, reject) => {
      this.rxBuf = "";
      let done = false;
      this.after(3, () => { if (!done) { this.onPrompt = null; reject(new Error("no reply to " + line)); } });
      this.onPrompt = () => { done = true; resolve(this.rxBuf); };
      const s = line + "\r";
      const b = new Uint8Array(s.length);
      for (let i = 0; i < s.length; i++) b[i] = s.charCodeAt(i);
      this.rx.writeValue(b).catch((e) => { done = true; this.onPrompt = null; reject(e); });
    });
  }

  // ---- the network steps ----

  private async netSteps(base: string) {
    this.log("net: " + base);
    await this.step("fetch", () => this.fetchStep(base));
    await this.step("media", () => this.mediaStep(base));
    await this.step("ws", () => this.wsStep(base));
    await this.step("webview", () => this.webviewStep(base));
  }

  private async step(name: string, fn: () => Promise<string>) {
    try {
      this.log(name + ": PASS " + await fn());
    } catch (e) {
      this.log(name + ": FAIL " + e);
    }
  }

  private show(tex: Texture) {
    if (this.frameImage) this.frameImage.mainPass.baseTex = tex;
  }

  private rate(n: number, ms: number): string {
    return n + " frames, " + (ms / Math.max(1, n)).toFixed(0) + " ms each, " + (n * 1000 / Math.max(1, ms)).toFixed(1) + " fps";
  }

  private async fetchStep(base: string): Promise<string> {
    const info = await this.internet.fetch(base + "info");
    if (!info.ok) throw new Error("/info HTTP " + info.status);
    const text = await info.text();
    const t0 = Date.now();
    let bytes = 0, last: Uint8Array = null;
    for (let i = 0; i < this.frames; i++) {
      const r = await this.internet.fetch(base + "frame.jpg?" + Date.now());
      if (!r.ok) throw new Error("/frame.jpg HTTP " + r.status);
      last = await r.bytes();
      bytes += last.length;
    }
    const ms = Date.now() - t0;
    if (last) Base64.decodeTextureAsync(Base64.encode(last), (t) => this.show(t), () => this.log("fetch: decode failed"));
    return text + "; " + this.rate(this.frames, ms) + ", " + Math.round(bytes / this.frames) + " B";
  }

  private loadUrl(url: string): Promise<Texture> {
    return new Promise((resolve, reject) =>
      this.media.loadResourceAsImageTexture(this.internet.makeResourceFromUrl(url), resolve, (m) => reject(new Error(m))));
  }

  private async mediaStep(base: string): Promise<string> {
    const t0 = Date.now();
    for (let i = 0; i < this.frames; i++) this.show(await this.loadUrl(base + "frame.jpg?" + Date.now()));
    return this.rate(this.frames, Date.now() - t0);
  }

  private wsStep(base: string): Promise<string> {
    const url = base.replace(/^http/, "ws") + "ws";
    return new Promise<string>((resolve, reject) => {
      let ws: WebSocket;
      try {
        ws = this.internet.createWebSocket(url);
      } catch (e) {
        reject(new Error("createWebSocket: " + e));
        return;
      }
      ws.binaryType = "blob";
      let n = 0, t0 = 0, settled = false;
      const finish = (err?: string) => {
        if (settled) return;
        settled = true;
        try { ws.close(); } catch (e) { /* closed */ }
        if (err && n === 0) reject(new Error(err));
        else resolve(this.rate(n, Date.now() - t0) + (err ? " (then " + err + ")" : ""));
      };
      this.after(this.wsSeconds + 5, () => finish(n ? undefined : "no frames in " + (this.wsSeconds + 5) + " s"));
      ws.onopen = () => { t0 = Date.now(); ws.send("f"); };
      ws.onerror = () => finish("socket error");
      ws.onclose = (e) => finish("closed " + e.code);
      ws.onmessage = async (e) => {
        if (settled) return;
        if (typeof e.data === "string") { finish("text " + e.data); return; }
        n++;
        if (Date.now() - t0 > this.wsSeconds * 1000) { finish(); return; }
        ws.send("f");                                 // ask for the next while this one decodes
        const blob = e.data as Blob;
        this.media.loadResourceAsImageTexture(this.internet.makeResourceFromBlob(blob), (t) => this.show(t), () => {});
      };
    });
  }

  private webviewStep(base: string): Promise<string> {
    if (!global.deviceInfoSystem.isSpectacles() || global.deviceInfoSystem.isEditor())
      return Promise.reject(new Error("WebView needs Spectacles"));
    if (this.webview) {
      this.webview.loadUrl(base);
      return Promise.resolve("reloaded " + base);
    }
    return new Promise<string>((resolve, reject) => {
      const t0 = Date.now();
      try {
        this.internet.createWebView(InternetModule.createWebViewOptions(new vec2(640, 480)), (tex) => {
          if (this.webImage) this.webImage.mainPass.baseTex = tex;
          this.webview = tex.control as WebPageTextureProvider;
          this.webview.onReady.add(() => {
            this.webview.loadUrl(base);
            resolve("ready in " + (Date.now() - t0) + " ms, loading " + base + " (watch its fps and Scout's web?)");
          });
        }, (m) => reject(new Error(m)));
      } catch (e) {
        reject(new Error("createWebView: " + e));
      }
      this.after(15, () => reject(new Error("no onReady in 15 s")));
    });
  }
}
