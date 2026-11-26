#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""(SLOOP 3.4.2 trial, step 2) The drum picked: the lesson player's check of it, and its words for the wrong one.

On 3.4.2's drum grid a black key picks the drum and a white key is a box. A lesson can now wait for the drum picked
(`Until: drums picked SNARE`), and when the drum picked is not the one a step's boxes ask for, that drum's black key
blinks, not the boxes, and the lesson says so once for each wrong drum: "Another drum is picked. / Tap the blinking
key." Checked on the simulated FM-1 (build/twin) with the test lesson in tutorial/upgrade/drumcheck/:

  fm1   the FM-1's own course (fw/course.inc), the keys pressed one by one from here, what it does read after each
  pc    the computer leading (tp/lesson.py), a robot doing step 2; in step 3 the keys are pressed from here, the
        wrong drum first (a robot leaves out the keys that are no boxes)

The lesson's blink is a light at level 4 that goes dark in turns; SLOOP's own lights stay as they are under a strip
(a dim glow on the keys, the black key of the drum picked lit), so "blinks" here means: 4 at some time, less at another.

Run from the trial's top folder:    python3 tutorial/upgrade/drumcheck.py
"""
import os, sys, threading, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..")); sys.path.insert(0, os.path.join(HERE, "..", "..", "tools"))
from tp import lesson as L
_lessons = L.lessons; L.lessons = lambda folder=None: _lessons(os.path.join(HERE, "drumcheck"))   # the pack: this lesson alone
import mkcourse, device
from tp.fm1 import FM1, Twin, BTN, light_of_key
from tp.teach import Teacher, Robot
PH = device.PH; NAME = {v: k for k, v in PH.items()}
KICK, SNARE, RIM, SNR2 = (L.PADS.index(x) for x in ("KICK", "SNARE", "RIM", "SNR 2"))
F_SHARP, G_SHARP, A_SHARP = 1, 3, 5                                 # the black keys of KICK, SNARE and RIM (fw/main.c, DB_KEY)
BOX1, BOX5 = L.WHITE[0], L.WHITE[4]
fails = []
def check(ok, what):
    print("  %s %s" % ("ok  " if ok else "FAIL", what), flush=True)
    if not ok: fails.append(what)
def tap(link, key): link.key(key, 1); time.sleep(0.1); link.key(key, 0); time.sleep(0.25)
def until(fn, secs):
    end = time.time() + secs
    while time.time() < end:
        if fn(): return True
        time.sleep(0.05)
    return False
def blinks(link, light, secs=1.2):
    """a light over a moment: the lesson's blink (level 4 at some time, less at another)"""
    seen = set(); end = time.time() + secs
    while time.time() < end: seen.add(link.lights()[light]); time.sleep(0.03)
    return 4 in seen and min(seen) < 4

