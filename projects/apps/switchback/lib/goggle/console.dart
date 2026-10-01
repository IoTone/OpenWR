// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The goggle console: one connection, a transcript, and `send()` which
// writes a line and collects what the device prints until its `ok>` prompt.
// Every screen in the app is a form over this.
import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'nus_transport.dart';
import 'status.dart';

enum GoggleLink { disconnected, connecting, connected }

class GoggleLocation {
  const GoggleLocation({required this.lat, required this.lon, required this.manual});
  final double lat;
  final double lon;
  final bool manual;
}

/// One entry in the transcript: what we sent or what the glass said.
class ConsoleEntry {
  const ConsoleEntry({required this.at, required this.text, required this.sent});
  final DateTime at;
  final String text;
  final bool sent;
}

class GoggleConsole extends ChangeNotifier {
  GoggleConsole({required ConsoleTransport transport}) : _t = transport {
    _inSub = _t.incoming.listen(_onIncoming);
    _connSub = _t.connected.listen(_onConnected);
  }

  final ConsoleTransport _t;
  StreamSubscription<String>? _inSub;
  StreamSubscription<bool>? _connSub;

  static const String _kLastId = 'goggle.lastId';
  static const String _kLastName = 'goggle.lastName';

  GoggleLink _link = GoggleLink.disconnected;
  String _phase = '';
  int _attempt = 0;
  String? _deviceId;
  String? _deviceName;
  String? _lastError;
  GoggleStatus? _status;
  List<ForecastHour> _forecast = const <ForecastHour>[];
  final List<ConsoleEntry> _transcript = <ConsoleEntry>[];
  static const int _transcriptCap = 150;

  // Notifications are coalesced: a wx-dump arrives as a dozen BLE chunks in
  // a few ms, and every notify rebuilds every live tab. Cheap phones ANR.
  Timer? _notifyTimer;
  void _notifySoon() {
    if (_notifyTimer != null) return;
    _notifyTimer = Timer(const Duration(milliseconds: 120), () {
      _notifyTimer = null;
      notifyListeners();
    });
  }

  // Reply assembly for the command in flight.
  final StringBuffer _pending = StringBuffer();
  String _inFlight = '';
  Completer<String>? _reply;
  Timer? _replyTimeout;
  Timer? _statusTimer;

  GoggleLink get link => _link;

  /// What the link is doing right now, for the Pair screen.
  String get phase => _phase;
  int get attempt => _attempt;
  bool get isConnected => _link == GoggleLink.connected;
  String? get deviceId => _deviceId;
  String? get deviceName => _deviceName;
  String? get lastError => _lastError;
  GoggleStatus? get status => _status;
  List<ForecastHour> get forecast => _forecast;
  List<ConsoleEntry> get transcript => List<ConsoleEntry>.unmodifiable(_transcript);

  /// Connect, then read the first `status` so Trailhead has something to say.
  /// Two attempts: the first link after the goggle starts advertising tends
  /// to drop within a second on Android, the second holds.
  Future<void> connect(String deviceId, {String? name, int attempts = 2}) async {
    if (_link == GoggleLink.connecting) return;
    _deviceId = deviceId;
    _deviceName = name;
    _lastError = null;
    for (int i = 1; i <= attempts; i++) {
      _attempt = i;
      _link = GoggleLink.connecting;
      _phase = 'connecting';
      notifyListeners();
      try {
        await _t.connect(deviceId);
        _link = GoggleLink.connected;
        _phase = 'reading status';
        notifyListeners();
        await refresh();
        if (_link != GoggleLink.connected) {
          _lastError = 'link dropped while reading status';
          throw StateError(_lastError!);
        }
        _phase = 'connected';
        _lastError = null;
        _statusTimer?.cancel();
        _statusTimer = Timer.periodic(const Duration(seconds: 30), (_) => refresh());
        notifyListeners();
        unawaited(_remember(deviceId, name));
        return;
      } catch (e) {
        _link = GoggleLink.disconnected;
        _lastError = _shortError(e);
        _phase = i < attempts ? 'retrying' : 'failed';
        notifyListeners();
        await _t.disconnect();
        if (i < attempts) await Future<void>.delayed(const Duration(milliseconds: 900));
      }
    }
  }

