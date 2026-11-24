// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// One zine: download it, render it for the pass's panel, preview the exact
// pages the pass will show, and send it.
import 'dart:typed_data';

import 'package:flutter/material.dart';
import 'package:image/image.dart' as img;

import '../catalog/opds.dart';
import '../main.dart';
import '../pack/render.dart';
import '../pack/svpk.dart';

class PublicationScreen extends StatefulWidget {
  const PublicationScreen({super.key, required this.pub});
  final Publication pub;
  @override
  State<PublicationScreen> createState() => _PublicationScreenState();
}

class _PublicationScreenState extends State<PublicationScreen> {
  String _stage = 'Downloading…';
  double? _progress;
  String? _error;
  List<Uint8List> _previews = <Uint8List>[];
  Uint8List? _pack;
  bool _sending = false;
  double _sent = 0;
  String? _sentMsg;

  @override
  void initState() {
    super.initState();
    _build();
  }

  Future<void> _build() async {
    final PanelProfile p = app.profile;
    try {
      setState(() {
        _stage = 'Downloading…';
        _error = null;
        _pack = null;
        _previews = <Uint8List>[];
      });
      final Acquisition a = widget.pub.acquisition!;
      final Uint8List src = await download(a.href);
      setState(() => _stage = 'Rendering for ${p.label}…');
      final List<img.Image> grey = await renderSource(src, a.kind, p,
          onPage: (int d, int t) => setState(() => _progress = d / t));
      final List<PackPage> pages = <PackPage>[for (final img.Image g in grey) toOneBit(g, p)];
      final Uint8List pack = buildPack(
          profile: p,
          pages: pages,
          title: widget.pub.title,
          author: widget.pub.author,
          source: widget.pub.id,
          sourceBytes: src);
      setState(() {
        _previews = <Uint8List>[for (final PackPage pg in pages) img.encodePng(previewOf(pg, p))];
        _pack = pack;
        _stage = '${pages.length} pages, ${(pack.length / 1024).toStringAsFixed(1)} KB';
        _progress = null;
      });
    } catch (e) {
      setState(() => _error = '$e');
    }
  }

  Future<void> _send() async {
    final Uint8List? pack = _pack;
    if (pack == null) return;
    setState(() {
      _sending = true;
      _sent = 0;
      _sentMsg = null;
    });
    final Stopwatch sw = Stopwatch()..start();
    try {
      await app.link.sendPack(packFileName(widget.pub.title), pack,
          onProgress: (double f) => setState(() => _sent = f));
      setState(() => _sentMsg = app.brand.sentLabel(sw.elapsedMilliseconds / 1000, pack.length / 1024));
    } catch (e) {
      setState(() => _sentMsg = 'Not sent: $e');
    } finally {
      setState(() => _sending = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final Publication p = widget.pub;
    final PanelProfile prof = app.profile;
    return Scaffold(
      appBar: AppBar(title: Text(p.title)),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: <Widget>[
          if (p.author.isNotEmpty) Text(p.author, style: Theme.of(context).textTheme.titleMedium),
          if (p.description.isNotEmpty) Padding(padding: const EdgeInsets.only(top: 4), child: Text(p.description)),
          const SizedBox(height: 12),
          if (_error != null)
            Column(children: <Widget>[
              Text("Couldn't render it: $_error"),
              TextButton(onPressed: _build, child: const Text('Try again')),
            ])
          else ...<Widget>[
            Text(_stage, style: Theme.of(context).textTheme.labelLarge),
            if (_pack == null) LinearProgressIndicator(value: _progress),
          ],
          const SizedBox(height: 12),
          if (_previews.isNotEmpty)
            SizedBox(
              // pages scale to fit: never taller than the row, whatever the phone or its rotation
              height: (MediaQuery.sizeOf(context).height * 0.6).clamp(200.0, 440.0),
              child: ListView.separated(
                scrollDirection: Axis.horizontal,
                itemCount: _previews.length,
                separatorBuilder: (_, __) => const SizedBox(width: 12),
                itemBuilder: (BuildContext context, int i) => Column(children: <Widget>[
                  Expanded(
                    child: AspectRatio(
                      aspectRatio: prof.width / prof.height,
                      child: DecoratedBox(
                        decoration: BoxDecoration(border: Border.all(color: Colors.black26)),
                        child: Image.memory(_previews[i], fit: BoxFit.contain, filterQuality: FilterQuality.none, gaplessPlayback: true),
                      ),
                    ),
                  ),
                  const SizedBox(height: 4),
                  Text('${i + 1}'),
                ]),
              ),
            ),
          const SizedBox(height: 16),
          ListenableBuilder(
            listenable: app.link,
            builder: (BuildContext context, _) {
              final bool can = _pack != null && app.link.connected && !_sending;
              return Column(crossAxisAlignment: CrossAxisAlignment.stretch, children: <Widget>[
                FilledButton.icon(
                  onPressed: can ? _send : null,
                  icon: const Icon(Icons.push_pin_outlined),
                  label: Text(app.link.connected ? app.brand.sendLabel(app.link.name ?? 'MY PASS') : 'CONNECT A PASS TO SEND'),
                ),
                if (_sending) Padding(padding: const EdgeInsets.only(top: 8), child: LinearProgressIndicator(value: _sent)),
                if (_sentMsg != null) Padding(padding: const EdgeInsets.only(top: 8), child: Text(_sentMsg!)),
              ]);
            },
          ),
        ],
      ),
    );
  }
}
