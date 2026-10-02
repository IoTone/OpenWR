import json
from PIL import Image, ImageDraw, ImageOps
W,H=212,104
F=json.load(open('font5x7.json'))
def pbm(p): return Image.open(p).convert('L')
def glyph(im,x,y,c,scale=1,ink=0):
    g=F[ord(c)-32] if 32<=ord(c)<=126 else F[0]; px=im.load()
    for col in range(5):
        for row in range(7):
            if g[col]>>row&1:
                for sx in range(scale):
                    for sy in range(scale):
                        X=x+col*scale+sx; Y=y+row*scale+sy
                        if 0<=X<im.width and 0<=Y<im.height: px[X,Y]=ink
def text(im,x,y,s,scale=1,inv=False,pad=2,band=None):
    w=len(s)*6*scale-scale; h=7*scale
    d=ImageDraw.Draw(im)
    if band=='full': d.rectangle((0,y-pad,im.width-1,y+h+pad-1),fill=0 if inv else 255)
    else: d.rectangle((x-pad,y-pad,x+w+pad-1,y+h+pad-1),fill=0 if inv else 255)
    for i,c in enumerate(s): glyph(im,x+i*6*scale,y,c,scale,ink=255 if inv else 0)
    return w
def bw(im): return im.point(lambda v:255 if v>128 else 0).convert('1')
# ---- Plan 1 Postcard (after a session) ----
pc=pbm('postcard.pbm')
text(pc,6,5,'FOREST PARK LOOP',2,inv=True,pad=3)
text(pc,6,26,'RUN 1:02:14  8.9km  +412m',1,pad=2)
text(pc,49,H-13,'500m',1,pad=1)
bw(pc).save('plan1_postcard.png')
# ---- Plan 2 Field Notes (before a session) ----
fn=Image.new('L',(W,H),255)
fn.paste(pbm('fieldnotes.pbm'),(72,0))
d=ImageDraw.Draw(fn)
d.line((71,0,71,H),fill=0)
text(fn,4,4,'PLAN',2)
text(fn,4,22,'WILDWOOD',1); text(fn,4,31,'> PITTOCK',1)
d.line((4,42,66,42),fill=0)
for i,(k,v) in enumerate([('DIST','8.9km'),('UP','+412m'),('EST','1h05'),('SUN','18:52')]):
        text(fn,4,47+i*12,k,1); text(fn,36,47+i*12,v,1)
text(fn,114,H-13,"500m",1,pad=1)
# waypoints on the route (field-grid coords)
pts=[tuple(map(int,l.split())) for l in open('track_field.txt').read().splitlines()]
for n,idx in enumerate([0,40,80,120,160],1):
    x,y=pts[idx]; x+=72
    d.rectangle((x-5,y-5,x+5,y+5),fill=255,outline=0); glyph(fn,x-2,y-3,str(n))
bw(fn).save('plan2_fieldnotes.png')
# ---- Plan 3 Snapshot (S3 Sense camera + inset) ----
ph=Image.open('balch.jpg').convert('L')
ph=ImageOps.fit(ph,(W,H),Image.LANCZOS,centering=(0.5,0.35))
ph=ImageOps.autocontrast(ph,cutoff=2)
ph=ph.point(lambda v:int(255*((v/255)**0.55)))
B=[[0,8,2,10],[12,4,14,6],[3,11,1,9],[15,7,13,5]]
pxl=ph.load()
for yy in range(H):
    for xx in range(W): pxl[xx,yy]=255 if pxl[xx,yy]>(B[yy%4][xx%4]+0.5)*16 else 0
ins=pbm('inset.pbm'); fr=Image.new('L',(ins.width+6,ins.height+6),255)
fr.paste(ins,(3,3)); ImageDraw.Draw(fr).rectangle((0,0,fr.width-1,fr.height-1),outline=0)
ph.paste(fr,(W-fr.width-3,3))
text(ph,0,H-10,' OCT 1 07:42  BALCH CREEK  4.1km',1,inv=True,pad=2,band='full')
bw(ph).save('plan3_snapshot.png')
# review sheet
sheet=Image.new('L',(W*3+40,(H*3+30)*3+10),255)
for i,p in enumerate(['plan1_postcard.png','plan2_fieldnotes.png','plan3_snapshot.png']):
    sheet.paste(Image.open(p).convert('L').resize((W*3,H*3),Image.NEAREST),(20,20+i*(H*3+30)))
sheet.save('sheet.png'); print('composed')
