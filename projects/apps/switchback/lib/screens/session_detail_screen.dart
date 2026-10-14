// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// One finished session: its numbers, the Postcard as the maplet will show
// it, and Send to maplet. The preview is the real 1-bit render, so what
// you see is what lands on the glass.
import 'dart:async';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/maplet.dart';
import '../goggle/session.dart';
import '../map/eink_canvas.dart';
import '../map/map_render.dart';
import '../theme/tokens.dart';

class SessionDetailScreen extends StatefulWidget {
  const SessionDetailScreen({super.key, required this.index});
  final int index;

  @override
  State<SessionDetailScreen> createState() => _SessionDetailScreenState();
}

class _SessionDetailScreenState extends State<SessionDetailScreen> {
  EinkCanvas? _canvas;
  ui.Image? _preview;
  String? _renderError;
  bool _rendering = false;
  bool _sending = false;
  double _progress = 0;
  String? _sendNote;
  int _renderedForW = -1;

  @override
  void didChangeDependencies() {
    super.didChangeDependencies();
    final MapletConsole m = context.watch<MapletConsole>();
    // Render for the glass that is connected, or the full panel if none.
    final int w = m.isConnected ? m.safeW : m.width;
    if (w != _renderedForW && !_rendering) {
      _renderedForW = w;
      unawaited(_render(w));
    }
  }

  SessionSummary? get _session {
    final List<SessionSummary> log = context.read<SessionController>().log;
    return widget.index < log.length ? log[widget.index] : null;
  }

  Future<void> _render(int w) async {
    final SessionSummary? s = _session;
    if (s == null) return;
    setState(() {
      _rendering = true;
      _renderError = null;
    });
    try {
      final EinkCanvas c = await MapRenderer().postcard(postcardFor(s, AppLocalizations.of(context)), w, context.read<MapletConsole>().height);
      final ui.Image img = await c.toImage();
      if (!mounted) return;
      setState(() {
        _canvas = c;
        _preview = img;
      });
    } catch (e) {
      if (mounted) setState(() => _renderError = e.toString());
    } finally {
      if (mounted) setState(() => _rendering = false);
    }
  }

