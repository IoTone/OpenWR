\ graphics.fs — port of U8g2's GraphicsTest.ino as shipped in the DFRobot
\ demo folder for the DFR0934.
\
\ The original steps a counter `draw_state` from 0..111 at 150 ms per
\ frame; the high bits pick the scene, the low 3 bits (`a`) animate it.
\ The scenes that need U8g2's own fonts (ASCII page 2 = Latin-1, the
\ unicode symbol page) are left out; everything the 5x7 font and the
\ primitives can do is here.
\
\ Lines are kept short because the REPL takes at most 255 chars per line.

variable a          \ animation phase 0..7

\ --- scene 0: title ------------------------------------------------
: g-title  ( -- )
  s" U8g2" 10 a @ 2 * + 5 oled-str
  s" GraphicsTest" 10 20 oled-str
  0 0 oled-width oled-height oled-frame ;

\ --- scene 1: box / frame -----------------------------------------
: g-box  ( -- )
  s" drawBox" 0 0 oled-str
  5 10 20 10 oled-box
  10 a @ + 15 30 7 oled-box
  s" drawFrame" 0 30 oled-str
  5 40 20 10 oled-frame
  10 a @ + 45 30 7 oled-frame ;

\ --- scene 2: disc / circle ---------------------------------------
: g-disc  ( -- )
  s" drawDisc" 0 0 oled-str
  10 18 9 oled-disc
  24 a @ + 16 7 oled-disc
  s" drawCircle" 0 30 oled-str
  10 48 9 oled-circle
  24 a @ + 46 7 oled-circle ;

\ --- scene 3: rounded frame / box ---------------------------------
: g-rframe  ( -- )
  s" drawRFrame/Box" 0 0 oled-str
  5 10 40 30 a @ 1 + oled-rframe
  50 10 25 40 a @ 1 + oled-rbox ;

\ --- scene 4: text in four directions -----------------------------
: g-string  ( -- )
  0 oled-dir  s"  0"   30 a @ + 31 oled-str
  1 oled-dir  s"  90"  30 31 a @ + oled-str
  2 oled-dir  s"  180" 30 a @ - 31 oled-str
  3 oled-dir  s"  270" 30 31 a @ - oled-str
  0 oled-dir ;

\ --- scene 5: lines -----------------------------------------------
: g-line  ( -- )
  s" drawLine" 0 0 oled-str
  7 a @ + 10 40 55 oled-line
  7 a @ 2 * + 10 60 55 oled-line
  7 a @ 3 * + 10 80 55 oled-line
  7 a @ 4 * + 10 100 55 oled-line ;

\ --- scene 6: triangles -------------------------------------------
: g-triangle  ( -- )
  s" drawTriangle" 0 0 oled-str
  14 7 45 30 10 40 oled-triangle
  14 a @ + 7 a @ - 45 a @ + 30 a @ - 57 a @ + 10 a @ - oled-triangle
  57 a @ 2 * + 10 45 a @ 2 * + 30 86 a @ 2 * + 53 oled-triangle
  10 a @ + 40 a @ + 45 a @ + 30 a @ + 86 a @ + 53 a @ + oled-triangle ;

\ --- scene 7: ASCII table -----------------------------------------
\ 16 columns x 6 rows of 7x10 px cells starting at char 32.
: g-ascii  ( -- )
  s" ASCII page 1" 0 0 oled-str
  6 0 do 16 0 do
    j 16 * i + 32 +  i 7 *  j 10 * 10 +  oled-glyph
  loop loop ;

\ --- scene 8: XOR -------------------------------------------------
\ U8g2 draws five boxes in XOR, then text in XOR / clear / set over them.
: g-xor  ( -- )
  s" XOR" 0 0 oled-str
  2 oled-color
  5 0 do 10 i 16 * + 18 i 1 and 4 * + 21 31 oled-box loop
  s" XOR XOR XOR XOR" 5 a @ + 19 oled-str
  0 oled-color s" CLR CLR CLR CLR" 5 a @ + 29 oled-str
  1 oled-color s" SET SET SET SET" 5 a @ + 39 oled-str ;

\ --- scene 9: bitmap ----------------------------------------------
\ cross_bits from GraphicsTest.ino: 24x24 XBM, 3 bytes per row.
variable cross 71 allot          \ 72 bytes: the variable cell + 68 more
: >cross ( b idx -- ) cross + c! ;
0x00 0 >cross 0x18 1 >cross 0x00 2 >cross 0x00 3 >cross 0x24 4 >cross 0x00 5 >cross
0x00 6 >cross 0x24 7 >cross 0x00 8 >cross 0x00 9 >cross 0x42 10 >cross 0x00 11 >cross
0x00 12 >cross 0x42 13 >cross 0x00 14 >cross 0x00 15 >cross 0x42 16 >cross 0x00 17 >cross
0x00 18 >cross 0x81 19 >cross 0x00 20 >cross 0x00 21 >cross 0x81 22 >cross 0x00 23 >cross
0xC0 24 >cross 0x00 25 >cross 0x03 26 >cross 0x38 27 >cross 0x3C 28 >cross 0x1C 29 >cross
0x06 30 >cross 0x42 31 >cross 0x60 32 >cross 0x01 33 >cross 0x42 34 >cross 0x80 35 >cross
0x01 36 >cross 0x42 37 >cross 0x80 38 >cross 0x06 39 >cross 0x42 40 >cross 0x60 41 >cross
0x38 42 >cross 0x3C 43 >cross 0x1C 44 >cross 0xC0 45 >cross 0x00 46 >cross 0x03 47 >cross
0x00 48 >cross 0x81 49 >cross 0x00 50 >cross 0x00 51 >cross 0x81 52 >cross 0x00 53 >cross
0x00 54 >cross 0x42 55 >cross 0x00 56 >cross 0x00 57 >cross 0x42 58 >cross 0x00 59 >cross
0x00 60 >cross 0x42 61 >cross 0x00 62 >cross 0x00 63 >cross 0x24 64 >cross 0x00 65 >cross
0x00 66 >cross 0x24 67 >cross 0x00 68 >cross 0x00 69 >cross 0x18 70 >cross 0x00 71 >cross

: g-bitmap  ( -- )
  s" Bitmap overlay" 0 0 oled-str
  s" Solid / transparent" 0 40 oled-str
  0 10 28 28 oled-frame
  cross 24 24 2 12 oled-xbm-solid
  33 10 28 28 oled-frame
  cross 24 24 35 12 oled-xbm ;

\ --- driver -------------------------------------------------------
variable scene
: draw  ( -- )
  oled-clear 1 oled-color 0 oled-dir 1 oled-scale
  scene @
  dup 0 = if g-title then
  dup 1 = if g-box then
  dup 2 = if g-disc then
  dup 3 = if g-rframe then
  dup 4 = if g-string then
  dup 5 = if g-line then
  dup 6 = if g-triangle then
  dup 7 = if g-ascii then
  dup 8 = if g-xor then
  dup 9 = if g-bitmap then
  drop oled-show ;

\ One pass through every scene, 8 phases each, 150 ms per frame — the
\ same cadence as the Arduino loop(). About 12 seconds.
: graphics-test  ( -- )
  10 0 do i scene !
    8 0 do i a ! draw 150 ms loop
  loop ;

\ Loop until the BOOT button is pressed.
: graphics-loop  ( -- )
  begin graphics-test button? until ;

graphics-test
