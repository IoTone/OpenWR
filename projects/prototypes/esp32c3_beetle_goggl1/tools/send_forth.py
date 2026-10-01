#!/usr/bin/env python3
"""
send_forth.py — send Forth source files to the goggl1 REPL over USB, one
line at a time, waiting for the ok> prompt after each, and report any line
the engine answered with a '?' error.

    python3 tools/send_forth.py /dev/cu.usbmodem101 forth/weather-hud/*.fs

Needs pyserial. PlatformIO's own Python has it:
    ~/Library/Application\ Support/pipx/venvs/platformio/bin/python
"""
import sys, time, serial

def main():
    if len(sys.argv) < 3:
        print(__doc__); sys.exit(2)
    port, files = sys.argv[1], sys.argv[2:]
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

if __name__ == "__main__":
    main()
