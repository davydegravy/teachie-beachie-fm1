#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: the lessons packed for the FM-1 itself (the course with no computer).

    python3 tools/mkcourse.py [OUT.bin] [FIRST-LAST]     e.g. tools/mkcourse.py build/course.bin 1-7

Reads tutorial/lessons/NN-name.md (the same files the computer plays) and their songs, and writes one pack: the
lessons' words and what each step asks for, as small numbered instructions the firmware reads (fw/main.c, "the
course"). The numbers are kept in one place, here; `--header` writes them as app/tp_course.h for the firmware.

The pack (every number little-endian):
    "TPC2"  u16 size  u8 lessons  u8 clip sets  u16 lesson offsets...  u16 clip-set offsets...  u16 small font, u16 large font
    (the two fonts last, each from an even byte: assets/fonts/lessons-font.bin, made by tools/mklessonfont.py)
  a lesson:    u8 number, u8 steps, its title, 0, then its steps
  a step:      u16 the bytes that follow, u8 kind, u8 flags (1: beginners only), then fields: tag, length, bytes
  a clip set (a song's section, or one bar of it, ready to go into patterns):
               u16 tempo x 10, then 5 tracks in the firmware's order: the sound's name, 0, u8 kit, u8 level 0..127,
               then for each of the 5 tracks: u16 steps, u16 notes, the notes (8 bytes each: tick u16, length u16,
               note or pad, velocity, condition, 1)
  Setup's instructions, an answer's effects and Until's conditions are each: length (of what follows), number, values.
  Do's moves are 3 bytes each: number, which, how much.
In a sample slot the pack sits behind a 512-byte header: "TPCO", u16 1, zeros, u32 size at 16, u32 CRC-32 at 20."""
import json, os, re, struct, sys, zlib
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "tutorial"))
from tp import lesson as L
from tp.song import load_spec, TRACK

KIND = dict(setup=0, card=1, big=2, strip=3, **{"strip top": 4, "strip corner": 5}, reply=6, wait=7, play=8, echo=9)
TAG = dict(TEXT=1, LEFT=2, RIGHT=3, DO=4, UNTIL=5, HINT=6, PLAY=7, ECHO=8, WAIT=9, SILENT=10, PAGE=11, FOLLOW=12, SETUP=13, RIGHTTXT=14, WRONGTXT=15, LIGHTS=16)
EFF = dict(GOTO=1, RIGHT=2, WRONG=3, SETUP=4)
DO = dict(PRESS=1, HOLD=2, TAP=3, LONG=4, KHOLD=5, TURN=6, ANY=7, LETGO=8)
SET = dict(NEW=1, SONG=2, PATTERN=3, EMPTY=4, STEPS=5, ADD=6, TRACK=7, TEMPO=8, SWING=9, PAGE=10, PLAY=11, STOP=12, MUTE=13, SOLO=14, UNMUTE=15,
           SOUND=16, LEVEL=17, PICK=18, KEY=19, FILLEMPTY=20, LINES=21, SONGMODE=22, MFILT=23, SEND=24, RECOFF=25)
CON = dict(PLAYING=1, STOPPED=2, RECORDING=3, NOTREC=4, ARMED=5, SONGPLAY=6, SONGREC=7, FILLON=8, PUNCHON=9, NOMUTE=10, NOSOLO=11, TRACKIS=12, PAGEIS=13,
           MUTED=14, SOLOED=15, HAS=16, HASNO=17, EMPTY=18, NUM=19, SOUNDCHG=20, SOUNDIS=21, INSERT=22, ONPAT=23, PATHAS=24, SAYS=25, SAYSONLY=26,
           PRESSED=27, TWICE=28, TURNED=29, LINEPAT=30, LINEBARS=31)
NUM = dict(TEMPO=0, SWING=1, MFILT=2, DUST=3, DUCK=4, TRK=5, VALUE=6, NOTES=7, LONGEST=8, SOFTEST=9, LOUDEST=10, FILLS=11, HARD=12, SOFT=13, PATTERNS=14, TRACKS=15, LINES=16, PLAYED=17)
PROP = dict(level=0, pan=1, chorus=2, delay=3, reverb=4, filter=5, steps=6, swing=7, quant=8, gate=9)
CMP = {"above": 0, "below": 1, "is": 2, "is not": 3, "at least": 4, "at most": 5, "changed": 6}
WAIT = dict(second=0, bar=1, beat=2, until=3)

class Cant(Exception): pass
def s0(text): return text.encode("ascii") + b"\0"
def rec(num, *vals): body = bytes([num]) + b"".join(v if isinstance(v, bytes) else bytes([v & 255]) for v in vals); return bytes([len(body)]) + body
def mask(boxes):
    if any(b > 15 for b in boxes): raise Cant("a box past 16")
    m = sum(1 << b for b in set(boxes)); return bytes([m & 255, m >> 8])
def i16(v): return struct.pack("<h", int(round(v)))

class Pack:
    def __init__(self): self.clips, self.clip_id = [], {}
    def clip(self, song, section, bar):
        key = (song, section, bar)
        if key not in self.clip_id:
            sg = load_spec(song, section)
            if bar is not None: L.one_bar(sg, bar)
            b = struct.pack("<H", int(round(sg["bpm"] * 10)))
            tracks = sorted(sg["tracks"], key=lambda t: t["track"])
            assert [t["track"] for t in tracks] == [0, 1, 2, 3, 4]
            for t in tracks: b += s0(t["sound"]) + bytes([1 if t.get("kit") else 0, t["level"]])
            for t in tracks:
                notes = [(int(round(n[0] * 96)), max(12, int(round(n[1] * 96))), n[2], n[3]) for n in t["notes"]]
                b += struct.pack("<HH", t["steps"], len(notes)) + b"".join(struct.pack("<HHBBBB", a, l, n, v, 0, 1) for a, l, n, v in notes)
            self.clip_id[key] = len(self.clips); self.clips.append(b)
        return self.clip_id[key]
    # ---- Setup
    def setup(self, text, song):
        out = b""
        for op in L.parse_setup(text):
            o = op[0]
            if o == "new": out += rec(SET["NEW"])
            elif o == "song": out += rec(SET["SONG"], self.clip(op[1], op[2], op[3]))
            elif o == "pattern": out += rec(SET["PATTERN"], op[1], self.clip(song, op[2], op[3]))
            elif o == "empty": out += rec(SET["EMPTY"], op[1])
            elif o == "steps": out += rec(SET["STEPS"], op[1], op[2])
            elif o == "add": out += rec(SET["ADD"], op[1], op[2], 112 if op[1] == TRACK["drums"] else 100, op[4] if len(op) > 4 else 0, mask(op[3]))
            elif o == "track": out += rec(SET["TRACK"], op[1])
            elif o == "tempo": out += rec(SET["TEMPO"], struct.pack("<H", int(round(op[1] * 10))))
            elif o == "swing": out += rec(SET["SWING"], op[1])
            elif o == "page": out += rec(SET["PAGE"], L.PAGES[op[1]])
            elif o in ("play", "stop"): out += rec(SET[o.upper()])
            elif o in ("mute", "solo"): out += rec(SET[o.upper()], sum(1 << t for t in op[1]))
            elif o == "unmute": out += rec(SET["UNMUTE"])
            elif o == "sound": out += rec(SET["SOUND"], op[1], s0(op[2]))
            elif o == "level": out += rec(SET["LEVEL"], op[1], round(op[2] * 127 / 100))
            elif o == "pick": out += rec(SET["PICK"], op[1])
            elif o == "key": out += rec(SET["KEY"], "C C#D D#E F F#G G#A A#B ".index(op[1].ljust(2)) // 2, 2 if op[2].startswith("min") else 1)
            elif o == "fill": out += rec(SET["FILLEMPTY"])
            elif o == "lines": out += rec(SET["LINES"], len(op[1]), op[2], bytes(op[1]))
            elif o == "songmode": out += rec(SET["SONGMODE"], int(op[1]))
            elif o == "mfilt": out += rec(SET["MFILT"], op[1])
            elif o == "send": out += rec(SET["SEND"], op[1], dict(chorus=0, delay=1, reverb=2)[op[2]], round(op[3] * 127 / 100))
            elif o == "rec off": out += rec(SET["RECOFF"])
            else: raise Cant("setup: %s" % (op,))
        return out
    # ---- Do
    def do(self, text, phrases):
        out = b""
        for op in L.parse_do(text, phrases):
            o = op[0]
            if o in ("press", "hold"): out += bytes([DO[o.upper()], op[1], 0])
            elif o in ("tap", "long", "khold"): out += bytes([DO[o.upper()], op[1], 0])
            elif o == "turn": out += bytes([DO["TURN"], op[1], op[2] & 255])
            elif o == "any": out += bytes([DO["ANY"], 255 if op[1] is None else op[1], 0])
            elif o == "letgo": out += bytes([DO["LETGO"], 0, 0])
            elif o in ("wait", "idle", "answer"): pass                       # (the robot's only)
            else: raise Cant("do: %s" % (op,))
        return out
    # ---- Until
    def until(self, text):
        out = b""
        for clause in re.split(r"\band\b", text):
            c = clause.strip().rstrip("."); l = c.lower(); w = l.split()
            if not c: continue
            t = TRACK.get(w[0]); rest = " ".join(w[1:]) if t is not None else l
            def num(what, a, b, rest):
                m = re.match(r"(above|below|is not|is|at least|at most)\s+(-?[\d.]+)$", rest)
                if m: return rec(CON["NUM"], what, a, b, CMP[m.group(1)], i16(float(m.group(2)) * (10 if what in (NUM["LONGEST"],) else 1)))
                if rest in ("changed", "has changed"): return rec(CON["NUM"], what, a, b, CMP["changed"], i16(0))
                raise Cant("until: %s" % c)
            simple = {"playing": "PLAYING", "stopped": "STOPPED", "recording": "RECORDING", "not recording": "NOTREC", "rec ready": "ARMED", "song playing": "SONGPLAY",
                      "song recording": "SONGREC", "fill on": "FILLON", "punch on": "PUNCHON", "nothing muted": "NOMUTE", "nothing solo": "NOSOLO"}
            if l in simple: out += rec(CON[simple[l]])
            elif l.startswith("says only "): out += rec(CON["SAYSONLY"], s0(c[10:].strip().upper()))
            elif l.startswith("says "): out += rec(CON["SAYS"], s0(c[5:].strip().upper()))
            elif re.match(r"song line (\d+) is pattern (\d)$", l): out += rec(CON["LINEPAT"], int(w[2]) - 1, int(w[5]) - 1)
            elif re.match(r"song line (\d+) has (\d+) bars$", l): out += rec(CON["LINEBARS"], int(w[2]) - 1, int(w[4]))
            elif l.startswith("track is ") and w[2] in TRACK: out += rec(CON["TRACKIS"], TRACK[w[2]])
            elif l.startswith("page is "): out += rec(CON["PAGEIS"], L.PAGES[c[8:].strip().upper()])
            elif re.match(r"played (\d+) keys?$", l): out += rec(CON["NUM"], NUM["PLAYED"], 0, 0, CMP["at least"], i16(int(w[1])))
            elif w[0] == "pressed" and w[-1] == "twice": out += rec(CON["TWICE"], L.BTN_NAMES[" ".join(c.split()[1:-1]).upper()])
            elif w[0] == "pressed": out += rec(CON["PRESSED"], L.BTN_NAMES[c[8:].strip().upper()])
            elif w[0] == "turned": out += rec(CON["TURNED"], L.KNOB_NAMES[c[7:].strip().upper()])
            elif l.startswith("tempo "): out += num(NUM["TEMPO"], 0, 0, l[6:])
            elif l.startswith(("swing ", "dust ", "duck ")): out += num(NUM[w[0].upper()], 0, 0, " ".join(w[1:]))
            elif l.startswith("master filter "): out += num(NUM["MFILT"], 0, 0, " ".join(w[2:]))
            elif l.startswith("song has "): out += num(NUM["LINES"], 0, 0, re.sub(r"(\d+) lines? or more", r"at least \1", l[9:]))
            elif re.match(r"(\d+) (patterns|tracks) or more$", l): out += rec(CON["NUM"], NUM[w[1].upper()], 0, 0, CMP["at least"], i16(int(w[0])))
            elif t is None: raise Cant("until: %s" % c)
            elif rest in ("is muted", "is not muted"): out += rec(CON["MUTED"], t, int("not" not in rest))
            elif rest in ("is solo", "is not solo"): out += rec(CON["SOLOED"], t, int("not" not in rest))
            elif rest == "is empty": out += rec(CON["EMPTY"], t)
            elif re.match(r"has nothing on box", rest): out += rec(CON["HASNO"], t, 255, mask(L.boxes(rest)))
            elif re.match(r"has (\d+) notes? or more$", rest): out += rec(CON["NUM"], NUM["NOTES"], t, 0, CMP["at least"], i16(int(rest.split()[1])))
            elif re.match(r"has (\d+) notes? or fewer$", rest): out += rec(CON["NUM"], NUM["NOTES"], t, 0, CMP["at most"], i16(int(rest.split()[1])))
            elif re.match(r"has (\d+) (fill|hard|soft) notes? or more$", rest):
                out += rec(CON["NUM"], NUM[dict(fill="FILLS", hard="HARD", soft="SOFT")[rest.split()[2]]], t, 0, CMP["at least"], i16(int(rest.split()[1])))
            elif re.match(r"(longest|softest|loudest) note ", rest): out += num(NUM[w[1].upper()], t, 0, " ".join(w[3:]))
            elif rest.startswith("has no ") and " on box" in rest:
                what, where = rest[7:].split(" on box", 1); out += rec(CON["HASNO"], t, L.PADS.index(what.strip().upper()), mask(L.boxes(where)))
            elif rest.startswith("has no "):
                what = rest[7:].strip().upper(); out += rec(CON["HASNO"], t, L.PADS.index(what) if what in L.PADS else 255, mask(range(16)))
            elif rest.startswith("has ") and " on box" in rest:
                what, where = rest[4:].split(" on box", 1); what = what.strip().upper()
                note = L.PADS.index(what) if what in L.PADS else 255 if what in ("NOTES", "A NOTE", "NOTE") else L.midi_of(what)
                if note is None: raise Cant("until: %s" % c)
                out += rec(CON["HAS"], t, note, mask(L.boxes(where)))
            elif rest.startswith("sound is not "): out += rec(CON["SOUNDIS"], t, 1, s0(c.split(None, 4)[4].upper()))
            elif rest.startswith("sound is "): out += rec(CON["SOUNDIS"], t, 0, s0(c.split(None, 3)[3].upper()))
            elif rest in ("sound changed", "sound has changed"): out += rec(CON["SOUNDCHG"], t)
            elif re.match(r"on pattern (\d)$", rest): out += rec(CON["ONPAT"], t, int(rest[-1]) - 1)
            elif re.match(r"pattern (\d) has notes$", rest): out += rec(CON["PATHAS"], t, int(rest.split()[1]) - 1, 1)
            elif re.match(r"pattern (\d) is empty$", rest): out += rec(CON["PATHAS"], t, int(rest.split()[1]) - 1, 0)
            elif rest.startswith("insert is"): out += rec(CON["INSERT"], t, L.INSERTS.index(rest.split()[-1].upper()), int(" not " in rest))
            elif w[1] in PROP: out += num(NUM["TRK"], t, PROP[w[1]], " ".join(w[2:]))
            elif w[1] == "notes": out += num(NUM["NOTES"], t, 0, " ".join(w[2:]))
            elif w[1].upper() in L.VALUE: out += num(NUM["VALUE"], t, L.VALUE[w[1].upper()], " ".join(w[2:]))
            else: raise Cant("until: %s" % c)
        return out
    # ---- a step, a lesson
    def effect(self, text, song):
        out = b""
        for e in [x.strip() for x in text.split(";") if x.strip()]:
            m = re.match(r"go to (\d+)$", e)
            if m: out += rec(EFF["GOTO"], int(m.group(1)) - 1)
            elif e in ("right", "wrong"): out += rec(EFF[e.upper()])
            else: out += rec(EFF["SETUP"], self.setup(e[6:].strip() if e.startswith("setup:") else e, song))
        return out
    def step(self, s, lesson, song):
        kind = s.kind.lower(); f = b""
        def field(tag, body):
            if len(body) > 255: raise Cant("%s is %d bytes long, and a field holds 255" % (tag, len(body)))
            return bytes([TAG[tag], len(body)]) + body
        keys = lambda text: bytes(L.keys_of(" ".join(x for x in text.split() if x.lower() not in L.SPEED), lesson.phrases))
        speed = lambda text: next((int(L.SPEED[x.lower()] * 100) for x in text.split() if x.lower() in L.SPEED), 38)
        if kind == "setup": f += field("SETUP", self.setup(s.text, song))
        elif kind == "wait": f += field("WAIT", self.wait(s.text))
        elif kind == "play": f += field("PLAY", bytes([speed(s.text)]) + keys(s.text))
        elif kind == "echo": f += field("ECHO", keys(s.text))
        else: f += field("TEXT", L.words(s.text).encode("ascii"))
        for k, v in s.subs:
            if k in ("OCT-", "OCT+"):
                label, _, eff = v.partition("→"); f += field("LEFT" if k == "OCT-" else "RIGHT", s0(label.strip()) + self.effect(eff.strip(), song))
            elif k == "Do": f += field("DO", self.do(v, lesson.phrases))
            elif k == "Until": f += field("UNTIL", self.until(v))
            elif k == "Hint": f += field("HINT", L.words(v).encode("ascii"))
            elif k == "Play": f += field("PLAY", bytes([speed(v)]) + keys(v))
            elif k == "Echo": f += field("ECHO", keys(v))
            elif k == "Follow": f += field("FOLLOW", keys(v))
            elif k == "Lights": f += field("LIGHTS", keys(v))
            elif k == "Wait": f += field("WAIT", self.wait(v))
            elif k == "Keys" and v.strip() == "silent": f += field("SILENT", b"")
            elif k == "Page": f += field("PAGE", bytes([L.PAGES[v.strip().upper()]]))
            elif k in ("Right", "Wrong"): f += field(k.upper() + "TXT", L.words(v).encode("ascii"))
            elif k == "Robot": pass
            else: raise Cant("no bullet called %s" % k)
        body = bytes([KIND[kind], 1 if "beginners only" in s.tags else 0]) + f
        return struct.pack("<H", len(body)) + body
    def wait(self, text):
        m = re.match(r"until bar (\d+)", text.lower())
        if m: return bytes([WAIT["until"]]) + struct.pack("<H", int(m.group(1)) - 1)
        m = re.match(r"([\d.]+)\s*(second|bar|beat)", text.lower())
        return bytes([WAIT[m.group(2)]]) + struct.pack("<H", int(round(float(m.group(1)) * 10)))
    def lesson(self, l):
        m = re.search(r"song (.+?),? section", " ".join(s.text for s in l.steps if s.kind.lower() == "setup").lower())
        song = m.group(1).replace(" ", "-") if m else "house-one"
        out = bytes([l.number, len(l.steps)]) + s0(l.title)
        for s in l.steps:
            try: out += self.step(s, l, song)
            except (Cant, KeyError, ValueError, L.Bad) as e: raise Cant("lesson %d step %d: %s" % (l.number, s.n, e))
        return out

def build(first=1, last=99):
    p = Pack(); lessons = [p.lesson(l) for l in L.lessons() if first <= l.number <= last]
    fb = open(os.path.join(HERE, "..", "assets", "fonts", "lessons-font.bin"), "rb").read()      # the lessons' two fonts
    if fb[:4] != b"TPF1": raise Cant("assets/fonts/lessons-font.bin is not a font file of tools/mklessonfont.py")
    o = struct.unpack("<HH", fb[4:8]); fonts = [fb[o[0]:o[1]], fb[o[1]:]]
    head = 8 + 2 * (len(lessons) + len(p.clips) + len(fonts)); at = head; offs = []
    for b in lessons + p.clips: offs.append(at); at += len(b)
    pad = at & 1; at += pad                                                   # (a glyph holds a 16-bit number: the fonts from an even byte)
    for b in fonts: offs.append(at); at += len(b)
    if at > 0xFFFF: raise Cant("the pack is %d bytes: more than its offsets can say" % at)
    pack = b"TPC2" + struct.pack("<HBB", at, len(lessons), len(p.clips)) + b"".join(struct.pack("<H", o) for o in offs) + b"".join(lessons + p.clips) + bytes(pad) + b"".join(fonts)
    return pack, [len(b) for b in lessons], [len(b) for b in p.clips]

def slot_image(pack):
    """the pack as it sits in a sample slot: (the 480 bytes of its header, the data that follows at +512)"""
    head = b"TPCO" + struct.pack("<H", 1) + bytes(10) + struct.pack("<II", len(pack), zlib.crc32(pack) & 0xFFFFFFFF)
    return head + bytes(480 - len(head)), pack

def header():
    out = "/* SPDX-License-Identifier: GPL-3.0-only\n * Teachie Beachie: the numbers of the course's pack, written by tools/mkcourse.py --header: do not edit. */\n"
    for name, d in (("CK", KIND), ("CT", TAG), ("CE", EFF), ("CD", DO), ("CS", SET), ("CC", CON), ("CN", NUM), ("CP", PROP), ("CW", WAIT)):
        out += "enum { " + ", ".join("%s_%s = %d" % (name, re.sub(r"\W", "_", k).upper(), v) for k, v in d.items()) + " };\n"
    out += "enum { " + ", ".join("CM_%s = %d" % (k.upper().replace(" ", "_"), v) for k, v in CMP.items()) + " };\n"
    return out

if __name__ == "__main__":
    if "--header" in sys.argv:
        open(os.path.join(HERE, "..", "app", "tp_course.h"), "w").write(header()); print("app/tp_course.h written")
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    out = args[0] if args else os.path.join(HERE, "..", "build", "course.bin")
    first, last = (int(x) for x in args[1].split("-")) if len(args) > 1 else (1, 99)
    try: pack, ls, cs = build(first, last)
    except Cant as e: raise SystemExit("NOT PACKED: %s" % e)
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True); open(out, "wb").write(pack)
    print("%s: %d bytes: %d lessons (%d bytes), %d clip sets (%d bytes)" % (out, len(pack), len(ls), sum(ls), len(cs), sum(cs)))
