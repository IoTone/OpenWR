#!/usr/bin/env python3
"""ui_tap.py DEVICE cmd...  — drive the app from adb by Flutter semantics labels (content-desc).

    python3 tools/ui_tap.py SX3LHMB3A1707477 tap "Session / Tab 2" 2 tap "demo session" dump

cmds: tap SUBSTRING [SECONDS]  |  dump  |  back  |  sleep SECONDS"""
import subprocess, sys, re, time
A='/Users/dkords/Library/Android/sdk/platform-tools/adb'; D=sys.argv[1]
def sh(*a): return subprocess.run([A,'-s',D,'shell',*a],capture_output=True,text=True).stdout
def dump():
    sh('uiautomator','dump','/sdcard/ui.xml'); x=sh('cat','/sdcard/ui.xml')
    out=[]
    for m in re.finditer(r'content-desc="([^"]*)"[^>]*bounds="\[(\d+),(\d+)\]\[(\d+),(\d+)\]"',x):
        if m.group(1): out.append((m.group(1).replace('&#10;',' / '),tuple(map(int,m.groups()[1:]))))
    return out
def find(sub):
    items=dump()
    for t,b in items:
        if t.lower()==sub.lower(): return t,b
    for t,b in items:
        if sub.lower() in t.lower(): return t,b
    return None
def tap(sub, wait=2):
    f=find(sub)
    if not f: print('NOT FOUND:',sub); return False
    t,(x0,y0,x1,y1)=f; sh('input','tap',str((x0+x1)//2),str((y0+y1)//2)); print('tap',repr(t)); time.sleep(wait); return True
cmd=sys.argv[2:]
i=0
while i<len(cmd):
    c=cmd[i]
    if c=='tap': tap(cmd[i+1], float(cmd[i+2]) if i+2<len(cmd) and re.match(r'^[\d.]+$',cmd[i+2]) else 2); i+= 3 if i+2<len(cmd) and re.match(r'^[\d.]+$',cmd[i+2]) else 2
    elif c=='dump':
        for t,b in dump(): print(' ',t[:70]); i+=1
    elif c=='back': sh('input','keyevent','4'); time.sleep(1.5); i+=1
    elif c=='sleep': time.sleep(float(cmd[i+1])); i+=2
    else: print('?',c); i+=1
