# NFCTapInXR promo

A 30 s, 1920x1080 explainer for YouTube: the tags, the reader, the four tap
scenes captured on Spectacles, and an end screen with the link to this code.
Laid out like the earlier Laser Duck promo: Blender cuts the picture, Python
makes the sound, ffmpeg puts them together.

Published: https://youtu.be/uXBH_5CcVdo

| File | Does |
|---|---|
| `render.py` | Blender 5.0 video-sequencer edit: captions, fades, the neon end screen -> `renders/frames/` and `NFCTapInXR_Promo.blend` |
| `music.py` | Synthesizes the music bed (synthy downtempo, D minor, 96 BPM so a bar is 2.5 s) and the SFX (impact, riser, tap chirp) -> `audio/` |
| `promo.py` | SFX cue list, mix, loudness to -14 LUFS, mux -> `NFCTapInXR_Promo_1080p.mp4` |
| `assets/` | Background and divider (rendered from the `.html` with headless Chrome), the lens icon, the tags photo, the wiring diagram |

## Build

```sh
BL=/Applications/Blender.app/Contents/MacOS/Blender
BPY=/Applications/Blender.app/Contents/Resources/5.0/python/bin/python3.11   # has numpy
$BL -b --factory-startup --python render.py           # picture (or: -- 50 340 for test stills)
$BPY music.py                                         # sound
python3 promo.py                                      # final MP4
```

## The Spectacles clips (not in git)

`assets/s1_tap.mp4` .. `s5_card.mp4` are cut from two Spectacles captures of
the lens running with the reader (clip A: connect, model, fare; clip B: sign-in,
card), brightened slightly:

```sh
cut() { ffmpeg -ss $2 -t $3 -i $1 -an -vf "fps=30,scale=1440:1080,eq=gamma=1.18:saturation=1.1" \
          -c:v libx264 -crf 16 -pix_fmt yuv420p assets/$4.mp4; }
cut clipA.mp4  1.4 3.0 s1_tap
cut clipA.mp4  5.2 4.0 s2_model
cut clipA.mp4 14.6 4.0 s3_fare
cut clipB.mp4  2.6 4.5 s4_signin
cut clipB.mp4 20.3 4.0 s5_card
```

## Edit points

- Captions and timings: `SECTIONS` in `render.py`; the end-screen link is `CODE_HOST` / `CODE_PATH`.
- Music: chords `BARS`, bass rhythm `PATTERN`, brightness over time `cutoff_curve` in `music.py`.
- SFX timing: the `cue(...)` lines in `promo.py`.
