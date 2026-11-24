// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// Direction 5, Xerox Punk: cut, paste, photocopy, staple, hand it out at the
// show. Black and paper white, one hot-pink highlighter, one yellow scrap.
// Name: ransom-note letters cut from four fonts, "reader" in marker.
import 'dart:math' as math;

import 'package:flutter/material.dart';

import 'brand.dart';

class Punk {
  static const Color ink = Color(0xFF111111);
  static const Color paper = Color(0xFFF4F4F0);
  static const Color pink = Color(0xFFD4157F); // white text on it: 4.9:1
  static const Color yellow = Color(0xFFF2E600);
  static const Color grey = Color(0xFF5A5A55); // on paper: 6.6:1

  static const String anton = 'Anton';
  static const String typed = 'SpecialElite';
  static const String marker = 'PermanentMarker';
  static const String mono = 'RubikMonoOne';
}

class PunkBrand implements Brand {
  @override
  String get id => 'punk';
  @override
  String get label => 'Xerox Punk';

  @override
  ThemeData get theme {
    const ColorScheme cs = ColorScheme(
      brightness: Brightness.light,
      primary: Punk.ink,
      onPrimary: Punk.paper,
      secondary: Punk.pink,
      onSecondary: Colors.white,
      tertiary: Punk.yellow,
      onTertiary: Punk.ink,
      error: Color(0xFFB00020),
      onError: Colors.white,
      surface: Punk.paper,
      onSurface: Punk.ink,
      surfaceContainerHighest: Colors.white,
      outline: Punk.ink,
    );
    const BorderSide line = BorderSide(color: Punk.ink, width: 3);
    final TextTheme text = Typography.blackMountainView.apply(fontFamily: Punk.typed, bodyColor: Punk.ink, displayColor: Punk.ink).copyWith(
          headlineLarge: const TextStyle(fontFamily: Punk.anton, fontSize: 36, height: 1, color: Punk.ink),
          headlineMedium: const TextStyle(fontFamily: Punk.anton, fontSize: 30, height: 1, color: Punk.ink),
          titleLarge: const TextStyle(fontFamily: Punk.anton, fontSize: 26, height: 1.1, color: Punk.ink),
          titleMedium: const TextStyle(fontFamily: Punk.typed, fontSize: 17, color: Punk.ink),
          titleSmall: const TextStyle(fontFamily: Punk.marker, fontSize: 18, color: Punk.ink),
          labelLarge: const TextStyle(fontFamily: Punk.anton, fontSize: 18, letterSpacing: 1.2),
        );
    return ThemeData(
      useMaterial3: true,
      colorScheme: cs,
      scaffoldBackgroundColor: Punk.paper,
      textTheme: text,
      appBarTheme: AppBarTheme(
        backgroundColor: Punk.paper,
        foregroundColor: Punk.ink,
        elevation: 0,
        scrolledUnderElevation: 0,
        titleTextStyle: text.headlineMedium,
        shape: const Border(bottom: line),
      ),
      filledButtonTheme: FilledButtonThemeData(
        style: FilledButton.styleFrom(
          backgroundColor: Punk.pink,
          foregroundColor: Colors.white,
          disabledBackgroundColor: const Color(0xFFCFCFC8),
          disabledForegroundColor: Punk.grey,
          minimumSize: const Size(64, 56),
          shape: const RoundedRectangleBorder(side: line),
          textStyle: const TextStyle(fontFamily: Punk.anton, fontSize: 20, letterSpacing: 1.2),
        ),
      ),
      textButtonTheme: TextButtonThemeData(
        style: TextButton.styleFrom(
          foregroundColor: Punk.ink,
          textStyle: const TextStyle(fontFamily: Punk.marker, fontSize: 18),
        ),
      ),
      segmentedButtonTheme: SegmentedButtonThemeData(
        style: ButtonStyle(
          shape: const WidgetStatePropertyAll<OutlinedBorder>(RoundedRectangleBorder()),
          side: const WidgetStatePropertyAll<BorderSide>(BorderSide(color: Punk.ink, width: 2)),
          textStyle: const WidgetStatePropertyAll<TextStyle>(TextStyle(fontFamily: Punk.typed, fontSize: 14)),
          backgroundColor: WidgetStateProperty.resolveWith(
              (Set<WidgetState> s) => s.contains(WidgetState.selected) ? Punk.yellow : Colors.white),
          foregroundColor: const WidgetStatePropertyAll<Color>(Punk.ink),
        ),
      ),
      navigationBarTheme: NavigationBarThemeData(
        backgroundColor: Punk.ink,
        indicatorColor: Punk.pink,
        indicatorShape: const RoundedRectangleBorder(),
        height: 68,
        iconTheme: WidgetStateProperty.resolveWith(
            (Set<WidgetState> s) => IconThemeData(color: s.contains(WidgetState.selected) ? Colors.white : Punk.paper)),
        labelTextStyle: WidgetStateProperty.resolveWith((Set<WidgetState> s) => TextStyle(
            fontFamily: Punk.marker, fontSize: 15, color: s.contains(WidgetState.selected) ? Punk.yellow : Punk.paper)),
      ),
      progressIndicatorTheme: const ProgressIndicatorThemeData(
          color: Punk.pink, linearTrackColor: Color(0xFFDADAD3), linearMinHeight: 6),
      dividerTheme: const DividerThemeData(color: Punk.ink, thickness: 2),
      listTileTheme: const ListTileThemeData(iconColor: Punk.ink),
      dialogTheme: const DialogThemeData(
        backgroundColor: Punk.paper,
        shape: RoundedRectangleBorder(side: line),
        titleTextStyle: TextStyle(fontFamily: Punk.anton, fontSize: 24, color: Punk.ink),
      ),
      inputDecorationTheme: const InputDecorationTheme(
        border: OutlineInputBorder(borderRadius: BorderRadius.zero, borderSide: BorderSide(color: Punk.ink, width: 2)),
        focusedBorder: OutlineInputBorder(borderRadius: BorderRadius.zero, borderSide: BorderSide(color: Punk.pink, width: 3)),
      ),
    );
  }

