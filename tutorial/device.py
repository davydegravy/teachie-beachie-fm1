#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: the course on the FM-1 itself, with no computer leading.

    python3 tutorial/device.py              every lesson packed (tools/mkcourse.py), put on a simulated FM-1, and played
                                            there by the FM-1's own course (fw/course.inc). A robot only presses: it
                                            reads the lesson's file to know what a learner would do at each step
    python3 tutorial/device.py 3 4          only lessons 3 and 4
    python3 tutorial/device.py --clumsy     the robot first taps a blinking box with SEQ not held, as a beginner does
    python3 tutorial/device.py --sticky     the robot is slow to let go: what it holds for a step stays down a second into
                                            the next one (a step must not go by on a fill or an effect held from before)
    tutorial/.venv/bin/python tutorial/device.py --install [--slot 4] [1-7]
                                            the pack put on a real FM-1 over USB, in a user sample slot (the last empty
                                            one, unless told otherwise); the lessons 1-7 only, or all of them

Run it from the repository's top folder; the simulated FM-1 needs build/twin (make build/twin)."""
import os, re, sys, time
from concurrent.futures import ThreadPoolExecutor
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE); sys.path.insert(0, os.path.join(HERE, "..", "tools"))
import mkcourse
from tp.fm1 import FM1, Twin, Midi, BTN, KNOB
from tp.teach import Robot
from tp import lesson as L

PH = dict(OFF=0, BEGIN=1, LIST=2, STEP=3, PLAY=4, ECHO=5, DO=6, ANSWER=7, WAIT=8, MSG=9, STUCK=10, END=11, BYE=12, TOC=13)
OUT = os.path.join("build", "tw", "device")
LIMIT = 300                                                         # a lesson going round and round (a "not yet", again and again) stops here
clumsy = "--clumsy" in sys.argv
sticky = "--sticky" in sys.argv

def tap(link, what, which, hold=0.1, after=0.25):
    (link.button if what == "button" else link.key)(which, 1); time.sleep(hold); (link.button if what == "button" else link.key)(which, 0); time.sleep(after)

def hands(link, ops, keep=False):
    """the moves made; keep: what is held at the end stays down, and is returned as [(what, which)]"""
    mv = list(L.moves([("khold", o[1]) if keep and o[0] == "long" else o for o in ops])); down = []
    while keep and mv and mv[-1][0] in ("button", "key") and mv[-1][2] == 0: down.append(mv.pop()[:2])
    for m in mv:
        if m[0] == "button": link.button(m[1], m[2])
        elif m[0] == "key": link.key(m[1], m[2])
        elif m[0] == "knob": link.knob(m[1], m[2])
        time.sleep(m[3])
    return down

def play(lesson, index, seed):
    """one lesson on a simulated FM-1 that leads itself: (it reached its end, what was noticed, seconds)"""
    t0 = time.time(); out = os.path.join(OUT, lesson.name); os.makedirs(out, exist_ok=True); log = []
    for f in os.listdir(out):
        if f.endswith(".ppm"): os.remove(os.path.join(out, f))
    os.environ["TP_HOME"] = "1"; os.environ.setdefault("UBSAN_OPTIONS", "print_stacktrace=1:halt_on_error=1")   # (a fault in the firmware stops the run)
    link = Twin(out=out, flash=seed); f = FM1(link); robot = Robot(link)
    try:
        time.sleep(3.2)                                             # (the logo, then the start card)
        tap(link, "button", BTN["OCT_UP"])                          # START: nobody answers, so the FM-1's own course begins
        end = time.time() + 6
        while f.course()[0] != PH["TOC"]:                           # (the card of the whole course)
            if time.time() > end: return False, ["START did not begin the FM-1's own course"], time.time() - t0
            time.sleep(0.1)
        if not index and not os.listdir(out).count("00-course.ppm"): time.sleep(0.3); link.shot("00-course")
        if index:                                                   # LESSONS: the list of lessons, SELECT to this one
            tap(link, "button", BTN["OCT_DOWN"])
            for _ in range(index): link.knob(KNOB["SELECT"], 1); time.sleep(0.12)
        else: tap(link, "button", BTN["OCT_UP"])                    # BEGIN: the lesson's first card
        tap(link, "button", BTN["OCT_UP"])                          # BEGIN
        last, since, tries, shot, down = None, time.time(), 0, 0, []
        while True:
            ph, num, step, _ = f.course(); now = time.time()
            if down and (ph, step) != last:                         # slow to let go: still down a second into what comes next
                at = step; time.sleep(1.0); ph, num, step, _ = f.course(); now = time.time()
                for what, which in down: (link.button if what == "button" else link.key)(which, 0)
                if os.environ.get("TP_DEBUG"): print("      held %s into step %d for a second; now at step %d" % (down, at + 1, step + 1), flush=True)
                down = []; time.sleep(0.3)
                if step != at and at < len(lesson.steps) and (lesson.steps[at].sub("Until") or "").lower() in ("fill on", "punch on"):
                    return False, log + ["step %d went by on what was still held from the step before" % (at + 1)], now - t0
            if (ph, step) != last:
                last, since, tries = (ph, step), now, 0
                if ph in (PH["DO"], PH["ANSWER"], PH["ECHO"], PH["WAIT"], PH["END"]): shot += 1; time.sleep(0.2); link.shot("%02d-%02d" % (lesson.number, shot))
            if ph == PH["END"]: return num == lesson.number, log, now - t0
            if ph == PH["OFF"]: return False, log + ["the course stopped"], now - t0
            if ph == PH["STUCK"]: return False, log + ["STUCK? at step %d" % (step + 1)], now - t0
            if now - since > 45: return False, log + ["step %d never ended (phase %d)" % (step + 1, ph)], now - t0
            if now - t0 > LIMIT: return False, log + ["no end after %d s: at step %d (phase %d), going round" % (LIMIT, step + 1, ph)], now - t0
            if step >= len(lesson.steps): time.sleep(0.05); continue
            s = lesson.steps[step]
            if ph == PH["ECHO"]: robot.act(); time.sleep(0.15); continue
            if ph not in (PH["DO"], PH["ANSWER"]) or now - since < 0.5 + 2.0 * tries: time.sleep(0.05); continue
            tries += 1
            ops = L.parse_do(s.sub("Robot") or s.sub("Do") or "", lesson.phrases)
            answer = next((o[1] for o in ops if o[0] == "answer"), "right" if s.sub("OCT+") else "left"); ops = [o for o in ops if o[0] not in ("answer", "idle")]
            if ph == PH["ANSWER"] and not s.sub("Robot"): ops = []                         # (a question: only its answer, unless the robot is told more)
            conds = L.parse_until(s.sub("Until") or "")
            if ph == PH["DO"] and any(c.miss for c in conds):                              # the boxes already right are left alone
                seen = L.Seen(f, {}, [], set()); miss = {L.WHITE[b % 16] for c in conds if c.miss for b in c.miss(seen)}
                ops = [o for o in ops if o[0] not in ("tap", "long", "khold") or o[1] in miss]
                if clumsy and tries == 1 and miss and any(c.drum is not None for c in conds):
                    tap(link, "key", sorted(miss)[-1]); log.append("step %d: a box tapped with SEQ not held" % (step + 1)); time.sleep(3.2)
            down = hands(link, ops, keep=sticky and ph == PH["DO"] and (s.sub("Until") or "").lower() in ("fill on", "punch on"))
            if ph == PH["ANSWER"]: time.sleep(0.2); tap(link, "button", BTN["OCT_DOWN"] if answer == "left" and s.sub("OCT-") else BTN["OCT_UP"])
    finally:
        link.close()

