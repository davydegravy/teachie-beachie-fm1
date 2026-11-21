# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: a lesson's song, read from a song spec (tutorial/songs/*.json). The spec has the same form as
the FM-1 song library's, with one more track for SLOOP 3: five tracks (bass, lead, chords, pad, drums), sections A-D with each track's notes as
[pitch, start beat, length in beats, velocity], and the SLOOP sound of each track. The same file builds the song
in Ableton Live (the library's tools/build_song.py) and feeds the lessons."""
import json, os

TRACK = dict(bass=0, lead=1, chords=2, drums=3, pad=4)   # the FM-1's tracks as the firmware counts them (the screen shows the pad fourth, the drums fifth)
PAD = {36: 0, 35: 1, 38: 2, 39: 3, 42: 4, 46: 5, 44: 6, 37: 7, 40: 8, 45: 9, 41: 9, 50: 10, 48: 10, 49: 11, 51: 12, 70: 13, 82: 13, 63: 14, 56: 15}   # General MIDI drums to the kit's 16 pads
LEVEL = dict(bass=100, lead=92, chords=80, drums=104, pad=52)

def load_spec(name, section):
    """one section of a spec, as Teacher.load_song takes it"""
    spec = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "songs", name + ".json")))
    sec, tracks = spec["sections"][section], []
    for t in spec["tracks"]:
        drums = t["role"] == "drums"; s = t["sloop"]
        notes = [(b * 4, l * 4, PAD[p] if drums else p, v) for p, b, l, v in sec["clips"].get(t["name"], [])]
        tracks.append(dict(track=TRACK[t["role"]], sound=s["kit"] if drums else s["preset"], kit=drums, level=s.get("level", LEVEL[t["role"]]), steps=sec["bars"] * 16, notes=notes))
    return dict(name="%s, %s" % (spec["title"], sec["name"]), bpm=spec["tempo"], tracks=tracks)
