\ 12-ribbon.fs — Ribbon theme: the hourly strip (design 7.3)
\ Nine 14 px columns: hour, icon, temperature, precipitation (or snow),
\ wind speed, wind direction tick. Current hour underlined.

\ precipitation cell at column x: snow cm with a * if any, else pop %
\ (a dot pair when under 20)
: ribbon-precip  ( i x -- )
  over wx-snow dup 0 > if 10 / over 28 oled-num 42 over 8 + 28 oled-glyph else drop
    over wx-pop dup 20 >= if over 28 oled-num else drop
      dup 3 + 31 oled-pixel dup 7 + 31 oled-pixel then then
  2drop ;

: ribbon-col  ( i -- )
  dup 14 * 1 +
  over hour over 0 oled-num02
  over wx-code over 1 + 9 wx-icon
  over wx-temp r10 over 19 oled-num
  2dup ribbon-precip
  over wx-wind over 37 oled-num
  over wx-wdir over 5 + 49 wx-arrow
  2drop ;

: ribbon  ( -- )
  9 0 do i ribbon-col loop
  13 frame @ 3 mod do i 7 2 oled-hline 3 +loop ;   \ marching underline on the current hour
