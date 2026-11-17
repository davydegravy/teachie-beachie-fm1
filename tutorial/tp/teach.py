# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Peachie: what a lesson is written with. A Teacher shows cards, points with lights, waits for a press,
plays a phrase, and keeps a log of what happened and when."""
import time
from .fm1 import BTN, BLINK, light_of_key

class Teacher:
    def __init__(self, fm1, learner=None, shots=False):
        self.fm1, self.learner, self.shots, self.t0, self.log = fm1, learner, shots, time.time(), []
    def note(self, what, value=""):
        self.log.append((round(time.time() - self.t0, 2), what, value)); print("%7.2f  %-12s %s" % (self.log[-1]))
    def _shot(self, name):
        if name and self.shots and hasattr(self.fm1.link, "shot"): time.sleep(0.25); self.fm1.link.shot(name)
    # words
    def say(self, words, left="", right="", shot=None, big=False):
        self.fm1.card(words, left, right, big=big); self.note("card", words.replace("\n", " / ")); self._shot(shot)
    def reply(self, word, hold=1.2, shot=None):
        """the dry voice: a word or two, large, alone on the dark screen"""
        self.fm1.lights({}, only=True); self.fm1.card(word, big=True); self.note("reply", word); self._shot(shot); time.sleep(hold)
    def strip(self, words, where=1, shot=None):
        """a line or two over SLOOP's own screen, which stays live (where: 1 the top, 2 the bottom)"""
        self.fm1.card(words, strip=where); self.note("strip", words.replace("\n", " / ")); self._shot(shot)
    def load_song(self, song):
        """a lesson's song put on the FM-1: each player's sound, level and notes, and the tempo"""
        f = self.fm1; f.tempo(song["bpm"])
        for tr in song["tracks"]:
            f.sound(tr["track"], tr["sound"], kit=tr.get("kit", False)); f.level(tr["track"], tr["level"])
            n = f.write_pattern(tr["track"], tr["steps"], tr["notes"])
            self.note("song", "%-9s %-10s %3d notes" % ("track %d" % tr["track"], tr["sound"], n))
    # lights and presses
    def point(self, lights, only=True): self.fm1.lights({l: BLINK for l in lights}, only=only)
    def wait_for(self, want, timeout=120):
        """wait for one of the presses in want, a set of (kind, which); the others are logged"""
        self.fm1.events(); t0 = time.time(); nudge = t0 + 0.4
        while time.time() - t0 < timeout:
            for ev in self.fm1.events():
                if ev in want: self.note("press", "%s %d  (%.1f s)" % (ev[0], ev[1], time.time() - t0)); return ev
                if not ev[0].endswith("_up"): self.note("other press", "%s %d" % ev)
            if self.learner and time.time() >= nudge: self.learner.act(); nudge = time.time() + 1.5
            time.sleep(0.02)
        raise TimeoutError("nobody pressed %s" % sorted(want))
    def ask(self, words, left, right, shot=None, big=False):
        """a question with two answers on the OCT buttons (one answer: only that button)"""
        oct_ = [b for b, a in ((BTN["OCT_DOWN"], left), (BTN["OCT_UP"], right)) if a]
        self.fm1.take(buttons=[BTN["OCT_DOWN"], BTN["OCT_UP"]])
        self.say(words, left, right, shot, big); self.point(oct_)
        ev = self.wait_for({("button", b) for b in oct_})
        self.fm1.take(); self.fm1.lights()
        return "left" if ev[1] == BTN["OCT_DOWN"] else "right"
    # music
    def play(self, keys, beat=0.3):
        """the FM-1 plays a phrase itself, each key lit as it sounds"""
        self.note("plays", "%d notes" % len(keys))
        for k in keys:
            self.fm1.lights({light_of_key(k): 4}, only=True); self.fm1.play_key(k, True)
            time.sleep(beat * 0.7); self.fm1.play_key(k, False); time.sleep(beat * 0.3)
        self.fm1.lights({}, only=True)
    def echo(self, keys):
        """the learner plays the phrase back, each key pointed at in turn"""
        for k in keys: self.point([light_of_key(k)]); self.wait_for({("key", k)})
        self.fm1.lights({}, only=True)

class Robot:
    """A learner for the tests: it looks at the lights and presses what blinks. With two blinking OCT buttons it
    gives its next answer; with nothing blinking it presses a key."""
    def __init__(self, link, answers=()): self.link, self.answers = link, list(answers)
    def act(self):
        on, off = set(), set()
        for _ in range(12):
            for i, v in enumerate(self.link.lights()): (on if v >= 4 else off if v == 0 else set()).add(i)
            time.sleep(0.045)
        blink = sorted(on & off)
        if 0 in blink and 1 in blink: b = 0 if (self.answers.pop(0) if self.answers else "right") == "left" else 1
        elif blink: b = blink[0]
        else: b = 14 + 7
        press = (lambda d: self.link.button(b, d)) if b < 14 else (lambda d: self.link.key(b - 14, d))
        press(True); time.sleep(0.09); press(False)
