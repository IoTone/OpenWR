// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../goggle/status.dart';
import 'tokens.dart';

/// Appearance and accessibility. The active [Experience] is chosen from
/// the goggle's weather and clock ([setFromGoggle]) unless the user pins
/// one. Font scale, high contrast, reduce motion and red shift persist.
class ThemeController extends ChangeNotifier {
  static const String _kPinned = 'app.pinnedExperience';
  static const String _kFontScale = 'app.fontScale';
  static const String _kHighContrast = 'app.highContrast';
  static const String _kReduceMotion = 'app.reduceMotion';
  static const String _kRedShift = 'app.redShift';

  Experience? _pinned;
  Experience _auto = Experience.dawn;
  double _fontScale = 1.0;
  bool _highContrast = false;
  bool _reduceMotion = false;
  bool _redShift = false;

  Experience? get pinned => _pinned;
  Experience get auto => _auto;
  Experience get experience => _pinned ?? _auto;
  double get fontScale => _fontScale;
  bool get highContrast => _highContrast;
  bool get reduceMotion => _reduceMotion;
  bool get redShift => _redShift;

  AppTokens get tokens {
    final Experience e = experience;
    if (e == Experience.headlamp && _redShift) return kHeadlampRed;
    return kExperiences[e]!;
  }

  ThemeData get theme => buildAppTheme(tokens);

  /// The rule from the design: Squall wins over Headlamp, which wins over
  /// Dawn Patrol. With no goggle, the phone's clock decides day or night.
  static Experience choose({GoggleStatus? status, List<ForecastHour> forecast = const <ForecastHour>[], DateTime? now}) {
    if (forecast.isNotEmpty) {
      final ForecastHour h0 = forecast.first;
      final bool gusty = h0.gust >= 40;
      final bool rainSoon = forecast.take(3).any((ForecastHour h) => h.pop >= 70);
      if (h0.stormy || (h0.rainy && gusty) || rainSoon) return Experience.squall;
      if (!h0.isDay) return Experience.headlamp;
      return Experience.dawn;
    }
    final DateTime t = now ?? DateTime.now();
    return (t.hour < 6 || t.hour >= 19) ? Experience.headlamp : Experience.dawn;
  }

  void setFromGoggle({GoggleStatus? status, List<ForecastHour> forecast = const <ForecastHour>[]}) {
    final Experience e = choose(status: status, forecast: forecast);
    if (e == _auto) return;
    _auto = e;
    notifyListeners();
  }

  Future<void> load() async {
    final SharedPreferences p = await SharedPreferences.getInstance();
    final String? pin = p.getString(_kPinned);
    _pinned = pin == null ? null : experienceFromId(pin);
    _fontScale = (p.getDouble(_kFontScale) ?? 1.0).clamp(0.8, 1.6);
    _highContrast = p.getBool(_kHighContrast) ?? false;
    _reduceMotion = p.getBool(_kReduceMotion) ?? false;
    _redShift = p.getBool(_kRedShift) ?? false;
    _auto = choose();
    notifyListeners();
  }

  Future<void> _save(void Function(SharedPreferences p) write) async {
    final SharedPreferences p = await SharedPreferences.getInstance();
    write(p);
  }

  Future<void> pin(Experience? e) async {
    _pinned = e;
    notifyListeners();
    await _save((SharedPreferences p) =>
        e == null ? p.remove(_kPinned) : p.setString(_kPinned, e.name));
  }

  Future<void> setFontScale(double v) async {
    final double c = v.clamp(0.8, 1.6);
    if (c == _fontScale) return;
    _fontScale = c;
    notifyListeners();
    await _save((SharedPreferences p) => p.setDouble(_kFontScale, c));
  }

  Future<void> setHighContrast(bool v) async {
    _highContrast = v;
    notifyListeners();
    await _save((SharedPreferences p) => p.setBool(_kHighContrast, v));
  }

  Future<void> setReduceMotion(bool v) async {
    _reduceMotion = v;
    notifyListeners();
    await _save((SharedPreferences p) => p.setBool(_kReduceMotion, v));
  }

  Future<void> setRedShift(bool v) async {
    _redShift = v;
    notifyListeners();
    await _save((SharedPreferences p) => p.setBool(_kRedShift, v));
  }
}
