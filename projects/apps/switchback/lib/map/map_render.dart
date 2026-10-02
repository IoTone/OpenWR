// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The Postcard renderer: hillshade as a dither, contours by marching
// squares, the track as a heavy line with a halo, a scale bar, a north
// arrow, a banner. The parameters are the ones that read well at one bit
// on the maplet's glass (design doc, "How the picture is made").
import 'dart:math' as math;
import 'dart:typed_data';

import 'eink_canvas.dart';
import 'terrain_tiles.dart';

class PostcardSpec {
  const PostcardSpec({required this.title, required this.stats, required this.track, this.planned = false});
  final String title; // banner, drawn at 2x; keep it short
  final String stats; // one line under the banner
  final List<(double, double)> track; // lat, lon
  final bool planned; // dashed route instead of a walked track
}

/// Field Notes: a notebook page before the session. Left, the notes;
/// right, the route as a dashed line with numbered waypoints.
class FieldNotesSpec {
  const FieldNotesSpec({
    required this.title, required this.subtitle, required this.rows, required this.track, this.waypoints = 5,
  });
  final String title; // 2x, short ("PLAN")
  final String subtitle; // two lines max, 1x ("WILDWOOD" / "> PITTOCK")
  final List<(String, String)> rows; // label, value: DIST 8.9km ...
  final List<(double, double)> track;
  final int waypoints;
}

/// Map where I am: contours around a point, a ring at a planned radius.
class HereSpec {
  const HereSpec({required this.lat, required this.lon, required this.radiusM, required this.title, required this.rows});
  final double lat, lon, radiusM;
  final String title;
  final List<(String, String)> rows;
}

class MapRenderer {
  MapRenderer({TerrainTiles? tiles}) : _tiles = tiles ?? TerrainTiles();
  final TerrainTiles _tiles;

  /// Render into a [w] x [h] window (the glass's safe area).
  Future<EinkCanvas> postcard(PostcardSpec spec, int w, int h) async {
    final EinkCanvas c = EinkCanvas(width: w, height: h);
    if (spec.track.length < 2) {
      c.text(6, 6, spec.title, scale: 2, inverted: true, pad: 3);
      c.text(6, 28, spec.stats);
      c.text(6, h - 14, 'NO TRACK');
      return c;
    }
    final GeoBox box = TerrainTiles.boxFor(spec.track, w, h);
    final ElevationGrid g = await _tiles.grid(box, w, h);
    final double zmin = g.min, zmax = g.max, relief = zmax - zmin;

    // hillshade, light from the west-southwest, relief x2
    final Float32List shade = _hillshade(g, 2.0 / g.metersPerPixel);
    _ditherShade(c, shade, 1.4);

    // contours: interval by relief
    final double interval = relief < 100 ? 10 : relief < 300 ? 25 : relief < 600 ? 50 : 100;
    _contours(c, g, interval, 4);

    // track, in canvas pixels
    final List<(int, int)> pts = spec.track.map(((double, double) p) {
      final (double x, double y) = g.project(p.$1, p.$2);
      return (x.round(), y.round());
    }).toList();
    if (spec.planned) {
      _route(c, pts);
    } else {
      _track(c, pts);
    }

    // furniture
    final int bar = (500 / g.metersPerPixel).round().clamp(10, w ~/ 2);
    final String barLabel = bar == (500 / g.metersPerPixel).round() ? '500m' : '${(bar * g.metersPerPixel).round()}m';
    _scalebar(c, 8, h - 8, bar);
    c.text(8 + bar + 4, h - 13, barLabel, pad: 1);
    _north(c, w - 12, 14);
    c.text(6, 5, spec.title, scale: 2, inverted: true, pad: 3);
    c.text(6, 26, spec.stats, pad: 2);
    return c;
  }