  static String _shortError(Object e) {
    final String s = e.toString();
    if (s.contains('133')) return 'Android GATT error 133 (first-connect glitch)';
    if (s.contains('TimeoutException') || s.contains('timeout')) return 'timed out: is the goggle advertising? (B on the glass)';
    if (s.contains('No Nordic UART')) return 'that device is not a goggle';
    return s.replaceFirst('StateError: ', '').replaceFirst('Exception: ', '');
  }

  Future<void> _remember(String id, String? name) async {
    final SharedPreferences p = await SharedPreferences.getInstance();
    await p.setString(_kLastId, id);
    if (name != null) await p.setString(_kLastName, name);
  }

  /// The goggle paired last time, if any.
  Future<(String, String?)?> remembered() async {
    final SharedPreferences p = await SharedPreferences.getInstance();
    final String? id = p.getString(_kLastId);
    return id == null ? null : (id, p.getString(_kLastName));
  }

  /// Quietly try the remembered goggle (app start, app resumed). One attempt;
  /// the goggle re-advertises for three minutes after any drop, so this
  /// usually lands without anyone touching anything.
  Future<void> reconnectIfNeeded() async {
    if (_link != GoggleLink.disconnected) return;
    final (String, String?)? r = await remembered();
    if (r == null) return;
    await connect(r.$1, name: r.$2, attempts: 1);
  }

  Future<void> disconnect() async {
    _statusTimer?.cancel();
    _statusTimer = null;
    await _t.disconnect();
    _onConnected(false);
  }

  /// Send one line and return everything printed before the next prompt
  /// (without the prompt). Lines the engine rejects come back with `?`.
  Future<String> send(String line, {Duration timeout = const Duration(seconds: 20)}) async {
    if (!isConnected) throw StateError('Not connected');
    // One command at a time: the device has one prompt.
    while (_reply != null) {
      await _reply!.future.catchError((_) => '');
    }
    _record(line, sent: true);
    _pending.clear();
    _inFlight = line.trim();
    final Completer<String> c = Completer<String>();
    _reply = c;
    _replyTimeout = Timer(timeout, () {
      if (!c.isCompleted) c.completeError(TimeoutException('no ok> within $timeout'));
      _reply = null;
    });
    try {
      await _t.writeLine(line);
      return await c.future;
    } finally {
      _replyTimeout?.cancel();
      _replyTimeout = null;
    }
  }

  /// `status` and the forecast, which Trailhead and the experience need.
  Future<void> refresh() async {
    if (!isConnected) return;
    try {
      final String s = await send('status');
      for (final String l in s.split('\n')) {
        final GoggleStatus? st = GoggleStatus.parse(l);
        if (st != null) _status = st;
      }
      if (_status?.forecastOk == true) {
        final String d = await send('wx-dump');
        final List<ForecastHour> f = parseForecastDump(d);
        if (f.isNotEmpty) _forecast = f;
      }
      notifyListeners();
    } catch (e) {
      _lastError = e.toString();
      notifyListeners();
    }
  }

  // --- the forms' verbs -----------------------------------------------

  Future<void> setTheme(GlassTheme t) async {
    await send('${t.index} theme!');
    await refresh();
  }

  Future<void> setAnimation(bool on) => send('${on ? -1 : 0} anim-on !');

  Future<void> setContrast(int v) => send('${v.clamp(0, 255)} oled-contrast');

  Future<void> setUnits({required bool imperial}) async {
    await send('${imperial ? 1 : 0} units!');
    await refresh();
  }

  Future<void> setWifi(String ssid, String password) async {
    await send('s" ${_q(ssid)}" s" ${_q(password)}" wifi!');
  }

  Future<void> setLocation(double lat, double lon) async {
    await send('${(lat * 10000).round()} ${(lon * 10000).round()} geo!');
    await send('wx-fetch');
    await refresh();
  }

  Future<void> openPairingWindow() => send('pair');

