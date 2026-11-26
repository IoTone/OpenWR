#!/usr/bin/env python3
"""
fake_pass.py: a pretend pass on this computer's Bluetooth, for working on the phone app with no
hardware on the desk. It advertises as SR-16 (or SR-15), speaks the same console over the Nordic
UART Service as the firmware, takes packs with sd-recv (checking the CRC), and "shows" the open
page by writing glass.png.

    fake_pass.py                      # an SR-16 with no keys, always listening
    fake_pass.py --checkin 20         # asleep; wakes and advertises 5 s in every 20, like the real one
    fake_pass.py --board sr15 --dir /tmp/pass

It only knows the words the app uses (sr-info, sr-next, sr-prev, sr-shelf, sr-open, sd-rm,
sd-recv, sr-auto!, sr-sync!, sr-sleep). It proves the app and the pack format, not the firmware.
Needs bless and Pillow:  uv pip install bless pillow
"""
import argparse, asyncio, os, sys, time, zlib

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import svpk
from bless import BlessServer, GATTCharacteristicProperties as P, GATTAttributePermissions as A

NUS = "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
RX = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
TX = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
BOARDS = {"sr16": ("SR-16", 240, 416, 0, 6592), "sr15": ("SR-15", 300, 400, 5, 1900000)}


class Pass:
    def __init__(self, a):
        self.a, self.dir = a, a.dir
        self.name, self.w, self.h, self.keys, self.total = BOARDS[a.board]
        self.auto, self.sync = 0, 300 if self.keys == 0 else 0
        self.open, self.page = None, 0
        self.buf, self.recv, self.out = b"", None, asyncio.Queue()
        self.sleep_asked = False
        os.makedirs(self.dir, exist_ok=True)

    def packs(self):
        return sorted(f for f in os.listdir(self.dir) if f.endswith(".svpk"))

    def glass(self):
        from PIL import Image, ImageDraw
        path = os.path.join(self.dir, "glass.png")
        if self.open and os.path.exists(os.path.join(self.dir, self.open)):
            p = svpk.read_pack(os.path.join(self.dir, self.open))
            off, ln, comp, _, _ = p["pages"][self.page]
            raw = svpk.page_bytes(p["data"][off:off + ln], comp, ((p["w"] + 7) // 8) * p["h"])
            Image.frombytes("1", (p["w"], p["h"]), bytes(b ^ 0xFF for b in raw)).save(path)
            print(f"   glass: {self.open} page {self.page + 1}/{p['n']} ({p['w']}x{p['h']}) -> {path}")
        else:
            img = Image.new("1", (self.w, self.h), 1)
            d = ImageDraw.Draw(img)
            d.text((8, 8), "SUBVERSE READER", fill=0)
            for i, f in enumerate(self.packs()):
                d.text((8, 40 + 14 * i), f, fill=0)
            img.save(path)
            print(f"   glass: shelf, {len(self.packs())} packs -> {path}")

    def info(self):
        used = sum(os.path.getsize(os.path.join(self.dir, f)) for f in self.packs()) // 1024
        s = (f"screen {'page' if self.open else 'shelf'}, flash mounted, {len(self.packs())} packs, panel UC8253, rot 0, "
             f"sleep after 120 s, full refresh every 8\n"
             f"pass {self.a.board} {self.w}x{self.h} keys {self.keys} auto {self.auto} sync {self.sync} "
             f"store {self.total}/{self.total - used} KB bat 87%\n")
        if self.open:
            p = svpk.read_pack(os.path.join(self.dir, self.open))
            s += f"open: /fl/packs/{self.open}  '{p['title']}' by {p['author']}, page {self.page + 1} of {p['n']}\n"
        for i, f in enumerate(self.packs()):
            s += f"  {i:2d}  /fl/packs/{f}\n"
        return s

    def turn(self, d):
        if not self.open:
            return
        n = svpk.read_pack(os.path.join(self.dir, self.open))["n"]
        p = self.page + d
        if self.keys == 0:
            p %= n
        if 0 <= p < n:
            self.page = p
            self.glass()

    def run(self, line):
        print(f"<- {line}")
        t, st = line.split(), []
        for w in t:
            if w.lstrip("-").isdigit():
                st.append(int(w))
            elif w == "sr-info": return self.info()
            elif w == "sr-next": self.turn(+1)
            elif w == "sr-prev": self.turn(-1)
            elif w == "sr-shelf": self.open = None; self.glass()
            elif w == "sr-open":
                i = st.pop()
                if 0 <= i < len(self.packs()):
                    f = self.packs()[i]
                    if f != self.open: self.page = 0
                    self.open = f; self.glass()
            elif w == "sd-rm":
                i = st.pop()
                if 0 <= i < len(self.packs()):
                    f = self.packs()[i]; os.unlink(os.path.join(self.dir, f))
                    if f == self.open: self.open = None
                    return f"removed /fl/packs/{f}\n"
                return f"no pack {i}\n"
            elif w == "sr-auto!": self.auto = max(0, st.pop())
            elif w == "sr-sync!": self.sync = max(0, st.pop())
            elif w == "sr-sleep": self.sleep_asked = True
            elif w == "sd-recv": self.recv = {"stage": "header", "buf": b""}; return "\nREADY\n"
            else: return f"{w} ?\n"
        return ""

    def feed(self, data):
        """Bytes from the phone. Returns nothing; replies go on self.out."""
        if self.recv:
            r = self.recv
            r["buf"] += data
            if r["stage"] == "header" and b"\n" in r["buf"]:
                head, r["buf"] = r["buf"].split(b"\n", 1)
                name, size, crc = head.decode().split()
                r.update(stage="data", name=name, size=int(size), crc=int(crc, 16), t0=time.time())
                print(f"<- sd-recv {name} {size} bytes")
            if r["stage"] == "data" and len(r["buf"]) >= r["size"]:
                blob, rest = r["buf"][:r["size"]], r["buf"][r["size"]:]
                self.recv = None
                if zlib.crc32(blob) != r["crc"]:
                    self.say(f"ERR crc {zlib.crc32(blob):08x}, expected {r['crc']:08x}\n ok> ")
                else:
                    open(os.path.join(self.dir, r["name"]), "wb").write(blob)
                    dt = time.time() - r["t0"]
                    print(f"   got {r['name']}: {r['size'] / 1024:.1f} KB in {dt:.1f} s = {r['size'] / 1024 / max(dt, .01):.1f} KB/s, CRC ok")
                    self.say(f"OK {r['name']} {r['size']}\n ok> ")
                if rest: self.feed(rest)
            return
        self.buf += data
        while b"\r" in self.buf:
            line, self.buf = self.buf.split(b"\r", 1)
            line = line.decode("utf-8", "replace").strip()
            out = self.run(line)
            self.say(f"{line}\n{out}" + ("" if self.recv else " ok> "))

    def say(self, s):
        self.out.put_nowait(s.replace("\n", "\r\n").encode())


async def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("--board", default="sr16", choices=sorted(BOARDS))
    ap.add_argument("--dir", default="fake-pass", help="where its packs and glass.png live")
    ap.add_argument("--checkin", type=int, default=0, help="seconds between 5 s check-ins (0: always listening)")
    a = ap.parse_args()
    ps = Pass(a)
    server = BlessServer(name=ps.name)
    connected = asyncio.Event()

    def on_write(ch, value, **kw):
        connected.set()
        ps.feed(bytes(value))

    server.write_request_func = on_write
    server.read_request_func = lambda ch, **kw: ch.value
    await server.add_new_service(NUS)
    await server.add_new_characteristic(NUS, RX, P.write | P.write_without_response, None, A.writeable)
    await server.add_new_characteristic(NUS, TX, P.notify | P.read, None, A.readable)
    ps.glass()

    async def pump():
        while True:
            data = await ps.out.get()
            for i in range(0, len(data), 160):
                server.get_characteristic(TX).value = bytearray(data[i:i + 160])
                server.update_value(NUS, TX)
                await asyncio.sleep(0.03)

    asyncio.create_task(pump())
    await server.start()

    async def advertise(on):
        """Later wakes: only the advertising goes on and off (macOS refuses a second start())."""
        d = getattr(server, "peripheral_manager_delegate", None)
        if d is None:                                # not macOS: a full stop/start is fine
            await (server.start() if on else server.stop())
        elif on:
            await d.start_advertising({"kCBAdvDataLocalName": ps.name,
                                       "kCBAdvDataServiceUUIDs": [server.services[u].obj.UUID() for u in server.services]})
        else:
            await d.stop_advertising()

    first = True
    while True:
        if not first:
            await advertise(True)
        first = False
        print(f"{ps.name}: advertising" + (f" for 5 s (check-in every {a.checkin} s)" if a.checkin else ""))
        if a.checkin:
            try:
                await asyncio.wait_for(connected.wait(), 5)
            except asyncio.TimeoutError:
                await advertise(False)
                print(f"{ps.name}: asleep")
                await asyncio.sleep(a.checkin)
                continue
        while not ps.sleep_asked:                   # stay up while in use
            await asyncio.sleep(0.5)
        ps.sleep_asked = False
        connected.clear()
        await asyncio.sleep(0.5)
        await advertise(False)
        print(f"{ps.name}: told to sleep")
        await asyncio.sleep(a.checkin or 5)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        pass
