// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The phone hosts the release. A tiny HTTP server on the WiFi interface
// serves the app's embedded .mnpkg files to the goggle, which is on the
// same network. No cloud in the loop; RobotARme can take this job later.
import 'dart:async';
import 'dart:io';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart' show rootBundle;
import 'package:network_info_plus/network_info_plus.dart';

import 'mnpkg.dart';

class ReleaseServer extends ChangeNotifier {
  static const int port = 8777;

  HttpServer? _server;
  String? _ip;
  final Map<String, Uint8List> _files = <String, Uint8List>{};
  int _served = 0;
  String? _lastError;

  bool get running => _server != null;
  String? get ip => _ip;
  int get served => _served;
  String? get lastError => _lastError;

  String urlFor(String fileName) => 'http://$_ip:$port/$fileName';

  /// Load the embedded releases: `assets/releases/releases.txt` lists one
  /// asset path per line. Each header is parsed so the UI can show it.
  Future<List<EmbeddedRelease>> listEmbedded() async {
    final List<EmbeddedRelease> out = <EmbeddedRelease>[];
    String index;
    try {
      index = await rootBundle.loadString('assets/releases/releases.txt');
    } catch (_) {
      return out;
    }
    for (final String raw in index.split('\n')) {
      final String asset = raw.trim();
      if (asset.isEmpty || asset.startsWith('#')) continue;
      try {
        final ByteData d = await rootBundle.load(asset);
        final Uint8List bytes = d.buffer.asUint8List(d.offsetInBytes, d.lengthInBytes);
        final MnpkgHeader? h = MnpkgHeader.parse(bytes);
        if (h == null) continue;
        out.add(EmbeddedRelease(asset: asset, fileName: asset.split('/').last, header: h, sizeBytes: bytes.length));
      } catch (_) {}
    }
    return out;
  }

  /// Start serving one release. Returns the URL the goggle should fetch.
  Future<String> serve(EmbeddedRelease r) async {
    final ByteData d = await rootBundle.load(r.asset);
    _files[r.fileName] = d.buffer.asUint8List(d.offsetInBytes, d.lengthInBytes);
    if (_server == null) {
      _ip = await NetworkInfo().getWifiIP();
      if (_ip == null || _ip!.isEmpty) {
        _lastError = 'phone has no WiFi address';
        notifyListeners();
        throw StateError(_lastError!);
      }
      _server = await HttpServer.bind(InternetAddress.anyIPv4, port, shared: true);
      _server!.listen(_handle, onError: (Object e) {
        _lastError = e.toString();
        notifyListeners();
      });
    }
    _lastError = null;
    notifyListeners();
    return urlFor(r.fileName);
  }

  void _handle(HttpRequest req) {
    final String name = req.uri.pathSegments.isEmpty ? '' : req.uri.pathSegments.last;
    final Uint8List? bytes = _files[name];
    if (bytes == null) {
      req.response.statusCode = HttpStatus.notFound;
      req.response.close();
      return;
    }
    req.response.headers.contentType = ContentType('application', 'vnd.magnet.ota-package');
    req.response.headers.contentLength = bytes.length;
    req.response.add(bytes);
    req.response.close();
    _served++;
    notifyListeners();
  }

  Future<void> stop() async {
    await _server?.close(force: true);
    _server = null;
    _files.clear();
    notifyListeners();
  }

  @override
  void dispose() {
    unawaited(stop());
    super.dispose();
  }
}
