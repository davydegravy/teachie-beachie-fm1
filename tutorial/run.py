#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: run the lessons (tutorial/lessons/NN-name.md).

    tutorial/.venv/bin/python -u tutorial/run.py --fm1 --wait     a real FM-1 over USB: wait for START on its start
                                                                  card, then play the lessons from where the learner is
    tutorial/.venv/bin/python -u tutorial/run.py 5 --fm1 --wait   the same, starting from lesson 5
    python3 tutorial/run.py 5 --twin                              lesson 5 on the simulated FM-1, a robot learner doing
                                                                  what each step asks; its screens are saved

--wait keeps going: START on the FM-1 opens the next lesson's first card; on that card OCT+ begins it and OCT- opens the
list of lessons (SELECT chooses). When a lesson ends, NEXT goes on to the following one. The program finds the FM-1
again after it is unplugged or switched off. --once: one lesson, then stop (the tests). --fast: short pauses (the tests).
--advanced: the steps marked "beginners only" are left out.

Run it from the repository's top folder. --fm1 needs the Python with the two MIDI packages (tutorial/.venv); --twin
needs build/twin (make build/twin). Before each lesson the project on the FM-1 is copied to tutorial/backups/. The
lesson's song stays on the FM-1 afterwards. Each run's log goes to tutorial/logbook/, and where the learner has got
to is kept in tutorial/progress.json."""
import json, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from tp.fm1 import FM1, Twin, Midi, BTN, KNOB, BLINK
from tp.teach import Teacher, Robot
from tp.lesson import Player, lessons as read_lessons

args = [a for a in sys.argv[1:] if not a.startswith("--")]
twin, wait, fast = "--fm1" not in sys.argv, "--wait" in sys.argv, "--fast" in sys.argv
LESSONS = read_lessons()
PROGRESS = os.path.join(HERE, "progress.json")

def find(word):
    """a lesson's place in the list, by its number or a part of its file's name"""
    for i, l in enumerate(LESSONS):
        if (word.isdigit() and l.number == int(word)) or (not word.isdigit() and word in l.name): return i
    raise SystemExit("no lesson %s" % word)

def progress(i=None):
    if i is not None and not twin: json.dump(dict(next=LESSONS[i].number), open(PROGRESS, "w"))
    if args: return find(args[0])
    try: return find(str(json.load(open(PROGRESS))["next"]))
    except Exception: return 0

def begin(teacher, fm1, i):
    """a lesson's first card. OCT+ begins it; OCT- opens the list, where SELECT chooses. Returns the lesson to play, or
    None to go back to the start card"""
    while True:
        l = LESSONS[i]
        if teacher.ask("LESSON %d\n%s" % (l.number, l.title.upper()), "LESSONS", "BEGIN", big=True) == "right": return i
        fm1.take(buttons=[BTN["OCT_DOWN"], BTN["OCT_UP"]], knobs=[KNOB["SELECT"]]); fm1.lights({BTN["OCT_DOWN"]: BLINK, BTN["OCT_UP"]: BLINK}, only=True)
        shown = None
        while True:
            if shown != i: l = LESSONS[i]; fm1.card("^< TURN [SELECT] >\nLESSON %d\n%s" % (l.number, l.title.upper()), "HOME", "BEGIN", big=True); shown = i
            got = None
            for kind, which in fm1.events():
                if kind in ("knob_right", "knob_left") and which == KNOB["SELECT"]: i = max(0, min(len(LESSONS) - 1, i + (1 if kind == "knob_right" else -1)))
                elif kind == "button" and which in (BTN["OCT_DOWN"], BTN["OCT_UP"]): got = which
            if got is not None: break
            time.sleep(0.03)
        fm1.take(); fm1.lights()
        if got == BTN["OCT_DOWN"]: return None
        return i

def lesson(fm1, link, i):
    """one lesson played (again while the learner asks for REPLAY). Returns "next" """
    try: LESSONS[i] = type(LESSONS[i])(LESSONS[i].path)        # the lesson's file read again: an edit made since the program began is in
    except Exception as e: print("lesson %d not read again (%s): played as it was" % (LESSONS[i].number, e), flush=True)
    l = LESSONS[i]; stamp = time.strftime("%Y%m%d-%H%M%S")
    teacher = Teacher(fm1, learner=Robot(link) if twin else None, shots=twin)
    there = fm1.read_project()                                  # the project that was there: a copy kept, not put back
    if not twin:
        os.makedirs(os.path.join(HERE, "backups"), exist_ok=True)
        open(os.path.join(HERE, "backups", "project-%s.s3p" % stamp), "wb").write(there)
    teacher.note("kept", "a copy of the project that was there, %d bytes" % len(there))
    player = Player(teacher, robot_link=link if twin else None, fast=fast, path="advanced" if "--advanced" in sys.argv else "beginner")
    player.clumsy = "--clumsy" in sys.argv                    # (the tests: a robot that taps a blinking box before holding SEQ)
    try:
        while player.play(l) == "replay": teacher.note("again", l.name)
        teacher.note("finished", l.name)
    finally:
        try:
            fm1.off(); fm1.all_keys_up()                         # (no light left on, no note left sounding)
        finally:
            folder = os.path.join(HERE, "logbook") if not twin else link.out
            os.makedirs(folder, exist_ok=True)
            json.dump(dict(lesson=l.name, when=stamp, where="twin" if twin else "fm1", log=teacher.log), open(os.path.join(folder, "%s-%s.json" % (l.name, stamp)), "w"), indent=1)
    print("LESSON DONE: %s, %d lines logged, %.1f s; answers lost on the way and asked again: %d" % (l.name, len(teacher.log), teacher.log[-1][0], fm1.lost), flush=True)
    return "next"

def course(fm1, link, i):
    """from a lesson's first card on: the lesson, then the next one's first card, until the learner goes home"""
    while True:
        teacher = Teacher(fm1, learner=Robot(link) if twin else None)
        i = begin(teacher, fm1, i)
        if i is None: return
        progress(i); lesson(fm1, link, i)
        if "--once" in sys.argv: return
        if i + 1 >= len(LESSONS): teacher.reply("THAT WAS\nTHE LAST\nLESSON.", hold=2.5); return
        i += 1; progress(i)

def serve():
    """wait for START, play lessons, show the start card again; find the FM-1 again when it comes back"""
    said = None
    while True:
        try: link = Twin() if twin else Midi()
        except RuntimeError as e:
            if said != "none": print("waiting for the FM-1 (%s)" % e, flush=True); said = "none"
            time.sleep(2); continue
        robot = Robot(link) if twin else None
        try:
            fm1 = FM1(link); fm1.events()
            if fm1.state & 1 and not fm1.state & 2:                  # a lesson's card still up: the lesson before was cut short
                fm1.off(); fm1.all_keys_up()
                if fm1.playing(): fm1.live(5, 0)
                fm1.home()
            elif not fm1.state & 2 and not fm1.playing(): fm1.home()   # (no card up and not playing: the start card)
            print("FM-1 found: waiting for START", flush=True); said = "found"
            while True:
                if fm1.start_asked():
                    print("START pressed", flush=True)
                    course(fm1, link, progress()); fm1.home()
                    if "--once" in sys.argv: return
                    print("waiting for START", flush=True)
                if robot: robot.act()
                time.sleep(0.2)
        except (TimeoutError, OSError) as e:
            print("lost the FM-1 (%s); looking for it again" % e, flush=True); said = None
            time.sleep(2)
        finally:
            link.close()

if __name__ == "__main__":
    if wait: serve()
    else:
        link = Twin(out=os.path.join("build", "tw", "lessons", LESSONS[progress()].name)) if twin else Midi()
        try:
            fm1 = FM1(link); i = progress()
            teacher = Teacher(fm1, learner=Robot(link) if twin else None)
            if begin(teacher, fm1, i) is not None: lesson(fm1, link, i)
        finally: link.close()
