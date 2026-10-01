\ cube.fs — port of DFRobot's Cube.ino (a rotating tetrahedron) for the
\ DFR0934.
\
\ The original keeps four 3-D vertices in doubles and rotates them 0.1°
\ about Z every frame, projecting with
\     u = x + 64 - z*0.35
\     v = y + 26 - z*0.35
\
\ Here everything is integer. The base vertices are fixed and the angle
\ accumulates, so there is no drift from repeated rounding. sin1k/cos1k
\ return sin/cos * 1000 for integer degrees; 0.35 becomes 35/100.
\
\ Vertices (x y z): (0 20 -20) (-20 -20 -20) (20 -20 -20) (0 0 20)

variable angle          \ degrees, 0..359
variable cs             \ cos * 1000 for this frame
variable sn             \ sin * 1000 for this frame

\ Rotate (x y) about Z by the current angle; z is untouched.
\ (The engine has no -rot, so it is spelled rot rot.)
: rotz  ( x y -- x' y' )
  2dup sn @ * swap cs @ * swap - 1000 /   \ x y x'      x' = x cos - y sin
  rot rot                                 \ x' x y
  sn @ * swap cs @ * + 1000 / ;           \ x' y'      y' = x sin + y cos

\ Project (x y z) to screen (u v).
: proj  ( x y z -- u v )
  35 * 100 / >r          \ x y        r: z*0.35
  r@ - 26 +              \ x v        v = y + 26 - z*0.35
  swap r> - 64 +         \ v u        u = x + 64 - z*0.35
  swap ;                 \ u v

\ Each vertex: rotate, project, leave (u v) on the stack.
: v0  ( -- u v )    0  20 rotz -20 proj ;
: v1  ( -- u v )  -20 -20 rotz -20 proj ;
: v2  ( -- u v )   20 -20 rotz -20 proj ;
: v3  ( -- u v )    0   0 rotz  20 proj ;

: edge  ( u0 v0 u1 v1 -- ) oled-line ;

: frame  ( -- )
  angle @ dup cos1k cs ! sin1k sn !
  oled-clear
  v0 v1 edge  v1 v2 edge  v0 v2 edge
  v0 v3 edge  v1 v3 edge  v2 v3 edge
  oled-show ;

\ The Arduino sketch rotates 0.1° per frame with no delay; at ~2 ms per
\ SPI flush that is one full turn every 7 s or so. Integer degrees need a
\ coarser step, so: 2° per frame and 30 ms between frames (6 s per turn).
: spin  ( n -- )
  0 do frame
    angle @ 2 + 360 mod angle !
    30 ms
  loop ;

\ Until the BOOT button is pressed.
: cube  ( -- ) begin 10 spin button? until ;

180 spin
