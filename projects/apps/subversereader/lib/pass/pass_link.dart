// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The link to a pass: Nordic UART Service on Bluetooth LE (as in Switchback),
// carrying the pass's ok> console. Commands are console lines; a reply ends
// at the next "ok>" prompt. Packs go over with sd-recv: a header line, then
// raw bytes the pass checks against a CRC before the pack lands on its card.
import 'dart:async';
import 'dart:convert';
import 'dart:io' show Platform;

import 'package:archive/archive.dart' show getCrc32;
import 'package:flutter/foundation.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

class Nus {
  static final Guid service = Guid('6e400001-b5a3-f393-e0a9-e50e24dcca9e');
  static final Guid rx = Guid('6e400002-b5a3-f393-e0a9-e50e24dcca9e');
  static final Guid tx = Guid('6e400003-b5a3-f393-e0a9-e50e24dcca9e');
}

enum LinkState { idle, scanning, connecting, connected }

class ShelfItem {
  ShelfItem(this.index, this.path);
  final int index;
  final String path;
  String get file => path.split('/').last;
}

class PassLink extends ChangeNotifier {
  LinkState state = LinkState.idle;
  String? name;
  String status = 'Not connected';
  String panel = '';
  String reading = '';

  /// From the pass's "pass ..." line (firmware 0.2 on): which panel profile it
  /// wants, how many keys it has (0 = none: this app is its remote), auto-turn
  /// and check-in seconds, storage in KB, and the battery in its own words.
  String profileId = '';
  int keys = -1;
  int autoS = 0;
  int syncS = 0;
  int storeKb = 0;
  int freeKb = 0;
  String battery = '';

  /// Called when a pass says which profile it is, so packs get built for it.
  void Function(String profileId)? onProfile;

  /// Remembered between runs (set by the app): the pass to reconnect to.
  String? lastId;
  void Function(String id)? onConnectedId;
  bool _waiting = false;
  bool get waiting => _waiting;
  List<ShelfItem> shelf = <ShelfItem>[];
  final List<ScanResult> found = <ScanResult>[];

  BluetoothDevice? _dev;
  BluetoothCharacteristic? _rx;
  StreamSubscription<List<int>>? _txSub;
  StreamSubscription<BluetoothConnectionState>? _connSub;
  StreamSubscription<List<ScanResult>>? _scanSub;
  final StringBuffer _in = StringBuffer();
  int _mtu = 23;
  Future<void> _busy = Future<void>.value();

  bool get connected => state == LinkState.connected;

  void _set(LinkState s, String msg) {
    state = s;
    status = msg;
    notifyListeners();
  }

  Future<bool> _permissions() async {
    if (!Platform.isAndroid) return true;
    final Map<Permission, PermissionStatus> r =
        await <Permission>[Permission.bluetoothScan, Permission.bluetoothConnect].request();
    return r.values.every((PermissionStatus s) => s.isGranted);
  }

  /// Look for passes for a few seconds. Passes advertise only while their sync
  /// window is open: power-up, or hold MENU + EXIT.
  Future<void> scan() async {
    if (state == LinkState.scanning) return;
    if (!await _permissions()) {
      _set(LinkState.idle, 'Bluetooth permission refused');
      return;
    }
    if (await FlutterBluePlus.adapterState.first != BluetoothAdapterState.on) {
      if (Platform.isAndroid) {
        try {
          await FlutterBluePlus.turnOn();
        } catch (_) {}
      }
    }
    found.clear();
    _set(LinkState.scanning, 'Looking for passes…');
    await _scanSub?.cancel();
    _scanSub = FlutterBluePlus.onScanResults.listen((List<ScanResult> rs) {
      for (final ScanResult r in rs) {
        final String n = r.advertisementData.advName.isNotEmpty ? r.advertisementData.advName : r.device.platformName;
        // Name only: other gear (mesh radios, bike sensors) offers the same serial service.
        if (!n.startsWith('SR-')) continue;
        final int i = found.indexWhere((ScanResult f) => f.device.remoteId == r.device.remoteId);
        if (i >= 0) {
          found[i] = r;
        } else {
          found.add(r);
        }
      }
      notifyListeners();
    });
    try {
      await FlutterBluePlus.startScan(timeout: const Duration(seconds: 8));
      await FlutterBluePlus.isScanning.where((bool s) => !s).first;
    } catch (e) {
      status = 'Scan failed: $e';
    }
    await _scanSub?.cancel();
    if (state == LinkState.scanning) {
      _set(LinkState.idle, found.isEmpty ? 'No pass found. Open its sync window (see below), then look again.' : 'Pick a pass');
    }
  }

