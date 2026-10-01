// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/session.dart';
import '../theme/tokens.dart';

/// Session: pick a sport, start, and the phone streams time, pace, distance
/// and heart rate to the glass once a second. The phone shows the same
/// numbers large, and keeps a log of finished sessions.
class SessionScreen extends StatelessWidget {
  const SessionScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final SessionController ss = context.watch<SessionController>();
    final GoggleConsole g = context.watch<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final String unit = ss.imperial ? 'mi' : 'km';
    final Map<Sport, IconData> icons = <Sport, IconData>{
      Sport.run: Icons.directions_run,
      Sport.ride: Icons.directions_bike,
      Sport.ski: Icons.downhill_skiing,
      Sport.paddle: Icons.kayaking,
      Sport.climb: Icons.terrain,
    };

    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
      children: <Widget>[
        if (!ss.running) ...<Widget>[
          Text(l.sessionTitle, style: t.textTheme.headlineMedium),
          const SizedBox(height: 8),
          Text(l.sessionBody, style: t.textTheme.bodyLarge),
          const SizedBox(height: 16),
          Wrap(
            spacing: 8,
            runSpacing: 8,
            children: <Widget>[
              for (final Sport s in Sport.values)
                ChoiceChip(
                  avatar: Icon(icons[s], size: 18),
                  label: Text(_label(l, s)),
                  selected: ss.sport == s,
                  onSelected: (_) => ss.setSport(s),
                ),
            ],
          ),
          const SizedBox(height: 24),
          FilledButton(onPressed: ss.start, child: Text(l.startSession)),
          const SizedBox(height: 8),
          Text(g.isConnected ? l.sessionGlassReady : l.sessionNoGlass, style: t.textTheme.bodySmall),
        ] else ...<Widget>[
          Row(
            children: <Widget>[
              Icon(icons[ss.sport], color: cs.primary),
              const SizedBox(width: 8),
              Text(_label(l, ss.sport).toUpperCase(), style: t.textTheme.labelMedium?.copyWith(color: cs.primary)),
              const Spacer(),
              if (g.isConnected) Icon(Icons.visibility, size: 18, color: cs.tertiary),
            ],
          ),
          const SizedBox(height: 8),
          Text(fmtClock(ss.seconds), style: t.textTheme.displayLarge?.copyWith(fontFeatures: const <FontFeature>[FontFeature.tabularFigures()])),
          const SizedBox(height: 14),
          Row(
            children: <Widget>[
              Expanded(child: _Cell(label: l.sessionPace(unit), value: fmtPace(ss.paceMinPerUnit))),
              Expanded(child: _Cell(label: unit, value: ss.distanceUnits.toStringAsFixed(2))),
              Expanded(child: _Cell(label: 'bpm', value: ss.hr > 0 ? '${ss.hr}' : '--')),
            ],
          ),
          if (ss.gpsNote != null) ...<Widget>[
            const SizedBox(height: 10),
            Text(ss.gpsNote!, style: t.textTheme.bodySmall),
          ],
          const SizedBox(height: 24),
          FilledButton(
            style: FilledButton.styleFrom(backgroundColor: cs.error, foregroundColor: cs.onError, minimumSize: const Size(64, 64)),
            onPressed: () async {
              final SessionSummary? s = await ss.stop();
              if (s != null && context.mounted) {
                ScaffoldMessenger.of(context).showSnackBar(SnackBar(
                  content: Text(l.sessionDone(fmtClock(s.seconds), s.distanceUnits.toStringAsFixed(2), unit)),
                ));
              }
            },
            child: Text(l.sessionStop),
          ),
        ],
        if (ss.log.isNotEmpty) ...<Widget>[
          const Divider(height: 32),
          Text(l.sessionLogTitle, style: t.textTheme.titleMedium?.copyWith(fontFamily: kFontDisplay, fontSize: 18)),
          const SizedBox(height: 6),
          for (final SessionSummary s in ss.log.take(10))
            ListTile(
              contentPadding: EdgeInsets.zero,
              leading: Icon(icons[s.sport], color: cs.onSurfaceVariant),
              title: Text('${fmtClock(s.seconds)} · ${s.distanceUnits.toStringAsFixed(2)} ${s.imperial ? 'mi' : 'km'} · ${fmtPace(s.avgPaceMinPerUnit)} ${l.sessionPace(s.imperial ? 'mi' : 'km')}'),
              subtitle: Text('${_label(l, s.sport)} · ${s.startedAt.toLocal().toString().substring(0, 16)}'),
            ),
        ],
      ],
    );
  }

  static String _label(AppLocalizations l, Sport s) => switch (s) {
        Sport.run => l.sportRun,
        Sport.ride => l.sportRide,
        Sport.ski => l.sportSki,
        Sport.paddle => l.sportPaddle,
        Sport.climb => l.sportClimb,
      };
}

class _Cell extends StatelessWidget {
  const _Cell({required this.label, required this.value});
  final String label;
  final String value;
  @override
  Widget build(BuildContext context) {
    final ThemeData t = Theme.of(context);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        Text(value, style: t.textTheme.headlineLarge?.copyWith(fontFeatures: const <FontFeature>[FontFeature.tabularFigures()])),
        Text(label, style: t.textTheme.labelMedium),
      ],
    );
  }
}
