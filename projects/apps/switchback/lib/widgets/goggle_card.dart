// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/status.dart';

/// The device card: name, glass theme, data age, city, link state.
class GoggleCard extends StatelessWidget {
  const GoggleCard({super.key});

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final GoggleStatus? st = g.status;

    final String sub = switch (g.link) {
      GoggleLink.disconnected => l.goggleNotConnected,
      GoggleLink.connecting => l.goggleConnecting,
      GoggleLink.connected => st == null
          ? l.goggleConnected
          : '${st.theme.label} · ${_age(l, st.forecastAgeS)}',
    };
    final Color dot = switch (g.link) {
      GoggleLink.disconnected => cs.onSurfaceVariant,
      GoggleLink.connecting => cs.secondary,
      GoggleLink.connected => st?.forecastFresh == true ? cs.tertiary : cs.error,
    };

    return Card(
      child: InkWell(
        borderRadius: BorderRadius.circular(12),
        onTap: () => context.push(g.isConnected ? '/console' : '/pair'),
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
                    Text(g.deviceName ?? 'goggl1', style: t.textTheme.titleMedium?.copyWith(fontFamily: 'BricolageGrotesque')),
                    Text(sub, style: t.textTheme.bodySmall),
                  ],
                ),
              ),
              if (st != null)
                Column(
                  crossAxisAlignment: CrossAxisAlignment.end,
                  children: <Widget>[
                    Text(st.city, style: t.textTheme.bodyMedium),
                    Text('v${st.version}', style: t.textTheme.labelSmall),
                  ],
                ),
              const SizedBox(width: 6),
              Icon(Icons.chevron_right, color: cs.onSurfaceVariant),
            ],
          ),
        ),
      ),
    );
  }

  static String _age(AppLocalizations l, int s) {
    if (s < 0) return l.ageUnknown;
    if (s < 90) return l.ageJustNow;
    if (s < 3600) return l.ageMinutes(s ~/ 60);
    return l.ageHours(s ~/ 3600);
  }
}
