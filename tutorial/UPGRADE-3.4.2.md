# Trial: Teachie Beachie on SLOOP 3.4.2

A trial, not for `main`. It answers one question: what does it take to move Teachie Beachie from SLOOP 3.3.7 to
SLOOP 3.4.2? Started on 2026-11-24. Nothing here is installed on an FM-1, pushed, or published.

## Where it is

- **Branch:** `trial/sloop-3.4.2`, local only (not pushed).
- **Folder:** `.worktrees/sloop-3.4.2` inside the project's folder: a second copy of the project (a git worktree).
  `main`, the published page and the FM-1 are untouched. `.worktrees/` is ignored locally (`.git/info/exclude`).
- **Every finished step is a commit on the branch,** so the folder can be thrown away and made again with
  `git worktree add .worktrees/sloop-3.4.2 trial/sloop-3.4.2`.

## To pick it up

Run everything from the trial's folder. The FM-1's toolchain is in the main copy:

```sh
cd .worktrees/sloop-3.4.2
make test                                   # SLOOP's own tests: pass
make build/twin                             # the simulated FM-1: builds
EDITOR_E2E=0 INSTALLER_E2E=0 sh fw/build.sh "$(git rev-parse --git-common-dir)/../toolchain/bin"
                                            # the firmware: compiles and links, then stops: 6,468 bytes too big
```

The simulated FM-1 needs `build/fw/fw.area` for its sample bank, and the build stops before writing it. Until the
firmware fits, a stand-in is made from the first 283,360 bytes of `app.bin` (**for the simulated FM-1 only, never
to install**: its code is cut off):

```sh
python3 - <<'EOF'
import re, subprocess
bank = int(re.search(r"BANK_AT = (0x[0-9a-fA-F]+)", open("tools/mkfw.py").read()).group(1), 16)
open("build/fw/app-twin-only.bin", "wb").write(open("build/fw/app.bin", "rb").read()[:bank - 0x120])
subprocess.run(["python3", "tools/mkfw.py", "build/fw/app-twin-only.bin", "build/fw/fw.area", "assets/samples.bin"])
EOF
```

## Done so far

1. **The merge.** `git merge upstream/main` (SLOOP 3.4.2, `159e4b4`). Six files had conflicts:
   - `README.md`, `.github/ISSUE_TEMPLATE/bug_report.yml`: Teachie Beachie's kept.
   - `tests/sound_test.c`: SLOOP's (it took the same fix, from our report).
   - `fw/main.c`, two places: `COLD` as SLOOP has it now (`minsize` on the FM-1, so the interface's code is
     compiled for size), and SLOOP's `chain` variables (`chain_arm` is gone).
   - `app/store.c`, `host/hal.c`, `Makefile`: 3.4.1 added a check to the simulated FM-1 (code of the flash window
     run while the window is off). It needs `objcopy` and ELF sections, which a Mac does not have: on a Mac the
     simulated FM-1 is built in one go without that check; on Linux, SLOOP's way is kept.
2. **One thing no longer compiled:** `c_goto` in `fw/course.inc` (the lesson player opening one of SLOOP's pages)
   used 3.3.7's page tables. SLOOP 3.4 keeps each button's pages in `grp_of()` and its last page in `grp_last[]`:
   adapted.
3. **Builds and tests:** `make test` passes; `make build/twin` builds; the firmware compiles and links.
4. **A first look on the simulated FM-1** (no lessons yet): it starts, the start card shows, a card of the guide
   shows, PLAY plays, a pattern is read back, the course reports no lessons (none are in its flash).

## Not done

- **The 30-lesson run** (stopped before it, as asked). From the trial's folder, with the stand-in `fw.area` above:
  `python3 tutorial/device.py` (the lessons as the FM-1 plays them alone; about six minutes) and
  `python3 tutorial/check.py --run` (as the computer's program plays them). Each lesson says "played through" or
  where it stopped.
- **Making the firmware fit** (below).
- **The installer page:** the build stops before it, so `tools/teachie_page.py` has not met SLOOP's new page
  template yet. Its rules report any sentence they no longer find.
- **A real FM-1:** nothing from here has been on one.

## The room

| | bytes |
|---|---|
| The merged firmware (`build/fw/app.bin`) | 289,828 |
| Room before the sample bank (3.4.2 moved the bank 5 KB later) | 283,360 |
| **Over by** | **6,468** |

Remembering the lesson after power-off (NOTES, item 118) needs about 100 more. What could come out, as measured in
this build's `ram.elf` (`llvm-nm --print-size`):

| What | bytes | What is lost |
|---|---|---|
| The visualizer: `vis_draw`, `vis_frame` and their callers | 3,777 and a little more | HOME on the tracks page shows nothing. No lesson uses it, and learners fall into it by accident |
| What is left of SLOOP 2.8's import (presets, favorites, kits, bank) | 3,418 | A SLOOP 2.8 user's own sounds do not come along at the first start (the project already does not) |
| Teachie Beachie's logo (`tp_logo_rle`) | 2,118 | Nothing, if it moves into the lesson pack; with no lessons on the FM-1 the start would show the name only |
| SLOOP's demo song | 440 | The demo |

The visualizer and the SLOOP 2.8 import together are about 7.2 KB: enough, with about 700 bytes to spare. Each
would go behind a switch of our own, as `TP_SLOOP28_PROJECT` does, so SLOOP's files stay mergeable.

## What will break in the lessons (from the code, before the run)

SLOOP 3.4.0 took SLOOP 2.8.5's pages and gestures. The page numbers the lessons use are unchanged (new pages were
added after the old ones), so nothing is silently wrong; but which button opens what has changed:

- **SEQ on a synth track opens the piano roll (NOTES) first,** then PATTERN, then SONG. The page of boxes (STEPS,
  now called GRID) is the drums' only. Lesson 8 writes a bass line on that page ("Press [SEQ]. The same 16
  boxes."), and lesson 13 says the piano roll is "[SEQ] twice".
- **EDIT's pages** are now OSC, FILTER, the analog filter, VOICE, VOICE 2, SOUNDS. PLUS is gone and YOURS (user
  sounds) moved to SAVE. Lessons 9, 20 and 21 step through EDIT.
- **FX's pages** are SENDS, a track filter, SLICER, INSERT, INSERT 2, DLY, REV/CHO. MASTER moved to GLO. Lessons
  18 to 20 and 27 step through FX.
- **LFO's pages** are LFO, LFO 2, LFO DEST (the last two swapped). Lesson 22.
- **SAVE** is the largest change. On the tracks page it opens the SONG screen, and from there PROJECT (SAVE, NEW,
  LOAD); elsewhere it steps through PRESETS, USER, PATTERNS, TOOLS. SAVE held + a key plays that pattern as saved,
  several keys make a chain, and holding 1.5 s saves into it; the eight patterns change only when saved. Lessons 16
  and 23 (copy a page), 24 to 26 (the song), 23, 29 and 30 ("press [SAVE] twice").
- **The tracks page** shows the pattern as a number in a square at the top right, not `P1`. Lesson 16's new cards
  say "P1, top right".
- **Saved projects** are format 16, which 3.3.x cannot read.

## To check

- The firmware build now says `xipcheck: ... roots not found: trs_midi_poll`. It did not on 3.3.7. See whether
  SLOOP's own 3.4.2 build says the same before reading anything into it.
- 3.4.1's fixes for the random freezes are in this merge. They are the main reason to move.