  Future<EinkCanvas> fieldNotes(FieldNotesSpec spec, int w, int h) async {
    final EinkCanvas c = EinkCanvas(width: w, height: h);
    final int col = (w * 0.34).round().clamp(56, 80); // notes column
    final int mx = col + 2, mw = w - mx;
    // notes
    c.text(4, 4, spec.title, scale: 2, pad: 0);
    final List<String> sub = spec.subtitle.split('\n');
    for (int i = 0; i < sub.length && i < 2; i++) {
      c.text(4, 22 + i * 9, _fit(sub[i], col - 8), pad: 0);
    }
    c.line(4, 42, col - 6, 42);
    _rows(c, spec.rows, col);
    c.line(col, 0, col, h);
    if (spec.track.length < 2) {
      c.text(mx + 6, h ~/ 2 - 3, 'NO ROUTE', pad: 0);
      return c;
    }
    // map
    final EinkCanvas m = EinkCanvas(width: mw, height: h);
    final GeoBox box = TerrainTiles.boxFor(spec.track, mw, h, pad: 0.12);
    final ElevationGrid g = await _tiles.grid(box, mw, h);
    final double relief = g.max - g.min;
    final double interval = relief < 100 ? 10 : relief < 300 ? 25 : relief < 600 ? 50 : 100;
    _contours(m, g, interval, 4);
    _gridDots(m, (1000 / g.metersPerPixel).round().clamp(20, 200));
    final List<(int, int)> pts = spec.track.map(((double, double) p) {
      final (double x, double y) = g.project(p.$1, p.$2);
      return (x.round(), y.round());
    }).toList();
    _route(m, pts);
    // waypoints: every 1/n of the route by distance, numbered
    final List<double> cum = <double>[0];
    for (int i = 1; i < spec.track.length; i++) {
      cum.add(cum.last + _haversine(spec.track[i - 1], spec.track[i]));
    }
    final double total = cum.last;
    for (int k = 0; k < spec.waypoints; k++) {
      final double target = total * k / spec.waypoints;
      int idx = 0;
      while (idx + 1 < cum.length && cum[idx + 1] <= target) {
        idx++;
      }
      final (int x, int y) = pts[idx];
      m.box(x - 5, y - 5, 11, 11, v: 0);
      m.frame(x - 5, y - 5, 11, 11);
      m.text(x - 2, y - 3, '${k + 1}', pad: 0);
    }
    final int bar = (500 / g.metersPerPixel).round().clamp(10, mw ~/ 2);
    _scalebar(m, 6, h - 8, bar);
    m.text(6 + bar + 4, h - 13, '${(bar * g.metersPerPixel).round()}m', pad: 1);
    _north(m, mw - 10, 12);
    c.blit(m, mx, 0);
    return c;
  }

  Future<EinkCanvas> here(HereSpec spec, int w, int h) async {
    final EinkCanvas c = EinkCanvas(width: w, height: h);
    final int col = (w * 0.34).round().clamp(56, 80);
    final int mx = col + 2, mw = w - mx;
    c.text(4, 4, spec.title, scale: 2, pad: 0);
    c.line(4, 42, col - 6, 42);
    _rows(c, spec.rows, col);
    c.line(col, 0, col, h);
    final EinkCanvas m = EinkCanvas(width: mw, height: h);
    // a box that holds the ring with a little room
    final double dLat = spec.radiusM / 111320 * 1.25;
    final double dLon = spec.radiusM / (111320 * math.cos(spec.lat * math.pi / 180)) * 1.25;
    final GeoBox box = TerrainTiles.boxFor(<(double, double)>[
      (spec.lat - dLat, spec.lon - dLon), (spec.lat + dLat, spec.lon + dLon),
    ], mw, h, pad: 0);
    final ElevationGrid g = await _tiles.grid(box, mw, h);
    final Float32List shade = _hillshade(g, 2.0 / g.metersPerPixel);
    _ditherShade(m, shade, 1.2);
    final double relief = g.max - g.min;
    final double interval = relief < 100 ? 10 : relief < 300 ? 25 : relief < 600 ? 50 : 100;
    _contours(m, g, interval, 4);
    final (double px, double py) = g.project(spec.lat, spec.lon);
    final int r = (spec.radiusM / g.metersPerPixel).round();
    _dashedRing(m, px.round(), py.round(), r);
    m.disc(px.round(), py.round(), 4, v: 0);
    m.disc(px.round(), py.round(), 2);
    final int bar = (1000 / g.metersPerPixel).round().clamp(10, mw ~/ 2);
    _scalebar(m, 6, h - 8, bar);
    m.text(6 + bar + 4, h - 13, '${(bar * g.metersPerPixel / 1000).toStringAsFixed(bar * g.metersPerPixel >= 950 ? 0 : 1)}km', pad: 1);
    _north(m, mw - 10, 12);
    c.blit(m, mx, 0);
    return c;
  }

