// Copyright (c) 2026 IoTone, Inc. MIT/X License — see LICENSE.txt
//
// RemoteView.ts — what goggl1 shows, picked in the air.
//
// The goggle's screens as true-pixel cards (its own renders, tools/
// hud_mock.py and goggle_frames.py). A pinch on a weather card sends
// `pulse-off n theme!` (0 Strip, 1 Glance, 2 Ribbon, weather-hud 0.8) and
// tells Pulse to stop streaming; a pinch on Pulse hands the glass back to the
// heart rate. On connect it asks `theme@ .` so the right card is lit.
// Selected card: Hi-vis outline; the rest dimmed.

import { Links } from "../ble/Links";
import { makeQuad, makeTappable, makeTexture, Z_BACK, Z_IMAGE } from "../ui/Bitmap";
import { placeInFront } from "../ui/Place";

const HIVIS = new vec4(1.0, 0.824, 0.247, 1);
const GLACIER = new vec4(0.471, 0.718, 0.831, 1);

interface Card { name: string; theme: number; tex: Texture; quad: SceneObject; ring: SceneObject; label: Text; }

@component
export class RemoteView extends BaseScriptComponent {

  @input links: Links;

  @input
  @hint("Unlit material with a base texture")
  imageMaterial: Material;

  @input stripTexture: Texture;
  @input glanceTexture: Texture;
  @input ribbonTexture: Texture;
  @input pulseTexture: Texture;

  @input
  @hint("A Text to copy for the card labels")
  labelText: Text;

  @input captionText: Text;

  @input
  @allowUndefined
  frame: SceneObject;

  @input
  @allowUndefined
  camera: SceneObject;

  @input distance: number = 85;
  @input drop: number = 6;
  @input
  @hint("Degrees to the side; negative is left")
  yaw: number = -32;

  @input
  @hint("Card width, cm (at 45 cm)")
  cardWidth: number = 7.6;

  private cards: Card[] = [];
  private current = -2;            // -1 Pulse, 0..2 theme, -2 unknown
  private asked = false;

  onAwake() {
    this.createEvent("OnStartEvent").bind(() => {
      this.build();
      this.light();
      this.links.after(() => {
        if (this.frame && this.camera) placeInFront(this.frame, this.camera, this.distance, this.drop, this.yaw);
      }, 0.3);
      this.links.onChange(() => this.maybeAsk());
      this.maybeAsk();
    });
  }

  // 2 x 2 cards, each the goggle's 128x64 glass at its own aspect
  private build() {
    const root = this.getSceneObject();
    const w = this.cardWidth, h = w / 2, gapX = 1.2, gapY = 1.9;
    const outline = makeTexture(1, 1, new Uint8Array([255, 210, 63, 255]));
    const defs: [string, number, Texture][] = [
      ["PULSE", -1, this.pulseTexture], ["STRIP", 0, this.stripTexture],
      ["GLANCE", 1, this.glanceTexture], ["RIBBON", 2, this.ribbonTexture],
    ];
    defs.forEach(([name, theme, tex], i) => {
      const cx = (i % 2 === 0 ? -1 : 1) * (w + gapX) / 2;
      const cy = (i < 2 ? 1 : -1) * (h + gapY) / 2 + 0.6;
      const ring = makeQuad(root, name + "Ring", w + 0.5, h + 0.5, this.imageMaterial, outline, 9);
      ring.getTransform().setLocalPosition(new vec3(cx, cy, Z_BACK));
      const quad = makeQuad(root, name, w, h, this.imageMaterial, tex);
      quad.getTransform().setLocalPosition(new vec3(cx, cy, Z_IMAGE));
      makeTappable(quad, w, h, () => this.pick(theme));
      const lo = global.scene.createSceneObject(name + "Label");
      lo.setParent(root);
      lo.getTransform().setLocalPosition(new vec3(cx, cy - h / 2 - 0.6, 0));
      const label = lo.createComponent("Component.Text") as Text;
      label.font = this.labelText.font;
      label.size = this.labelText.size;
      label.text = name;
      this.cards.push({ name, theme, tex, quad, ring, label });
    });
    this.labelText.getSceneObject().enabled = false;   // only a template
    print("[remote] " + this.cards.length + " cards");
  }

  private maybeAsk() {
    const g = this.links.goggle;
    if (!g.live) {
      this.caption("waiting for goggl1");
      return;
    }
    if (this.asked) return;
    this.asked = true;
    g.send("theme@ .").then((r) => {
      const n = parseInt(r.trim(), 10);
      this.current = this.links.goggleView === "pulse" && this.links.strap.state === "live" ? -1 : isNaN(n) ? -2 : n;
      this.light();
      this.caption("pinch a card: goggl1 shows it");
    }, (e) => { this.asked = false; print("[remote] " + e); });
  }

  private pick(theme: number) {
    const g = this.links.goggle;
    if (!g.live) {
      this.caption("goggl1 is not connected");
      return;
    }
    if (theme < 0) {
      this.links.goggleView = "pulse";        // Pulse streams again; goggl1 follows within a second
      this.current = -1;
    } else {
      this.links.goggleView = "weather";
      this.current = theme;
      g.send("pulse-off " + theme + " theme!").catch((e) => print("[remote] " + e));
    }
    this.light();
    this.caption("goggl1: " + (theme < 0 ? "PULSE" : this.cards[theme + 1].name));
  }

  private light() {
    for (const c of this.cards) {
      const on = c.theme === this.current;
      c.ring.enabled = on;
      const vis = c.quad.getComponent("Component.RenderMeshVisual") as RenderMeshVisual;
      vis.mainMaterial.mainPass.baseColor = on ? new vec4(1, 1, 1, 1) : new vec4(0.55, 0.55, 0.55, 1);
      c.label.textFill.color = on ? HIVIS : GLACIER;
    }
  }

  private caption(s: string) {
    if (!this.captionText) return;
    this.captionText.text = s.toUpperCase();
    this.captionText.textFill.color = GLACIER;
  }
}
