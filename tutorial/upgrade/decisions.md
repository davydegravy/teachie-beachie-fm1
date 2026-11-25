# Decisions for the move to SLOOP 3.4.2

Questions only the course's author can answer, asked before the work starts so it does not stop halfway. Each has
the options, the one suggested first, and the lessons it touches. The facts behind them are in
`tutorial/UPGRADE-3.4.2.md`; the steps they touch, in `tutorial/upgrade/worksheet.md`. Written 2026-11-24.

Write the answer under each, or say it in the session.

**The rule for all of them (the course's author, 2026-11-24):** "Do it SLOOP's way and then easier for children;
if we don't do it SLOOP's way it will get more confusing." SLOOP's way comes first (its guide, what a button opens
first); among SLOOP's ways, the easiest for a kid (single presses and a knob over a held combination). More work is
fine: "hopefully if we follow their direction there will be less changes in the next SLOOP version". Where a
decision below is blank, pick by this rule and say so.

## 1. What to cut to make room

The merged firmware is 6,468 bytes too big. Remembering the lesson after power-off needs about 100 more, the
screen colors for newer FM-1s 40 to 80.

- **A (suggested): the visualizer and SLOOP 2.8's import,** about 7.2 KB, about 700 bytes to spare.
  - Lost: HOME on the tracks page shows no visualizer, and nothing else happens there. No lesson uses it, and
    learners fall into it by accident.
  - Lost: a SLOOP 2.8 user's own sounds no longer come along at the first start. Their project already does not.
- **B: A, and the logo moved into the lesson pack:** 2.1 KB more room for later. With no lessons on the FM-1, the
  start shows the name only.

Each cut goes behind a switch of our own, so SLOOP's files stay as they are.

Answer (2026-11-24): **A, the visualizer and SLOOP 2.8's import.** The installer page's "Already use SLOOP?" note
then says a SLOOP 2.8 user's presets, favorites, kits and bank no longer come along.

**The logo in full colour, later:** liked, "but it's not very important". Not part of this upgrade; its own small
project once the upgrade is done, or when a later SLOOP needs the room. What was measured
(`tutorial/upgrade/logo-compare.png`):

- The screen is 240 x 240 and the logo already takes 168 x 197 of it, a pixel for a pixel: it cannot get more
  pixels, only smoother. Today it is 8 flat colours with no shades between them (stair-step edges), 2,118 bytes.
- Full colour from `assets/logo/teachie-beachie.jpg` (1254 x 1254): about 17 to 20 KB packed, so the lesson pack
  goes from about 55 KB to about 75 KB of its 80 KB slot. 16 colours: 6 to 9 KB, but the colours chosen by
  themselves drift (the stem turned orange): it would need its colours picked by hand.
- The firmware needs a decoder, a few hundred bytes (its own start already unpacks LZ4: `fw/unpack.c`), and the
  2,118 bytes come out: about 1.8 KB of room gained. With no lessons on the FM-1 the start shows the name only.
- Best with the screen's setup table (decision 8): on newer FM-1s the soft shades wash out without it.

## 2. The drum grid: tap a box, or hold [SEQ] and tap

On 3.4.2's drum grid a white key sets or clears a box by itself, with or without [SEQ], and makes no sound. A drum
is picked with the black keys or [KNOB1]. Playing a drum's white key there no longer picks it.

- **A (suggested): teach the new way.** Tap a box to set it. Pick the drum with [KNOB1] (the top shows its name),
  and the black keys later as a shortcut. Fewer buttons for a kid, and it matches SLOOP's own guide.
- **B: keep "hold [SEQ] and tap" in the words,** which still works, and change only how a drum is picked. Fewer
  words change, but it teaches a step that is no longer needed.

The black keys pick two drums each, in turns: 1 KICK / KICK2, 2 SNARE / SNR 2, 3 RIM, 4 CLAP, 5 HAT / PEDAL,
6 OPEN, 7 TOM L / TOM H, 8 CRASH, 9 RIDE, 10 SHAKE / CONGA, 11 BELL. [KNOB1] goes through them all by name.

Lessons: 2, 3, 4, 7, 15, 16, 17, 28, 30.

Answer (2026-11-24): **A, and the drum picked with a black key on the grid.**

- **A box:** just tap it. [SEQ] is only the way to the grid. The lessons say once that any white key tapped on the
  grid writes or clears a box (and makes no sound).
