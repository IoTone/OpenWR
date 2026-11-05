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

// ---- the UI's click and fail sounds ---------------------------------------
// LiveView registers the clips at start; every makeTappable() tap clicks, and
// a shot or send that failed plays the fail sound (at most once in 1.5 s, so
// one failure reported in two places sounds once).

let ui: { owner: SceneObject; click: AudioTrackAsset; fail: AudioTrackAsset } = null;
let failAt = 0;

export function setUiSounds(owner: SceneObject, click: AudioTrackAsset, fail: AudioTrackAsset) {
  ui = { owner, click, fail };
}

export function uiClick() {
  if (ui) playOnce(ui.owner, ui.click);
}

export function uiFail() {
  const now = Date.now();
  if (!ui || now - failAt < 1500) return;
  failAt = now;
  playOnce(ui.owner, ui.fail);
}
