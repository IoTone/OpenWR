#!/usr/bin/env python3
"""
sr_push.py: copy .svpk packs onto a pass (its microSD card, or its flash on SR-16) over its
USB-C console.

    sr_push.py packs/*.svpk                  # port auto-detected (CH340, or a XIAO's own USB)
    sr_push.py --port /dev/cu.usbserial-110 zine.svpk
    sr_push.py --opds https://spicemine.iotj.cc/opds   # build from a catalog and push

The pass must be awake (press a key, or its reset button) and running the pass firmware.
With --opds the packs are built for the panel the pass says it has. Each
file goes over with a CRC and lands only if it arrived whole. At 115200 baud
that is about 11 KB/s: a 40 KB zine takes 4 s. Needs pyserial.
"""
import argparse, glob, os, subprocess, sys, tempfile, time, zlib

import serial


def find_port():
    pats = ["/dev/cu.usbserial-*", "/dev/cu.wchusbserial*", "/dev/cu.usbmodem*", "/dev/ttyUSB*", "/dev/ttyACM*"]
    ports = [p for pat in pats for p in sorted(glob.glob(pat))]
    if not ports:
        sys.exit("sr_push: no USB serial port found; plug the pass in, or give --port")
    return ports[0]


def wait_for(s, words, timeout):
    """Read lines until one starts with any of words; return it."""
    end, buf = time.time() + timeout, b""
    while time.time() < end:
        buf += s.read(256)
        while b"\n" in buf:
            line, buf = buf.split(b"\n", 1)
            t = line.decode("utf-8", "replace").strip()
            if any(t.startswith(w) for w in words):
                return t
    return None


def ask_profile(s):
    """The pass's panel profile from sr-info ("pass sr16 240x416 ..."), or None (old firmware)."""
    s.reset_input_buffer()
    s.write(b"sr-info\r")
    r = wait_for(s, ["pass "], 3)
    return r.split()[1] if r else None


def push(s, path):
    data = open(path, "rb").read()
    name = os.path.basename(path).replace(" ", "_")
    s.reset_input_buffer()
    s.write(b"sd-recv\r")
    r = wait_for(s, ["READY", "ERR"], 10)
    if r != "READY":
        sys.exit(f"sr_push: the pass didn't answer ({r or 'nothing'}). Is it awake?")
    s.write(f"{name} {len(data)} {zlib.crc32(data):08x}\n".encode())
    t0 = time.time()
    for i in range(0, len(data), 1024):
        s.write(data[i:i + 1024])
    r = wait_for(s, ["OK", "ERR"], 30 + len(data) / 5000)
    dt = time.time() - t0
    print(f"  {name}: {len(data) / 1024:.1f} KB in {dt:.1f} s  {r}")
    return bool(r and r.startswith("OK"))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("packs", nargs="*")
    ap.add_argument("--port")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--opds", help="build every publication in this OPDS feed first (svpk.py build)")
    ap.add_argument("--profile", help="panel profile for --opds (default: ask the pass; sr15 if it doesn't say)")
    a = ap.parse_args()
    packs = list(a.packs)
    if not packs and not a.opds:
        ap.error("no packs given")
    port = a.port or find_port()
    s = serial.Serial(port, a.baud, timeout=0.1)
    s.dtr = False
    s.rts = False                     # don't reset the board on open
    time.sleep(0.2)
    s.write(b"\r")
    if a.opds:
        profile = a.profile or ask_profile(s) or "sr15"
        out = tempfile.mkdtemp(prefix="svpk-")
        print(f"building for {profile}")
        subprocess.run([sys.executable, os.path.join(os.path.dirname(__file__), "svpk.py"), "build", a.opds,
                        "-o", out, "-p", profile], check=True)
        packs += sorted(glob.glob(os.path.join(out, "*.svpk")))
    print(f"pushing {len(packs)} pack(s) to {port}")
    ok = all(push(s, p) for p in packs)
    s.write(b"sr-shelf\r")
    time.sleep(0.5)
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
