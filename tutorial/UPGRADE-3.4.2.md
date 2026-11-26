# Trial: Teachie Beachie on SLOOP 3.4.2

A trial, not for `main`. It answers one question: what does it take to move Teachie Beachie from SLOOP 3.3.7 to
SLOOP 3.4.2? Started on 2026-11-24. Nothing here is installed on an FM-1, merged into `main`, or published.

## Where it is

- **Branch:** `trial/sloop-3.4.2`, pushed to GitHub (since 2026-11-24, at the course's author's word, so a cloud
  session can pick it up). Not merged into `main`, which serves the live page.
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
dash host/twin.sh 60 "1 2"                  # SLOOP's scenarios on it: all pass (18 s). On a Mac use dash, not sh: see below
EDITOR_E2E=0 INSTALLER_E2E=0 sh fw/build.sh "$(git rev-parse --git-common-dir)/../toolchain/bin"
                                            # the firmware, to the end: build/fw/fw.area, and its five rooms (tools/tp_room.py)
python3 tutorial/upgrade/keepcheck.py       # a lesson's song kept over a change of pattern (step 2)
python3 tutorial/upgrade/drumcheck.py       # the drum picked: the lesson's check of it, its words for the wrong one (step 2)
git checkout -- docs/index.html tutorial/lessons/screens   # after a build or a lesson run, until step 9: what they rewrote
```

After a change to the firmware, `make build/twin` before `fw/build.sh`: the build's `tests/lessons_sim.js` runs the
simulated FM-1 as it is.

The two Playwright tests `fw/build.sh` runs last (`tests/editor_e2e.js`, `tests/installer_e2e.js`) have never run on
this Mac: Playwright is not installed, and `EDITOR_E2E=0 INSTALLER_E2E=0` leaves them out. The installer page is
tested in the Claude app's built-in browser instead, as it was on 2026-11-21: a preview server
(`installer-page-trial` in the main copy's `.claude/launch.json`, not in git) serves this folder's `build/fw` on port
8792; a pretend FM-1 put into the page by script (`navigator.requestMIDIAccess`, answering ED_GUIDE and ED_SAMPLE as
`tests/lessons_sim.js` does) takes the lessons; what it got is read back.

Three files go with this note: `tutorial/upgrade/decisions.md` (the questions for the course's author, to be
answered before the work starts), `tutorial/upgrade/worksheet.md` (every step that touches a change, with today's
words; made by `tutorial/upgrade/worksheet.py`), and `tutorial/upgrade/inventory.md` (everything the lessons rely
on).

A clean copy of SLOOP's own 3.4.2, for comparing what is SLOOP's and what is ours: `.worktrees/sloop-upstream-342`
(detached at `159e4b4`, with only the Mac build fixes, not committed). Its `build/twin` and `build/fw/fw.area` are
built.

The simulated FM-1 needs `build/fw/fw.area` for its sample bank. Since step 1 (2026-11-25) the firmware fits and
`fw/build.sh` writes a real one; the stand-in made from a cut-off `app.bin` before step 1 is no longer needed.

## The plan for the full move

In this order; each step ends with its checks passing and is a commit on this branch (when the user says so).
Nothing goes to `main`, is pushed or is installed without the user's say.

0. **Read the answers** in `tutorial/upgrade/decisions.md`. Where one is blank, ask before the step that needs it.
1. **Make room** (decision 1; **done 2026-11-25**: "Step 1: the room made", below): the cuts behind switches of our own (as `TP_SLOOP28_PROJECT`), until
   `fw/build.sh` writes a real `build/fw/fw.area`. Then the stand-in below is no longer needed. Checks:
   `make test`, `make build/twin`, `dash host/twin.sh 60 "1 2"`, `fw/build.sh` to the end.
   - **The logo into the lesson slot** (decision 1, changed 2026-11-25): full colour, flat with smooth edges, as
     large as the screen allows over WITH SLOOP 3; after the pack, outside its u16 offsets, leaving the blank bytes
     after the pack for step 8. Today's picture is a stand-in until the course's author picks the new one: build
     the way it is drawn, not the picture. The start screen checked in a screenshot of the simulated FM-1, with
     the lessons on it and without.
2. **The lesson player** (`fw/course.inc`, `tools/mkcourse.py`; a firmware change, so said first; **done 2026-11-25**,
   and reviewed: "Step 2: the lesson player" and "The review of steps 1 and 2", below):
   - `keep_all()` after `c_setup`'s writes, so a lesson's song is the patterns' saved copy and does not empty when
     the FM-1 leaves page 1 (the review's first table). The computer's setups (`ED_NOTES`, `tutorial/tp/teach.py`)
     have the same gap.
   - `That played a drum.\nHold [SEQ] first.` (`fw/course.inc` line 481): wrong on the new grid, where picking a
     drum is right (decision 2).
   - `pressed SAVE twice` passes though nothing is saved (decision 6); `CHAIN DONE` is never said (decision 5);
     SAVE on the tracks page opens `SONG 2/3`, not `SONG` (lessons 25 and 26).
   - A new mark for the pack (`TPC4`), so a 3.3.7 Teachie Beachie finds no 3.4.2 lessons ("Step 1: the room made",
     below).
3. **The icons** (decision 12; the pack, no firmware; **the black key's picture done 2026-11-25**: "Step 3", below):
   the black key's picture first, drawn into one of the nine
   spare characters by `tools/mklessonfont.py`, its word in `tools/mkcourse.py`, `tutorial/tp/teach.py` and
   `tutorial/lessons/README.md`. Shown to the course's author before a lesson uses it. Other pictures only where
   the lessons in step 4 show a need.
4. **The lessons, a part at a time** (**begun 2026-11-25**: "Step 4: the lessons", below), from `tutorial/upgrade/worksheet.md`: part 1 (1 to 7, the drum grid), part 2
   (8 to 15: SEQ, SEL, the piano roll), part 3 (16 to 23: patterns, the sound pages), part 4 (24 to 30: chains,
   songs, saving). Each change shown first as Today / Suggested, one screen line per line, measured. After each
   lesson: `python3 tutorial/check.py N`, `--run N`, `tutorial/device.py N`. Their notes for the AI assistant too.
   The first card that shows the black key's picture (`[BLACK 1]`) says what it is.
5. **The whole course:** `python3 tutorial/device.py` (about 7 min) and `python3 tutorial/check.py --run` (about
   4.5 min): all 30 through.
6. **The frames that answer the controls** (decision 11; the guide in `fw/main.c`, a firmware change, so said
   first). Measured after it: the room left, the load with a knob spun fast (`ld_peak`), screenshots of a frame
   held and one half filled; then step 5's runs again.
7. **The screen colors** (decision 8): M-VAVE's table in `lcd_init`, behind a switch of our own (the section on
   newer FM-1s, below).
8. **Remembering the lesson** (decision 9; `tutorial/NOTES.md` item 118), if there is room and time. Its rules, from
   the review (finding 4):
   - It writes into the slot's blank bytes, between the pack's end (the pack's own length, at 4) and the logo (its
     place, in the slot's header at 24). `tools/mkcourse.py` keeps 1,024 of them at least (`RESERVE`).
   - **One new byte for each record, and no byte written twice.** The original FM-1's flash is Puya's (its code
     begins `85`), and Puya's chips are known for not taking a second write into a byte that already holds
     something. A blank byte written once is the ordinary case.
   - **To try on a real FM-1 (step 10):** a byte written there and read back. The install now sends those bytes as
     blank (0xFF); that should be the same to the chip as leaving them, and the simulated FM-1 cannot say.
   - The header's CRC covers the blank bytes and stops matching at the first record: nothing reads it after the
     install, and nothing should. When the blank bytes are used up, no more is written (the last record stays).
9. **The words around it:** SLOOP's version in `README.md`, `TEACHIE-BEACHIE.md`, the installer page's words and
   `tools/teachie_page.py`'s rules, `AGENTS.md` and `tutorial/AI.md` where they name buttons that changed (and
   the frames that answer, and the icons), `tutorial/NOTES.md`; the lesson screens (`tutorial/lessons/screens/`);
   the page rebuilt into `docs/`.
10. **A real FM-1** (decision 10): a backup first (a project saved by 3.4.x can't be opened by 3.3.x). From the
    review:
    - **Before it:** SLOOP's scenarios and the 30 lessons once on Linux, where the simulated FM-1's own check of the
      flash window works (a cloud session can: the branch is on GitHub).
    - **On it:** the start screen (the logo there at once, no strips, the buttons quick); a byte written into the
      slot's blank part and read back (step 8); lessons sent over other lessons with the start card up.

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
  | 12, 10 | chords: 12 notes or more | SEL's knobs: [KNOB3] is SNAP now, so chords were never turned on (the review, below) |
  | 16, 6 and 23, 4 | the copy of a page | SAVE |
  | 17, 12 | TOM H on box 12 | drum grid: [11] wrote a box instead of picking the high tom |
  | 25, 3 and 26, 3 | the SONG page | SAVE |

  Lesson 3 and 17 passed the FM-1-led run only by accident, as suspected; this run catches them. Lesson 13 passes
  here and stopped in the FM-1-led run: the two paths get to the piano roll differently.
- **SLOOP's own scenarios for the simulated FM-1** (`sh host/twin.sh 60 "1 2"`): all but two ok, `settings`
  ("HOME tapped: the menu (0) or a flash of it (1 frames)") and `bigproj` ("the interface stood still 3.0 s (the
  sound going on): a freeze"). **Settled, not ours** (2026-11-24, late evening):
  - **The Mac's `sh` caused both.** `host/twin.sh` runs some scenarios with a setting of their own,
    `TWIN_DRAW_US=1500 run boot-slow ...` and `TWIN_ERASE_MS=300 run oversave ...`. On a Mac `sh` is bash 3.2 in
    POSIX mode, which keeps such a setting for the rest of the script when it comes before a function. So every
    scenario after `boot-slow` drew slowly, and every one after `oversave` erased slowly. Linux's `sh` (dash) does
    not; macOS has dash too, `/bin/dash`.
  - **The slow drawing alone fails `settings`, the slow erases alone fail `bigproj`,** in exactly the same way on
    SLOOP's own 3.4.2 (the clean copy above) as on the trial. Without them both pass, on both, also with every
    core of the computer busy.
  - **With `dash host/twin.sh 60 "1 2"` the trial passes everything:** 99 lines ok, none failed, in 18 s.
  - **For SLOOP's author, perhaps:** with 300 ms erases (a real sector erase can take 400 ms, `hal/fm1/flash.h`)
    SLOOP's own 3.4.2 also stands still 3 s while it saves the fullest project. SLOOP's tests do not run it that
    way on Linux. Not Teachie Beachie's to fix.

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
low risk). The `bigproj` freeze above is settled: the Mac's `sh`, not ours.

**Unchanged** (checked): every sound and kit name the lessons and HOUSE ONE use, and their order; the editor
protocol and the LIVE status; conditions, FILL / NO FILL, GLO's mutes, solos and fills; ARP and rolls; ENV, LFO
RATE and LFO DEST; INSERT; the piano roll's STEP, NOTE, LEN; the tracks page's knobs and tempo; SEQ + a step held
(`STEP 12`); the SOUNDS page and favorites; the messages UNDONE, KEY A, PROJECT SAVED, LOOP, the SONG messages;
undo and redo; REC, erase, count-in. Harmless: a new project's tracks all at level 45, leads POLY, the mix 6 dB
hotter, the project format (17). Our cards keep their own font; SLOOP's pages draw text a little narrower.

**What it adds up to:** nine lessons stop, and about fifteen more need their words or steps changed. With the
firmware's room (6.5 KB to cut) and the setups' saved copy, moving to 3.4.2 is a rewrite of most of the course,
not an update.

## The newer FM-1s: the flash chip and the screen (2026-11-24)

Asked because a friend's FM-1 has the newer flash chip, the UCUN UC25HQ80 (M-Flasher shows FLASH CHIP
`0xB36014`); the tester's has the original (`0x856014`). Found on GitHub (no report on SLOOP 3's own tracker, good or
bad):

- **m-bootloader on the new chip:** the developer, asked on 2026-11-21 whether 2.4 was tested on it: "1. Yes. 2. No
  [known issues]. 3. Install it." And holding REC while switching on "bypasses almost all the bootloader code so
  you should be back to download mode with a very high degree of confidence"
  ([m-bootloader#37](https://github.com/jvitkauskas/m-bootloader/issues/37)). Earlier, one new-chip unit did not
  start after m-bootloader 1.1 or 1.2; 3.0's release notes still list the chip under "Not tested".
- **M-Flasher on the new chip:** an open report that it did not find a new-chip FM-1 in download mode; restarting
  the installer got round it ([m-flasher#64](https://github.com/jvitkauskas/m-flasher/issues/64)).

### The screen: faded colors on newer units

- **The report** ([sloop-fm1#269](https://github.com/isod89/sloop-fm1/issues/269), SLOOP 2.8, open, 2026-11-19, by
  an owner of a newer unit, chip key `0x980F`): on M-VAVE's own firmware (V15, V16) the colors are right; on SLOOP
  2.8.5 and other custom firmware they are faded: greys turn white, and the drum grid's GHOST, SOFT and NORM look
  the same. On a test card the grey ramps reach full level within the first 2 of 32 steps.
- **Why:** the screen's controller (an ST7789V) is told its voltages, VCOM and gamma at start-up by M-VAVE's
  firmware, with one table, the same in V15 and V16, on every unit. SLOOP leaves them all at the controller's reset
  values. The original units look right that way; the newer ones' panel does not.
- **SLOOP 3 does the same:** `lcd_init()` in `hal/fm1/board.c` (unchanged from 3.3.7 to 3.4.2) sends only reset
  (`01`), sleep out (`11`), the pixel format (`3A` `55`), `36` `00`, inversion on (`21`), normal mode (`13`) and
  display on (`29`). So Teachie Beachie, on 3.3.7 or 3.4.2, very likely looks faded on a newer unit: the peach cards
  paler, and the subtle shades (the drum grid's levels, the keys' faint glow, dim against bright) hard to tell
  apart.
- **M-VAVE's table, as the report gives it** (`COLMOD 55`, `MADCTL 00` and `INVON` are the same as SLOOP's):

  | Register | | Values |
  |---|---|---|
  | `B2` | PORCTRL | `0C 0C 0C 00 33` |
  | `B7` | GCTRL | `56` |
  | `BB` | VCOMS | `18` |
  | `C0` | LCMCTRL | `2C` |
  | `C2` | VDVVRHEN | `01` |
  | `C3` | VRHS | `1F` |
  | `C4` | VDVS | `20` |
  | `C6` | FRCTRL2 | `0F` |
  | `D0` | PWCTRL1 | `A6 A1` |
  | `E0` | PVGAMCTRL | `D0 0D 14 0B 0B 07 3A 44 50 08 13 13 2D 32` |
  | `E1` | NVGAMCTRL | `D0 0D 14 0B 0B 07 3A 44 50 08 13 13 2D 32` |

- **The reporter's fix, tested on their unit** (one commit on SLOOP 2.8.5,
  [Tragiciy/sloop-fm1@73f00ee](https://github.com/Tragiciy/sloop-fm1/commit/73f00ee270fde8c08bd924e16cb20628abe0ce95)):
  the table sent after the base setup and before display on (`29`), on every FM-1, as M-VAVE's firmware does; a
  switch to build without it; a test that records the commands `lcd_init` sends. About 80 bytes on SLOOP 2.8.5.
- **For Teachie Beachie:** the same table in `lcd_init()`, between `13` and `29`: a short table and a loop, perhaps
  40 to 80 bytes. On 3.3.7 that is most of the 92 free (the visualizer's cut would give room); on 3.4.2 it is part
  of the room to make anyway. It is SLOOP's file, so behind a switch of our own, and worth offering to SLOOP 3 too.
  **To check:** the original units look the same with it (the tester's FM-1), and the newer ones look right (the
  friend's). M-VAVE sends the same table to both, so the first should hold.

## Step 1: the room made (2026-11-25)

Decision 1, as answered: the visualizer and SLOOP 2.8's import out, the logo into the lesson slot. **The firmware
fits, with 6,400 bytes to spare** (`build/fw/app.bin` 276,960 of 283,360).

- **Two switches of our own,** as `TP_SLOOP28_PROJECT`: SLOOP's code stays in its files, left out unless the switch
  is defined (no build defines them):
  - `TP_VISUALIZER`: `fw/main.c` (HOME on TRACKS opening it, its pass, SELECT choosing its style, its ring) and
    `app/ui.c` (its page drawn). HOME on the tracks page now does nothing. `app/vis.c`'s pictures are untouched;
    the linker drops what nothing calls.
  - `TP_SLOOP28_DATA`: what was left of reading SLOOP 2.8's data at the start (`fw/main.c`, `boot_flash`): its user
    sounds, favourites and colour theme, its SYN kits, MIDI LEARN and MIDI settings, the place of its FM6 bank.
    Samples in the user slots still come along (they are not an import). Its menu settings and calibration were cut
    too at first, and came back in the review (finding 6: "Yes, bring them back").
  - The three unused-variable warnings this made are quieted (`vis_named`, `vis_ring`, `home_had`).
  - The two cuts gave 10.8 KB, not the 7.2 KB estimated: the linker dropped more of the import than `llvm-nm`
    counted.
- **SLOOP's `vis` scenario** (`host/twin.c`, `sc_vis`, run twice by `host/twin.sh`): without `TP_VISUALIZER` it checks
  that HOME on TRACKS stays on TRACKS, then goes on to what it checks after the visualizer (SAVE on the SONG
  screen, PROJECT > SAVE). SLOOP's own checks are kept under the switch.
- **The logo in the lesson slot.** `tools/mklogo.py` is rewritten: the picture made flat in its 8 colours at 8 times
  the size, then shrunk, each pixel the mean of its 8 x 8: flat colours with smooth edges, in full colour (RGB565),
  174 x 204 (was 168 x 197), 18,217 bytes, 818 colours. Its file, `assets/logo/teachie-beachie-fm1.bin` (the format
  is at the top of the tool), is kept in the repository as the lessons' font is, so packing the lessons needs no
  Pillow. `app/tp_logo.h` is gone, and with it 2,118 bytes of the firmware.
  - **The slot** (`tools/mkcourse.py`, `slot_image`): the pack, then blank bytes (0xFF), then the logo ending at the
    slot's last byte; the header's version is 2, the logo's place and length at 24 and 28. SLOOP's slot writing
    takes data in one run from the start and erases a sector only when it writes it, so the blank bytes are sent:
    they are then sure to be blank, for step 8 (8,633 of them today). An install sends 81,408 bytes (was 54,558):
    two sectors more than the pack and the logo alone.
  - **The firmware:** `course_find` (`fw/course.inc`) finds the logo with the pack, at the start and whenever lessons
    arrive; `vis_splash_draw` (`app/vis.c`, our part) draws it from the slot, 4 px from the top, WITH SLOOP 3 under
    it as before. With no lessons on the FM-1, the name instead: TEACHIE / BEACHIE in SLOOP's large capitals. Since
    the review (finding 9) the start is drawn when it changes, not each pass.
  - **Checked:** the tool reads its file back the decoder's way before writing it; the simulated FM-1's start
    screen, with the lessons, is the file's picture pixel for pixel (0 of 35,496 differ); screenshots with and
    without lessons (`build/start-with-lessons.png`, `build/start-no-lessons.png`, from `build/twin vis OUT FLASH`).
  - `tests/lessons_sim.js` (ours) checks the slot's new shape: the pack at the start, blank up to the logo, the logo
    at the end.
- **Checks, all passing:** `make test`; `make build/twin`; `dash host/twin.sh 60 "1 2"` (99 ok); `fw/build.sh` to the
  end with `install_sim` and `lessons_sim` (the two Playwright tests not run: above); `tutorial/device.py 1`.
- **The page in the built-in browser** (above): the lessons sent to a pretend FM-1 with empty slots: 81,408 bytes in
  USR4, every message in order, the same bytes as the page carries, the pack (54,558), 8,633 blank bytes, the logo
  (`TPL1`, 174 x 204, 18,217 bytes) ending on the slot's last byte, header version 2, 30 lessons found, "Teachie
  Beachie is ready". A full FM-1: the choice of four slots, the question, USR2 written. An older Teachie Beachie
  and SLOOP 3 alone: their messages, nothing written. No console errors.
- **The page read through on 2026-11-25** (every word it can show, `build/page-text.txt`; in the built-in browser at
  desktop size and at the phone preset, 375 x 812: no sideways scroll, the step list scrolls in its own box, the
  screens two to a row). The course's author: "I think we should update the website as the very last step". So
  nothing on it was changed; **for step 9:**
  - **Wrong now:** the last step's "Coming from SLOOP 2.8?" says SLOOP 2.8's presets, favorites, samples, SYN kits,
    FM6 bank and settings come along (`tools/teachie_page.py`, the "first start" rule). Since step 1 and the review only its
    samples, menu settings and calibration do.
  - **Wrong now:** Help, "Updating Teachie Beachie later": "steps 3 to 5" (SLOOP's page has five steps). An update
    to the 3.4.2 Teachie Beachie needs the new lessons too: steps 3 to 6.
  - **Wrong once the lessons change:** the twelve pictures of the course are 3.3.7's: lesson 1 shows `P1`, lesson 3
    "Hold [SEQ]. Hats on ...", lesson 8 the STEPS boxes, lesson 16 copying a page with SAVE, lesson 25 the old
    chain. Made again from the new lessons (`tutorial/lessons/screens/`).
  - **The version:** the page says `v0.1 · SLOOP 3.4.2`. Whether the 3.4.2 Teachie Beachie is 0.2 is the course's
    author's to say.
  - **Checked, right:** the three cheat cards of the last step (SLOOP's own words for 3.4.2): ALGORITHM picks the
    track, PRESETS opens SOUNDS, EDIT held + OCT- undoes (`docs/GUIDE.md`), HOME held lights OPEN, NEW and SAVE
    (`fw/main.c`, 3400). The newer flash chip (step 2's "Good to know", SLOOP's words): m-bootloader 3.0's release
    notes (22 Nov 2026) still list the UC25HQ80 as not tested, and one unit with it did not start after 1.1 or 1.2;
    the maintainer's "Yes." on m-bootloader#37 (21 Nov) contradicts 2.4's own notes. Left as SLOOP says it.
- **For step 2, found while reading the page:** "send the lessons only" on an FM-1 that still runs the 3.3.7
  Teachie Beachie would be taken: its firmware reads the same mark (`TPC3`), so it would play the 3.4.2 lessons
  and teach the old buttons wrongly. A new mark for the 3.4.2 pack (`TPC4`, in `tools/mkcourse.py`,
  `fw/course.inc` and the page's check) makes the old firmware find no lessons, and the page then says to install
  Teachie Beachie first.

## Step 2: the lesson player (2026-11-25)

Said first, then done at the course's author's word ("Yes, start step 2"); reviewed and corrected the same day ("The
review of steps 1 and 2", below). This is how it stands after the review.

- **A lesson's song is the patterns' saved copy.** On 3.4.2 leaving a pattern puts its saved copy (`keep[]`) back,
  so a setup that wrote only what plays emptied the song at the first change of pattern.
  - **The rule:** a setup saves the tracks whose playing pattern it wrote, and no other. A song: all five. `patterns A
    B C D` and `pattern 2: section C`: the tracks on which that pattern is the one playing (a pattern that is not
    playing is its own saved copy already). `fill empty tracks`: the tracks it filled. `empty`, `steps`, `add`,
    `sound`: that track. A new project saves its own (`new_project`). A setup that only plays, stops, mutes or turns
    to a page saves nothing. Levels and sends are the mix, not the pattern's (`app/psound.h`), and need nothing.
  - **On the FM-1** (`fw/course.inc`): `c_clip_load` answers which tracks' playing pattern it wrote; `c_setup` makes
    those their saved copy at its end, notes and sound (`commit_trk`).
  - **From the computer:** an op of our guide, `ED_GUIDE 7` with a byte of tracks (`fw/main.c`), does the same;
    `tutorial/tp/fm1.py` `keep()`, and `tutorial/tp/lesson.py` `setup()` sends it by the same rule. A firmware
    before it answers and does nothing.
  - **Checked:** `tutorial/upgrade/keepcheck.py`, four cases: the computer's setup with its keep left out (72 drum
    notes and 20 bass notes become 0 after SAVE + white key 2, then SAVE + white key 1: the problem, seen); as it
    is (all back, the lead still FAT LEAD); the FM-1's own course, lesson 24's setup (all back); and a note of the
    learner's own on the drums, not saved, then `fill empty tracks` (gone after a change of pattern, as SLOOP has
    it: the lesson did not save it by the way).
- **The drum picked** (decision 2: on the grid a black key picks, a white key is a box).
  - **A new `Until` word, `drums picked SNARE`** (`CC_PICKED`, 32; `tools/mkcourse.py`, `app/tp_course.h`,
    `fw/course.inc`, `tutorial/lessons/README.md`): a step can wait for the drum that boxes write. Without it a
    "pick the snare" step ended at the tap of a black key, whichever drum that gave (a pair's key tapped twice
    gives the other: SNR 2, KICK2).
  - **The FM-1 says which drum is picked:** bits 4 to 7 of the state byte in our guide's answer (`fw/main.c`,
    `ED_GUIDE`). `tutorial/tp/fm1.py` keeps it as `picked`; `tp/lesson.py` takes the FM-1's word over its own
    guess from the presses.
  - **Another drum picked than a step's boxes ask for:** that drum's black key blinks, not the boxes, until it is
    picked, and the lesson says so once for each wrong drum, for 2.4 s: `Another drum is picked. / Tap the blinking
    key.` (201 and 172 px of 220; "Yes, add them"). The same on the FM-1 (`fw/course.inc`) and from the computer
    (`tp/lesson.py`). SLOOP's own lights stay under a strip: the black key of the drum picked is lit, the keys glow.
  - **`That played a drum. / Hold [SEQ] first.` is gone**, on both: on 3.4.2 picking a drum is the right move. The
    lesson no longer puts the drum back by itself.
  - **The computer's own picking** (`tp/lesson.py`): its `pick` (a setup's `pick KICK`, a step's start) goes to
    TRACKS first if the FM-1 is on the grid, and back; its "do it for me" no longer taps the drum's white key first
    (on the grid a stray box).
  - **Checked:** `tutorial/upgrade/drumcheck.py`, with the test lesson in `tutorial/upgrade/drumcheck/`: on the
    FM-1's own course, key by key (the rim's key: the step waits; the snare's: it ends; the snare picked where the
    kick is wanted: the words, the kick's black key blinking and not the boxes, not said twice; the same key again,
    SNR 2: said again; the blinking key: KICK, the boxes blink again; kicks on 1 and 5 and nothing else), and with
    the computer leading. A robot leaves out keys that are no boxes, so the test presses step 3's keys itself.
- **No player change needed** for saving, chains and the SONG page: they are the lessons' words, for step 4, with
  conditions the player already has. Lessons 6, 7, 15, 23 and 29 already wait for `says PROJECT SAVED` (their Do and
  words say "SAVE twice"); lesson 30 waits for `pressed SAVE twice`, which passes with nothing saved: `says PROJECT
  SAVED` instead. Lesson 25's `says CHAIN DONE` is never said: 3.4.2 says `CHAIN 1 2 ...` when two or more pattern
  keys are tapped in one SAVE hold, and `PATTERN n` when one is. Lessons 25 and 26 wait for `page is SONG`, and SAVE
  on the tracks page opens `SONG 2`: `page is SONG 2` (lesson 26 has it already).
- **The pack's mark is `TPC4`** (`tools/mkcourse.py`, `fw/course.inc`, `tests/lessons_sim.js`): a 3.3.7 Teachie
  Beachie (`TPC3`) finds no 3.4.2 lessons, and the page then says to install first.
- **Until step 4 rewrites the drum lessons** their cards say "hold [SEQ]" while a black key may blink, and the
  `--clumsy` robot (a box tapped before SEQ is held) sticks on those steps: it needs a new slip to practise (a black
  key tapped twice), in step 5.

## The review of steps 1 and 2 (2026-11-25)

Steps 1 and 2 were made in a session that ran on Opus by mistake. The course's author asked for them to be looked
over on Fable: "review the work so far with fresh eyes ... Look for anything wrong or risky, in particular code that
might run while the flash window is off, the firmware's room, and the pack layout with the logo after it. Tell me
what you find, one thing at a time, then stop and wait for me before fixing anything". Ten findings, each answered,
then applied together ("Yes, apply them"). Should it all be run again? No: the checks are the same whichever model
ran them, a new run would need this same review, and step 1 was already on GitHub.

1. **The flash window's rule holds; this Mac cannot test it by running.** Everything of ours that reads the lesson
   slot (the lessons, their fonts, the logo) is code in the window itself (`llvm-nm` on `ram.elf`: `course_*`,
   `c_*`, `guide_*`, the logo's drawing in `app/vis.c`), so it cannot run while a save has the window off;
   `tools/xipcheck.py` proves nothing that runs then reaches window code; the main loop does only the sound, the
   panel and the flash while the window is off (`fw/main.c`, `if (!flash_xip_on()) { ... continue; }`) and calls
   the interface, and with it the lesson player, after that. What step 2 touches during a save is `keep[]`, in RAM.
   But SLOOP 3.4.1's check in the running simulated FM-1 needs Linux, and the build's check cannot see a read
   through a pointer (the three into the slot were checked by hand: only window code reads them). **For step 10:**
   SLOOP's scenarios and the 30 lessons once on Linux before a real FM-1.
2. **Lessons sent while a card is on the screen: the simulated FM-1 died.** The FM-1 kept its pointers into the slot
   (the pack, the fonts, since step 1 the logo) while the slot was erased and rewritten, and looked again only at the
   end. A card redrawn in between took its letters from the bytes being replaced. Since step 1 a slot is sent whole,
   so with shorter lessons the blank bytes land where the old fonts were, and a letter's place read from them ran
   past the flash. Tried on the simulated FM-1 (scratch scripts, not kept): all 30 on it and lessons 1 to 15 sent,
   the start card up: 3.3.7 goes on, the trial died (a lesson running: the same; no card: fine; the same 30 again:
   fine; 1 to 15 on it and all 30 sent: fine on both). The installer page puts the start card up first;
   `device.py --install` did nothing first. On a real FM-1 it is not known: a freeze, a restart or junk letters.
   - **Fixed:** when a slot begins to be written or erased (`ED_SAMPLE` 1 and 4, one line of ours in SLOOP's
     handler) and it is the pack's slot, `course_forget` (`fw/course.inc`) stops the FM-1's own lesson and forgets
     the pack, the fonts and the logo; the start card, or a computer's card, is drawn in SLOOP's own letters for
     the seconds of the transfer; `course_found` at its end finds them again and puts the start card back in the
     lessons' font. `tutorial/device.py --install` puts the start card up before and after, as the page does.
   - **Kept as a test:** `tests/lessons_sim.js`, on the simulated FM-1: lessons 1 to 15 (`build/course-short.bin`,
     made by `fw/build.sh`) sent over all 30 with the start card up, then all 30 again. Against the firmware as it
     was, this case fails (the twin stops); now it passes.
3. **The firmware has five limits; the note watched one.** `tools/tp_room.py`, run at the end of `fw/build.sh`,
   prints them all ("The room", below). The second to watch is the RAM for code outside the window and for
   variables: what must run during a save, and every new variable, comes out of it. The two small ones are SLOOP's
   own design, the same on its build. A limit passed stops the build at the link: it cannot reach an FM-1.
4. **The slot's blank bytes were a leftover.** `tools/mkcourse.py` now keeps `RESERVE` (1,024) of them between the
   pack and the logo and refuses a pack that leaves fewer; it prints the slot's three parts (`device.py --install`
   too). The firmware's check of the logo's place no longer adds two numbers that could wrap. **Step 8's rules:**
   below, in the plan.
5. **Some setups saved every track,** the learner's own unsaved work with them (`fill empty tracks`, lesson 15 step
   6; `patterns ...`). Fixed: "Step 2", the rule. Lesson 16 is to teach that an unsaved change goes away (decision 4);
   a lesson that saves behind the learner's back would work against it.
6. **The import cut took more than decision 1 named:** also SLOOP 2.8's colour theme, MIDI LEARN and MIDI settings,
   menu settings and calibration. The calibration is the one whose loss makes buttons and knobs answer wrongly.
   **The menu settings and the calibration are back** ("Yes, bring them back"), as SLOOP reads them, with no
   switch; the rest stays out behind `TP_SLOOP28_DATA`. Cost: about 1 KB of the flash.
7. **The lessons could not check the drum picked.** Added: "Step 2", `drums picked SNARE`.
8. **The wrong drum: words with the blinking key** ("Yes, add them"): "Step 2".
9. **The start screen was drawn about 65 times** (25 times a second for 2.6 s, as SLOOP's logo, which moved), each
   time reading most of the logo again: 18 KB where there were 2. Is there a reason to draw it more than once? No:
   a piece of the screen is sent only when it differs, so the second drawing sends nothing. Now it is drawn when it
   changes, as any other page (`fw/main.c`, `ui_step`; SLOOP's way kept under `TP_SPLASH_MOVES`); `V`, what was
   drawn last, is not zero at the start (`.lowbss`), so a flag of its own says the start was drawn. The simulated
   FM-1 does not model how slow the flash is to read: **for step 10**, a look at the start screen on the FM-1.
10. **The runs rewrite files kept in the project:** `check.py --run` saves a picture of every lesson that plays
    through into `tutorial/lessons/screens/`, and `fw/build.sh` writes the page into `docs/`. Neither is for a
    commit before step 9 (the pictures of lessons half moved; "update the website as the very last step"). **After
    a build or a run, until step 9:** `git checkout -- docs/index.html tutorial/lessons/screens`. The pictures kept
    there are already behind the lessons' words (from before the card reviews of 2026-11-21 to 24); step 9 makes
    all 30 again.

**The checks after it, all passing:** `make test`; `make build/twin`; `dash host/twin.sh 60 "1 2"` (99 ok);
`fw/build.sh` with `install_sim` and `lessons_sim` (its new case too); `tutorial/upgrade/keepcheck.py`;
`tutorial/upgrade/drumcheck.py`; the start screen with the lessons, drawn once, the logo's file pixel for pixel at
0.7 s and 1.7 s, and the name without them. The 30 lessons: on the FM-1's own course (`tutorial/device.py`, 6 min 26 s) 21 play through and the same nine stop at the same steps as before step 2 (6, 7, 8, 13, 15, 16, 23, 25, 26); with the computer leading (`tutorial/check.py --run`, 4 min 12 s) 22 play through and eight stop (6, 7, 8, 15, 16, 23, 25, 26). Lessons 3 and 17 get through since step 2, because the computer now picks the drum the 3.4.2 way at a step's start; their words still teach the old way. Lesson 12 got through this once by accident: the robot repeated its four keys until twelve single notes had piled up, and the chords are still not on ([KNOB3] is SNAP now). All of these are still for step 4.

## Step 3: the black key's picture (2026-11-25)

Decision 12, the first picture. No firmware change: the lessons' fonts are in the pack, and the picture is one of
their letters.

- **The picture:** two white keys in outline and the black key between them, filled, as high as a capital, in the
  letters' ink. It takes the place of `@`, which no lesson's words use, at both sizes (11 x 13 px on cards and
  strips, 21 x 25 on big cards). `tools/mklessonfont.py` draws it (at 8 times the size, then shrunk) with the font;
  every other letter of `assets/fonts/lessons-font.bin` is byte for byte what it was. The pack is 92 bytes smaller
  (the picture is smaller than the letter it replaces): 54,466 bytes, 8,725 blank in the slot.
- **Chosen from three,** each shown on the simulated FM-1's screens (a strip over the drum grid, a card, a big card):
  this one; the key alone, a bar; this one with the white keys in a lighter ink. The course's author: "The first
  one shows three keys but its easier to see that they are keys", then "I think I like the first one better at the
  beginning we can just tell the user that this is supposed to be a black key".
- **In a lesson file:** `[BLACK 2]` on a card, a strip or a hint is the second black key from the left (of eleven):
  `tutorial/tp/lesson.py`, `words()`, turns it into the picture and the number in a frame, so the pack and the
  computer both get it and `tutorial/check.py` measures it as it will be drawn. In Do, Robot, Lights and Follow:
  `black 2` (`keys_of`; `black 1` to `black 11`). `tutorial/lessons/README.md` has both.
- **An FM-1 with no lessons on it, led by a computer,** draws cards in SLOOP's own letters, which have no pictures:
  `tutorial/tp/fm1.py`, `card()`, asks once whether the lessons are there (`has_lessons`) and sends the words
  `[BLACK 2]` instead.
- **For step 4:** the first card that uses the picture says what it is (the first drum lesson). For step 6 (the
  frames that answer the controls): a frame with the picture is that black key's.
- **Checked:** `tutorial/upgrade/drumcheck.py`, whose test lesson now says `tap [BLACK 2]` on its strip and
  `black 2` in its Do (the snare's black key blinks; the lesson plays through both ways); `tutorial/check.py`
  reads all 30 lessons; `fw/build.sh` with `install_sim` and `lessons_sim`; the two screens side by side, with
  the lessons (the picture) and without (the words).
- **Other pictures:** none yet. Decision 12 has them only where a lesson in step 4 shows the need.

## Step 4: the lessons (begun 2026-11-25)

One lesson and one choice at a time, each change shown as Today / Suggested with its lines measured. What changed
and why, with the course's author's words, is in each lesson's own Tester notes; here, where each lesson stands.
After a lesson: `python3 tutorial/check.py N`, `--run N` and `tutorial/device.py N`, then the picture the run
rewrote put back (`git checkout -- tutorial/lessons/screens`).

| Lesson | Stands | What changed |
|---|---|---|
| 1, Hello | done 2026-11-25; all three checks pass | Nothing of it touches a change of 3.4.2 (no flag in the worksheet; it played through both ways as it was; its counts, 153 sounds and 37 drum kits, are 3.4.2's too). One card added, step 6, with its `Good.`: the black key's picture introduced with the cards' other rules and tapped at once ("Yes, add it"). 23 steps |

| 2, Sixteen boxes | done 2026-11-25; all three checks pass | The drum grid: a white key is a box by itself. The kick is played on the tracks page, before [SEQ] (it was played on the grid, where that key now writes in silence); the card says once that a tap fills a box and makes no sound; three strips and two hints lose "hold [SEQ]"; the free step in the course's author's words (`Tap it again: removed.`). 17 steps |

| 3, Kick, snare, hat | done 2026-11-25; all three checks pass (it had stopped at step 6 with the computer leading) | A drum is picked with its black key: `Pick the snare: tap [BLACK 2].`, `Pick the hat: tap [BLACK 5].`, each waiting for `drums picked ...`, with a hint. The explanatory card kept, with "black key" and "Tap" ("I'm worried we lose that"), and a new card after it: the top names the selected drum; some black keys hold two drums ("shouldn't we mention that too?"). The boxes without [SEQ]; [KNOB1] named once. Shown whole, at once. 20 steps |

| 4, The drum kit | done 2026-11-26; all three checks pass | A new lesson under the old name, on the grid: lesson 3's beat, then the clap with its black key, the two toms that share one (tap again for the other), any of the 16 with [KNOB1], then the kits as before. First moved whole to SLOOP's KIT page and approved, then rethought ("if we are just trying to preserve an old lesson that doesn't seem like it should be a priority"). Gone: playing every white key, the play-along step. 20 steps |

| 5, Meet the tracks | done 2026-11-26; unchanged; plays through both ways | Nothing of it touches a change of 3.4.2: [GLO] held still shows the MIX page, with the mutes on 1 to 5 and the solos on 6 to 10 |

| 6, Faster, slower, oops | done 2026-11-26; all three checks pass (it had stopped at the save both ways) | The boxes without [SEQ]; undo's 3 moves (the card said 8); saving as SLOOP 3.4.2 has it: `Press [SAVE] twice: / the PROJECT page.`, then `Turn [KNOB1]: SAVE.`, both strips along the top so the page's big SAVE and its name in the corner show ("I don't see that in the screenshot": the first placing covered the corner it named); the save card says SLOOP also saves by itself. `PROJECT` and `PATTERN` are page names for the lessons now (`tutorial/tp/lesson.py`). 22 steps |

| 7, Your own beat | done 2026-11-26; all three checks pass (it had stopped at its free step both ways) | Its purpose and shape kept: a beat made alone, then saved. The card of moves for the new grid (`Pick a drum: a black key / or [KNOB1]. / A white key: a box fills. / Tap again: it empties.`); saving as in lesson 6, two strips along the top. 11 steps |

**Part 1 (lessons 1 to 7) is done:** all seven play through both ways, together as well as one by one, and the build's
page tests pass with them. The pack is 55,428 bytes (54,466 before part 1); the slot keeps 7,763 blank bytes (1,024 is
the least).

**To carry into the lessons that follow:** the picture is known from lesson 1, step 6, and lesson 3 says what a
black key does on the grid (steps 4 and 5: it picks the drum; some hold two). The words the course's author keeps:
"selected" for the drum the boxes write, "pick" for the act, "the top" for where its name shows. **A lesson is
kept for what it teaches on 3.4.2, not for its old shape** (lesson 4): ask first what a learner needs it for now.
The white keys as drums (the tracks page; SLOOP's KIT page names all 16) are left for lesson 17, where a roll is
played on one; lessons 2 to 4 use them as boxes. With a key's first drum already selected, one tap gives its second. **A strip never points at something it, or
the next strip, covers** (lesson 6): look at the step's screenshot for the thing the words name. On SLOOP 3.4.2's
knob pages the page's name is small in the bottom right corner and the knob names are at the top. Saving: `Press
[SAVE] twice: / the PROJECT page.`, `Turn [KNOB1]: SAVE.` (strips at the top), from the tracks page or a drum page. A whole lesson is
shown at once, each changed card as Today / Suggested, after it has played through both ways from a scratch copy
(`trylesson.py` in the session's scratch folder: a lesson file from a folder, packed alone, played by `device.play`
and by `Player.play` with a robot).

## Not done

Lessons 2 to 30 of step 4, then steps 5 to 10 of "The plan for the full move". Nothing from here has been on a real
FM-1.

**How the replay was done,** to repeat it on another step: `tutorial/upgrade/replay.py LESSON STEP OUTDIR`, run
from a copy's top folder. It plays the lesson as `device.py` does up to the step, then makes that step's robot
moves exactly as `L.moves()` gives them (presses, releases, timing), and after each release prints the FM-1's
phase, step and drum notes (`f.pattern(3)`), with a screenshot. The control ran on today's 3.3.7 build through a
folder of links to the main copy. Running moves one at a time through `device.hands()` is wrong: it lets go of
what is held between them.

## The room

| | bytes |
|---|---|
| The merged firmware (`build/fw/app.bin`) | 289,828 |
| Room before the sample bank (3.4.2 moved the bank 5 KB later) | 283,360 |
| **Over by** | **6,468** |
| After step 1 (the visualizer, SLOOP 2.8's import and the logo out): `app.bin` 276,960 | **6,400 to spare** |
| After step 2: `app.bin` 276,908 | 6,452 to spare |
| After the review (SLOOP 2.8's settings and calibration back, about 1 KB; the fixes): `app.bin` 278,164 | **5,196 to spare** |

**The flash before the bank is one limit of five** (the review, finding 3; `tools/tp_room.py` prints them at the end
of `fw/build.sh`). Free bytes:

| Limit | After step 2 | After the review | SLOOP's own 3.4.2 |
|---|---|---|---|
| Flash before the sample bank | 6,452 | 5,196 | 1,912 |
| RAM: code outside the window, variables (`slot`) | 2,152 | 2,136 | 1,584 |
| RAM: the patterns, their saved copies, the stack (`low`) | 200 | 200 | 200 |
| RAM: buffers (`boot`) | 184 | 184 | 184 |
| The flash window's own region (`xip`) | 79,940 | 78,692 | 75,308 |

Code in the window (the lesson player, the guide, the pages) costs its full size of the first; what runs from RAM
is packed in the flash (to about 78 %) and costs its full size of the second, as every new variable does. Still to
come: the frames that answer the controls (window code, 400 to 600 estimated, and a few variables), the screen
colors (`hal/fm1/board.c` runs from RAM: about 80 of the second), remembering the lesson (window code, about 100).

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

Since 2026-11-25 the logo moves into the lesson slot too (decision 1, changed): about 2 KB more, about 2.8 KB in
all. Spent on: the frames that answer the controls (decision 11, about 400 to 600, estimated), the screen colors
(40 to 80), remembering the lesson (about 100).

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

- The firmware build says `xipcheck: ... roots not found: trs_midi_poll`. **Settled:** SLOOP's own 3.4.2 build
  says exactly the same (`xipcheck: 237 functions may run with the window off; 514 in the window; roots not
  found: trs_midi_poll`). It is SLOOP's, not ours.
- 3.4.1's fixes for the random freezes are in this merge. They are the main reason to move.
- Before starting: `git ls-remote --tags upstream` for a SLOOP newer than `v3.4.2-beta` (none on 2026-11-24;
  `upstream/main` was still `159e4b4`).
