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

## The 30-lesson run (2026-11-24)

`python3 tutorial/device.py` on the merged firmware (the stand-in `fw.area` above), the lessons as the FM-1 plays
them alone. **21 played through, 9 stopped.**

- **Played through:** 1 to 5, 9 to 12, 14, 17 to 22, 24, 27 to 30. Lessons 9 (EDIT to SOUNDS) and 18 to 22 (the FX,
  LFO and ARP pages) were expected to break and did not.
- **Stopped, because [SAVE] works differently** (SLOOP 3.4.0):
  - lesson 6, step 18, `Press [SAVE] twice, quickly.`: no PROJECT SAVED (saving is on the PROJECT page now);
  - lesson 16, step 6, and lesson 23, step 4, the copy of a page (`[SAVE]` + `[2]` held): no copy is made (a key
    held with SAVE saves into that pattern after 1.5 s; the eight patterns change only when saved);
  - lessons 25 and 26, step 3, `Press [SAVE] once: the SONG page.`: SAVE on the tracks page opens SONG 2/3
    (`PG_SONG2`), not the page the step waits for (`PG_SONG`).
- **Stopped, because [SEQ] works differently on a synth track** (SLOOP 3.4.0: NOTES, then PATTERN, then SONG):
  - lesson 8, step 4, `Press [SEQ]. The same 16 boxes.`: SEQ opens the piano roll; the page of boxes is the
    drums' only. STUCK? came up;
  - lesson 13, step 8: "[SEQ] twice" now lands on PATTERN (LEN, DIV, SWG, GATE), not the piano roll, so the knobs
    do something else (the screen says `PATTERN 2/2`).
