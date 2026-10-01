// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'dart:ui' show PathMetric;

import 'package:flutter/material.dart';
import 'package:package_info_plus/package_info_plus.dart';

import '../shell/home_shell.dart';
import '../theme/tokens.dart';

/// Flutter-side splash after the native one: the switchback mark drawing
/// itself in, the name, the motto. Always on Fog with Basalt, the one
/// place the app looks the same in every experience.
class BrandedSplashScreen extends StatefulWidget {
  const BrandedSplashScreen({super.key});

  @override
  State<BrandedSplashScreen> createState() => _BrandedSplashScreenState();
}

class _BrandedSplashScreenState extends State<BrandedSplashScreen> with SingleTickerProviderStateMixin {
  late final AnimationController _draw = AnimationController(vsync: this, duration: const Duration(milliseconds: 1100))
    ..forward();
  final Future<PackageInfo> _info = PackageInfo.fromPlatform();

  @override
  void dispose() {
    _draw.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: kFog,
      body: SafeArea(
        child: Center(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: <Widget>[
              AnimatedBuilder(
                animation: _draw,
                builder: (BuildContext _, Widget? __) => CustomPaint(
                  size: const Size(120, 120),
                  painter: _DrawingMark(progress: Curves.easeOut.transform(_draw.value)),
                ),
              ),
              const SizedBox(height: 26),
              const Text(
                'Switchback',
                style: TextStyle(fontFamily: kFontDisplay, fontSize: 34, fontWeight: FontWeight.w800, color: kBasalt, letterSpacing: -1),
              ),
              const SizedBox(height: 8),
              const Text(
                'OPEN · WEIRD · WILD · STRONG',
                style: TextStyle(fontFamily: kFontMono, fontSize: 12, letterSpacing: 2, color: kEvergreen),
              ),
              const SizedBox(height: 18),
              FutureBuilder<PackageInfo>(
                future: _info,
                builder: (BuildContext _, AsyncSnapshot<PackageInfo> s) => Text(
                  s.hasData ? 'v${s.data!.version} · #PNWbuilt' : '#PNWbuilt',
                  style: const TextStyle(fontFamily: kFontMono, fontSize: 11, color: Color(0xFF5F6A64)),
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}

/// The mark, revealed along its path as the app starts.
class _DrawingMark extends CustomPainter {
  _DrawingMark({required this.progress});
  final double progress;

  @override
  void paint(Canvas canvas, Size s) {
    final Paint p = Paint()
      ..color = kEvergreen
      ..style = PaintingStyle.stroke
      ..strokeWidth = s.width * .17
      ..strokeCap = StrokeCap.round
      ..strokeJoin = StrokeJoin.round;
    final Path full = Path()
      ..moveTo(s.width * .12, s.height * .86)
      ..lineTo(s.width * .80, s.height * .58)
      ..lineTo(s.width * .22, s.height * .36)
      ..lineTo(s.width * .70, s.height * .14);
    for (final PathMetric m in full.computeMetrics()) {
      canvas.drawPath(m.extractPath(0, m.length * progress), p);
    }
    if (progress > .45) {
      canvas.drawCircle(Offset(s.width * .80, s.height * .58), s.width * .10, Paint()..color = kAlpenglow);
    }
  }

  @override
  bool shouldRepaint(_DrawingMark old) => old.progress != progress;
}

// Keep the shared painter referenced so the shell and splash stay in step.
// ignore: unused_element
final SwitchbackMarkPainter _sameMark = SwitchbackMarkPainter(color: kEvergreen, dot: kAlpenglow);