- **A drum:** tap its black key on the grid, the lesson blinking the right one; the top shows the name and the
  key stays lit (`drum_black_key`, `fw/main.c`). Tap it **once**: some black keys hold two drums and the same key
  again switches to the other (`drum_black`: KICK and KICK2, SNARE and the 9th drum, HAT and OPEN's pair ...), so
  a hint catches KICK2. [KNOB1] is shown once as the other way.
- **Lesson 4** ("Play every white key. The top names each drum") moves to the tracks page, where a white key still
  plays its drum and picks it.

## 3. Lesson 8, "A bass line in boxes"

The synth tracks have no boxes on 3.4.2. [SEQ] opens the piano roll (NOTES), where "the keys write the step" and
[KNOB1] (STEP) picks the step.

- **A (suggested): lesson 8 becomes the first piano-roll lesson.** Same song, same A on 1, 4, 7, 11, 14 and 16,
  written with STEP and a key. Lesson 13 ("The piano roll") then goes further: lengths, moving a note.
- **B: lesson 8 records the bass live** with REC, and lesson 13 keeps the piano roll. It repeats lesson 10's idea.

Lessons: 8, 13.

**Tried on the simulated FM-1 (2026-11-24):** [SEQ] held + a white key still writes the last note played into that
step on a synth track, from the tracks page and from NOTES (on a filled step, its release clears it: `hold_step`,
`hold_erase`). On NOTES every key played writes its note at the step shown and the step moves on one when the keys
are let go (`notes_on`, `notes_advance` in `fw/main.c`); [EDIT] tapped there deletes that note.

Answer (2026-11-24): **A, the piano roll.** "To me the knob and the keys sounds easier than doing a double press",
and it is SLOOP 3.4's own way: [SEQ] opens NOTES first, and SLOOP's guide says "the keys write the step".

- Lesson 8: [SEQ] opens NOTES; [KNOB1] until STEP says 1, play the A; STEP to 4, the A again; and so on. The step
  moves on by itself after each note, so notes side by side need no knob. The A shows on its row.
- On NOTES every key played writes, so the lesson says it once, and choosing a note by ear happens before [SEQ]
  (or [EDIT] takes a stray note out).
- Lesson 13 goes further on the same page: LEN, the NOTE knob, OCT+ held (the note's sheet), the zoom.

## 4. "Pages" or "patterns"

On 3.4.2 the screen says PATTERN (`PATTERN 2 FROM 1 · SAVING`). Each pattern keeps a saved copy, and leaving a
pattern puts that copy back, so a change is lost unless it is saved into the pattern (hold [SAVE] + its key for
1.5 s). Lesson 16 needs a new way to copy a page anyway.

- **A (suggested): say "pattern" everywhere,** as the screen does. Lesson 16 teaches "save into a pattern" and
  warns that an unsaved change goes away on a switch.
- **B: keep "page"** and say once that a page is a pattern, as lesson 16's last card does today.

Lessons: 16, 23, 24, 25, 26, 30.

Answer (2026-11-24): **A, "pattern"**, what SLOOP uses ("lets go with what Sloop uses"). Lesson 16 becomes
"Eight patterns" and teaches saving into a pattern: hold [SAVE] + hold its key 1.5 s.

## 5. Chains

[15] is no longer CHAIN. A chain is now made by tapping pattern keys while [SAVE] is held, and CHAIN DONE is never
said.

- **A (suggested): teach the new chain** in lesson 25, and keep the course's shape.
- **B: drop chains:** lesson 25 goes straight to the SONG page's lines, and lesson 26 goes further.

Lessons: 25, 26, 30.

Answer (2026-11-24): **A, the new chain**, SLOOP's way ("lets go with the Sloops way for 5"): hold [SAVE] and tap the
patterns in order; hold [SAVE] and tap one to end it. Not kept, as today.

## 6. How to save the project

"Press [SAVE] twice" no longer saves. 3.4.2 has three ways: [SAVE] on the tracks page, [SAVE] again (PROJECT 3/3),
[KNOB1]; or [SAVE] held + a pattern's key held 1.5 s; or [HOME] held + white key 5 (no longer labelled: HOME held
opens a menu).

- **A (suggested): [SAVE], [SAVE], [KNOB1].** Every step is labelled on the screen. Lesson 29 shows the
  HOME shortcut as a second way.
- **B: [HOME] held + [5].** Shorter, but nothing on the screen says what [5] does.

Lessons: 6, 29, 30.

Answer (2026-11-24): **two ways, each where it is needed, and the autosave.** SLOOP has four because it kept each
group's habit: the PROJECT page (asked for in 3.4), [SAVE] + a pattern's key held (SLOOP 2.8.5's), [HOME] + [5]
(SLOOP 3.3's) and the autosave.

- **Lesson 6, the first save: [SAVE], [SAVE], [KNOB1]**, the first way in SLOOP's guide, every step named on the
  screen (`fw/main.c`: SAVE on the tracks page opens SONG 2/3, SAVE there opens `PROJECT 3/3`, whose knobs are
  SAVE, NEW and LOAD, each "GO"). And: the FM-1 also saves by itself once the music has been stopped and untouched
  for 2.5 s (at most every 20 s, `t_autosave`). The header comment's "tapped twice: the project saved" is older than
  4.0.6.
- **Lesson 16: [SAVE] held + a pattern's key held 1.5 s** saves into that pattern and then the project at once
  (`pat_go`, `save_soon`).
- **Lesson 29: the whole PROJECT page:** [KNOB1] SAVE, [KNOB3] LOAD (asked twice), [KNOB2] NEW.
- **[HOME] held + [5] is left out:** nothing on the screen says what [5] does.

## 7. Strips on the sound pages

On 3.4.2 the sound pages are drawn the other way up: knob names at the top, the page's title small at the bottom
right. A strip at the bottom hides the title; a strip at the top hides the knob names.

- **A (suggested): strips at the bottom,** and the lessons name the page by its knobs, not by its title.
- **B: strips at the top, one line,** and the learner reads the knob names after the strip goes.

Lessons: 11, 12, 19 to 22.

Answer (2026-11-24): **swap the sides, as today's idea** (seen on lesson 19's FX page on both firmwares, the
simulated FM-1). Today the lessons already put a strip where it hides nothing the learner needs: a knob step at the
top (the knob names are at the bottom), a find-the-page step at the bottom (the page's name is at the top). On
3.4.2's sound pages the knob names and values are at the top and the page's name (`FX 1/7`) is small and grey in
the bottom-right corner, so on those pages:

- **a knob step: the strip at the bottom** ("[KNOB3], echo: up");
- **a find-the-page step: the strip at the top**, pointing at the corner ("until the bottom corner says FX 1/7").

SLOOP's pages stay as SLOOP draws them; only our strips move.

## 8. The screen colors for newer FM-1s

M-VAVE's own setup table for the screen, sent at the start, fixes the faded colors on newer FM-1s (40 to 80
bytes). M-VAVE sends the same table to every FM-1, old and new.

- **A (suggested): add it now,** behind a switch of our own. Check on your FM-1 that nothing changes, then ask
  your friend to look at the colors on theirs. If it looks wrong, a reinstall takes it out: it does not touch
  the flash or the projects.
- **B: leave it out** until someone with a newer FM-1 asks.

Answer (2026-11-24): **A, and offered to SLOOP as an issue with the change in it.** SLOOP's author does not merge
pull requests (two from others, both open, no reply, on 2026-11-24); he fixes what issues report, in his own
release (3.4.2's notes list the issue numbers; our earlier sound report was fixed that way). So:

1. Build it into the trial: M-VAVE's table in `lcd_init()` (`hal/fm1/board.c`), between `13` and `29`, as close as
   possible to how SLOOP would write it, so that a merge of SLOOP's own version later is easy.
2. The tester's FM-1 (the original screen): nothing should change.
3. The friend's FM-1 (newer): the colors should look right.
4. An issue on SLOOP 3 (`isod89/sloop3-fm1`), written here, posted by the course's author from their account: the
   problem with a link to sloop-fm1#269, the table and why it is safe (M-VAVE sends it to every FM-1), the change
   itself, and both results. Credit the reporter's tested commit (Tragiciy/sloop-fm1@73f00ee).

Teachie Beachie carries it until SLOOP has it; then the next merge takes SLOOP's and ours goes.

## 9. Remembering the lesson after power-off (NOTES, item 118)

- **A (suggested): last, once all 30 lessons pass on 3.4.2,** if there is time left.
- **B: in a later round,** on its own.

Answer (2026-11-24): **A, in this upgrade, last** ("yes lets put that in"), once all 30 lessons pass on 3.4.2. As
`tutorial/NOTES.md` item 118 has it: the lesson's number written into the blank bytes after the pack in its slot
(no erase, the learner's song untouched), read back at START; [OCT+] goes back to that lesson from its first step;
sending the lessons again starts over at 1. About 70 to 100 bytes, within the 700 left after decision 1. Tested on
the simulated FM-1, a power cut during the write included.

## 10. Testing on your FM-1 during the work

A project saved by 3.4.x cannot be opened by 3.3.x, so going back to today's Teachie Beachie loses the projects
saved in between.

- **A (suggested): the simulated FM-1 only** until all 30 lessons pass, then one install on your FM-1, after a
  backup (the installer page makes one; the editor's BACK UP EVERYTHING keeps the projects).
- **B: install early and often** on your FM-1, to feel the changes on the real buttons sooner.

Answer (2026-11-24): **A, the simulated FM-1 only, until everything passes** ("I want to test everything on the
emulator before I put anything on my FM-1"). The projects on the FM-1 do not matter ("I don't care about the
existing projects"), and the children who use it are the testers ("they are doing testing for me"): they get it
once all 30 lessons pass, to test it on the real buttons.
