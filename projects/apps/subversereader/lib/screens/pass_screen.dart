// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The pass: find it, connect, see what is on it, and drive it. For a pass
// with no keys this screen is its controls. The pass is the source of truth
// for its shelf.
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import '../main.dart';
import '../pack/svpk.dart';
import '../pass/pass_link.dart';

class PassScreen extends StatelessWidget {
  const PassScreen({super.key});

  @override
  Widget build(BuildContext context) {
    final PassLink l = app.link;
    return ListenableBuilder(
      listenable: Listenable.merge(<Listenable>[l, app]),
      builder: (BuildContext context, _) => Scaffold(
        appBar: AppBar(
          title: Text(l.connected ? (l.name ?? app.brand.navPass) : app.brand.navPass.toUpperCase()),
          actions: <Widget>[
            if (l.connected) IconButton(icon: const Icon(Icons.refresh), tooltip: 'Refresh', onPressed: l.refresh),
            if (l.connected)
              IconButton(icon: const Icon(Icons.link_off), tooltip: 'Disconnect', onPressed: l.disconnect),
          ],
        ),
        body: ListView(
          padding: const EdgeInsets.all(16),
          children: <Widget>[
            Text(l.status, style: Theme.of(context).textTheme.titleMedium),
            const SizedBox(height: 8),
            if (!l.connected) ...<Widget>[
              const Text('A pass listens for 3 minutes after it powers up. Any other time: on SR-15 hold MENU and '
                  'EXIT, on SR-16 hold PREV and NEXT, until the screen says SYNC.'),
              const SizedBox(height: 8),
              const Text('A pass with no keys checks in by itself every few minutes. Tap Wait for my pass, keep '
                  'this screen open, and it connects when the pass wakes. Its reset button wakes it at once.'),
              const SizedBox(height: 12),
              Wrap(spacing: 12, runSpacing: 8, children: <Widget>[
                FilledButton.icon(
                  onPressed: l.state == LinkState.scanning || l.state == LinkState.connecting ? null : l.scan,
                  icon: const Icon(Icons.bluetooth_searching),
                  label: Text(l.state == LinkState.scanning && !l.waiting ? 'Looking…' : 'Find my pass'),
                ),
                if (l.waiting)
                  OutlinedButton.icon(
                      onPressed: l.stopWaiting, icon: const Icon(Icons.close), label: const Text('Stop waiting'))
                else
                  OutlinedButton.icon(
                    onPressed: l.state == LinkState.idle ? l.waitForPass : null,
                    icon: const Icon(Icons.hourglass_bottom),
                    label: const Text('Wait for my pass'),
                  ),
              ]),
              if (l.waiting) const Padding(padding: EdgeInsets.only(top: 12), child: LinearProgressIndicator()),
              for (final ScanResult r in l.found)
                ListTile(
                  leading: const Icon(Icons.badge_outlined),
                  title: Text(r.advertisementData.advName.isNotEmpty ? r.advertisementData.advName : r.device.platformName),
                  subtitle: Text('${r.device.remoteId}  ·  ${r.rssi} dBm'),
                  trailing: const Icon(Icons.chevron_right),
                  onTap: l.state == LinkState.connecting ? null : () => l.connect(r),
                ),
            ] else ...<Widget>[
              _PassFacts(l),
              const SizedBox(height: 12),
              _Remote(l),
              const SizedBox(height: 16),
              Text('On the pass (${l.shelf.length})', style: Theme.of(context).textTheme.titleSmall),
              if (l.shelf.isEmpty) const Text('Nothing yet. Send a zine from the zines tab.'),
              for (final ShelfItem it in l.shelf)
                ListTile(
                  contentPadding: EdgeInsets.zero,
                  leading: const Icon(Icons.menu_book_outlined),
                  title: Text(it.file, maxLines: 1, overflow: TextOverflow.ellipsis),
                  subtitle: const Text('Tap to show it on the pass'),
                  onTap: () => l.open(it),
                  trailing: IconButton(
                    icon: const Icon(Icons.delete_outline),
                    tooltip: 'Remove from the pass',
                    onPressed: () async {
                      final bool? ok = await showDialog<bool>(
                        context: context,
                        builder: (BuildContext ctx) => AlertDialog(
                          title: Text('Remove ${it.file}?'),
                          content: const Text('It is deleted from the pass. You can send it again any time.'),
                          actions: <Widget>[
                            TextButton(onPressed: () => Navigator.pop(ctx, false), child: const Text('Keep')),
                            FilledButton(onPressed: () => Navigator.pop(ctx, true), child: const Text('Remove')),
                          ],
                        ),
                      );
                      if (ok == true) await l.remove(it);
                    },
                  ),
                ),
            ],
            const Divider(height: 32),
            Text('Render for', style: Theme.of(context).textTheme.titleSmall),
            const SizedBox(height: 8),
            SegmentedButton<PanelProfile>(
              segments: <ButtonSegment<PanelProfile>>[
                for (final PanelProfile p in PanelProfile.all)
                  ButtonSegment<PanelProfile>(value: p, label: Text(p.label.split(' (').first)),
              ],
              selected: <PanelProfile>{app.profile},
              onSelectionChanged: (Set<PanelProfile> v) => app.setProfile(v.first),
            ),
          ],
        ),
      ),
    );
  }
}

/// What the pass says about itself: panel, battery, room left.
class _PassFacts extends StatelessWidget {
  const _PassFacts(this.l);
  final PassLink l;

