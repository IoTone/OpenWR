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
  // Running Speed and Cadence measurement, and Cycling Speed and Cadence.
  static const String rsc = '00002a53-0000-1000-8000-00805f9b34fb';
  static const String csc = '00002a5b-0000-1000-8000-00805f9b34fb';

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

/// One RSC measurement (0x2A53): speed in m/s and cadence per minute.
class RscSample {
  const RscSample({required this.speedMps, required this.cadence});
  final double speedMps;
  final int cadence;
}

RscSample? parseRsc(List<int> b) {
  if (b.length < 4) return null;
  return RscSample(speedMps: (b[1] | (b[2] << 8)) / 256, cadence: b[3]);
}

/// Crank revolutions and event time (1/1024 s) from a CSC measurement
/// (0x2A5B), or null when it carries no crank data.
(int, int)? parseCscCrank(List<int> b) {
  if (b.isEmpty) return null;
  final int f = b[0];
  int i = 1;
  if (f & 0x01 != 0) i += 6; // wheel revs u32 + event time u16
  if (f & 0x02 == 0 || b.length < i + 4) return null;
  return (b[i] | (b[i + 1] << 8), b[i + 2] | (b[i + 3] << 8));
}

/// Crank rpm between two CSC samples; both counters wrap at 16 bits.
int crankRpm((int, int) prev, (int, int) now) {
  final int revs = (now.$1 - prev.$1) & 0xFFFF;
  final int ticks = (now.$2 - prev.$2) & 0xFFFF;
  if (ticks == 0) return -1; // no new crank event: keep the last value
  return (revs * 60 * 1024 / ticks).round();
}

/// Heart-rate zone 1..5 by percent of max (50-60-70-80-90), 0 for no reading.
int hrZone(int bpm, int maxHr) {
  if (bpm <= 0 || maxHr <= 0) return 0;
  final double p = bpm / maxHr;
  if (p >= .9) return 5;
  if (p >= .8) return 4;
  if (p >= .7) return 3;
  if (p >= .6) return 2;
  return 1;
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
  static const String _kMaxHr = 'hr.max';

  BluetoothDevice? _device;
  StreamSubscription<List<int>>? _hrSub;
  StreamSubscription<List<int>>? _rscSub;
  StreamSubscription<List<int>>? _cscSub;
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
  int _maxHr = 185;
  int _stepCad = 0, _crankCad = 0;
  DateTime? _stepAt, _crankAt;
  (int, int)? _lastCrank;

  HrStrap() {
    unawaited(_loadMax());
  }

  Future<void> _loadMax() async {
    final SharedPreferences p = await SharedPreferences.getInstance();
    _maxHr = p.getInt(_kMaxHr) ?? 185;
    notifyListeners();
  }

  /// Max heart rate for the zones; set on the strap page.
  int get maxHr => _maxHr;
  Future<void> setMaxHr(int v) async {
    _maxHr = v.clamp(120, 230);
    notifyListeners();
    final SharedPreferences p = await SharedPreferences.getInstance();
    await p.setInt(_kMaxHr, _maxHr);
  }

  int get zone => hrZone(bpm, _maxHr);

  /// Steps per minute from the strap's running cadence, 0 when stale.
  int get stepCadence => _fresh(_stepAt) ? _stepCad : 0;

  /// Crank rpm from its cycling cadence, 0 when stale.
  int get crankCadence => _fresh(_crankAt, secs: 6) ? _crankCad : 0;

  bool get hasRunCadence => _rscSub != null;
  bool get hasRideCadence => _cscSub != null;

  bool _fresh(DateTime? t, {int secs = 5}) => isConnected && t != null && DateTime.now().difference(t).inSeconds <= secs;

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
      BluetoothCharacteristic? hrm, batt, rsc, csc;
      for (final BluetoothService s in services) {
        for (final BluetoothCharacteristic c in s.characteristics) {
          if (HrUuids.same(c.uuid.str, HrUuids.measurement)) hrm = c;
          if (HrUuids.same(c.uuid.str, HrUuids.batteryLevel)) batt = c;
          if (HrUuids.same(c.uuid.str, HrUuids.rsc)) rsc = c;
          if (HrUuids.same(c.uuid.str, HrUuids.csc)) csc = c;
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
      // Cadence is a bonus: a strap without it, or one that refuses, still counts.
      if (rsc != null) {
        try {
          _rscSub = rsc.onValueReceived.listen(_onRsc);
          await rsc.setNotifyValue(true);
        } catch (e) {
          debugPrint('[hr] rsc: $e');
        }
      }
      if (csc != null) {
        try {
          _cscSub = csc.onValueReceived.listen(_onCsc);
          await csc.setNotifyValue(true);
        } catch (e) {
          debugPrint('[hr] csc: $e');
        }
      }
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

  void _onRsc(List<int> v) {
    final RscSample? s = parseRsc(v);
    if (s == null) return;
    debugPrint('[hr] rsc ${v.map((int x) => x.toRadixString(16).padLeft(2, '0')).join()} cad=${s.cadence} v=${s.speedMps.toStringAsFixed(2)}');
    _stepAt = DateTime.now();
    if (s.cadence != _stepCad) {
      _stepCad = s.cadence;
      notifyListeners();
    }
  }

  void _onCsc(List<int> v) {
    final (int, int)? c = parseCscCrank(v);
    debugPrint('[hr] csc ${v.map((int x) => x.toRadixString(16).padLeft(2, '0')).join()} crank=$c');
    if (c == null) return;
    final (int, int)? prev = _lastCrank;
    _lastCrank = c;
    if (prev == null) return;
    final int rpm = crankRpm(prev, c);
    if (rpm < 0) {
      if (!_fresh(_crankAt, secs: 3)) _crankCad = 0; // stopped pedalling
      return;
    }
    _crankAt = DateTime.now();
    if (rpm != _crankCad && rpm < 250) {
      _crankCad = rpm;
      notifyListeners();
    }
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
    await _rscSub?.cancel();
    await _cscSub?.cancel();
    await _stateSub?.cancel();
    _rscSub = null;
    _cscSub = null;
    _lastCrank = null;
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
