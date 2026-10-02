// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The maplet's own page: what it runs and shows, and the way into the
// shared Setup / Update / Console screens, which run against the maplet
// through [MapletScope].
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/maplet.dart';
import '../goggle/status.dart';
import '../theme/tokens.dart';

/// Re-provides the maplet as the [GoggleConsole] below it, so the screens
/// written for the goggle work on the maplet unchanged.
class MapletScope extends StatelessWidget {
  const MapletScope({super.key, required this.child});
  final Widget child;

  @override
  Widget build(BuildContext context) {
    final MapletConsole m = context.watch<MapletConsole>();
    return ChangeNotifierProvider<GoggleConsole>.value(value: m, child: child);
  }
}

class MapletScreen extends StatelessWidget {
  const MapletScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final MapletConsole m = context.watch<MapletConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final GoggleStatus? st = m.status;
    final TextStyle mono = TextStyle(fontFamily: kFontMono, fontSize: 12.5, color: cs.onSurfaceVariant);

    return Scaffold(
      appBar: AppBar(title: Text(m.deviceName ?? l.mapletTitle)),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
        children: <Widget>[
          Text(l.mapletHubBody, style: t.textTheme.bodyLarge),
          const SizedBox(height: 16),
          if (m.isConnected) ...<Widget>[
            Text('v${m.mapletVersion}${st != null ? ' · ${st.netOk ? 'net ${st.ip}' : 'no net'} · heap ${(st.freeHeap / 1024).round()} KB' : ''}', style: mono),
            Text(m.mapletSlot >= 0 ? l.mapletSlot(m.mapletSlot) : l.mapletNoMap, style: mono),
            Text(l.mapletSafeArea(m.safeW, m.safeX), style: mono),
          ] else
            Text(l.mapletNotConnected, style: t.textTheme.bodyMedium),
          const SizedBox(height: 20),
          Wrap(
            spacing: 8,
            runSpacing: 8,
            children: <Widget>[
              FilledButton.tonalIcon(onPressed: m.isConnected && !m.busy ? m.showTestPattern : null, icon: const Icon(Icons.grid_4x4), label: Text(l.mapletShowTest)),
              FilledButton.tonalIcon(onPressed: m.isConnected && !m.busy ? m.showSplash : null, icon: const Icon(Icons.auto_awesome), label: Text(l.mapletShowSplash)),
              FilledButton.tonalIcon(onPressed: () => context.push('/session/0'), icon: const Icon(Icons.map_outlined), label: Text(l.mapletLastPostcard)),
              FilledButton.tonalIcon(onPressed: () => context.push('/plan'), icon: const Icon(Icons.edit_note), label: Text(l.planTitle)),
            ],
          ),
          const Divider(height: 36),
          ListTile(leading: const Icon(Icons.wifi), title: Text(l.mapletSetupWifi), trailing: const Icon(Icons.chevron_right), onTap: () => context.push('/maplet/setup')),
          ListTile(leading: const Icon(Icons.system_update), title: Text(l.mapletUpdate), trailing: const Icon(Icons.chevron_right), onTap: () => context.push('/maplet/update')),
          ListTile(leading: const Icon(Icons.terminal), title: Text(l.mapletConsole), trailing: const Icon(Icons.chevron_right), onTap: () => context.push('/maplet/console')),
          const SizedBox(height: 12),
          if (m.isConnected)
            OutlinedButton(onPressed: m.disconnect, child: Text(l.mapletDisconnect))
          else
            FilledButton(onPressed: () => context.push('/pair-maplet'), child: Text(l.pairMaplet)),
        ],
      ),
    );
  }
}
