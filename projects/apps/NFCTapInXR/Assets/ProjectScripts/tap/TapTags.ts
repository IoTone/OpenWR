// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// TapTags.ts — WHICH TAG PLAYS WHICH SCENE. Edit this file for your tags.
//
// Every NFC tag has a fixed ID (its UID, 4, 7 or 10 bytes). The reader sends
// it as "TAP <id hex>", for example "TAP 04A2B3C4D5E680". Put each of your
// tags' IDs into the list of the scene it should play. A scene can have
// several tags (one per demo kit, say).
//
// How to find a tag's ID (any of these):
//   - Tap it on the reader with the lens running: the caption under the stage
//     shows "UNKNOWN TAG <id>" (or "TAG <id>"), and the Lens Studio / Spectacles
//     log prints "[tapin] tap <id>".
//   - Tap it on the reader with the ATOM on USB: the serial console at
//     115200 prints "TAP <id>".
//   - Read it with a phone app such as NFC Tools (wakdev, iOS and Android):
//     "Serial number", shown as 04:A2:B3:C4:D5:E6:80.
// Any of these spellings works here: colons, spaces, dashes and case are
// ignored, so "04:a2:b3:c4:d5:e6:80" and "04A2B3C4D5E680" are the same tag.
//
// Scenes with an empty list learn instead: the first unknown tag tapped is
// bound to the first scene that still has no tag, and is remembered on the
// glasses until RESET is pinched. Fill all four lists for a fixed demo.
//
// The four tag inputs on the TapIn object (cardTag, signInTag, modelTag,
// fareTag) add to these lists, so a tag can also be set from the Inspector
// without editing code.

export const TAG_SCENES: string[][] = [
  // 0 CARD: the trading card you can pinch and hold
  [
    // TODO: your card tag, e.g. "04:A2:B3:C4:D5:E6:80",
  ],

  // 1 SIGN-IN: "Signing in as Iron Man", then "Session secured"
  [
    // TODO: your sign-in tag,
  ],

  // 2 MODEL: a 3D model materializing in space
  [
    // TODO: your model tag,
  ],

  // 3 FARE: a transit fare taken from a counting-down card balance
  [
    // TODO: your fare tag,
  ],
];

// "04:a2 b3-C4" -> "04A2B3C4". Anything that is not a hex digit is dropped.
export function normalizeTagId(s: string): string {
  return (s || "").replace(/[^0-9A-Fa-f]/g, "").toUpperCase();
}
