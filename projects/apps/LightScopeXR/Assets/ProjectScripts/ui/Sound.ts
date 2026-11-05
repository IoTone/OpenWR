// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Sound.ts — play a short clip (a shutter) once, from a component made on
// first use. A clip that is still playing starts again from the top.

const players: { [name: string]: AudioComponent } = {};

export function playOnce(owner: SceneObject, track: AudioTrackAsset) {
  if (!owner || !track) return;
  let a = players[track.name];
  if (!a) {
    a = owner.createComponent("Component.AudioComponent") as AudioComponent;
    a.audioTrack = track;
    players[track.name] = a;
  }
  try {
    if (a.isPlaying()) a.stop(false);
    a.play(1);
  } catch (e) {
    print("[sound] " + track.name + ": " + e);
  }
}
