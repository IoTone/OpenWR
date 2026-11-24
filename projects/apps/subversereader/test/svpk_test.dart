// The phone's pack builder must match tools/svpk.py byte for byte where it
// counts: PackBits round trip and the header layout the firmware reads.
import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:subversereader/pack/svpk.dart';

Uint8List unpackBits(Uint8List d, int size) {
  final List<int> out = <int>[];
  int i = 0;
  while (out.length < size && i < d.length) {
    final int c = d[i++];
    if (c < 128) {
      out.addAll(d.sublist(i, i + c + 1));
      i += c + 1;
    } else if (c > 128) {
      out.addAll(List<int>.filled(257 - c, d[i++]));
    }
  }
  return Uint8List.fromList(out);
}

void main() {
  test('PackBits round trip', () {
    final Uint8List data = Uint8List.fromList(<int>[
      ...List<int>.filled(300, 0),
      1, 2, 3, 4, 5,
      ...List<int>.filled(3, 9),
      ...List<int>.generate(200, (int i) => i & 0xFF),
      0xFF, 0xFF,
    ]);
    expect(unpackBits(packBits(data), data.length), data);
  });

  test('header layout', () {
    const PanelProfile p = PanelProfile.sr15;
    final Uint8List pack = buildPack(
      profile: p,
      pages: <PackPage>[PackPage(Uint8List(p.stride * p.height), image: false)],
      title: 'Test zine',
      author: 'me',
      sourceBytes: Uint8List(1),
    );
    final ByteData h = ByteData.sublistView(pack);
    expect(String.fromCharCodes(pack.sublist(0, 4)), 'SVPK');
    expect(h.getUint16(8, Endian.little), 300);
    expect(h.getUint16(10, Endian.little), 400);
    expect(h.getUint32(16, Endian.little), 1);
    expect(h.getUint32(20, Endian.little), 256);
    expect(String.fromCharCodes(pack.sublist(60, 69)), 'Test zine');
    expect(packFileName('Zine #5: Joe Blow!'), 'zine-5-joe-blow.svpk');
  });
}
