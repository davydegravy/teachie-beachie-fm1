#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""(SLOOP 3.4.2 trial, step 8) The lesson remembered after the FM-1 is switched off.

When a lesson of the FM-1's own course begins, its number is written into the next blank byte of the lessons' slot
(between the pack and the logo); the first START after switching on reads the last one and comes up on that lesson.
No erase, and the learner's song is not touched (fw/course.inc; the write is started by the loop in fw/main.c).
The rules, each checked here on the simulated FM-1 (build/twin), which is switched off and on by ending it and
starting another from the flash it left:

  remembered      lesson 3 begun, off and on: START comes up on lesson 3
  one byte each   each record changes exactly one byte of the slot, in its blank part; nothing else in the slot moves
  not twice       a lesson begun again writes nothing
  a cut write     the last byte left blank, or half written (more than four bits set): the lesson before stands, and the
                  next record goes into the next blank byte, never into the spoiled one
  full            every blank byte used: nothing more is written, the last record stays, the course goes on
  sent again      the lessons sent again, the FM-1 still on: the course starts at lesson 1, the blank part blank

A real chip is not checked here: whether the FM-1's flash takes a byte written into blank flash (step 10).

Run from the trial's top folder:    python3 tutorial/upgrade/remembercheck.py
"""
import os, shutil, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..")); sys.path.insert(0, os.path.join(HERE, "..", "..", "tools"))
import mkcourse, device
from tp.fm1 import FM1, Twin, BTN, KNOB
PH = device.PH; NAME = {v: k for k, v in PH.items()}
OUT = os.path.join("build", "tw", "remembercheck")
SLOT = 0xE7000                                                      # the last sample slot, where the test's pack is (fw/main.c SLOT_AT[3])
fails = []
def check(ok, what):
    print("  %s %s" % ("ok  " if ok else "FAIL", what), flush=True)
    if not ok: fails.append(what)
def until(fn, secs):
    end = time.time() + secs
    while time.time() < end:
        if fn(): return True
        time.sleep(0.05)
    return False
FOUR = [b for b in range(256) if bin(b).count("1") == 4]            # a record: a byte with four bits set; the lesson's number counted among them
def code(lesson): return FOUR[lesson - 1]
def area(flash):
    """the blank part of the slot in a flash file: (where it begins in the file, its bytes)"""
    d = open(flash, "rb").read(); tpc = SLOT + 512
    a = tpc + int.from_bytes(d[tpc + 4: tpc + 6], "little"); e = tpc + int.from_bytes(d[SLOT + 24: SLOT + 28], "little")
    return a, d[a:e]
def records(flash): a, b = area(flash); return list(b.rstrip(b"\xff"))
def slot(flash): return open(flash, "rb").read()[SLOT: SLOT + 0x14000]
def patched(flash, name, change):
    """a copy of a flash file with its blank part changed: what a power cut, or long use, leaves"""
    d = bytearray(open(flash, "rb").read()); a, b = area(flash); b = bytearray(b); change(b); d[a: a + len(b)] = b
    os.makedirs(os.path.join(OUT, name), exist_ok=True); p = os.path.join(OUT, name, "start.bin"); open(p, "wb").write(d); return p

class On:
    """a simulated FM-1 switched on with a flash; its own course driven by the two OCT buttons and [SELECT]"""
    def __init__(self, flash, name):
        os.environ["TP_HOME"] = "1"; self.name = name
        self.link = Twin(out=os.path.join(OUT, name), flash=flash); self.f = FM1(self.link); time.sleep(3.2)
    def tap(self, b): device.tap(self.link, "button", BTN[b])
    def ph(self): return self.f.course()[0]
    def start(self):                                                # START, then the course's card: the remembered lesson's first card
        self.tap("OCT_UP"); ok = until(lambda: self.ph() == PH["TOC"], 6); self.tap("OCT_UP"); return ok and until(lambda: self.ph() == PH["BEGIN"], 4)
    def begin(self):                                                # BEGIN: the lesson's number, once it runs
        self.tap("OCT_UP"); until(lambda: self.ph() >= PH["STEP"] and self.f.course()[1] > 0, 6); time.sleep(1.5); return self.f.course()[1]
    def leave(self):                                                # both OCT buttons held: out of the lesson, to its first card
        self.link.button(BTN["OCT_DOWN"], 1); self.link.button(BTN["OCT_UP"], 1); ok = until(lambda: self.ph() == PH["BEGIN"], 5)
        self.link.button(BTN["OCT_UP"], 0); self.link.button(BTN["OCT_DOWN"], 0); time.sleep(0.4); return ok
    def pick(self, clicks):                                         # LESSONS: the list, [SELECT] to another, BEGIN
        self.tap("OCT_DOWN"); until(lambda: self.ph() == PH["LIST"], 4)
        for _ in range(abs(clicks)): self.link.knob(KNOB["SELECT"], 1 if clicks > 0 else -1); time.sleep(0.25)
        return self.begin()
    def off(self):
        """switched off: the flash as it leaves it, and what the simulated FM-1 said of its flash"""
        self.link.close(); said = []
        while not self.link.lines.empty(): said.append(self.link.lines.get())
        self.said = [l for l in said if "TWIN" in l]
        return os.path.join(OUT, self.name, "flash.bin")
def clean(on, what, writes):
    """what the simulated FM-1 said of its flash when it ended: no write over bits already used, no read of the window's
    old copy of a byte, and so many writes, no erase"""
    bad = [l for l in on.said if "not erased" in l or "rewritten since" in l or "breach" in l]; end = [l for l in on.said if "stale reads" in l]
    want = "flash: 0 erases, %d writes; window: 0 stale reads" % writes
    check(not bad and bool(end) and want in end[-1], "%s: %d %s to the flash, no erase, none over used bits, no stale read (%s)" % (what, writes, "write" if writes == 1 else "writes", (bad or end or ["nothing said"])[-1].split("failed; ")[-1][:70]))

if __name__ == "__main__":
    shutil.rmtree(OUT, ignore_errors=True); os.makedirs(OUT)
    pack = mkcourse.build()[0]; seed = device.seed_flash(pack); a0, blank = area(seed)
    check(set(blank) == {0xFF} and len(blank) >= 1024, "a fresh slot: %d blank bytes between the pack and the logo" % len(blank))

    print("remembered: lessons 1 and 3 begun, then off and on", flush=True)
    on = On(seed, "a"); check(on.start(), "START on a fresh FM-1: lesson 1's card")
    check(on.begin() == 1, "BEGIN: lesson 1 runs"); check(on.leave(), "both OCT buttons: out of it")
    check(on.pick(2) == 3, "LESSONS, [SELECT] two on, BEGIN: lesson 3 runs"); a = on.off(); clean(on, "two lessons begun", 2)
    check(records(a) == [code(1), code(3)], "the blank part: two bytes, lessons 1 and 3 (%s)" % ["%02X" % b for b in records(a)])
    s0, s1 = slot(seed), slot(a); diff = [i for i in range(len(s0)) if s0[i] != s1[i]]
    check(diff == [a0 - SLOT, a0 - SLOT + 1], "the slot is as it was but for those two bytes (%d bytes differ)" % len(diff))
    on = On(a, "b"); check(on.start(), "off and on, START")
    check(on.begin() == 3, "BEGIN: it comes up on lesson 3"); check(on.leave() and on.begin() == 3, "lesson 3 left and begun again"); b = on.off(); clean(on, "a lesson begun again", 0)
    check(records(b) == [code(1), code(3)], "not twice: still two bytes (%d)" % len(records(b)))

    print("a write cut by a power-off", flush=True)
    c = patched(b, "c", lambda x: x.__setitem__(1, 0xFF))
    on = On(c, "c"); on.start(); check(on.begin() == 1, "the last byte left blank: lesson 1, the one before"); c2 = on.off()
    check(records(c2) == [code(1)], "and nothing written for it (it is the lesson remembered)")
    half = code(3) | next(1 << i for i in range(8) if not code(3) >> i & 1)             # one bit more than lesson 3's byte: a write stopped half way
    d = patched(b, "d", lambda x: x.__setitem__(1, half))
    on = On(d, "d"); on.start(); check(on.begin() == 1, "the last byte half written (%02X, five bits set): lesson 1 again" % half)
    on.leave(); check(on.pick(4) == 5, "then lesson 5 begun"); d2 = on.off(); clean(on, "a lesson begun after a spoiled byte", 1)
    check(records(d2) == [code(1), half, code(5)], "its record is in the next blank byte, the spoiled one untouched (%s)" % ["%02X" % x for x in records(d2)])
    on = On(d2, "d2"); on.start(); check(on.begin() == 5, "off and on: lesson 5"); on.off()

    print("the blank part used up", flush=True)
    e = patched(seed, "e", lambda x: x.__setitem__(slice(0, len(x)), bytes([code(2)]) * len(x)))
    on = On(e, "e"); on.start(); check(on.begin() == 2, "every blank byte holds lesson 2: it comes up on lesson 2")
    on.leave(); check(on.pick(2) == 4, "lesson 4 begun all the same"); e2 = on.off(); clean(on, "a lesson begun with no blank byte left", 0)
    check(slot(e2) == slot(e), "nothing written: the slot is as it was")
    on = On(e2, "e2"); on.start(); check(on.begin() == 2, "off and on: still lesson 2, the last record"); on.off()

    print("the lessons sent again", flush=True)
    on = On(a, "f"); on.start(); check(on.begin() == 3, "lesson 3 remembered and running")
    head, data = mkcourse.slot_image(pack); on.f.write_slot(3, head, data); time.sleep(1.0)
    check(until(lambda: on.ph() == PH["OFF"], 4) and on.f.course()[3] == 30, "the lessons sent again while it is on: the course stops, 30 lessons found")
    check(on.start() and on.begin() == 1, "START, BEGIN: lesson 1"); f2 = on.off()
    check(records(f2) == [code(1)], "the blank part blank again, but for lesson 1 just begun (%s)" % ["%02X" % x for x in records(f2)])
    print("remembercheck: %s" % ("OK" if not fails else "%d FAILED" % len(fails)))
    sys.exit(1 if fails else 0)
