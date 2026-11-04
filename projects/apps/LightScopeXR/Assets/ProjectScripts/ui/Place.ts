// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Place.ts — put a panel in front of the wearer when the lens starts.
//
// World origin is the head pose at lens start on the glasses but not in the
// editor preview, so panels are placed relative to the camera: level, at a
// distance, turned yawDeg to the side (negative is left), a little low, and
// facing the eyes. Layouts are designed in cm at 45 cm; further away they are
// scaled by distance / 45 so they subtend the same angle without the eyes
// converging on something close (nearer than ~60 cm is hard to focus on).

export const DESIGN_DISTANCE = 45;

export function placeInFront(frame: SceneObject, camera: SceneObject, distance: number, dropAt45: number, yawDeg = 0): void {
  const cam = camera.getTransform();
  const fwd = cam.back;                        // a camera looks down its -Z
  const flat = new vec3(fwd.x, 0, fwd.z);
  let dir = flat.length > 0.01 ? flat.normalize() : new vec3(0, 0, -1);
  if (yawDeg !== 0) dir = quat.angleAxis(-yawDeg * Math.PI / 180, vec3.up()).multiplyVec3(dir);
  const k = distance / DESIGN_DISTANCE;
  const pos = cam.getWorldPosition().add(dir.uniformScale(distance)).add(new vec3(0, -dropAt45 * k, 0));
  const tr = frame.getTransform();
  tr.setWorldPosition(pos);
  tr.setWorldRotation(quat.lookAt(dir.uniformScale(-1), vec3.up()));
  tr.setWorldScale(new vec3(k, k, k));
}
