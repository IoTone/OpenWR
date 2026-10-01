// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The goggle's one-line `status` report, parsed. The firmware prints:
//
//   goggl1 v0.4.1 net=1 ip=10.0.0.202 geo=Portland wx=1 age=22 clock=1 01:27
//   theme=1 units=1 bundle=weather-hud_0.5 heap=104532 ble=2
//
// Everything is `key=value` split on spaces, except the leading name and
// version and the bare local time that follows `clock=`.

/// Which glass theme the goggle is showing. Matches `theme!` indices.
enum GlassTheme { strip, glance, ribbon }

extension GlassThemeX on GlassTheme {
  String get label => switch (this) {
        GlassTheme.strip => 'Strip',
        GlassTheme.glance => 'Glance',
        GlassTheme.ribbon => 'Ribbon',
      };
  String get asset => 'assets/images/glass/theme-$name.png';
}

class GoggleStatus {
  const GoggleStatus({
    required this.version,
    required this.netOk,
    required this.ip,
    required this.city,
    required this.forecastOk,
    required this.forecastAgeS,
    required this.clockOk,
    required this.localTime,
    required this.theme,
    required this.imperial,
    required this.bundle,
    required this.freeHeap,
    required this.ble,
    required this.receivedAt,
  });

  final String version;
  final bool netOk;
  final String ip;
  final String city;
  final bool forecastOk;
  final int forecastAgeS; // -1 when unknown
  final bool clockOk;
  final String localTime; // "01:27"
  final GlassTheme theme;
  final bool imperial;
  final String bundle;
  final int freeHeap;
  final int ble; // 0 off, 1 advertising, 2 connected
  final DateTime receivedAt;

  bool get forecastFresh =>
      forecastOk && forecastAgeS >= 0 && forecastAgeS < 30 * 60;

  /// Parse a status line. Returns null if it is not one.
  static GoggleStatus? parse(String line, {DateTime? now}) {
    final List<String> parts = line.trim().split(RegExp(r'\s+'));
    if (parts.length < 3 || parts[0] != 'goggl1') return null;
    final Map<String, String> kv = <String, String>{};
    String localTime = '--:--';
    for (int i = 1; i < parts.length; i++) {
      final String p = parts[i];
      final int eq = p.indexOf('=');
      if (eq > 0) {
        kv[p.substring(0, eq)] = p.substring(eq + 1);
      } else if (RegExp(r'^\d\d:\d\d$').hasMatch(p)) {
        localTime = p;
      }
    }
    int intOf(String k, [int dflt = 0]) => int.tryParse(kv[k] ?? '') ?? dflt;
    final int themeIdx = intOf('theme').clamp(0, GlassTheme.values.length - 1);
    return GoggleStatus(
      version: parts[1].startsWith('v') ? parts[1].substring(1) : parts[1],
      netOk: intOf('net') == 1,
      ip: kv['ip'] ?? '',
      city: (kv['geo'] ?? '-').replaceAll('_', ' '),
      forecastOk: intOf('wx') == 1,
      forecastAgeS: intOf('age', -1),
      clockOk: intOf('clock') == 1,
      localTime: localTime,
      theme: GlassTheme.values[themeIdx],
      imperial: intOf('units') == 1,
      bundle: (kv['bundle'] ?? '-').replaceAll('_', ' '),
      freeHeap: intOf('heap'),
      ble: intOf('ble'),
      receivedAt: now ?? DateTime.now(),
    );
  }
}

/// One forecast hour as the `wx-*` words report it, in the goggle's units.
class ForecastHour {
  const ForecastHour({
    required this.hour,
    required this.temp10,
    required this.feels10,
    required this.pop,
    required this.precip10,
    required this.snow10,
    required this.wind,
    required this.gust,
    required this.wdir,
    required this.code,
    required this.isDay,
  });
  final int hour;
  final int temp10;
  final int feels10;
  final int pop;
  final int precip10;
  final int snow10;
  final int wind;
  final int gust;
  final int wdir;
  final int code;
  final bool isDay;

  double get temp => temp10 / 10;
  double get feels => feels10 / 10;

  bool get rainy => (code >= 51 && code <= 67) || (code >= 80 && code <= 82);
  bool get snowy => (code >= 71 && code <= 77) || code == 85 || code == 86;
  bool get stormy => code >= 95;
  bool get foggy => code == 45 || code == 48;
  bool get clear => code <= 1;
}

/// Parse the reply to `wx-dump`, nine lines of eleven integers:
///   hour temp10 feels10 pop precip10 snow10 wind gust wdir code day
List<ForecastHour> parseForecastDump(String text) {
  final List<ForecastHour> out = <ForecastHour>[];
  for (final String raw in text.split('\n')) {
    final List<int> n = raw
        .trim()
        .split(RegExp(r'\s+'))
        .map((String s) => int.tryParse(s))
        .whereType<int>()
        .toList();
    if (n.length != 11) continue;
    out.add(ForecastHour(
      hour: n[0],
      temp10: n[1],
      feels10: n[2],
      pop: n[3],
      precip10: n[4],
      snow10: n[5],
      wind: n[6],
      gust: n[7],
      wdir: n[8],
      code: n[9],
      isDay: n[10] != 0,
    ));
  }
  return out;
}
