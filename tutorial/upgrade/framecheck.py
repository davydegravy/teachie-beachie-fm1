#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""(SLOOP 3.4.2 trial, step 6) The frames that answer the controls.

The framed names on a card or a strip ([SEQ], [KNOB1], [5], the black key's picture) answer the real controls:

  a button or a key   its frame is filled, the name in the paper's colour, while it is held; outlined again when let go
  a knob              its frame fills as far as the turn the step asks for is done, from the left (a turn to the right) or
                      from the right (a turn to the left): a set number of clicks, each its share; a turn to a value the
                      knob sets (the tempo, a level, a sound's value), as the value nears it. Full: the turn is done
  left as they are    a name that is no control the firmware can see ([MASTER]); the [OCT-] and [OCT+] answers of a card

The guide draws it (fw/main.c); the lesson player says how far a knob's turn is (fw/course.inc on the FM-1; from the
computer tp/lesson.py, with the guide's message 8). Checked on the simulated FM-1 (build/twin) by looking at its
screen, with the test lesson in tutorial/upgrade/framecheck/:

  fm1    the FM-1's own course: every press and turn from here, the frames looked at after each
  pc     the computer: cards shown straight (held buttons and keys, the black key's picture, [MASTER], the answers), then
         the lesson led by the computer, a knob turned by clicks and the tempo turned down

Run from the trial's top folder:    python3 tutorial/upgrade/framecheck.py
"""
import os, sys, threading, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..")); sys.path.insert(0, os.path.join(HERE, "..", "..", "tools"))
from tp import lesson as L
_lessons = L.lessons; L.lessons = lambda folder=None: _lessons(os.path.join(HERE, "framecheck"))   # the pack: this lesson alone
import mkcourse, device
from tp.fm1 import FM1, Twin, BTN, KNOB
from tp.teach import Teacher, Robot
PH = device.PH; NAME = {v: k for k, v in PH.items()}
PAPER, INK = (255, 202, 164), (148, 0, 0)                           # app/vis.h: TP_PAPER, TP_INK, as the screen shows them
fails = []
def check(ok, what):
    print("  %s %s" % ("ok  " if ok else "FAIL", what), flush=True)
    if not ok: fails.append(what)
def until(fn, secs):
    end = time.time() + secs
    while time.time() < end:
        if fn(): return True
        time.sleep(0.05)
    return False
def turn(link, knob, clicks):
    for _ in range(abs(clicks)): link.knob(KNOB[knob], 1 if clicks > 0 else -1); time.sleep(0.08)
    time.sleep(0.45)

# ---- looking at the screen
def dist(a, b): return sum(abs(a[i] - b[i]) for i in range(3))
def is_ink(p): return dist(p, INK) < dist(p, PAPER)
def card(p): return dist(p, INK) < 90 or dist(p, PAPER) < 90        # a pixel of a card or a strip (SLOOP's pages are dark)
def load(path):
    b = open(path, "rb").read(); assert b[:15] == b"P6 240 240 255\n", path; d = b[15:]
    return [[tuple(d[3 * (y * 240 + x): 3 * (y * 240 + x) + 3]) for x in range(240)] for y in range(240)]
def frames(im):
    """the framed names, top to bottom and left to right: a top edge is a long run of ink with paper above it, its
    bottom edge the same run lower down with paper under it"""
    found = []
    for y in range(1, 238):
        x = 0
        while x < 240:
            if dist(im[y][x], INK) < 90 and dist(im[y - 1][x], PAPER) < 90:
                x0 = x
                while x < 240 and dist(im[y][x], INK) < 90 and dist(im[y - 1][x], PAPER) < 90: x += 1
                if x - x0 >= 10:
                    for y2 in range(y + 19, min(y + 22, 238)):      # (a frame in the small letters is 20 px high: not the edge of the frame under it)
                        if all(dist(im[y2][xx], INK) < 90 for xx in range(x0, x)) and all(dist(im[y2 + 1][xx], PAPER) < 90 for xx in range(x0, x)):
                            if not any(abs(f[0] - (x0 - 3)) < 5 and abs(f[1] - y) < 5 for f in found): found.append((x0 - 3, y, x + 2, y2))
                            break
            else: x += 1
    return sorted(found, key=lambda f: (f[1] // 12, f[0]))
def filled(im, fr, a=0.0, b=1.0):
    """how much of a frame's inside is ink, between a and b of its width (a name's letters alone: about a third)"""
    x0, y0, x1, y1 = fr; w = x1 - x0 - 5; n = k = 0
    for y in range(y0 + 3, y1 - 2):
        for x in range(x0 + 3 + int(a * w), x0 + 3 + int(b * w)): n += 1; k += is_ink(im[y][x])
    return k / max(n, 1)
def look(link, name, wait=0.45):
    """the screen now: for each framed name, how full its left, middle and right thirds are"""
    time.sleep(wait); im = load(link.shot(name))
    return [tuple(round(filled(im, fr, i / 3, (i + 1) / 3), 2) for i in range(3)) for fr in frames(im)]
FULL = lambda f: min(f) > 0.5
PLAIN = lambda f: max(f) < 0.5

def fm1(seed):
    print("fm1: the FM-1's own course", flush=True)
    os.environ["TP_HOME"] = "1"; os.environ.setdefault("UBSAN_OPTIONS", "print_stacktrace=1:halt_on_error=1")
    link = Twin(out=os.path.join("build", "tw", "framecheck", "fm1"), flash=seed); f = FM1(link)
    at = lambda: f.course()[:3]                                     # (phase, lesson, step from 0)
    try:
        time.sleep(3.2); device.tap(link, "button", BTN["OCT_UP"])
        if not until(lambda: f.course()[0] == PH["TOC"], 6): check(False, "START begins the FM-1's own course"); return
        device.tap(link, "button", BTN["OCT_UP"]); device.tap(link, "button", BTN["OCT_UP"])
        check(until(lambda: at() == (PH["DO"], 1, 1), 8), "step 2, a button and a key to hold: waiting (%s, step %d)" % (NAME.get(at()[0]), at()[2] + 1))
        fr = look(link, "2-at-rest"); check(len(fr) == 3 and all(PLAIN(x) for x in fr), "at rest: three frames, all outlined (%s)" % fr)
        link.button(BTN["GLO"], 1); fr = look(link, "2-glo-held"); check(len(fr) == 3 and FULL(fr[0]) and PLAIN(fr[1]) and PLAIN(fr[2]), "[GLO] held: its frame filled, the others not (%s)" % fr)
        link.key(L.WHITE[10], 1); fr = look(link, "2-glo-and-11"); check(len(fr) == 3 and FULL(fr[0]) and FULL(fr[1]) and PLAIN(fr[2]), "key 11 held too: both filled; [MASTER] as it was (%s)" % fr)
        link.key(L.WHITE[10], 0); fr = look(link, "2-key-let-go"); check(len(fr) == 3 and FULL(fr[0]) and PLAIN(fr[1]), "the key let go: outlined again, [GLO] still filled (%s)" % fr)
        link.button(BTN["GLO"], 0); fr = look(link, "2-let-go"); check(len(fr) == 3 and all(PLAIN(x) for x in fr), "both let go: all outlined (%s)" % fr)
        device.tap(link, "button", BTN["PLAY"])
        check(until(lambda: at() == (PH["DO"], 1, 3), 8), "step 4, two knobs by clicks: waiting (%s, step %d)" % (NAME.get(at()[0]), at()[2] + 1))
        fr = look(link, "4-at-rest"); check(len(fr) == 2 and all(PLAIN(x) for x in fr), "at rest: [KNOB2] and [KNOB4] outlined (%s)" % fr)
        turn(link, "KNOB2", 1); fr = look(link, "4-knob2-1-of-3", 0.1); check(len(fr) == 2 and fr[0][0] > 0.5 and fr[0][1] < 0.5 and fr[0][2] < 0.5 and PLAIN(fr[1]), "[KNOB2] one click of three: its left third filled (%s)" % fr)
        turn(link, "KNOB2", 1); fr = look(link, "4-knob2-2-of-3", 0.1); check(len(fr) == 2 and fr[0][0] > 0.5 and fr[0][1] > 0.5 and fr[0][2] < 0.5, "two clicks: two thirds (%s)" % fr)
        turn(link, "KNOB2", -1); fr = look(link, "4-knob2-back", 0.1); check(len(fr) == 2 and fr[0][0] > 0.5 and fr[0][1] < 0.5, "one click back: one third again (%s)" % fr)
        turn(link, "KNOB2", 2); fr = look(link, "4-knob2-3-of-3", 0.1); check(len(fr) == 2 and FULL(fr[0]) and PLAIN(fr[1]), "three clicks: [KNOB2] full, [KNOB4] not begun (%s)" % fr)
        turn(link, "KNOB4", 1); fr = look(link, "4-knob4-1-of-2", 0.1); check(len(fr) == 2 and FULL(fr[0]) and fr[1][0] > 0.5 and fr[1][2] < 0.5, "[KNOB4] one click of two: its left half filled (%s)" % fr)
        turn(link, "KNOB4", 1)
        check(until(lambda: at() == (PH["DO"], 1, 5), 8), "the second click: HARD, the step ends; step 6, the tempo down: waiting (%s, step %d)" % (NAME.get(at()[0]), at()[2] + 1))
        t0 = f.status()["tempo"]; fr = look(link, "6-at-rest"); check(len(fr) == 1 and PLAIN(fr[0]), "at rest: [SELECT] outlined (tempo %d) (%s)" % (t0, fr))
        turn(link, "SELECT", -6); t1 = f.status()["tempo"]; fr = look(link, "6-part-way", 0.1); part = (t0 - t1) / max(t0 - 109, 1)
        check(len(fr) == 1 and 0.25 < part < 0.75 and fr[0][2] > 0.5 and fr[0][0] < 0.5, "tempo %d, %d%% of the way to below 110: filled from the right, the left still empty (%s)" % (t1, round(100 * part), fr))
        for _ in range(40):
            if f.status()["tempo"] < 110: break
            turn(link, "SELECT", -1)
        check(until(lambda: at()[2] == 6, 8), "below 110: the step ends (tempo %d)" % f.status()["tempo"])
    finally: link.close()

def pc(seed):
    print("pc: the computer", flush=True)
    os.environ.pop("TP_HOME", None)
    link = Twin(out=os.path.join("build", "tw", "framecheck", "pc"), flash=seed); f = FM1(link); seen = {}
    try:
        time.sleep(3.2)
        f.card(L.words("Press [PLAY], hold [REC], / tap [BLACK 2] or [KEY 5]. / [MASTER] is the volume."), "Back", "Next")
        fr = look(link, "card-at-rest"); check(len(fr) == 7 and all(PLAIN(x) for x in fr), "a card: seven frames (five names, two answers), all outlined (%s)" % len(fr))
        link.button(BTN["REC"], 1); link.key(L.BLACK[1], 1); link.key(L.WHITE[4], 1); link.button(BTN["OCT_UP"], 1); fr = look(link, "card-held")
        check(len(fr) == 7 and PLAIN(fr[0]) and FULL(fr[1]) and FULL(fr[2]) and FULL(fr[3]) and PLAIN(fr[4]), "[REC], the black key's picture and [KEY 5] held: filled; [PLAY] and [MASTER] not (%s)" % fr[:5])
        check(len(fr) == 7 and PLAIN(fr[5]) and PLAIN(fr[6]), "[OCT+] held: the card's answers stay as they are (%s)" % fr[5:])
        link.button(BTN["OCT_UP"], 0); link.key(L.WHITE[4], 0); link.key(L.BLACK[1], 0); link.button(BTN["REC"], 0); time.sleep(0.3)
        f.card(L.words("[KNOB4] down past 30."), strip=2); f.knob_fill(KNOB["KNOB4"], 128, True); fr = look(link, "strip-from-the-right")
        check(len(fr) == 1 and fr[0][2] > 0.5 and fr[0][0] < 0.5, "the guide's message 8, half from the right: the right half filled (%s)" % fr)
        f.card(L.words("[KNOB4] down past 30."), strip=2); fr = look(link, "strip-new-card"); check(len(fr) == 1 and PLAIN(fr[0]), "the card shown again: its frame empty (%s)" % fr)
        f.off(); time.sleep(0.3)
        teacher = Teacher(f, learner=Robot(link)); player = L.Player(teacher, robot_link=link, fast=True); lesson = L.lessons()[0]
        shown = lambda words: any(x[1] == "strip" and words in str(x[2]) for x in list(teacher.log))
        def hands():                                                # (only presses, turns and pictures from this thread: the player's talk with the FM-1 is its own)
            if not until(lambda: shown("stays plain"), 60): return
            time.sleep(1.0); link.button(BTN["GLO"], 1); link.key(L.WHITE[10], 1); seen["held"] = look(link, "lesson-held"); link.key(L.WHITE[10], 0); link.button(BTN["GLO"], 0)
            time.sleep(0.3); device.tap(link, "button", BTN["PLAY"])
            if not until(lambda: shown("to STEP 4"), 60): return
            time.sleep(1.2); turn(link, "KNOB2", 1); seen["1 of 3"] = look(link, "lesson-knob2-1-of-3", 0.3)
            turn(link, "KNOB2", 2); seen["3 of 3"] = look(link, "lesson-knob2-3-of-3", 0.3); turn(link, "KNOB4", 2)
            if not until(lambda: shown("below 110"), 60): return
            time.sleep(1.2); turn(link, "SELECT", -6); seen["tempo"] = look(link, "lesson-tempo-part-way", 0.3)
            for _ in range(12): turn(link, "SELECT", -1)
        threading.Thread(target=hands, daemon=True).start()
        try: player.play(lesson); done = True
        except Exception as e: done = False; print("    %s: %s" % (type(e).__name__, e))
        check(done, "the lesson led by the computer played through")
        fr = seen.get("held") or []; check(len(fr) == 3 and FULL(fr[0]) and FULL(fr[1]) and PLAIN(fr[2]), "its step 2: [GLO] and key 11 held, both filled (%s)" % fr)
        fr = seen.get("1 of 3") or []; check(len(fr) == 2 and fr[0][0] > 0.5 and fr[0][2] < 0.5, "its step 4, [KNOB2] one click of three: the left third (%s)" % fr)
        fr = seen.get("3 of 3") or []; check(len(fr) == 2 and FULL(fr[0]) and PLAIN(fr[1]), "three clicks: [KNOB2] full (%s)" % fr)
        fr = seen.get("tempo") or []; check(len(fr) == 1 and fr[0][2] > 0.5 and fr[0][0] < 0.5, "its step 6, the tempo part of the way down: filled from the right (%s)" % fr)
    finally:
        try: f.off(); f.all_keys_up()
        except Exception: pass
        link.close()

if __name__ == "__main__":
    seed = device.seed_flash(mkcourse.build()[0])
    fm1(seed); pc(seed)
    print("framecheck: %s" % ("OK" if not fails else "%d FAILED" % len(fails)))
    sys.exit(1 if fails else 0)
