// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/status.dart';
import '../theme/tokens.dart';

/// Update: what is on the goggle, and an OTA from any URL on the local
/// network. Phase D adds the on-phone HTTP server and bundle push.
class UpdateScreen extends StatefulWidget {
  const UpdateScreen({super.key});

  @override
  State<UpdateScreen> createState() => _UpdateScreenState();
}

class _UpdateScreenState extends State<UpdateScreen> {
  final TextEditingController _url = TextEditingController(text: 'http://');
  String? _version;
  String? _bundle;
  bool _busy = false;

  @override
  void dispose() {
    _url.dispose();
    super.dispose();
  }

  Future<void> _read() async {
    final GoggleConsole g = context.read<GoggleConsole>();
    if (!g.isConnected) return;
    setState(() => _busy = true);
    try {
      final String v = await g.send('version');
      final String b = await g.send('bundle-info');
      if (mounted) {
        setState(() {
          _version = v;
          _bundle = b;
        });
      }
    } finally {
      if (mounted) setState(() => _busy = false);
    }
  }

  Future<void> _ota() async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final String url = _url.text.trim();
    if (!url.startsWith('http')) return;
    final bool? ok = await showDialog<bool>(
      context: context,
      builder: (BuildContext c) => AlertDialog(
        title: Text(l.updateConfirmTitle),
        content: Text(l.updateConfirmBody(url)),
        actions: <Widget>[
          TextButton(onPressed: () => Navigator.pop(c, false), child: Text(l.commonCancel)),
          FilledButton(onPressed: () => Navigator.pop(c, true), child: Text(l.updateStart)),
        ],
      ),
    );
    if (ok != true || !mounted) return;
    await g.send('s" $url" ota!', timeout: const Duration(seconds: 5)).catchError((_) => '');
    if (mounted) {
      ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(l.updateStarted)));
    }
  }

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final GoggleStatus? st = g.status;
    final TextStyle mono = TextStyle(fontFamily: kFontMono, fontSize: 13);

    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
      children: <Widget>[
        Text(l.updateTitle, style: t.textTheme.headlineMedium),
        const SizedBox(height: 8),
        if (st != null) ...<Widget>[
          Text(l.updateRunning(st.version, st.bundle), style: t.textTheme.bodyLarge),
          const SizedBox(height: 8),
        ],
        OutlinedButton(onPressed: g.isConnected && !_busy ? _read : null, child: Text(l.updateReadDetails)),
        if (_version != null) ...<Widget>[
          const SizedBox(height: 10),
          Card(
            child: Padding(
              padding: const EdgeInsets.all(12),
              child: SelectableText('$_version\n$_bundle', style: mono),
            ),
          ),
        ],
        const Divider(height: 32),
        Text(l.updateOtaTitle, style: t.textTheme.titleMedium),
        const SizedBox(height: 6),
        Text(l.updateOtaBody, style: t.textTheme.bodySmall),
        const SizedBox(height: 12),
        TextField(
          controller: _url,
          style: mono,
          autocorrect: false,
          keyboardType: TextInputType.url,
          decoration: InputDecoration(labelText: l.updateUrlLabel),
        ),
        const SizedBox(height: 12),
        FilledButton(onPressed: g.isConnected ? _ota : null, child: Text(l.updateStart)),
        const SizedBox(height: 10),
        Text(l.updatePhaseNote, style: t.textTheme.bodySmall),
      ],
    );
  }
}
