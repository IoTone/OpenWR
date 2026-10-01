// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// Nordic UART Service client. The goggle exposes its ok> console as NUS:
// write lines to RX, read everything it prints from TX notifications.
import 'dart:async';
import 'dart:convert';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';

/// The NUS UUIDs, lowercased as flutter_blue_plus reports them.
class NusUuids {
  static const String service = '6e400001-b5a3-f393-e0a9-e50e24dcca9e';
  static const String rx = '6e400002-b5a3-f393-e0a9-e50e24dcca9e';
  static const String tx = '6e400003-b5a3-f393-e0a9-e50e24dcca9e';
}

/// Transport abstraction so the console model can be driven by a fake.
abstract class ConsoleTransport {
  /// Raw text from the device, in notification-sized chunks.
  Stream<String> get incoming;

  /// True while connected; false once the link drops for any reason.
  Stream<bool> get connected;

  Future<void> connect(String deviceId);
  Future<void> writeLine(String line);
  Future<void> disconnect();
  void dispose();
}

class NusTransport implements ConsoleTransport {
  BluetoothDevice? _device;
  BluetoothCharacteristic? _rx;
  BluetoothCharacteristic? _tx;
  StreamSubscription<List<int>>? _txSub;
  StreamSubscription<BluetoothConnectionState>? _stateSub;

  final StreamController<String> _in = StreamController<String>.broadcast();
  final StreamController<bool> _conn = StreamController<bool>.broadcast();

  int _mtu = 23;

  @override
  Stream<String> get incoming => _in.stream;
  @override
  Stream<bool> get connected => _conn.stream;

  @override
  Future<void> connect(String deviceId) async {
    await disconnect();
    final BluetoothDevice d = BluetoothDevice.fromId(deviceId);
    _device = d;
    await d.connect(timeout: const Duration(seconds: 15), mtu: 185);
    _stateSub = d.connectionState.listen((BluetoothConnectionState s) {
      if (s == BluetoothConnectionState.disconnected) {
        _conn.add(false);
      }
    });
    try {
      _mtu = await d.requestMtu(185);
    } catch (_) {
      _mtu = d.mtuNow;
    }
    final List<BluetoothService> services = await d.discoverServices();
    for (final BluetoothService s in services) {
      if (s.uuid.str.toLowerCase() != NusUuids.service) continue;
      for (final BluetoothCharacteristic c in s.characteristics) {
        final String u = c.uuid.str.toLowerCase();
        if (u == NusUuids.rx) _rx = c;
        if (u == NusUuids.tx) _tx = c;
      }
    }
    if (_rx == null || _tx == null) {
      await d.disconnect();
      throw StateError('No Nordic UART service on this device');
    }
    await _tx!.setNotifyValue(true);
    _txSub = _tx!.onValueReceived.listen((List<int> v) {
      _in.add(utf8.decode(v, allowMalformed: true));
    });
    _conn.add(true);
  }

  @override
  Future<void> writeLine(String line) async {
    final BluetoothCharacteristic? rx = _rx;
    if (rx == null) throw StateError('Not connected');
    final List<int> bytes = utf8.encode('$line\r');
    final int chunk = (_mtu - 3).clamp(20, 180);
    for (int off = 0; off < bytes.length; off += chunk) {
      final int end = (off + chunk) > bytes.length ? bytes.length : off + chunk;
      await rx.write(bytes.sublist(off, end), withoutResponse: true);
    }
  }

  @override
  Future<void> disconnect() async {
    await _txSub?.cancel();
    await _stateSub?.cancel();
    _txSub = null;
    _stateSub = null;
    _rx = null;
    _tx = null;
    final BluetoothDevice? d = _device;
    _device = null;
    if (d != null) {
      try {
        await d.disconnect();
      } catch (_) {}
    }
  }

  @override
  void dispose() {
    unawaited(disconnect());
    _in.close();
    _conn.close();
  }
}
