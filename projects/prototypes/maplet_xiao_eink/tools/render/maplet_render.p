;;; maplet_render.p — e-paper map renderer for the OpenWR maplet, in Pop-11.
;;;
;;; Inputs (written by Python):
;;;   grid.txt   first line "w h", then h lines of w integers: elevation in
;;;              decimetres, already resampled to the canvas size
;;;   track.txt  one "x y" per line, canvas pixel coordinates of the track
;;; Output: a PBM (P1) bitmap, 1 = black.
;;;
;;; Layers are procedures over a global canvas so a plan is just a recipe.

vars gw = 0, gh = 0;          ;;; grid / canvas size
vars grid;                    ;;; intvec, elevation dm, index y*gw + x + 1
vars canvas;                  ;;; intvec of 0/1
vars shade;                   ;;; vector of reals 0..1 (hillshade, 1 = lit)

;;; ---------- grid ----------
define load_grid(path);
    lvars dev = sysopen(path, 0, "line"), rep = line_repeater(dev, inits(8192));
    lvars ln = rep(), items = sysparse_string(ln);
    items(1) -> gw; items(2) -> gh;
    initintvec(gw * gh) -> grid;
    lvars y, x;
    for y from 0 to gh - 1 do
        rep() -> ln; sysparse_string(ln) -> items;
        for x from 0 to gw - 1 do items(x + 1) -> grid(y * gw + x + 1) endfor;
    endfor;
    sysclose(dev);
    initintvec(gw * gh) -> canvas;
    initv(gw * gh) -> shade;
enddefine;

define g(x, y);
    if x < 0 then 0 -> x endif; if y < 0 then 0 -> y endif;
    if x >= gw then gw - 1 -> x endif; if y >= gh then gh - 1 -> y endif;
    grid(y * gw + x + 1)
enddefine;

;;; ---------- canvas ----------
define clear();
    lvars i;
    for i from 1 to gw * gh do 0 -> canvas(i) endfor;
enddefine;

define px(x, y, v);
    if x >= 0 and y >= 0 and x < gw and y < gh then v -> canvas(y * gw + x + 1) endif;
enddefine;

define seg_line(x0, y0, x1, y1);
    lvars dx = abs(x1 - x0), dy = -abs(y1 - y0), sx = if x0 < x1 then 1 else -1 endif,
          sy = if y0 < y1 then 1 else -1 endif, err = dx + dy, e2;
    repeat
        px(x0, y0, 1);
        quitif(x0 == x1 and y0 == y1);
        2 * err -> e2;
        if e2 >= dy then err + dy -> err; x0 + sx -> x0 endif;
        if e2 <= dx then err + dx -> err; y0 + sy -> y0 endif;
    endrepeat;
enddefine;

define thick(x0, y0, x1, y1, r);
    lvars i, j;
    for i from -r to r do
        for j from -r to r do
            if i * i + j * j <= r * r then seg_line(x0 + i, y0 + j, x1 + i, y1 + j) endif;
        endfor;
    endfor;
enddefine;

define disc(cx, cy, r, v);
    lvars i, j;
    for i from -r to r do for j from -r to r do
        if i * i + j * j <= r * r then px(cx + i, cy + j, v) endif;
    endfor endfor;
enddefine;

define ring(cx, cy, r);
    lvars i, j, d;
    for i from -r - 1 to r + 1 do for j from -r - 1 to r + 1 do
        i * i + j * j -> d;
        if d <= r * r and d > (r - 2) * (r - 2) then px(cx + i, cy + j, 1) endif;
    endfor endfor;
enddefine;

define frame(x, y, w, h);
    lvars i;
    for i from 0 to w - 1 do px(x + i, y, 1); px(x + i, y + h - 1, 1) endfor;
    for i from 0 to h - 1 do px(x, y + i, 1); px(x + w - 1, y + i, 1) endfor;
enddefine;

define rect(x, y, w, h, v);
    lvars i, j;
    for j from 0 to h - 1 do for i from 0 to w - 1 do px(x + i, y + j, v) endfor endfor;
enddefine;

