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

  Future<void> showTestPattern() => send('epd-test epd-show', timeout: const Duration(seconds: 15));
  Future<void> showSplash() => send('epd-splash epd-show', timeout: const Duration(seconds: 15));
}
