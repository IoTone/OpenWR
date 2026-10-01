#!/usr/bin/env python3
"""
send_forth.py — send Forth source files to the goggl1 REPL over USB, one
line at a time, waiting for the ok> prompt after each, and report any line
the engine answered with a '?' error.

    python3 tools/send_forth.py /dev/cu.usbmodem101 forth/weather-hud/*.fs

With --bundle the files are sent as one use-case bundle instead: wrapped in
bundle-begin / bundle-end, applied all-or-nothing, and stored in NVS so the
device re-applies it at every boot. Comment-only and blank lines are dropped
to save flash; the manifest comments at the top of the first file are kept.

    python3 tools/send_forth.py --bundle /dev/cu.usbmodem101 forth/weather-hud/*.fs

Needs pyserial. PlatformIO's own Python has it:
    ~/Library/Application\ Support/pipx/venvs/platformio/bin/python
"""
import sys, time, serial

def main():
    if len(sys.argv) < 3:
        print(__doc__); sys.exit(2)
    args = sys.argv[1:]
    bundle = "--bundle" in args
    args = [a for a in args if a != "--bundle"]
    port, files = args[0], args[1:]
    ser = serial.Serial(port, 115200, timeout=0.2)

    def until_prompt(timeout):
        buf, t0 = b"", time.time()
        while time.time() - t0 < timeout:
            chunk = ser.read(4096)
            if chunk:
                buf += chunk
                if buf.rstrip().endswith(b"ok>") or buf.rstrip().endswith(b"] "):
                    break
        return buf.decode("utf-8", "replace")

    ser.write(b"\r"); until_prompt(2)
    if bundle:
        send_bundle(ser, files, until_prompt)
        ser.close()
        return
    total_err = 0
    for f in files:
        errs = 0
        for raw in open(f):
            line = raw.rstrip("\n")
            if not line.strip() or line.strip().startswith("\\"):
                continue
            ser.write(line.encode() + b"\r")
            out = until_prompt(40)
            # a '?' at the start of a reply line is the engine's error marker
            if any(l.lstrip().startswith("?") for l in out.splitlines()):
                errs += 1
                print(f"!! {f}: {line}\n{out}")
        print(f"{f}: {'ok' if errs == 0 else str(errs) + ' errors'}")
        total_err += errs
    ser.write(b".s\r"); print(until_prompt(2).strip())
    ser.close()
    sys.exit(1 if total_err else 0)

def send_bundle(ser, files, until_prompt):
    lines = []
    for f in files:
        for raw in open(f):
            line = raw.rstrip("\n").rstrip()
            st = line.strip()
            if not st:
                continue
            if st.startswith("\\"):
                # keep manifest comments, drop the rest
                if st.startswith("\\ bundle:") or st.startswith("\\ requires:"):
                    lines.append(st)
                continue
            lines.append(line)
    text = "\n".join(lines)
    ser.write(b"bundle-begin\r"); time.sleep(0.3); ser.read(4096)
    for line in lines:
        ser.write(line.encode() + b"\r")
        time.sleep(0.02)                   # the capture path has no flow control
    ser.write(b"bundle-end\r")
    out = until_prompt(30)
    print(out.strip())
    print(f"{len(lines)} lines, {len(text)} bytes sent")
    sys.exit(0 if "applied" in out else 1)

if __name__ == "__main__":
    main()