- **Stopped, because the drum grid works differently** (SLOOP 3.4: SLOOP 2.8.5's grid): lessons 7 and 15, step
  4, a beat of one's own. Found by replaying the robot's presses one by one on both firmwares, reading the drums
  after each release (the script is below):

  | The robot's press | 3.3.7 | 3.4.2 |
  |---|---|---|
  | white key 1 alone, to pick the kick | picks the kick, writes nothing | writes a kick on box 1 |
  | [SEQ] + 1, 5, 9, 13 | four kicks | box 1 toggled off; 5, 9, 13 on |
  | white key 3 alone, to pick the snare | picks the snare | writes a kick on box 3: still the kick |
  | [SEQ] + 5, 13 | two snares | takes the kicks on 5 and 13 out |
  | in the end | 6 notes: through | 2 notes (lesson 15: 3): "not yet" again and again |

  On 3.4.2's drum grid (`docs/GUIDE.md`, "DRUMS grid"; `fw/main.c`, `grid_on`, `drum_black`): **the white keys are
  the 16 boxes, with or without [SEQ]** (each tap sets or clears that box), **the black keys pick the drum** (each
  black key between two drums), and so does KNOB 1 (SOUND). Playing a drum's white key no longer picks it.
- **The tracks page has not changed.** Tested on both firmwares (a second script, `tutorial/upgrade/drumkeys.py OUTDIR`: a key tapped, its
  sound measured from the simulated FM-1's audio, the drum pattern read, the grid's header shown after):

  | | 3.3.7 | 3.4.2 |
  |---|---|---|
  | tracks page, a white key | sounds; picks that drum (the grid then says SNARE) | the same |
  | tracks page, a black key | sounds; picks a drum | the same |
  | drum grid, a white key | sounds; picks that drum | silent; sets or clears that box for the drum picked |
  | drum grid, a black key | silent; picks a drum | the same |

  So a drum picked by playing it on the tracks page is still the one the grid writes; only the grid's white keys
  changed. The grid's knobs changed too: SOUND, STEP, HIT, LEVEL (3.3.7: SOUND, QUANT, GATE, STEPS).
- **Lesson 4 is affected as well:** `Play every white key. The top names each drum.` is on the grid (`Page:
  STEPS`). On 3.4.2 that writes sixteen silent boxes. Its echo step checks keys, not sounds, so the robot passed.
- **Lessons that passed may have passed by accident.** Every lesson that picks a drum by playing its key does it on
  the grid, where that key now writes a box: lesson 2 step 7 (`Play [KEY 1]... the kick`), lesson 3 steps 4, 5 and
  9 (`Each key is a different drum. Play one to pick it.`, `Pick the snare: play [KEY 3].`, `Now pick the hat:
  play [5].`), lesson 16 step 10, lesson 17 step 10 (`Pick the high tom: [11]`), lesson 28 step 9 (`Play [5]: the
  hat.`), and the cards of lessons 7 and 30 (`Pick a drum: play its key.`). The robot reaches their checks anyway,
  with stray boxes written on the way; a learner would be told the wrong thing. They all need rewriting for the new
  grid: pick with a black key or KNOB 1, and tap a white key for a box.

**How long it took:** the first run (16:58 to 17:15) stopped at lesson 6's report after 16 min 39 s: lessons 7
and 15 went round and round, and `device.py` had no limit for that. It now stops a lesson after 300 s (committed
on this branch). The rerun of lessons 6 to 30 took **5 min 58 s** (17:15:23 to 17:21:21). A whole run with the
limit: about 7 minutes, more if several lessons stop at the limit.

## More checks (2026-11-24, evening)

- **The installer page** built from SLOOP 3.4.2's template with `tools/teachie_page.py`: no rule missed, and both
  page tests pass (`install_sim.js`, `lessons_sim.js`). The editor protocol (`app/edproto.*`) is unchanged, so the
  page's lessons step, `tutorial/tp/fm1.py` and an AI assistant's control talk to 3.4.2 as before.
- **The lessons as the computer leads them** (`python3 tutorial/check.py --run`, 19:00:36 to 19:05:05, **4 min 29
  s**): 19 through, 11 stopped. Each stopped at a known cause:

  | Lesson, step | Waits for | Cause |
  |---|---|---|
  | 3, 6 | snares on 5 and 13 | drum grid: playing [KEY 3] wrote a box instead of picking the snare |
  | 6, 18 | PROJECT SAVED | SAVE |
  | 7, 4 and 15, 4 | the beat (no answer) | drum grid |
  | 8, 4 | the STEPS page | SEQ on a synth track |
  | 12, 10 | chords: 12 notes or more | **new, not explained yet:** recording the four chords live |
  | 16, 6 and 23, 4 | the copy of a page | SAVE |
  | 17, 12 | TOM H on box 12 | drum grid: [11] wrote a box instead of picking the high tom |
  | 25, 3 and 26, 3 | the SONG page | SAVE |

  Lesson 3 and 17 passed the FM-1-led run only by accident, as suspected; this run catches them. Lesson 13 passes
  here and stopped in the FM-1-led run: the two paths get to the piano roll differently.
- **SLOOP's own scenarios for the simulated FM-1** (`sh host/twin.sh 60 "1 2"`): all but two ok.
  - `settings`: "HOME tapped: the menu (0) or a flash of it (1 frames)". A check new in 3.3.8 (`host/twin.c`,
    `sc_settings`): a tap of HOME must not show any page but the tracks page or the visualizer for a single frame.
    Likely Teachie Beachie's start card, which is up at power-on in the twin and is not SLOOP's tracks page. To
    confirm.
  - `bigproj`: "the interface stood still 3.0 s (the sound going on): a freeze", during the fullest project's
    saves. A check new in 3.4.1 (the freeze SLOOP fixed). **Important to look into:** if Teachie Beachie's own code
    (the guide, the course, `COLD`, run from the flash window) runs while a save has the window off, the FM-1
    freezes. On a Mac the twin's check of exactly that (`tw_xt`) is off, so this scenario is the only warning.
    Could also be the Mac build being slower; to be told apart on Linux, or by turning the guide off in the twin.

## The review of SLOOP's changes (2026-11-24, evening)

Everything the lessons rely on was listed by a script (`tutorial/upgrade/inventory.md`), then SLOOP's changes from
3.3.7 to 3.4.2 were read against it in three parts: `fw/main.c` with our `fw/course.inc`; what the pages show
(`app/ui.c`, page and knob names, fonts); the sequencer, saving, the song and the sounds. Read from the code; none
of this is seen on a real FM-1 yet. Beyond what is above (SAVE, SEQ on a synth track, the drum grid, the page
orders):

**Would break a lesson or teach the wrong thing**

| What changed | Lessons |
|---|---|
| **Each pattern keeps a saved copy (`keep[]`); leaving a pattern puts that copy back.** Our lesson setups write only the playing pattern and never update the saved copy, so a lesson's song on page 1 empties the first time the FM-1 leaves page 1, and a learner's unsaved changes vanish on a page switch (`fw/main.c` 462-476, `if (was < SEQ_NPAT) Q.t[t].pat[was] = keep[t];`). Fix for the setups: `keep_all()` after `c_setup`'s writes (`fw/course.inc`); the learner's side needs teaching (save into a pattern). The computer's setups (`ED_NOTES`) have the same gap. | 16, 23, 24, 25, 26, 30 |
| SAVE + a key held 1.5 s **saves into** that pattern and **does not switch** to it; the screen says `PATTERN 2 FROM 1 · SAVING`, never COPY. SAVE + a tap on the playing pattern's key **puts back its saved copy** on every track (`AS SAVED`). | 16, 23 |
| **[15] is no longer CHAIN:** it switches `PATTERNS: THIS TRACK ONLY` / every track, and stays switched; [16] opens SONG. A chain is now several pattern keys tapped within one SAVE hold; `CHAIN DONE` is never said. If track-only is left on, lesson 26's line check reads the wrong track. | 25, 26, 30 |
| SAVE + [13] plays the song **from the LINE shown** (lesson 26 step 16 skips the intro). | 26 |
| **SEL tapped opens `SEL 1/2`, a knob page with no tiles:** ROOT, SCL, QNT, CHORD (was ROOT, SCALE, CHORD, KEYS). [KNOB4] now turns chords on (TRIAD), not SNAP; [KNOB3] now SNAP, not chords. This is why lesson 12 stopped in the computer-led run. | 11, 12 |
| **HOME held 0.7 s opens a MENU** (SCREEN, LIGHTS ...) that stays open; there is no PROJECT page with OPEN, NEW, SAVE (keys 1, 3, 5 still work, unlabelled); a second HOME + [1] within 0.3 s is ignored. | 29 (and any slow `Press [HOME]`) |
| On the drum grid **OCT+ held 0.4 s opens the hit SHEET** and OCT- turns the page: soft and hard hits by hand (OCT- / OCT+ held + a drum) no longer work there. | 28 |
| **Sound pages are laid out the other way up** (SLOOP 2.8.5's): knob labels and values at the top, the page's title small and grey at the bottom right. Bottom strips now hide the title the learner is asked to find; one-line top strips cover the knob labels. | 11, 12, 19, 20, 21, 22 |
| **Names on screen:** SENDS is `FX 1/7` (its ECHO knob is `DLY`), DELAY is `DLY 6/7`, FX held: FILT is `FILTER` (FILT is now the track filter, on FX 2/7), SCALE is `SCL`; EDIT now has `FILTER 3/6` next to `FLT 2/6`. | 11, 19, 20, 21, 27 |
| **Undo keeps 3 moves** (was 8). Lesson 6's card says 8; its step needing 3 undos still works. | 6 |
| The FX + HOME lock happens on HOME's release, and only if FX is still held. | 18 (likely) |
| Our player's message `That played a drum. Hold [SEQ] first.` now fires when a learner picks a drum on the grid (black keys, KNOB1). `pressed SAVE twice` passes though nothing is saved. | 7, 15, 30 |

**To look into:** a chain playing during a save may run code from the flash window while it is off (SLOOP's own,
low risk); the `bigproj` freeze above.

**Unchanged** (checked): every sound and kit name the lessons and HOUSE ONE use, and their order; the editor
protocol and the LIVE status; conditions, FILL / NO FILL, GLO's mutes, solos and fills; ARP and rolls; ENV, LFO
RATE and LFO DEST; INSERT; the piano roll's STEP, NOTE, LEN; the tracks page's knobs and tempo; SEQ + a step held
(`STEP 12`); the SOUNDS page and favorites; the messages UNDONE, KEY A, PROJECT SAVED, LOOP, the SONG messages;
undo and redo; REC, erase, count-in. Harmless: a new project's tracks all at level 45, leads POLY, the mix 6 dB
hotter, the project format (17). Our cards keep their own font; SLOOP's pages draw text a little narrower.

**What it adds up to:** nine lessons stop, and about fifteen more need their words or steps changed. With the
firmware's room (6.5 KB to cut) and the setups' saved copy, moving to 3.4.2 is a rewrite of most of the course,
not an update.

## Not done

- **The fixes** for the nine stopped lessons and the drum lessons above: mostly the lessons' words and steps
  (SAVE, SEQ, the drum grid), and in a few places what the lesson player checks (the SONG page's number, STEPS).
- **How the replay was done,** to repeat it on another step: `tutorial/upgrade/replay.py LESSON STEP OUTDIR`, run
  from a copy's top folder. It plays the lesson as `device.py` does up to the
  step, then makes that step's robot moves exactly as `L.moves()` gives them (presses, releases, timing), and after
  each release prints the FM-1's phase, step and drum notes (`f.pattern(3)`), with a screenshot. The control ran
  on today's 3.3.7 build through a folder of links to the main copy. Running moves one at a time through
  `device.hands()` is wrong: it lets go of what is held between them.
- **`python3 tutorial/check.py --run`** (the lessons as the computer's program plays them): not run.
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
