// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter_test/flutter_test.dart';
import 'package:switchback/goggle/hr_strap.dart';
import 'package:switchback/goggle/session.dart';

void main() {
  group('parseHeartRate', () {
    test('Rhythm 24 on the arm: u8 bpm, contact detected', () {
      final HrSample s = parseHeartRate(<int>[0x06, 0x56])!;
      expect(s.bpm, 86);
      expect(s.contact, isTrue);
      expect(s.rrMs, isEmpty);
    });

    test('Rhythm 24 off the arm: 0 bpm, no contact', () {
      final HrSample s = parseHeartRate(<int>[0x04, 0x00])!;
      expect(s.bpm, 0);
      expect(s.contact, isFalse);
    });

    test('u16 bpm, energy and two RR intervals', () {
      // flags: u16 | contact supported+detected | energy | rr
      final HrSample s = parseHeartRate(<int>[0x1F, 0x2C, 0x01, 0x10, 0x00, 0x00, 0x04, 0x00, 0x02])!;
      expect(s.bpm, 300);
      expect(s.energyKj, 16);
      expect(s.rrMs, <int>[1000, 500]);
    });

    test('contact not supported is null', () {
      expect(parseHeartRate(<int>[0x00, 72])!.contact, isNull);
    });

    test('too short is null', () {
      expect(parseHeartRate(<int>[0x06]), isNull);
      expect(parseHeartRate(<int>[0x01, 0x2C]), isNull);
    });
  });

  group('looksLikeHrStrap', () {
    test('by service, short or long form', () {
      expect(looksLikeHrStrap('', <String>['180d']), isTrue);
      expect(looksLikeHrStrap('', <String>['0000180d-0000-1000-8000-00805f9b34fb']), isTrue);
    });
    test('by name, as the Rhythm 24 advertises with no services', () {
      expect(looksLikeHrStrap('Rhythm 24 5248', const <String>[]), isTrue);
      expect(looksLikeHrStrap('Polar H10 1234', const <String>[]), isTrue);
    });
    test('not the glass', () {
      expect(looksLikeHrStrap('goggl1', <String>['6e400001-b5a3-f393-e0a9-e50e24dcca9e']), isFalse);
      expect(looksLikeHrStrap('maplet', const <String>[]), isFalse);
    });
  });

  group('cadence', () {
    test('RSC from the Rhythm 24: flags, speed, cadence, total distance', () {
      final RscSample s = parseRsc(<int>[0x02, 0x00, 0x00, 0x00, 0x1a, 0x09, 0x00, 0x00])!;
      expect(s.cadence, 0);
      expect(s.speedMps, 0);
      expect(parseRsc(<int>[0x00, 0x80, 0x02, 172])!.cadence, 172);
      expect(parseRsc(<int>[0x00, 0x80, 0x02, 172])!.speedMps, 2.5);
    });

    test('CSC crank data, with and without wheel data ahead of it', () {
      expect(parseCscCrank(<int>[0x02, 0x0a, 0x00, 0x00, 0x04]), (10, 1024));
      expect(parseCscCrank(<int>[0x03, 1, 0, 0, 0, 0, 0, 0x0b, 0x00, 0x00, 0x08]), (11, 2048));
      expect(parseCscCrank(<int>[0x01, 1, 0, 0, 0, 0, 0]), isNull);
    });

    test('crank rpm, across the 16-bit wrap', () {
      expect(crankRpm((10, 1024), (11, 1024 + 683)), 90);
      expect(crankRpm((0xFFFF, 0xFF00), (1, 0xFF00 + 1365 - 0x10000)), 90);
      expect(crankRpm((10, 1024), (10, 1024)), -1);
    });
  });

  test('zones are percent of max', () {
    expect(hrZone(0, 185), 0);
    expect(hrZone(75, 185), 1);
    expect(hrZone(120, 185), 2);
    expect(hrZone(148, 185), 4);
    expect(hrZone(170, 185), 5);
  });

  test('sess-x! needs the 0.7 bundle', () {
    expect(bundleHasSessX('weather-hud 0.6'), isFalse);
    expect(bundleHasSessX('weather-hud 0.7'), isTrue);
    expect(bundleHasSessX('weather-hud_0.12'), isTrue);
    expect(bundleHasSessX('-'), isFalse);
    expect(bundleHasSessX(null), isFalse);
  });

  test('session summary keeps heart rate, and old logs read as none', () {
    final SessionSummary s = SessionSummary(
      sport: Sport.ride, startedAt: DateTime.utc(2026, 10, 14), seconds: 60, meters: 500, imperial: false, hrAvg: 131, hrMax: 162,
    );
    final SessionSummary r = SessionSummary.fromJson(s.toJson());
    expect(r.hrAvg, 131);
    expect(r.hrMax, 162);
    final Map<String, dynamic> old = s.toJson()..remove('hrAvg')..remove('hrMax');
    expect(SessionSummary.fromJson(old).hrMax, 0);
  });
}
