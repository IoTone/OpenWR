// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// SubverseReader: browse a Subverse (OPDS) catalog, render zines for your
// e-paper pass on the phone, and send them to it over Bluetooth.
import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'catalog/opds.dart';
import 'pack/svpk.dart';
import 'pass/pass_link.dart';
import 'screens/catalog_screen.dart';
import 'screens/pass_screen.dart';
import 'theme/brand.dart';

class AppState extends ChangeNotifier {
  AppState(this.prefs)
      : catalogUrl = prefs.getString('catalog') ?? kDefaultCatalog,
        profile = PanelProfile.all.firstWhere((PanelProfile p) => p.id == prefs.getString('profile'),
            orElse: () => PanelProfile.sr15),
        brand = brandById(prefs.getString('brand')) {
    link.lastId = prefs.getString('pass');
    link.onConnectedId = (String id) => prefs.setString('pass', id);
    // A pass says which panel it has; build for that one from now on.
    link.onProfile = (String id) {
      final PanelProfile? p = PanelProfile.byId(id);
      if (p != null && p.id != profile.id) setProfile(p);
    };
  }
  final SharedPreferences prefs;
  final PassLink link = PassLink();
  String catalogUrl;
  PanelProfile profile;
  Brand brand;

  /// Started at launch so the catalog loads behind the splash.
  late Future<List<Publication>> catalog = fetchCatalog(catalogUrl);

  void setBrand(Brand b) {
    brand = b;
    prefs.setString('brand', b.id);
    notifyListeners();
  }

  void setCatalog(String u) {
    catalogUrl = u.trim().isEmpty ? kDefaultCatalog : u.trim();
    prefs.setString('catalog', catalogUrl);
    notifyListeners();
  }

  void setProfile(PanelProfile p) {
    profile = p;
    prefs.setString('profile', p.id);
    notifyListeners();
  }
}

late AppState app;

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  app = AppState(await SharedPreferences.getInstance());
  app.catalog; // start loading now
  runApp(const SubverseReaderApp());
}

class SubverseReaderApp extends StatelessWidget {
  const SubverseReaderApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: app,
      builder: (BuildContext context, _) => MaterialApp(
        title: 'SubverseReader',
        debugShowCheckedModeBanner: false,
        theme: app.brand.theme,
        themeMode: ThemeMode.light,
        home: const HomeShell(),
      ),
    );
  }
}

class HomeShell extends StatefulWidget {
  const HomeShell({super.key});
  @override
  State<HomeShell> createState() => _HomeShellState();
}

class _HomeShellState extends State<HomeShell> {
  int _tab = 0;
  bool _splash = true;

  @override
  void initState() {
    super.initState();
    Future<void>.delayed(const Duration(milliseconds: 1400), () {
      if (mounted) setState(() => _splash = false);
    });
  }

  @override
  Widget build(BuildContext context) {
    return AnimatedSwitcher(
      duration: const Duration(milliseconds: 300),
      child: _splash
          ? KeyedSubtree(key: const ValueKey<String>('splash'), child: app.brand.splash())
          : Scaffold(
              key: const ValueKey<String>('shell'),
              body: IndexedStack(index: _tab, children: const <Widget>[CatalogScreen(), PassScreen()]),
              bottomNavigationBar: ListenableBuilder(
                listenable: app.link,
                builder: (BuildContext context, _) => NavigationBar(
                  selectedIndex: _tab,
                  onDestinationSelected: (int i) => setState(() => _tab = i),
                  destinations: <Widget>[
                    NavigationDestination(icon: const Icon(Icons.auto_stories_outlined), label: app.brand.navZines),
                    NavigationDestination(
                      icon: Icon(app.link.connected ? Icons.badge : Icons.badge_outlined),
                      label: app.link.connected ? '${app.brand.navPass} · ${app.link.name ?? ''}' : app.brand.navPass,
                    ),
                  ],
                ),
              ),
            ),
    );
  }
}
