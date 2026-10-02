// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The MagNET OTA package header (.mnpkg, wire format 1): 256 bytes, big
// endian, signed. The app only reads it to show what a release is and to
// check it targets the goggle before offering it; the goggle does the
// signature and hash checks itself.
import 'dart:typed_data';

import 'console.dart' show DeviceKind;

class MnpkgHeader {
  const MnpkgHeader({
    required this.sigAlg,
    required this.chip,
    required this.board,
    required this.imageFormat,
    required this.flags,
    required this.version,
    required this.minRunning,
    required this.imageLen,
    required this.imageSha256,
    required this.buildTime,
    required this.variant,
    required this.fwVersion,
    required this.keyId,
  });

  static const int headerLen = 256;
  static const int chipEsp32c3 = 0x0005;
  static const int boardBeetleEsp32c3 = 0x0503;
  static const int chipEsp32s3 = 0x0004;
  static const int boardXiaoEsp32s3 = 0x0408; // maplet on the XIAO ESP32S3 Sense (proposed id)
  static const int flagRequiresRepartition = 0x01;
  static const int flagClearsBundles = 0x02;

  final int sigAlg; // 1 P-256, 2 Ed25519
  final int chip;
  final int board;
  final int imageFormat; // 1 ESP-IDF, 2 MCUboot
  final int flags;
  final String version; // "0.5.0+1"
  final String minRunning;
  final int imageLen;
  final String imageSha256; // hex
  final DateTime buildTime;
  final String variant;
  final String fwVersion;
  final String keyId; // hex

  bool get clearsBundles => flags & flagClearsBundles != 0;
  bool get requiresRepartition => flags & flagRequiresRepartition != 0;
  bool get isDevSigned => keyId == 'f0846c794440d0ea';
  /// The OpenWR release keys: ota_release signs, ota_release_next is the rotation key.
  static const Map<String, String> releaseKeys = <String, String>{
    '1500c14420e0910a': 'release',
    'bd0d667bf74d1d52': 'release, next',
  };
  String get keyLabel => releaseKeys[keyId] ?? (isDevSigned ? 'dev' : 'unknown key');

  /// True when a goggl1 would accept the target fields (spec check 4).
  bool get targetsGoggle => chip == chipEsp32c3 && (board == 0 || board == boardBeetleEsp32c3) && imageFormat == 1;
  bool get targetsMaplet => chip == chipEsp32s3 && (board == 0 || board == boardXiaoEsp32s3) && imageFormat == 1;
  bool targets(DeviceKind k) => k == DeviceKind.maplet ? targetsMaplet : targetsGoggle;
  String get targetName => targetsMaplet ? 'maplet' : targetsGoggle ? 'goggl1' : 'other';
  String get targetDesc => targetsMaplet ? 'esp32s3 / xiao sense' : targetsGoggle ? 'esp32c3 / beetle' : 'other target';

  static MnpkgHeader? parse(Uint8List bytes) {
    if (bytes.length < headerLen) return null;
    final ByteData b = ByteData.sublistView(bytes, 0, headerLen);
    if (String.fromCharCodes(bytes.sublist(0, 4)) != 'MNPK' || b.getUint8(4) != 1 || b.getUint16(6) != headerLen) return null;
    String ver(int off) => '${b.getUint8(off)}.${b.getUint8(off + 1)}.${b.getUint16(off + 2)}+${b.getUint32(off + 4)}';
    String ascii(int off, int len) => String.fromCharCodes(bytes.sublist(off, off + len)).replaceAll('\u0000', '');
    String hex(int off, int len) => bytes.sublist(off, off + len).map((int x) => x.toRadixString(16).padLeft(2, '0')).join();
    return MnpkgHeader(
      sigAlg: b.getUint8(5),
      chip: b.getUint16(8),
      board: b.getUint16(10),
      imageFormat: b.getUint8(12),
      flags: b.getUint8(13),
      version: ver(16),
      minRunning: ver(24),
      imageLen: b.getUint32(32),
      imageSha256: hex(36, 32),
      buildTime: DateTime.fromMillisecondsSinceEpoch(b.getInt64(68) * 1000, isUtc: true),
      variant: ascii(76, 16),
      fwVersion: ascii(92, 32),
      keyId: hex(124, 8),
    );
  }
}

/// A release the app carries, ready to serve.
class EmbeddedRelease {
  const EmbeddedRelease({required this.asset, required this.fileName, required this.header, required this.sizeBytes});
  final String asset;
  final String fileName;
  final MnpkgHeader header;
  final int sizeBytes;
}
