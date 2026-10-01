// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../scanner/scanned_device.dart';
import '../scanner/scanner_controller.dart';
import 'scan_controls.dart';

/// "Hold BOOT for a second. The glass shows a B." Scans for a goggle and
/// connects to the first one it sees; a list appears only if there are
/// several. Pairing on this device is the button, so nothing else to do.
class PairScreen extends StatefulWidget {
  const PairScreen({super.key});

  @override
  State<PairScreen> createState() => _PairScreenState();
}

class _PairScreenState extends State<PairScreen> {
  bool _autoTried = false;

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      final ScannerController sc = context.read<ScannerController>();
      if (!sc.isScanning) unawaited(ScanControls.toggle(context));
    });
  }

  @override
  void dispose() {
    final ScannerController sc = context.read<ScannerController>();
    if (sc.isScanning) unawaited(sc.stop());
    super.dispose();
  }

  Future<void> _connect(ScannedDevice d) async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final ScannerController sc = context.read<ScannerController>();
    await sc.stop();
    await g.connect(d.id, name: d.displayName);
    if (!mounted) return;
    if (g.isConnected) {
      context.pop();
    } else {
      setState(() => _autoTried = true);
      unawaited(sc.start());
    }
  }

  @override
  Widget build(BuildContext context) {
    final ScannerController sc = context.watch<ScannerController>();
    final GoggleConsole g = context.watch<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;

    final List<ScannedDevice> goggles =
        sc.devices.where((ScannedDevice d) => d.name.toLowerCase().startsWith('goggl')).toList();

    // One goggle in range and nobody connecting: that is the one.
    if (goggles.length == 1 && !_autoTried && g.link == GoggleLink.disconnected) {
      _autoTried = true;
      WidgetsBinding.instance.addPostFrameCallback((_) => _connect(goggles.first));
    }

    return Scaffold(
      appBar: AppBar(title: Text(l.pairTitle)),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
        children: <Widget>[
          Text(l.pairHeadline, style: t.textTheme.headlineMedium),
          const SizedBox(height: 10),
          Text(l.pairBody, style: t.textTheme.bodyLarge),
          const SizedBox(height: 20),
          Row(
            children: <Widget>[
              SizedBox(
                width: 18,
                height: 18,
                child: g.link == GoggleLink.connecting || sc.isScanning
                    ? CircularProgressIndicator(strokeWidth: 2, color: cs.primary)
                    : Icon(Icons.bluetooth, size: 18, color: cs.onSurfaceVariant),
              ),
              const SizedBox(width: 10),
              Text(
                g.link == GoggleLink.connecting
                    ? l.goggleConnecting
                    : sc.isScanning
                        ? l.pairScanning
                        : l.scanIdle,
                style: t.textTheme.labelMedium,
              ),
            ],
          ),
          const SizedBox(height: 16),
          if (g.lastError != null) ...<Widget>[
            Text(l.pairFailed, style: t.textTheme.bodyMedium?.copyWith(color: cs.error)),
            const SizedBox(height: 10),
          ],
          for (final ScannedDevice d in goggles)
            Card(
              child: ListTile(
                leading: Icon(Icons.visibility_outlined, color: cs.primary),
                title: Text(d.displayName),
                subtitle: Text(l.rssiDbm(d.rssi)),
                trailing: FilledButton(
                  onPressed: g.link == GoggleLink.connecting ? null : () => _connect(d),
                  child: Text(l.deviceConnect),
                ),
              ),
            ),
          if (goggles.isEmpty && sc.isScanning) ...<Widget>[
            const SizedBox(height: 8),
            Text(l.pairNothingYet, style: t.textTheme.bodySmall),
          ],
          const SizedBox(height: 24),
          OutlinedButton.icon(
            onPressed: () => context.push('/radar'),
            icon: const Icon(Icons.radar),
            label: Text(l.pairFindRadar),
          ),
          const SizedBox(height: 8),
          OutlinedButton(
            onPressed: () => sc.isScanning ? sc.stop() : sc.start(),
            child: Text(sc.isScanning ? l.scanStop : l.scanStart),
          ),
        ],
      ),
    );
  }
}