  /// Where the goggle thinks it is, and whether that was set by hand.
  Future<GoggleLocation?> readLocation() async {
    final String r = await send('geo-lat . geo-lon . geo-manual? .');
    final List<int> n = r.trim().split(RegExp(r'\s+')).map((String s) => int.tryParse(s)).whereType<int>().toList();
    if (n.length < 3) return null;
    return GoggleLocation(lat: n[0] / 10000, lon: n[1] / 10000, manual: n[2] != 0);
  }

  /// Back to IP geolocation; true if the lookup and the forecast both came in.
  Future<bool> setAutoLocation() async {
    final String r = await send('geo-auto .', timeout: const Duration(seconds: 40));
    await refresh();
    return r.trim().endsWith('-1');
  }

  /// Poll status until the goggle reports an IP, or give up.
  Future<bool> waitForNetwork(Duration limit) async {
    final DateTime end = DateTime.now().add(limit);
    while (DateTime.now().isBefore(end)) {
      await Future<void>.delayed(const Duration(seconds: 3));
      await refresh();
      if (_status?.netOk == true) return true;
    }
    return false;
  }

  /// Install a use-case bundle: the lines are captured by the device and
  /// applied all or nothing. Returns the device's verdict text.
  Future<String> installBundle(List<String> lines) async {
    await send('bundle-begin', timeout: const Duration(seconds: 5))
        .catchError((_) => '');
    for (final String l in lines) {
      await _t.writeLine(l);
      _record(l, sent: true);
      await Future<void>.delayed(const Duration(milliseconds: 30));
    }
    _pending.clear();
    final Completer<String> c = Completer<String>();
    _reply = c;
    _replyTimeout = Timer(const Duration(seconds: 60), () {
      if (!c.isCompleted) c.completeError(TimeoutException('bundle apply timed out'));
      _reply = null;
    });
    await _t.writeLine('bundle-end');
    _record('bundle-end', sent: true);
    try {
      return await c.future;
    } finally {
      _replyTimeout?.cancel();
    }
  }

  /// A Forth string literal cannot contain a double quote.
  static String _q(String s) => s.replaceAll('"', '');

  // --- plumbing ---------------------------------------------------------

  void _onIncoming(String chunk) {
    _pending.write(chunk);
    final String text = _pending.toString();
    // The engine ends every reply with "ok> " (prompt, no newline).
    final int at = text.lastIndexOf('ok>');
    if (at >= 0 && _reply != null) {
      String body = text.substring(0, at).replaceAll('\r', '').trim();
      // The console echoes the line we sent; the transcript already has it.
      if (_inFlight.isNotEmpty && body.startsWith(_inFlight)) {
        body = body.substring(_inFlight.length).trimLeft();
      }
      final Completer<String> c = _reply!;
      _reply = null;
      _inFlight = '';
      _pending.clear();
      _record(body, sent: false);
      if (!c.isCompleted) c.complete(body);
    } else if (_reply == null && text.contains('\n')) {
      // Unsolicited output (fetch logs, animation chatter): show it.
      _record(text.replaceAll('\r', '').trim(), sent: false);
      _pending.clear();
    }
  }

  void _onConnected(bool up) {
    if (up) return;
    final bool was = _link != GoggleLink.disconnected;
    if (was && _phase == 'connected') _phase = 'dropped';
    _link = GoggleLink.disconnected;
    _statusTimer?.cancel();
    _statusTimer = null;
    final Completer<String>? c = _reply;
    _reply = null;
    if (c != null && !c.isCompleted) c.completeError(StateError('disconnected'));
    if (was) notifyListeners();
  }

  void _record(String text, {required bool sent}) {
    if (text.isEmpty) return;
    _transcript.add(ConsoleEntry(at: DateTime.now(), text: text, sent: sent));
    if (_transcript.length > _transcriptCap) _transcript.removeAt(0);
    _notifySoon();
  }

  void clearTranscript() {
    _transcript.clear();
    notifyListeners();
  }

  @override
  void dispose() {
    _notifyTimer?.cancel();
    _statusTimer?.cancel();
    _replyTimeout?.cancel();
    _inSub?.cancel();
    _connSub?.cancel();
    _t.dispose();
    super.dispose();
  }
}
