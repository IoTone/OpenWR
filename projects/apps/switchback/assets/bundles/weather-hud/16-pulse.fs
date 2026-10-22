\ 16-pulse.fs — heart rate first, streamed by the glasses (SpecsSwitchbackXR).
\ Loads after 15-session (it shares s-secs s-hr s-zone s-cad s-cadk), before
\ 20-hud.
\
\ The glasses read the strap themselves and send one line a second:
\
\     secs bpm zone cad kind sess-hr!     ( e.g. 1934 148 4 172 1 sess-hr! )
\
\ secs is the session's elapsed time, bpm 0 means no reading (strap off or
\ no skin contact), zone 1..5 (0 none), cad and kind as for sess-x!. The
\ first line switches the glass to the Pulse layout; when lines stop for
\ 30 s it goes back to whatever it showed before, so a lens that closes or
\ walks out of range never strands the goggle. pulse-off does it at once.

variable pulse-on?  0 pulse-on? !
variable p-last                     \ us when the last line arrived

: p-age  ( -- us )  us p-last @ - ;

: sess-hr!  ( secs bpm zone cad kind -- )
  s-cadk ! s-cad ! s-zone ! s-hr ! s-secs !
  us p-last !  -1 pulse-on? !  hud-now ;      \ hud-now only flags a redraw

: pulse-off  ( -- )  0 pulse-on? ! hud-now ;

\ on, and heard from in the last 30 s; switches itself off otherwise
: pulse-live?  ( -- flag )
  pulse-on? @ if
    p-age 30000000 < dup 0= if 0 pulse-on? ! then
  else 0 then ;

\ a reading older than 5 s is not shown as current
: p-fresh?  ( -- flag )  p-age 5000000 <  s-hr @ 0 > and ;

\ 7x6 heart beside the unit
: p-heart  ( x y -- )
  2dup swap 1 + swap 2 1 oled-box
  2dup swap 4 + swap 2 1 oled-box
  2dup 1 + 7 2 oled-box
  2dup swap 1 + swap 3 + 5 1 oled-box
  2dup swap 2 + swap 4 + 3 1 oled-box
  swap 3 + swap 5 + 1 1 oled-box ;

\ bpm four times the size of everything else
: p-bpm  ( -- )
  4 oled-scale
  p-fresh? if s-hr @ 0 2 oled-num else s" --" 0 2 oled-str then
  1 oled-scale
  s" bpm" 76 2 oled-str
  p-fresh? if 76 13 p-heart then ;

\ Z and the zone at twice the size, top right
: p-zone  ( -- )
  s-zone @ 0 > p-fresh? and if
    2 oled-scale 90 98 2 oled-glyph s-zone @ 110 2 oled-num 1 oled-scale
    s" of 5" 98 19 oled-str
  then ;

\ five cells: the ones below the zone filled, the zone itself taller
: p-cells  ( -- )
  5 0 do
    i 25 * 2 +
    p-fresh? if
      i s-zone @ 1 - < if 40 22 5 oled-box else
      i s-zone @ 1 - = if 36 22 9 oled-box else 40 22 5 oled-frame then then
    else 40 22 5 oled-frame then
  loop ;

\ elapsed as m:ss, or h:mm:ss past an hour
: p-time  ( -- )
  s-secs @ 3600 >= if
    s-secs @ 3600 / 0 47 oled-num  58 6 47 oled-glyph
    s-secs @ 3600 mod 60 / 12 47 oled-num02  58 24 47 oled-glyph
    s-secs @ 60 mod 30 47 oled-num02
  else
    s-secs @ 60 / 0 47 oled-num02  58 12 47 oled-glyph
    s-secs @ 60 mod 18 47 oled-num02
  then ;

\ cadence right aligned: steps a minute on foot, crank rpm riding
: p-cad  ( -- )
  s-cadk @ 0 > s-cad @ 0 > and if
    s-cad @ 104 47 oled-numr
    s-cadk @ 1 = if s" spm" else s" rpm" then 110 47 oled-str
  then ;

: pulse-draw  ( -- )  p-bpm p-zone p-cells p-time p-cad ;