  /// Label and value on one line, the label shortened to two letters when
  /// the column is narrow (a 156-column glass gives 8 characters).
  static void _rows(EinkCanvas c, List<(String, String)> rows, int col) {
    final int chars = (col - 6) ~/ 6;
    for (int i = 0; i < rows.length && i < 4; i++) {
      final String label = rows[i].$1, value = rows[i].$2;
      String line;
      if (label.length + 1 + value.length <= chars) {
        line = label.padRight(chars - value.length) + value;
      } else {
        final String l2 = label.length > 2 ? label.substring(0, 2) : label;
        line = l2.padRight(chars - value.length > l2.length ? chars - value.length : l2.length + 1) + value;
      }
      c.text(4, 47 + i * 12, _fit(line, col - 6), pad: 0);
    }
  }

  static String _fit(String s, int widthPx) {
    final int n = (widthPx + 1) ~/ 6;
    return s.length <= n ? s : s.substring(0, n);
  }

  static double _haversine((double, double) a, (double, double) b) {
    const double R = 6371000;
    final double p1 = a.$1 * math.pi / 180, p2 = b.$1 * math.pi / 180;
    final double dp = p2 - p1, dl = (b.$2 - a.$2) * math.pi / 180;
    final double x = math.sin(dp / 2) * math.sin(dp / 2) + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) * math.sin(dl / 2);
    return 2 * R * math.atan2(math.sqrt(x), math.sqrt(1 - x));
  }

  static void _gridDots(EinkCanvas c, int step) {
    for (int x = step; x < c.width; x += step) {
      for (int y = 0; y < c.height; y += 3) {
        c.set(x, y, 1);
      }
    }
    for (int y = step; y < c.height; y += step) {
      for (int x = 0; x < c.width; x += 3) {
        c.set(x, y, 1);
      }
    }
  }

  static void _dashedRing(EinkCanvas c, int cx, int cy, int r) {
    final int n = (2 * math.pi * r).round().clamp(24, 2000);
    for (int i = 0; i < n; i++) {
      if (i % 6 >= 3) continue;
      final double a = 2 * math.pi * i / n;
      c.set(cx + (r * math.cos(a)).round(), cy + (r * math.sin(a)).round(), 1);
      c.set(cx + ((r + 1) * math.cos(a)).round(), cy + ((r + 1) * math.sin(a)).round(), 1);
    }
  }

  static Float32List _hillshade(ElevationGrid g, double zscale) {
    const double lx = -0.78, ly = 0.50, lz = 0.38;
    final Float32List s = Float32List(g.w * g.h);
    for (int y = 0; y < g.h; y++) {
      for (int x = 0; x < g.w; x++) {
        final double dzdx = (g.at(x + 1, y) - g.at(x - 1, y)) * zscale / 2;
        final double dzdy = (g.at(x, y + 1) - g.at(x, y - 1)) * zscale / 2;
        final double nx = -dzdx, ny = -dzdy, nz = 1;
        final double nl = math.sqrt(nx * nx + ny * ny + nz * nz);
        s[y * g.w + x] = ((nx * lx + ny * ly + nz * lz) / nl + 1) / 2;
      }
    }
    return s;
  }

  /// Floyd-Steinberg on "darker than flat ground", capped so no area
  /// goes solid; ink lands only on slopes facing away from the light.
  static void _ditherShade(EinkCanvas c, Float32List shade, double gain) {
    const double sflat = (0.38 + 1) / 2;
    final int w = c.width, h = c.height;
    final Float32List buf = Float32List(w * h);
    for (int i = 0; i < w * h; i++) {
      double v = (sflat - shade[i]) * gain;
      if (v < 0) v = 0;
      if (v > 0.75) v = 0.75;
      buf[i] = v;
    }
    for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
        final int i = y * w + x;
        final double v = buf[i];
        final int out = v >= 0.5 ? 1 : 0;
        if (out == 1) c.set(x, y, 1);
        final double err = v - out;
        if (x + 1 < w) buf[i + 1] += err * 7 / 16;
        if (y + 1 < h) {
          if (x > 0) buf[i + w - 1] += err * 3 / 16;
          buf[i + w] += err * 5 / 16;
          if (x + 1 < w) buf[i + w + 1] += err * 1 / 16;
        }
      }
    }
  }

  static void _contours(EinkCanvas c, ElevationGrid g, double interval, int major) {
    final double zmin = g.min, zmax = g.max;
    final double lvlMin = (zmin / interval).floor() * interval, lvlMax = ((zmax / interval).floor() + 1) * interval;
    double lerp(double p, double q, double v) => q == p ? 0.5 : (v - p) / (q - p);
    for (double lvl = lvlMin; lvl <= lvlMax; lvl += interval) {
      final bool thick = major > 0 && ((lvl / interval).round() % major == 0);
      for (int y = 0; y < g.h - 1; y++) {
        for (int x = 0; x < g.w - 1; x++) {
          final double a = g.at(x, y), b = g.at(x + 1, y), cc = g.at(x + 1, y + 1), d = g.at(x, y + 1);
          int code = 0;
          if (a >= lvl) code |= 8;
          if (b >= lvl) code |= 4;
          if (cc >= lvl) code |= 2;
          if (d >= lvl) code |= 1;
          if (code == 0 || code == 15) continue;
          final (double, double) t = (x + lerp(a, b, lvl), y.toDouble());
          final (double, double) r = (x + 1.0, y + lerp(b, cc, lvl));
          final (double, double) bo = (x + lerp(d, cc, lvl), y + 1.0);
          final (double, double) l = (x.toDouble(), y + lerp(a, d, lvl));
          void draw((double, double) p, (double, double) q) {
            if (thick) {
              c.thick(p.$1.round(), p.$2.round(), q.$1.round(), q.$2.round(), 1);
            } else {
              c.line(p.$1.round(), p.$2.round(), q.$1.round(), q.$2.round());
            }
          }
          switch (code) {
            case 1: case 14: draw(l, bo);
            case 2: case 13: draw(bo, r);
            case 3: case 12: draw(l, r);
            case 4: case 11: draw(t, r);
            case 5: draw(t, l); draw(r, bo);
            case 6: case 9: draw(t, bo);
            case 7: case 8: draw(t, l);
            case 10: draw(t, r); draw(l, bo);
          }
        }
      }
    }
  }

  static void _track(EinkCanvas c, List<(int, int)> pts, {int r = 1}) {
    for (int i = 0; i + 1 < pts.length; i++) {
      c.thick(pts[i].$1, pts[i].$2, pts[i + 1].$1, pts[i + 1].$2, r + 2, v: 0); // halo
    }
    for (int i = 0; i + 1 < pts.length; i++) {
      c.thick(pts[i].$1, pts[i].$2, pts[i + 1].$1, pts[i + 1].$2, r);
    }
    final (int, int) s = pts.first, e = pts.last;
    c.disc(s.$1, s.$2, r + 3, v: 0);
    c.ring(s.$1, s.$2, r + 3);
    c.disc(e.$1, e.$2, r + 3, v: 0);
    c.disc(e.$1, e.$2, r + 1);
  }

  static void _route(EinkCanvas c, List<(int, int)> pts, {int r = 1, int dash = 3}) {
    for (int i = 0; i + 1 < pts.length; i++) {
      if (i % (2 * dash) < dash) c.thick(pts[i].$1, pts[i].$2, pts[i + 1].$1, pts[i + 1].$2, r);
    }
  }

  static void _scalebar(EinkCanvas c, int x, int y, int px) {
    c.box(x - 2, y - 4, px + 4, 9, v: 0);
    for (int i = 0; i <= px; i++) {
      c.set(x + i, y, 1);
      c.set(x + i, y + 1, 1);
    }
    for (int i = -3; i <= 3; i++) {
      c.set(x, y + i, 1);
      c.set(x + px, y + i, 1);
    }
  }

  static void _north(EinkCanvas c, int cx, int cy) {
    c.box(cx - 5, cy - 7, 11, 15, v: 0);
    c.line(cx, cy + 6, cx, cy - 6);
    c.line(cx, cy - 6, cx - 3, cy - 2);
    c.line(cx, cy - 6, cx + 3, cy - 2);
  }
}
