// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// OPDS 2.0 (JSON) catalog client: publications under a feed, following its
// subsections and next pages once each, and the best file to render for a
// pass: the reading copy (pages in order) over the comic copy over the print
// sheets.
import 'dart:convert';
import 'dart:typed_data';

import 'package:http/http.dart' as http;

const String kDefaultCatalog = 'https://spicemine.iotj.cc/opds';
const Map<String, String> _headers = <String, String>{
  'Accept': 'application/opds+json, application/json;q=0.9, */*;q=0.5',
  'User-Agent': 'SubverseReader/0.1 (Android)',
};

class Acquisition {
  Acquisition(this.href, this.type, this.title);
  final String href;
  final String type;
  final String title;
  String get kind => type.contains('comicbook') ? 'cbz' : 'pdf';
}

class Publication {
  Publication({
    required this.id,
    required this.title,
    required this.author,
    required this.pages,
    required this.cover,
    required this.description,
    required this.acquisition,
    required this.collection,
  });
  final String id;
  final String title;
  final String author;
  final int? pages;
  final String? cover;
  final String description;
  final Acquisition? acquisition;
  final String collection;
}

int _rank(Map<String, dynamic> l) {
  final String t = (l['type'] ?? '') as String;
  final String title = ((l['title'] ?? '') as String).toLowerCase();
  if (t == 'application/pdf' && title.contains('reading')) return 0;
  if (t.contains('comicbook')) return 1;
  if (t == 'application/pdf' && !title.contains('print')) return 2;
  if (t == 'application/pdf') return 3;
  return 9;
}

Publication _pub(Map<String, dynamic> p, Uri base) {
  final Map<String, dynamic> m = (p['metadata'] ?? <String, dynamic>{}) as Map<String, dynamic>;
  final List<Map<String, dynamic>> links =
      ((p['links'] ?? <dynamic>[]) as List<dynamic>).cast<Map<String, dynamic>>();
  final List<Map<String, dynamic>> acq =
      links.where((Map<String, dynamic> l) => ((l['rel'] ?? '') as String).contains('acquisition')).toList()
        ..sort((Map<String, dynamic> a, Map<String, dynamic> b) => _rank(a).compareTo(_rank(b)));
  final Map<String, dynamic>? best = acq.isNotEmpty && _rank(acq.first) < 9 ? acq.first : null;
  final List<dynamic> images = (p['images'] ?? <dynamic>[]) as List<dynamic>;
  final dynamic authors = m['author'];
  final String author = authors is List
      ? authors.map((dynamic a) => a is Map ? (a['name'] ?? '') : '$a').join(', ')
      : authors is Map
          ? '${authors['name'] ?? ''}'
          : '${authors ?? ''}';
  final dynamic belongs = (m['belongsTo'] as Map<String, dynamic>?)?['collection'];
  final String coll = belongs is List && belongs.isNotEmpty && belongs.first is Map ? '${belongs.first['name'] ?? ''}' : '';
  return Publication(
    id: '${m['identifier'] ?? m['title'] ?? ''}',
    title: '${m['title'] ?? 'untitled'}',
    author: author,
    pages: m['numberOfPages'] is int ? m['numberOfPages'] as int : null,
    cover: images.isNotEmpty ? base.resolve('${(images.first as Map<String, dynamic>)['href']}').toString() : null,
    description: '${m['description'] ?? ''}',
    acquisition: best == null
        ? null
        : Acquisition(base.resolve('${best['href']}').toString(), '${best['type']}', '${best['title'] ?? ''}'),
    collection: coll,
  );
}

Future<List<Publication>> fetchCatalog(String url) async {
  final Set<String> seen = <String>{};
  final List<Publication> out = <Publication>[];
  final Set<String> ids = <String>{};
  Future<void> walk(String u, int depth) async {
    if (seen.contains(u) || depth > 3) return;
    seen.add(u);
    final http.Response r = await http.get(Uri.parse(u), headers: _headers).timeout(const Duration(seconds: 30));
    if (r.statusCode != 200) throw Exception('catalog $u: HTTP ${r.statusCode}');
    final Map<String, dynamic> feed = jsonDecode(utf8.decode(r.bodyBytes)) as Map<String, dynamic>;
    final Uri base = Uri.parse(u);
    final List<dynamic> pubs = <dynamic>[
      ...((feed['publications'] ?? <dynamic>[]) as List<dynamic>),
      for (final dynamic g in (feed['groups'] ?? <dynamic>[]) as List<dynamic>)
        ...(((g as Map<String, dynamic>)['publications'] ?? <dynamic>[]) as List<dynamic>),
    ];
    for (final dynamic p in pubs) {
      final Publication pub = _pub(p as Map<String, dynamic>, base);
      if (ids.add(pub.id)) out.add(pub);
    }
    final List<dynamic> nav = <dynamic>[
      ...((feed['navigation'] ?? <dynamic>[]) as List<dynamic>),
      ...((feed['links'] ?? <dynamic>[]) as List<dynamic>).where((dynamic l) => (l as Map<String, dynamic>)['rel'] == 'next'),
    ];
    for (final dynamic n in nav) {
      final Map<String, dynamic> l = n as Map<String, dynamic>;
      if (('${l['type'] ?? ''}').contains('opds+json')) await walk(base.resolve('${l['href']}').toString(), depth + 1);
    }
  }

  await walk(url, 0);
  return out;
}

Future<Uint8List> download(String url) async {
  final http.Response r = await http.get(Uri.parse(url), headers: _headers).timeout(const Duration(seconds: 60));
  if (r.statusCode != 200) throw Exception('download: HTTP ${r.statusCode}');
  return r.bodyBytes;
}
