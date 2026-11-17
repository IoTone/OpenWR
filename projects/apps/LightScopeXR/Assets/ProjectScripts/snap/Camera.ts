// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Camera.ts — the glasses' colour camera, asked for once and shared (the
// Viewfinder's 1-bit shots and HubFollow's colour shots for ReelHub).

const cameraModule = require("LensStudio:CameraModule") as CameraModule;
let tex: Texture = null;

export function glassesCamera(): Texture {
  if (tex) return tex;
  const req = CameraModule.createCameraRequest();
  req.cameraId = CameraModule.CameraId.Default_Color;
  req.imageSmallerDimension = 480;
  tex = cameraModule.requestCamera(req);
  print("[camera] requested");
  return tex;
}
