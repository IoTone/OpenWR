// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// A heart-rate strap over the standard Bluetooth Heart Rate service
// (0x180D, measurement 0x2A37). The Scosche Rhythm 24 speaks it, as do
// Polar, Garmin and Wahoo straps, so no vendor SDK is needed for bpm.
// The session reads [bpm] once a second and sends it to the glass.
import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'console.dart' show GoggleLink;

class HrUuids {
  static const String service = '0000180d-0000-1000-8000-00805f9b34fb';
  static const String measurement = '00002a37-0000-1000-8000-00805f9b34fb';
  static const String battery = '0000180f-0000-1000-8000-00805f9b34fb';
  static const String batteryLevel = '00002a19-0000-1000-8000-00805f9b34fb';

  /// Matches the short (`180d`) and long forms a scan or discovery reports.
  static bool same(String a, String full) {
    final String x = a.toLowerCase();
    return x == full || (x.length == 4 && full.substring(4, 8) == x);
  }
}

/// One Heart Rate Measurement notification.
class HrSample {
  const HrSample({required this.bpm, required this.contact, this.rrMs = const <int>[], this.energyKj});
  final int bpm;

  /// true/false when the strap reports skin contact, null when it can't tell.
  final bool? contact;

  /// Beat-to-beat intervals in ms, when the strap sends them.
  final List<int> rrMs;
  final int? energyKj;
}

/// Decode 0x2A37: flags, bpm as u8 or u16, then optional energy and RR
/// intervals (1/1024 s). Returns null for a packet too short to hold a bpm.
HrSample? parseHeartRate(List<int> b) {
  if (b.length < 2) return null;
  final int f = b[0];
  int i = 1;
  int bpm;
  if (f & 0x01 != 0) {
    if (b.length < 3) return null;
    bpm = b[1] | (b[2] << 8);
    i = 3;
  } else {
    bpm = b[1];
    i = 2;
  }
  final bool? contact = switch ((f >> 1) & 3) { 2 => false, 3 => true, _ => null };
  int? energy;
  if (f & 0x08 != 0 && i + 1 < b.length) {
    energy = b[i] | (b[i + 1] << 8);
    i += 2;
  }
  final List<int> rr = <int>[];
  if (f & 0x10 != 0) {
    for (; i + 1 < b.length; i += 2) {
      rr.add(((b[i] | (b[i + 1] << 8)) * 1000 / 1024).round());
    }
  }
  return HrSample(bpm: bpm, contact: contact, rrMs: rr, energyKj: energy);
}

/// Is this scan result worth offering as a strap? Advertises the Heart Rate
/// service, or carries a name the common straps use (the Rhythm 24 puts
/// its services only in the scan response, which not every phone merges).
bool looksLikeHrStrap(String name, List<String> serviceUuids) {
  if (serviceUuids.any((String u) => HrUuids.same(u, HrUuids.service))) return true;
  final String n = name.toLowerCase();
  return n.startsWith('rhythm') || n.startsWith('r+') || n.startsWith('polar') || n.startsWith('hrm') ||
      n.startsWith('tickr') || n.contains('heart');
}

class HrStrap extends ChangeNotifier {
  static const String _kLastId = 'hr.lastId';
  static const String _kLastName = 'hr.lastName';

  BluetoothDevice? _device;
  StreamSubscription<List<int>>? _hrSub;
  StreamSubscription<BluetoothConnectionState>? _stateSub;
  GoggleLink _link = GoggleLink.disconnected;
  String? _deviceId;
  String? _deviceName;
  String? _lastError;
  int _bpm = 0;
  bool? _contact;
  int _battery = -1;
  DateTime? _lastSample;
  List<int> _lastRr = const <int>[];
  bool _userDisconnect = false;
  Timer? _retryTimer;
  int _retriesLeft = 0;

  GoggleLink get link => _link;
  bool get isConnected => _link == GoggleLink.connected;
  String? get deviceId => _deviceId;
  String? get deviceName => _deviceName;
  String? get lastError => _lastError;
  bool? get contact => _contact;
  int get battery => _battery;
  List<int> get lastRr => _lastRr;

  /// Beats per minute, or 0 when there is no live reading: not connected,
  /// no skin contact, or nothing heard for five seconds.
  int get bpm {
    final DateTime? t = _lastSample;
    if (!isConnected || t == null || _contact == false) return 0;
    if (DateTime.now().difference(t).inSeconds > 5) return 0;
    return _bpm;
  }

