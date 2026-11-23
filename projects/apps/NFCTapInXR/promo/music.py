"""NFCTapInXR promo, sound: a 30 s synthy downtempo bed in the spirit of an
80s-grid film score (detuned brass pads, a pulsing 16th bass, a delayed
arpeggio, half-time drums, a big reverb), plus the SFX that promo.py cues.

Run with Blender's Python (it has numpy):
  /Applications/Blender.app/Contents/Resources/5.0/python/bin/python3.11 music.py
Writes audio/Bed.wav, audio/Impact.wav, audio/Riser.wav, audio/Tap.wav.

96 BPM, so one 4/4 bar is exactly 2.5 s and the 12 bars line up with the edit:
  bar 1  (0-2.5)    title: pad swell
  bar 2-3 (2.5-7.5) tags, reader: bass enters, filter opening
  bar 4-10 (7.5-25) the glasses: drums, arpeggio, full pads
  bar 11 (25-27.5)  outro: drums drop, back to D minor
  bar 12 (27.5-30)  tail
"""
import os, wave
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
AUD = os.path.join(HERE, "audio")
os.makedirs(AUD, exist_ok=True)

SR = 48000
BPM = 96
BEAT = 60 / BPM            # 0.625 s
BAR = 4 * BEAT             # 2.5 s
DUR = 30.0
N = int(SR * (DUR + 3.5))  # room for the reverb tail; trimmed at the end
rng = np.random.default_rng(1982)

def hz(n):                 # MIDI note -> Hz
    return 440.0 * 2 ** ((n - 69) / 12)

def at(t):
    return int(round(t * SR))

def lp_gain(f, fc, order=2):
    return 1.0 / np.sqrt(1.0 + (f / fc) ** (2 * order))

def saw(f0, t, fc, detune_cents=0.0, phase=0.0):
    """Band-limited saw built from harmonics, each scaled by a low-pass
    response at cutoff fc (scalar or array over t), so the filter can sweep."""
    f0 = f0 * 2 ** (detune_cents / 1200)
    out = np.zeros_like(t)
    k = 1
    while k * f0 < 14000 and k <= 60:
        out += np.sin(2 * np.pi * k * f0 * t + phase * k) / k * lp_gain(k * f0, fc)
        k += 1
    return out * 0.6

def adsr(n, a, d, s, r, sustain_n=None):
    a, d, r = at(a), at(d), at(r)
    env = np.ones(n) * s
    env[:a] = np.linspace(0, 1, a, endpoint=False) if a else env[:a]
    env[a:a + d] = np.linspace(1, s, max(0, min(d, n - a)), endpoint=False)
    if r:
        env[-r:] *= np.linspace(1, 0, min(r, n))
    return env

def place(buf, sig, t0, pan=0.0):
    i = at(t0)
    j = min(i + len(sig), buf.shape[1])
    if j <= i: return
    l = np.cos((pan + 1) * np.pi / 4); r = np.sin((pan + 1) * np.pi / 4)
    buf[0, i:j] += sig[:j - i] * l
    buf[1, i:j] += sig[:j - i] * r

# ---- harmony: D minor, i - VI - III - VII ---------------------------------
Dm = (50, 57, 62, 65)          # D3 A3 D4 F4
Bb = (46, 53, 62, 65)          # Bb2 F3 D4 F4
F_ = (53, 57, 60, 65)          # F3 A3 C4 F4
C_ = (48, 55, 60, 64)          # C3 G3 C4 E4
A_ = (45, 52, 57, 61)          # A2 E3 A3 C#4
BARS = [Dm, Dm, Bb, F_, C_, Dm, Bb, F_, C_, A_, Dm, Dm]
ROOT = {Dm: 38, Bb: 34, F_: 41, C_: 36, A_: 33}   # bass roots (D2 Bb1 F2 C2 A1)

# Song-wide brightness: closed at the title, opening through the glasses.
def cutoff_curve(t):
    return np.interp(t, [0, 2.5, 7.5, 12, 22.5, 25, 27.5, 30], [500, 900, 1800, 2600, 3200, 1500, 900, 500])

def section_gain(t, pts):
    xs, ys = zip(*pts)
    return np.interp(t, xs, ys)

dry = np.zeros((2, N))
verb_send = np.zeros((2, N))

