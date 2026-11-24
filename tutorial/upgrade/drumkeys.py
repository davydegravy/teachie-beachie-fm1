# The drum track's keys on the tracks page and on the drum grid: does a key sound, write a box, pick the drum?
#   cd <a copy of the project> && python3 trk.py OUTDIR
import os, sys, time, wave, struct
sys.path.insert(0, "tutorial")
from tp.fm1 import FM1, Twin, BTN
from tp.teach import Teacher
from tp.lesson import Player
out = sys.argv[1]; os.makedirs(out, exist_ok=True)
link = Twin(out=out); f = FM1(link); p = Player(Teacher(f))
def loud(path):
    w = wave.open(path); n = w.getnframes(); raw = w.readframes(n); w.close()
    s = struct.unpack("<%dh" % (len(raw) // 2), raw); return max((abs(x) for x in s), default=0)
def press(tag, key, page):
    p.setup("drums empty; page %s" % page); time.sleep(0.4); f.guide(4)
    before = len(f.pattern(3)[1]); link.wav("snd"); time.sleep(0.15)
    link.key(key, 1); time.sleep(0.12); link.key(key, 0); time.sleep(0.7)
    link.wav(); peak = loud(os.path.join(out, "snd.wav")); f.guide(4)
    after = f.pattern(3)[1]
    p.setup("page STEPS"); time.sleep(0.5); link.shot("%s-grid" % tag)   # the grid names the drum picked, top right
    print("%-30s on %-6s | sound peak %5d | boxes written: %s" % (tag, page, peak, sorted({(int(x[0]) + 1, x[2]) for x in after}) if len(after) != before else "none"), flush=True)
try:
    time.sleep(1.0)
    p.setup("song HOUSE ONE section C, 1 bar; mute bass lead chords pad; track drums; page TRACKS"); time.sleep(0.5)
    press("A white key 3 (snare)", 4, "TRACKS")
    press("B white key 5 (hat)", 7, "STEPS")
    press("C black key 2 (snare pair)", 3, "TRACKS")
    press("D black key 5 (hat pair)", 10, "STEPS")
finally:
    link.close()
