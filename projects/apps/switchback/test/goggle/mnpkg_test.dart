// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:switchback/goggle/mnpkg.dart';

Uint8List _header({int chip = 0x0005, int board = 0x0503, int fmt = 1, int flags = 0}) {
  final Uint8List h = Uint8List(256);
  final ByteData b = ByteData.sublistView(h);
  h.setAll(0, 'MNPK'.codeUnits);
  b.setUint8(4, 1); // format_version
  b.setUint8(5, 2); // ed25519
  b.setUint16(6, 256);
  b.setUint16(8, chip);
  b.setUint16(10, board);
  b.setUint8(12, fmt);
  b.setUint8(13, flags);
  // version 0.5.1+1, min_running 0.4.0+0
  b.setUint8(16, 0); b.setUint8(17, 5); b.setUint16(18, 1); b.setUint32(20, 1);
  b.setUint8(24, 0); b.setUint8(25, 4); b.setUint16(26, 0); b.setUint32(28, 0);
  b.setUint32(32, 1164316);
  b.setInt64(68, 1790850000);
  h.setAll(76, 'dev-weather'.codeUnits);
  h.setAll(92, '0.5.1'.codeUnits);
  h.setAll(124, <int>[0xf0, 0x84, 0x6c, 0x79, 0x44, 0x40, 0xd0, 0xea]);
  return h;
}

void main() {
  test('parses a goggl1 package header', () {
    final MnpkgHeader h = MnpkgHeader.parse(_header())!;
    expect(h.sigAlg, 2);
    expect(h.chip, 0x0005);
    expect(h.board, 0x0503);
    expect(h.version, '0.5.1+1');
    expect(h.minRunning, '0.4.0+0');
    expect(h.imageLen, 1164316);
    expect(h.variant, 'dev-weather');
    expect(h.fwVersion, '0.5.1');
    expect(h.keyId, 'f0846c794440d0ea');
    expect(h.isDevSigned, isTrue);
    expect(h.targetsGoggle, isTrue);
    expect(h.clearsBundles, isFalse);
    expect(h.buildTime.year, 2026);
  });

  test('rejects wrong magic and short input', () {
    final Uint8List bad = _header();
    bad[0] = 0x58;
    expect(MnpkgHeader.parse(bad), isNull);
    expect(MnpkgHeader.parse(Uint8List(100)), isNull);
  });

  test('target check: any board of the chip is fine, other chips are not', () {
    expect(MnpkgHeader.parse(_header(board: 0))!.targetsGoggle, isTrue);
    expect(MnpkgHeader.parse(_header(chip: 0x0001, board: 0x0102))!.targetsGoggle, isFalse);
    expect(MnpkgHeader.parse(_header(fmt: 2))!.targetsGoggle, isFalse);
  });

  test('flags', () {
    expect(MnpkgHeader.parse(_header(flags: 0x02))!.clearsBundles, isTrue);
    expect(MnpkgHeader.parse(_header(flags: 0x01))!.requiresRepartition, isTrue);
  });
}
