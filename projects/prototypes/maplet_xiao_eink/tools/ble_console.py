#!/usr/bin/env python3
"""
ble_console.py — the stand-in for the OpenWR app: talk to maplet's ok>
prompt over Bluetooth LE (Nordic UART Service).

    python3 tools/ble_console.py status                 # send one line, print reply
    python3 tools/ble_console.py "2 theme!" status      # several lines
    python3 tools/ble_console.py --bundle forth/weather-hud/*.fs
    python3 tools/ble_console.py                        # interactive

The device only advertises inside a pairing window: long-press BOOT, or
type `pair` on the USB console. Needs bleak (pip install bleak).
"""
import asyncio, sys, time
from bleak import BleakScanner, BleakClient

NUS_RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
NUS_TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
NAME   = "maplet"

def bundle_lines(files):
    out = []
    for f in files:
        for raw in open(f):
            line = raw.rstrip("\n").rstrip(); st = line.strip()
            if not st: continue
            if st.startswith("\\"):
                if st.startswith("\\ bundle:") or st.startswith("\\ requires:"): out.append(st)
                continue
            out.append(line)
    return out

async def main():
    args = sys.argv[1:]
    bundle = "--bundle" in args
    args = [a for a in args if a != "--bundle"]

    print(f"scanning for {NAME} (open a pairing window on the device)…")
    dev = None
    for _ in range(6):
        dev = await BleakScanner.find_device_by_name(NAME, timeout=5.0)
        if dev: break
    if not dev:
        print("not found. Long-press BOOT or type `pair` at the USB prompt, then retry."); sys.exit(1)

    rx_buf = []
    def on_notify(_, data: bytearray):
        text = data.decode("utf-8", "replace")
        rx_buf.append(text)
        sys.stdout.write(text); sys.stdout.flush()

    async with BleakClient(dev) as c:
        print(f"connected, mtu {c.mtu_size}")
        await c.start_notify(NUS_TX, on_notify)
        async def send(line):
            await c.write_gatt_char(NUS_RX, (line + "\r").encode(), response=False)

        async def wait_prompt(timeout):
            t0 = time.time()
            while time.time() - t0 < timeout:
                if "".join(rx_buf).rstrip().endswith("ok>"): return
                await asyncio.sleep(0.05)

        await send(""); await wait_prompt(2); rx_buf.clear()

        if bundle:
            lines = bundle_lines(args)
            await send("bundle-begin"); await asyncio.sleep(0.3)
            for l in lines:
                await send(l); await asyncio.sleep(0.03)
            await send("bundle-end"); await wait_prompt(40)
            print(f"\n{len(lines)} lines sent")
        elif args:
            for l in args:
                rx_buf.clear(); await send(l); await wait_prompt(30)
        else:
            print("interactive; Ctrl-C to quit")
            loop = asyncio.get_event_loop()
            while True:
                line = await loop.run_in_executor(None, sys.stdin.readline)
                if not line: break
                await send(line.rstrip("\n")); await wait_prompt(30)
        await asyncio.sleep(0.3)

asyncio.run(main())
