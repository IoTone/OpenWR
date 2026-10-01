// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter_test/flutter_test.dart';
import 'package:switchback/goggle/status.dart';
import 'package:switchback/theme/theme_controller.dart';
import 'package:switchback/theme/tokens.dart';

void main() {
  const String line =
      'goggl1 v0.4.2 net=1 ip=10.0.0.202 geo=Portland wx=1 age=22 clock=1 01:27 theme=1 units=1 bundle=weather-hud_0.5 heap=104532 ble=2';

  test('status line parses', () {
    final GoggleStatus s = GoggleStatus.parse(line)!;
    expect(s.version, '0.4.2');
    expect(s.netOk, isTrue);
    expect(s.ip, '10.0.0.202');
    expect(s.city, 'Portland');
    expect(s.forecastOk, isTrue);
    expect(s.forecastAgeS, 22);
    expect(s.forecastFresh, isTrue);
    expect(s.localTime, '01:27');
    expect(s.theme, GlassTheme.glance);
    expect(s.imperial, isTrue);
    expect(s.bundle, 'weather-hud 0.5');
    expect(s.freeHeap, 104532);
    expect(s.ble, 2);
  });

  test('non-status lines are rejected', () {
    expect(GoggleStatus.parse('ok>'), isNull);
    expect(GoggleStatus.parse('wx-test: PASS'), isNull);
  });

  test('wx-dump parses nine hours', () {
    const String dump = '''
2 552 521 1 0 0 6 7 315 3 0
3 545 514 1 0 0 6 7 313 3 0
4 537 501 1 0 0 6 7 312 3 0
5 536 500 1 0 0 7 7 311 3 0
6 537 498 1 0 0 6 8 309 3 0
7 536 496 1 0 0 6 7 318 3 0
8 543 509 1 0 0 5 6 319 3 1
9 577 552 1 0 0 5 7 335 3 1
10 606 586 1 0 0 5 10 347 3 1''';
    final List<ForecastHour> f = parseForecastDump(dump);
    expect(f.length, 9);
    expect(f.first.hour, 2);
    expect(f.first.temp, 55.2);
    expect(f.first.isDay, isFalse);
    expect(f.last.isDay, isTrue);
  });

  group('experience choice', () {
    ForecastHour h({int code = 1, bool day = true, int pop = 0, int gust = 10}) => ForecastHour(
        hour: 12, temp10: 150, feels10: 150, pop: pop, precip10: 0, snow10: 0,
        wind: 5, gust: gust, wdir: 180, code: code, isDay: day);

    test('day and dry is Dawn Patrol', () {
      expect(ThemeController.choose(forecast: <ForecastHour>[h()]), Experience.dawn);
    });
    test('night is Headlamp', () {
      expect(ThemeController.choose(forecast: <ForecastHour>[h(day: false)]), Experience.headlamp);
    });
    test('storm is Squall even at night', () {
      expect(ThemeController.choose(forecast: <ForecastHour>[h(code: 95, day: false)]), Experience.squall);
    });
    test('rain likely within three hours is Squall', () {
      expect(ThemeController.choose(forecast: <ForecastHour>[h(), h(), h(pop: 80)]), Experience.squall);
      expect(ThemeController.choose(forecast: <ForecastHour>[h(), h(), h(), h(pop: 80)]), Experience.dawn);
    });
    test('rain with strong gusts is Squall', () {
      expect(ThemeController.choose(forecast: <ForecastHour>[h(code: 61, gust: 45)]), Experience.squall);
      expect(ThemeController.choose(forecast: <ForecastHour>[h(code: 61, gust: 20)]), Experience.dawn);
    });
    test('no goggle: clock decides', () {
      expect(ThemeController.choose(now: DateTime(2026, 10, 1, 14)), Experience.dawn);
      expect(ThemeController.choose(now: DateTime(2026, 10, 1, 22)), Experience.headlamp);
    });
  });
}