  /// A pass with no keys can't be asked to listen: it wakes by itself every
  /// few minutes and advertises for a few seconds. Keep scanning until it (the
  /// remembered pass, or failing that the first one seen) shows up, then
  /// connect at once.
  Future<void> waitForPass({Duration limit = const Duration(minutes: 16)}) async {
    if (state == LinkState.scanning || state == LinkState.connecting || connected) return;
    if (!await _permissions()) {
      _set(LinkState.idle, 'Bluetooth permission refused');
      return;
    }
    found.clear();
    _waiting = true;
    _set(LinkState.scanning, 'Waiting for the pass to check in…');
    await _scanSub?.cancel();
    ScanResult? hit;
    _scanSub = FlutterBluePlus.onScanResults.listen((List<ScanResult> rs) {
      for (final ScanResult r in rs) {
        final String n = r.advertisementData.advName.isNotEmpty ? r.advertisementData.advName : r.device.platformName;
        if (!n.startsWith('SR-')) continue;
        if (lastId != null && r.device.remoteId.str != lastId) continue;
        hit ??= r;
      }
      if (hit != null) FlutterBluePlus.stopScan();
    });
    try {
      await FlutterBluePlus.startScan(timeout: limit, androidScanMode: AndroidScanMode.lowLatency);
      await FlutterBluePlus.isScanning.where((bool s) => !s).first;
    } catch (e) {
      status = 'Scan failed: $e';
    }
    await _scanSub?.cancel();
    _waiting = false;
    if (hit != null) {
      await connect(hit!);
    } else if (state == LinkState.scanning) {
      _set(LinkState.idle, 'The pass did not check in. Press its reset button, or switch it off and on.');
    }
  }

  Future<void> stopWaiting() async {
    if (!_waiting) return;
    await FlutterBluePlus.stopScan();
  }

  Future<void> connect(ScanResult r) async {
    await FlutterBluePlus.stopScan();
    await disconnect();
    final BluetoothDevice d = r.device;
    name = r.advertisementData.advName.isNotEmpty ? r.advertisementData.advName : d.platformName;
    _set(LinkState.connecting, 'Connecting to $name…');
    try {
      await d.connect(timeout: const Duration(seconds: 15), mtu: null);
      _dev = d;
      try {
        _mtu = Platform.isAndroid ? await d.requestMtu(185) : d.mtuNow;
      } catch (_) {
        _mtu = d.mtuNow;
      }
      if (Platform.isAndroid) {
        try {
          await d.requestConnectionPriority(connectionPriorityRequest: ConnectionPriority.high);
        } catch (_) {}
      }
      BluetoothCharacteristic? rx, tx;
      for (final BluetoothService s in await d.discoverServices()) {
        if (s.uuid != Nus.service) continue;
        for (final BluetoothCharacteristic c in s.characteristics) {
          if (c.uuid == Nus.rx) rx = c;
          if (c.uuid == Nus.tx) tx = c;
        }
      }
      if (rx == null || tx == null) throw StateError('no Nordic UART service');
      _rx = rx;
      await tx.setNotifyValue(true);
      _txSub = tx.onValueReceived.listen((List<int> v) => _in.write(utf8.decode(v, allowMalformed: true)));
      _connSub = d.connectionState.listen((BluetoothConnectionState s) {
        if (s == BluetoothConnectionState.disconnected && state == LinkState.connected) {
          _cleanup();
          _set(LinkState.idle, 'Disconnected from $name');
        }
      });
      _set(LinkState.connected, 'Connected to $name');
      lastId = d.remoteId.str;
      onConnectedId?.call(lastId!);
      await refresh();
    } catch (e) {
      _cleanup();
      try {
        await d.disconnect();
      } catch (_) {}
      _set(LinkState.idle, 'Could not connect: $e');
    }
  }

