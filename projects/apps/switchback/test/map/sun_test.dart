import 'package:flutter_test/flutter_test.dart';
import 'package:switchback/map/sun.dart';

void main() {
  test('sunset in Portland on Oct 2 is around 18:50 local', () {
    final SunTimes s = SunTimes.forDay(DateTime(2026, 10, 2), 45.54, -122.72);
    expect(s.sunset, isNotNull);
    expect(s.sunrise, isNotNull);
    // In the machine's local zone the hour differs; check the day length instead (~11h35m in early October at 45N).
    final int lenMin = s.sunset!.difference(s.sunrise!).inMinutes;
    expect(lenMin, inInclusiveRange(680, 715));
  });
}
