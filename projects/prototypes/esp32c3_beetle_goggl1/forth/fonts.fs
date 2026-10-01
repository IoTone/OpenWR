\ fonts.fs — ports of DFRobot's Font.ino and FontDirection.ino.
\
\ Both demos exist to show off U8g2's font library. We have one font,
\ so what survives the port is the *layout*: Font.ino's two pages of
\ "DFR123" at different sizes, and FontDirection.ino's "DFR" written in
\ all four directions around a 30x30 icon.

\ --- Font.ino -------------------------------------------------------
\ Page 0: three sizes stacked.  Page 1: four more, staggered.
: font-page0  ( -- )
  3 oled-scale s" DFR123" 0 0 oled-str
  2 oled-scale s" DFR123" 0 25 oled-str
  1 oled-scale s" DFR123" 0 45 oled-str ;

: font-page1  ( -- )
  2 oled-scale s" DFR123" 0 0 oled-str
  1 oled-scale s" DFR123" 56 16 oled-str
  2 oled-scale s" DFR123" 0 32 oled-str
  1 oled-scale s" DFR123" 56 48 oled-str ;

: font-demo  ( -- )
  oled-clear font-page0 oled-show 2000 ms
  oled-clear font-page1 oled-show 2000 ms
  1 oled-scale ;

\ --- FontDirection.ino ----------------------------------------------
\ The 30x30 "col" icon is a weather glyph; a disc stands in for it.
: dir-icon  ( -- ) 15 15 14 oled-circle 15 15 9 oled-disc ;

\ "DFR" drawn from (64,32) in each of the four directions at once.
: dir-rose  ( -- )
  oled-clear dir-icon
  0 oled-dir s"  DFR" 64 32 oled-str
  1 oled-dir s"  DFR" 64 32 oled-str
  2 oled-dir s"  DFR" 64 32 oled-str
  3 oled-dir s"  DFR" 64 32 oled-str
  0 oled-dir oled-show ;

\ Then one direction at a time, 1.5 s each.
: dir-step  ( n -- )
  oled-clear dir-icon
  oled-dir s" DFR" 64 32 oled-str
  0 oled-dir oled-show 1500 ms ;

: direction-demo  ( -- )
  2 oled-scale
  dir-rose 1500 ms
  4 0 do i dir-step loop
  1 oled-scale ;

font-demo
direction-demo
