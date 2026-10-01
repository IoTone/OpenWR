\ 20-hud.fs — the use case entry point. Load last.
\ The firmware calls hud-tick every 60 s, after every fetch, and on the
\ button. Everything on the glass goes through here.

\ what to show while there is nothing to show
: hud-wait  ( -- )
  2 oled-scale s" goggl1" 8 6 oled-str 1 oled-scale
  net-ok? if s" fetching forecast" 8 28 oled-str
  else s" no wifi" 8 28 oled-str s" set with wifi! at ok>" 8 40 oled-str then ;

: hud-draw  ( -- )
  sess-on? @ if session-draw else
  theme@ dup 0 = if drop strip else dup 1 = if drop glance else drop ribbon then then then ;

\ a forecast older than 3 h is not shown as current
: hud-fresh?  ( -- flag )  wx-ok? wx-age 10800 < and ;

: hud-frame  ( -- )
  oled-clear 1 oled-color 0 oled-dir 1 oled-scale
  hud-fresh? if hud-draw else hud-wait then
  hud-status ;

\ dim the glass at night: the panel is in front of an eye
: hud-dim  ( -- )  wx-ok? if 0 wx-day? if 255 else 48 then oled-contrast then ;

: hud-tick  ( -- )  hud-frame oled-show hud-dim ;

hud-tick
