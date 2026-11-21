// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// BleHub.ts — one scanner for the whole lens.
//
// Spectacles run one scan at a time: starting a second scan fails the first
// with "Scan failed: DISCONNECTED" (seen on the glasses 2026-10-22). Every
// link asks the hub to find its device, and the hub runs those requests one
// after another. Connected links scan nothing, so the queue is short.

export type ScanMatch = (r: Bluetooth.ScanResult) => boolean;

// "0x2A37", "2a37" and "00002a37-0000-1000-8000-00805f9b34fb" all name 2a37.
export function short16(uuid: string): string {
  const u = (uuid || "").toLowerCase().replace(/^0x/, "");
  return u.length === 36 && u.endsWith("-0000-1000-8000-00805f9b34fb") ? u.substring(4, 8) : u;
}

export function sameUuid(a: string, b: string): boolean {
  return short16(a) === short16(b);
}

export function findChar(gatt: Bluetooth.BluetoothGatt, service: string, chr: string): Bluetooth.BluetoothGattCharacteristic {
  for (const s of gatt.getServices()) {
    if (!sameUuid(s.uuid, service)) continue;
    for (const c of s.getCharacteristics()) if (sameUuid(c.uuid, chr)) return c;
  }
  return null;
}

export class BleHub {
  private queue: Promise<unknown> = Promise.resolve();

  constructor(
    public readonly module: Bluetooth.BluetoothCentralModule,
    private pause: (ms: number) => Promise<void>,
  ) {}

  // Find the first device that matches, after any scan already running.
  // Resolves null on timeout instead of throwing, so callers just retry.
  find(match: ScanMatch, timeoutSeconds = 20): Promise<Bluetooth.ScanResult | null> {
    const run = async (): Promise<Bluetooth.ScanResult | null> => {
      const settings = new Bluetooth.ScanSettings();
      settings.timeoutSeconds = timeoutSeconds;
      settings.scanMode = Bluetooth.ScanMode.LowLatency;
      let found: Bluetooth.ScanResult | null = null;
      try {
        found = (await this.module.startScan([new Bluetooth.ScanFilter()], settings, match)) || null;
      } catch (e) {
        print("[ble] scan: " + e);
      }
      // A matched scan is still winding down when its promise resolves; the
      // next one started at once failed with DISCONNECTED. Give it a moment.
      await this.pause(400);
      return found;
    };
    const next = this.queue.then(run, run);
    this.queue = next.catch(() => null);
    return next;
  }

  connect(r: Bluetooth.ScanResult): Promise<Bluetooth.BluetoothGatt> {
    return this.module.connectGatt(r.deviceAddress);
  }
}