  @override
  Widget build(BuildContext context) {
    final PanelProfile? p = PanelProfile.byId(l.profileId);
    final List<String> facts = <String>[
      if (p != null) p.label else if (l.panel.isNotEmpty) 'Panel controller: ${l.panel}',
      if (l.keys == 0) 'No keys: this screen is its controls',
      if (l.battery.isNotEmpty) l.battery.startsWith('battery') ? 'Battery: not measured on this pass' : 'Battery ${l.battery}',
    ];
    return Column(crossAxisAlignment: CrossAxisAlignment.start, children: <Widget>[
      for (final String f in facts) Text(f),
      if (l.storeKb > 0) ...<Widget>[
        const SizedBox(height: 8),
        LinearProgressIndicator(value: ((l.storeKb - l.freeKb) / l.storeKb).clamp(0.0, 1.0)),
        const SizedBox(height: 4),
        Text('${_size(l.freeKb)} free of ${_size(l.storeKb)}', style: Theme.of(context).textTheme.bodySmall),
      ],
    ]);
  }

  static String _size(int kb) => kb >= 1024 * 1024
      ? '${(kb / 1024 / 1024).toStringAsFixed(1)} GB'
      : kb >= 1024
          ? '${(kb / 1024).toStringAsFixed(1)} MB'
          : '$kb KB';
}

/// Page turns, auto-turn and sleep, sent as console words.
class _Remote extends StatefulWidget {
  const _Remote(this.l);
  final PassLink l;
  @override
  State<_Remote> createState() => _RemoteState();
}

class _RemoteState extends State<_Remote> {
  bool _busy = false;
  String? _error;

  Future<void> _do(Future<void> Function() f) async {
    if (_busy) return;
    setState(() {
      _busy = true;
      _error = null;
    });
    try {
      await f();
    } catch (e) {
      _error = '$e';
    } finally {
      if (mounted) setState(() => _busy = false);
    }
  }

  /// "/fl/packs/x.svpk  'Title' by Someone, page 2 of 8" -> "Title · page 2 of 8"
  static String _reading(String s) {
    final RegExpMatch? m = RegExp(r"'(.*)' by .*, (page \d+ of \d+)").firstMatch(s);
    return m == null ? s : '${m.group(1)} · ${m.group(2)}';
  }

  static const List<(int, String)> _autos = <(int, String)>[
    (0, 'Off'), (10, '10 s'), (30, '30 s'), (60, '1 min'), (300, '5 min'), (900, '15 min')
  ];
  static const List<(int, String)> _syncs = <(int, String)>[
    (0, 'Never'), (60, '1 min'), (300, '5 min'), (900, '15 min'), (3600, '1 hour')
  ];

  @override
  Widget build(BuildContext context) {
    final PassLink l = widget.l;
    final TextTheme tt = Theme.of(context).textTheme;
    final bool onPage = l.reading.isNotEmpty;
    final bool modern = l.keys >= 0; // firmware 0.2 on
    return Column(crossAxisAlignment: CrossAxisAlignment.stretch, children: <Widget>[
      Text('Remote', style: tt.titleSmall),
      const SizedBox(height: 4),
      Text(onPage ? _reading(l.reading) : 'The pass is showing its shelf. Tap a zine below to open it.',
          maxLines: 2, overflow: TextOverflow.ellipsis, style: tt.bodySmall),
      const SizedBox(height: 8),
      Row(children: <Widget>[
        Expanded(
          child: OutlinedButton.icon(
            onPressed: onPage && !_busy ? () => _do(() => l.turn(-1)) : null,
            icon: const Icon(Icons.chevron_left),
            label: const Text('PREV'),
          ),
        ),
        const SizedBox(width: 12),
        Expanded(
          child: FilledButton.icon(
            onPressed: onPage && !_busy ? () => _do(() => l.turn(1)) : null,
            icon: const Icon(Icons.chevron_right),
            label: const Text('NEXT'),
          ),
        ),
      ]),
      const SizedBox(height: 4),
      Wrap(spacing: 8, children: <Widget>[
        TextButton.icon(
            onPressed: _busy ? null : () => _do(l.showShelf),
            icon: const Icon(Icons.view_list_outlined),
            label: const Text('Show shelf')),
        if (onPage)
          TextButton.icon(
              onPressed: _busy ? null : () => _do(l.showWhere),
              icon: const Icon(Icons.pin_outlined),
              label: const Text('Show page number')),
        TextButton.icon(
            onPressed: _busy ? null : () => _do(l.sleepNow),
            icon: const Icon(Icons.bedtime_outlined),
            label: const Text('Sleep now')),
      ]),
      if (modern) ...<Widget>[
        const SizedBox(height: 8),
        Text('Turn the page by itself', style: tt.labelLarge),
        Wrap(spacing: 8, children: <Widget>[
          for (final (int s, String label) in _autos)
            ChoiceChip(
                label: Text(label),
                selected: l.autoS == s,
                onSelected: _busy ? null : (_) => _do(() => l.setAuto(s))),
        ]),
        const SizedBox(height: 8),
        Text('Check in for this app while asleep', style: tt.labelLarge),
        Wrap(spacing: 8, children: <Widget>[
          for (final (int s, String label) in _syncs)
            ChoiceChip(
                label: Text(label),
                selected: l.syncS == s,
                onSelected: _busy ? null : (_) => _do(() => l.setSync(s))),
        ]),
        Text(
            l.syncS == 0
                ? (l.keys == 0
                    ? 'Off: only its reset button or power switch lets this app back in.'
                    : 'Off: open a sync window with its keys.')
                : 'More often finds the pass sooner and uses more battery.',
            style: tt.bodySmall),
      ],
      if (_error != null) Padding(padding: const EdgeInsets.only(top: 8), child: Text('Not done: $_error')),
    ]);
  }
}
