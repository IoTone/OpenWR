// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';

import 'gen/app_localizations.dart';
import 'screens/about_screen.dart';
import 'screens/console_screen.dart';
import 'screens/diagnostics_screen.dart';
import 'screens/looks_screen.dart';
import 'screens/pair_screen.dart';
import 'screens/radar_screen.dart';
import 'goggle/console.dart';
import 'screens/plan_screen.dart';
import 'screens/session_detail_screen.dart';
import 'screens/session_screen.dart';
import 'screens/setup_screen.dart';
import 'screens/settings_screen.dart';
import 'shell/home_shell.dart';

/// `/` hosts the tab shell. Pushed pages wrap a tab's widget in a
/// Scaffold so the same screens work from a button and from the bar.
final GoRouter appRouter = GoRouter(
  initialLocation: '/',
  routes: <RouteBase>[
    GoRoute(path: '/', builder: (_, __) => const HomeShell()),
    GoRoute(path: '/pair', builder: (_, __) => const PairScreen()),
    GoRoute(path: '/pair-maplet', builder: (_, __) => const PairScreen(kind: DeviceKind.maplet)),
    GoRoute(path: '/plan', builder: (_, __) => const PlanScreen()),
    GoRoute(path: '/session/:i', builder: (_, GoRouterState st) => SessionDetailScreen(index: int.tryParse(st.pathParameters['i'] ?? '') ?? 0)),
    GoRoute(path: '/radar', builder: (_, __) => const RadarScreen()),
    GoRoute(path: '/setup', builder: (_, __) => const SetupScreen()),
    GoRoute(path: '/looks', builder: (BuildContext c, __) => _Page(title: AppLocalizations.of(c).tabLooks, child: const LooksScreen())),
    GoRoute(path: '/console', builder: (BuildContext c, __) => _Page(title: AppLocalizations.of(c).tabConsole, child: const ConsoleScreen())),
    GoRoute(path: '/session', builder: (BuildContext c, __) => _Page(title: AppLocalizations.of(c).tabSession, child: const SessionScreen())),
    GoRoute(path: '/settings', builder: (BuildContext c, __) => _Page(title: AppLocalizations.of(c).settingsTitle, child: const SettingsScreen())),
    GoRoute(path: '/diagnostics', builder: (_, __) => const DiagnosticsScreen()),
    GoRoute(path: '/about', builder: (_, __) => const AboutScreen()),
  ],
);

class _Page extends StatelessWidget {
  const _Page({required this.title, required this.child});
  final String title;
  final Widget child;
  @override
  Widget build(BuildContext context) => Scaffold(appBar: AppBar(title: Text(title)), body: child);
}
