#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""(SLOOP 3.4.2 trial, step 2) A lesson's song kept over a change of pattern.

SLOOP 3.4 keeps a saved copy of each track's pattern and puts it back when the FM-1 leaves that pattern (keep[],
fw/main.c). A lesson's setup writes what plays; unless it is also made the saved copy, the song empties the first
time the FM-1 goes to another pattern and back. Checked three ways on the simulated FM-1 (build/twin):

  pc-off   the computer's setup (tp/lesson.py) with its keep (ED_GUIDE 7) left out: the problem, seen
  pc       the computer's setup as it is: the song and the lead's sound come back
  fm1      the FM-1's own course (fw/course.inc, c_setup), lesson 24's first step: the song comes back
  pc-own   a setup saves only the tracks it wrote: after the song, a note of the learner's own on the drums (not
           saved), then "fill empty tracks", which writes other tracks: the note is gone after a change of pattern,
           as SLOOP has it (a setup that saved every track kept it: the first version of step 2)

Each: the setup, then SAVE held + white key 2 (pattern 2), then SAVE held + white key 1 (pattern 1 again), the
drums' and the bass's notes and the lead's sound read before and after. Run from the trial's top folder:
    python3 tutorial/upgrade/keepcheck.py
"""
import os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..")); sys.path.insert(0, os.path.join(HERE, "..", "..", "tools"))
import mkcourse, device
from tp.fm1 import FM1, Twin, BTN
from tp.teach import Teacher
from tp import lesson as L

SETUP = "song HOUSE ONE section A; patterns A B C D; lead sound FAT LEAD; track drums; page TRACKS"

def tap(link, what, which, hold=0.1, after=0.3): device.tap(link, what, which, hold, after)
def to_pattern(link, n):
    """SAVE held, the pattern's white key tapped, SAVE let go: that pattern, at once (stopped)"""
    link.button(BTN["SAVE"], 1); time.sleep(0.25); tap(link, "key", L.WHITE[n - 1]); link.button(BTN["SAVE"], 0); time.sleep(0.6)
def look(f):
    st = f.status(); return dict(pattern=st["tracks"][3]["cur"] + 1, drums=len(f.pattern(3)[1]), bass=len(f.pattern(0)[1]),
                                 lead_sound=f.sound_values(1)[0])
def round_trip(link, f, name):
    a = look(f); to_pattern(link, 2); b = look(f); to_pattern(link, 1); c = look(f)
    same = a["drums"] == c["drums"] > 0 and a["bass"] == c["bass"] > 0 and a["lead_sound"] == c["lead_sound"] == "FAT LEAD"
    print("%-7s before %s\n        pattern 2 %s\n        back %s\n        %s" % (name, a, b, c, "KEPT" if same else "LOST"), flush=True)
    return same

def pc_own():
    link = Twin(out=os.path.join("build", "tw", "keepcheck", "pc-own")); f = FM1(link)
    try:
        time.sleep(3.2); f.guide(0)
        p = L.Player(Teacher(f)); p.lesson = next(l for l in L.lessons() if l.number == 24)
        p.setup(SETUP); time.sleep(0.5)
        steps, notes = f.pattern(3); before = len(notes)
        f.write_pattern(3, steps, [(a, l, n, v, cd) for a, l, n, v, cd, _ in notes] + [(1, 1, 15, 112, 0)]); time.sleep(0.2)   # the learner's own: a BELL on box 2
        own = len(f.pattern(3)[1])
        p.setup("fill empty tracks"); time.sleep(0.4)
        to_pattern(link, 2); to_pattern(link, 1); after = len(f.pattern(3)[1])
        ok = own == before + 1 and after == before
        print("pc-own  the drums: %d notes, %d with the learner's own, %d after \"fill empty tracks\" and a change of pattern\n        %s" % (
            before, own, after, "NOT SAVED BY THE WAY" if ok else "SAVED BY THE WAY" if after == own else "?"), flush=True)
        return ok
    finally: link.close()

def pc(keep=True):
    link = Twin(out=os.path.join("build", "tw", "keepcheck", "pc" if keep else "pc-off")); f = FM1(link)
    try:
        time.sleep(3.2); f.guide(0)                                 # (the start's card away: a computer leads)
        p = L.Player(Teacher(f)); p.lesson = next(l for l in L.lessons() if l.number == 24)   # (its song: HOUSE ONE)
        if not keep: f.keep = lambda tracks: None
        p.setup(SETUP); time.sleep(0.5)
        return round_trip(link, f, "pc" if keep else "pc-off")
    finally: link.close()

def fm1(seed):
    os.environ["TP_HOME"] = "1"
    link = Twin(out=os.path.join("build", "tw", "keepcheck", "fm1"), flash=seed); f = FM1(link)
    try:
        time.sleep(3.2); tap(link, "button", BTN["OCT_UP"])         # START: nobody answers, the FM-1's own course
        end = time.time() + 6
        while f.course()[0] != device.PH["TOC"]:
            if time.time() > end: raise SystemExit("START did not begin the FM-1's own course")
            time.sleep(0.1)
        tap(link, "button", BTN["OCT_DOWN"])                        # LESSONS, SELECT to lesson 24, BEGIN
        for _ in range(23): link.knob(device.KNOB["SELECT"], 1); time.sleep(0.12)
        tap(link, "button", BTN["OCT_UP"])
        end = time.time() + 10
        while True:                                                 # its first step, the setup, done
            ph, num, step, _ = f.course()
            if num == 24 and step >= 1: break
            if time.time() > end: raise SystemExit("lesson 24 did not begin (phase %d, lesson %d, step %d)" % (ph, num, step + 1))
            time.sleep(0.1)
        f.guide(0); time.sleep(0.4)                                 # (a computer leads now: the course gives way, the FM-1 as the setup left it)
        print("fm1     lesson 24's setup: song HOUSE ONE section A; patterns A B C D; song lines ...", flush=True)
        a = dict(drums=len(f.pattern(3)[1]), bass=len(f.pattern(0)[1]))
        to_pattern(link, 2); to_pattern(link, 1)
        c = dict(drums=len(f.pattern(3)[1]), bass=len(f.pattern(0)[1]))
        same = a == c and a["drums"] > 0 and a["bass"] > 0
        print("        before %s\n        back %s\n        %s" % (a, c, "KEPT" if same else "LOST"), flush=True)
        return same
    finally: link.close()

if __name__ == "__main__":
    off = pc(keep=False); on = pc(keep=True); mine = pc_own()
    seed = device.seed_flash(mkcourse.build()[0]); own = fm1(seed)
    ok = (not off) and on and own and mine
    print("keepcheck: %s (without the keep: %s; the computer's setup: %s; the FM-1's own: %s; a learner's own note: %s)" % (
        "OK" if ok else "FAILED", "lost, as expected" if not off else "kept?!", "kept" if on else "LOST", "kept" if own else "LOST",
        "not saved by the way" if mine else "SAVED BY THE WAY"))
    sys.exit(0 if ok else 1)
