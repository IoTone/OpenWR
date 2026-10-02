// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// Elevation from AWS Terrain Tiles (Terrarium PNG, free, no key):
// elevation = (R * 256 + G + B / 256) - 32768 metres. Tiles are cached on
// disk by id, so a region you ride often is offline after the first time.
import 'dart:io';
import 'dart:math' as math;
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:path_provider/path_provider.dart';

/// A box on the ground, in degrees.
class GeoBox {
  const GeoBox({required this.south, required this.west, required this.north, required this.east});
  final double south, west, north, east;
  double get latSpan => north - south;
  double get lonSpan => east - west;
  double get centerLat => (north + south) / 2;
  double get centerLon => (east + west) / 2;
}

/// An elevation grid sampled to a canvas: [w] x [h] metres, plus the
/// box it covers and the ground distance one pixel spans.
class ElevationGrid {
  ElevationGrid({required this.w, required this.h, required this.z, required this.box, required this.metersPerPixel});
  final int w, h;
  final Float32List z;
  final GeoBox box;
  final double metersPerPixel;

  double at(int x, int y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= w) x = w - 1;
    if (y >= h) y = h - 1;
    return z[y * w + x];
  }

  double get min => z.reduce(math.min);
  double get max => z.reduce(math.max);

  /// Canvas pixel for a lat/lon, in Web Mercator inside the box.
  (double, double) project(double lat, double lon) {
    final double x0 = _mercX(box.west), x1 = _mercX(box.east), y0 = _mercY(box.north), y1 = _mercY(box.south);
    return ((_mercX(lon) - x0) / (x1 - x0) * w, (_mercY(lat) - y0) / (y1 - y0) * h);
  }
}

double _mercX(double lon) => (lon + 180) / 360;
double _mercY(double lat) {
  final double r = lat * math.pi / 180;
  return (1 - math.log(math.tan(r) + 1 / math.cos(r)) / math.pi) / 2;
}

class TerrainTiles {
  TerrainTiles({this.maxZoom = 14});
  final int maxZoom;
  static const String _base = 'https://s3.amazonaws.com/elevation-tiles-prod/terrarium';

  final Map<String, Float32List> _mem = <String, Float32List>{};
  Directory? _dir;

  Future<Directory> _cacheDir() async {
    if (_dir != null) return _dir!;
    final Directory d = Directory('${(await getApplicationSupportDirectory()).path}/terrain');
    if (!d.existsSync()) d.createSync(recursive: true);
    return _dir = d;
  }

  /// One tile as 256x256 metres, from memory, disk, or the network.
  Future<Float32List> tile(int z, int x, int y) async {
    final String key = '$z/$x/$y';
    final Float32List? m = _mem[key];
    if (m != null) return m;
    final File f = File('${(await _cacheDir()).path}/${z}_${x}_$y.png');
    Uint8List png;
    if (f.existsSync()) {
      png = await f.readAsBytes();
    } else {
      final HttpClient c = HttpClient()..connectionTimeout = const Duration(seconds: 10);
      try {
        final HttpClientRequest rq = await c.getUrl(Uri.parse('$_base/$z/$x/$y.png'));
        rq.headers.set('User-Agent', 'Switchback/1.0 (OpenWR maplet)');
        final HttpClientResponse rs = await rq.close();
        if (rs.statusCode != 200) throw HttpException('terrain tile $key: HTTP ${rs.statusCode}');
        final BytesBuilder bb = BytesBuilder(copy: false);
        await for (final List<int> chunk in rs) {
          bb.add(chunk);
        }
        png = bb.takeBytes();
        await f.writeAsBytes(png, flush: true);
      } finally {
        c.close();
      }
    }
    final Float32List z1 = await _decode(png);
    _mem[key] = z1;
    if (_mem.length > 48) _mem.remove(_mem.keys.first);
    return z1;
  }

