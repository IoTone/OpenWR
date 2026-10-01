// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../theme/tokens.dart';

/// Weird mode: the raw ok> prompt. This is the whole app with the forms
/// removed, and it is how the forms were designed.
class ConsoleScreen extends StatefulWidget {
  const ConsoleScreen({super.key});

  @override
  State<ConsoleScreen> createState() => _ConsoleScreenState();
}

class _ConsoleScreenState extends State<ConsoleScreen> {
  final TextEditingController _input = TextEditingController();
  final ScrollController _scroll = ScrollController();
  final List<String> _history = <String>[];
  int _histPos = -1;
  bool _busy = false;
  int _seenLen = 0;

  static const List<String> _quick = <String>['status', 'words', 'wx-fetch .', 'hud-now', 'mem', 'version'];

  @override
  void dispose() {
    _input.dispose();
    _scroll.dispose();
    super.dispose();
  }

  Future<void> _send([String? text]) async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final String line = (text ?? _input.text).trim();
    if (line.isEmpty || !g.isConnected || _busy) return;
    setState(() {
      _busy = true;
      _input.clear();
      _history.add(line);
      _histPos = -1;
    });
    try {
      await g.send(line);
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(e.toString())));
      }
    } finally {
      if (mounted) setState(() => _busy = false);
      WidgetsBinding.instance.addPostFrameCallback((_) {
        if (_scroll.hasClients) _scroll.jumpTo(_scroll.position.maxScrollExtent);
      });
    }
  }

  void _recall(int dir) {
    if (_history.isEmpty) return;
    int p = _histPos < 0 ? _history.length : _histPos;
    p = (p + dir).clamp(0, _history.length - 1);
    setState(() {
      _histPos = p;
      _input.text = _history[p];
      _input.selection = TextSelection.collapsed(offset: _input.text.length);
    });
  }

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final TextStyle mono = TextStyle(fontFamily: kFontMono, fontSize: 13, color: cs.onSurface, height: 1.35);

    // Follow the transcript as the goggle talks (status refreshes, OTA progress).
    if (g.transcript.length != _seenLen) {
      _seenLen = g.transcript.length;
      WidgetsBinding.instance.addPostFrameCallback((_) {
        if (_scroll.hasClients) _scroll.jumpTo(_scroll.position.maxScrollExtent);
      });
    }

    return Column(
      children: <Widget>[
        if (!g.isConnected)
          MaterialBanner(
            content: Text(l.consoleNotConnected),
            actions: <Widget>[
              TextButton(onPressed: () => context.push('/pair'), child: Text(l.pairGoggle)),
            ],
          ),
        Expanded(
          child: Container(
            color: cs.surfaceContainerLowest,
            child: ListView.builder(
              controller: _scroll,
              padding: const EdgeInsets.fromLTRB(14, 10, 14, 10),
              itemCount: g.transcript.length,
              itemBuilder: (BuildContext c, int i) {
                final ConsoleEntry e = g.transcript[i];
                return Text(
                  e.sent ? 'ok> ${e.text}' : e.text,
                  style: e.sent ? mono.copyWith(color: cs.primary, fontWeight: FontWeight.w500) : mono,
                );
              },
            ),
          ),
        ),
        SizedBox(
          height: 44,
          child: ListView(
            scrollDirection: Axis.horizontal,
            padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 6),
            children: <Widget>[
              for (final String q in _quick)
                Padding(
                  padding: const EdgeInsets.only(right: 6),
                  child: ActionChip(label: Text(q, style: TextStyle(fontFamily: kFontMono)), onPressed: g.isConnected ? () => _send(q) : null),
                ),
              ActionChip(
                label: Text(l.commonCopy),
                avatar: const Icon(Icons.copy, size: 14),
                onPressed: () {
                  Clipboard.setData(ClipboardData(
                      text: g.transcript.map((ConsoleEntry e) => e.sent ? 'ok> ${e.text}' : e.text).join('\n')));
                  ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(l.commonCopied)));
                },
              ),
              const SizedBox(width: 6),
              ActionChip(label: Text(l.diagClear), onPressed: g.clearTranscript),
            ],
          ),
        ),
        SafeArea(
          top: false,
          child: Padding(
            padding: const EdgeInsets.fromLTRB(12, 4, 12, 10),
            child: Row(
              children: <Widget>[
                IconButton(onPressed: () => _recall(-1), icon: const Icon(Icons.arrow_upward), tooltip: l.consoleHistory),
                Expanded(
                  child: TextField(
                    controller: _input,
                    enabled: g.isConnected && !_busy,
                    style: TextStyle(fontFamily: kFontMono),
                    autocorrect: false,
                    enableSuggestions: false,
                    textInputAction: TextInputAction.send,
                    onSubmitted: (_) => _send(),
                    decoration: InputDecoration(hintText: 'ok> ', isDense: true),
                  ),
                ),
                const SizedBox(width: 8),
                FilledButton(
                  onPressed: g.isConnected && !_busy ? _send : null,
                  child: Text(l.consoleSend),
                ),
              ],
            ),
          ),
        ),
      ],
    );
  }
}