# ---- pads: three detuned saws per note, slow attack, legato across bars ----
for b, chord in enumerate(BARS):
    t0 = b * BAR
    n = at(BAR + 0.6)
    t = np.arange(n) / SR
    fc = cutoff_curve(t0 + t) * 0.9
    env = adsr(n, 0.35 if b else 1.6, 0.4, 0.85, 0.6)
    for i, note in enumerate(chord):
        f = hz(note)
        v = sum(saw(f, t, fc, dc, ph) for dc, ph in ((-9, 0.0), (0, 1.3), (8, 2.1)))
        pan = (-0.45, 0.45, -0.2, 0.25)[i]
        g = section_gain(t0 + t, [(0, 0.06), (2.5, 0.09), (7.0, 0.11), (7.5, 0.14), (25, 0.14), (27.5, 0.11), (30, 0.0)])
        sig = v * env * g * (0.5 if i == 0 else 1.0)       # thin the lowest voice
        place(dry, sig * 0.55, t0, pan)
        place(verb_send, sig * 0.8, t0, pan)

# ---- bass: pulsing 16ths, root / octave, from bar 2 ------------------------
SIXT = BEAT / 4
PATTERN = [0, 0, 12, 0, 0, 12, 0, 12, 0, 0, 12, 0, 0, 12, 7, 12]
for b, chord in enumerate(BARS):
    if b < 1 or b > 10: continue
    root = ROOT[chord]
    for s in range(16):
        t0 = b * BAR + s * SIXT
        n = at(SIXT * 0.95)
        t = np.arange(n) / SR
        note = root + PATTERN[s]
        fc = np.full(n, 180 + 0.55 * float(cutoff_curve(t0))) * (1.6 - 0.6 * t / (SIXT * 0.95))
        sig = saw(hz(note), t, fc) + 0.6 * np.sin(2 * np.pi * hz(root) * t)
        env = np.exp(-t * 9) * (1.0 if s % 4 == 0 else 0.78)
        env[:48] *= np.linspace(0, 1, 48)
        fade_in = min(1.0, (t0 - 2.5) / 3.0 + 0.25)          # creeps in over bars 2-3
        fade_out = 1.0 if b < 10 else max(0.0, 1 - (t0 - 25) / 2.5)
        place(dry, sig * env * 0.30 * fade_in * fade_out, t0, 0.0)

# ---- arpeggio: 8ths over the chord, an octave up, with a dotted-8th delay --
EIGHTH = BEAT / 2
arp = np.zeros((2, N))
for b, chord in enumerate(BARS):
    if b < 3 or b > 10: continue
    tones = [chord[1] + 12, chord[2] + 12, chord[3] + 12, chord[2] + 12]
    for s in range(8):
        t0 = b * BAR + s * EIGHTH
        n = at(EIGHTH * 1.5)
        t = np.arange(n) / SR
        f = hz(tones[s % 4] + (12 if s in (3, 7) else 0))
        sq = sum(np.sin(2 * np.pi * k * f * t) / k for k in range(1, 12, 2) if k * f < 12000)
        sig = sq * np.exp(-t * 6.5) * 0.06
        sig[:96] *= np.linspace(0, 1, 96)
        g = 0.6 if b == 10 else 1.0
        place(arp, sig * g, t0, (-0.35, 0.35)[s % 2])
DLY = at(BEAT * 0.75)                                     # dotted 8th, ping-pong
echo = np.zeros_like(arp)
for k, fb in enumerate((0.5, 0.28, 0.15)):
    shift = DLY * (k + 1)
    src = arp[(k + 1) % 2]
    echo[k % 2, shift:] += src[:-shift] * fb
arp_all = arp + echo
dry += arp_all * 0.9
verb_send += arp_all * 0.6

# ---- drums: half-time, from bar 4 to bar 10 --------------------------------
def kick():
    n = at(0.55); t = np.arange(n) / SR
    f = 45 + 85 * np.exp(-t * 28)
    ph = 2 * np.pi * np.cumsum(f) / SR
    return np.sin(ph) * np.exp(-t * 7) * 0.9

def snare():
    n = at(0.45); t = np.arange(n) / SR
    noise = rng.standard_normal(n)
    noise = noise - np.convolve(noise, np.ones(6) / 6, mode="same")      # bright
    body = np.sin(2 * np.pi * 190 * t) * np.exp(-t * 30)
    return (noise * np.exp(-t * 14) * 0.35 + body * 0.5) * 0.8

def hat(open_=False):
    n = at(0.25 if open_ else 0.06); t = np.arange(n) / SR
    noise = rng.standard_normal(n)
    noise = noise - np.convolve(noise, np.ones(3) / 3, mode="same")
    return noise * np.exp(-t * (18 if open_ else 80)) * 0.12

