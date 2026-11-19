#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Peachie: the lessons checked.

    python3 tutorial/check.py              every lesson file read: its steps understood, its words measured against
                                           the FM-1's screen (lines that would not fit are named)
    python3 tutorial/check.py --run        and every lesson played from its first card to its last on a simulated FM-1,
                                           by a robot learner that does what each step asks (several lessons at once).
                                           A picture of each lesson's screens goes to tutorial/lessons/screens/
    python3 tutorial/check.py --run 3 4    only lessons 3 and 4

Run it from the repository's top folder; --run needs build/twin (make build/twin)."""
import os, re, subprocess, sys, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from tp import lesson as L

def advances(name):
    """the width of each letter of one of the screen's fonts, from app/fonts.c"""
    src = open(os.path.join(HERE, "..", "app", "fonts.c")).read()
    block = src[src.index("gfx_glyph %s_g[" % name):]; block = block[:block.index("};")]
    return [int(m.group(1)) for m in re.finditer(r"\{\s*\d+,\s*\d+,\s*-?\d+,\s*-?\d+,\s*(\d+),\s*\d+\s*\}", block)]
ADV = dict(s=advances("font_s"), l=advances("font_l"))

def width(font, text): return sum(ADV[font][ord(c) - 32] if 32 <= ord(c) < 96 else ADV[font][31] for c in text)

def wrap(text, font):
    """the words cut into lines as the FM-1 does it (fw/main.c guide_wrap): at a new line, and at a space before the edge"""
    lines, i = [], 0
    while i < len(text) and len(lines) < 99:
        s = e = i; last = -1
        while e < len(text) and text[e] != "\n":
            if text[e] == " ": last = e
            if last > s and width(font, text[s:e + 1]) > 240 - 28: e = last; break
            e += 1
        lines.append(text[s:e]); i = e
        if i < len(text) and text[i] in " \n": i += 1
    return lines

KEYS = {"OCT-", "OCT+", "Do", "Until", "Hint", "Play", "Echo", "Wait", "Keys", "Lights", "Page", "Robot", "Follow", "Right", "Wrong"}

def lint(lesson):
    bad = []
    def say(step, what): bad.append("%s step %d: %s" % (lesson.name, step.n, what))
    def fits(step, text, kind, answers=False):
        w = L.words(text); font = "l" if kind in ("big", "reply") else "s"
        for c in w:
            if c != "\n" and not 32 <= ord(c) < 96: say(step, "the screen has no letter %r (in \"%s\")" % (c, text[:40]))
        if len(w) > 175: say(step, "%d letters, and the screen's card holds 175: \"%s...\"" % (len(w), text[:40]))
        lines = wrap(w, font); room = 4 if font == "l" else 3 if kind.startswith("strip") else 7 if answers else 8
        if len(lines) > room: say(step, "%d lines on the screen, and a %s holds %d: %s" % (len(lines), kind, room, " | ".join(lines)))
        for ln in lines:
            if width(font, ln) > 240 - 20: say(step, "a line too wide for the screen: \"%s\"" % ln)
    for step in lesson.steps:
        kind = step.kind.lower()
        try:
            if kind not in L.KINDS: say(step, "no kind of step called %s" % step.kind); continue
            for k, v in step.subs:
                if k not in KEYS: say(step, "no bullet called %s" % k)
            if kind == "setup": L.parse_setup(step.text)
            elif kind == "wait": pass
            elif kind in ("play", "echo"): L.keys_of(" ".join(x for x in step.text.split() if x.lower() not in L.SPEED), lesson.phrases)
            else:
                answers = bool(step.sub("OCT-") or step.sub("OCT+"))
                fits(step, step.text, kind, answers)
                for key in ("Hint", "Right", "Wrong"):
                    for h in step.all(key): fits(step, h, kind if key == "Hint" else "big" if len(h) < 24 else "card", answers and key == "Hint")
            for key in ("OCT-", "OCT+"):
                v = step.sub(key)
                if v is None: continue
                label, _, effect = v.partition("→"); label = label.strip().upper()
                if kind.startswith("strip") and "[%s]" % key not in step.text.upper().replace("−", "-"): say(step, "a strip shows no answers: its words should name [%s]" % key)
                other = (step.sub("OCT+" if key == "OCT-" else "OCT-") or "").partition("→")[0].strip().upper()
                if width("s", label) > 150 or (key == "OCT-" and width("s", label) + width("s", other) > 200): say(step, "the answers \"%s\" and \"%s\" are too wide to sit side by side" % (label, other))
                for e in [x.strip() for x in effect.split(";") if x.strip()]:
                    m = re.match(r"go to (\d+)$", e)
                    if m and not 1 <= int(m.group(1)) <= len(lesson.steps): say(step, "\"%s\": no such step" % e)
                    elif not m and e not in ("right", "wrong"): L.parse_setup(e[6:].strip() if e.startswith("setup:") else e)
            for key in ("Do", "Robot"):
                if not step.sub(key): continue
                for op in L.parse_do(step.sub(key), lesson.phrases):
                    if op[0] in ("press", "hold") and op[1] in (0, 1) and step.sub("OCT+" if op[1] else "OCT-"):
                        say(step, "%s asks for [%s], which is also this step's answer button" % (key, "OCT+" if op[1] else "OCT-"))
            if step.sub("Until"): L.parse_until(step.sub("Until"))
            for key in ("Play", "Echo", "Follow", "Lights"):
                if step.sub(key): L.keys_of(" ".join(x for x in step.sub(key).split() if x.lower() not in L.SPEED), lesson.phrases)
            if step.sub("Page") and step.sub("Page").upper() not in L.PAGES: say(step, "no page called %s" % step.sub("Page"))
            if step.sub("Wait") and not re.match(r"(until bar \d+|[\d.]+\s*(second|bar|beat))", step.sub("Wait").lower()): say(step, "a wait is written \"4 seconds\", \"2 bars\" or \"until bar 9\"")
        except L.Bad as e: say(step, str(e))
        except Exception as e: say(step, "%s: %s" % (type(e).__name__, e))
    for need in ("Part", "The idea", "The learner does", "Got it when", "Song"):
        if need not in lesson.head: bad.append("%s: the list at the top has no \"%s\"" % (lesson.name, need))
    return bad

