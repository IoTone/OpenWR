// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The maplet: an e-paper patch with the same Forth console as the goggle.
// It receives a finished 212x104 picture as hex lines and shows it.
import 'dart:typed_data';

import 'console.dart';
import 'nus_transport.dart';

class MapletConsole extends GoggleConsole {
  MapletConsole({required super.transport}) : super(kind: DeviceKind.maplet);

  static const int width = 212;
  static const int height = 104;
  static const int stride = 27;
  static const int frameBytes = stride * height; // 2808

  /// Push a packed frame (27 bytes a row, MSB first, 1 = black), show it,
  /// and keep it in [slot] (0..3) so it comes back at power-up.
  /// 26 lines of 110 bytes; about 12 s over BLE.
  Future<void> pushFrame(Uint8List frame, {int slot = 0, void Function(double)? onProgress}) async {
    if (frame.length != frameBytes) throw ArgumentError('frame must be $frameBytes bytes');
    await send('epd-clear 0 epd-cursor!', timeout: const Duration(seconds: 4));
    const int chunk = 110; // 220 hex chars + the word fits the 256-byte input line
    for (int i = 0; i < frame.length; i += chunk) {
      final int end = (i + chunk < frame.length) ? i + chunk : frame.length;
      final String hex = frame.sublist(i, end).map((int b) => b.toRadixString(16).padLeft(2, '0')).join();
      await send('s" $hex" epd-hex!', timeout: const Duration(seconds: 6));
      onProgress?.call(end / frame.length);
    }
    await send('epd-show', timeout: const Duration(seconds: 15));
    if (slot >= 0 && slot <= 3) await send('$slot epd-save', timeout: const Duration(seconds: 4));
    mapletSlot = slot;
    notifyListeners();
  }

  /// The minimap inset for Snapshot: a packed 1-bit bitmap up to 128x64.
  Future<void> sendInset(int w, int h, Uint8List packed) async {
    await send('$w $h inset!', timeout: const Duration(seconds: 4));
    const int chunk = 110;
    for (int i = 0; i < packed.length; i += chunk) {
      final int end = (i + chunk < packed.length) ? i + chunk : packed.length;
      final String hex = packed.sublist(i, end).map((int b) => b.toRadixString(16).padLeft(2, '0')).join();
      await send('s" $hex" inset-hex!', timeout: const Duration(seconds: 6));
    }
  }

  /// Take a photo and show it with the inset and [caption]; the device
  /// answers "snap: roll N", "snap: shown (no card)" or "snap: no camera frame".
  Future<String> snap(String caption) async {
    final String c = caption.replaceAll('"', '').replaceAll(RegExp(r'[^\x20-\x7e]'), '?');
    final String r = await send('s" ${c.length > 38 ? c.substring(0, 38) : c}" snap', timeout: const Duration(seconds: 25));
    return r.split('\n').firstWhere((String l) => l.startsWith('snap:'), orElse: () => r.trim());
  }

  /// The roll on the card: frame numbers, newest last.
  Future<List<int>> rollList() async {
    final String r = await send('roll-ls', timeout: const Duration(seconds: 8));
    final List<int> out = <int>[];
    for (final String l in r.split('\n')) {
      final RegExpMatch? m = RegExp(r'^(\d{4})\.bin', caseSensitive: false).firstMatch(l.trim());
      if (m != null) out.add(int.parse(m.group(1)!));
    }
    out.sort();
    return out;
  }

  /// One frame back from the card: 2 808 bytes as 26 hex lines, about 12 s over BLE.
  Future<Uint8List> rollGet(int n) async {
    final String r = await send('$n roll-get', timeout: const Duration(seconds: 40));
    final StringBuffer hex = StringBuffer();
    for (final String l in r.split('\n')) {
      final String t = l.trim();
      if (RegExp(r'^[0-9a-fA-F]+$').hasMatch(t) && t.length.isEven) hex.write(t);
    }
    final String h = hex.toString();
    if (h.length != frameBytes * 2) throw StateError('frame $n: got ${h.length ~/ 2} of $frameBytes bytes');
    final Uint8List out = Uint8List(frameBytes);
    for (int i = 0; i < frameBytes; i++) {
      out[i] = int.parse(h.substring(i * 2, i * 2 + 2), radix: 16);
    }
    return out;
  }

  Future<void> showTestPattern() => send('epd-test epd-show', timeout: const Duration(seconds: 15));
  Future<void> showSplash() => send('epd-splash epd-show', timeout: const Duration(seconds: 15));
}
