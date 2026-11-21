// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// NusProbe.ts — can Spectacles talk to a maplet or goggl1 without bonding?
//
// Lens Studio's BluetoothCentralModule has no createBond(), so a device
// whose characteristics need encryption hangs registerNotifications
// (SkywriterBLE hit this with an HID keyboard). The maplet and goggl1 serve
// an open Nordic UART console, no SMP at all. This probe walks the path
// SpecsSwitchbackXR would use and reports each step:
//
//   scan by name -> connect -> list services and characteristic properties
//   -> subscribe to TX -> write Forth lines to RX in (mtu - 3) chunks
//   -> time 20 round trips with and without write response
//
// Setup in Lens Studio:
//   1. Project Settings: Experimental API on (already set in this project)
//   2. Add this script to a SceneObject
//   3. Drag Assets/Bluetooth/Bluetooth Central Module into "bluetoothModule"
//   4. Optional: drag a Text component into "output" to read results in the glasses
//   5. On the device, run `pair` on its console (or power-cycle) so it advertises
//   6. Send the lens to Spectacles; watch the Logger
//
// Every line is printed with a [nus] prefix.

const NUS_SERVICE = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";   // we write here
const NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e";   // device notifies here

const PROP_NAMES: string[] = [
  "Broadcast", "Read", "WriteWithoutResponse", "Write", "Notify", "Indicate",
  "SignedWrite", "ExtendedProps", "NotifyEncryptionRequired", "IndicateEncryptionRequired",
];

@component
export class NusProbe extends BaseScriptComponent {

  @input
  @hint("Bluetooth Central Module asset")
  bluetoothModule: Bluetooth.BluetoothCentralModule;

  @input
  @hint("Advertised name to connect to: maplet, goggl1, ...")
  deviceName: string = "maplet";

  @input
  @hint("Round trips to time for each write mode")
  rounds: number = 20;

  @input
  @hint("Seconds to wait before scanning (stagger probes; one scan at a time)")
  startDelay: number = 0;

  @input
  @allowUndefined
  @hint("Optional Text to show results in the glasses")
  output: Text;

  private lines: string[] = [];
  private rxBuf = "";
  private onPrompt: (() => void) | null = null;

  onAwake() {
    const ev = this.createEvent("DelayedCallbackEvent");
    ev.bind(() => { this.run(); });
    ev.reset(Math.max(0.01, this.startDelay));
  }

  private log(s: string) {
    print("[nus] " + s);
    this.lines.push(s);
    if (this.lines.length > 14) this.lines.shift();
    if (this.output) this.output.text = this.lines.join("\n");
  }

  private async run() {
    const t0 = Date.now();
    try {
      this.log("scanning for '" + this.deviceName + "'");
      const settings = new Bluetooth.ScanSettings();
      settings.timeoutSeconds = 30;
      settings.scanMode = Bluetooth.ScanMode.LowLatency;
      const found = await this.bluetoothModule.startScan([new Bluetooth.ScanFilter()], settings,
        (r: Bluetooth.ScanResult) => r.deviceName === this.deviceName);
      if (!found) throw new Error("scan found nothing (no Bluetooth in the editor preview; run on Spectacles)");
      this.log("found " + found.deviceName + " rssi " + found.rssi + " (" + (Date.now() - t0) + " ms)");

      const t1 = Date.now();
      const gatt = await this.bluetoothModule.connectGatt(found.deviceAddress);
      this.log("connected in " + (Date.now() - t1) + " ms, mtu " + gatt.mtu);
      gatt.onConnectionStateChangedEvent.add((e) => this.log("link state " + e.state));
      gatt.onMtuChangedEvent.add((e) => this.log("mtu now " + e.mtu));

      let rx: Bluetooth.BluetoothGattCharacteristic = null;
      let tx: Bluetooth.BluetoothGattCharacteristic = null;
      for (const s of gatt.getServices()) {
        for (const c of s.getCharacteristics()) {
          const props = (c.properties || []).map((p) => PROP_NAMES[p] || String(p)).join(",");
          print("[nus]   " + s.uuid + " / " + c.uuid + " [" + props + "]");
          if (s.uuid.toLowerCase() !== NUS_SERVICE) continue;
          if (c.uuid.toLowerCase() === NUS_RX) rx = c;
          if (c.uuid.toLowerCase() === NUS_TX) tx = c;
        }
      }
      if (!rx || !tx) throw new Error("no Nordic UART service; is this a maplet or goggl1?");
      this.log("NUS found");

      const t2 = Date.now();
      await tx.registerNotifications((v: Uint8Array) => this.onTx(v));
      this.log("subscribed without bonding in " + (Date.now() - t2) + " ms");

      await this.query(rx, "depth .", true);
      for (const noRsp of [false, true]) {
        const rtts: number[] = [];
        for (let i = 0; i < this.rounds; i++) {
          const t = Date.now();
          await this.query(rx, i + " 2 + .", noRsp);
          rtts.push(Date.now() - t);
        }
        rtts.sort((a, b) => a - b);
        this.log((noRsp ? "no-rsp" : "with-rsp") + " rtt ms min " + rtts[0] + " med "
          + rtts[rtts.length >> 1] + " max " + rtts[rtts.length - 1]);
      }
      const long = await this.query(rx, "1 2 3 4 5 6 7 8 9 10 + + + + + + + + + . depth .", true);
      this.log("long line: " + long.replace(/\s+/g, " ").trim());
      this.log("PASS: no bonding needed (" + (Date.now() - t0) + " ms total)");
    } catch (e) {
      this.log("FAIL: " + e);
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

  // Send one console line and wait for the "ok>" prompt; returns the reply.
  private query(rx: Bluetooth.BluetoothGattCharacteristic, line: string, noRsp: boolean): Promise<string> {
    return new Promise<string>((resolve, reject) => {
      this.rxBuf = "";
      const timeout = this.createEvent("DelayedCallbackEvent");
      timeout.bind(() => {
        if (!this.onPrompt) return;
        this.onPrompt = null;
        reject(new Error("no reply to '" + line + "'"));
      });
      this.onPrompt = () => { timeout.enabled = false; resolve(this.rxBuf); };
      timeout.reset(3);
      this.write(rx, line + "\r", noRsp).catch((e) => { this.onPrompt = null; reject(e); });
    });
  }

  private async write(rx: Bluetooth.BluetoothGattCharacteristic, s: string, noRsp: boolean) {
    const bytes = new Uint8Array(s.length);
    for (let i = 0; i < s.length; i++) bytes[i] = s.charCodeAt(i) & 0xff;
    const chunk = 20;   // fits the default 23-byte MTU; there is no requestMtu()
    for (let i = 0; i < bytes.length; i += chunk) {
      const part = bytes.slice(i, i + chunk);
      if (noRsp) await rx.writeValueWithoutResponse(part);
      else await rx.writeValue(part);
    }
  }
}