;;; ---------- hillshade (light from the north-west, 45 degrees up) ----------
define hillshade(zscale);
    ;;; zscale: grid units of height per pixel of ground, to exaggerate relief
    lvars x, y, dzdx, dzdy, nx, ny, nz, nl, lx = -0.5, ly = -0.5, lz = 0.7071, s;
    for y from 0 to gh - 1 do
        for x from 0 to gw - 1 do
            (g(x + 1, y) - g(x - 1, y)) * zscale / 2.0 -> dzdx;
            (g(x, y + 1) - g(x, y - 1)) * zscale / 2.0 -> dzdy;
            -dzdx -> nx; -dzdy -> ny; 1.0 -> nz;
            sqrt(nx * nx + ny * ny + nz * nz) -> nl;
            (nx * lx + ny * ly + nz * lz) / nl -> s;      ;;; -1..1
            (s + 1.0) / 2.0 -> s;                          ;;; 0..1
            if s < 0.0 then 0.0 -> s endif; if s > 1.0 then 1.0 -> s endif;
            s -> shade(y * gw + x + 1);
        endfor;
    endfor;
enddefine;

;;; Floyd-Steinberg: ink where the hillshade is dark. strength 0..1 scales
;;; how much ink the shading may lay down (0.35 keeps the map readable).
define dither(strength);
    lvars x, y, i, v, out, err, buf = initv(gw * gh);
    for i from 1 to gw * gh do (1.0 - shade(i)) * strength -> buf(i) endfor;
    for y from 0 to gh - 1 do
        for x from 0 to gw - 1 do
            y * gw + x + 1 -> i;
            buf(i) -> v;
            if v >= 0.5 then 1 else 0 endif -> out;
            if out == 1 and canvas(i) == 0 then 1 -> canvas(i) endif;
            v - out -> err;
            if x + 1 < gw then buf(i + 1) + err * 7 / 16 -> buf(i + 1) endif;
            if y + 1 < gh then
                if x > 0 then buf(i + gw - 1) + err * 3 / 16 -> buf(i + gw - 1) endif;
                buf(i + gw) + err * 5 / 16 -> buf(i + gw);
                if x + 1 < gw then buf(i + gw + 1) + err * 1 / 16 -> buf(i + gw + 1) endif;
            endif;
        endfor;
    endfor;
enddefine;

;;; ---------- contours by marching squares ----------
;;; interval in dm; every `major`th line is drawn 2 px thick
define contours(interval, major);
    lvars x, y, lvl, lo, hi, a, b, c, d, code, lvlmin, lvlmax, k, idx;
    lvars gmin = grid(1), gmax = grid(1), i;
    for i from 2 to gw * gh do
        if grid(i) < gmin then grid(i) -> gmin endif;
        if grid(i) > gmax then grid(i) -> gmax endif;
    endfor;
    (gmin div interval) * interval -> lvlmin; ((gmax div interval) + 1) * interval -> lvlmax;
    define lerp(p, q, v);
        ;;; fraction along an edge where the level v crosses p..q
        if q == p then 0.5 else (v - p) / (q - p) endif
    enddefine;
    define seg(x, y, a, b, c, d, lvl, w);
        ;;; corners a=(x,y) b=(x+1,y) c=(x+1,y+1) d=(x,y+1)
        lvars code = 0, pts = [], ex, ey;
        if a >= lvl then code + 8 -> code endif; if b >= lvl then code + 4 -> code endif;
        if c >= lvl then code + 2 -> code endif; if d >= lvl then code + 1 -> code endif;
        if code == 0 or code == 15 then return endif;
        ;;; crossing points on the four edges (top, right, bottom, left)
        lvars t = [^(x + lerp(a, b, lvl)) ^y],
              r = [^(x + 1) ^(y + lerp(b, c, lvl))],
              bo = [^(x + lerp(d, c, lvl)) ^(y + 1)],
              l = [^x ^(y + lerp(a, d, lvl))];
        define draw(p, q, w);
            lvars x0 = round(p(1)), y0 = round(p(2)), x1 = round(q(1)), y1 = round(q(2));
            if w > 1 then thick(x0, y0, x1, y1, 1) else seg_line(x0, y0, x1, y1) endif;
        enddefine;
        if code == 1 or code == 14 then draw(l, bo, w)
        elseif code == 2 or code == 13 then draw(bo, r, w)
        elseif code == 3 or code == 12 then draw(l, r, w)
        elseif code == 4 or code == 11 then draw(t, r, w)
        elseif code == 5 then draw(t, l, w); draw(r, bo, w)
        elseif code == 6 or code == 9 then draw(t, bo, w)
        elseif code == 7 or code == 8 then draw(t, l, w)
        elseif code == 10 then draw(t, r, w); draw(l, bo, w)
        endif;
    enddefine;
    lvars n = 0;
    for lvl from lvlmin by interval to lvlmax do
        n + 1 -> n;
        if major > 0 and (lvl div interval) mod major == 0 then 2 else 1 endif -> k;
        for y from 0 to gh - 2 do
            for x from 0 to gw - 2 do
                seg(x, y, g(x, y), g(x + 1, y), g(x + 1, y + 1), g(x, y + 1), lvl, k);
            endfor;
        endfor;
    endfor;
