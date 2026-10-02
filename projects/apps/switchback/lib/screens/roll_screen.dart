// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The roll: snapshots on the maplet's card. Listed from the device, pulled
// one at a time over BLE (about ten seconds each), shown as the glass
// shows them, and kept on the phone once pulled.
import 'dart:async';
import 'dart:typed_data';
import 'dart:ui' as ui;

import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/maplet.dart';
import '../map/eink_canvas.dart';

class RollScreen extends StatefulWidget {
  const RollScreen({super.key});

  @override
  State<RollScreen> createState() => _RollScreenState();
}

class _RollScreenState extends State<RollScreen> {
  List<int>? _frames;
  String? _error;
  final Map<int, ui.Image> _images = <int, ui.Image>{};
  final Map<int, Uint8List> _raw = <int, Uint8List>{};
  int? _pulling;

  @override
  void initState() {
    super.initState();
    unawaited(_list());
  }

  Future<void> _list() async {
    final MapletConsole m = context.read<MapletConsole>();
    try {
      final List<int> f = await m.rollList();
      if (mounted) setState(() => _frames = f.reversed.toList());
    } catch (e) {
      if (mounted) setState(() => _error = e.toString());
    }
  }

  Future<void> _pull(int n) async {
    final MapletConsole m = context.read<MapletConsole>();
    setState(() => _pulling = n);
    try {
      final Uint8List f = await m.rollGet(n);
      final ui.Image img = await EinkCanvas.unpack(f).toImage();
      if (!mounted) return;
      setState(() {
        _raw[n] = f;
        _images[n] = img;
      });
    } catch (e) {
      if (mounted) ScaffoldMessenger.of(context).showSnackBar(SnackBar(content: Text(e.toString())));
    } finally {
      if (mounted) setState(() => _pulling = null);
    }
  }

  Future<void> _show(int n) async {
    final MapletConsole m = context.read<MapletConsole>();
    final Uint8List? f = _raw[n];
    if (f == null) return;
    await m.pushFrame(f, slot: 2);
  }

  @override
  Widget build(BuildContext context) {
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final MapletConsole m = context.watch<MapletConsole>();
    return Scaffold(
      appBar: AppBar(title: Text(l.rollTitle)),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
        children: <Widget>[
          Text(l.rollBody, style: t.textTheme.bodyLarge),
          const SizedBox(height: 16),
          if (_frames == null && _error == null)
            const Padding(padding: EdgeInsets.all(12), child: LinearProgressIndicator())
          else if (_error != null)
            Text(l.rollNoCard, style: t.textTheme.bodyMedium)
          else if (_frames!.isEmpty)
            Text(l.rollEmpty, style: t.textTheme.bodyMedium)
          else
            for (final int n in _frames!) ...<Widget>[
              Text(l.rollFrame(n), style: t.textTheme.titleMedium),
              const SizedBox(height: 6),
              AspectRatio(
                aspectRatio: 212 / 104,
                child: Container(
                  decoration: BoxDecoration(color: const Color(0xFFECEBE4), borderRadius: BorderRadius.circular(4), border: Border.all(color: Colors.black26)),
                  clipBehavior: Clip.antiAlias,
                  child: _images[n] != null
                      ? FittedBox(fit: BoxFit.contain, child: RawImage(image: _images[n], filterQuality: FilterQuality.none, scale: 1 / 3))
                      : Center(
                          child: _pulling == n
                              ? const CircularProgressIndicator()
                              : FilledButton.tonal(onPressed: m.isConnected && _pulling == null ? () => _pull(n) : null, child: Text(l.rollPull)),
                        ),
                ),
              ),
              if (_images[n] != null) ...<Widget>[
                const SizedBox(height: 6),
                Align(
                  alignment: Alignment.centerRight,
                  child: TextButton.icon(
                    onPressed: m.isConnected && !m.busy ? () => _show(n) : null,
                    icon: const Icon(Icons.send_to_mobile, size: 18),
                    label: Text(l.rollShow),
                  ),
                ),
              ],
              const SizedBox(height: 18),
            ],
        ],
      ),
    );
  }
}