  Future<void> _send() async {
    final MapletConsole m = context.read<MapletConsole>();
    final EinkCanvas? c = _canvas;
    if (c == null || !m.isConnected) return;
    setState(() {
      _sending = true;
      _progress = 0;
      _sendNote = null;
    });
    try {
      final EinkCanvas full = EinkCanvas(width: m.width, height: m.height);
      full.blit(c, m.safeX, 0);
      await m.pushFrame(full.pack(), slot: 0, onProgress: (double p) {
        if (mounted) setState(() => _progress = p);
      });
      if (mounted) setState(() => _sendNote = AppLocalizations.of(context).mapletSent);
    } catch (e) {
      if (mounted) setState(() => _sendNote = e.toString());
    } finally {
      if (mounted) setState(() => _sending = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final MapletConsole m = context.watch<MapletConsole>();
    final SessionSummary? s = _session;
    if (s == null) {
      return Scaffold(appBar: AppBar(title: Text(l.sessionTitle)), body: Center(child: Text(l.sessionLogTitle)));
    }
    final String unit = s.imperial ? 'mi' : 'km';
    return Scaffold(
      appBar: AppBar(title: Text(sessionTitle(s, l))),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
        children: <Widget>[
          Text(fmtClock(s.seconds), style: t.textTheme.displayLarge?.copyWith(fontFeatures: const <FontFeature>[FontFeature.tabularFigures()])),
          const SizedBox(height: 8),
          Text('${s.distanceUnits.toStringAsFixed(2)} $unit · ${fmtPace(s.avgPaceMinPerUnit)} ${l.sessionPace(unit)} · +${s.climbMeters.round()} m',
              style: t.textTheme.bodyLarge),
          Text('${s.startedAt.toLocal().toString().substring(0, 16)} · ${s.track.length} ${l.mapletTrackPoints}', style: t.textTheme.bodySmall),
          const SizedBox(height: 20),
          Text(l.mapletPostcard.toUpperCase(), style: t.textTheme.labelMedium?.copyWith(color: cs.primary, letterSpacing: 1.2)),
          const SizedBox(height: 8),
          _Glass(preview: _preview, rendering: _rendering, error: _renderError),
          const SizedBox(height: 6),
          Text(
            m.isConnected ? l.mapletGlassArea(m.safeW, m.height) : l.mapletNotConnected,
            style: t.textTheme.bodySmall,
          ),
          const SizedBox(height: 20),
          if (m.isConnected)
            FilledButton.icon(
              onPressed: (_canvas == null || _sending) ? null : _send,
              icon: _sending
                  ? SizedBox(width: 18, height: 18, child: CircularProgressIndicator(strokeWidth: 2, value: _progress > 0 ? _progress : null))
                  : const Icon(Icons.send_to_mobile),
              label: Text(_sending ? l.mapletSending((_progress * 100).round()) : l.mapletSend),
            )
          else
            OutlinedButton.icon(
              onPressed: () => context.push('/pair-maplet'),
              icon: const Icon(Icons.map_outlined),
              label: Text(l.pairMaplet),
            ),
          if (_sendNote != null) ...<Widget>[
            const SizedBox(height: 8),
            Text(_sendNote!, style: t.textTheme.bodySmall),
          ],
          if (s.track.isEmpty) ...<Widget>[
            const SizedBox(height: 16),
            Text(l.mapletNoTrack, style: t.textTheme.bodySmall),
          ],
          const SizedBox(height: 16),
          Text(l.mapletAttribution, style: t.textTheme.labelSmall?.copyWith(color: cs.onSurfaceVariant)),
        ],
      ),
    );
  }
}

/// The banner and stats line for a session, in the device's ASCII.
PostcardSpec postcardFor(SessionSummary s, AppLocalizations l) {
  final String unit = s.imperial ? 'mi' : 'km';
  final String dist = s.distanceUnits.toStringAsFixed(1);
  final String time = fmtClock(s.seconds);
  final String stats = '$time $dist$unit +${s.climbMeters.round()}m';
  return PostcardSpec(title: sessionTitle(s, l).toUpperCase(), stats: stats, track: s.track);
}

String sessionTitle(SessionSummary s, AppLocalizations l) {
  const List<String> months = <String>['JAN', 'FEB', 'MAR', 'APR', 'MAY', 'JUN', 'JUL', 'AUG', 'SEP', 'OCT', 'NOV', 'DEC'];
  final DateTime d = s.startedAt.toLocal();
  final String sport = switch (s.sport) {
    Sport.run => l.sportRun,
    Sport.ride => l.sportRide,
    Sport.ski => l.sportSki,
    Sport.snowboard => l.sportSnowboard,
    Sport.hike => l.sportHike,
    Sport.paddle => l.sportPaddle,
    Sport.climb => l.sportClimb,
  };
  return '$sport ${months[d.month - 1]} ${d.day}';
}

/// The e-paper preview: paper, ink, pixel-exact at 3x.
class _Glass extends StatelessWidget {
  const _Glass({required this.preview, required this.rendering, required this.error});
  final ui.Image? preview;
  final bool rendering;
  final String? error;

  @override
  Widget build(BuildContext context) {
    final ThemeData t = Theme.of(context);
    return AspectRatio(
      aspectRatio: preview != null ? preview!.width / preview!.height : 212 / 104,
      child: Container(
        decoration: BoxDecoration(
          color: const Color(0xFFECEBE4),
          borderRadius: BorderRadius.circular(4),
          border: Border.all(color: kBasalt.withValues(alpha: 0.25)),
        ),
        clipBehavior: Clip.antiAlias,
        child: preview != null
            ? FittedBox(fit: BoxFit.contain, child: RawImage(image: preview, filterQuality: FilterQuality.none, scale: 1 / 3))
            : Center(
                child: rendering
                    ? const CircularProgressIndicator()
                    : Text(error ?? '', style: t.textTheme.bodySmall?.copyWith(color: kBasalt)),
              ),
      ),
    );
  }
}
