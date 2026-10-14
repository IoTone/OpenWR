// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/status.dart';
import '../theme/theme_controller.dart';
import '../theme/tokens.dart';
import '../widgets/forecast_graph.dart';
import '../widgets/goggle_card.dart';
import '../widgets/hr_card.dart';
import '../widgets/maplet_card.dart';

/// Home. The experience, the headline, the device card, the one big button.
/// Everything here comes from one `status` line and the forecast dump.
class TrailheadScreen extends StatelessWidget {
  const TrailheadScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final ThemeController tc = context.watch<ThemeController>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final Experience ex = tc.experience;
    final List<ForecastHour> f = g.forecast;
    final GoggleStatus? st = g.status;
    final bool imperial = st?.imperial ?? true;
    final String unit = imperial ? '°F' : '°C';
    final String speed = imperial ? 'mph' : 'km/h';

    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
      children: <Widget>[
        Row(
          crossAxisAlignment: CrossAxisAlignment.baseline,
          textBaseline: TextBaseline.alphabetic,
          children: <Widget>[
            Text(ex.label.toUpperCase(), style: t.textTheme.labelMedium?.copyWith(color: cs.primary)),
            const SizedBox(width: 10),
            Text(ex.tagline, style: t.textTheme.bodySmall),
            const Spacer(),
            if (tc.pinned != null)
              ActionChip(
                label: Text(l.experiencePinned),
                avatar: const Icon(Icons.push_pin, size: 14),
                onPressed: () => tc.pin(null),
              ),
          ],
        ),
        const SizedBox(height: 14),
        if (ex == Experience.squall && f.isNotEmpty && _threat(f)) ...<Widget>[
          _WarningBar(forecast: f, speed: speed),
          const SizedBox(height: 14),
        ],
        if (f.isNotEmpty) ...<Widget>[
          Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              Text('${f.first.temp.round()}°', style: t.textTheme.displayLarge),
              const SizedBox(width: 10),
              Padding(
                padding: const EdgeInsets.only(top: 12),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: <Widget>[
                    Text(l.feelsLike(f.first.feels.round()), style: t.textTheme.labelMedium),
                    Text(l.hiLo(_max(f), _min(f)), style: t.textTheme.labelMedium),
                    Text(l.windNow(f.first.wind, speed, _dir(f.first.wdir)), style: t.textTheme.labelMedium),
                  ],
                ),
              ),
            ],
          ),
          const SizedBox(height: 6),
          Text(_headline(l, f, unit), style: t.textTheme.bodyLarge),
          const SizedBox(height: 12),
          Wrap(
            spacing: 6,
            runSpacing: 6,
            children: <Widget>[
              Chip(label: Text(_dryOrRain(l, f))),
              Chip(label: Text(l.chipUnits(unit))),
              if (st != null) Chip(label: Text(st.localTime)),
            ],
          ),
          const SizedBox(height: 14),
          SizedBox(
            height: 80,
            child: ForecastGraph(
              forecast: f,
              mode: ex == Experience.squall ? ForecastGraphMode.rain : ForecastGraphMode.temperature,
            ),
          ),
          const SizedBox(height: 16),
        ] else ...<Widget>[
          Text(l.trailheadNoForecastTitle, style: t.textTheme.headlineMedium),
          const SizedBox(height: 8),
          Text(g.isConnected ? l.trailheadWaitingForecast : l.trailheadNotConnected,
              style: t.textTheme.bodyLarge),
          const SizedBox(height: 16),
        ],
        const GoggleCard(),
        const SizedBox(height: 8),
        const MapletCard(),
        const SizedBox(height: 8),
        const HrCard(),
        const SizedBox(height: 16),
        FilledButton(
          onPressed: () => context.push(g.isConnected ? '/session' : '/pair'),
          child: Text(g.isConnected ? l.startSession : l.pairGoggle),
        ),
        const SizedBox(height: 8),
        Row(
          children: <Widget>[
            Expanded(
              child: OutlinedButton(
                onPressed: () => context.push('/looks'),
                child: Text(l.tabLooks),
              ),
            ),
            const SizedBox(width: 8),
            Expanded(
              child: OutlinedButton(
                onPressed: () => context.push('/console'),
                child: Text(l.tabConsole),
              ),
            ),
          ],
        ),
      ],
    );
  }

  /// Something worth a warning bar: likely rain, strong gusts, or a storm.
  static bool _threat(List<ForecastHour> f) =>
      f.any((ForecastHour h) => h.pop >= 50 || h.gust >= 40 || h.stormy);

  static int _max(List<ForecastHour> f) =>
      f.map((ForecastHour h) => h.temp).reduce((double a, double b) => a > b ? a : b).round();
  static int _min(List<ForecastHour> f) =>
      f.map((ForecastHour h) => h.temp).reduce((double a, double b) => a < b ? a : b).round();

  static String _dir(int deg) {
    const List<String> names = <String>['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW'];
    return names[((deg + 22) % 360) ~/ 45];
  }

  static int? _firstRain(List<ForecastHour> f) {
    for (int i = 0; i < f.length; i++) {
      if (f[i].pop >= 50) return i;
    }
    return null;
  }

  static String _dryOrRain(AppLocalizations l, List<ForecastHour> f) {
    final int? i = _firstRain(f);
    return i == null ? l.chipDry8h : l.chipRainAt(_hh(f[i].hour));
  }

  static String _hh(int h) => '${h.toString().padLeft(2, '0')}:00';

  /// The sentence that answers "can I go?": the next change in conditions.
  static String _headline(AppLocalizations l, List<ForecastHour> f, String unit) {
    final int? r = _firstRain(f);
    final ForecastHour h0 = f.first;
    if (h0.stormy) return l.headlineStormNow;
    if (h0.rainy || h0.snowy) {
      final int? clears = _firstIndexWhere(f, (ForecastHour h) => h.pop < 30);
      return clears == null ? l.headlineWetAllWindow : l.headlineClearsAt(_hh(f[clears].hour));
    }
    if (r != null) return l.headlineRainAt(_hh(f[r].hour), f[r].pop);
    final int maxGust = f.map((ForecastHour h) => h.gust).reduce((int a, int b) => a > b ? a : b);
    if (maxGust >= 40) return l.headlineWindy(maxGust);
    return h0.isDay ? l.headlineDryDay : l.headlineDryNight;
  }

  static int? _firstIndexWhere(List<ForecastHour> f, bool Function(ForecastHour) p) {
    for (int i = 0; i < f.length; i++) {
      if (p(f[i])) return i;
    }
    return null;
  }
}

class _WarningBar extends StatelessWidget {
  const _WarningBar({required this.forecast, required this.speed});
  final List<ForecastHour> forecast;
  final String speed;

  @override
  Widget build(BuildContext context) {
    final AppLocalizations l = AppLocalizations.of(context);
    final ColorScheme cs = Theme.of(context).colorScheme;
    final ForecastHour worst = forecast.reduce((ForecastHour a, ForecastHour b) => a.pop >= b.pop ? a : b);
    final int gust = forecast.map((ForecastHour h) => h.gust).reduce((int a, int b) => a > b ? a : b);
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 14, vertical: 10),
      decoration: BoxDecoration(color: cs.error, borderRadius: BorderRadius.circular(10)),
      child: Row(
        children: <Widget>[
          Icon(Icons.warning_amber_rounded, color: cs.onError),
          const SizedBox(width: 10),
          Expanded(
            child: Text(
              l.squallWarning(TrailheadScreen._hh(worst.hour), worst.pop, gust, speed),
              style: TextStyle(color: cs.onError, fontWeight: FontWeight.w700, fontFamily: kFontBody),
            ),
          ),
        ],
      ),
    );
  }
}
