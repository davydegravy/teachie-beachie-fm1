#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: hear a lesson's song before it goes on an FM-1. The simulated FM-1 plays it through SLOOP 3's
own sound engine and keeps the sound as a file.
    python3 tutorial/preview.py house-one [section] [seconds]   ->  build/tw/lesson/house-one.wav"""
import importlib, os, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
for d in ("", "songs"): sys.path.insert(0, os.path.join(HERE, d))
from tp.fm1 import FM1, Twin
from tp.teach import Teacher
from tp.song import load_spec

name = sys.argv[1] if len(sys.argv) > 1 else "house-one"
section = sys.argv[2] if len(sys.argv) > 2 else "C"
secs = float(sys.argv[3]) if len(sys.argv) > 3 else 16.2
link = Twin()
try:
    fm1 = FM1(link); Teacher(fm1).load_song(load_spec(name, section))
    path = link.wav(name); fm1.live(5, 1); time.sleep(secs); fm1.live(5, 0); time.sleep(0.6); link.wav()
    print("SOUND:", path)
finally:
    link.close()
