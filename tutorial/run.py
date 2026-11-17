#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Peachie: run a lesson.
    python3 tutorial/run.py welcome --twin     on the simulated FM-1, a robot learner pressing what blinks; screens saved
    tutorial/.venv/bin/python tutorial/run.py welcome --fm1      on a real FM-1 over USB (not tried on hardware yet);
                                               that Python has the two MIDI packages, mido and python-rtmidi
Run it from the repository's top folder; --twin needs build/twin (make build/twin).
Before the lesson the project on the FM-1 is copied to tutorial/seachest/, in case it is wanted again. It is not
put back: the lesson's song stays on the FM-1, to go on playing with. The run's log goes to tutorial/logbook/."""
import importlib, json, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
for d in ("", "lessons", "songs"): sys.path.insert(0, os.path.join(HERE, d))
from tp.fm1 import FM1, Twin, Midi
from tp.teach import Teacher, Robot

name = sys.argv[1] if len(sys.argv) > 1 else "welcome"
twin = "--fm1" not in sys.argv
stamp = time.strftime("%Y%m%d-%H%M%S")
link = Twin() if twin else Midi()
try:
    fm1 = FM1(link)
    teacher = Teacher(fm1, learner=Robot(link) if twin else None, shots=twin)
    mine = fm1.read_project()                                   # the project that was there: a copy kept, not put back
    os.makedirs(os.path.join(HERE, "seachest"), exist_ok=True)
    open(os.path.join(HERE, "seachest", "project-%s.s3p" % stamp), "wb").write(mine)
    teacher.note("kept", "a copy of the project that was there, %d bytes" % len(mine))
    try:
        while importlib.import_module(name).lesson(teacher) == "left": teacher.note("again", name)   # REPLAY LESSON
        teacher.note("done", name)
    finally:
        fm1.off(); fm1.all_keys_up()                             # (no light left on, no note left sounding)
        if fm1.playing(): fm1.live(5, 0)
        teacher.note("left", "the lesson's song stays on the FM-1")
        os.makedirs(os.path.join(HERE, "logbook"), exist_ok=True)
        json.dump(dict(lesson=name, when=stamp, where="twin" if twin else "fm1", log=teacher.log), open(os.path.join(HERE, "logbook", "%s-%s.json" % (name, stamp)), "w"), indent=1)
    print("LESSON DONE: %d lines logged, %.1f s; answers lost on the way and asked again: %d" % (len(teacher.log), teacher.log[-1][0], fm1.lost))
finally:
    link.close()
