#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: the firmware's rooms, each against its limit, printed at the end of fw/build.sh. The flash before
# the sample bank is one limit of five: the linker's regions (fw/ram.ld) are the others, and the RAM for code outside
# the flash window and for variables ("slot") is the next to run out. A region passed stops the build at the link;
# this says how near each one is.     tp_room.py RAM.ELF LLVM_BIN APP.BIN
import os, re, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__))
elf, tc, appbin = sys.argv[1], sys.argv[2], sys.argv[3]
bank = int(re.search(r"BANK_AT = (0x[0-9a-fA-F]+)", open(os.path.join(HERE, "mkfw.py")).read()).group(1), 16)
regions = {m.group(1): (int(m.group(2), 16), int(m.group(3), 16))
           for m in re.finditer(r"^\s*(\w+)\s*\(\w+\)\s*:\s*ORIGIN = (0x[0-9A-Fa-f]+), LENGTH = (0x[0-9A-Fa-f]+)", open(os.path.join(HERE, "..", "fw", "ram.ld")).read(), re.M)}
sh = subprocess.run([os.path.join(tc, "llvm-readelf"), "-S", elf], capture_output=True, text=True).stdout
secs = [(int(m.group(1), 16), int(m.group(2), 16)) for m in re.finditer(r"\]\s+\.\w+\s+\w+\s+([0-9a-f]{8})\s+[0-9a-f]+\s+([0-9a-f]+)", sh)]
WHAT = dict(slot="RAM: code outside the window, variables", low="RAM: the patterns, their saved copies, the stack", boot="RAM: buffers", xip="the flash window's own region")
a = os.path.getsize(appbin); rows = [("flash before the sample bank", a, bank - 0x120)]
for name in ("slot", "low", "boot", "xip"):
    if name not in regions: sys.exit("tp_room: fw/ram.ld has no region called %s" % name)
    lo, ln = regions[name]; ends = [ad + sz for ad, sz in secs if lo <= ad < lo + ln]
    rows.append((WHAT[name], (max(ends) - lo) if ends else 0, ln))
print("the firmware's rooms:")
for what, used, of in rows: print("  %-48s %7d of %7d  %6d free" % (what, used, of, of - used))