for b in range(3, 10):
    t0 = b * BAR
    for beat in (0, 1.5):                                  # 1 and the and-of-2
        place(dry, kick(), t0 + beat * BEAT)
    place(dry, snare(), t0 + 2 * BEAT, 0.05)
    place(verb_send, snare() * 1.4, t0 + 2 * BEAT)        # big gated-ish room
    for e in range(8):
        place(dry, hat(e == 7) * (1.0 if e % 2 else 0.6), t0 + e * EIGHTH, 0.3)
# a snare build into bar 4
for i, s in enumerate(np.arange(0, 1.0, BEAT / 4)):
    place(dry, snare() * (0.15 + 0.5 * i / 16), 7.5 - 1.0 + s, -0.05)
    place(verb_send, snare() * (0.1 + 0.4 * i / 16), 7.5 - 1.0 + s)

# ---- reverb: FFT convolution with a 3 s decaying stereo noise tail ----------
def reverb(x, secs=3.0, decay=2.4):
    m = at(secs); t = np.arange(m) / SR
    out = np.zeros_like(x)
    size = 1 << int(np.ceil(np.log2(x.shape[1] + m)))
    for c in range(2):
        ir = rng.standard_normal(m) * np.exp(-t * decay)
        ir = np.convolve(ir, np.ones(24) / 24, mode="same")             # darker
        ir[: at(0.02)] = 0                                                # pre-delay
        ir /= np.sqrt(np.sum(ir ** 2))
        y = np.fft.irfft(np.fft.rfft(x[c], size) * np.fft.rfft(ir, size), size)
        out[c] = y[: x.shape[1]]
    return out

def highpass(x, hz_=220):
    w = max(1, int(SR / hz_ / 2))
    k = np.ones(w) / w
    return x - np.stack([np.convolve(c, k, mode="same") for c in x])

mix = dry + reverb(highpass(verb_send)) * 0.55

def master(x, length):
    x = x[:, :at(length)]
    x = np.tanh(x * 0.8) / np.tanh(0.8)                    # gentle glue
    peak = np.max(np.abs(x)) or 1
    return x / peak * 0.89                                 # about -1 dBFS

def write(path, x):
    x = np.clip(x, -1, 1)
    data = (x.T * 32767).astype("<i2").tobytes()
    with wave.open(path, "wb") as w:
        w.setnchannels(2); w.setsampwidth(2); w.setframerate(SR); w.writeframes(data)

bed = master(mix, DUR)
fade = at(2.0)
bed[:, -fade:] *= np.linspace(1, 0, fade) ** 1.5
write(os.path.join(AUD, "Bed.wav"), bed)

# ---- SFX --------------------------------------------------------------------
def sfx(sig, tail=2.5, wet=0.5):
    x = np.zeros((2, len(sig) + at(tail)))
    x[0, :len(sig)] = sig; x[1, :len(sig)] = sig
    y = x + reverb(x, tail, 2.0) * wet
    return y / (np.max(np.abs(y)) or 1) * 0.89

# Impact: a sub drop with a noise crack.
n = at(2.2); t = np.arange(n) / SR
f = 32 + 70 * np.exp(-t * 6)
sub = np.sin(2 * np.pi * np.cumsum(f) / SR) * np.exp(-t * 1.8)
crack = rng.standard_normal(n) * np.exp(-t * 22) * 0.35
write(os.path.join(AUD, "Impact.wav"), sfx(sub + crack, 2.5, 0.35))

# Riser: noise and a rising tone that swell into the downbeat.
n = at(2.4); t = np.arange(n) / SR
f = 220 * 2 ** (t / 2.4 * 2.5)
tone = np.sin(2 * np.pi * np.cumsum(f) / SR) * 0.25
noise = rng.standard_normal(n)
noise = np.convolve(noise, np.ones(8) / 8, mode="same") * 0.5
sw = (t / 2.4) ** 2.2
write(os.path.join(AUD, "Riser.wav"), sfx((tone + noise) * sw, 1.5, 0.4))

# Tap: a two-note digital chirp (the reader "seeing" a tag).
def blip(fr, n):
    t = np.arange(n) / SR
    return np.sin(2 * np.pi * fr * t) * np.exp(-t * 26)
a = blip(1760, at(0.12)); b = blip(2637, at(0.18))
chirp = np.concatenate([a, np.zeros(at(0.03)), b]) * 0.6
write(os.path.join(AUD, "Tap.wav"), sfx(chirp, 1.2, 0.35))

print("wrote", sorted(os.listdir(AUD)))
