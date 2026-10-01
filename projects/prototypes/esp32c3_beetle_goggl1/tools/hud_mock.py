"""
hud_mock.py — render the three weather HUD themes on the host, using the
same ssd1309.c the device runs, against a fixed forecast fixture.

    python3 tools/hud_mock.py docs/images

Builds a shared library from src/ssd1309.c with stub ESP-IDF headers (kept
under tools/stubs/), then draws each theme through ctypes and writes
theme-<name>.png and theme-<name>.txt. This is the layout regression test
for docs/DESIGN-weather-hud.md section 7: if it fits here, it fits on the
glass. Requires Pillow.
"""
import ctypes, sys, os, subprocess
from PIL import Image

HERE = os.path.dirname(os.path.abspath(__file__))
SRC  = os.path.join(HERE, '..', 'src')
STUB = os.path.join(HERE, 'stubs')
LIB  = os.path.join(HERE, 'libssd1309.so')
if not os.path.exists(LIB) or os.path.getmtime(LIB) < os.path.getmtime(os.path.join(SRC, 'ssd1309.c')):
    subprocess.check_call(['cc', '-std=gnu11', '-shared', '-fPIC', '-I' + STUB, '-I' + SRC,
                           '-o', LIB, os.path.join(SRC, 'ssd1309.c')])
L = ctypes.CDLL(LIB)
class Cfg(ctypes.Structure):
    _fields_ = [('spi_host',ctypes.c_int)] + [(n,ctypes.c_int) for n in 'pin_sclk pin_mosi pin_dc pin_rst pin_cs clock_hz x_offset'.split()]