def fm1(seed):
    print("fm1: the FM-1's own course", flush=True)
    os.environ["TP_HOME"] = "1"; os.environ.setdefault("UBSAN_OPTIONS", "print_stacktrace=1:halt_on_error=1")
    link = Twin(out=os.path.join("build", "tw", "drumcheck", "fm1"), flash=seed); f = FM1(link)
    at = lambda: f.course()[:3]                                     # (phase, lesson, step from 0)
    def picked(): f.guide(4); return f.picked                       # (4: only the presses since: the course goes on)
    try:
        time.sleep(3.2); device.tap(link, "button", BTN["OCT_UP"])
        if not until(lambda: f.course()[0] == PH["TOC"], 6): check(False, "START begins the FM-1's own course"); return
        device.tap(link, "button", BTN["OCT_UP"]); device.tap(link, "button", BTN["OCT_UP"])
        check(until(lambda: at() == (PH["DO"], 1, 1), 6), "step 2, Pick the snare: waiting (%s, step %d)" % (NAME.get(at()[0]), at()[2] + 1))
        time.sleep(0.3); link.shot("pick-the-snare")                # (its strip: the black key's picture and 2, in a frame)
        check(blinks(link, light_of_key(G_SHARP)), "black 2 in its Do: the snare's black key blinks")
        tap(link, A_SHARP); time.sleep(0.4)
        check(picked() == RIM and at()[2] == 1, "the rim's black key: RIM picked, the step waits (picked %s, step %d)" % (L.PADS[picked()], at()[2] + 1))
        tap(link, G_SHARP)
        check(until(lambda: at()[2] == 2, 3) and picked() in (SNARE, KICK), "the snare's black key: the step ends (step %d)" % (at()[2] + 1))
        check(until(lambda: at() == (PH["DO"], 1, 2), 4) and picked() == KICK, "step 3, kicks on 1 and 5: the kick picked for it (picked %s)" % L.PADS[picked()])
        check(blinks(link, light_of_key(BOX1)), "the right drum: box 1 blinks")
        tap(link, G_SHARP)                                          # the wrong drum, by mistake
        said = until(lambda: at()[0] == PH["MSG"], 1.5); time.sleep(0.3); link.shot("wrong-drum")
        check(said and picked() == SNARE, "the snare picked instead: the lesson says so (%s, picked %s)" % (NAME.get(at()[0]), L.PADS[picked()]))
        check(blinks(link, light_of_key(F_SHARP)) and not blinks(link, light_of_key(BOX1), 0.6), "the kick's black key blinks, box 1 does not")
        check(until(lambda: at()[0] == PH["DO"], 4), "the step's own words again")
        time.sleep(0.6); check(at()[0] == PH["DO"], "the same wrong drum is not said twice")
        check(blinks(link, light_of_key(F_SHARP)) and not blinks(link, light_of_key(BOX1), 0.6), "after the words: still the kick's black key, not the boxes")
        tap(link, G_SHARP)                                          # the same key again: the pair's other drum, SNR 2
        check(until(lambda: at()[0] == PH["MSG"], 1.5) and picked() == SNR2, "the same key again, SNR 2: said again (picked %s)" % L.PADS[picked()])
        until(lambda: at()[0] == PH["DO"], 4)
        tap(link, F_SHARP); time.sleep(0.3)
        check(picked() == KICK and blinks(link, light_of_key(BOX1)), "the blinking key tapped: KICK picked, the boxes blink again")
        tap(link, BOX1); tap(link, BOX5)
        check(until(lambda: at()[2] == 3, 3), "boxes 1 and 5 tapped: the step ends")
        notes = sorted((int(round(n[0])), n[2]) for n in f.pattern(3)[1])
        check(notes == [(0, KICK), (4, KICK)], "the drums: kicks on boxes 1 and 5, nothing else (%s)" % notes)
    finally: link.close()

def pc(seed):
    print("pc: the computer leading, a robot learner", flush=True)
    os.environ.pop("TP_HOME", None)
    link = Twin(out=os.path.join("build", "tw", "drumcheck", "pc"), flash=seed); f = FM1(link)
    try:
        time.sleep(3.2); teacher = Teacher(f, learner=Robot(link)); player = L.Player(teacher, robot_link=link, fast=True)
        lesson = L.lessons()[0]
        def hands():                                                # step 3's keys (only presses from this thread: the player's talk with the FM-1 is its own)
            if not until(lambda: any(x[1] == "strip" and "Kicks" in str(x[2]) for x in list(teacher.log)), 60): return
            time.sleep(2.5); tap(link, G_SHARP)                     # (after the lesson has picked the kick) the snare, by mistake
            time.sleep(2.5); tap(link, F_SHARP); time.sleep(1.0); tap(link, BOX1); tap(link, BOX5)
        threading.Thread(target=hands, daemon=True).start()
        try: player.play(lesson); done = True
        except Exception as e: done = False; print("    %s: %s" % (type(e).__name__, e))
        slips = [(x[1], x[2]) for x in teacher.log if x[1] == "slip"]
        check(done, "the lesson played through")
        check(any("SNARE" in str(x[1]) for x in slips), "the snare picked where the kick is wanted: said (%s)" % slips)
        notes = sorted((int(round(n[0])), n[2]) for n in f.pattern(3)[1])
        check(notes == [(0, KICK), (4, KICK)], "the drums: kicks on boxes 1 and 5, nothing else (%s)" % notes)
        f.guide(4); check(f.picked == KICK, "the FM-1 says KICK is picked (%s)" % L.PADS[f.picked])
    finally:
        try: f.off(); f.all_keys_up()
        except Exception: pass
        link.close()

if __name__ == "__main__":
    seed = device.seed_flash(mkcourse.build()[0])
    fm1(seed); pc(seed)
    print("drumcheck: %s" % ("OK" if not fails else "%d FAILED" % len(fails)))
    sys.exit(1 if fails else 0)