  void _cleanup() {
    _txSub?.cancel();
    _connSub?.cancel();
    _txSub = null;
    _connSub = null;
    _rx = null;
    _dev = null;
  }

  Future<void> disconnect() async {
    final BluetoothDevice? d = _dev;
    _cleanup();
    if (d != null) {
      try {
        await d.disconnect();
      } catch (_) {}
    }
    if (state == LinkState.connected || state == LinkState.connecting) _set(LinkState.idle, 'Not connected');
  }

  Future<void> _write(List<int> bytes, {bool withResponse = false}) async {
    final BluetoothCharacteristic? rx = _rx;
    if (rx == null) throw StateError('not connected');
    final int chunk = (_mtu - 3).clamp(20, 180);
    for (int off = 0; off < bytes.length; off += chunk) {
      final int end = off + chunk > bytes.length ? bytes.length : off + chunk;
      await rx.write(bytes.sublist(off, end), withoutResponse: !withResponse);
    }
  }

  Future<String?> _waitFor(List<String> marks, Duration t, {int from = 0}) async {
    final DateTime end = DateTime.now().add(t);
    while (DateTime.now().isBefore(end)) {
      final String s = _in.toString();
      if (s.length >= from) {
        // (indexOf throws if `from` is past the end: the echo may not be in yet)
        for (final String m in marks) {
          if (s.indexOf(m, from) >= 0) return s;
        }
      }
      await Future<void>.delayed(const Duration(milliseconds: 30));
    }
    return null;
  }

  /// One console line; returns what the pass printed up to its next prompt.
  Future<String> command(String line, {Duration timeout = const Duration(seconds: 10)}) {
    final Completer<String> done = Completer<String>();
    _busy = _busy.then((_) async {
      try {
        _in.clear();
        await _write(utf8.encode('$line\r'), withResponse: true);
        final String? r = await _waitFor(<String>['ok>'], timeout, from: line.length);
        done.complete(_clean(r ?? _in.toString(), line));
      } catch (e) {
        done.completeError(e);
      }
    });
    return done.future;
  }

  String _clean(String s, String line) {
    String t = s.replaceAll('\r', '');
    if (t.startsWith(line)) t = t.substring(line.length);
    final int p = t.lastIndexOf('ok>');
    if (p >= 0) t = t.substring(0, p);
    return t.trim();
  }

  /// Ask the pass what it has (sr-info) and parse its shelf.
  Future<void> refresh() async {
    if (!connected) return;
    final String r = await command('sr-info');
    final List<ShelfItem> items = <ShelfItem>[];
    for (final String l in r.split('\n')) {
      final RegExpMatch? m = RegExp(r'^\s*(\d+)\s+(/\S+\.svpk)\s*$').firstMatch(l);
      if (m != null) items.add(ShelfItem(int.parse(m.group(1)!), m.group(2)!));
      final RegExpMatch? pm = RegExp(r'panel (\S+)').firstMatch(l);
      if (pm != null) panel = pm.group(1)!.replaceAll(',', '');
      if (l.startsWith('open:')) reading = l.substring(5).trim();
      final RegExpMatch? ps = RegExp(
              r'^pass (\S+) \d+x\d+ keys (\d+) auto (\d+) sync (\d+) store (\d+)/(\d+) KB bat (.*)$')
          .firstMatch(l.trim());
      if (ps != null) {
        profileId = ps.group(1)!;
        keys = int.parse(ps.group(2)!);
        autoS = int.parse(ps.group(3)!);
        syncS = int.parse(ps.group(4)!);
        storeKb = int.parse(ps.group(5)!);
        freeKb = int.parse(ps.group(6)!);
        battery = ps.group(7)!.trim();
        onProfile?.call(profileId);
      }
    }
    // The pass keeps the last zine loaded while it shows the shelf: go by the screen it names.
    if (!r.contains('open:') || r.contains('screen shelf')) reading = '';
    shelf = items;
    notifyListeners();
  }