L.oled_init(ctypes.byref(Cfg()))
clear=L.oled_clear; show=L.oled_show; color=L.oled_set_color
pixel=L.oled_pixel; line=L.oled_line; hline=L.oled_hline; vline=L.oled_vline
frame=L.oled_frame; box=L.oled_box; rframe=L.oled_rframe; rbox=L.oled_rbox; circle=L.oled_circle; disc=L.oled_disc
scale=L.oled_set_scale; scalexy=L.oled_set_scale_xy; ddir=L.oled_set_dir
def text(x,y,s): L.oled_str(x,y,s.encode(),len(s))
def xbm(x,y,w,h,rows,transparent=True):
    # rows: list of strings of '#'/'.' ; pack LSB-left
    stride=(w+7)//8; buf=bytearray(stride*h)
    for j,r in enumerate(rows):
        for i,ch in enumerate(r):
            if ch=='#': buf[j*stride+i//8] |= 1<<(i%8)
    b=(ctypes.c_uint8*len(buf))(*buf); L.oled_xbm(x,y,w,h,b,transparent)
def getpix(x,y): return L.oled_get_pixel(x,y)
def dump(title):
    rows=[''.join('#' if getpix(x,y) else '.' for x in range(128)) for y in range(64)]
    lit=sum(r.count('#') for r in rows)
    return rows, lit
def png(path, scale=4):
    im=Image.new('RGB',(128*scale+8,64*scale+8),(12,12,16))
    px=im.load()
    for y in range(64):
        for x in range(128):
            on=getpix(x,y)
            c=(60,220,235) if on else (26,28,34)
            for dy in range(scale):
                for dx in range(scale):
                    if dx<scale-1 and dy<scale-1: px[4+x*scale+dx,4+y*scale+dy]=c
    # mark the opaque status strip (rows 56-63) with a slightly lighter backing
    for y in range(56,64):
        for x in range(128):
            if not getpix(x,y):
                for dy in range(scale-1):
                    for dx in range(scale-1): px[4+x*scale+dx,4+y*scale+dy]=(40,42,48)
    im.save(path)

# ---------------- fixture: an autumn afternoon, rain arriving ----------------
HOUR0=14
T10 =[158,161,163,160,152,141,133,128,124]   # temp *10 C
POP =[5,10,20,55,80,85,70,40,20]              # precip probability %
RAIN=[0,0,0,4,16,22,12,5,1]                   # mm*10
SNOW=[0]*9
WIND=[12,14,15,19,26,31,29,22,18]             # km/h
GUST=[20,22,26,33,45,52,48,38,30]
WDIR=[200,210,220,230,240,250,250,245,240]
CODE=[1,2,3,3,61,63,61,51,2]                  # WMO codes
CITY="Portland"; CLOCK="14:07"; AGE_MIN=6; NET=True

ICON8={
 'sun':   ["...#....","#..#..#.",".#...#..","..###...","#.#.#.#.","..###...",".#...#..","#..#..#."],
 'part':  ["..#.....","#.#..#..",".###....","#.##.###","..#.#..#","..#....#","..######","........"],
 'cloud': ["........","..###...",".#...#..","#.....#.","#......#","#......#",".######.","........"],
 'rain':  ["..###...",".#...#..","#......#",".######.","........",".#..#..#","#..#..#.","........"],
 'drizz': ["..###...",".#...#..","#......#",".######.","........","..#..#..","........",".#..#..."],
 'snow':  ["..###...",".#...#..","#......#",".######.","........","#.#.#.#.",".#.#.#.#","#.#.#.#."],
 'storm': ["..###...",".#...#..","#......#",".######.","...##...","..##....","...##...","..#....."],
 'fog':   ["........","######..","........","..######","........","######..","........","..######"],
}
def icon_for(code):
    if code==0: return 'sun'
    if code==1: return 'sun'
    if code==2: return 'part'
    if code==3: return 'cloud'
    if code in (45,48): return 'fog'
    if 51<=code<=57: return 'drizz'
    if 61<=code<=67 or 80<=code<=82: return 'rain'
    if 71<=code<=77 or 85<=code<=86: return 'snow'
    if code>=95: return 'storm'
    return 'cloud'
def icon(x,y,code): xbm(x,y,8,8,ICON8[icon_for(code)])
ICON16={ # 16x16 for Glance; derived by doubling 8x8
}
def icon16(x,y,code):
    rows=ICON8[icon_for(code)]
    big=[]
    for r in rows:
        rr=''.join(c*2 for c in r); big+= [rr,rr]
    xbm(x,y,16,16,big)
def r10(t): return (t+5)//10 if t>=0 else -((-t+5)//10)
def degree(x,y): pixel(x,y);pixel(x+1,y);pixel(x,y+1);pixel(x+1,y+1)   # 2x2 degree mark

def status_strip():
    # rows 56-63 sit over the opaque backing: shared across themes
    hline(0,56,128)
    text(0,57,CITY[:10])
    text(128-30,57,CLOCK)
    # freshness: filled dot = fresh (<30 min), hollow = stale, 'x' = no net
    cx=128-30-8
    if not NET: text(cx-2,57,"x")
    elif AGE_MIN<30: disc(cx,60,2)
    else: circle(cx,60,2)

def t_strip():
    clear(); color(1); scale(1); ddir(0)
    # temperature line, x columns 14 px apart from x=14
    tmin=min(T10); tmax=max(T10); 
    def ty(t):  # map to rows 1..25
        return 25 - (t-tmin)*24//max(1,(tmax-tmin))
    xs=[12+13*i for i in range(9)]
    for i in range(8): line(xs[i],ty(T10[i]),xs[i+1],ty(T10[i+1]))
    for i in range(9): pixel(xs[i],ty(T10[i])-1); pixel(xs[i],ty(T10[i])+1)
    text(0,0,"%d"%r10(tmax)); text(0,19,"%d"%r10(tmin))
    # precip bars rows 28..39 (height 0..12 by probability)
    text(0,30,"%")
    for i in range(9):
        h=POP[i]*12//100
        if h>0: box(xs[i]-2,40-h,5,h)
    hline(10,40,118)
    # wind km/h every 2 h
    text(0,42,"w")
    for k in range(5):
        i=2*k; text(xs[i]-5,42,"%d"%WIND[i])
    # hour labels every 2 h; now marker
    for k in range(5):
        i=2*k; text(xs[i]-5,49,"%02d"%((HOUR0+i)%24))
    for y in range(0,55,2): pixel(xs[0],y)   # dotted "now" line
    status_strip(); return "strip"

def t_glance():
    clear(); color(1); ddir(0)
    scale(3); text(0,1,"%d"%r10(T10[0])); scale(1); degree(31,2)
    icon16(40,4,CODE[0])
    # right column: what matters next
    first_rain=next((i for i in range(9) if POP[i]>=50),None)
    if first_rain is None: text(62,1,"dry 8h")
    else: text(62,1,"rain %02d:00"%((HOUR0+first_rain)%24)); text(62,10,"%d%% %d.%dmm"%(max(POP),max(RAIN)//10,max(RAIN)%10))
    text(62,19,"wind %d g%d"%(max(WIND),max(GUST)))
    text(62,28,"lo %d hi %d"%(r10(min(T10)),r10(max(T10))))
    # sparkline rows 38..48, hours 0..8 at 16 px
    tmin=min(T10); tmax=max(T10)
    def ty(t): return 48-(t-tmin)*10//max(1,tmax-tmin)
    for i in range(8): line(i*16,ty(T10[i]),(i+1)*16,ty(T10[i+1]))
    # rain band rows 51..53: solid where pop>=50, dotted where >=20
    for i in range(9):
        x0=i*16
        if POP[i]>=50: hline(x0,52,16); hline(x0,53,16)
        elif POP[i]>=20:
            for x in range(x0,x0+16,2): pixel(x,52)
    status_strip(); return "glance"

def t_ribbon():
    clear(); color(1); scale(1); ddir(0)
    for i in range(9):
        x=i*14+1
        text(x,0,"%02d"%((HOUR0+i)%24))
        icon(x+1,9,CODE[i])
        text(x,19,"%d"%r10(T10[i]))
        if POP[i]>=20: text(x,28,"%d"%POP[i])
        else: pixel(x+3,31); pixel(x+7,31)
        text(x,37,"%d"%WIND[i])
        # wind direction tick: 3 px from centre toward the direction wind blows TO
        import math
        a=math.radians(WDIR[i]+180); cx=x+5; cy=49
        line(cx,cy,int(round(cx+3*math.sin(a))),int(round(cy-3*math.cos(a))))
        pixel(cx,cy)
    # now column highlighted: underline the hour label
    hline(0,7,13)
    status_strip(); return "ribbon"

out=sys.argv[1]
for fn in (t_strip,t_glance,t_ribbon):
    name=fn(); rows,lit=dump(name)
    open(f"{out}/theme-{name}.txt","w").write("\n".join(rows)+"\n")
    png(f"{out}/theme-{name}.png")
    print(f"== {name}: {lit} lit px of 8192 ({lit*100//8192}%), HUD area {sum(r.count('#') for r in rows[:56])} of 7168")
    print("\n".join(rows))
