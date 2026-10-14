import 'package:flutter_test/flutter_test.dart';
import 'package:switchback/goggle/console.dart';
import 'package:switchback/map/eink_canvas.dart';

void main() {
  test('the maplet status names its panel', () {
    expect(GoggleConsole.parseEpdField('epd=ssd1680-290:296x128'), ('ssd1680-290', 296, 128));
    expect(GoggleConsole.parseEpdField('epd=uc8151-213flex:212x104'), ('uc8151-213flex', 212, 104));
    expect(GoggleConsole.parseEpdField('map=1'), isNull);
  });

  test('a 296x128 canvas packs to 4736 bytes and back', () {
    final EinkCanvas c = EinkCanvas(width: 296, height: 128)..set(295, 127, 1)..set(0, 0, 1);
    final EinkCanvas back = EinkCanvas.unpack(c.pack(), width: 296, height: 128);
    expect(c.pack().length, 37 * 128);
    expect(back.get(295, 127), 1);
    expect(back.get(0, 0), 1);
    expect(back.get(1, 0), 0);
  });
}
