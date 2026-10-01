\ 11-glance.fs — Glance theme: the headline (design 7.2)
\ Current temperature at 3x with a 16 px icon; the three facts that change
\ a decision on the right; a sparkline with a rain band underneath.

: glance-now  ( -- )
  0 wx-temp r10
  3 oled-scale dup 0 1 oled-num 1 oled-scale
  ndig 18 * 1 + 2 2 2 oled-box
  0 wx-code 40 4 wx-icon2 ;

: glance-rain  ( -- )
  first-rain dup 0< if drop s" dry 8h" 62 1 oled-str else
    s" rain" 62 1 oled-str hour 92 1 oled-num02 s" :00" 104 1 oled-str then
  popmax 74 10 oled-numr 37 75 10 oled-glyph
  rainmax dup 10 / 86 10 oled-num 46 92 10 oled-glyph 10 mod 98 10 oled-num
  s" mm" 104 10 oled-str ;

: glance-wind  ( -- )
  s" wind" 62 19 oled-str windmax 92 19 oled-num
  47 104 19 oled-glyph gustmax 110 19 oled-num ;   \ "wind 7/11" = speed/gust

: glance-range  ( -- )
  s" lo" 62 28 oled-str tmin r10 80 28 oled-num
  s" hi" 98 28 oled-str tmax r10 116 28 oled-num ;

: ty2  ( t10 -- y )  tlo @ - 10 * thi @ tlo @ - 1 max / 48 swap - ;

: glance-spark  ( -- )
  8 0 do i 16 * i wx-temp ty2 i 1 + 16 * i 1 + wx-temp ty2 oled-line loop
  9 0 do
    i wx-pop dup 50 >= if drop i 16 * 52 16 oled-hline i 16 * 53 16 oled-hline
    else 20 >= if 8 0 do j 16 * i 2 * + 52 oled-pixel loop then then
  loop ;

: glance  ( -- )
  tmin tlo ! tmax thi !
  glance-now glance-rain glance-wind glance-range glance-spark ;