enddefine;

;;; ---------- track ----------
vars track = [];
define load_track(path);
    lvars dev = sysopen(path, 0, "line"), rep = line_repeater(dev, inits(256)), ln, items;
    [] -> track;
    repeat
        rep() -> ln; quitif(ln == termin);
        sysparse_string(ln) -> items;
        if length(items) >= 2 then [^(items(1)) ^(items(2))] :: track -> track endif;
    endrepeat;
    sysclose(dev);
    rev(track) -> track;
enddefine;

;;; a halo first (white, radius r+2) so the line reads over the shading
define draw_track(r);
    lvars p, q, rest = track;
    while rest /== [] and tl(rest) /== [] do
        hd(rest) -> p; hd(tl(rest)) -> q;
        lvars i, j, x0 = p(1), y0 = p(2), x1 = q(1), y1 = q(2);
        for i from -(r + 2) to r + 2 do for j from -(r + 2) to r + 2 do
            if i * i + j * j <= (r + 2) * (r + 2) then
                ;;; erase along the segment
                lvars dx = abs(x1 - x0), dy = -abs(y1 - y0), sx = if x0 < x1 then 1 else -1 endif,
                      sy = if y0 < y1 then 1 else -1 endif, err = dx + dy, e2, ax = x0, ay = y0;
                repeat
                    px(ax + i, ay + j, 0);
                    quitif(ax == x1 and ay == y1);
                    2 * err -> e2;
                    if e2 >= dy then err + dy -> err; ax + sx -> ax endif;
                    if e2 <= dx then err + dx -> err; ay + sy -> ay endif;
                endrepeat;
            endif;
        endfor endfor;
        tl(rest) -> rest;
    endwhile;
    track -> rest;
    while rest /== [] and tl(rest) /== [] do
        hd(rest) -> p; hd(tl(rest)) -> q;
        thick(p(1), p(2), q(1), q(2), r);
        tl(rest) -> rest;
    endwhile;
    if track /== [] then
        lvars s = hd(track), e = last(track);
        disc(s(1), s(2), r + 3, 0); ring(s(1), s(2), r + 3);          ;;; start: open ring
        disc(e(1), e(2), r + 3, 0); disc(e(1), e(2), r + 1, 1);       ;;; end: filled dot
    endif;
enddefine;

;;; dashed route for a plan (not yet walked)
define draw_route(r, dash);
    lvars p, q, rest = track, n = 0;
    while rest /== [] and tl(rest) /== [] do
        hd(rest) -> p; hd(tl(rest)) -> q;
        n + 1 -> n;
        if n mod (2 * dash) < dash then thick(p(1), p(2), q(1), q(2), r) endif;
        tl(rest) -> rest;
    endwhile;
enddefine;

;;; ---------- furniture ----------
define scalebar(x, y, pixels);
    lvars i;
    rect(x - 2, y - 4, pixels + 4, 9, 0);
    for i from 0 to pixels do px(x + i, y, 1); px(x + i, y + 1, 1) endfor;
    for i from -3 to 3 do px(x, y + i, 1); px(x + pixels, y + i, 1) endfor;
enddefine;

define north(cx, cy);
    rect(cx - 5, cy - 7, 11, 15, 0);
    seg_line(cx, cy + 6, cx, cy - 6); seg_line(cx, cy - 6, cx - 3, cy - 2); seg_line(cx, cy - 6, cx + 3, cy - 2);
enddefine;

define gridlines(stp);
    lvars x, y;
    for x from stp by stp to gw - 1 do for y from 0 by 3 to gh - 1 do px(x, y, 1) endfor endfor;
    for y from stp by stp to gh - 1 do for x from 0 by 3 to gw - 1 do px(x, y, 1) endfor endfor;
enddefine;

;;; ---------- output ----------
define save_pbm(path);
    lvars o = discout(path), x, y;
    appdata('P1', o); o(10);
    appdata(gw sys_>< ' ' sys_>< gh, o); o(10);
    for y from 0 to gh - 1 do
        for x from 0 to gw - 1 do
            if canvas(y * gw + x + 1) == 1 then o(`1`) else o(`0`) endif;
        endfor;
        o(10);
    endfor;
    o(termin);
enddefine;

npr('maplet_render.p loaded');