  Future<void> connect(String deviceId, {String? name}) async {
    if (_link == GoggleLink.connecting) return;
    _userDisconnect = false;
    _retryTimer?.cancel();
    _deviceId = deviceId;
    _deviceName = name;
    _lastError = null;
    _link = GoggleLink.connecting;
    notifyListeners();
    try {
      await _teardown();
      final BluetoothDevice d = BluetoothDevice.fromId(deviceId);
      _device = d;
      await d.connect(timeout: const Duration(seconds: 15), autoConnect: false);
      final List<BluetoothService> services = await d.discoverServices();
      BluetoothCharacteristic? hrm, batt;
      for (final BluetoothService s in services) {
        for (final BluetoothCharacteristic c in s.characteristics) {
          if (HrUuids.same(c.uuid.str, HrUuids.measurement)) hrm = c;
          if (HrUuids.same(c.uuid.str, HrUuids.batteryLevel)) batt = c;
        }
      }
      if (hrm == null) {
        await d.disconnect();
        throw StateError('that device has no heart-rate service');
      }
      _stateSub = d.connectionState.listen((BluetoothConnectionState s) {
        if (s == BluetoothConnectionState.disconnected) _onDropped();
      });
      _hrSub = hrm.onValueReceived.listen(_onMeasurement);
      await hrm.setNotifyValue(true);
      if (batt != null) {
        try {
          final List<int> v = await batt.read();
          if (v.isNotEmpty) _battery = v[0];
        } catch (_) {}
      }
      _link = GoggleLink.connected;
      _retriesLeft = 0;
      notifyListeners();
      final SharedPreferences p = await SharedPreferences.getInstance();
      await p.setString(_kLastId, deviceId);
      if (name != null) await p.setString(_kLastName, name);
    } catch (e) {
      _lastError = e.toString().replaceFirst('StateError: ', '').replaceFirst('Exception: ', '');
      debugPrint('[hr] connect: $_lastError');
      await _teardown();
      _link = GoggleLink.disconnected;
      notifyListeners();
      _scheduleRetry();
    }
  }

  void _onMeasurement(List<int> v) {
    final HrSample? s = parseHeartRate(v);
    if (s == null) return;
    final bool changed = s.bpm != _bpm || s.contact != _contact;
    _bpm = s.bpm;
    _contact = s.contact;
    _lastSample = DateTime.now();
    if (s.rrMs.isNotEmpty) _lastRr = s.rrMs;
    if (changed) notifyListeners();
  }

  void _onDropped() {
    if (_link == GoggleLink.disconnected) return;
    unawaited(_teardown());
    _link = GoggleLink.disconnected;
    notifyListeners();
    if (!_userDisconnect) {
      _retriesLeft = 15; // every 12 s for three minutes, like the glass
      _scheduleRetry();
    }
  }

  void _scheduleRetry() {
    if (_userDisconnect || _retriesLeft <= 0 || _deviceId == null) return;
    _retriesLeft--;
    _retryTimer?.cancel();
    _retryTimer = Timer(const Duration(seconds: 12), () {
      if (_link == GoggleLink.disconnected && !_userDisconnect) unawaited(connect(_deviceId!, name: _deviceName));
    });
  }

  /// The strap paired last time, if any.
  Future<(String, String?)?> remembered() async {
    final SharedPreferences p = await SharedPreferences.getInstance();
    final String? id = p.getString(_kLastId);
    return id == null ? null : (id, p.getString(_kLastName));
  }

  /// App start and resume: one quiet try at the remembered strap.
  Future<void> reconnectIfNeeded() async {
    if (_link != GoggleLink.disconnected) return;
    final (String, String?)? r = await remembered();
    if (r == null) return;
    await connect(r.$1, name: r.$2);
  }

  Future<void> disconnect() async {
    _userDisconnect = true;
    _retryTimer?.cancel();
    await _teardown();
    _link = GoggleLink.disconnected;
    notifyListeners();
  }

  /// Disconnect and stop reconnecting to it at start.
  Future<void> forget() async {
    await disconnect();
    final SharedPreferences p = await SharedPreferences.getInstance();
    await p.remove(_kLastId);
    await p.remove(_kLastName);
    _deviceId = null;
    _deviceName = null;
    notifyListeners();
  }

  Future<void> _teardown() async {
    await _hrSub?.cancel();
    await _stateSub?.cancel();
    _hrSub = null;
    _stateSub = null;
    _lastSample = null;
    _contact = null;
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
    _retryTimer?.cancel();
    unawaited(_teardown());
    super.dispose();
  }
}
