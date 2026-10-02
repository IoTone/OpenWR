// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// A session: the phone is the sensor hub, the glass is the display. GPS
// gives distance and pace, a timer gives elapsed time, and once a second
// one Forth line goes to the goggle:  secs pace10 dist100 hr sess!
import 'dart:async';
import 'dart:convert';
import 'dart:math' as math;

import 'package:flutter/foundation.dart';
import 'package:geolocator/geolocator.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:wakelock_plus/wakelock_plus.dart';

import 'console.dart';

enum Sport { run, ride, ski, paddle, climb }

class SessionSummary {
  const SessionSummary({
    required this.sport, required this.startedAt, required this.seconds, required this.meters, required this.imperial,
    this.track = const <(double, double)>[], this.climbMeters = 0,
  });
  final Sport sport;
  final DateTime startedAt;
  final int seconds;
  final double meters;
  final bool imperial;
  /// GPS points, thinned to one every ~10 m, so a map can be drawn later.
  final List<(double, double)> track;
  final double climbMeters;

  double get distanceUnits => imperial ? meters / 1609.344 : meters / 1000;
  double get avgPaceMinPerUnit => distanceUnits < 0.01 ? 0 : (seconds / 60) / distanceUnits;

  Map<String, dynamic> toJson() => <String, dynamic>{
        'sport': sport.name, 'startedAt': startedAt.toIso8601String(), 'seconds': seconds, 'meters': meters, 'imperial': imperial,
        'climb': climbMeters,
        // 1e-5 degrees is about a metre; plenty for a 13 m/px map
        'track': track.map(((double, double) p) => <int>[(p.$1 * 1e5).round(), (p.$2 * 1e5).round()]).toList(),
      };
  static SessionSummary fromJson(Map<String, dynamic> j) => SessionSummary(
        sport: Sport.values.firstWhere((Sport s) => s.name == j['sport'], orElse: () => Sport.run),
        startedAt: DateTime.parse(j['startedAt'] as String),
        seconds: j['seconds'] as int,
        meters: (j['meters'] as num).toDouble(),
        imperial: j['imperial'] as bool,
        climbMeters: ((j['climb'] ?? 0) as num).toDouble(),
        track: ((j['track'] ?? const <dynamic>[]) as List<dynamic>)
            .map((dynamic e) => ((e[0] as num) / 1e5, (e[1] as num) / 1e5))
            .toList(),
      );
}

class SessionController extends ChangeNotifier {
  SessionController({required GoggleConsole goggle}) : _g = goggle;

  final GoggleConsole _g;
  static const String _kLog = 'session.log';

  Sport sport = Sport.run;
  void setSport(Sport s) {
    sport = s;
    notifyListeners();
  }
  bool _running = false;
  DateTime? _startedAt;
  int _seconds = 0;
  double _meters = 0;
  double _paceMinPerUnit = 0; // rolling, over the last ~30 s
  int _hr = 0;
  Position? _last;
  final List<(double, double)> _track = <(double, double)>[];
  double _climb = 0;
  double? _lastAlt;
  final List<(DateTime, double)> _recent = <(DateTime, double)>[]; // (time, cumulative meters)
  StreamSubscription<Position>? _pos;
  Timer? _tick;
  String? _gpsNote;
  List<SessionSummary> _log = <SessionSummary>[];

  bool get running => _running;
  int get seconds => _seconds;
  double get meters => _meters;
  double get paceMinPerUnit => _paceMinPerUnit;
  int get hr => _hr;
  String? get gpsNote => _gpsNote;
  List<SessionSummary> get log => List<SessionSummary>.unmodifiable(_log);
  List<(double, double)> get liveTrack => List<(double, double)>.unmodifiable(_track);
  bool get imperial => _g.status?.imperial ?? true;
  double get distanceUnits => imperial ? _meters / 1609.344 : _meters / 1000;

  Future<void> load() async {
    final SharedPreferences p = await SharedPreferences.getInstance();
    final List<String> raw = p.getStringList(_kLog) ?? <String>[];
    _log = raw.map((String s) => SessionSummary.fromJson(jsonDecode(s) as Map<String, dynamic>)).toList();
    notifyListeners();
  }

  Future<void> start() async {
    if (_running) return;
    _running = true;
    _startedAt = DateTime.now();
    _seconds = 0;
    _meters = 0;
    _paceMinPerUnit = 0;
    _last = null;
    _track.clear();
    _climb = 0;
    _lastAlt = null;
    _recent.clear();
    _gpsNote = null;
    notifyListeners();
    unawaited(WakelockPlus.enable());
    unawaited(_startGps());
    if (_g.isConnected) unawaited(_g.send('sess-on', timeout: const Duration(seconds: 3)).catchError((_) => ''));
    _tick = Timer.periodic(const Duration(seconds: 1), (_) => _onTick());
  }

  Future<void> _startGps() async {
    try {
      LocationPermission p = await Geolocator.checkPermission();
      if (p == LocationPermission.denied) p = await Geolocator.requestPermission();
      if (p == LocationPermission.denied || p == LocationPermission.deniedForever) {
        _gpsNote = 'no location permission: time only';
        notifyListeners();
        return;
      }
      if (!await Geolocator.isLocationServiceEnabled()) {
        _gpsNote = 'location is off: time only';
        notifyListeners();
        return;
      }
      _pos = Geolocator.getPositionStream(
        locationSettings: const LocationSettings(accuracy: LocationAccuracy.best, distanceFilter: 3),
      ).listen(_onPosition, onError: (Object e) {
        _gpsNote = 'gps: $e';
        notifyListeners();
      });
      _gpsNote = 'waiting for GPS…';
      notifyListeners();
    } catch (e) {
      _gpsNote = 'gps: $e';
      notifyListeners();
    }
  }

