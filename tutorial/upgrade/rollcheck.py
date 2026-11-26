#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""(SLOOP 3.4.2 trial, step 4) The piano roll: which key a lesson blinks there, and what SHOW ME writes.

On 3.4.2 a synth track has no boxes on the white keys: [SEQ] opens NOTES, where a key played is written at the step
shown. A step that waits for notes in boxes on such a track (`Until: bass has notes on boxes 1 4`) must blink the key
its Do names, not the white keys of the boxes still empty (a tap on those would write other notes); on the drums the
boxes still blink (drumcheck.py). The key waits for the knob: it blinks only while STEP is on a step that still lacks
its note, so a blinking key always means "press this". And SHOW ME on the FM-1 writes the track's last note into the
steps with none.
Checked on the simulated FM-1 (build/twin) with the test lesson in tutorial/upgrade/rollcheck/:

  fm1   the FM-1's own course (fw/course.inc), the keys pressed one by one from here; step 4 is left alone until the
        FM-1 offers to do it, and SHOW ME is taken
  pc    the computer leading (tp/lesson.py); the keys of steps 2 and 4 are pressed from here, a robot does step 3

Run from the trial's top folder:    python3 tutorial/upgrade/rollcheck.py
"""
import os, sys, threading, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..")); sys.path.insert(0, os.path.join(HERE, "..", "..", "tools"))
from tp import lesson as L
_lessons = L.lessons; L.lessons = lambda folder=None: _lessons(os.path.join(HERE, "rollcheck"))   # the pack: this lesson alone
import mkcourse, device
from tp.fm1 import FM1, Twin, BTN, KNOB, light_of_key
from tp.teach import Teacher, Robot
PH = device.PH; NAME = {v: k for k, v in PH.items()}
A3, BOX1, BOX4, BOX7 = L.key_of("A3"), L.WHITE[0], L.WHITE[3], L.WHITE[6]
fails = []
def check(ok, what):
    print("  %s %s" % ("ok  " if ok else "FAIL", what), flush=True)
    if not ok: fails.append(what)
def tap(link, key): link.key(key, 1); time.sleep(0.1); link.key(key, 0); time.sleep(0.3)
def turn(link, clicks):
    for _ in range(clicks): link.knob(KNOB["KNOB1"], 1); time.sleep(0.08)
    time.sleep(0.25)
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
def bass(f): return sorted((int(round(n[0])) + 1, n[2]) for n in f.pattern(0)[1])

def fm1(seed):
    print("fm1: the FM-1's own course", flush=True)
    os.environ["TP_HOME"] = "1"; os.environ.setdefault("UBSAN_OPTIONS", "print_stacktrace=1:halt_on_error=1")
    link = Twin(out=os.path.join("build", "tw", "rollcheck", "fm1"), flash=seed); f = FM1(link)
    at = lambda: f.course()[:3]                                     # (phase, lesson, step from 0)
    try:
        time.sleep(3.2); device.tap(link, "button", BTN["OCT_UP"])
        if not until(lambda: f.course()[0] == PH["TOC"], 6): check(False, "START begins the FM-1's own course"); return
        device.tap(link, "button", BTN["OCT_UP"]); device.tap(link, "button", BTN["OCT_UP"])
        check(until(lambda: at() == (PH["DO"], 1, 1), 6), "step 2, an A on step 1: waiting (%s, step %d)" % (NAME.get(at()[0]), at()[2] + 1))
        time.sleep(0.3); link.shot("play-the-a")
        check(blinks(link, light_of_key(A3)) and not blinks(link, light_of_key(BOX1), 0.6), "the A's key blinks, box 1's key does not")
        tap(link, A3)
        check(until(lambda: at() == (PH["DO"], 1, 2), 4), "the A played: written on step 1, the step ends (%s)" % bass(f))
        check(not blinks(link, light_of_key(A3)) and not blinks(link, light_of_key(BOX4), 0.6), "step 3, an A on step 4, STEP on 2: nothing blinks yet (the key waits for the knob)")
        turn(link, 1); check(not blinks(link, light_of_key(A3)), "[KNOB1] one click, STEP on 3: still nothing")
        turn(link, 1); check(blinks(link, light_of_key(A3)) and not blinks(link, light_of_key(BOX4), 0.6), "one more, STEP on 4: the A's key blinks, box 4's key does not")
        turn(link, 1); check(not blinks(link, light_of_key(A3)), "one too many, STEP on 5: the key stops blinking")
        link.knob(KNOB["KNOB1"], -1); time.sleep(0.3); tap(link, A3)
        check(until(lambda: at() == (PH["DO"], 1, 3), 4) and [s for s, _ in bass(f)] == [1, 4], "back one click and the A: on step 4, the step ends (%s)" % bass(f))
        print("    (step 4 left alone: the FM-1 offers to do it after 45 s)", flush=True)
        check(until(lambda: at()[0] == PH["STUCK"], 60), "step 4, nothing done: the FM-1 offers to do it (%s)" % NAME.get(at()[0]))
        time.sleep(0.3); link.shot("stuck"); device.tap(link, "button", BTN["OCT_UP"])
        check(until(lambda: at()[2] == 4, 6), "SHOW ME: the step ends")
        note = bass(f)[0][1] if bass(f) else None
        check(bass(f) == [(1, note), (4, note), (7, note)], "SHOW ME wrote the track's last note on step 7 (%s)" % bass(f))
    finally: link.close()

def pc(seed):
    print("pc: the computer leading", flush=True)
    os.environ.pop("TP_HOME", None)
    link = Twin(out=os.path.join("build", "tw", "rollcheck", "pc"), flash=seed); f = FM1(link); seen = {}
    try:
        time.sleep(3.2); teacher = Teacher(f, learner=Robot(link)); player = L.Player(teacher, robot_link=link, fast=True)
        lesson = L.lessons()[0]
        shown = lambda words: any(x[1] == "strip" and words in str(x[2]) for x in list(teacher.log))
        def hands():                                                # steps 2 and 4 (only presses and lights from this thread: the player's talk with the FM-1 is its own)
            if not until(lambda: shown("STEP says 1"), 60): return
            time.sleep(1.0); seen["a"] = blinks(link, light_of_key(A3)); seen["box"] = blinks(link, light_of_key(BOX1), 0.6); tap(link, A3)
            if not until(lambda: shown("Step 7"), 60): return
            time.sleep(1.0); seen["before"] = blinks(link, light_of_key(A3)); turn(link, 2); time.sleep(0.4); seen["at 7"] = blinks(link, light_of_key(A3)); tap(link, A3)
        threading.Thread(target=hands, daemon=True).start()
        try: player.play(lesson); done = True
        except Exception as e: done = False; print("    %s: %s" % (type(e).__name__, e))
        check(done, "the lesson played through")
        check(seen.get("a") and not seen.get("box"), "step 2, STEP on 1: the A's key blinks, box 1's key does not (%s)" % seen)
        check(seen.get("before") is False and seen.get("at 7"), "step 4: the key waits for the knob, and blinks once STEP is on 7 (%s)" % seen)
        check([s for s, _ in bass(f)] == [1, 4, 7], "the bass: notes on steps 1, 4 and 7 (%s)" % bass(f))
    finally:
        try: f.off(); f.all_keys_up()
        except Exception: pass
        link.close()

if __name__ == "__main__":
    seed = device.seed_flash(mkcourse.build()[0])
    fm1(seed); pc(seed)
    print("rollcheck: %s" % ("OK" if not fails else "%d FAILED" % len(fails)))
    sys.exit(1 if fails else 0)
