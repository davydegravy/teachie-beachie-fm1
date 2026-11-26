# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: a lesson read from its Markdown file (tutorial/lessons/NN-name.md) and played on an FM-1.

The file is written to be read by a person first. The player reads only two things in it: the list at the top
(the lesson's number and title come from the heading) and the numbered list under "## Steps". Everything else,
such as "## Notes for the AI assistant" and "## Tester notes", is for people. The format is described in
tutorial/lessons/README.md.

A step is one numbered line, "N. **Kind:** words", with what goes with it as bullets under it:
  kinds   Setup   what the FM-1 is given before the learner does anything (a song, an empty track, a page)
          Card    a full peach screen of words          Big     a few words, large and centred
          Strip   a band of words over SLOOP's own screen, at the bottom ("Strip top": at the top; "Strip corner": a
                  small tag in the top left corner, as wide as its words, for a step where the screen is needed)
          Reply   the dry voice: a word or two, large, for a moment
          Wait    nothing new for a while
          Play    the FM-1 plays a phrase itself, lighting each key      Echo   the learner plays it back
  bullets OCT- / OCT+   an answer by that button (after an arrow: what it does, "go to N" or a setup)
          Do      what the learner does; its buttons and keys blink
          Until   how the lesson knows it is done (read from the FM-1, so any way of getting there counts)
          Hint    words shown when it takes long (each a while after the one before)
          Play    the FM-1 plays a phrase itself, lighting each key      Echo   the learner plays it back
          Wait    how long the step stays, in seconds or bars
          Keys    "silent": the keys are the lesson's and play nothing
          Lights  keys lit faintly, as part of the picture
          Page    the page of SLOOP the step needs: the lesson brings the learner back to it
          Robot   for the tests only: what the robot learner does here