def seed_flash(pack):
    """a simulated FM-1's flash with the pack in its last sample slot"""
    out = os.path.join(OUT, "seed"); link = Twin(out=out); f = FM1(link)
    try:
        head, data = mkcourse.slot_image(pack); f.write_slot(3, head, data); time.sleep(0.4)
        if f.course()[3] == 0: raise SystemExit("the simulated FM-1 did not find the pack")
    finally: link.close()
    return os.path.join(out, "flash.bin")

def install(first, last, slot):
    """the pack put on a real FM-1: in the slot asked for, or the last empty one"""
    pack, ls, cs = mkcourse.build(first, last); head, data = mkcourse.slot_image(pack)
    f = FM1(Midi()); slots = f.slots(); had = f.course()[3]
    print("the FM-1's sample slots: " + ", ".join("%d %s" % (i + 1, "%s (%d samples)" % (n, z) if z else "empty") for i, (z, n, kb) in enumerate(slots)))
    if had: print("it already holds %d lessons; they are replaced" % had)
    if slot is None:
        free = [i for i, (z, n, kb) in enumerate(slots) if not z]
        if not free: raise SystemExit("all four slots hold samples. Name one to give up: --slot N")
        slot = free[-1]
    elif slots[slot][0] and "--force" not in sys.argv: raise SystemExit("slot %d holds samples (%s). Choose another with --slot, or add --force to replace them." % (slot + 1, slots[slot][1]))
    print("the slot: the lessons %d bytes, blank %d, the logo %d" % mkcourse.slot_room(pack))
    f.guide(5)                                                      # the start's card: a lesson running on the FM-1 stops (it reads the slot)
    t0 = time.time(); f.write_slot(slot, head, data); time.sleep(0.6); n = f.course()[3]
    f.guide(5)                                                      # (the start's card again, in the lessons' font)
    print("%d lessons (%d bytes) put in slot %d in %.0f s; the FM-1 finds %d lessons" % (len(ls), len(pack), slot + 1, time.time() - t0, n))
    if n != len(ls): raise SystemExit("the FM-1 does not find them: is this the firmware with the course in it?")

if __name__ == "__main__":
    span = next((a for a in sys.argv[1:] if re.fullmatch(r"\d+-\d+", a)), None)
    first, last = (int(x) for x in span.split("-")) if span else (1, 99)
    if "--install" in sys.argv:
        install(first, last, int(sys.argv[sys.argv.index("--slot") + 1]) - 1 if "--slot" in sys.argv else None); sys.exit(0)
    try: pack, ls, cs = mkcourse.build(first, last)
    except mkcourse.Cant as e: raise SystemExit("NOT PACKED: %s" % e)
    print("the pack: %d bytes, %d lessons" % (len(pack), len(ls)), flush=True)
    lessons = [l for l in L.lessons() if first <= l.number <= last]
    want = [int(a) for a in sys.argv[1:] if a.isdigit()]
    chosen = [(l, i) for i, l in enumerate(lessons) if not want or l.number in want]
    seed = seed_flash(pack); fail = 0
    print("playing %d lessons on simulated FM-1s that lead themselves%s ..." % (len(chosen), ", with a clumsy robot" if clumsy else ", with a robot slow to let go" if sticky else ""), flush=True)
    with ThreadPoolExecutor(max_workers=6) as pool:
        for (l, i), (ok, log, secs) in zip(chosen, pool.map(lambda li: play(li[0], li[1], seed), chosen)):
            print("%-28s %s  (%.0f s)" % (l.name, "played through" if ok else "STOPPED", secs), flush=True)
            for x in log: print("    " + x)
            fail += not ok
            if ok and "--pictures" in sys.argv:
                from sheet import sheet; sheet(os.path.join(OUT, l.name), os.path.join(OUT, l.name + ".png"), cols=6)
    sys.exit(1 if fail else 0)
