// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/hr_strap.dart';

/// The strap's row on Trailhead: a dot, its name, the live bpm, and a tap
/// to the strap page.
class HrCard extends StatelessWidget {
  const HrCard({super.key});

  @override
  Widget build(BuildContext context) {
    final HrStrap h = context.watch<HrStrap>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;

    final String sub = switch (h.link) {
      GoggleLink.disconnected => l.hrCardSub,
      GoggleLink.connecting => l.goggleConnecting,
      GoggleLink.connected => h.bpm > 0 ? l.hrCardLive(h.bpm) : (h.contact == false ? l.hrNoContact : l.hrCardNoReading),
    };
    final Color dot = switch (h.link) {
      GoggleLink.disconnected => cs.onSurfaceVariant,
      GoggleLink.connecting => cs.secondary,
      GoggleLink.connected => cs.tertiary,
    };

    return Card(
      child: InkWell(
        borderRadius: BorderRadius.circular(12),
        onTap: () => context.push('/hr'),
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
                    Text(h.deviceName ?? l.hrCardTitle, style: t.textTheme.titleMedium?.copyWith(fontFamily: 'BricolageGrotesque')),
                    Text(sub, style: t.textTheme.bodySmall),
                  ],
                ),
              ),
              Icon(Icons.favorite_outline, size: 18, color: h.bpm > 0 ? cs.error : cs.onSurfaceVariant),
              const SizedBox(width: 6),
              Icon(Icons.chevron_right, color: cs.onSurfaceVariant),
            ],
          ),
        ),
      ),
    );
  }
}
