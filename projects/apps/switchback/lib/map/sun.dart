// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// Sunrise and sunset from the NOAA solar calculator, good to a minute or
// two, so the Field Notes page can say when the light goes without asking
// anyone on the network.
import 'dart:math' as math;

class SunTimes {
  const SunTimes({required this.sunrise, required this.sunset});
  final DateTime? sunrise; // local
  final DateTime? sunset;

  static SunTimes forDay(DateTime day, double lat, double lon) {
    final DateTime d = DateTime(day.year, day.month, day.day);
    final int doy = d.difference(DateTime(d.year, 1, 1)).inDays + 1;
    final double tzHours = d.timeZoneOffset.inMinutes / 60;
    double? calc(bool rise) {
      final double lngHour = lon / 15;
      final double t = doy + ((rise ? 6 : 18) - lngHour) / 24;
      final double m = 0.9856 * t - 3.289;
      double l = m + 1.916 * _sin(m) + 0.020 * _sin(2 * m) + 282.634;
      l = l % 360;
      double ra = _atan(0.91764 * _tan(l)) % 360;
      final double lq = (l / 90).floor() * 90, raq = (ra / 90).floor() * 90;
      ra = (ra + (lq - raq)) / 15;
      final double sinDec = 0.39782 * _sin(l), cosDec = _cos(_asin(sinDec));
      const double zenith = 90.833;
      final double cosH = (_cos(zenith) - sinDec * _sin(lat)) / (cosDec * _cos(lat));
      if (cosH > 1 || cosH < -1) return null; // polar day or night
      double h = rise ? 360 - _acos(cosH) : _acos(cosH);
      h /= 15;
      final double tt = h + ra - 0.06571 * t - 6.622;
      return ((tt - lngHour) % 24 + 24) % 24 + tzHours;
    }
    DateTime? toLocal(double? ut) {
      if (ut == null) return null;
      final double hh = ((ut % 24) + 24) % 24;
      return d.add(Duration(minutes: (hh * 60).round()));
    }
    return SunTimes(sunrise: toLocal(calc(true)), sunset: toLocal(calc(false)));
  }

  static double _sin(double deg) => math.sin(deg * math.pi / 180);
  static double _cos(double deg) => math.cos(deg * math.pi / 180);
  static double _tan(double deg) => math.tan(deg * math.pi / 180);
  static double _asin(double v) => math.asin(v) * 180 / math.pi;
  static double _acos(double v) => math.acos(v) * 180 / math.pi;
  static double _atan(double v) => math.atan(v) * 180 / math.pi;
}