  @override
  Widget wordmark({double size = 30, bool onDark = false}) => _Ransom(size: size, onDark: onDark);

  @override
  Widget logo({double size = 48}) => Semantics(
        label: 'SubverseReader',
        image: true,
        child: SizedBox(width: size, height: size, child: CustomPaint(painter: _PinLogo())),
      );

  @override
  Widget splash() => const _PunkSplash();

  @override
  Widget card({required int index, required Widget child, VoidCallback? onTap}) {
    const List<double> tilt = <double>[-1.0, 1.5, -0.5, 0.8, -1.4];
    return Transform.rotate(
      angle: tilt[index % tilt.length] * math.pi / 180,
      child: Material(
        color: Colors.white,
        child: InkWell(
          onTap: onTap,
          child: CustomPaint(
            foregroundPainter: _Dashed(),
            child: Padding(padding: const EdgeInsets.all(10), child: child),
          ),
        ),
      ),
    );
  }

  @override
  String get catalogTitle => 'FREE ZINES';
  @override
  String get catalogTag => 'TAKE ONE';
  @override
  String get navZines => 'zines';
  @override
  String get navPass => 'my pass';
  @override
  String sendLabel(String pass) => 'STAPLE IT TO $pass';
  @override
  String sentLabel(double seconds, double kb) =>
      'done. ${kb.toStringAsFixed(1)} kb in ${seconds.toStringAsFixed(1)} sec. pass it on.';
  @override
  String get splashLine => 'copying zines… do not lift the lid';
}

// ---------------------------------------------------------------- wordmark

class _Cut {
  const _Cut(this.t, this.font, this.bg, this.fg, {this.tilt = 0, this.box = false, this.scale = 1});
  final String t;
  final String font;
  final Color bg;
  final Color fg;
  final double tilt;
  final bool box;
  final double scale;
}

class _Ransom extends StatelessWidget {
  const _Ransom({required this.size, required this.onDark});
  final double size;
  final bool onDark;

  static const List<_Cut> _letters = <_Cut>[
    _Cut('S', Punk.anton, Punk.ink, Colors.white),
    _Cut('U', Punk.mono, Colors.white, Punk.ink, box: true, scale: .8),
    _Cut('B', Punk.anton, Punk.pink, Colors.white, tilt: -6),
    _Cut('v', Punk.typed, Colors.white, Punk.ink),
    _Cut('E', Punk.marker, Punk.ink, Colors.white),
    _Cut('R', Punk.anton, Punk.yellow, Punk.ink, tilt: 5),
    _Cut('S', Punk.mono, Colors.white, Punk.ink, box: true, scale: .75),
    _Cut('e', Punk.typed, Punk.ink, Colors.white),
  ];

