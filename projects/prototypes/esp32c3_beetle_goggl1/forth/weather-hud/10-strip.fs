\ 10-strip.fs — Strip theme: the instrument (design 7.1)
\ Nine columns 13 px apart. Temperature line, precipitation bars, wind
\ every 2 h, hour labels every 2 h, dotted line at the current hour.

variable tlo
variable thi

: sx  ( i -- x )  13 * 12 + ;

\ temperature to a row in 1..25
: ty  ( t10 -- y )  tlo @ - 24 * thi @ tlo @ - 1 max / 25 swap - ;

\ precipitation bar from the baseline at row 40
: pbar  ( x h -- )  dup >r 40 swap - 5 r> oled-box ;

: strip-temp  ( -- )
  8 0 do i sx i wx-temp ty i 1 + sx i 1 + wx-temp ty oled-line loop
  9 0 do i sx i wx-temp ty 2dup 1 - oled-pixel 1 + oled-pixel loop
  thi @ r10 0 0 oled-num  tlo @ r10 0 19 oled-num ;

: strip-rain  ( -- )
  37 0 30 oled-glyph
  9 0 do i wx-pop 12 * 100 / dup 0 > if i sx 2 - swap pbar else drop then loop
  10 40 118 oled-hline ;

: strip-wind  ( -- )
  119 0 42 oled-glyph
  5 0 do i 2 * dup wx-wind swap sx 5 - 42 oled-num loop ;

: strip-hours  ( -- )
  5 0 do i 2 * dup hour swap sx 5 - 49 oled-num02 loop
  55 0 do 0 sx i oled-pixel 2 +loop ;

: strip  ( -- )
  tmin tlo ! tmax thi !
  strip-temp strip-rain strip-wind strip-hours ;
