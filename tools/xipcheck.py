#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# The flash window's rule, checked on the built firmware: while a save has the window off, only RAM code runs.
# From the functions that may run then (the audio block, the second core's job, the save's steps, the panel's scan,
# the USB poll), every function reachable by a call or a jump must be in RAM, and none may read an address in the
# window (code or tables). Edges that run only once the window is back are listed in ALLOWED (a runtime check guards
# them). Exit 1 and the path on a breach.     tools/xipcheck.py build/fw/ram.elf LLVM_BIN
import re, subprocess, sys

elf, tc = sys.argv[1], sys.argv[2]
ROOTS = ["render_block", "job_voices", "store_step", "flash_busy", "flash_wait_idle", "panel_step", "usb_midi_poll",
         "time_wait_us", "cpu1_main", "isr_tick", "irq_tick", "usb_tick", "tick", "panel_tick", "trs_midi_poll",
         "sq_on", "sq_off", "sq_locks",            # (the sequencer's calls into fw/main.c, through pointers)
         "flash_read", "flash_erase_start", "flash_program_start"]   # (the store's steps call these through SF: 3.1.1)
ALLOWED = {("midi_poll", "midi_in")}          # (midi_poll returns before parsing while the window is off)
GUARDED = {"kit_cached"}                      # (reads the window's tables only while gb_flash_ok says it is on)
WIN_LO, WIN_HI = 0x02000000, 0x02080000

dis = subprocess.run([f"{tc}/llvm-objdump", "-d", "--no-show-raw-insn", elf], capture_output=True, text=True).stdout
funcs, cur = {}, None
for line in dis.splitlines():
    m = re.match(r"^([0-9a-f]+) <([^>]+)>:", line)
    if m:
        cur = m.group(2); funcs[cur] = {"addr": int(m.group(1), 16), "calls": set(), "win": set()}
        continue
    if cur is None or not line.strip():
        continue
    for t in re.findall(r"<([^>+]+)(?:\+0x[0-9a-f]+)?>", line):
        if t != cur:
            funcs[cur]["calls"].add(t)
    if re.search(r"[|&^]=", line):           # (a bit mask, not an address: the soft float's 1 << 25)
        continue
    vals = [int(h, 16) for h in re.findall(r"0x([0-9a-f]{7,8})\b", line)]
    vals += [int(d) for d in re.findall(r"(?<![0-9a-fx])(\d{8})(?!\d)", line)]   # (addresses show as decimals)
    for v in vals:
        if WIN_LO <= v < WIN_HI:
            funcs[cur]["win"].add(v)

def in_window(f):
    return f in funcs and WIN_LO <= funcs[f]["addr"] < WIN_HI

# the engines' parts, called through their table (engine/engines.c): every one is a root too
BOOT = {"_start", "irq_start", "cpu1_start", "audio_start", "cal_start", "mclk_start", "store_save_begin", "app_main"}
ROOTS += [f for f in funcs if re.search(r"_(render|start|block|release|spread)$", f) and not in_window(f) and f not in ROOTS and f not in BOOT]
bad, seen, stack = [], set(), [(r, [r]) for r in ROOTS if r in funcs]
missing = [r for r in ROOTS if r not in funcs]
while stack:
    f, path = stack.pop()
    if f in seen:
        continue
    seen.add(f)
    if in_window(f):
        bad.append(" -> ".join(path)); continue
    if funcs[f]["win"] and f not in GUARDED:
        bad.append(" -> ".join(path) + "  reads the window at " + ", ".join(hex(v) for v in sorted(funcs[f]["win"])))
    for c in funcs[f]["calls"]:
        if c in funcs and (f, c) not in ALLOWED:
            stack.append((c, path + [c]))
# (3.3.8) what starts flash work goes on once the window is off: it, and what it returns into, must be in RAM
# (3.3.8's store_proj_prep was in the window: the FM-1 froze at the logo). In the sources, a function that calls
# erase_start or program_start must not be XT/COLD; in the binary, these and app_main must be in RAM
STARTERS = ["store_step", "store_proj_prep", "ram_erase", "app_main",
            # (3.4.1, from STOOP 0.1.1's black screen) hal/fm1/flash.c's own: they turn the window off and on, or
            # change how it deciphers
            "flash_open", "flash_jedec", "flash_plain_from", "flash_read", "flash_erase_start", "flash_program_start",
            "flash_busy", "flash_wait_idle", "flash_xip_forget"]
for s in STARTERS:
    if s in funcs and in_window(s):
        bad.append(f"{s} starts flash work (or runs on after it) but is in the window")
import os
src_root = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
for rel in ["app/store.c", "fw/main.c"]:
    lines = open(os.path.join(src_root, rel)).read().split("\n")
    for i, l in enumerate(lines):
        if re.match(r"^(XT|COLD)\b[^;]*\(", l):
            body = []
            for l2 in lines[i:]:
                body.append(l2)
                if l2.startswith("}") or (l2.rstrip().endswith("}") and l2 is l): break
            if re.search(r"\b(erase_start|program_start|flash_erase_start|flash_program_start)\s*\(", "\n".join(body)):
                bad.append(f"{rel}:{i + 1}: a window function starts flash work: {l.strip()[:80]}")
# a RAM table holding a window address (a pointer followed while the window is off: 3.1's SF, found by hand)
import struct
img = open(elf, "rb").read()
sh = subprocess.run([f"{tc}/llvm-readelf", "-S", elf], capture_output=True, text=True).stdout
for m in re.finditer(r"\]\s+(\.data)\s+PROGBITS\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)", sh):
    a0, off, size = int(m.group(2), 16), int(m.group(3), 16), int(m.group(4), 16)
    for i in range(0, size - 3, 4):
        v = struct.unpack_from("<I", img, off + i)[0]
        if WIN_LO <= v < WIN_HI:
            bad.append(f"RAM data at {a0 + i:#x} holds a window address {v:#x}")
# a window table holding RAM code's addresses: what runs while the window is off may be handed it (3.1's SF: the
# store's flash steps, read during a save)
rs = subprocess.run([f"{tc}/llvm-readelf", "-s", elf], capture_output=True, text=True).stdout
fn, objs = {}, []
for l in rs.splitlines():
    f = l.split()
    if len(f) >= 8 and f[3] in ("OBJECT", "FUNC"):
        a = int(f[1], 16)
        if f[3] == "FUNC": fn[a] = f[7]
        elif WIN_LO <= a < WIN_HI: objs.append((a, int(f[2]), f[7]))
m = re.search(r"\]\s+\.xtext\s+PROGBITS\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)", sh)
if m:
    xa, xo = int(m.group(1), 16), int(m.group(2), 16)
    for a, n, nm in objs:
        ram = sorted({fn[v] for i in range(0, n - 3, 4) if (a + i) % 4 == 0
                      for v in [struct.unpack_from("<I", img, xo + a - xa + i)[0]] if v in fn and not WIN_LO <= v < WIN_HI})
        if ram:
            bad.append(f"the window's {nm} holds RAM code's addresses ({', '.join(ram)}): put it in RAM (RAMCONST)")
print(f"xipcheck: {len(seen)} functions may run with the window off; {sum(1 for f in funcs if in_window(f))} in the window"
      + (f"; roots not found: {', '.join(missing)}" if missing else ""))
for b in bad:
    print("  BREACH:", b)
sys.exit(1 if bad else 0)