  @override
  Widget build(BuildContext context) {
    return Semantics(
      label: 'SubverseReader',
      excludeSemantics: true,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.start,
        children: <Widget>[
          Wrap(
            crossAxisAlignment: WrapCrossAlignment.center,
            children: <Widget>[
              for (final _Cut c in _letters)
                Transform.rotate(
                  angle: c.tilt * math.pi / 180,
                  child: Container(
                    margin: EdgeInsets.symmetric(horizontal: size * .03),
                    padding: EdgeInsets.symmetric(horizontal: size * .12),
                    decoration: BoxDecoration(
                      color: c.bg == Punk.ink && onDark ? Colors.white : c.bg,
                      border: c.box ? Border.all(color: Punk.ink, width: math.max(1.5, size / 16)) : null,
                    ),
                    child: Text(c.t,
                        style: TextStyle(
                            fontFamily: c.font,
                            fontSize: size * c.scale,
                            height: 1.15,
                            color: c.bg == Punk.ink && onDark ? Punk.ink : c.fg)),
                  ),
                ),
            ],
          ),
          SizedBox(height: size * .1),
          Container(
            color: Punk.pink,
            padding: EdgeInsets.symmetric(horizontal: size * .25),
            child: Text('reader', style: TextStyle(fontFamily: Punk.marker, fontSize: size, height: 1.2, color: Colors.white)),
          ),
        ],
      ),
    );
  }
}

// ---------------------------------------------------------------- logo

/// A torn, typed page with a safety pin through it.
class _PinLogo extends CustomPainter {
  @override
  void paint(Canvas canvas, Size s) {
    final double k = s.width / 96;
    canvas.scale(k);
    final Path page = Path()
      ..moveTo(14, 18)
      ..lineTo(70, 12)
      ..lineTo(66, 26)
      ..lineTo(78, 30)
      ..lineTo(72, 84)
      ..lineTo(20, 88)
      ..lineTo(24, 70)
      ..lineTo(12, 64)
      ..close();
    canvas.drawPath(page, Paint()..color = Colors.white);
    final Paint stroke = Paint()
      ..color = Punk.ink
      ..style = PaintingStyle.stroke
      ..strokeWidth = 3
      ..strokeJoin = StrokeJoin.round;
    canvas.drawPath(page, stroke);
    canvas.drawLine(const Offset(28, 34), const Offset(60, 30), stroke);
    canvas.drawLine(const Offset(28, 44), const Offset(56, 41), stroke);
    canvas.drawLine(const Offset(28, 54), const Offset(62, 50), stroke);
    canvas.save();
    canvas.translate(50, 50);
    canvas.rotate(-35 * math.pi / 180);
    canvas.translate(-50, -50);
    final Path pin = Path()
      ..moveTo(18, 50)
      ..lineTo(80, 50)
      ..arcToPoint(const Offset(80, 66), radius: const Radius.circular(8))
      ..lineTo(26, 66);
    canvas.drawPath(
        pin,
        Paint()
          ..color = Punk.pink
          ..style = PaintingStyle.stroke
          ..strokeWidth = 5
          ..strokeCap = StrokeCap.round);
    canvas.drawRRect(RRect.fromRectAndRadius(const Rect.fromLTWH(8, 44, 16, 12), const Radius.circular(3)),
        Paint()..color = Punk.ink);
    canvas.restore();
  }

  @override
  bool shouldRepaint(covariant CustomPainter old) => false;
}

class _Dashed extends CustomPainter {
  @override
  void paint(Canvas canvas, Size s) {
    final Paint p = Paint()
      ..color = Punk.ink
      ..strokeWidth = 2;
    void dash(Offset a, Offset b) {
      final double len = (b - a).distance;
      final Offset d = (b - a) / len;
      for (double t = 0; t < len; t += 10) {
        canvas.drawLine(a + d * t, a + d * math.min(t + 6, len), p);
      }
    }

    dash(Offset.zero, Offset(s.width, 0));
    dash(Offset(s.width, 0), Offset(s.width, s.height));
    dash(Offset(s.width, s.height), Offset(0, s.height));
    dash(Offset(0, s.height), Offset.zero);
  }

  @override
  bool shouldRepaint(covariant CustomPainter old) => false;
}

// ---------------------------------------------------------------- splash

class _PunkSplash extends StatelessWidget {
  const _PunkSplash();

  @override
  Widget build(BuildContext context) {
    return Material(
      color: Punk.ink,
      child: Center(
        child: Padding(
          padding: const EdgeInsets.all(28),
          child: Column(
            mainAxisSize: MainAxisSize.min,
            children: <Widget>[
              PunkBrand().logo(size: 110),
              const SizedBox(height: 28),
              PunkBrand().wordmark(size: 40, onDark: true),
              const SizedBox(height: 40),
              Text(PunkBrand().splashLine,
                  textAlign: TextAlign.center,
                  style: const TextStyle(fontFamily: Punk.typed, fontSize: 15, color: Punk.paper)),
            ],
          ),
        ),
      ),
    );
  }
}