"""
import os, re, time
from .fm1 import BTN, KNOB, BLINK, light_of_key
from .song import load_spec, TRACK, PAD as GM_PAD

HERE = os.path.dirname(os.path.abspath(__file__))
WHITE = [k for k in range(27) if (53 + k) % 12 in (0, 2, 4, 5, 7, 9, 11)]          # the 16 white keys, F3 to G5
BLACK = [k for k in range(27) if k not in WHITE]                                    # the 11 black keys, F#3 to F#5, from the left
PADS = ["KICK", "KICK2", "SNARE", "CLAP", "HAT", "OPEN", "PEDAL", "RIM", "SNR 2", "TOM L", "TOM H", "CRASH", "RIDE", "SHAKE", "CONGA", "BELL"]
PAGES = dict(TRACKS=0, STEPS=1, OSC=2, FILTER=3, VOICE=4, ENV=5, **{"ENV 2": 6}, SENDS=7, KIT=8, GLOBAL=9, SONG=10, LFO=11, KEY=12, ARP=13,
             **{"LFO 2": 14}, MASTER=15, PLUS=16, **{"ARP 2": 17}, DELAY=18, **{"DRUM SOUND": 19, "KEY 2": 20}, MIDI=21, **{"MIDI 2": 22}, NOTES=23,
             **{"LFO 3": 24}, INSERT=25, **{"INSERT MIX": 26}, SLICER=27, YOURS=28, TEXTURE=29, **{"TEXTURE 2": 30, "SONG 2": 31}, SOUNDS=32, REVERB=33, REC=34,
             PATTERN=35, PROJECT=44)                                                # (SLOOP 3.4.2: PG_PAT, PG_PROJ in fw/main.c)
GROUP = {}                                                                          # a page -> the button whose taps go through its pages
for _b, _names in dict(EDIT="OSC,FILTER,VOICE,PLUS,YOURS,SOUNDS,KIT,DRUM SOUND", FX="SENDS,INSERT,INSERT MIX,SLICER,DELAY,REVERB,MASTER", ARP="ARP,ARP 2",
                       SEL="KEY,KEY 2", ENV="ENV,ENV 2", LFO="LFO,LFO 2,LFO 3", GLO="GLOBAL,TEXTURE,TEXTURE 2,MIDI,MIDI 2,REC").items():
    for _n in _names.split(","): GROUP[_n] = _b
VALUE = dict(ATK=8, DEC=9, SUS=10, REL=11, EFLT=12, EPIT=13, ESHP=14, TRIM=15, LRATE=16, LWAVE=17, LPHASE=18, LFADE=19, LPIT=20, LFLT=21, LSHP=22, LAMP=23,
             LSYNC=24, LTRIG=25, LPOL=26, ESYNC=27, VOICE=28, GLIDE=29, GLMODE=30, PRIO=31, DETUNE=33, FTYPE=34, DIST=35, INSERT=36, IA=37, IB=38, IC=39,
             IMIX=40, SLICER=41, **{"E%d" % i: i for i in range(8)})                # a sound's values, by the names the lessons use
INSERTS = ["OFF", "SOFT", "HARD", "FOLD", "FUZZ", "CRUSH", "PHASR", "FLANG", "CHOR"]
SPEED = dict(slow=0.5, fast=0.075)
KINDS = ("setup", "card", "big", "reply", "strip", "strip top", "strip corner", "wait", "play", "echo")

class Bad(Exception): pass

def key_of(word):
    """a key by its note name (F3 .. G5); None if the word is no note"""
    m = re.fullmatch(r"([A-G])(#?)(\d)", word.upper())
    if not m: return None
    midi = 12 * (int(m.group(3)) + 1) + "C D EF G A B".index(m.group(1)) + (1 if m.group(2) else 0)
    if not 53 <= midi <= 79: raise Bad("%s is not one of the FM-1's keys (F3 to G5)" % word)
    return midi - 53

def midi_of(word):
    """a note of any octave by its name (A2, C#4): its MIDI number, or None"""
    m = re.fullmatch(r"([A-G])(#?)(-?\d)", word.upper())
    return 12 * (int(m.group(3)) + 1) + "C D EF G A B".index(m.group(1)) + (1 if m.group(2) else 0) if m else None

def keys_of(text, phrases=None):
    """the keys a few words name: note names, "box 1 5 9", "white key 3", "black 2" (the second black key from the left),
    "pad SNARE", or a phrase by its name"""
    out, mode = [], None
    for w in text.replace(",", " ").split():
        u = w.upper()
        if phrases and u in phrases: out += phrases[u]; continue
        if u == "BLACK": mode = "black"; continue
        if u in ("KEY", "KEYS") and mode == "black": continue
        if u in ("BOX", "BOXES", "KEY", "KEYS", "WHITE"): mode = "white"; continue
        if u == "PAD": mode = "pad"; continue
        if mode == "pad":
            if u == "SNR": continue
            if u not in PADS: raise Bad("no drum sound called %s" % w)
            out.append(WHITE[PADS.index(u)]); continue
        k = key_of(w)
        if k is not None: out.append(k); continue
        if u.isdigit() and mode == "white" and 1 <= int(u) <= 16: out.append(WHITE[int(u) - 1]); continue
        if u.isdigit() and mode == "black" and 1 <= int(u) <= 11: out.append(BLACK[int(u) - 1]); continue
        raise Bad("not a key: %s (in \"%s\")" % (w, text))
    return out

# ------------------------------------------------------------------------------------------------ reading the file
class Step:
    def __init__(self, n, kind, text, line): self.n, self.kind, self.text, self.line, self.subs, self.tags = n, kind, text, line, [], set()
    def sub(self, key): return next((v for k, v in self.subs if k == key), None)
    def all(self, key): return [v for k, v in self.subs if k == key]

class Lesson:
    def __init__(self, path):
        self.path, self.name = path, os.path.basename(path)[:-3]
        self.head, self.steps, self.phrases = {}, [], {}
        section, last, title = None, None, None
        for ln, raw in enumerate(open(path, encoding="utf-8").read().split("\n"), 1):
            if raw.startswith("# "):
                title = raw[2:].strip(); continue
            if raw.startswith("## "): section, last = raw[3:].strip().lower(), None; continue
            if section not in (None, "steps") or not raw.strip(): continue
            m = re.match(r"^(\d+)\.\s+\*\*([^:*]+):\*\*\s*(.*)$", raw)
            if m and section == "steps":
                last = Step(int(m.group(1)), m.group(2).strip(), m.group(3).strip(), ln); self.steps.append(last); where = ("text", None); continue
            m = re.match(r"^\s*[-*]\s+\*\*([^:*]+):\*\*\s*(.*)$", raw)
            if m and section is None: self.head[m.group(1).strip()] = m.group(2).strip(); headkey = m.group(1).strip(); where = ("head", headkey); continue
            if m and last is not None: last.subs.append([m.group(1).strip(), m.group(2).strip()]); where = ("sub", None); continue
            if raw.startswith(" ") and (last is not None or section is None):                # a line that goes on
                if where[0] == "text": last.text += " " + raw.strip()
                elif where[0] == "sub": last.subs[-1][1] += " " + raw.strip()
                elif where[0] == "head": self.head[where[1]] += " " + raw.strip()
                continue
            if section == "steps": raise Bad("%s line %d: not a step, and not part of one: %s" % (self.name, ln, raw.strip()[:60]))
        m = re.match(r"Lesson (\d+):\s*(.+)$", title or "")
        if not m: raise Bad("%s: the first line should be \"# Lesson N: Title\"" % self.name)
        self.number, self.title = int(m.group(1)), m.group(2).strip()
        for k, v in self.head.items():
            if k.lower().startswith("phrase "): self.phrases[k[7:].strip().upper()] = keys_of(v)
        for s in self.steps:                                                                  # *(beginners only)* and the like
            for where in [s] + s.subs:
                txt = where.text if where is s else where[1]
                for t in re.findall(r"\*\(([^)]*)\)\*", txt): s.tags.add(t.strip().lower())
                txt = re.sub(r"\s*\*\([^)]*\)\*", "", txt).strip()
                if where is s: s.text = txt
                else: where[1] = txt
        if [s.n for s in self.steps] != list(range(1, len(self.steps) + 1)): raise Bad("%s: the steps should be numbered 1, 2, 3 ... in order" % self.name)

def advances(k):
    """the width of each letter of one of the lessons' fonts (0 the small one, 1 the large), from assets/fonts/lessons-font.bin
    (tools/mklessonfont.py): the fonts the FM-1 draws the lessons with, from the pack"""
    b = open(os.path.join(HERE, "..", "..", "assets", "fonts", "lessons-font.bin"), "rb").read()
    at = b[4 + 2 * k] | b[5 + 2 * k] << 8; n = b[at]
    return [b[at + 6 + 8 * i + 4] for i in range(n)]
ADV = dict(s=advances(0), l=advances(1))

def width(font, text): return sum(ADV[font][ord(c) - 32] if 32 <= ord(c) < 127 else ADV[font][31] for c in text)

def wrap(text, font):
    """the words cut into lines as the FM-1 does it (fw/main.c guide_wrap): at a new line, and at a space before the edge.
    An empty line (a gap) comes back as an empty line"""
    lines, i = [], 0
    while i < len(text) and len(lines) < 99:
        s = e = i; last = -1; frame = False
        while e < len(text) and text[e] != "\n":
            if text[e] == "[": frame = True
            elif text[e] == "]": frame = False
            if text[e] == " " and not frame: last = e
            if last > s and width(font, text[s:e + 1]) > 240 - 28: e = last; break
            e += 1
        lines.append(text[s:e]); i = e
        if i < len(text) and text[i] in " \n": i += 1
    return lines

def words(text):
    """the words as the FM-1 gets them. In the file " / " is a new line on the screen and " // " a gap between two thoughts:
    an empty line, which the FM-1 draws 8 px high and does not count as a line. On a card of 4 lines or fewer each gap is
    two of them, 16 px (2026-11-22). A black key, [BLACK 2] in the file, becomes its picture and its number in the frame:
    the picture is the character @ of the lessons' fonts (tools/mklessonfont.py)"""
    text = re.sub(r"\[BLACK(?: (\d+))?\]", lambda m: "[@%s]" % (m.group(1) or ""), text)
    w = re.sub(r"\s+/\s+", "\n", re.sub(r"\s+//\s+", "\n\n", text.strip()))
    if "\n\n" in w and len([x for x in wrap(w, "s") if x]) <= 4: w = w.replace("\n\n", "\n\n\n")
    return w

# the card of the whole course, shown after START (in the lesson pack for the FM-1's own course; tutorial/run.py shows it too)
COURSE = "_30 lessons in 4 parts / 1-7: Your first beat / 8-15: Notes and chords / 16-23: Effects and sound / 24-30: Make a song // ~Each lesson teaches / ~one part of SLOOP."

def lessons(folder=None):
    """every lesson in the folder, in order"""
    folder = folder or os.path.join(HERE, "..", "lessons")
    return [Lesson(os.path.join(folder, f)) for f in sorted(os.listdir(folder)) if re.match(r"\d\d-.*\.md$", f)]

# ------------------------------------------------------------------------------------------------ what a learner does
def parse_do(text, phrases=None):
    """"hold SEQ, tap box 1 5 9 13" -> [("hold", button), ("tap", key), ...]"""
    ops = []
    for clause in re.split(r",|\bthen\b", text):
        c = clause.strip()
        if not c: continue
        w = c.split(); verb, rest = w[0].lower(), " ".join(w[1:])
        def button(name):
            name = name.upper().replace("−", "-")
            if name not in BTN_NAMES: raise Bad("no button called %s (in \"%s\")" % (name, text))
            return BTN_NAMES[name]
        if verb == "press":
            times = 2 if rest.lower().endswith(" twice") else 1
            ops += [("press", button(rest[:-6] if times == 2 else rest))] * times
        elif verb == "hold" and rest.upper().replace("−", "-") in BTN_NAMES: ops.append(("hold", button(rest)))
        elif verb in ("tap", "play"):
            if rest.lower() in ("any key", "a key"): ops.append(("any", None))
            elif rest.lower().endswith(" or any key"): ops.append(("any", keys_of(rest[:-11], phrases)[0]))
            else: ops += [("tap", k) for k in keys_of(rest, phrases)]
        elif verb == "long": ops += [("long", k) for k in keys_of(rest, phrases)]            # a key held about a second
        elif verb == "hold": ops += [("khold", k) for k in keys_of(rest, phrases)]           # a key kept down, as a button is
        elif verb == "turn":
            m = re.match(r"(.+?)\s+(left|right)(?:\s+(\d+))?$", rest, re.I)
            if not m or m.group(1).upper() not in KNOB_NAMES: raise Bad("a turn is written \"turn KNOB 1 right\" or \"turn SELECT left 5\" (in \"%s\")" % text)
            ops.append(("turn", KNOB_NAMES[m.group(1).upper()], (1 if m.group(2).lower() == "right" else -1) * int(m.group(3) or 1)))
        elif verb == "let": ops.append(("letgo", None))
        elif verb == "idle": ops.append(("idle", None))
        elif verb == "wait": ops.append(("wait", float(w[1])))
        elif verb == "answer": ops.append(("answer", w[1].lower()))
        else: raise Bad("I don't know how to \"%s\" (in \"%s\")" % (c, text))
    return ops

BTN_NAMES = {"OCT-": 0, "OCT+": 1, **{k: v for k, v in BTN.items() if not k.startswith("OCT")}}
KNOB_NAMES = {"SELECT": 0, "ALGORITHM": 1, "KNOB 1": 2, "KNOB 2": 3, "KNOB 3": 4, "KNOB 4": 5, "PRESETS": 6}
BTN_OF = {v: k for k, v in BTN_NAMES.items()}

def moves(ops, tap=0.09, gap=0.14):
    """the ops as a hand does them: (what, which, value, how long to wait after)"""
    held, kheld = [], []
    for op in ops:
        if op[0] == "press": yield ("button", op[1], 1, tap); yield ("button", op[1], 0, gap)
        elif op[0] == "hold": held.append(op[1]); yield ("button", op[1], 1, 0.45)
        elif op[0] in ("tap", "any"): k = 14 if op[1] is None else op[1]; yield ("key", k, 1, tap); yield ("key", k, 0, gap)
        elif op[0] == "long": yield ("key", op[1], 1, 0.95); yield ("key", op[1], 0, gap)
        elif op[0] == "khold": kheld.append(op[1]); yield ("key", op[1], 1, 0.3)
        elif op[0] == "turn":
            for _ in range(abs(op[2])): yield ("knob", op[1], 1 if op[2] > 0 else -1, 0.07)
        elif op[0] == "letgo":
            yield ("none", 0, 0, 0.35)
            while kheld: yield ("key", kheld.pop(), 0, gap)
            while held: yield ("button", held.pop(), 0, gap)
        elif op[0] == "wait": yield ("none", 0, 0, op[1])
    if kheld or held: yield ("none", 0, 0, 0.35)
    while kheld: yield ("key", kheld.pop(), 0, gap)
    while held: yield ("button", held.pop(), 0, gap)

# ------------------------------------------------------------------------------------------------ how the lesson knows
class Seen:
    """what the FM-1 says about itself, asked once each time round and kept while a condition is weighed"""
    def __init__(self, fm1, base=None, events=None, msgs=None):
        self.fm1, self.base, self.events, self.msgs, self._st, self._pat, self._snd = fm1, base, events or [], msgs or set(), None, {}, {}
    @property
    def st(self):
        if self._st is None:
            self._st = self.fm1.status(); self.fm1.guide(4)
            if self._st["msg"]: self.msgs.add(self._st["msg"])
        return self._st
    def pattern(self, t, pat=255):
        if (t, pat) not in self._pat: self._pat[(t, pat)] = self.fm1.pattern(t, pat)
        return self._pat[(t, pat)]
    def sound(self, t):
        if t not in self._snd: self._snd[t] = self.fm1.sound_values(t)
        return self._snd[t]
    def value(self, what):
        """a number or a name the conditions compare: ("tempo",), ("track", t, "level"), ("value", t, k) ..."""
        if what[0] == "tempo": return self.st["tempo"]
        if what[0] == "global": return self.st[what[1]]
        if what[0] == "track": return self.st["tracks"][what[1]][what[2]]
        if what[0] == "sound": return self.sound(what[1])[0]
        if what[0] == "value": return self.sound(what[1])[1][what[2]]
        if what[0] == "notes": return len(self.pattern(what[1])[1])
        if what[0] == "pattern": return self.st["tracks"][what[1]]["cur"] + 1
        if what[0] == "lines": return self.fm1.song_lines()
        if what[0] == "longest": return max([n[1] for n in self.pattern(what[1])[1]] or [0])
        if what[0] == "softest": return min([n[3] for n in self.pattern(what[1])[1]] or [127])
        if what[0] == "loudest": return max([n[3] for n in self.pattern(what[1])[1]] or [0])
        if what[0] == "fills": return sum(1 for n in self.pattern(what[1])[1] if n[4] == 1)
        if what[0] == "patterns": return bin(__import__("functools").reduce(lambda a, b: a | b, [x["used"] for x in self.st["tracks"]])).count("1")
        if what[0] == "tracks": return sum(1 for x in self.st["tracks"] if x["used"])
        raise Bad("nothing to read for %s" % (what,))

class Cond:
    def __init__(self, text): self.text, self.fn, self.miss, self.needs_base, self.drum = text, None, None, None, None   # drum: the pad its boxes ask for
    roll = None                                                                    # the steps another track's notes still lack (miss: the drums' boxes)
    def ok(self, seen): return self.fn(seen)

def boxes(text): return [int(x) - 1 for x in re.findall(r"\d+", text)]

def parse_until(text):
    """the conditions joined by "and", each a small sentence about the FM-1"""
    conds = []
    for clause in re.split(r"\band\b", text):
        c = clause.strip().rstrip("."); l = c.lower(); k = Cond(c); w = l.split()
        if not c: continue
        t = TRACK.get(w[0]); rest = " ".join(w[1:]) if t is not None else l
        def number_test(what, rest, k=k):
            m = re.match(r"(above|below|is not|is|at least|at most)\s+(-?[\d.]+)$", rest)
            if m:
                op, n = m.group(1), float(m.group(2))
                k.fn = {"above": lambda s: s.value(what) > n, "below": lambda s: s.value(what) < n, "is": lambda s: s.value(what) == n, "is not": lambda s: s.value(what) != n,
                        "at least": lambda s: s.value(what) >= n, "at most": lambda s: s.value(what) <= n}[op]; return True
            if rest in ("changed", "has changed"): k.needs_base = what; k.fn = lambda s: s.value(what) != s.base[what]; return True
            return False
        if l in ("playing", "stopped"): k.fn = (lambda s: s.st["playing"]) if l == "playing" else (lambda s: not s.st["playing"])
        elif l in ("recording", "not recording", "rec ready", "song playing", "song recording", "fill on", "punch on", "nothing muted", "nothing solo"):
            k.fn = {"recording": lambda s: s.st["recording"], "not recording": lambda s: not s.st["recording"] and not s.st["armed"], "rec ready": lambda s: s.st["armed"],
                    "song playing": lambda s: s.st["song"], "song recording": lambda s: s.st["song_rec"], "fill on": lambda s: s.st["fill"],
                    "punch on": lambda s: s.st["punch"] is not None, "nothing muted": lambda s: not any(x["mute"] for x in s.st["tracks"]),
                    "nothing solo": lambda s: not any(x["solo"] for x in s.st["tracks"])}[l]
        elif l.startswith("says only "): word = c[10:].strip().upper(); k.fn = lambda s, word=word: (s.st, any(word == m.upper().strip() for m in s.msgs))[1]
        elif l.startswith("says "): word = c[5:].strip().upper(); k.fn = lambda s, word=word: (s.st, any(word in m.upper() for m in s.msgs))[1]
        elif re.match(r"song line (\d+) is pattern (\d)$", l):
            n, pt = int(w[2]), int(w[5]); k.fn = lambda s, n=n, pt=pt: (lambda sg: len(sg) >= n and sg[n - 1][1][0] == pt - 1)(s.fm1.song())
        elif re.match(r"song line (\d+) has (\d+) bars$", l):
            n, b = int(w[2]), int(w[4]); k.fn = lambda s, n=n, b=b: (lambda sg: len(sg) >= n and sg[n - 1][2] == b)(s.fm1.song())
        elif l.startswith("track is ") and w[2] in TRACK: tt = TRACK[w[2]]; k.fn = lambda s, tt=tt: s.st["track"] == tt
        elif l.startswith("page is "):
            name = c[8:].strip().upper()
            if name not in PAGES: raise Bad("no page called %s" % name)
            k.fn = lambda s, p=PAGES[name]: (s.st, s.fm1.page == p)[1]
        elif re.match(r"(pressed|turned|played) ", l):
            m = re.match(r"played (\d+) keys?$", l)
            if m: n = int(m.group(1)); k.fn = lambda s, n=n: (s.st, sum(1 for e in s.events if e[0] == "key") >= n)[1]
            elif w[0] == "pressed" and w[-1] == "twice": b = BTN_NAMES[" ".join(c.split()[1:-1]).upper()]; k.fn = lambda s, b=b: (s.st, ("twice", b) in s.events)[1]
            elif w[0] == "pressed": b = BTN_NAMES[c[8:].strip().upper()]; k.fn = lambda s, b=b: (s.st, ("button", b) in s.events)[1]
            else: e = KNOB_NAMES[c[7:].strip().upper()]; k.fn = lambda s, e=e: (s.st, any(x[0].startswith("knob") and x[1] == e for x in s.events))[1]
        elif l.startswith("tempo "):
            if not number_test(("tempo",), l[6:]): raise Bad("I can't read the condition \"%s\"" % c)
        elif l.startswith(("swing ", "dust ", "duck ", "master filter ")):
            name = "mfilt" if l.startswith("master") else w[0]
            if not number_test(("global", name), " ".join(w[2 if name == "mfilt" else 1:])): raise Bad("I can't read the condition \"%s\"" % c)
        elif l.startswith("song has "):
            if not number_test(("lines",), re.sub(r"(\d+) lines? or more", r"at least \1", l[9:])): raise Bad("I can't read the condition \"%s\"" % c)
        elif re.match(r"(\d+) (patterns|tracks) or more$", l): n = int(w[0]); what = (w[1],); k.fn = lambda s, n=n, what=what: s.value(what) >= n
        elif t is None: raise Bad("I can't read the condition \"%s\"" % c)
        elif re.match(r"has nothing on box", rest): bx = boxes(rest); k.fn = lambda s, t=t, bx=bx: not any(int(round(n[0])) in bx for n in s.pattern(t)[1])
        elif re.match(r"has (\d+) fill notes? or more$", rest): n = int(rest.split()[1]); k.fn = lambda s, t=t, n=n: s.value(("fills", t)) >= n
        elif re.match(r"has (\d+) (hard|soft) notes? or more$", rest):
            n, hard = int(rest.split()[1]), rest.split()[2] == "hard"
            k.fn = lambda s, t=t, n=n, hard=hard: sum(1 for x in s.pattern(t)[1] if (x[3] > 115 if hard else x[3] < 65)) >= n
        elif re.match(r"(longest|softest|loudest) note ", rest):
            if not number_test((w[1], t), " ".join(w[3:])): raise Bad("I can't read the condition \"%s\"" % c)
        elif rest in ("is muted", "is not muted", "is solo", "is not solo"):
            key, want = ("mute" if "muted" in rest else "solo"), "not" not in rest
            k.fn = lambda s, t=t, key=key, want=want: s.st["tracks"][t][key] == want
        elif rest == "is empty": k.fn = lambda s, t=t: not s.pattern(t)[1]
        elif t == TRACK["drums"] and rest.startswith("picked "):                    # the drum that boxes write, as the FM-1 says it
            what = rest[7:].strip().upper()
            if what not in PADS: raise Bad("no drum sound called %s" % what)
            k.fn = lambda s, pad=PADS.index(what): (s.st, s.fm1.picked == pad)[1]
        elif re.match(r"has (\d+) notes? or more$", rest): n = int(rest.split()[1]); k.fn = lambda s, t=t, n=n: len(s.pattern(t)[1]) >= n
        elif re.match(r"has (\d+) notes? or fewer$", rest): n = int(rest.split()[1]); k.fn = lambda s, t=t, n=n: len(s.pattern(t)[1]) <= n
        elif rest.startswith("has no ") and " on box" in rest:
            what, where = rest[7:].split(" on box", 1); what = what.strip().upper(); bx = boxes(where)
            if what not in PADS: raise Bad("no drum sound called %s" % what)
            k.fn = lambda s, t=t, pad=PADS.index(what), bx=bx: not any(n[2] == pad and int(round(n[0])) in bx for n in s.pattern(t)[1])
        elif rest.startswith("has no "):
            what = rest[7:].strip().upper(); pad = PADS.index(what) if what in PADS else None
            if pad is None and what not in ("NOTES", "NOTE"): raise Bad("I can't read the condition \"%s\"" % c)
            k.fn = lambda s, t=t, pad=pad: not any(pad is None or n[2] == pad for n in s.pattern(t)[1])
        elif rest.startswith("has ") and " on box" in rest:
            what, where = rest[4:].split(" on box", 1); what = what.strip().upper(); bx = boxes(where)
            pad = PADS.index(what) if what in PADS else None
            if pad is None and what not in ("NOTES", "A NOTE", "NOTE"):
                pad = midi_of(what)
                if pad is None: raise Bad("I can't read \"%s\" in \"%s\"" % (what, c))
            def missing(s, t=t, pad=pad, bx=bx):
                have = {int(round(n[0])) for n in s.pattern(t)[1] if pad is None or n[2] == pad}
                return [b for b in bx if b not in have]
            k.fn = lambda s, missing=missing: not missing(s)
            if t == TRACK["drums"]: k.miss = missing                             # (SLOOP 3.4) the white keys are boxes on the drum grid only: elsewhere the Do's own keys blink,
            else: k.roll = missing                                                # on NOTES once STEP is on a step that still lacks the note (do_until)
            if what in PADS and t == TRACK["drums"]: k.drum = PADS.index(what)
        elif rest.startswith("has ") and " after box" in rest:
            b = boxes(rest)[0]; k.fn = lambda s, t=t, b=b: any(n[0] >= b for n in s.pattern(t)[1])
        elif rest.startswith("sound is not "): name = c.split(None, 4)[4].upper(); k.fn = lambda s, t=t, name=name: s.sound(t)[0].upper() != name
        elif rest.startswith("sound is "): name = c.split(None, 3)[3].upper(); k.fn = lambda s, t=t, name=name: s.sound(t)[0].upper() == name
        elif rest in ("sound changed", "sound has changed"): k.needs_base = ("sound", t); k.fn = lambda s, t=t: s.sound(t)[0] != s.base[("sound", t)]
        elif re.match(r"on pattern (\d)$", rest): n = int(rest[-1]); k.fn = lambda s, t=t, n=n: s.st["tracks"][t]["cur"] == n - 1 and s.st["tracks"][t]["next"] in (255, n - 1)
        elif re.match(r"pattern (\d) has notes$", rest): n = int(rest.split()[1]); k.fn = lambda s, t=t, n=n: bool(s.st["tracks"][t]["used"] >> (n - 1) & 1)
        elif re.match(r"pattern (\d) is empty$", rest): n = int(rest.split()[1]); k.fn = lambda s, t=t, n=n: not s.st["tracks"][t]["used"] >> (n - 1) & 1
        elif rest.startswith("insert is"):
            name = rest.split()[-1].upper(); neg = " not " in rest
            if name not in INSERTS: raise Bad("no insert called %s" % name)
            k.fn = lambda s, t=t, i=INSERTS.index(name), neg=neg: (s.sound(t)[1][VALUE["INSERT"]] == i) != neg
        else:
            name = w[1]
            if name in ("level", "pan", "chorus", "delay", "reverb", "filter", "steps", "swing", "quant", "gate"): what = ("track", t, name)
            elif name == "notes": what = ("notes", t)
            elif name.upper() in VALUE: what = ("value", t, VALUE[name.upper()])
            else: raise Bad("I can't read the condition \"%s\"" % c)
            if not number_test(what, " ".join(w[2:])): raise Bad("I can't read the condition \"%s\"" % c)
        conds.append(k)
    return conds

# ------------------------------------------------------------------------------------------------ a song for a lesson
def parse_setup(text):
    """"song HOUSE ONE section A, 1 bar; drums empty; track drums; stop" -> [(what, ...)]"""
    out = []
    for clause in text.split(";"):
        c = clause.strip().rstrip("."); l = c.lower(); w = l.split()
        if not c: continue
        if w[0] == "song" and "section" in w:
            m = re.match(r"song (.+?),? section ([a-d])(?:,? (?:the )?(?:first|1) bar|,? bar (\d))?$", l)
            if not m: raise Bad("a song is written \"song HOUSE ONE section A\" (and \", 1 bar\" for its first bar only); found \"%s\"" % c)
            bar = int(m.group(3)) - 1 if m.group(3) else 0 if "bar" in l.split("section")[1] else None
            out.append(("song", m.group(1).replace(" ", "-"), m.group(2).upper(), bar))
        elif w[0] == "pattern" and "section" in w:
            m = re.match(r"pattern (\d):? (?:from )?section ([a-d])(?:,? (?:the )?(?:first|1) bar)?$", l)
            if not m: raise Bad("written \"pattern 2: section C\"; found \"%s\"" % c)
            out.append(("pattern", int(m.group(1)) - 1, m.group(2).upper(), 0 if "bar" in l else None))
        elif w[0] == "new" and w[1] == "project": out.append(("new",))
        elif w[0] == "patterns":                                                  # "patterns A B C D": pattern 1 from section A, 2 from B ...
            out += [("pattern", i, x.upper(), 0 if "bar" in l else None) for i, x in enumerate(x for x in w[1:] if x in "abcd")]
        elif l.startswith("song lines "): out.append(("lines", [(int(x) - 1) for x in w[2:] if x.isdigit()], int(re.search(r"(\d+) bars? each", l).group(1)) if "each" in l else 4))
        elif l in ("song mode on", "song mode off"): out.append(("songmode", w[2] == "on"))
        elif w[0] == "key": out.append(("key", w[1].upper(), w[2] if len(w) > 2 else "minor"))
        elif l == "fill empty tracks": out.append(("fill",))
        elif w[0] == "pick":
            what = c[5:].strip().upper()
            if what not in PADS: raise Bad("no drum sound called %s" % what)
            out.append(("pick", PADS.index(what)))
        elif l.startswith("master filter "): out.append(("mfilt", int(w[2])))
        elif w[0] in ("play", "stop"): out.append((w[0],))
        elif w[0] == "track" and w[1] in TRACK: out.append(("track", TRACK[w[1]]))
        elif w[0] == "tempo": out.append(("tempo", float(w[1])))
        elif w[0] == "swing": out.append(("swing", int(w[1])))
        elif w[0] == "page":
            name = c[5:].strip().upper()
            if name not in PAGES: raise Bad("no page called %s" % name)
            out.append(("page", name))
        elif w[0] in ("mute", "solo"): out.append((w[0], [TRACK[x] for x in w[1:]]))
        elif w[0] == "unmute": out.append(("unmute",))
        elif w[0] == "rec" and w[1] == "off": out.append(("rec off",))
        elif w[0] in TRACK:
            t = TRACK[w[0]]; rest = " ".join(w[1:])
            if rest == "empty": out.append(("empty", t))
            elif re.match(r"(\d+) boxes$", rest): out.append(("steps", t, int(w[1])))
            elif rest.startswith("sound "): out.append(("sound", t, c.split(None, 2)[2].upper()))
            elif rest.startswith("level "): out.append(("level", t, int(w[2])))
            elif re.match(r"(delay|reverb|chorus) \d+$", rest): out.append(("send", t, w[1], int(w[2])))
            elif rest.startswith("fill ") and " on " in rest:
                what, where = c.split(None, 2)[2].rsplit(" on ", 1); what = what.strip().upper()
                if what not in PADS: raise Bad("no drum sound called %s" % what)
                out.append(("add", t, PADS.index(what), boxes(where), 1))
            elif " on " in rest:
                what, where = c.split(None, 1)[1].rsplit(" on ", 1); what = what.strip().upper()
                if t == TRACK["drums"]:
                    if what not in PADS: raise Bad("no drum sound called %s" % what)
                    out.append(("add", t, PADS.index(what), boxes(where)))
                else:
                    m = re.fullmatch(r"([A-G]#?)(\d)", what)
                    if not m: raise Bad("a note is written like A2 or C#4; found \"%s\"" % what)
                    out.append(("add", t, 12 * (int(m.group(2)) + 1) + "C D EF G A B".index(m.group(1)[0]) + (1 if "#" in m.group(1) else 0), boxes(where)))
            else: raise Bad("I can't read the setup \"%s\"" % c)
        else: raise Bad("I can't read the setup \"%s\"" % c)
    return out

def one_bar(song, bar):
    """a song cut down to one of its bars: every track 16 boxes long"""
    for tr in song["tracks"]:
        tr["notes"] = [(s - 16 * bar, min(l, 16 * bar + 16 - s), n, v) for s, l, n, v in tr["notes"] if 16 * bar <= s < 16 * bar + 16]; tr["steps"] = 16
    return song

# ------------------------------------------------------------------------------------------------ playing a lesson
class Hands:
    """a learner's hands on the simulated FM-1 (the tests), or the lesson's own on any FM-1 (SHOW ME)"""
    def __init__(self, fm1, link=None): self.fm1, self.link, self.down = fm1, link, set()
    def let_go(self):
        """whatever is still held, let go (a step can end while a finger is down)"""
        for what, which in sorted(self.down): self.move(what, which, 0)
    def move(self, what, which, value):
        if what == "none": return
        if what != "knob": (self.down.add if value else self.down.discard)((what, which))
        if self.link is not None: {"button": self.link.button, "key": self.link.key, "knob": self.link.knob}[what](which, value)
        elif what == "button": self.fm1.press_button(which, bool(value))
        elif what == "key": self.fm1.play_key(which, bool(value))
        else: self.fm1.turn(which, value)

LAYERS = {BTN[b] for b in ("FX", "SEL", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ")}   # a key pressed with one of these held plays nothing
# (SLOOP 3.4) the drum grid's black keys pick a drum, two of them in turns: the same key again, the other (fw/main.c, drum_black)
DB_KEY, DB_A, DB_B = (1, 3, 5, 8, 10, 13, 15, 17, 20, 22, 25), (0, 2, 7, 3, 4, 5, 9, 11, 12, 13, 15), (1, 8, 7, 3, 6, 5, 10, 11, 12, 14, 15)
def drum_black(k, cur): i = DB_KEY.index(k); return DB_B[i] if cur == DB_A[i] else DB_A[i]
def drum_black_key(pad): return next((k for k, a, b in zip(DB_KEY, DB_A, DB_B) if pad in (a, b)), DB_KEY[0])

class Player:
    """plays lessons through a Teacher (tp/teach.py). robot: a simulated FM-1's link, for a run with no person"""
    def __init__(self, teacher, robot_link=None, fast=False, path="beginner"):
        self.t, self.fm1, self.fast, self.path = teacher, teacher.fm1, fast, path
        self.robot = Hands(self.fm1, robot_link) if robot_link is not None else None
        self.held, self.events, self.msgs, self.shown = set(), [], set(), 0
        self.picked, self.keys_taken, self.clumsy = None, False, False       # the drum that boxes write, as far as the lesson can tell (None: it can't)
        self.hint_s, self.stuck_s = [float(x) for x in os.environ.get("TP_PATIENCE", "25,45").split(",")]   # a hint after this long; then, after the last hint, the offer to show the move
    # ---- small things
    def nap(self, s): time.sleep(min(s, 0.35) if self.fast else s)
    def pump(self):
        """the presses and turns since last time: kept for the step, and the buttons held now"""
        ev = self.fm1.events(); now = time.time()
        for kind, which in list(ev):
            if kind == "button":
                self.held.add(which)
                if which == getattr(self, "_last_b", None) and now - getattr(self, "_last_t", 0) < 0.7: ev.append(("twice", which))   # two quick taps of one button
                self._last_b, self._last_t = which, now
            elif kind == "button_up": self.held.discard(which)
            elif kind == "key":
                self._last_b = None
                if self.fm1.track == TRACK["drums"] and not self.held & LAYERS and not self.keys_taken:   # a drum picked: boxes write that one now
                    grid = self.fm1.page == PAGES["STEPS"]
                    if which in WHITE:
                        if not grid: self.picked = WHITE.index(which)                                     # (SLOOP 3.4: on the grid a white key is a box)
                    elif grid: self.picked = drum_black(which, self.picked)                               # (a black key there picks, a pair in turns)
                    else: self.picked = WHITE.index(max(k for k in WHITE if k < which))
            elif kind in ("knob_right", "knob_left") and which == KNOB["KNOB1"] and self.fm1.track == TRACK["drums"] and self.picked is not None:
                if BTN["SEQ"] in self.held or (not self.held & LAYERS and self.fm1.page in (PAGES["STEPS"], PAGES["DRUM SOUND"])):
                    self.picked = (self.picked + (1 if kind == "knob_right" else -1)) % 16
        self.events += ev
        self.picked = self.fm1.picked                                               # (SLOOP 3.4.2's Teachie Beachie says which drum is picked: its word over the guess above)
        return ev
    def seen(self, base=None): return Seen(self.fm1, base, self.events, self.msgs)
    def lights(self, how, only=False):
        key = (tuple(sorted(how.items())), only)
        if key != getattr(self, "_lit", None): self.fm1.lights(how, only=only); self._lit = key
    def show(self, step, text=None):
        kind, w = step.kind.lower(), words(text if text is not None else step.text)
        left, right = self.answers(step)
        if kind == "card": self.t.say(w, left[0] if left else "", right[0] if right else "")
        elif kind == "big": self.t.say(w, left[0] if left else "", right[0] if right else "", big=True)
        elif kind == "reply": self.fm1.card(w, big=True); self.t.note("reply", w.replace("\n", " / "))
        elif kind.startswith("strip"): self.t.strip(w, where=1 if "top" in kind else 3 if "corner" in kind else 2)
        self.shown += 1; self.t._shot("%02d-%02d" % (self.lesson.number, self.shown))
    def answers(self, step):
        out = []
        for key in ("OCT-", "OCT+"):
            v = step.sub(key)
            if v is None: out.append(None); continue
            label, _, effect = v.partition("→"); out.append((label.strip(), effect.strip()))
        return out
    # ---- the FM-1 made ready
    def goto(self, name):
        """the FM-1 brought to a page of SLOOP, by the same taps a person would use"""
        f, want = self.fm1, PAGES[name]
        f.guide(4)
        if f.page == want: return True
        if f.page != PAGES["TRACKS"]: f.tap(BTN["HOME"]); f.guide(4)
        if name == "TRACKS": return f.page == want
        b = "SEQ" if name in ("STEPS", "NOTES") else "SAVE" if name in ("SONG", "SONG 2") else GROUP.get(name)
        if b is None: raise Bad("I don't know the way to the page %s" % name)
        for _ in range(8):
            f.tap(BTN[b]); time.sleep(0.08); f.guide(4)
            if f.page == want or (name == "SONG 2" and f.page == PAGES["SONG"]): break
        if name == "SONG 2" and f.page == PAGES["SONG"]: f.turn(KNOB["SELECT"], 1); time.sleep(0.1); f.guide(4)
        return f.page == want
    def new_project(self):
        f = self.fm1
        if f.status()["playing"]: f.live(5, 0)
        f.press_button(BTN["HOME"], True); time.sleep(0.5)
        for _ in range(2): f.play_key(WHITE[2], True); time.sleep(0.1); f.play_key(WHITE[2], False); time.sleep(0.25)
        f.press_button(BTN["HOME"], False); time.sleep(0.2)
        f.select(TRACK["bass"]); self.goto("TRACKS")
        f.press_button(BTN["OCT_UP"], True); time.sleep(0.06); f.press_button(BTN["OCT_DOWN"], True); time.sleep(0.06)   # the octave back to the middle
        f.press_button(BTN["OCT_DOWN"], False); f.press_button(BTN["OCT_UP"], False); time.sleep(0.06)
        self.t.note("setup", "a new project")
    def write(self, tr, pat=255):
        f = self.fm1
        if pat == 255: f.sound(tr["track"], tr["sound"], kit=tr.get("kit", False)); f.level(tr["track"], tr["level"])
        n = f.write_pattern(tr["track"], tr["steps"], tr["notes"], pat)
        self.t.note("song", "%-9s %-10s %3d notes%s" % ("track %d" % tr["track"], tr["sound"], n, "" if pat == 255 else "  (pattern %d)" % (pat + 1)))
    def setup(self, text):
        f = self.fm1; kept = set()                # the tracks whose playing pattern is written: made its saved copy (SLOOP 3.4; fw/course.inc, c_setup)
        for op in parse_setup(text):
            if op[0] == "song": kept |= set(range(5))
            elif op[0] in ("empty", "steps", "add", "sound"): kept.add(op[1])
            if op[0] == "new": self.new_project()
            elif op[0] == "song":
                self.new_project(); song = load_spec(op[1], op[2])
                if op[3] is not None: one_bar(song, op[3])
                f.tempo(song["bpm"]); self.song = song
                for tr in song["tracks"]: self.write(tr)
                for tr in song["tracks"]:                                           # each track's sound given to its 8 patterns (SAVE held, KNOB 1)
                    f.select(tr["track"]); f.press_button(BTN["SAVE"], True); time.sleep(0.05); f.turn(KNOB["KNOB1"], 1); time.sleep(0.08); f.press_button(BTN["SAVE"], False); time.sleep(0.05)
                f.select(TRACK["bass"])
            elif op[0] == "pattern":
                song = load_spec(self.song_name(), op[2])
                if op[3] is not None: one_bar(song, op[3])
                for tr in song["tracks"]: self.write(tr, op[1])
                kept |= {t for t, x in enumerate(f.status()["tracks"]) if x["cur"] == op[1]}   # (a pattern not playing is its own saved copy already)
            elif op[0] == "empty": steps = f.pattern(op[1])[0]; f.write_pattern(op[1], steps, [])
            elif op[0] == "steps": steps, notes = f.pattern(op[1]); f.write_pattern(op[1], op[2], [(s, l, n, v, cd) for s, l, n, v, cd, _ in notes if s < op[2]])
            elif op[0] == "add":
                steps, notes = f.pattern(op[1]); vel = 112 if op[1] == TRACK["drums"] else 100; ln = 1 if op[1] == TRACK["drums"] else 2
                cond = op[4] if len(op) > 4 else 0; have = {(int(round(n[0])), n[2]) for n in notes}
                f.write_pattern(op[1], steps, [(s, l, n, v, cd) for s, l, n, v, cd, _ in notes] + [(b, ln, op[2], vel, cond) for b in op[3] if (b, op[2]) not in have])
            elif op[0] == "fill":                                                  # the tracks still empty: the song's own part
                for tr in getattr(self, "song", {}).get("tracks", []):
                    if not f.pattern(tr["track"])[1]: self.write(tr); kept.add(tr["track"])
            elif op[0] == "lines": f.write_song([(pat, op[2]) for pat in op[1]])
            elif op[0] == "songmode":
                if f.status()["song"] != op[1]:
                    f.press_button(BTN["SAVE"], True); time.sleep(0.45); f.play_key(WHITE[12], True); time.sleep(0.1); f.play_key(WHITE[12], False); time.sleep(0.15); f.press_button(BTN["SAVE"], False); time.sleep(0.1)
            elif op[0] == "key":                                                   # (on a new project, whose scale is the chromatic one)
                k = "C C#D D#E F F#G G#A A#B ".index(op[1].ljust(2)) // 2; key = (k - 5) % 12
                f.press_button(BTN["SEL"], True); time.sleep(0.45); f.play_key(key, True); time.sleep(0.1); f.play_key(key, False); time.sleep(0.1)
                f.turn(KNOB["KNOB2"], 2 if op[2].startswith("min") else 1); time.sleep(0.15); f.press_button(BTN["SEL"], False); time.sleep(0.1)
            elif op[0] == "mfilt": f.live(14, 255, 255, op[1] + 100)
            elif op[0] == "pick": self.pick(op[1])
            elif op[0] == "send": f.live(4, op[1], dict(chorus=2, delay=3, reverb=4)[op[2]], round(op[3] * 127 / 100))
            elif op[0] == "track": f.select(op[1])
            elif op[0] == "tempo": f.tempo(op[1])
            elif op[0] == "swing": b = int(round(f.status()["tempo"] * 10)); f.live(13, b & 255, b >> 8, op[1])
            elif op[0] == "page": self.goto(op[1])
            elif op[0] == "play": f.live(5, 1)
            elif op[0] == "stop": f.live(5, 0)
            elif op[0] in ("mute", "solo"):
                for t in op[1]: f.live(4, t, 6 if op[0] == "mute" else 7, 1)
            elif op[0] == "unmute":
                for t in range(5): f.live(4, t, 6, 0); f.live(4, t, 7, 0)
            elif op[0] == "sound": f.sound(op[1], op[2], kit=op[1] == TRACK["drums"])
            elif op[0] == "level": f.level(op[1], round(op[2] * 127 / 100))
            elif op[0] == "rec off":
                st = f.status()
                if st["recording"] or st["armed"]: f.tap(BTN["REC"])
        if kept: f.keep(kept)
        self.t.note("setup", text)
    def pick(self, pad):
        """the drum that boxes write, chosen from here: its key played with the drum track turned right down for a moment"""
        f = self.fm1; st = f.status(); was, lvl = st["track"], st["tracks"][TRACK["drums"]]["level"]; muted = st["tracks"][TRACK["drums"]]["mute"]
        f.guide(4); grid = f.page == PAGES["STEPS"]                                 # (SLOOP 3.4: on the drum grid a white key writes a box)
        if grid: self.goto("TRACKS")
        f.select(TRACK["drums"]); f.live(4, TRACK["drums"], 0, 0); time.sleep(0.08)
        f.play_key(WHITE[pad], True); time.sleep(0.06); f.play_key(WHITE[pad], False); time.sleep(0.25)
        f.live(4, TRACK["drums"], 0, round(lvl * 127 / 100)); f.select(was)
        if muted: f.live(4, TRACK["drums"], 6, 1)
        if grid: self.goto("STEPS")
        self.picked = pad; self.t.note("picked", PADS[pad])
    def song_name(self):
        m = re.search(r"song (.+?),? section", " ".join(s.text for s in self.lesson.steps if s.kind.lower() == "setup").lower())
        return m.group(1).replace(" ", "-") if m else "house-one"
    # ---- waiting for the learner
    def do_until(self, step):
        """the step's Do pointed at, until its Until is true (or, with no Until, until the Do has been seen)"""
        ops = parse_do(step.sub("Do") or "", self.lesson.phrases); conds = parse_until(step.sub("Until") or "")
        robot_ops = parse_do(step.sub("Robot"), self.lesson.phrases) if step.sub("Robot") else ops
        idle = any(o[0] == "idle" for o in robot_ops); robot_ops = [o for o in robot_ops if o[0] != "idle"]
        need = next((c.drum for c in conds if c.drum is not None), None)          # the drum this step's boxes are for
        hints, page = step.all("Hint"), step.sub("Page")
        base = {c.needs_base: None for c in conds if c.needs_base}
        if base: s0 = self.seen(); base = {k: s0.value(k) for k in base}
        t0 = time.time(); at = 0; turned = 0; hint_at = t0 + self.hint_s; hint_i = 0; robot = None; robot_due = t0 + 0.5; off_page = None; self.events = []; self.msgs.clear()
        said = None                                                                # the wrong drum last said
        if conds and all(c.ok(self.seen(base)) for c in conds) and not any(c.needs_base for c in conds):
            on = lambda: (lambda st: bool(st["fill"] or st["punch"] is not None))(self.seen().st)
            if not on(): self.t.note("already", step.sub("Until")); return
            self.t.note("held", "a fill or an effect still held from the step before: let go first")
            while on(): self.pump(); time.sleep(0.05)
            t0 = time.time(); hint_at = t0 + self.hint_s; robot_due = t0 + 0.5; self.events = []; self.msgs.clear()
        if need is not None:
            self.pump()
            if self.fm1.track != TRACK["drums"]: self.fm1.select(TRACK["drums"]); self.t.note("back", "to the drum track")
            if self.picked != need: self.pick(need)                                 # (whatever was played before: the right drum now)
            if self.robot and self.clumsy and not idle:                             # a learner who taps a blinking box before holding SEQ
                miss = [b for c in conds if c.miss for b in c.miss(self.seen(base))]
                if miss: robot_ops = [("tap", WHITE[miss[-1] % 16]), ("wait", 3.2)] + robot_ops
        while True:
            for ev in self.pump():
                if os.environ.get("TP_DEBUG"): print("      event", ev, "%.2f" % (time.time() - t0), flush=True)
                if conds or at >= len(ops): continue
                op = ops[at]                                                        # (no Until: the Do itself, in order)
                hit = (op[0] == "press" and ev == ("button", op[1])) or (op[0] == "hold" and ev == ("button", op[1])) or \
                      (op[0] in ("tap", "long", "khold") and ev == ("key", op[1])) or (op[0] == "any" and ev[0] == "key") or \
                      (op[0] == "turn" and ev == ("knob_right" if op[2] > 0 else "knob_left", op[1])) or (op[0] == "letgo" and ev[0] == "button_up")
                if hit and op[0] == "turn":
                    turned += 1
                    if turned >= abs(op[2]): at += 1; turned = 0
                elif hit: at += 1
                elif ev[0] in ("key", "button") and not (ev[0] == "button" and any(o[0] == "hold" and o[1] == ev[1] for o in ops)): self.t.note("other press", "%s %d" % ev)
            while not conds and at < len(ops) and (ops[at][0] == "wait" or (ops[at][0] == "hold" and ops[at][1] in self.held)): at += 1
            s = self.seen(base)
            if conds:
                if all(c.ok(s) for c in conds): break
            elif at >= len(ops): break
            # the lights: the buttons to hold, and what is still to press
            how = {}
            if conds:
                miss = [b for c in conds if c.miss for b in c.miss(s)]
                roll = [b for c in conds if c.roll for b in c.roll(s)]              # (SLOOP 3.4) NOTES, and a step still lacks its note: the key waits
                wait = bool(roll) and self.fm1.page == PAGES["NOTES"] and self.fm1.track != TRACK["drums"] and self.fm1.step not in roll   # until [KNOB1] has brought STEP there (fw/course.inc says the same)
                for op in ops:
                    if op[0] in ("hold", "press"): how[op[1]] = BLINK
                    elif op[0] in ("tap", "long", "khold") and not any(c.miss for c in conds) and not wait: how[light_of_key(op[1])] = BLINK
                for b in miss: how[light_of_key(WHITE[b % 16])] = BLINK
                if need is not None and self.picked != need:                       # (SLOOP 3.4) another drum picked than the boxes ask for:
                    how = {l: v for l, v in how.items() if l < 14}                  # its black key blinks, not the boxes, until it is picked
                    how[light_of_key(drum_black_key(need))] = BLINK
            else:
                for op in ops[:at]:
                    if op[0] == "hold": how[op[1]] = BLINK
                    elif op[0] == "khold": how[light_of_key(op[1])] = BLINK
                op = ops[at]
                if op[0] in ("hold", "press"): how[op[1]] = BLINK
                elif op[0] in ("tap", "long", "khold") or (op[0] == "any" and op[1] is not None): how[light_of_key(op[1])] = BLINK
            how.update(self.follow(step, s))
            self.lights(how, only=step.kind.lower() in ("card", "big"))
            if conds and need is not None and self.picked != need:                 # another drum picked: said once for each (fw/course.inc says the same)
                if said != self.picked:
                    said = self.picked; self.show(step, "Another drum is picked. / Tap the blinking key."); self.t.note("slip", "picked %s" % PADS[self.picked])
                    end = time.time() + (1.2 if self.fast else 2.4)
                    while time.time() < end: self.pump(); time.sleep(0.05)
                    self.show(step); hint_at = time.time() + self.hint_s
            else: said = None
            now = time.time()
            if page and not self.held:                                             # wandered to another page: brought back
                s.st
                if self.fm1.page != PAGES[page.upper()]:
                    off_page = off_page or now
                    if now - off_page > 2.5: self.goto(page.upper()); self.t.note("back", "to %s" % page); off_page = None
                else: off_page = None
            if self.robot and robot is None and now >= robot_due: robot = moves(robot_ops if not conds else self.left_to_do(robot_ops, conds, s)); robot_due = now
            if self.robot and robot is not None and now >= robot_due:
                mv = next(robot, None)
                if mv is None: robot = None; robot_due = now + (0.3 if any(o[0] == "turn" for o in robot_ops) else 2.0)
                else:
                    self.robot.move(*mv[:3]); robot_due = now + mv[3]
                    if os.environ.get("TP_DEBUG"): print("      robot", mv, "%.2f" % (now - t0), flush=True)
            if (not self.robot or idle) and hint_i < len(hints) and now >= hint_at:
                self.show(step, hints[hint_i]); self.t.note("hint", "%d" % (hint_i + 1)); hint_i += 1; hint_at = now + self.hint_s
            elif (not self.robot or idle) and hint_i >= len(hints) and now >= hint_at + self.stuck_s - self.hint_s and ops:
                if self.stuck(step, ops, conds, need, base): break                  # shown: the step is over, whatever came of it
                hint_at = time.time() + self.hint_s; self.show(step, hints[-1] if hints else None)
            if now - t0 > (60 if self.robot else 24 * 3600): raise TimeoutError("lesson %d step %d: never done: %s" % (self.lesson.number, step.n, step.sub("Until") or step.sub("Do")))
            time.sleep(0.03 if self.robot else 0.06)
        if self.robot: time.sleep(0.1); self.robot.let_go(); time.sleep(0.12)
        self.lights({}); self._lit = None; self.fm1.lights()
        self.t.note("done", "%s  (%.1f s)" % (step.sub("Until") or step.sub("Do"), time.time() - t0))
    def follow(self, step, s):
        """Follow: one key for each bar in turn, blinking while its bar plays (a part to play along with, in time)"""
        if not step.sub("Follow"): return {}
        keys = keys_of(step.sub("Follow"), self.lesson.phrases)
        return {light_of_key(keys[s.st["bar"] % len(keys)]): BLINK} if s.st["playing"] else {light_of_key(keys[0]): BLINK}
    def left_to_do(self, ops, conds, s):
        """the Do without the boxes that are already right (a tap there would take the note out again)"""
        miss = {WHITE[b % 16] for c in conds if c.miss for b in c.miss(s)}
        if not any(c.miss for c in conds): return ops
        return [op for op in ops if op[0] not in ("tap", "long", "khold") or op[1] in miss]
    def stuck(self, step, ops, conds, need=None, base=None):
        """a long time and every hint used: the lesson offers to do the move itself. Shown, the step is over: the move is
        made as a learner would make it, and what that leaves undone in a drum's boxes is written in"""
        f = self.fm1; f.take(buttons=[BTN["OCT_DOWN"], BTN["OCT_UP"]])
        self.t.say("Stuck?\nI can do this step for you.", "No", "Show me"); self.t.point([BTN["OCT_DOWN"], BTN["OCT_UP"]])
        ev = self.t.wait_for({("button", BTN["OCT_DOWN"]), ("button", BTN["OCT_UP"])}); f.take(); f.lights(); self.held.clear()
        if ev[1] == BTN["OCT_DOWN"]: return False
        self.show(step); hands = Hands(f); todo = self.left_to_do(ops, conds, self.seen(base)) if conds else ops
        if need is not None:                                                        # first the drum: boxes write the one picked (not on the grid: there a white key is a box)
            if f.track != TRACK["drums"]: f.select(TRACK["drums"])
            if self.picked != need: self.pick(need)
        for mv in moves(todo, tap=0.25, gap=0.45):
            if mv[0] != "none": self.lights({(mv[1] if mv[0] == "button" else light_of_key(mv[1])): 4} if mv[0] != "knob" else {})
            hands.move(*mv[:3]); time.sleep(mv[3])
        hands.let_go(); self.lights({}); time.sleep(0.3); s = self.seen(base); ok = all(c.ok(s) for c in conds)
        if not ok and need is not None:                                             # still not there: the notes written straight in
            for c in conds:
                if c.drum is None or not c.miss(s): continue
                steps, notes = f.pattern(TRACK["drums"]); have = {(int(round(n[0])), n[2]) for n in notes}
                f.write_pattern(TRACK["drums"], steps, [(a, l, n, v, cd) for a, l, n, v, cd, _ in notes] + [(b, 1, c.drum, 100, 0) for b in c.miss(s) if (b, c.drum) not in have])
            ok = all(c.ok(self.seen(base)) for c in conds)
        self.t.note("shown", "%s%s" % (step.sub("Do"), "" if ok or not conds else "  (and the lesson still does not see: %s)" % step.sub("Until"))); return True
    def answer(self, step):
        """a question on the OCT buttons. With an Until, the answer counts only once it is true"""
        left, right = self.answers(step); conds = parse_until(step.sub("Until") or ""); f = self.fm1
        base = {c.needs_base: None for c in conds if c.needs_base}
        if base: s0 = self.seen(); base = {k: s0.value(k) for k in base}
        robot_ops = parse_do(step.sub("Robot") or "", self.lesson.phrases)
        choice = next((o[1] for o in robot_ops if o[0] == "answer"), "right" if right else "left"); robot_ops = [o for o in robot_ops if o[0] != "answer"]
        btns = [b for b, a in ((BTN["OCT_DOWN"], left), (BTN["OCT_UP"], right)) if a]
        f.take(buttons=btns); self.events = []; self.msgs.clear(); t0 = time.time(); robot = moves(robot_ops); robot_due = t0 + 0.5; page = step.sub("Page"); off_page = None
        base_how = {b: BLINK for b in btns}
        for k in keys_of(step.sub("Lights") or "", self.lesson.phrases): base_how[light_of_key(k)] = 2
        while True:
            how = dict(base_how)
            if step.sub("Follow"): how.update(self.follow(step, self.seen(base)))
            self.lights(how, only=step.kind.lower() in ("card", "big"))
            got = None
            for ev in self.pump():
                if ev[0] == "button" and ev[1] in btns: got = ev[1]
            if got is not None:
                s = self.seen(base)
                if all(c.ok(s) for c in conds): break
                hint = step.sub("Hint") or "Not yet."
                self.show(step, hint); self.t.note("not yet", step.sub("Until")); self.nap(6.0); self.show(step)
            now = time.time()
            if page and not self.held:
                f.guide(4)
                if f.page != PAGES[page.upper()]:
                    off_page = off_page or now
                    if now - off_page > 2.5: self.goto(page.upper()); self.t.note("back", "to %s" % page); off_page = None
                else: off_page = None
            if self.robot and now >= robot_due:
                mv = next(robot, None) if robot is not None else None
                if mv is not None: self.robot.move(*mv[:3]); robot_due = now + mv[3]
                else:
                    robot = None; b = BTN["OCT_DOWN"] if choice == "left" and left else BTN["OCT_UP"]
                    self.robot.move("button", b, 1); time.sleep(0.09); self.robot.move("button", b, 0); robot_due = now + 1.5
            if now - t0 > (60 if self.robot else 24 * 3600): raise TimeoutError("lesson %d step %d: no answer" % (self.lesson.number, step.n))
            time.sleep(0.03)
        if self.robot: time.sleep(0.1); self.robot.let_go(); time.sleep(0.1)
        f.take(); f.lights(); self._lit = None; self.held.clear()
        which = left if got == BTN["OCT_DOWN"] else right
        self.t.note("answer", "%s  (%.1f s)" % (which[0], time.time() - t0))
        return which[1]
    def phrase(self, text):
        w = text.split(); speed = next((SPEED[x.lower()] for x in w if x.lower() in SPEED), 0.38)
        return keys_of(" ".join(x for x in w if x.lower() not in SPEED), self.lesson.phrases), speed
    def echo(self, keys):
        for k in keys:
            self.lights({light_of_key(k): BLINK}, only=True); t0 = time.time(); due = t0 + 0.4
            while True:
                ev = self.pump()
                if ("key", k) in ev: break
                for e in ev:
                    if e[0] == "key": self.t.note("other press", "key %d" % e[1])
                if self.robot and time.time() >= due: self.robot.move("key", k, 1); time.sleep(0.09); self.robot.move("key", k, 0); due = time.time() + 1.5
                if time.time() - t0 > (30 if self.robot else 24 * 3600): raise TimeoutError("nobody played key %d" % k)
                time.sleep(0.02)
        self.lights({}, only=True); self._lit = None
    def wait(self, text):
        m = re.match(r"until bar (\d+)", text.lower())
        if m:
            end = time.time() + (1.5 if self.fast else 180)
            while time.time() < end and self.fm1.status()["bar"] < int(m.group(1)) - 1: self.pump(); time.sleep(0.05)
            return
        m = re.match(r"([\d.]+)\s*(second|bar|beat)", text.lower())
        if not m: raise Bad("a wait is written \"4 seconds\" or \"2 bars\"; found \"%s\"" % text)
        n = float(m.group(1)); beat = 60.0 / max(40.0, self.fm1.status()["tempo"])
        s = n if m.group(2) == "second" else n * 4 * beat if m.group(2) == "bar" else n * beat
        end = time.time() + (min(s, 1.5) if self.fast else s)
        while time.time() < end: self.pump(); time.sleep(0.05)
    # ---- a step, a lesson
    def step(self, step):
        kind = step.kind.lower()
        if "beginners only" in step.tags and self.path != "beginner": self.t.note("skipped", "step %d (beginners only)" % step.n); return None
        if kind == "setup": self.setup(step.text); return None
        if kind == "wait": self.wait(step.text); return None
        if kind not in KINDS: raise Bad("lesson %d step %d: no kind of step called %s" % (self.lesson.number, step.n, step.kind))
        if step.sub("Keys") == "silent": self.fm1.take(keys=range(27)); self.keys_taken = True
        if kind == "play": keys, speed = self.phrase(step.text); self.t.play(keys, speed); self._lit = None
        elif kind == "echo": self.echo(self.phrase(step.text)[0])
        else: self.show(step)
        if kind in ("card", "big", "reply") and not (step.sub("Do") or step.sub("OCT+") or step.sub("OCT-")): self.lights({}, only=True)
        effect = None
        if step.sub("Play"): keys, speed = self.phrase(step.sub("Play")); self.t.play(keys, speed); self._lit = None
        if step.sub("Echo"): self.echo(self.phrase(step.sub("Echo"))[0])
        if step.sub("OCT+") or step.sub("OCT-"): effect = self.answer(step)
        elif step.sub("Do") or step.sub("Until"): self.do_until(step)
        if step.sub("Wait"): self.wait(step.sub("Wait"))
        elif kind == "reply": self.nap(1.2)
        if step.sub("Keys") == "silent": self.fm1.take(); self.keys_taken = False
        return effect
    def play(self, lesson):
        """the lesson from its first card to its last. Returns "replay", "next" or "lessons" """
        self.lesson, self.shown, t, f = lesson, 0, self.t, self.fm1
        t.note("lesson", "%d %s" % (lesson.number, lesson.title))
        i = 0
        while i < len(lesson.steps):
            step = lesson.steps[i]; effect = self.step(step); i += 1
            for e in [x.strip() for x in (effect or "").split(";") if x.strip()]:
                m = re.match(r"go to (\d+)$", e)
                if m: i = int(m.group(1)) - 1
                elif e in ("right", "wrong"):
                    self.fm1.lights({}, only=True); self.fm1.card(words(step.sub(e.capitalize()) or ("Yes." if e == "right" else "Not quite.")), big=len(step.sub(e.capitalize()) or "") < 24)
                    self.t.note(e, ""); self.nap(1.4 if e == "right" else 3.0)
                else: self.setup(e[6:].strip() if e.startswith("setup:") else e)
        f.take(); f.lights()
        if f.status()["playing"] and (lesson.head.get("Ends") or "").lower() != "playing": f.live(5, 0)
        got = t.ask("_Lesson %d\nDone" % lesson.number, "Replay", "Next", big=True)
        return "replay" if got == "left" else "next"