  /// Send a pack to the pass (its card, or its flash). onProgress gets 0..1.
  Future<void> sendPack(String fileName, Uint8List data, {void Function(double)? onProgress}) {
    final Completer<void> done = Completer<void>();
    _busy = _busy.then((_) async {
      try {
        _in.clear();
        await _write(utf8.encode('sd-recv\r'), withResponse: true);
        final String? ready = await _waitFor(<String>['READY', 'ERR'], const Duration(seconds: 10));
        if (ready == null || !ready.contains('READY')) throw Exception('the pass did not answer: ${ready ?? 'timeout'}');
        final int crc = getCrc32(data);
        _in.clear();
        await _write(utf8.encode('$fileName ${data.length} ${crc.toRadixString(16).padLeft(8, '0')}\n'), withResponse: true);
        final int chunk = (_mtu - 3).clamp(20, 180);
        final BluetoothCharacteristic rx = _rx!;
        int n = 0;
        for (int off = 0; off < data.length; off += chunk) {
          final int end = off + chunk > data.length ? data.length : off + chunk;
          // every 16th packet with a response, so the phone can't run far ahead of the pass
          await rx.write(data.sublist(off, end), withoutResponse: (++n % 16) != 0);
          onProgress?.call(end / data.length);
        }
        final String? r = await _waitFor(<String>['OK ', 'ERR'], const Duration(seconds: 30));
        if (r == null) throw Exception('no reply after sending');
        final String last = r.split('\n').map((String l) => l.trim()).lastWhere(
            (String l) => l.startsWith('OK') || l.startsWith('ERR'),
            orElse: () => r);
        if (!last.startsWith('OK')) throw Exception(last);
        await _waitFor(<String>['ok>'], const Duration(seconds: 3));
        done.complete();
      } catch (e) {
        done.completeError(e);
      }
    });
    return done.future.then((_) async {
      await command('sr-shelf', timeout: const Duration(seconds: 15));
      await refresh();
    });
  }

  Future<void> open(ShelfItem it) async {
    await command('${it.index} sr-open', timeout: const Duration(seconds: 15));
    await refresh();
  }

  // The remote: what a pass with no keys is driven by. Works on any pass.
  Future<void> turn(int d) async {
    await command(d > 0 ? 'sr-next' : 'sr-prev', timeout: const Duration(seconds: 15));
    await refresh();
  }

  /// Put the title and "page N of M" on the glass for a few seconds.
  Future<void> showWhere() async {
    await command('sr-where', timeout: const Duration(seconds: 15));
  }

  Future<void> showShelf() async {
    await command('sr-shelf', timeout: const Duration(seconds: 15));
    await refresh();
  }

  Future<void> setAuto(int seconds) async {
    await command('$seconds sr-auto!');
    await refresh();
  }

  Future<void> setSync(int seconds) async {
    await command('$seconds sr-sync!');
    await refresh();
  }

  /// Let the pass sleep now; it keeps what it shows.
  Future<void> sleepNow() async {
    try {
      await _write(utf8.encode('sr-sleep\r'), withResponse: true);
    } catch (_) {}
    await disconnect();
  }

  Future<void> remove(ShelfItem it) async {
    await command('${it.index} sd-rm');
    await command('sr-shelf', timeout: const Duration(seconds: 15));
    await refresh();
  }
}
