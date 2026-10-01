\ 15-session.fs — the session HUD: elapsed time, pace, distance, heart rate,
\ streamed from the phone. Loads after the themes, before 20-hud.
\
\ The phone is the sensor hub, the glass is the display. Switchback sends
\ one line a second while a session runs:
\
\     secs pace10 dist100 hr sess!      ( e.g. 754 52 321 142 sess! )
\
\ pace10 is minutes per unit x10 (5.2 min/mi -> 52), dist100 is distance
\ x100 in the goggle's units, hr is beats per minute or 0. sess-on and
\ sess-off switch the glass between this layout and the weather themes.
\ Units follow units@: 1 miles, 0 km.

variable sess-on?   0 sess-on? !
variable s-secs  variable s-pace  variable s-dist  variable s-hr
variable sx  variable sy          \ origin of the cell being drawn

: sess!  ( secs pace10 dist100 hr -- )
  s-hr ! s-dist ! s-pace ! s-secs ! -1 sess-on? ! ;
: sess-on   ( -- )  -1 sess-on? ! hud-now ;
: sess-off  ( -- )   0 sess-on? ! hud-now ;

: at  ( x y -- )  sy ! sx ! ;
: num  ( n dx -- )    sx @ + sy @ oled-num ;
: num02  ( n dx -- )  sx @ + sy @ oled-num02 ;
: glyph  ( c dx -- )  sx @ + sy @ oled-glyph ;
: str  ( c-addr u dx -- )  sx @ + sy @ oled-str ;

\ elapsed as m:ss, or h:mm:ss past an hour, at scale 2
: sess-time  ( -- )
  2 oled-scale
  s-secs @ 3600 >= if
    s-secs @ 3600 / 0 num  58 12 glyph
    s-secs @ 3600 mod 60 / 24 num02  58 48 glyph
    s-secs @ 60 mod 60 num02
  else
    s-secs @ 60 / 0 num02  58 24 glyph
    s-secs @ 60 mod 36 num02
  then
  1 oled-scale ;

\ pace as m:ss per unit
: sess-pace  ( -- )
  s-pace @ 10 / 0 num  58 6 glyph  s-pace @ 10 mod 6 * 12 num02
  units@ if s" /mi" else s" /km" then 26 str ;

\ distance with two decimals
: sess-dist  ( -- )
  s-dist @ 100 / 0 num  46 12 glyph  s-dist @ 100 mod 18 num02
  units@ if s" mi" else s" km" then 32 str ;

: sess-hr  ( -- )
  s-hr @ 0 > if s-hr @ 0 num  s" bpm" 20 str
  else s" -- bpm" 0 str then ;

\ time large across the top; pace and distance in a row; heart rate and the
\ next weather change at the bottom
: session-draw  ( -- )
  2 3 at sess-time
  0 24 at sess-pace
  66 24 at sess-dist
  0 40 at sess-hr
  first-rain dup 0< if drop else 70 40 at s" rain" 0 str hour 30 num02 then ;