def run(lesson):
    """the lesson played on a simulated FM-1; (it went through, the last lines of what it said, seconds)"""
    t0 = time.time(); out = os.path.join("build", "tw", "lessons", lesson.name)
    if os.path.isdir(out):
        for f in os.listdir(out):
            if f.endswith((".ppm", ".json")): os.remove(os.path.join(out, f))
    env = dict(os.environ, UBSAN_OPTIONS="print_stacktrace=1:halt_on_error=1", ASAN_OPTIONS="detect_leaks=0:handle_segv=0:allow_user_segv_handler=1:handle_sigfpe=0")
    p = subprocess.run([sys.executable, os.path.join(HERE, "run.py"), str(lesson.number), "--twin", "--fast"], capture_output=True, text=True, env=env, timeout=900)
    text = p.stdout + p.stderr; ok = p.returncode == 0 and "LESSON DONE" in text
    open(os.path.join(out, "log.txt"), "w").write(text) if os.path.isdir(out) else None
    if ok:
        from sheet import sheet
        os.makedirs(os.path.join(HERE, "lessons", "screens"), exist_ok=True)
        sheet(out, os.path.join(HERE, "lessons", "screens", lesson.name + ".png"), cols=6)
    return ok, text.strip().split("\n")[-6:], time.time() - t0

if __name__ == "__main__":
    try: all_ = L.lessons()
    except L.Bad as e: raise SystemExit("NOT READ: %s" % e)
    want = [int(a) for a in sys.argv[1:] if a.isdigit()]
    chosen = [l for l in all_ if not want or l.number in want]
    fail = 0
    for l in chosen:
        bad = lint(l)
        print("%-28s %s" % (l.name, "read: %d steps" % len(l.steps) if not bad else "PROBLEMS"))
        for b in bad: print("    " + b); fail += 1
    if "--run" in sys.argv:
        print("playing %d lessons on simulated FM-1s ..." % len(chosen), flush=True)
        with ThreadPoolExecutor(max_workers=6) as pool:
            for l, (ok, tail, secs) in zip(chosen, pool.map(run, chosen)):
                print("%-28s %s  (%.0f s)" % (l.name, "played through" if ok else "STOPPED", secs), flush=True)
                if not ok:
                    fail += 1
                    for t in tail: print("    " + t[:230])
    sys.exit(1 if fail else 0)
