// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart' show rootBundle;
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/mnpkg.dart';
import '../goggle/release_server.dart';
import '../goggle/status.dart';
import '../theme/tokens.dart';

/// Update: the releases this app carries, served from the phone to the
/// goggle as .mnpkg packages; the use-case bundle it carries, pushed over
/// the console; and a URL field for anything hosted elsewhere.
class UpdateScreen extends StatefulWidget {
  const UpdateScreen({super.key});

  @override
  State<UpdateScreen> createState() => _UpdateScreenState();
}

class _UpdateScreenState extends State<UpdateScreen> {
  final TextEditingController _url = TextEditingController(text: 'http://');
  List<EmbeddedRelease>? _releases;
  String? _busyFile;
  String? _log;
  bool _bundleBusy = false;

  @override
  void initState() {
    super.initState();
    unawaited(_load());
  }

  @override
  void dispose() {
    _url.dispose();
    super.dispose();
  }

  Future<void> _load() async {
    final List<EmbeddedRelease> r = await context.read<ReleaseServer>().listEmbedded();
    if (mounted) setState(() => _releases = r);
  }

  Future<void> _install(EmbeddedRelease r) async {
    final ReleaseServer srv = context.read<ReleaseServer>();
    final AppLocalizations l = AppLocalizations.of(context);
    final bool? ok = await showDialog<bool>(
      context: context,
      builder: (BuildContext c) => AlertDialog(
        title: Text(l.updateConfirmTitle),
        content: Text(l.updateInstallBody(r.header.fwVersion, r.header.version, r.header.variant)),
        actions: <Widget>[
          TextButton(onPressed: () => Navigator.pop(c, false), child: Text(l.commonCancel)),
          FilledButton(onPressed: () => Navigator.pop(c, true), child: Text(l.updateStart)),
        ],
      ),
    );
    if (ok != true || !mounted) return;
    setState(() {
      _busyFile = r.fileName;
      _log = l.updateServing;
    });
    try {
      final String url = await srv.serve(r);
      await _sendOta(url);
    } catch (e) {
      if (mounted) setState(() => _log = e.toString());
    } finally {
      if (mounted) setState(() => _busyFile = null);
    }
  }

