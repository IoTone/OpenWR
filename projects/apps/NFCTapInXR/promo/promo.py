"""NFCTapInXR promo: music bed + SFX cues mixed onto the picture.

  python3 promo.py                 # -> NFCTapInXR_Promo_1080p.mp4
Picture comes from renders/frames/f_####.png if render.py has written them,
else from renders/NFCTapInXR_silent.mp4. Sound from audio/ (music.py).
The mix is measured and set to -14 LUFS (YouTube's level), peaks under -1 dBTP.
"""
import os, re, subprocess, sys

HERE = os.path.dirname(os.path.abspath(__file__))
AUD = os.path.join(HERE, "audio")
REN = os.path.join(HERE, "renders")
OUT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "NFCTapInXR_Promo_1080p.mp4")
DUR, FPS = 30.0, 30
TARGET = -14.0

# ---- audio cue list (seconds, file, volume) ----
CUES = []
def cue(t, f, v=1.0):
    CUES.append((t, f, v))

cue(0.0, "Impact.wav", 0.8)      # title
cue(5.1, "Riser.wav", 0.45)      # into the glasses footage
cue(7.5, "Impact.wav", 0.4)      # the drop: drums come in
cue(8.2, "Tap.wav", 0.6)         # the tag meets the reader
cue(9.5, "Tap.wav", 0.7)         # tag 1: model
cue(13.5, "Tap.wav", 0.7)        # tag 2: fare
cue(17.5, "Tap.wav", 0.7)        # tag 3: sign-in
cue(22.0, "Tap.wav", 0.7)        # tag 4: card
cue(23.6, "Riser.wav", 0.35)     # into the outro
cue(26.0, "Impact.wav", 0.9)     # outro

def run(cmd):
    return subprocess.run(cmd, check=True, capture_output=True, text=True)

# 1. Mix bed + cues.
mix = os.path.join(AUD, "Mix.wav")
inputs = ["-i", os.path.join(AUD, "Bed.wav")]
filt = [f"[0:a]volume=0.8[m]"]
labels = ["[m]"]
for i, (t, f, v) in enumerate(CUES):
    inputs += ["-i", os.path.join(AUD, f)]
    ms = int(t * 1000)
    filt.append(f"[{i+1}:a]volume={v},adelay={ms}|{ms}[s{i}]")
    labels.append(f"[s{i}]")
filt.append("".join(labels) + f"amix=inputs={len(labels)}:normalize=0,atrim=0:{DUR}[a]")
run(["ffmpeg", "-y", "-loglevel", "error"] + inputs +
    ["-filter_complex", ";".join(filt), "-map", "[a]", "-ar", "48000", "-c:a", "pcm_s24le", mix])

# 2. Measure, then set the level with a limiter on the peaks.
r = run(["ffmpeg", "-hide_banner", "-i", mix, "-af", "ebur128", "-f", "null", "-"])
lufs = float(re.findall(r"I:\s+(-?[\d.]+) LUFS", r.stderr)[-1])
gain = TARGET - lufs
print(f"mix {lufs:.1f} LUFS -> gain {gain:+.1f} dB")

# 3. Picture + sound.
frames = os.path.join(REN, "frames", "f_0001.png")
if os.path.exists(frames):
    vin = ["-framerate", str(FPS), "-i", os.path.join(REN, "frames", "f_%04d.png")]
    vcodec = ["-c:v", "libx264", "-preset", "slow", "-crf", "17", "-pix_fmt", "yuv420p", "-profile:v", "high"]
else:
    vin = ["-i", os.path.join(REN, "NFCTapInXR_silent.mp4")]
    vcodec = ["-c:v", "copy"]
run(["ffmpeg", "-y", "-loglevel", "error"] + vin + ["-i", mix,
     "-filter_complex", f"[1:a]volume={gain}dB,alimiter=limit=0.84:level=false,afade=t=out:st={DUR-1.5}:d=1.5[a]",
     "-map", "0:v", "-map", "[a]"] + vcodec +
    ["-c:a", "aac", "-b:a", "192k", "-ar", "48000", "-t", str(DUR), "-movflags", "+faststart", OUT])

r = run(["ffmpeg", "-hide_banner", "-i", OUT, "-af", "ebur128=peak=true", "-f", "null", "-"])
i = re.findall(r"I:\s+(-?[\d.]+) LUFS", r.stderr)[-1]
pk = re.findall(r"Peak:\s+(-?[\d.]+) dBFS", r.stderr)[-1]
print(f"wrote {OUT}: {i} LUFS, true peak {pk} dBFS")
