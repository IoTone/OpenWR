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

/// One line that always says what is going on, and the error when there is one.
class _StatusCard extends StatelessWidget {
  const _StatusCard({required this.goggle, required this.scanning, required this.found});
  final GoggleConsole goggle;
  final bool scanning;
  final int found;

  @override
  Widget build(BuildContext context) {
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final bool busy = goggle.link == GoggleLink.connecting || scanning;
    final String line = switch (goggle.phase) {
      'connecting' => l.pairPhaseConnecting(goggle.attempt),
      'reading status' => l.pairPhaseReading,
      'retrying' => l.pairPhaseRetrying(goggle.attempt + 1),
      'connected' => l.pairPhaseConnected,
      'failed' => l.pairPhaseFailed,
      'dropped' => l.pairPhaseDropped,
      _ => scanning ? (found == 0 ? l.pairScanning : l.pairFound(found)) : l.scanIdle,
    };
    final bool bad = goggle.phase == 'failed' || goggle.phase == 'dropped';
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 12),
      decoration: BoxDecoration(
        color: bad ? cs.error.withValues(alpha: .12) : cs.surfaceContainerHigh,
        borderRadius: BorderRadius.circular(10),
        border: Border.all(color: bad ? cs.error : cs.outline),
      ),
      child: Row(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: <Widget>[
          SizedBox(
            width: 18,
            height: 18,
            child: busy
                ? CircularProgressIndicator(strokeWidth: 2, color: cs.primary)
                : Icon(bad ? Icons.error_outline : goggle.isConnected ? Icons.bluetooth_connected : Icons.bluetooth,
                    size: 18, color: bad ? cs.error : goggle.isConnected ? cs.tertiary : cs.onSurfaceVariant),
          ),
          const SizedBox(width: 10),
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: <Widget>[
                Text(line, style: t.textTheme.titleSmall),
                if (bad && goggle.lastError != null) ...<Widget>[
                  const SizedBox(height: 4),
                  Text(goggle.lastError!, style: t.textTheme.bodySmall),
                  const SizedBox(height: 4),
                  Text(l.pairFailedHint, style: t.textTheme.bodySmall),
                ],
              ],
            ),
          ),
        ],
      ),
    );
  }
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
      if (g.status != null && !g.status!.netOk) {
        context.pushReplacement('/setup');
      } else {
        context.pop();
      }
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
          _StatusCard(goggle: g, scanning: sc.isScanning, found: goggles.length),
          const SizedBox(height: 16),
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
