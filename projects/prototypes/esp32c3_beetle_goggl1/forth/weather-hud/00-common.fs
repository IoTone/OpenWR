\ bundle: weather-hud 0.4
\ requires: oled-str oled-num wx-temp wx-icon geo-city clock-hm theme@ net-ok? ble-on?
\ 00-common.fs — shared helpers and the status strip for the weather HUD.
\ Load order: 00-common, 10-strip, 11-glance, 12-ribbon, 20-hud.
\ Requires the goggl1 firmware words: oled-*, wx-*, geo-*, clock-*, net-ok?

\ round a x10 value to the nearest whole unit (C division truncates to 0)
: r10  ( t10 -- t )  dup 0< if 5 - else 5 + then 10 / ;

\ local hour label for forecast slot i
: hour  ( i -- h )  wx-hour0 + 24 mod ;

\ window extremes, all over the 9 slots
: tmin  ( -- t10 )  0 wx-temp 9 1 do i wx-temp min loop ;
: tmax  ( -- t10 )  0 wx-temp 9 1 do i wx-temp max loop ;
: popmax  ( -- % )  0 9 0 do i wx-pop max loop ;
: rainmax ( -- mm10 )  0 9 0 do i wx-precip max loop ;
: windmax ( -- v )  0 9 0 do i wx-wind max loop ;
: gustmax ( -- v )  0 9 0 do i wx-gust max loop ;

\ first slot with rain probability >= 50, or -1
: first-rain  ( -- i )
  -1 9 0 do dup 0< if i wx-pop 50 >= if drop i then then loop ;

\ digits an integer will take, sign included
: ndig  ( n -- n )
  dup 0< if negate 1 else 0 then swap
  dup 10 < if drop 1 + else 100 < if 2 + else 3 + then then ;

\ --- status strip: rows 56..63, over the opaque backing ---------------
\ city left; freshness mark and local clock right
: hud-clock  ( -- )
  clock-ok? if
    clock-hm swap 98 57 oled-num02 116 57 oled-num02 58 110 57 oled-glyph
  else s" --:--" 98 57 oled-str then ;

: hud-fresh  ( -- )
  net-ok? 0= if 120 120 57 oled-glyph else
  wx-age dup 0< swap 1800 < or if 90 60 2 oled-disc else 90 60 2 oled-circle then then ;

: hud-ble  ( -- )  ble-on? if 66 78 57 oled-glyph then ;   \ B while pairing

: hud-status  ( -- )
  0 56 128 oled-hline
  geo-ok? if geo-city 10 min 0 57 oled-str else s" no location" 0 57 oled-str then
  hud-ble hud-fresh hud-clock ;
