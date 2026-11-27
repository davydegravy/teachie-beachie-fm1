#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: an FM-1 that never ran SLOOP 3 gets its "your sounds" file at the first start.

SLOOP 3 writes that file (app/usersnd.h: "S3US", 32 places, then the favourites "FV" and a theme) the first time it
starts, even with nothing to bring over from SLOOP 2.8; its web editor keeps a sound by reading the file, changing
one place and writing it back, and refuses when there is none. Teachie Beachie leaves SLOOP 2.8's sounds out for the
room (TP_SLOOP28_DATA, fw/main.c boot_flash) and once left the empty file out with them (found by the run on Linux,
2026-11-27, where SLOOP's own browser test of the editor runs).

Checked here, with no browser: the simulated FM-1 started from a blank flash and ended leaves one such file, empty
and whole; started again from that flash, the file is where it was, byte for byte (it is written once).
    python3 tutorial/upgrade/soundscheck.py        (from the top folder; needs build/twin and build/fw/fw.area)"""
import os, shutil, sys, time
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
from tp.fm1 import Twin

US_N, US_REC, US_NFAV = 32, 80, 16                      # app/usersnd.h
US_LEN = 8 + US_N * US_REC
US_FILE = US_LEN + 2 + 2 * US_NFAV + 40
fails = 0
def check(ok, what):
    global fails
    print("  %s %s" % ("ok  " if ok else "FAIL", what)); fails += 0 if ok else 1

def start(out, flash):
    """the simulated FM-1 started from `flash` ("-": blank), left to start, ended: the flash it leaves"""
    shutil.rmtree(out, ignore_errors=True)
    t = Twin(out=out, flash=flash); time.sleep(6.0); t.close(); time.sleep(0.5)
    return open(os.path.join(out, "flash.bin"), "rb").read()
def banks(d): return [i for i in range(len(d) - 4) if d[i:i + 4] == b"S3US"]

first = start(os.path.join("build", "tw", "sounds-1"), "-")
at = banks(first)
check(len(at) == 1, "a blank FM-1, started once: one \"your sounds\" file in its flash (%d found)" % len(at))
if at:
    f = first[at[0]:at[0] + US_FILE]
    check(f[4] == 1 and f[5] == US_N, "its header: version 1, %d places" % US_N)
    check(all(f[8 + k * US_REC] == 0 for k in range(US_N)), "every place empty")
    check(f[US_LEN:US_LEN + 2] == b"FV" and f[US_LEN + 2:US_LEN + 2 + 2 * US_NFAV] == b"\xff" * (2 * US_NFAV), "the favourites after it, none set")
    again = start(os.path.join("build", "tw", "sounds-2"), os.path.join("build", "tw", "sounds-1", "flash.bin"))
    at2 = banks(again)
    check(at2 == at and again[at[0]:at[0] + US_FILE] == f, "started again from that flash: the file where it was, byte for byte")
print("soundscheck: %s" % ("OK" if not fails else "%d FAILED" % fails))
sys.exit(1 if fails else 0)
