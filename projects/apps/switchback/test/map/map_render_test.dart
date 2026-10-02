import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:switchback/map/eink_canvas.dart';
import 'package:switchback/map/terrain_tiles.dart';

void main() {
  test('pack is 27 bytes a row, MSB first, 1 = black', () {
    final EinkCanvas c = EinkCanvas();
    c.set(0, 0, 1);
    c.set(7, 0, 1);
    c.set(211, 103, 1);
    final Uint8List p = c.pack();
    expect(p.length, 2808);
    expect(p[0], 0x81);
    expect(p[27 * 103 + 26], 0x10); // column 211 = byte 26 bit 3
  });

  test('text draws in the 5x7 font and reports its width', () {
    final EinkCanvas c = EinkCanvas(width: 60, height: 12);
    expect(c.text(1, 2, 'A'), 5);
    // 'A' = 7E 11 11 11 7E: column 0 rows 1..6 set, row 0 clear
    expect(c.get(1, 2), 0);
    expect(c.get(1, 3), 1);
    expect(c.get(5, 3), 1);
    expect(EinkCanvas.textWidth('FOREST', scale: 2), 70);
  });

  test('box for a track keeps the canvas aspect and contains the track', () {
    final List<(double, double)> track = <(double, double)>[(45.5363, -122.7122), (45.5440, -122.7290)];
    final GeoBox b = TerrainTiles.boxFor(track, 156, 104);
    expect(b.south < 45.5363 && b.north > 45.5440, isTrue);
    expect(b.west < -122.7290 && b.east > -122.7122, isTrue);
    expect((b.east - b.west) / (b.north - b.south), greaterThan(1.3)); // wider than tall in degrees at 45N
  });

  test('blit places a safe-area render at an offset', () {
    final EinkCanvas small = EinkCanvas(width: 10, height: 4)..set(0, 0, 1);
    final EinkCanvas full = EinkCanvas()..blit(small, 16, 0);
    expect(full.get(16, 0), 1);
    expect(full.get(15, 0), 0);
  });

  test('unpack is the inverse of pack', () {
    final EinkCanvas c = EinkCanvas();
    c.text(10, 10, 'ROLL', scale: 2);
    c.disc(100, 50, 7);
    final EinkCanvas back = EinkCanvas.unpack(c.pack());
    for (int y = 0; y < 104; y += 3) {
      for (int x = 0; x < 212; x += 3) {
        expect(back.get(x, y), c.get(x, y));
      }
    }
  });
}
