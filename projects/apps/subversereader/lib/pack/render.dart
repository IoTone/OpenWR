// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// Zine pages to 1-bit pages for one panel, the same rules as tools/svpk.py:
// crop plain white margins, fit inside the panel, then a threshold of 215 for
// text and line art (keeps small grey type) or Floyd-Steinberg for pages that
// are mostly mid-grey (photos).
import 'dart:typed_data';

import 'package:archive/archive.dart';
import 'package:image/image.dart' as img;
import 'package:pdfrx/pdfrx.dart';

import 'svpk.dart';

const int kThreshold = 215;
const double kPhotoMidGrey = 0.12;

/// Greyscale pages, already fitted to the profile.
Future<List<img.Image>> renderSource(Uint8List data, String kind, PanelProfile p,
    {void Function(int done, int total)? onPage}) async {
  if (kind == 'cbz') return _renderCbz(data, p, onPage);
  return _renderPdf(data, p, onPage);
}

Future<List<img.Image>> _renderPdf(Uint8List data, PanelProfile p, void Function(int, int)? onPage) async {
  await pdfrxFlutterInitialize();
  final PdfDocument doc = await PdfDocument.openData(data);
  final List<img.Image> out = <img.Image>[];
  try {
    final int n = doc.pages.length;
    for (int i = 0; i < n; i++) {
      final PdfPage page = doc.pages[i];
      // render at twice the size it will end up, then downsample: smoother text
      final double s = 2 * ((p.width / page.width) < (p.height / page.height) ? p.width / page.width : p.height / page.height);
      final int w = (page.width * s).round(), h = (page.height * s).round();
      final PdfImage? pi = await page.render(width: w, height: h, fullWidth: w.toDouble(), fullHeight: h.toDouble(), backgroundColor: 0xFFFFFFFF);
      if (pi == null) continue;
      final img.Image grey = _bgraToGrey(pi.pixels, pi.width, pi.height);
      pi.dispose();
      out.add(fitPage(grey, p));
      onPage?.call(i + 1, n);
    }
  } finally {
    await doc.dispose();
  }
  return out;
}

Future<List<img.Image>> _renderCbz(Uint8List data, PanelProfile p, void Function(int, int)? onPage) async {
  final Archive z = ZipDecoder().decodeBytes(data);
  final List<ArchiveFile> files = z.files
      .where((ArchiveFile f) => f.isFile && RegExp(r'\.(png|jpe?g|webp|gif)$', caseSensitive: false).hasMatch(f.name))
      .toList()
    ..sort((ArchiveFile a, ArchiveFile b) => a.name.toLowerCase().compareTo(b.name.toLowerCase()));
  final List<img.Image> out = <img.Image>[];
  for (int i = 0; i < files.length; i++) {
    final img.Image? im = img.decodeImage(files[i].content);
    if (im == null) continue;
    out.add(fitPage(img.grayscale(im).convert(numChannels: 1), p));
    onPage?.call(i + 1, files.length);
  }
  return out;
}

img.Image _bgraToGrey(Uint8List bgra, int w, int h) {
  final img.Image g = img.Image(width: w, height: h, numChannels: 1);
  final Uint8List o = g.buffer.asUint8List();
  for (int i = 0, j = 0; j < w * h; i += 4, j++) {
    o[j] = (bgra[i + 2] * 299 + bgra[i + 1] * 587 + bgra[i] * 114) ~/ 1000;
  }
  return g;
}

int _lum(img.Image g, int x, int y) => g.getPixel(x, y).r.toInt();

/// Crop white margins (6 px kept), scale to fit, centre on white.
img.Image fitPage(img.Image g, PanelProfile p) {
  int l = g.width, t = g.height, r = -1, b = -1;
  for (int y = 0; y < g.height; y++) {
    for (int x = 0; x < g.width; x++) {
      if (_lum(g, x, y) < 231) {
        if (x < l) l = x;
        if (x > r) r = x;
        if (y < t) t = y;
        if (y > b) b = y;
      }
    }
  }
  img.Image src = g;
  if (r >= 0) {
    const int pad = 6;
    l = (l - pad).clamp(0, g.width - 1);
    t = (t - pad).clamp(0, g.height - 1);
    r = (r + pad).clamp(0, g.width - 1);
    b = (b + pad).clamp(0, g.height - 1);
    src = img.copyCrop(g, x: l, y: t, width: r - l + 1, height: b - t + 1);
  }
  final double s = (p.width / src.width) < (p.height / src.height) ? p.width / src.width : p.height / src.height;
  final img.Image fitted = img.copyResize(src,
      width: (src.width * s).round().clamp(1, p.width),
      height: (src.height * s).round().clamp(1, p.height),
      interpolation: img.Interpolation.average);
  final img.Image page = img.Image(width: p.width, height: p.height, numChannels: 1);
  img.fill(page, color: img.ColorUint8.rgb(255, 255, 255));
  img.compositeImage(page, fitted, dstX: (p.width - fitted.width) ~/ 2, dstY: (p.height - fitted.height) ~/ 2);
  return page;
}

/// Greyscale page to 1-bit bits (1 = black), choosing threshold or dither.
PackPage toOneBit(img.Image page, PanelProfile p) {
  final int w = p.width, h = p.height;
  final Float32List v = Float32List(w * h);
  int mid = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      final int l = _lum(page, x, y);
      v[y * w + x] = l.toDouble();
      if (l >= 48 && l < 208) mid++;
    }
  }
  final bool photo = mid / (w * h) > kPhotoMidGrey;
  final Uint8List bits = Uint8List(p.stride * h);
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      final int i = y * w + x;
      final double old = v[i];
      final bool black = photo ? old < 128 : old <= kThreshold;
      if (black) bits[y * p.stride + (x >> 3)] |= 0x80 >> (x & 7);
      if (photo) {
        final double err = old - (black ? 0 : 255);
        if (x + 1 < w) v[i + 1] += err * 7 / 16;
        if (y + 1 < h) {
          if (x > 0) v[i + w - 1] += err * 3 / 16;
          v[i + w] += err * 5 / 16;
          if (x + 1 < w) v[i + w + 1] += err * 1 / 16;
        }
      }
    }
  }
  return PackPage(bits, image: photo);
}

/// 1-bit page back to an image, for the on-phone preview.
img.Image previewOf(PackPage pg, PanelProfile p) {
  final img.Image out = img.Image(width: p.width, height: p.height, numChannels: 1);
  for (int y = 0; y < p.height; y++) {
    for (int x = 0; x < p.width; x++) {
      final bool black = (pg.bits[y * p.stride + (x >> 3)] & (0x80 >> (x & 7))) != 0;
      out.setPixelRgb(x, y, black ? 0 : 255, black ? 0 : 255, black ? 0 : 255);
    }
  }
  return out;
}
