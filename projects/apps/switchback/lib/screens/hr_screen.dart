// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The heart-rate strap: pair one, see it live, forget it. Scans with the
// shared scanner and offers anything that advertises the Heart Rate
// service or carries a strap's name; one in range connects by itself.
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/hr_strap.dart';
import '../scanner/scanned_device.dart';
import '../scanner/scanner_controller.dart';
import 'scan_controls.dart';

class HrScreen extends StatefulWidget {
  const HrScreen({super.key});

  @override
  State<HrScreen> createState() => _HrScreenState();
}

class _HrScreenState extends State<HrScreen> {
  bool _autoTried = false;

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) => _scanIfNeeded());
  }

  void _scanIfNeeded() {
    if (!mounted) return;
    final ScannerController sc = context.read<ScannerController>();
    if (!context.read<HrStrap>().isConnected && !sc.isScanning) unawaited(ScanControls.toggle(context));
  }

  @override
  void dispose() {
    final ScannerController sc = context.read<ScannerController>();
    if (sc.isScanning) unawaited(sc.stop());
    super.dispose();
  }

  Future<void> _connect(ScannedDevice d) async {
    final HrStrap h = context.read<HrStrap>();
    await context.read<ScannerController>().stop();
    await h.connect(d.id, name: d.displayName);
    if (mounted && !h.isConnected) {
      setState(() => _autoTried = true);
      _scanIfNeeded();
    }
  }

  @override
  Widget build(BuildContext context) {
    final HrStrap h = context.watch<HrStrap>();
    final ScannerController sc = context.watch<ScannerController>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;

    final List<ScannedDevice> straps =
        sc.devices.where((ScannedDevice d) => looksLikeHrStrap(d.name, d.serviceUuids)).toList();
    if (straps.length == 1 && !_autoTried && h.link == GoggleLink.disconnected) {
      _autoTried = true;
      WidgetsBinding.instance.addPostFrameCallback((_) => _connect(straps.first));
    }

    return Scaffold(
      appBar: AppBar(title: Text(h.deviceName ?? l.hrTitle)),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
        children: <Widget>[
          Text(l.hrBody, style: t.textTheme.bodyLarge),
          const SizedBox(height: 20),
          if (h.isConnected) ...<Widget>[
            Row(
              crossAxisAlignment: CrossAxisAlignment.end,
              children: <Widget>[
                Icon(Icons.favorite, color: h.bpm > 0 ? cs.error : cs.onSurfaceVariant, size: 36),
                const SizedBox(width: 12),
                Text(h.bpm > 0 ? '${h.bpm}' : '--',
                    style: t.textTheme.displayLarge?.copyWith(fontFeatures: const <FontFeature>[FontFeature.tabularFigures()])),
                const SizedBox(width: 8),
                Padding(padding: const EdgeInsets.only(bottom: 10), child: Text('bpm', style: t.textTheme.titleMedium)),
              ],
            ),
            const SizedBox(height: 8),
            if (h.bpm == 0) Text(h.contact == false ? l.hrNoContact : l.hrWaiting, style: t.textTheme.bodyMedium),
            if (h.battery >= 0) Text(l.hrBattery(h.battery), style: t.textTheme.bodySmall),
            const SizedBox(height: 24),
            OutlinedButton(onPressed: h.disconnect, child: Text(l.hrDisconnect)),
            TextButton(onPressed: h.forget, child: Text(l.hrForget)),
          ] else ...<Widget>[
            if (h.link == GoggleLink.connecting) ...<Widget>[
              Row(children: <Widget>[
                const SizedBox(width: 18, height: 18, child: CircularProgressIndicator(strokeWidth: 2)),
                const SizedBox(width: 10),
                Text(l.goggleConnecting, style: t.textTheme.titleSmall),
              ]),
              const SizedBox(height: 12),
            ] else if (sc.isScanning) ...<Widget>[
              Text(l.hrScanning, style: t.textTheme.titleSmall),
              const SizedBox(height: 12),
            ],
            if (h.lastError != null) ...<Widget>[
              Text(h.lastError!, style: t.textTheme.bodySmall?.copyWith(color: cs.error)),
              const SizedBox(height: 12),
            ],
            for (final ScannedDevice d in straps)
              Card(
                child: ListTile(
                  leading: Icon(Icons.favorite_outline, color: cs.primary),
                  title: Text(d.displayName),
                  subtitle: Text(l.rssiDbm(d.rssi)),
                  trailing: FilledButton(
                    onPressed: h.link == GoggleLink.connecting ? null : () => _connect(d),
                    child: Text(l.deviceConnect),
                  ),
                ),
              ),
            if (straps.isEmpty && sc.isScanning) Text(l.hrNoneYet, style: t.textTheme.bodySmall),
            const SizedBox(height: 16),
            if (h.deviceId != null && h.link == GoggleLink.disconnected)
              OutlinedButton(onPressed: () => h.connect(h.deviceId!, name: h.deviceName), child: Text(l.hrReconnect)),
            OutlinedButton(
              onPressed: () => sc.isScanning ? sc.stop() : sc.start(),
              child: Text(sc.isScanning ? l.scanStop : l.scanStart),
            ),
          ],
        ],
      ),
    );
  }
}
