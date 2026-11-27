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

**2026-11-25, in the review of step 1: SLOOP 2.8's menu settings and calibration stay** ("Yes, bring them back").
The cut had taken them too, with the colour theme and the MIDI settings, which this answer does not name. Without
its calibration an FM-1 that needed one answers its buttons and knobs wrongly until it is calibrated again (OCT-
and OCT+ held), and a beginner would not know why. About 1 KB. The theme and the MIDI settings stay out.

**Changed 2026-11-25: the logo moves into the lesson pack, at the best the screen can show.** (On 2026-11-24 it
was "later": liked, "but it's not very important".) The course's author is considering a new logo: a round
sticker, the name over a sunset on the sea, 6 flat colours (peach round it, orange, black, white, yellow, teal).
Until it is chosen and in `assets/logo/`, today's goes in as a stand-in: do not tune its picture. What was measured
(`tutorial/upgrade/logo-compare.png`, and again on 2026-11-25):

- The screen is 240 x 240. Today the logo is 168 x 197, a pixel for a pixel, in 8 flat colours with no shades
  between them (stair-step edges), 2,118 bytes. The largest a logo can be with WITH SLOOP 3 under it is about 204
  high: the round one is 199 x 204, its name larger than today's. More size and smooth edges, not more detail.
- In full colour (RGB565), packed as rows of runs with every 16th row's place kept (what a tile of the screen
  needs, as `tp_logo_draw` in `app/vis.c` reads today's): made flat with smooth edges (its flat colours at 8 times
  the size, then shrunk), the round logo is about 18 KB, today's about 18 KB at 174 x 204. A picture's own colours
  (the JPEG's shading and noise) come to about 31 to 32 KB. The slot has about 26 KB free beside the lessons
  (54,558 of 81,408 bytes), so the logo is made flat, with smooth edges.
- The firmware loses `tp_logo_rle` (2,118 bytes) and keeps a decoder much like today's, reading the slot: about
  2 KB of room, which decision 11 needs.
- The pack's offsets are u16 (`tools/mkcourse.py` stops past 64 KB), and the lessons and the logo together pass
  that. So the logo goes after the pack, outside its offsets: for example at the end of the slot, its place in the
  512-byte slot header. The blank bytes right after the pack stay free for decision 9.
- With no lessons on the FM-1 the start shows the name only.
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
- **2026-11-26, lesson 4 is a new lesson, on the grid** ("yes lets use this, this is great"). The tracks page, tried,
  plays the drums but names none; SLOOP's KIT page names all 16, and the lesson was moved there first. Then the
  question of what it is for: "if we are just trying to preserve an old lesson that doesn't seem like it should be a
  priority". On 3.4.2 nobody needs to know which white key is which drum to write a beat. The lesson now adds drums
  to lesson 3's beat on the grid: the clap with its black key, the two toms that share one ("tap again"), any of the
  16 with [KNOB1]; then the kits, as before. The white keys as drums wait for lesson 17 (rolls).
- **2026-11-25, in the review of step 2:**
  - **A lesson can wait for the drum picked** ("Yes, approve"): a new `Until` word, `drums picked SNARE`. Without it
    a "pick the snare" step ended at the tap, whichever drum it gave, and no hint could catch KICK2.
  - **Another drum picked than a step's boxes ask for** ("Yes, add them"): that drum's black key blinks, not the
    boxes, and the lesson says, once for each wrong drum: `Another drum is picked. / Tap the blinking key.` (in place
    of 3.3.7's `That played a drum. / Hold [SEQ] first.`). It no longer puts the drum back by itself.

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

- **2026-11-26, as built** (each lesson's Tester notes have the words). Lesson 8: the A is played on the tracks page
  first; STEP stands at 1 when the roll opens, so no knob is needed for the first note; the strips are
  `STEP says 1. Play [KEY 3].`, `[KNOB1] to step 4: [3].`, `The same on 7, 11 and 14.`, `[KNOB1] to step 16: [10].`;
  the key to play blinks only once [KNOB1] has brought STEP to its step ("yes make the key wait until the knob is
  turned"). Lesson 13 keeps its three fixes (NOTE, LEN, [EDIT]) and **leaves the note's sheet and the zoom out of its
  steps** ("Yes, all of it"): the sheet is a held button with its own meanings for the knobs, the zoom changes
  nothing useful on 16 steps. Both are in lesson 13's notes for the AI assistant.

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

- **2026-11-26, as built** (lessons 16 and 23; their Tester notes have the words). 3.4.2 has no "copy this page", so
  lesson 16's order turns round: the learner adds two kicks, saves that into pattern 2 (`Hold [SAVE] + [2] until / the
  top says SAVED.`) and stays on pattern 1; then [SAVE] + [1] brings pattern 1 back as it was saved and the kicks go
  away, which shows the rule the last card states: `A change not saved is / gone when you switch.` The file is
  `16-eight-patterns.md`. Lesson 23 builds its second pattern the same way: change the beat, mark a fill note, save
  into pattern 2. A key let go too early goes to the empty pattern and loses the change; both lessons' hints give the
  way back.
- **2026-11-26, part 4:** "pattern" for "page" in lessons 24 to 26 and 30 as well; lesson 25 is "Chain the patterns"
  (`25-chain-the-patterns.md`). "Page" is now only what the screen shows, and **a page is called what its own screen
  calls it**: of lesson 26's last card, "I just don't like Song lines, it seems like we are navigating ot a page, what
  is that page called?" It is `SONG` (its title says `SONG 2/3`), so the lessons say "the SONG page" and `SONG page:
  [SAVE].`

## 5. Chains

[15] is no longer CHAIN. A chain is now made by tapping pattern keys while [SAVE] is held, and CHAIN DONE is never
said.

- **A (suggested): teach the new chain** in lesson 25, and keep the course's shape.
- **B: drop chains:** lesson 25 goes straight to the SONG page's lines, and lesson 26 goes further.

Lessons: 25, 26, 30.

Answer (2026-11-24): **A, the new chain**, SLOOP's way ("lets go with the Sloops way for 5"): hold [SAVE] and tap the
patterns in order; hold [SAVE] and tap one to end it. Not kept, as today.

- **2026-11-26, as built (lesson 25).** Tried on the simulated FM-1 first: pattern keys tapped in one hold of [SAVE],
  two or more, are a chain (`CHAIN 1 2 3`), each pattern played to its end in turn; one pattern tapped alone ends it.
  On the SONG page, where the keys work with no button held, two taps are only two changes of pattern: a chain needs
  [SAVE] held. So lesson 25 does everything with [SAVE] held, lesson 16's move, and never leaves the tracks page:
  `Hold [SAVE]: [1], [2], [3]. / Then let go.`, `[SAVE] + [1]: / the chain ends.`, `Hold [SAVE], tap [14].` (SONG REC),
  `[SAVE] + [13] plays / the song you recorded.` With [SAVE] held the screen names keys 13 to 16 itself. The lesson
  waits for `says CHAIN`, `says PATTERN 1`, `says SONG KEPT`, `says LOOP`.

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

- **2026-11-26, as built.** Lesson 29 teaches the PROJECT page on its three knobs, with the page's own words: `[KNOB1]:
  SAVE. / [KNOB2]: NEW. / [KNOB3]: LOAD.`; LOAD asks once more (`AGAIN: CHANGES LOST`), so `[KNOB3], LOAD: / two clicks
  right.`, the second within a second and a half. Lesson 30's step ends on a real save ([SAVE] twice, [KNOB1]); it
  used to end on two presses of [SAVE].
- **2026-11-26, the autosave and the lessons' check.** The FM-1's own save, when the music is stopped and untouched
  for about two and a half seconds, says `PROJECT SAVED (AUTO)`; a step waiting for `says PROJECT SAVED` took it for
  the learner's save (played on the simulated FM-1 with lesson 23: a learner who only waits is moved on). Every save
  step now waits for exactly that word, `Until: says only PROJECT SAVED`: lessons 6, 7, 15, 23, 29 and 30 ("Yes,
  both").

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

- **2026-11-26, changed on the [SEL] page (lesson 11): a knob step's strip at the top after all.** "I think Strip 12
  should move to the top"; then "Yes, both at the top". On 3.4.2 these pages show each value twice: small in the row
  at the top, and big in the block below. A two-line strip at the top covers only the small row; a strip at the bottom
  cuts the lower two big values in half and hides the page's name in the corner. So on the [SEL] page the knob strips
  are at the top, where they were before the move. Lessons 19 to 22 use pages of the same kind: each is looked at in a
  picture before its strips are placed.
- **2026-11-26, as built in lessons 19 to 22: the side follows from the kind of page.** Looked at in pictures, 3.4.2's
  knob pages are of two kinds:
  - **four big values in the middle,** repeating the small row at the top (`SEL 1/2`, `INSERT 4/7`, `FLT 2/6`): a knob
    step's strip at the top, as on the [SEL] page;
  - **a picture in the middle** (sliders, a wave, the sound's shape, the arpeggio's notes), where the small row at
    the top is the only place with the knobs' names and values (`FX 1/7`, `DLY 6/7`, `ENV 1/2`, `LFO 1/3`, `LFO DEST
    3/3`, `ARP 1/2`): a knob step's strip at the bottom, as answered on 2026-11-24.
  A find-the-page step's strip is at the top on both, and its words say which corner: `Tap [FX] until the bottom /
  corner says FX 1/7.` Asked of lesson 19, then of lesson 20: "For step 4 should the strip be on the bottom? where
  does it say FX 1/7"; after a picture of both placings, "I see it now, It changes to insert". The name is in the
  bottom right corner and changes with each tap, so a strip at the bottom covers the thing it points at. With a wider
  button the line starts with it (`[EDIT] until the bottom / corner says FLT 2/6.`, `[LFO] until the bottom / corner
  says LFO DEST.`).

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

- **2026-11-26, who looks at a newer screen, and when.** The course's author: "My friend will likely not test on his
  FM-1 I can test closer to Christmas on one of the ones I ordered". So point 3 above changes: the newer screen is
  looked at on one of the FM-1s the course's author has ordered, when it arrives (late December 2026), not on the
  friend's. Until then the change can only be checked by the commands it sends, and on the original FM-1 (point 2).
  The issue for SLOOP (point 4) waits for that look, or goes out with the one result and says so.
- **2026-11-26, built (step 7)** ("Yes, build it now"): the table and one loop in `lcd_init()`, behind
  `-DTP_LCD_RESET`; 88 bytes of flash and 128 of RAM. `tutorial/upgrade/lcdcheck.py` records what the start-up sends,
  with the table and without. The issue's text is `tutorial/upgrade/sloop-issue-screen-colors.md`, not posted.

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

## 11. Frames that answer the controls (asked 2026-11-25)

The course's author asked for the framed names on the cards and strips ([SEQ], [KNOB1], [5]) to answer the real
controls, so the cards "feel alive and acknowledge the user's physical interactions without adding visual clutter":

- **A button or a key:** its frame filled and its name in the paper's colour while it is held, outlined again when
  it is let go. Several held together: each frame answers its own.
- **A knob:** the frame fills a little each click: from the left turning clockwise, from the right turning
  anticlockwise, the name in the paper's colour where the fill passes. Driven by the real clicks, not an animation.
- The same Google Sans, sizes and places; no more of the screen; in one place, for every card and strip; no harm to
  the sound.

Answer (2026-11-25): **yes, in this upgrade** ("Add both"). It is firmware: the pack holds words, songs and fonts,
no code. But it is Teachie Beachie's own part, the guide in `fw/main.c`, and 3.4.2 is a reinstall anyway. How, as
looked into on 2026-11-25:

- **Which control a frame is:** SLOOP already names its 14 buttons (`CAL_BTN`) and 7 knobs (`CAL_ENC`, "KNOB 1"
  with a space) in `fw/main.c`, for its calibration page, and `white_key[]` turns [5] or [KEY 5] into its key.
  Matched once when the words change (`guide_wrap`) and kept in `G`, so every card, strip and hint, the [OCT-] and
  [OCT+] answers and the computer's lessons all have it at once. A name that is no control ([THIS] in lesson 1)
  stays outlined. A black key's picture (decision 12) needs matching too.
- **What is held:** `panel_buttons()` and `panel_keys()`, the raw state, so a button the guide takes counts too.
- **The clicks:** `guide_ev` kinds 5 and 6, already counted for the lessons. About 10 clicks fill a frame; it
  empties after about half a second at rest.
- **The fill:** inside the frame, paper and ink swapped pixel by pixel (each channel: paper + ink - the pixel), so
  the letters keep their soft edges; the frame drawn again over it. Done on `tilebuf` before `gfx_finish`, in our
  code: SLOOP's `app/gfx.c` stays as it is.
- **Drawing:** a change bumps `G.gen`, which starts a pass (25 a second at most; 30 tiles of 120 x 16; only
  changed tiles are sent). The sound is made in the same loop and comes first (`audio_half_free`; `ui_step` stops
  after 0.25 ms), and a knob already redraws SLOOP's own page at each click. To check on the simulated FM-1: the
  load (`ld_peak`) with a knob spun fast, and screenshots of a frame held and half filled.
- **Cost:** about 400 to 600 bytes (estimated, not measured), from the room the logo's move makes (decision 1).
- **Limits:** [MASTER] is the FM-1's volume knob, which the firmware cannot see (lesson 1): it stays outlined. An
  [OCT+] answer moves to the next card at once, so its fill only flashes.

When: after the whole course passes, so a shortage of room never holds up the lessons.

**2026-11-26, changed before it was built, then built (step 6).** Gone over with the course's author first ("Lets go over
it"), with a mock-up drawn over real screens. What changed from the answer above:

- **A button or a key, as answered:** "I would like an outlined button to fill, meaning the text inside would need to
  invert." Its frame is filled, the letters in the paper's colour, while it is held; outlined again when let go.
- **A knob's frame is full exactly when the turn the step asks for is done, not after "about ten clicks":** "10 clicks
  to fill it ... is not useful for the user, the amount of clicks should equal the turn, so two clicks are needed then
  two clicks would fill it, if they are in the sequencer and need to move from 4 steps to the right 4 clicks would
  fill it. there is no step in the entire tutorial that has anything like 10 clicks." (The longest counted turn in the
  course is six.) And "its directional if a knob wants them to turn it left the fill should move left": a turn to the
  left fills from the right. Turning back takes the fill back.
- **What needs no fill has none:** "If something like the Oct+ only flashes we don't need to make it this kind of
  button. if there is a knob that uses only one click and it advances we don't need to use it." The [OCT-] and [OCT+]
  answers of a card stay as they are; a turn of one click shows nothing.
- **A turn to a value fills as the value nears it** ("Yes, follow the value"). Of the 65 steps that ask for a knob
  turn, 23 are one click, 20 a set number of clicks (2 to 6), 19 a turn to a value (the tempo above 135, DLY above
  50, FILTER until OFF: no set number of clicks), 3 open ("more dirt or less"). For the 19 the frame is empty at the
  value the step begins with and full where the step waits for it.
- **So the lesson says how far, not the guide alone.** The fill depends on what the step asks for: on the FM-1 the
  lesson player works it out (`fw/course.inc`, `c_fills`: the Do's clicks for each knob, or with one knob and a
  condition on a value a knob sets, that value from where it stood to where it must get); from the computer
  `tutorial/tp/lesson.py` does the same and tells the FM-1 (the guide's message 8, `FM1.knob_fill`). The guide
  (`fw/main.c`) matches each framed name once to its control, reads what is held, and draws the fill by swapping paper
  and ink inside the frame.
- **Cost, measured:** 1,696 bytes of the flash before the sample bank (3,296 left) and 48 of RAM; the estimate was 400
  to 600 for the simpler version. **The load could not be measured:** the simulated FM-1 reports 0 % whatever it
  draws (its sound costs it no time). It is on the list for the real FM-1 (decision 10).
- **Checked:** `tutorial/upgrade/framecheck.py`, which looks at the simulated FM-1's screen after each press and turn.

## 12. Icons (asked 2026-11-25)

Small pictures in the cards' words, where a picture says it better than a word.

Answer (2026-11-25): **yes, in this upgrade** ("Add both"), with no firmware change. The lessons' fonts are in the
pack, and `app/gfx.c` draws any of ASCII's 95 characters from them (`glyph_of`: with `FN > 77`, a character's glyph
is its code less 32). Nine never appear in the lessons' words: `$ % & = @ \ { | }`. Each can hold a picture
instead:

- drawn by `tools/mklessonfont.py` at both sizes (3 bits a pixel small, 2 bits large), to sit with Google Sans
  SemiBold, in the ink's colour;
- named in a lesson file by a short word that `tools/mkcourse.py` and `tutorial/tp/teach.py` swap for its
  character, and that `tutorial/lessons/README.md` lists; `tutorial/check.py` measures it like a letter;
- about 100 to 250 bytes each, in the pack.

Which ones:

- **First, the black keys:** decision 2 picks drums with black keys, and the lessons can name only white keys
  ([5]). A black key's picture with its number from the left, in a frame, names one. Decision 11 then lights that
  frame when the key is held.
- **Then only where a picture replaces words:** a curved arrow for a knob's way (one each way), if "up" confuses a
  tester; a tick on the Done card and a speaker on the nine Listen steps are nice, not needed. The Done card's
  words are the firmware's (`fw/course.inc`, `c_to_step`), so its tick is a one-character firmware change. White
  keys stay `[5]`: the testers know it. Which of these, and how they look, is left to the session doing the work
  (2026-11-25, "Leave it to Fable"), each shown before a lesson uses it.
- **Each drawn and shown to the course's author before it goes into a lesson,** as words are (Today / Suggested).

A catch: a lesson from the computer on an FM-1 with no lessons on it is drawn in SLOOP's own font, where these
characters show as `?`. The computer can ask the FM-1 how many lessons it has (`ED_GUIDE` op 6) and send a word
instead.

When: the black key before part 1 of the lessons, which needs it.

**2026-11-25, the black key's picture chosen:** two white keys in outline and the black key between them, filled.
Shown beside the key alone (a bar) and beside itself with the white keys in a lighter ink: "The first one shows three
keys but its easier to see that they are keys"; "I think I like the first one better at the beginning we can just
tell the user that this is supposed to be a black key". So the first card that uses it says what it is. In a lesson
file it is `[BLACK 2]` (the second black key from the left), and `black 2` in Do.
