import 'dart:io';
import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:switchback/goggle/console.dart';
import 'package:switchback/goggle/mnpkg.dart';

void main() {
  test('every embedded release is signed with the OpenWR release key and targets a known device', () {
    final List<String> lines = File('assets/releases/releases.txt')
        .readAsLinesSync()
        .map((String l) => l.trim())
        .where((String l) => l.isNotEmpty && !l.startsWith('#'))
        .toList();
    expect(lines, isNotEmpty);
    final Set<String> seen = <String>{};
    for (final String path in lines) {
      final Uint8List bytes = File(path).readAsBytesSync();
      final MnpkgHeader? h = MnpkgHeader.parse(bytes);
      expect(h, isNotNull, reason: path);
      expect(h!.keyId, '1500c14420e0910a', reason: '$path must be signed with ota_release');
      expect(h.keyLabel, 'release');
      expect(h.isDevSigned, isFalse);
      expect(h.targets(DeviceKind.goggle) || h.targets(DeviceKind.maplet), isTrue, reason: path);
      expect(bytes.length, MnpkgHeader.headerLen + h.imageLen, reason: path);
      seen.add(h.targetName);
    }
    expect(seen, containsAll(<String>['goggl1', 'maplet']));
  });
}
