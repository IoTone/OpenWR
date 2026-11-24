// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The pass: find it, connect, and see what is on its card. The pass is the
// source of truth for its shelf.
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
              const Text('The pass listens for 3 minutes after it powers up. Any other time, hold MENU and EXIT '
                  'together until the screen says SYNC.'),
              const SizedBox(height: 12),
              FilledButton.icon(
                onPressed: l.state == LinkState.scanning || l.state == LinkState.connecting ? null : l.scan,
                icon: const Icon(Icons.bluetooth_searching),
                label: Text(l.state == LinkState.scanning ? 'Scanning…' : 'Find my pass'),
              ),
              for (final ScanResult r in l.found)
                ListTile(
                  leading: const Icon(Icons.badge_outlined),
                  title: Text(r.advertisementData.advName.isNotEmpty ? r.advertisementData.advName : r.device.platformName),
                  subtitle: Text('${r.device.remoteId}  ·  ${r.rssi} dBm'),
                  trailing: const Icon(Icons.chevron_right),
                  onTap: l.state == LinkState.connecting ? null : () => l.connect(r),
                ),
            ] else ...<Widget>[
              if (l.panel.isNotEmpty) Text('Panel controller: ${l.panel}'),
              if (l.reading.isNotEmpty) Text('Reading: ${l.reading}'),
              const SizedBox(height: 12),
              Text('On the card (${l.shelf.length})', style: Theme.of(context).textTheme.titleSmall),
              if (l.shelf.isEmpty) const Text('Nothing yet. Send a zine from the Subverse tab.'),
              for (final ShelfItem it in l.shelf)
                ListTile(
                  leading: const Icon(Icons.menu_book_outlined),
                  title: Text(it.file),
                  onTap: () => l.open(it),
                  trailing: IconButton(
                    icon: const Icon(Icons.delete_outline),
                    tooltip: 'Remove from the pass',
                    onPressed: () async {
                      final bool? ok = await showDialog<bool>(
                        context: context,
                        builder: (BuildContext ctx) => AlertDialog(
                          title: Text('Remove ${it.file}?'),
                          content: const Text('It is deleted from the pass\'s card. You can send it again any time.'),
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
