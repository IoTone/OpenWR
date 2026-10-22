\ 30-anim.fs — ambient animation, 5 frames a second, driven by the weather
\ that is actually happening and the time of day. Load after 20-hud.
\
\ The firmware calls hud-anim every 200 ms when it exists. Each call draws
\ the full theme frame, then a particle layer on top, then shows. Particles
\ are few (at most 10) because every lit pixel is in front of an eye.
\
\   rain / drizzle / showers   drops fall, leaning with the wind
\   snow                       flakes drift down and sideways
\   thunderstorm               rain plus the occasional lightning flash
\   fog                        three dotted bands shimmer
\   clear night                stars twinkle
\   windy (20 km/h+), else     streaks blow across
\
\ fx-code overrides the live weather code for demos: 61 fx-code ! (rain),
\ 75 (snow), 95 (storm), 45 (fog), -1 to follow the forecast again.
\ anim-on toggles the whole layer: 0 anim-on !

variable anim-on   -1 anim-on !
variable fx-code   -1 fx-code !

10 constant np
variable px  np 4 * allot
variable py  np 4 * allot
: px@  ( i -- x )  4 * px + @ ;
: px!  ( x i -- )  4 * px + ! ;
: py@  ( i -- y )  4 * py + @ ;
: py!  ( y i -- )  4 * py + ! ;
: p-scatter  ( -- )  np 0 do 128 random i px! 56 random i py! loop ;
p-scatter

\ --- what is the weather right now ----------------------------------
: cur  ( -- code )  fx-code @ dup 0< if drop 0 wx-code then ;
: between  ( n lo hi -- flag )  2 pick >= swap 3 pick <= and nip ;
: rainy?   ( -- flag )  cur 51 67 between cur 80 82 between or ;
: snowy?   ( -- flag )  cur 71 77 between cur 85 86 between or ;
: stormy?  ( -- flag )  cur 95 >= ;
: foggy?   ( -- flag )  cur 45 = cur 48 = or ;
: clear?   ( -- flag )  cur 1 <= ;
: night?   ( -- flag )  0 wx-day? 0= ;
: windy?   ( -- flag )  0 wx-wind 20 >= ;

\ how many particles: more rain when it is more likely
: n-active  ( -- n )  0 wx-pop 12 / 2 + np min ;

\ --- effects ----------------------------------------------------------
\ drops fall 3 px a frame and lean with the wind; re-enter at the top
: fx-rain  ( -- )
  n-active 0 do
    i px@ i py@
    2dup 2 oled-vline
    3 + dup 55 > if 2drop 128 random 0 else swap 0 wx-wind 10 / - 128 + 128 mod swap then
    i py! i px!
  loop ;

\ flakes drift 1 px down and wander sideways
: fx-snow  ( -- )
  n-active 0 do
    i px@ i py@
    2dup oled-pixel
    1 + dup 55 > if 2drop 128 random 0 else swap 3 random 1 - + 128 + 128 mod swap then
    i py! i px!
  loop ;

\ one frame in twenty, the whole panel inverts: lightning
: fx-flash  ( -- )  20 random 0= if 1 oled-invert else 0 oled-invert then ;

\ three dotted bands whose phase slides each frame
: fx-fog  ( -- )
  3 0 do
    128 frame @ i + 4 mod do i 14 j 16 * + oled-pixel 4 +loop
  loop ;

\ stars: fixed positions, each lit three frames in four
: fx-stars  ( -- )
  8 0 do 4 random if i px@ i py@ oled-pixel then loop ;

\ streaks blow right to left, faster in more wind
: fx-wind  ( -- )
  6 0 do
    i px@ i py@
    2dup 3 oled-hline
    swap 0 wx-wind 8 / 1 + - dup 0< if 2drop 127 56 random else swap then
    i py! i px!
  loop ;

: fx  ( -- )
  stormy? if fx-rain fx-flash else
  rainy?  if fx-rain else
  snowy?  if fx-snow else
  foggy?  if fx-fog else
  clear? night? and if fx-stars else
  windy?  if fx-wind then then then then then then ;

\ keep the panel upright when a storm passes
: fx-settle  ( -- )  stormy? 0= if 0 oled-invert then ;

\ --- the hook ---------------------------------------------------------
: hud-anim  ( -- )
  anim-on @ hud-fresh? pulse-on? @ or and if
    frame @ 1 + frame !
    hud-frame pulse-on? @ if 0 oled-invert else fx fx-settle then oled-show
  then ;
