\ hello.fs — port of DFRobot's HelloWorld.ino for the DFR0934
\
\ Original (U8g2):
\     u8g2.clearBuffer();
\     u8g2.setFont(u8g2_font_t0_17b_tr);
\     u8g2.drawStr(0, 5, "Hello World!");
\     u8g2.sendBuffer();
\
\ The t0_17b font is tall and narrow: ~16 px high, ~8 px per glyph, so
\ "Hello World!" is about 100 px wide. The built-in 5x7 font at a uniform
\ scale 2 is 12 px per glyph, 144 px for 12 characters, and the "d!" ran
\ off the right edge of the glass (seen on hardware 2026-10-01). Scaling
\ 1 wide by 2 tall gives 72 x 14 px, which matches the original's shape.
\
\ Paste at the ok> prompt, or send the file. Requires the goggl1 words.

: hello  ( -- )
  oled-clear
  1 2 oled-scale-xy
  s" Hello World!" 0 5 oled-str
  1 oled-scale
  oled-show ;

hello
