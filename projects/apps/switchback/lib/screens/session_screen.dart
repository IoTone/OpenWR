// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';

/// Session: the sports half. Phase E of the app design installs the
/// `session-hud` bundle and streams pace, distance and heart rate to the
/// glass. Until then this screen says what it will do and lets you pick
/// the sport, so the shell and navigation are real.
class SessionScreen extends StatefulWidget {
  const SessionScreen({super.key});

  @override
  State<SessionScreen> createState() => _SessionScreenState();
}

class _SessionScreenState extends State<SessionScreen> {
  String _sport = 'run';

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final Map<String, IconData> sports = <String, IconData>{
      'run': Icons.directions_run,
      'ride': Icons.directions_bike,
      'ski': Icons.downhill_skiing,
      'paddle': Icons.kayaking,
      'climb': Icons.terrain,
    };
    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
      children: <Widget>[
        Text(l.sessionTitle, style: t.textTheme.headlineMedium),
        const SizedBox(height: 8),
        Text(l.sessionBody, style: t.textTheme.bodyLarge),
        const SizedBox(height: 16),
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: <Widget>[
            for (final MapEntry<String, IconData> s in sports.entries)
              ChoiceChip(
                avatar: Icon(s.value, size: 18),
                label: Text(_label(l, s.key)),
                selected: _sport == s.key,
                onSelected: (_) => setState(() => _sport = s.key),
              ),
          ],
        ),
        const SizedBox(height: 24),
        FilledButton(
          onPressed: g.isConnected
              ? () => ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(l.sessionComingSoon)))
              : null,
          child: Text(l.startSession),
        ),
        const SizedBox(height: 10),
        Text(l.sessionPhaseNote, style: t.textTheme.bodySmall),
      ],
    );
  }

  String _label(AppLocalizations l, String key) => switch (key) {
        'run' => l.sportRun,
        'ride' => l.sportRide,
        'ski' => l.sportSki,
        'paddle' => l.sportPaddle,
        _ => l.sportClimb,
      };
}
