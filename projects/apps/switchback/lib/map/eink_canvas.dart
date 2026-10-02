// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// A 1-bit canvas the size of the maplet's glass, with the same primitives
// the device has (pixel, line, box, frame, 5x7 text), packed the way
// epd-hex! wants it. Drawing happens in a window (the safe area) so a
// panel with dead columns still gets a complete picture.
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'font5x7.dart';

class EinkCanvas {
  EinkCanvas({this.width = 212, this.height = 104}) : _px = Uint8List(width * height);

  final int width;
  final int height;
  final Uint8List _px; // 1 = black

  int get(int x, int y) => (x < 0 || y < 0 || x >= width || y >= height) ? 0 : _px[y * width + x];

  void set(int x, int y, int v) {
    if (x < 0 || y < 0 || x >= width || y >= height) return;
    _px[y * width + x] = v;
  }

  void clear() => _px.fillRange(0, _px.length, 0);

  void line(int x0, int y0, int x1, int y1, {int v = 1}) {
    int dx = (x1 - x0).abs(), dy = -(y1 - y0).abs();
    final int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
      set(x0, y0, v);
      if (x0 == x1 && y0 == y1) break;
      final int e2 = 2 * err;
      if (e2 >= dy) {
        err += dy;
        x0 += sx;
      }
      if (e2 <= dx) {
        err += dx;
        y0 += sy;
      }
    }
  }

  /// A line of radius r (r = 1 gives 3 px).
  void thick(int x0, int y0, int x1, int y1, int r, {int v = 1}) {
    for (int i = -r; i <= r; i++) {
      for (int j = -r; j <= r; j++) {
        if (i * i + j * j <= r * r) line(x0 + i, y0 + j, x1 + i, y1 + j, v: v);
      }
    }
  }

  void box(int x, int y, int w, int h, {int v = 1}) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        set(x + i, y + j, v);
      }
    }
  }

  void frame(int x, int y, int w, int h) {
    for (int i = 0; i < w; i++) {
      set(x + i, y, 1);
      set(x + i, y + h - 1, 1);
    }
    for (int j = 0; j < h; j++) {
      set(x, y + j, 1);
      set(x + w - 1, y + j, 1);
    }
  }

  void disc(int cx, int cy, int r, {int v = 1}) {
    for (int i = -r; i <= r; i++) {
      for (int j = -r; j <= r; j++) {
        if (i * i + j * j <= r * r) set(cx + i, cy + j, v);
      }
    }
  }

  void ring(int cx, int cy, int r) {
    for (int i = -r - 1; i <= r + 1; i++) {
      for (int j = -r - 1; j <= r + 1; j++) {
        final int d = i * i + j * j;
        if (d <= r * r && d > (r - 2) * (r - 2)) set(cx + i, cy + j, 1);
      }
    }
  }

  /// 5x7 text at [scale]; returns the width drawn. [inverted] paints a
  /// black band and white glyphs. ASCII only, like the device.
  int text(int x, int y, String s, {int scale = 1, bool inverted = false, int pad = 2}) {
    final int w = s.length * 6 * scale - scale, h = 7 * scale;
    box(x - pad, y - pad, w + 2 * pad, h + 2 * pad, v: inverted ? 1 : 0);
    for (int i = 0; i < s.length; i++) {
      final int c = s.codeUnitAt(i);
      final List<int> g = (c >= 32 && c <= 126) ? font5x7[c - 32] : font5x7[0];
      for (int col = 0; col < 5; col++) {
        for (int row = 0; row < 7; row++) {
          if ((g[col] >> row) & 1 == 0) continue;
          for (int a = 0; a < scale; a++) {
            for (int b = 0; b < scale; b++) {
              set(x + i * 6 * scale + col * scale + a, y + row * scale + b, inverted ? 0 : 1);
            }
          }
        }
      }
    }
    return w;
  }

  static int textWidth(String s, {int scale = 1}) => s.length * 6 * scale - scale;

  /// Pack into the device frame: 27 bytes a row, MSB first, 1 = black.
  Uint8List pack() {
    final int stride = (width + 7) ~/ 8;
    final Uint8List out = Uint8List(stride * height);
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        if (_px[y * width + x] != 0) out[y * stride + (x >> 3)] |= 0x80 >> (x & 7);
      }
    }
    return out;
  }

  /// Blit another canvas (a safe-area render) into this one at (x0, y0).
  void blit(EinkCanvas src, int x0, int y0) {
    for (int y = 0; y < src.height; y++) {
      for (int x = 0; x < src.width; x++) {
        set(x0 + x, y0 + y, src._px[y * src.width + x]);
      }
    }
  }

  /// A phone-side preview: paper white and ink, one pixel per canvas pixel.
  Future<ui.Image> toImage({int paper = 0xFFECEBE4, int ink = 0xFF1A1A1A}) async {
    final Uint8List rgba = Uint8List(width * height * 4);
    for (int i = 0; i < width * height; i++) {
      final int c = _px[i] != 0 ? ink : paper;
      rgba[i * 4] = (c >> 16) & 0xff;
      rgba[i * 4 + 1] = (c >> 8) & 0xff;
      rgba[i * 4 + 2] = c & 0xff;
      rgba[i * 4 + 3] = 0xff;
    }
    final ui.ImmutableBuffer buf = await ui.ImmutableBuffer.fromUint8List(rgba);
    final ui.ImageDescriptor desc = ui.ImageDescriptor.raw(buf, width: width, height: height, pixelFormat: ui.PixelFormat.rgba8888);
    final ui.Codec codec = await desc.instantiateCodec();
    final ui.FrameInfo fi = await codec.getNextFrame();
    return fi.image;
  }
}