  Future<void> _sendOta(String url) async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final ReleaseServer srv = context.read<ReleaseServer>();
    final AppLocalizations l = AppLocalizations.of(context);
    // ota! returns at once; the task reports through the console. Watch the
    // transcript for the verdict, then stop serving.
    final int mark = g.transcript.length;
    await g.send('s" $url" ota!', timeout: const Duration(seconds: 5)).catchError((_) => '');
    setState(() => _log = l.updateStarted);
    final DateTime end = DateTime.now().add(const Duration(seconds: 150));
    String verdict = l.updateNoVerdict;
    while (DateTime.now().isBefore(end)) {
      await Future<void>.delayed(const Duration(seconds: 1));
      if (!mounted) return;
      final Iterable<String> lines = g.transcript.skip(mark).map((ConsoleEntry e) => e.text);
      final String text = lines.join('\n');
      final RegExpMatch? pct = RegExp(r'mnpkg: (\d+)%').allMatches(text).lastOrNull;
      if (pct != null) setState(() => _log = l.updateProgress(int.parse(pct.group(1)!)));
      if (text.contains('refused') || text.contains('failed')) {
        verdict = RegExp(r'(mnpkg|ota): (refused[^\n]*|[^\n]*failed[^\n]*)').firstMatch(text)?.group(0) ?? verdict;
        break;
      }
      if (text.contains('rebooting') || !g.isConnected) {
        verdict = l.updateRebooting;
        break;
      }
    }
    await srv.stop();
    if (mounted) setState(() => _log = verdict);
  }

  Future<void> _otaFromUrl() async {
    final String url = _url.text.trim();
    if (!url.startsWith('http')) return;
    await _sendOta(url);
  }

  Future<void> _pushBundle() async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    setState(() {
      _bundleBusy = true;
      _log = l.updateBundleSending;
    });
    try {
      final String index = await rootBundle.loadString('assets/bundles/weather-hud/files.txt');
      final List<String> lines = <String>[];
      for (final String f in index.split('\n')) {
        if (f.trim().isEmpty) continue;
        final String src = await rootBundle.loadString('assets/bundles/weather-hud/${f.trim()}');
        for (final String raw in src.split('\n')) {
          final String st = raw.trim();
          if (st.isEmpty) continue;
          if (st.startsWith('\\')) {
            if (st.startsWith('\\ bundle:') || st.startsWith('\\ requires:')) lines.add(st);
            continue;
          }
          lines.add(raw.trimRight());
        }
      }
      final String verdict = await g.installBundle(lines);
      if (mounted) setState(() => _log = verdict.split('\n').where((String s) => s.startsWith('bundle:')).join('\n'));
    } catch (e) {
      if (mounted) setState(() => _log = e.toString());
    } finally {
      if (mounted) setState(() => _bundleBusy = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final ReleaseServer srv = context.watch<ReleaseServer>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final GoggleStatus? st = g.status;
    final TextStyle mono = TextStyle(fontFamily: kFontMono, fontSize: 12, color: cs.onSurfaceVariant);

    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
      children: <Widget>[
        Text(l.updateTitle, style: t.textTheme.headlineMedium),
        const SizedBox(height: 6),
        Text(st != null ? l.updateRunning(st.version, st.bundle) : l.updateNotConnected, style: t.textTheme.bodyLarge),
        if (_log != null) ...<Widget>[
          const SizedBox(height: 10),
          Container(
            padding: const EdgeInsets.all(10),
            decoration: BoxDecoration(color: cs.surfaceContainerHigh, borderRadius: BorderRadius.circular(8)),
            child: Text(_log!, style: TextStyle(fontFamily: kFontMono, fontSize: 12.5, color: cs.onSurface)),
          ),
        ],
        const SizedBox(height: 18),
        Text(l.updateEmbeddedTitle, style: t.textTheme.titleMedium?.copyWith(fontFamily: kFontDisplay, fontSize: 18)),
        const SizedBox(height: 4),
        Text(l.updateEmbeddedBody, style: t.textTheme.bodySmall),
        const SizedBox(height: 10),
        if (_releases == null)
          const Padding(padding: EdgeInsets.all(12), child: LinearProgressIndicator())
        else if (_releases!.isEmpty)
          Text(l.updateNoReleases, style: t.textTheme.bodySmall)
        else
          for (final EmbeddedRelease r in _releases!) ...<Widget>[
            _ReleaseCard(
              release: r,
              running: st?.version,
              busy: _busyFile == r.fileName,
              enabled: g.isConnected && _busyFile == null && (st?.netOk ?? false),
              onInstall: () => _install(r),
            ),
            const SizedBox(height: 10),
          ],
        if (srv.running)
          Text(l.updateServingAt(srv.ip ?? '?', ReleaseServer.port, srv.served), style: mono),
        const Divider(height: 32),
        Text(l.updateBundleTitle, style: t.textTheme.titleMedium?.copyWith(fontFamily: kFontDisplay, fontSize: 18)),
        const SizedBox(height: 4),
        Text(l.updateBundleBody, style: t.textTheme.bodySmall),
        const SizedBox(height: 10),
        FilledButton.tonal(
          onPressed: g.isConnected && !_bundleBusy ? _pushBundle : null,
          child: Text(_bundleBusy ? l.updateBundleSending : l.updateBundlePush),
        ),
        const Divider(height: 32),
        Text(l.updateOtaTitle, style: t.textTheme.titleMedium?.copyWith(fontFamily: kFontDisplay, fontSize: 18)),
        const SizedBox(height: 6),
        Text(l.updateOtaBody, style: t.textTheme.bodySmall),
        const SizedBox(height: 10),
        TextField(
          controller: _url,
          style: TextStyle(fontFamily: kFontMono, fontSize: 13),
          autocorrect: false,
          keyboardType: TextInputType.url,
          decoration: InputDecoration(labelText: l.updateUrlLabel),
        ),
        const SizedBox(height: 10),
        OutlinedButton(onPressed: g.isConnected && _busyFile == null ? _otaFromUrl : null, child: Text(l.updateStart)),
      ],
    );
  }
}

class _ReleaseCard extends StatelessWidget {
  const _ReleaseCard({required this.release, required this.running, required this.busy, required this.enabled, required this.onInstall});
  final EmbeddedRelease release;
  final String? running;
  final bool busy;
  final bool enabled;
  final VoidCallback onInstall;

  @override
  Widget build(BuildContext context) {
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final AppLocalizations l = AppLocalizations.of(context);
    final MnpkgHeader h = release.header;
    final bool same = running != null && h.fwVersion == running;
    final TextStyle mono = TextStyle(fontFamily: kFontMono, fontSize: 12, color: cs.onSurfaceVariant);
    return Card(
      child: Padding(
        padding: const EdgeInsets.fromLTRB(14, 12, 14, 12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            Row(
              children: <Widget>[
                Expanded(child: Text('goggl1 ${h.fwVersion}', style: t.textTheme.titleMedium?.copyWith(fontFamily: kFontDisplay, fontSize: 18))),
                if (same) Chip(label: Text(l.updateInstalled), visualDensity: VisualDensity.compact),
              ],
            ),
            const SizedBox(height: 4),
            Text('${h.version} · ${h.variant} · ${(release.sizeBytes / 1024).round()} KB · ${h.buildTime.toLocal().toString().substring(0, 16)}', style: mono),
            Text(
              '${h.targetsGoggle ? 'esp32c3 / beetle' : 'other target'} · ${h.sigAlg == 2 ? 'ed25519' : 'p256'} ${h.keyId.substring(0, 8)}${h.isDevSigned ? ' (dev)' : ''}${h.clearsBundles ? ' · clears bundles' : ''}',
              style: mono,
            ),
            const SizedBox(height: 10),
            FilledButton(
              onPressed: enabled && h.targetsGoggle ? onInstall : null,
              child: Text(busy ? l.updateServing : l.updateInstall),
            ),
          ],
        ),
      ),
    );
  }
}
