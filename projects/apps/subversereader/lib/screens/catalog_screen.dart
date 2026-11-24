// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';

import '../catalog/opds.dart';
import '../main.dart';
import 'publication_screen.dart';

class CatalogScreen extends StatefulWidget {
  const CatalogScreen({super.key});
  @override
  State<CatalogScreen> createState() => _CatalogScreenState();
}

class _CatalogScreenState extends State<CatalogScreen> {
  Future<List<Publication>>? _pubs;

  @override
  void initState() {
    super.initState();
    _pubs = app.catalog;            // already loading since the app started
  }

  void _load() {
    final Future<List<Publication>> f = app.catalog = fetchCatalog(app.catalogUrl);
    setState(() {
      _pubs = f;
    });
  }

  Future<void> _settings() async {
    final TextEditingController c = TextEditingController(text: app.catalogUrl);
    final String? u = await showDialog<String>(
      context: context,
      builder: (BuildContext ctx) => AlertDialog(
        title: const Text('Catalog'),
        content: TextField(
          controller: c,
          keyboardType: TextInputType.url,
          decoration: const InputDecoration(labelText: 'OPDS 2.0 feed URL', helperText: 'Default: $kDefaultCatalog'),
        ),
        actions: <Widget>[
          TextButton(onPressed: () => Navigator.pop(ctx), child: const Text('Cancel')),
          FilledButton(onPressed: () => Navigator.pop(ctx, c.text), child: const Text('Use')),
        ],
      ),
    );
    if (u != null) {
      app.setCatalog(u);
      _load();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        toolbarHeight: 72,
        titleSpacing: 12,
        title: Row(children: <Widget>[
          app.brand.logo(size: 44),
          const SizedBox(width: 10),
          Flexible(child: Text(app.brand.catalogTitle, overflow: TextOverflow.ellipsis)),
          const SizedBox(width: 8),
          Container(
            color: Theme.of(context).colorScheme.secondary,
            padding: const EdgeInsets.symmetric(horizontal: 6),
            child: Text(app.brand.catalogTag,
                style: Theme.of(context).textTheme.titleLarge!.copyWith(fontSize: 18, color: Theme.of(context).colorScheme.onSecondary)),
          ),
        ]),
        actions: <Widget>[
          IconButton(icon: const Icon(Icons.refresh), tooltip: 'Reload', onPressed: _load),
          IconButton(icon: const Icon(Icons.settings_outlined), tooltip: 'Catalog', onPressed: _settings),
        ],
      ),
      body: FutureBuilder<List<Publication>>(
        future: _pubs,
        builder: (BuildContext context, AsyncSnapshot<List<Publication>> s) {
          if (s.connectionState != ConnectionState.done) return const Center(child: CircularProgressIndicator());
          if (s.hasError) {
            return Center(
              child: Padding(
                padding: const EdgeInsets.all(24),
                child: Column(mainAxisSize: MainAxisSize.min, children: <Widget>[
                  Text("Couldn't load the catalog.\n${s.error}", textAlign: TextAlign.center),
                  const SizedBox(height: 12),
                  FilledButton(onPressed: _load, child: const Text('Try again')),
                ]),
              ),
            );
          }
          final List<Publication> pubs = s.data!;
          if (pubs.isEmpty) return const Center(child: Text('Nothing in this catalog yet.'));
          return RefreshIndicator(
            onRefresh: () async => _load(),
            child: ListView.separated(
              padding: const EdgeInsets.all(16),
              itemCount: pubs.length,
              separatorBuilder: (_, __) => const SizedBox(height: 14),
              itemBuilder: (BuildContext context, int i) {
                final Publication p = pubs[i];
                final TextTheme t = Theme.of(context).textTheme;
                final List<Color> blocks = <Color>[
                  Theme.of(context).colorScheme.primary,
                  Theme.of(context).colorScheme.secondary,
                  Theme.of(context).colorScheme.tertiary,
                ];
                return app.brand.card(
                  index: i,
                  onTap: p.acquisition == null
                      ? null
                      : () => Navigator.of(context)
                          .push(MaterialPageRoute<void>(builder: (_) => PublicationScreen(pub: p))),
                  child: Row(children: <Widget>[
                    SizedBox(
                      width: 46,
                      height: 62,
                      child: p.cover != null
                          ? Image.network(p.cover!, fit: BoxFit.cover,
                              errorBuilder: (_, __, ___) => ColoredBox(color: blocks[i % blocks.length]))
                          : ColoredBox(color: blocks[i % blocks.length]),
                    ),
                    const SizedBox(width: 12),
                    Expanded(
                      child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: <Widget>[
                        Text(p.title, style: t.titleMedium, maxLines: 2, overflow: TextOverflow.ellipsis),
                        const SizedBox(height: 3),
                        Text(
                            <String>[
                              if (p.author.isNotEmpty) p.author,
                              if (p.pages != null) '${p.pages} pp',
                              if (p.acquisition == null) "can't render",
                            ].join(' / '),
                            style: t.bodySmall),
                      ]),
                    ),
                  ]),
                );
              },
            ),
          );
        },
      ),
    );
  }
}