  void _onPosition(Position p) {
    if (p.accuracy > 40) {
      _gpsNote = 'GPS accuracy ${p.accuracy.round()} m, waiting';
      notifyListeners();
      return;
    }
    final Position? prev = _last;
    if (prev != null) {
      final double d = Geolocator.distanceBetween(prev.latitude, prev.longitude, p.latitude, p.longitude);
      if (d < 200) _meters += d; // a jump bigger than that is a GPS glitch
      if (d >= 10 || _track.isEmpty) _track.add((p.latitude, p.longitude));
    } else {
      _track.add((p.latitude, p.longitude));
    }
    if (p.altitude != 0 && p.altitudeAccuracy < 15) {
      final double? la = _lastAlt;
      if (la != null && p.altitude - la > 3) _climb += p.altitude - la; // 3 m hysteresis: GPS altitude wobbles
      if (la == null || (p.altitude - la).abs() > 3) _lastAlt = p.altitude;
    }
    _last = p;
    _gpsNote = null;
    final DateTime now = DateTime.now();
    _recent.add((now, _meters));
    _recent.removeWhere(((DateTime, double) r) => now.difference(r.$1).inSeconds > 30);
    if (_recent.length >= 2) {
      final double dm = _meters - _recent.first.$2;
      final double ds = now.difference(_recent.first.$1).inMilliseconds / 1000;
      final double unitsPerSec = (imperial ? dm / 1609.344 : dm / 1000) / math.max(ds, 1);
      _paceMinPerUnit = unitsPerSec < 1e-4 ? 0 : (1 / unitsPerSec) / 60;
    }
    notifyListeners();
  }

  void _onTick() {
    if (!_running) return;
    _seconds = DateTime.now().difference(_startedAt!).inSeconds;
    notifyListeners();
    if (_g.isConnected && !_g.busy) {
      final int pace10 = (_paceMinPerUnit * 10).round().clamp(0, 999);
      final int dist100 = (distanceUnits * 100).round();
      unawaited(_g.send('$_seconds $pace10 $dist100 $_hr sess!', timeout: const Duration(seconds: 2), background: true).catchError((_) => ''));
    }
  }

  Future<SessionSummary?> stop() async {
    if (!_running) return null;
    _running = false;
    _tick?.cancel();
    _tick = null;
    await _pos?.cancel();
    _pos = null;
    unawaited(WakelockPlus.disable());
    if (_g.isConnected) unawaited(_g.send('sess-off', timeout: const Duration(seconds: 3)).catchError((_) => ''));
    final Position? lp = _last;
    if (lp != null && (_track.isEmpty || _track.last != (lp.latitude, lp.longitude))) _track.add((lp.latitude, lp.longitude));
    final SessionSummary s = SessionSummary(
      sport: sport, startedAt: _startedAt!, seconds: _seconds, meters: _meters, imperial: imperial,
      track: List<(double, double)>.unmodifiable(_track), climbMeters: _climb,
    );
    _log.insert(0, s);
    if (_log.length > 50) _log.removeLast();
    final SharedPreferences p = await SharedPreferences.getInstance();
    await p.setStringList(_kLog, _log.map((SessionSummary x) => jsonEncode(x.toJson())).toList());
    notifyListeners();
    return s;
  }

  /// A finished session with a real track (a Forest Park loop, Portland)
  /// so the Postcard can be tried before anyone has run anywhere.
  Future<void> addDemo() async {
    const List<(double, double)> wps = <(double, double)>[
      (45.5363, -122.7122), (45.5389, -122.7169), (45.5412, -122.7212), (45.5440, -122.7251), (45.5428, -122.7290),
      (45.5395, -122.7306), (45.5366, -122.7272), (45.5340, -122.7215), (45.5334, -122.7161), (45.5363, -122.7122),
    ];
    final List<(double, double)> track = <(double, double)>[];
    for (int i = 0; i + 1 < wps.length; i++) {
      for (int t = 0; t < 20; t++) {
        final double u = t / 20;
        track.add((wps[i].$1 + (wps[i + 1].$1 - wps[i].$1) * u, wps[i].$2 + (wps[i + 1].$2 - wps[i].$2) * u));
      }
    }
    track.add(wps.last);
    double m = 0;
    for (int i = 0; i + 1 < track.length; i++) {
      m += Geolocator.distanceBetween(track[i].$1, track[i].$2, track[i + 1].$1, track[i + 1].$2);
    }
    _log.insert(0, SessionSummary(
      sport: Sport.run, startedAt: DateTime.now().subtract(const Duration(hours: 2)), seconds: 3734, meters: m,
      imperial: imperial, track: track, climbMeters: 412,
    ));
    if (_log.length > 50) _log.removeLast();
    final SharedPreferences p = await SharedPreferences.getInstance();
    await p.setStringList(_kLog, _log.map((SessionSummary x) => jsonEncode(x.toJson())).toList());
    notifyListeners();
  }

  @override
  void dispose() {
    _tick?.cancel();
    _pos?.cancel();
    super.dispose();
  }
}

String fmtClock(int secs) {
  final int h = secs ~/ 3600, m = (secs % 3600) ~/ 60, s = secs % 60;
  final String mm = m.toString().padLeft(2, '0'), ss = s.toString().padLeft(2, '0');
  return h > 0 ? '$h:$mm:$ss' : '$mm:$ss';
}

String fmtPace(double minPerUnit) {
  if (minPerUnit <= 0 || minPerUnit > 99) return '--:--';
  final int m = minPerUnit.floor();
  final int s = ((minPerUnit - m) * 60).round();
  return '$m:${s.toString().padLeft(2, '0')}';
}
