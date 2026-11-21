// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// Bitmap.ts — 1-bit device frames on the glasses.
//
// goggl1 and the maplet speak packed 1-bit bitmaps: MSB first, rows of
// ceil(w/8) bytes. The maplet's frames use 1 = ink on white paper; the glass
// shows light, so paper becomes a colour and ink becomes clear glass. Frames
// go into procedural textures on flat quads; quads can be made pinchable.

import { Interactable } from "SpectaclesInteractionKit.lspkg/Components/Interaction/Interactable/Interactable";

// Known frame sizes by byte count: maplet 212x104 (2.13in) and 296x128 (2.9in).
export function frameSize(bytes: number): [number, number] | null {
  if (bytes === 2808) return [212, 104];
  if (bytes === 4736) return [296, 128];
  if (bytes === 1024) return [128, 64];
  return null;
}

export function hexToBytes(hex: string): Uint8Array {
  const out = new Uint8Array(hex.length >> 1);
  for (let i = 0; i < out.length; i++) out[i] = parseInt(hex.substr(i * 2, 2), 16);
  return out;
}

// Lines of base64 (the maplet's roll-b64), joined; padding only at the end.
export function b64ToBytes(b64: string): Uint8Array {
  const A = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  const s = b64.replace(/=+$/, ""), out = new Uint8Array(Math.floor(s.length * 3 / 4));
  let v = 0, bits = 0, o = 0;
  for (let i = 0; i < s.length; i++) {
    v = (v << 6) | A.indexOf(s[i]); bits += 6;
    if (bits >= 8) { bits -= 8; out[o++] = (v >> bits) & 255; }
  }
  return out;
}

// RGBA pixels, bottom row first (procedural textures start at the bottom left).
// on = colour where the bit is 1, off = colour where it is 0.
export function unpack(frame: Uint8Array, w: number, h: number, on: number[], off: number[]): Uint8Array {
  const stride = (w + 7) >> 3;
  const px = new Uint8Array(w * h * 4);
  for (let y = 0; y < h; y++) {
    const dst = (h - 1 - y) * w * 4;
    for (let x = 0; x < w; x++) {
      const bit = (frame[y * stride + (x >> 3)] >> (7 - (x & 7))) & 1;
      const c = bit ? on : off;
      const o = dst + x * 4;
      px[o] = c[0]; px[o + 1] = c[1]; px[o + 2] = c[2]; px[o + 3] = c[3];
    }
  }
  return px;
}

export function makeTexture(w: number, h: number, rgba: Uint8Array): Texture {
  const tex = ProceduralTextureProvider.createWithFormat(w, h, TextureFormat.RGBA8Unorm);
  (tex.control as ProceduralTextureProvider).setPixels(0, 0, w, h, rgba);
  return tex;
}

// In front of the ContainerFrame's glass, which sits on the content plane
// (z = 0) and would z-fight a quad placed there. Positions use these z values;
// draw order follows, so the outline goes first, then the image.
export const Z_BACK = 0.3;
export const Z_IMAGE = 0.4;
const ORDER_IMAGE = 10;

// A w x h cm quad facing +Z, UVs 0..1, centred on its object.
export function makeQuad(parent: SceneObject, name: string, w: number, h: number, material: Material, tex: Texture, order = ORDER_IMAGE): SceneObject {
  const b = new MeshBuilder([
    { name: "position", components: 3 },
    { name: "texture0", components: 2 },
  ]);
  b.topology = MeshTopology.Triangles;
  b.indexType = MeshIndexType.UInt16;
  const x = w / 2, y = h / 2;
  b.appendVerticesInterleaved([-x, -y, 0, 0, 0, x, -y, 0, 1, 0, x, y, 0, 1, 1, -x, y, 0, 0, 1]);
  b.appendIndices([0, 1, 2, 0, 2, 3]);
  b.updateMesh();
  const o = global.scene.createSceneObject(name);
  o.setParent(parent);
  const vis = o.createComponent("Component.RenderMeshVisual") as RenderMeshVisual;
  vis.mesh = b.getMesh();
  const mat = material.clone();
  mat.mainPass.baseTex = tex;
  vis.mainMaterial = mat;
  vis.setRenderOrder(order);
  return o;
}

export function setQuadTexture(quad: SceneObject, tex: Texture) {
  const vis = quad.getComponent("Component.RenderMeshVisual") as RenderMeshVisual;
  vis.mainMaterial.mainPass.baseTex = tex;
}

// Pinch (or poke) the quad to call onTap. SIK needs a collider to target.
export function makeTappable(quad: SceneObject, w: number, h: number, onTap: () => void): Interactable {
  const col = quad.createComponent("Physics.ColliderComponent") as ColliderComponent;
  const shape = Shape.createBoxShape();
  shape.size = new vec3(w, h, 0.4);
  col.shape = shape;
  const it = quad.createComponent(Interactable.getTypeName()) as Interactable;
  it.onTriggerEnd.add(() => onTap());
  return it;
}
