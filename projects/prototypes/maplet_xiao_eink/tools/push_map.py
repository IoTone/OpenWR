#!/usr/bin/env python3
"""
push_map.py — send a 212x104 1-bit picture to a maplet over USB and show it.

    python3 tools/push_map.py /dev/cu.usbmodem101 docs/images/plan1_postcard.png [slot]

Packs the image (dark = ink) as 27 bytes a row, MSB first, 1 = black, and
streams it as 36 lines of 78 bytes through epd-cursor! / epd-hex!, then
epd-show and, if a slot (0..3) is given, epd-save. Needs pyserial, and Pillow unless the input is a pre-packed .bin
(python3 -c 'import tools.push_map as m; open("x.bin","wb").write(m.pack("x.png"))').
"""
import sys, time

W, H, STRIDE = 212, 104, 27

def pack(path):
    if path.endswith(".bin"): return open(path, "rb").read()   # pre-packed 2808 bytes
    from PIL import Image
    im = Image.open(path).convert("L")
    if im.size != (W, H): im = im.resize((W, H), Image.NEAREST)
    px = im.load(); out = bytearray(STRIDE * H)
    for y in range(H):
        for x in range(W):
            if px[x, y] < 128: out[y * STRIDE + (x >> 3)] |= 0x80 >> (x & 7)
    return bytes(out)

def main():
    if len(sys.argv) < 3: print(__doc__); sys.exit(2)
    port, path = sys.argv[1], sys.argv[2]
    slot = int(sys.argv[3]) if len(sys.argv) > 3 else None
    data = pack(path)
    import serial
    ser = serial.Serial(port, 115200, timeout=0.2)
    def send(line, timeout=6):
        ser.write(line.encode() + b"\r"); out = b""; t = time.time()
        while time.time() - t < timeout:
            out += ser.read(4096)
            if out.rstrip().endswith(b"ok>"): break
        if b" ?" in out or b"? " in out: print("ERR", line[:40], out.decode("utf8", "replace").strip())
        return out
    ser.write(b"\r"); send("", 2)
    send("epd-clear"); send("0 epd-cursor!")
    t0 = time.time()
    for i in range(0, len(data), 78):
        send('s" ' + data[i:i+78].hex() + '" epd-hex!')
    print(f"{len(data)} bytes in {time.time()-t0:.1f}s")
    send("epd-show", 12)
    if slot is not None: send(f"{slot} epd-save")
    print(send("epd-slot@ .", 2).decode("utf8", "replace").strip())
    ser.close()

if __name__ == "__main__": main()
