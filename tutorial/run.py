#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Peachie: run a lesson.
    python3 tutorial/run.py welcome --twin     on the simulated FM-1, a robot learner pressing what blinks; screens saved
    tutorial/.venv/bin/python tutorial/run.py welcome --fm1      on a real FM-1 over USB; that Python has the two
                                               MIDI packages, mido and python-rtmidi
Add --wait to keep going: the program waits for START on the FM-1's start card, runs the lesson, shows the start
card again, and waits once more. It finds the FM-1 again after it is unplugged or switched off. (--once: one lesson,
then stop; for the tests.)
Run it from the repository's top folder; --twin needs build/twin (make build/twin).
Before a lesson the project on the FM-1 is copied to tutorial/seachest/, in case it is wanted again. It is not
put back: the lesson's song stays on the FM-1, to go on playing with. The run's log goes to tutorial/logbook/."""
import importlib, json, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
for d in ("", "lessons", "songs"): sys.path.insert(0, os.path.join(HERE, d))
from tp.fm1 import FM1, Twin, Midi
from tp.teach import Teacher, Robot

name = next((a for a in sys.argv[1:] if not a.startswith("--")), "welcome")
twin = "--fm1" not in sys.argv
wait = "--wait" in sys.argv

def lesson(fm1, link):
    stamp = time.strftime("%Y%m%d-%H%M%S")
    teacher = Teacher(fm1, learner=Robot(link) if twin else None, shots=twin)
    there = fm1.read_project()                                  # the project that was there: a copy kept, not put back
    os.makedirs(os.path.join(HERE, "seachest"), exist_ok=True)
    open(os.path.join(HERE, "seachest", "project-%s.s3p" % stamp), "wb").write(there)
    teacher.note("kept", "a copy of the project that was there, %d bytes" % len(there))
    try:
        while importlib.import_module(name).lesson(teacher) == "left": teacher.note("again", name)   # REPLAY
        teacher.note("done", name)
    finally:
        try:
            fm1.off(); fm1.all_keys_up()                         # (no light left on, no note left sounding)
            if fm1.playing(): fm1.live(5, 0)
        finally:
            teacher.note("left", "the lesson's song stays on the FM-1")
            os.makedirs(os.path.join(HERE, "logbook"), exist_ok=True)
            json.dump(dict(lesson=name, when=stamp, where="twin" if twin else "fm1", log=teacher.log), open(os.path.join(HERE, "logbook", "%s-%s.json" % (name, stamp)), "w"), indent=1)
    print("LESSON DONE: %d lines logged, %.1f s; answers lost on the way and asked again: %d" % (len(teacher.log), teacher.log[-1][0], fm1.lost), flush=True)

def serve():
    """wait for START, run the lesson, show the start card again; find the FM-1 again when it comes back"""
    said = None
    while True:
        try: link = Twin() if twin else Midi()
        except RuntimeError as e:
            if said != "none": print("waiting for the FM-1 (%s)" % e, flush=True); said = "none"
            time.sleep(2); continue
        robot = Robot(link) if twin else None
        try:
            fm1 = FM1(link); fm1.events()
            if not fm1.state & 2 and not fm1.playing(): fm1.home()   # (no card up and not playing: the start card)
            print("FM-1 found: waiting for START", flush=True); said = "found"
            while True:
                if fm1.start_asked():
                    print("START pressed", flush=True)
                    lesson(fm1, link); fm1.home()
                    if "--once" in sys.argv: return
                    print("waiting for START", flush=True)
                if robot: robot.act()
                time.sleep(0.2)
        except (TimeoutError, OSError) as e:
            print("lost the FM-1 (%s); looking for it again" % e, flush=True); said = None
            time.sleep(2)
        finally:
            link.close()

if wait: serve()
else:
    link = Twin() if twin else Midi()
    try: lesson(FM1(link), link)
    finally: link.close()
