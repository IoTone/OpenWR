// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../screens/console_screen.dart';
import '../screens/looks_screen.dart';
import '../screens/session_screen.dart';
import '../screens/trailhead_screen.dart';
import '../screens/update_screen.dart';
import '../theme/theme_controller.dart';
import '../theme/tokens.dart';

/// The five tabs from the design. Settings is a gear in the app bar.
enum AppView { trailhead, session, looks, console, update }

extension on AppView {
  String title(AppLocalizations l) => switch (this) {
        AppView.trailhead => l.tabTrailhead,
        AppView.session => l.tabSession,
        AppView.looks => l.tabLooks,
        AppView.console => l.tabConsole,
        AppView.update => l.tabUpdate,
      };
  IconData get icon => switch (this) {
        AppView.trailhead => Icons.landscape_outlined,
        AppView.session => Icons.timer_outlined,
        AppView.looks => Icons.visibility_outlined,
        AppView.console => Icons.terminal,
        AppView.update => Icons.system_update_alt,
      };
}

class HomeShell extends StatefulWidget {
  const HomeShell({super.key, this.initial = 0});
  final int initial;

  @override
  State<HomeShell> createState() => _HomeShellState();
}

class _HomeShellState extends State<HomeShell> {
  late final PageController _pc = PageController(initialPage: widget.initial);
  late int _index = widget.initial;

  @override
  void dispose() {
    _pc.dispose();
    super.dispose();
  }

  void _goTo(int i) {
    final bool reduceMotion = context.read<ThemeController>().reduceMotion;
    if (reduceMotion) {
      _pc.jumpToPage(i);
    } else {
      _pc.animateToPage(i, duration: const Duration(milliseconds: 250), curve: Curves.easeOut);
    }
  }

  @override
  Widget build(BuildContext context) {
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    return PopScope(
      canPop: _index == 0,
      onPopInvokedWithResult: (bool didPop, Object? _) {
        if (!didPop) _goTo(0);
      },
      child: Scaffold(
        appBar: AppBar(
          title: Row(
            children: <Widget>[
              const _Mark(size: 22),
              const SizedBox(width: 10),
              Text(l.appTitle, style: t.textTheme.titleLarge?.copyWith(fontWeight: FontWeight.w800)),
            ],
          ),
          actions: <Widget>[
            const _LinkIndicator(),
            IconButton(
              icon: const Icon(Icons.settings_outlined),
              tooltip: l.settingsTitle,
              onPressed: () => context.push('/settings'),
            ),
          ],
        ),
        body: PageView(
          controller: _pc,
          onPageChanged: (int i) => setState(() => _index = i),
          children: const <Widget>[
            TrailheadScreen(),
            SessionScreen(),
            LooksScreen(),
            ConsoleScreen(),
            UpdateScreen(),
          ],
        ),
        bottomNavigationBar: NavigationBar(
          selectedIndex: _index,
          onDestinationSelected: _goTo,
          backgroundColor: cs.surface,
          indicatorColor: cs.primary.withValues(alpha: .18),
          labelTextStyle: WidgetStatePropertyAll<TextStyle>(
            TextStyle(fontFamily: kFontMono, fontSize: 11, letterSpacing: .5, color: cs.onSurface),
          ),
          destinations: <Widget>[
            for (final AppView v in AppView.values)
              NavigationDestination(icon: Icon(v.icon), label: v.title(l)),
          ],
        ),
      ),
    );
  }
}

/// The switchback mark, drawn so it matches the launcher icon at any size.
class _Mark extends StatelessWidget {
  const _Mark({required this.size});
  final double size;
  @override
  Widget build(BuildContext context) {
    final ColorScheme cs = Theme.of(context).colorScheme;
    return CustomPaint(size: Size.square(size), painter: SwitchbackMarkPainter(color: cs.primary, dot: cs.error));
  }
}

class SwitchbackMarkPainter extends CustomPainter {
  SwitchbackMarkPainter({required this.color, required this.dot});
  final Color color;
  final Color dot;
  @override
  void paint(Canvas canvas, Size s) {
    final Paint p = Paint()
      ..color = color
      ..style = PaintingStyle.stroke
      ..strokeWidth = s.width * .17
      ..strokeCap = StrokeCap.round
      ..strokeJoin = StrokeJoin.round;
    final Path path = Path()
      ..moveTo(s.width * .12, s.height * .86)
      ..lineTo(s.width * .80, s.height * .58)
      ..lineTo(s.width * .22, s.height * .36)
      ..lineTo(s.width * .70, s.height * .14);
    canvas.drawPath(path, p);
    canvas.drawCircle(Offset(s.width * .80, s.height * .58), s.width * .10, Paint()..color = dot);
  }

  @override
  bool shouldRepaint(SwitchbackMarkPainter old) => old.color != color || old.dot != dot;
}

/// Link state in the app bar; tap goes to Pair or Console.
class _LinkIndicator extends StatelessWidget {
  const _LinkIndicator();
  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final ColorScheme cs = Theme.of(context).colorScheme;
    final AppLocalizations l = AppLocalizations.of(context);
    final (Color, IconData, String) v = switch (g.link) {
      GoggleLink.connected => (cs.tertiary, Icons.bluetooth_connected, l.goggleConnected),
      GoggleLink.connecting => (cs.secondary, Icons.bluetooth_searching, l.goggleConnecting),
      GoggleLink.disconnected => (cs.onSurfaceVariant, Icons.bluetooth, l.goggleNotConnected),
    };
    return IconButton(
      tooltip: v.$3,
      icon: Icon(v.$2, color: v.$1),
      onPressed: () => context.push(g.isConnected ? '/console' : '/pair'),
    );
  }
}
