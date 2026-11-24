// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// .svpk pack format v0, byte for byte what tools/svpk.py writes and the
// pass firmware reads (projects/prototypes/SubverseReader_REV1).
//
//   header 256 bytes: "SVPK", u16 version 0, u16 256, u16 w, u16 h, u8 bpp,
//     u8 orientation, u16 flags, u32 pages, u32 table offset, u32 0,
//     sha256[32], title[64], author[64], source[64], u32 unix time
//   table: per page u32 offset, u32 length, u8 compression (0 raw,
//     1 PackBits), u8 hint (0 text, 1 image), u16 0
//   pages: rows top to bottom, MSB first, 1 = black ink
import 'dart:convert';
import 'dart:typed_data';

import 'package:crypto/crypto.dart' show sha256;

/// What a pass's panel looks like, held the way you read it.
class PanelProfile {
  const PanelProfile(this.id, this.width, this.height, this.label);
  final String id;
  final int width;
  final int height;
  final String label;

  int get stride => (width + 7) >> 3;

  static const PanelProfile sr15 = PanelProfile('sr15', 300, 400, 'SR-15 Dial Pass (4.2in, 300×400)');
  static const PanelProfile sr16 = PanelProfile('sr16', 240, 416, 'SR-16 Setlist (3.7in, 240×416)');
  static const List<PanelProfile> all = <PanelProfile>[sr15, sr16];
}

/// One 1-bit page: [bits] is stride*height bytes, 1 = black.
class PackPage {
  PackPage(this.bits, {required this.image});
  final Uint8List bits;
  final bool image; // dithered photo page (vs thresholded text)
}

Uint8List packBits(Uint8List data) {
  final BytesBuilder out = BytesBuilder();
  int i = 0;
  final int n = data.length;
  while (i < n) {
    int j = i;
    while (j + 1 < n && data[j + 1] == data[i] && j - i < 127) {
      j++;
    }
    if (j > i) {
      out.addByte(257 - (j - i + 1));
      out.addByte(data[i]);
      i = j + 1;
      continue;
    }
    j = i;
    while (j < n && j - i < 128 && !(j + 2 < n && data[j] == data[j + 1] && data[j] == data[j + 2])) {
      j++;
    }
    out.addByte(j - i - 1);
    out.add(data.sublist(i, j));
    i = j;
  }
  return out.toBytes();
}

Uint8List _fixed(String? s, int n) {
  final Uint8List out = Uint8List(n);
  List<int> b = utf8.encode(s ?? '');
  if (b.length > n - 1) {
    b = b.sublist(0, n - 1);
    // don't leave half a UTF-8 sequence at the end
    while (b.isNotEmpty && (b.last & 0xC0) == 0x80) {
      b = b.sublist(0, b.length - 1);
    }
    if (b.isNotEmpty && b.last >= 0xC0) b = b.sublist(0, b.length - 1);
  }
  out.setRange(0, b.length, b);
  return out;
}

Uint8List buildPack({
  required PanelProfile profile,
  required List<PackPage> pages,
  required String title,
  String author = '',
  String source = '',
  required Uint8List sourceBytes,
}) {
  final List<(Uint8List, int, int)> blobs = <(Uint8List, int, int)>[];
  for (final PackPage p in pages) {
    final Uint8List comp = packBits(p.bits);
    blobs.add(comp.length < p.bits.length ? (comp, 1, p.image ? 1 : 0) : (p.bits, 0, p.image ? 1 : 0));
  }
  const int hdr = 256;
  final int tableOff = hdr;
  int off = tableOff + 12 * blobs.length;
  final ByteData table = ByteData(12 * blobs.length);
  for (int i = 0; i < blobs.length; i++) {
    final (Uint8List data, int comp, int hint) = blobs[i];
    table.setUint32(i * 12, off, Endian.little);
    table.setUint32(i * 12 + 4, data.length, Endian.little);
    table.setUint8(i * 12 + 8, comp);
    table.setUint8(i * 12 + 9, hint);
    off += data.length;
  }
  final ByteData h = ByteData(hdr);
  final Uint8List hb = h.buffer.asUint8List();
  hb.setRange(0, 4, ascii.encode('SVPK'));
  h.setUint16(4, 0, Endian.little);
  h.setUint16(6, hdr, Endian.little);
  h.setUint16(8, profile.width, Endian.little);
  h.setUint16(10, profile.height, Endian.little);
  h.setUint8(12, 1);
  h.setUint8(13, 0);
  h.setUint16(14, 0, Endian.little);
  h.setUint32(16, blobs.length, Endian.little);
  h.setUint32(20, tableOff, Endian.little);
  h.setUint32(24, 0, Endian.little);
  hb.setRange(28, 60, sha256.convert(sourceBytes).bytes);
  hb.setRange(60, 124, _fixed(title, 64));
  hb.setRange(124, 188, _fixed(author, 64));
  hb.setRange(188, 252, _fixed(source, 64));
  h.setUint32(252, DateTime.now().millisecondsSinceEpoch ~/ 1000, Endian.little);
  final BytesBuilder out = BytesBuilder(copy: false)
    ..add(hb)
    ..add(table.buffer.asUint8List());
  for (final (Uint8List data, int _, int _) in blobs) {
    out.add(data);
  }
  return out.toBytes();
}

/// A file name the pass's FAT card is happy with.
String packFileName(String title) {
  String s = title.toLowerCase().replaceAll(RegExp(r'[^a-z0-9]+'), '-');
  s = s.replaceAll(RegExp(r'^-+|-+$'), '');
  if (s.length > 40) s = s.substring(0, 40);
  return '${s.isEmpty ? 'pack' : s}.svpk';
}