  static Future<Float32List> _decode(Uint8List png) async {
    final ui.Codec codec = await ui.instantiateImageCodec(png);
    final ui.FrameInfo fi = await codec.getNextFrame();
    final ByteData? bd = await fi.image.toByteData(format: ui.ImageByteFormat.rawRgba);
    fi.image.dispose();
    if (bd == null) throw StateError('terrain tile: no pixels');
    final Float32List out = Float32List(256 * 256);
    for (int i = 0; i < 256 * 256; i++) {
      final int r = bd.getUint8(i * 4), g = bd.getUint8(i * 4 + 1), b = bd.getUint8(i * 4 + 2);
      out[i] = (r * 256 + g + b / 256) - 32768;
    }
    return out;
  }

  /// Sample [box] into a [w] x [h] grid. Zoom is the finest that keeps the
  /// box inside about a dozen tiles.
  Future<ElevationGrid> grid(GeoBox box, int w, int h) async {
    int z = maxZoom;
    while (z > 8) {
      final int n = 1 << z;
      final int tx0 = (_mercX(box.west) * n).floor(), tx1 = (_mercX(box.east) * n).floor();
      final int ty0 = (_mercY(box.north) * n).floor(), ty1 = (_mercY(box.south) * n).floor();
      if ((tx1 - tx0 + 1) * (ty1 - ty0 + 1) <= 12) break;
      z--;
    }
    final int n = 1 << z;
    final Map<String, Float32List> tiles = <String, Float32List>{};
    Future<double> elev(double fx, double fy) async {
      // fx, fy: fractional tile coordinates
      int tx = fx.floor(), ty = fy.floor();
      double px = (fx - tx) * 256, py = (fy - ty) * 256;
      final String k = '$tx/$ty';
      final Float32List t = tiles[k] ??= await tile(z, tx, ty);
      final int ix = px.floor().clamp(0, 255), iy = py.floor().clamp(0, 255);
      return t[iy * 256 + ix];
    }
    final Float32List out = Float32List(w * h);
    final double x0 = _mercX(box.west) * n, x1 = _mercX(box.east) * n;
    final double y0 = _mercY(box.north) * n, y1 = _mercY(box.south) * n;
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        out[j * w + i] = await elev(x0 + (i + 0.5) / w * (x1 - x0), y0 + (j + 0.5) / h * (y1 - y0));
      }
    }
    final double mpp = 156543.03 * math.cos(box.centerLat * math.pi / 180) / n * ((x1 - x0) * 256 / w);
    return ElevationGrid(w: w, h: h, z: out, box: box, metersPerPixel: mpp);
  }

  /// The box around a track, padded, widened to the canvas aspect.
  static GeoBox boxFor(List<(double, double)> track, int w, int h, {double pad = 0.10, double minSpanM = 600}) {
    double s = 90, n = -90, we = 180, e = -180;
    for (final (double lat, double lon) in track) {
      if (lat < s) s = lat;
      if (lat > n) n = lat;
      if (lon < we) we = lon;
      if (lon > e) e = lon;
    }
    final double cLat = (s + n) / 2, cLon = (we + e) / 2;
    // work in Mercator units so the aspect is right
    double bw = (_mercX(e) - _mercX(we)) * (1 + 2 * pad), bh = (_mercY(s) - _mercY(n)) * (1 + 2 * pad);
    final double minSpan = minSpanM / (40075016.686 * math.cos(cLat * math.pi / 180));
    if (bw < minSpan) bw = minSpan;
    if (bh < minSpan) bh = minSpan;
    if (bw / bh < w / h) {
      bw = bh * w / h;
    } else {
      bh = bw * h / w;
    }
    final double cx = _mercX(cLon), cy = _mercY(cLat);
    return GeoBox(
      west: _invX(cx - bw / 2), east: _invX(cx + bw / 2),
      north: _invY(cy - bh / 2), south: _invY(cy + bh / 2),
    );
  }

  static double _invX(double x) => x * 360 - 180;
  static double _invY(double y) => math.atan(_sinh(math.pi * (1 - 2 * y))) * 180 / math.pi;
  static double _sinh(double v) => (math.exp(v) - math.exp(-v)) / 2;
}
