// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// Plan: the Field Notes page before a session. Pick a past session as the
// route, or map where you are with a planned distance as a ring. The
// estimated time comes from your own pace for that sport; sunset from the
// phone's clock and position. Send puts it in the maplet's slot 1.
import 'dart:async';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:geolocator/geolocator.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/maplet.dart';
import '../goggle/session.dart';
import '../map/eink_canvas.dart';
import '../map/map_render.dart';
import '../map/sun.dart';
import 'session_detail_screen.dart';

class PlanScreen extends StatefulWidget {
  const PlanScreen({super.key});

  @override
  State<PlanScreen> createState() => _PlanScreenState();
}

class _PlanScreenState extends State<PlanScreen> {
  int? _routeIndex; // a session in the log, or null for "where I am"
  double _radiusKm = 5;
  EinkCanvas? _canvas;
  ui.Image? _preview;
  String? _note;
  bool _busy = false;
  double _progress = 0;
  String _key = '';

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    final List<SessionSummary> log = context.read<SessionController>().log;
    _routeIndex ??= log.indexWhere((SessionSummary s) => s.track.length >= 2) >= 0
        ? log.indexWhere((SessionSummary s) => s.track.length >= 2)
        : null;
    _maybeRender();
  }

  void _maybeRender() {
    final MapletConsole m = context.read<MapletConsole>();
    final int w = m.isConnected ? m.safeW : MapletConsole.width;
    final String key = '$_routeIndex/$_radiusKm/$w';
    if (key == _key || _busy) return;
    _key = key;
    unawaited(_render(w));
  }

  Future<void> _render(int w) async {
    setState(() {
      _busy = true;
      _note = null;
    });
    final AppLocalizations l = AppLocalizations.of(context);
    final SessionController sc = context.read<SessionController>();
    try {
      EinkCanvas c;
      if (_routeIndex != null && _routeIndex! < sc.log.length) {
        final SessionSummary s = sc.log[_routeIndex!];
        final double km = s.meters / 1000;
        final double pace = _paceMinPerKm(sc.log, s.sport);
        final int estMin = (km * pace).round();
        final (double, double) start = s.track.first;
        final SunTimes sun = SunTimes.forDay(DateTime.now(), start.$1, start.$2);
        final String sunset = sun.sunset == null ? '--:--' : _hhmm(sun.sunset!);
        final bool imp = sc.imperial;
        c = await MapRenderer().fieldNotes(
          FieldNotesSpec(
            title: l.planTitle.toUpperCase(),
            subtitle: sessionTitle(s, l).toUpperCase(),
            rows: <(String, String)>[
              ('DIST', imp ? '${(km / 1.609344).toStringAsFixed(1)}mi' : '${km.toStringAsFixed(1)}km'),
              ('UP', '+${s.climbMeters.round()}m'),
              ('EST', estMin >= 60 ? '${estMin ~/ 60}h${(estMin % 60).toString().padLeft(2, '0')}' : '${estMin}min'),
              ('SUN', sunset),
            ],
            track: s.track,
          ),
          w,
          MapletConsole.height,
        );
      } else {
        final Position p = await _position();
        final SunTimes sun = SunTimes.forDay(DateTime.now(), p.latitude, p.longitude);
        final bool imp = sc.imperial;
        final double r = _radiusKm * 1000;
        c = await MapRenderer().here(
          HereSpec(
            lat: p.latitude,
            lon: p.longitude,
            radiusM: r / 2, // a loop of the planned distance fits in a ring of half of it
            title: l.planHereTitle.toUpperCase(),
            rows: <(String, String)>[
              ('LOOP', imp ? '${(_radiusKm / 1.609344).toStringAsFixed(0)}mi' : '${_radiusKm.toStringAsFixed(0)}km'),
              ('ALT', '${p.altitude.round()}m'),
              ('SUN', sun.sunset == null ? '--:--' : _hhmm(sun.sunset!)),
              ('NOW', _hhmm(DateTime.now())),
            ],
          ),
          w,
          MapletConsole.height,
        );
      }
      final ui.Image img = await c.toImage();
      if (!mounted) return;
      setState(() {
        _canvas = c;
        _preview = img;
      });
    } catch (e) {
      if (mounted) setState(() => _note = e.toString());
    } finally {
      if (mounted) setState(() => _busy = false);
    }
  }

  static Future<Position> _position() async {
    LocationPermission p = await Geolocator.checkPermission();
    if (p == LocationPermission.denied) p = await Geolocator.requestPermission();
    if (p == LocationPermission.denied || p == LocationPermission.deniedForever) throw StateError('no location permission');
    return Geolocator.getCurrentPosition(locationSettings: const LocationSettings(accuracy: LocationAccuracy.high, timeLimit: Duration(seconds: 20)));
  }

  /// Your average pace for a sport from the log, else a sensible default.
  static double _paceMinPerKm(List<SessionSummary> log, Sport sport) {
    double km = 0, min = 0;
    for (final SessionSummary s in log) {
      if (s.sport == sport && s.meters > 500 && s.seconds > 60) {
        km += s.meters / 1000;
        min += s.seconds / 60;
      }
    }
    if (km > 1) return min / km;
    return switch (sport) {
      Sport.run => 7.0,
      Sport.ride => 3.0,
      Sport.ski => 4.0,
      Sport.paddle => 10.0,
      Sport.climb => 20.0,
    };
  }

  static String _hhmm(DateTime t) => '${t.hour.toString().padLeft(2, '0')}:${t.minute.toString().padLeft(2, '0')}';

  Future<void> _send() async {
    final MapletConsole m = context.read<MapletConsole>();
    final EinkCanvas? c = _canvas;
    if (c == null || !m.isConnected) return;
    setState(() {
      _busy = true;
      _progress = 0;
      _note = null;
    });
    try {
      final EinkCanvas full = EinkCanvas(width: MapletConsole.width, height: MapletConsole.height);
      full.blit(c, m.safeX, 0);
      await m.pushFrame(full.pack(), slot: 1, onProgress: (double p) {
        if (mounted) setState(() => _progress = p);
      });
      if (mounted) setState(() => _note = AppLocalizations.of(context).mapletSent);
    } catch (e) {
      if (mounted) setState(() => _note = e.toString());
    } finally {
      if (mounted) setState(() => _busy = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final MapletConsole m = context.watch<MapletConsole>();
    final List<SessionSummary> log = context.watch<SessionController>().log;
    final List<(int, SessionSummary)> routes = <(int, SessionSummary)>[
      for (final (int i, SessionSummary s) in log.indexed)
        if (s.track.length >= 2) (i, s),
    ];
    WidgetsBinding.instance.addPostFrameCallback((_) => _maybeRender());

    return Scaffold(
      appBar: AppBar(title: Text(l.planTitle)),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
        children: <Widget>[
          Text(l.planBody, style: t.textTheme.bodyLarge),
          const SizedBox(height: 14),
          Wrap(
            spacing: 8,
            runSpacing: 8,
            children: <Widget>[
              ChoiceChip(
                avatar: const Icon(Icons.my_location, size: 18),
                label: Text(l.planHere),
                selected: _routeIndex == null,
                onSelected: (_) => setState(() => _routeIndex = null),
              ),
              for (final (int i, SessionSummary s) in routes.take(6))
                ChoiceChip(
                  avatar: const Icon(Icons.route, size: 18),
                  label: Text(sessionTitle(s, l)),
                  selected: _routeIndex == i,
                  onSelected: (_) => setState(() => _routeIndex = i),
                ),
            ],
          ),
          if (_routeIndex == null) ...<Widget>[
            const SizedBox(height: 10),
            Row(
              children: <Widget>[
                Text(l.planLoop, style: t.textTheme.labelMedium),
                const SizedBox(width: 8),
                for (final double km in const <double>[3, 5, 10, 20])
                  Padding(
                    padding: const EdgeInsets.only(right: 6),
                    child: ChoiceChip(
                      label: Text('${km.round()} km'),
                      selected: _radiusKm == km,
                      onSelected: (_) => setState(() => _radiusKm = km),
                    ),
                  ),
              ],
            ),
          ],
          const SizedBox(height: 16),
          Text(l.mapletFieldNotes.toUpperCase(), style: t.textTheme.labelMedium?.copyWith(color: cs.primary, letterSpacing: 1.2)),
          const SizedBox(height: 8),
          AspectRatio(
            aspectRatio: 212 / 104,
            child: Container(
              decoration: BoxDecoration(color: const Color(0xFFECEBE4), borderRadius: BorderRadius.circular(4), border: Border.all(color: Colors.black26)),
              clipBehavior: Clip.antiAlias,
              child: _preview != null
                  ? FittedBox(fit: BoxFit.contain, child: RawImage(image: _preview, filterQuality: FilterQuality.none, scale: 1 / 3))
                  : Center(child: _busy ? const CircularProgressIndicator() : Text(_note ?? '', style: const TextStyle(color: Colors.black87))),
            ),
          ),
          const SizedBox(height: 6),
          Text(m.isConnected ? l.mapletGlassArea(m.safeW, MapletConsole.height) : l.mapletNotConnected, style: t.textTheme.bodySmall),
          const SizedBox(height: 20),
          if (m.isConnected)
            FilledButton.icon(
              onPressed: (_canvas == null || _busy) ? null : _send,
              icon: _busy && _progress > 0
                  ? SizedBox(width: 18, height: 18, child: CircularProgressIndicator(strokeWidth: 2, value: _progress))
                  : const Icon(Icons.send_to_mobile),
              label: Text(_busy && _progress > 0 ? l.mapletSending((_progress * 100).round()) : l.mapletSend),
            )
          else
            OutlinedButton.icon(
              onPressed: () => context.push('/pair-maplet'),
              icon: const Icon(Icons.map_outlined),
              label: Text(l.pairMaplet),
            ),
          if (_note != null && _preview != null) ...<Widget>[
            const SizedBox(height: 8),
            Text(_note!, style: t.textTheme.bodySmall),
          ],
        ],
      ),
    );
  }
}
