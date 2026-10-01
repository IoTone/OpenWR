// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';

import '../goggle/status.dart';

enum ForecastGraphMode { temperature, rain }

/// The eight-hour strip as a small chart: temperature as a line with a
/// soft fill, or rain probability as bars. Hour labels every two hours.
class ForecastGraph extends StatelessWidget {
  const ForecastGraph({super.key, required this.forecast, this.mode = ForecastGraphMode.temperature});
  final List<ForecastHour> forecast;
  final ForecastGraphMode mode;

  @override
  Widget build(BuildContext context) {
    final ColorScheme cs = Theme.of(context).colorScheme;
    return CustomPaint(
      painter: _Painter(
        forecast: forecast,
        mode: mode,
        line: mode == ForecastGraphMode.rain ? cs.secondary : cs.primary,
        label: cs.onSurfaceVariant,
        grid: cs.outline,
      ),
      size: Size.infinite,
    );
  }
}

class _Painter extends CustomPainter {
  _Painter({required this.forecast, required this.mode, required this.line, required this.label, required this.grid});
  final List<ForecastHour> forecast;
  final ForecastGraphMode mode;
  final Color line;
  final Color label;
  final Color grid;

  @override
  void paint(Canvas canvas, Size size) {
    if (forecast.isEmpty) return;
    const double labelH = 16;
    final double h = size.height - labelH;
    final double colW = size.width / forecast.length;

    final Paint gridP = Paint()..color = grid.withValues(alpha: .5)..strokeWidth = 1;
    canvas.drawLine(Offset(0, h), Offset(size.width, h), gridP);

    if (mode == ForecastGraphMode.rain) {
      final Paint bar = Paint()..color = line;
      for (int i = 0; i < forecast.length; i++) {
        final double bh = h * forecast[i].pop / 100;
        final Rect r = Rect.fromLTWH(i * colW + colW * .2, h - bh, colW * .6, bh);
        canvas.drawRRect(RRect.fromRectAndRadius(r, const Radius.circular(2)), bar);
      }
    } else {
      final List<double> temps = forecast.map((ForecastHour f) => f.temp).toList();
      double lo = temps.reduce((double a, double b) => a < b ? a : b);
      double hi = temps.reduce((double a, double b) => a > b ? a : b);
      if (hi - lo < 4) {
        hi += 2;
        lo -= 2;
      }
      final Path p = Path();
      for (int i = 0; i < temps.length; i++) {
        final double x = i * colW + colW / 2;
        final double y = 6 + (h - 12) * (1 - (temps[i] - lo) / (hi - lo));
        if (i == 0) {
          p.moveTo(x, y);
        } else {
          p.lineTo(x, y);
        }
      }
      final Path fill = Path.from(p)
        ..lineTo((temps.length - 1) * colW + colW / 2, h)
        ..lineTo(colW / 2, h)
        ..close();
      canvas.drawPath(fill, Paint()..color = line.withValues(alpha: .12));
      canvas.drawPath(
        p,
        Paint()
          ..color = line
          ..style = PaintingStyle.stroke
          ..strokeWidth = 2.5
          ..strokeCap = StrokeCap.round
          ..strokeJoin = StrokeJoin.round,
      );
      final double x0 = colW / 2;
      final double y0 = 6 + (h - 12) * (1 - (temps[0] - lo) / (hi - lo));
      canvas.drawCircle(Offset(x0, y0), 4, Paint()..color = line);
    }

    final TextStyle ts = TextStyle(color: label, fontSize: 11, fontFamily: 'IBMPlexMono');
    for (int i = 0; i < forecast.length; i += 2) {
      final TextPainter tp = TextPainter(
        text: TextSpan(text: forecast[i].hour.toString().padLeft(2, '0'), style: ts),
        textDirection: TextDirection.ltr,
      )..layout();
      tp.paint(canvas, Offset(i * colW + colW / 2 - tp.width / 2, h + 3));
    }
  }

  @override
  bool shouldRepaint(_Painter old) =>
      old.forecast != forecast || old.mode != mode || old.line != line;
}
