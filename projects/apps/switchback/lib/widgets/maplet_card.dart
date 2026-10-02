// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/maplet.dart';

/// The maplet's row on Trailhead: a dot, its name, what it is doing, and
/// a tap to pair or to the last session's Postcard.
class MapletCard extends StatelessWidget {
  const MapletCard({super.key});

  @override
  Widget build(BuildContext context) {
    final MapletConsole m = context.watch<MapletConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;

    final String sub = switch (m.link) {
      GoggleLink.disconnected => l.mapletCardSub,
      GoggleLink.connecting => l.goggleConnecting,
      GoggleLink.connected => l.mapletConnected(m.safeW),
    };
    final Color dot = switch (m.link) {
      GoggleLink.disconnected => cs.onSurfaceVariant,
      GoggleLink.connecting => cs.secondary,
      GoggleLink.connected => cs.tertiary,
    };

    return Card(
      child: InkWell(
        borderRadius: BorderRadius.circular(12),
        onTap: () => context.push(m.isConnected ? '/maplet' : '/pair-maplet'),
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 12),
          child: Row(
            children: <Widget>[
              Container(width: 10, height: 10, decoration: BoxDecoration(color: dot, shape: BoxShape.circle)),
              const SizedBox(width: 10),
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: <Widget>[
                    Text(m.deviceName ?? l.mapletCardTitle, style: t.textTheme.titleMedium?.copyWith(fontFamily: 'BricolageGrotesque')),
                    Text(sub, style: t.textTheme.bodySmall),
                  ],
                ),
              ),
              if (m.isConnected && m.mapletVersion.isNotEmpty) Text('v${m.mapletVersion}', style: t.textTheme.labelSmall),
              const SizedBox(width: 6),
              Icon(Icons.chevron_right, color: cs.onSurfaceVariant),
            ],
          ),
        ),
      ),
    );
  }
}
