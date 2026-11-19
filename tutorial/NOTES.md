# Notes for the next session

Written 2026-11-17, after lesson 1 first ran on a real FM-1.

## Where things stand (end of 2026-11-17)

- **The plan is `PLAN.md`, draft 3.** It holds every decision so far, the course, and the order of work.
  These notes are the record of what was tried and what the tester noticed.
- **The FM-1:**
  - It has the old flash chip (`0x856014`) and m-bootloader 2.4.
  - It runs Teachie Peachie: SLOOP 3.3.7 plus the guide, the logo at the start and the START card.
  - HOUSE ONE is on it, left by the last lesson.
  - The way back to SLOOP 2.8.5 is M-Flasher's RESTORE FULL FLASH with `fm1-20261117-113255.bin`, in
    `Documents/M-Flasher/backups`.
- **The Mac:**
  - Its two old MIDI names were changed to "FM-1 Update" and "FM-1 SLOOP 3".
  - The two MIDI packages are in `tutorial/.venv`.
  - The installer: `file:///Users/david/Development/teachie-peachie-fm1/build/fw/installer-fm1.html`
  - The waiting program: `tutorial/.venv/bin/python -u tutorial/run.py welcome --fm1 --wait`, from this folder.
    It runs only while something keeps it running.
- **Lesson 1 has run on the real FM-1 several times,** most recently started with START.
- **HOUSE ONE is not built in Ableton yet.**

## What the first real run showed

- **The link is fast:** about 3 ms for a message there and back.
- **The fast run plays at about 97 ms a note,** where 75 ms was asked for. Each note takes three messages;
  sending the whole run in one message would fix it.
- **In the five-note phrase, three other keys were pressed before the lit one** (C5, E5, F5, then B4 after
  four seconds). The lesson gives no reaction to a wrong key. It should either react, or accept the phrase
  played by ear.
- **Each card took 5 to 7 seconds before the first press,** which is reading time. That is fine for a first
  run and worth watching with a child.

## The second and third real runs

- **The second run stopped during the fast run.** One answer from the FM-1 never arrived, and the lesson gave up.
  The project was still put back, byte for byte, so the safety net held.
- **The cause is not proven.** A test right after sent 600 messages with none lost. Chrome was still open, and
  a page there talking to the FM-1 at the same moment could do it.
- **The fix:** the connector now asks again when an answer doesn't come, and at the end of a lesson it lets go of
  every key so no note is left sounding.
- **The third run went through,** with five players, in 46 seconds, and no answer lost.
- **Still to do:** a lost answer to "what was pressed?" could lose a press. Numbering the answers would close that.
- **A lost answer also showed up on the simulated FM-1** (one in a whole lesson), so it is not only Chrome. That
  makes it something to find in the firmware, and the simulated FM-1 is the place to look.

## What the tester noticed

- **SLOOP 3 has five tracks and the lesson spoke of four.** The song should use all five and the lesson should
  say so. Done the same day: HOUSE ONE has a fifth player, a soft pad, and the lesson says FIVE PLAYERS. The
  library's Ableton tools were written for four tracks, so check them before building the song in Live.

- **The cards did not stand apart from SLOOP enough.** Navy cards next to SLOOP's dark screens looked too alike.
  Changed the same day: cards and strips are white with black words and one orange-red accent. The new build is
  in `build/fw/installer-fm1.html` and has to be installed again to show on the FM-1.
- **"The FM-1 still has my old song on it, how does that work?"** The lesson saves the project to the computer,
  loads its own song, and puts the project back when it ends. It may be worth saying so on the last card, and
  offering to leave the lesson's song on the FM-1.

- **Decided the same day: the lesson's song stays on the FM-1.** Going back to the old project was confusing. A
  copy of the old project is still saved on the computer first. The course assumes the FM-1 is being used for
  Teachie Peachie, not for SLOOP 3 work of its own.

- **"When Teachie Peachie loads I want our card to show, not SLOOP."** Built the same day: after the sailboat the
  FM-1 shows a white card, `TEACHIE PEACHIE / READY FOR A LESSON. / START ONE FROM THE COMPUTER.`, with `JUST PLAY`
  on OCT+. A lesson's first card replaces it. On the simulated FM-1 it only shows when `TP_HOME` is set, so that
  SLOOP's own test scenarios run as before.
- **The fourth real run** used the white cards and left HOUSE ONE on the FM-1: 53 seconds, no answer lost.

- **After the white cards ran on the FM-1, a round of changes, all made the same day:**
  - No orange at all. Cards are white with black words only, and the edge bar is gone.
  - A button's name gets a thin frame: `[PLAY]` in a lesson's words, and the OCT labels by the answers.
  - A new card after the blinking-light card says the songs are made in SLOOP, describes SLOOP in one line, and
    asks for PLAY to open the song.
  - In the fast melody, the first two keys blink and are played for real. The screen then says
    `FASTER FASTER FASTER`, and the third key pressed sets the run off.
  - The line after it is `HA, THAT WAS TOO FAST. NOBODY CAN PLAY THAT BY HAND. A SEQUENCER CAN.`
  - The lesson ends on a card with `REPLAY LESSON` on OCT- and `SLOOP` on OCT+. A menu will go there later.

- **Wording, changed the same day:** not "a band in a box" but `SLOOP HAS 5 TRACKS WITH DIFFERENT INSTRUMENTS.`;
  the strip says `FIVE TRACKS. ONE SONG. LISTEN.`; the end card offers `REPLAY` and `NEXT` only. `NEXT` says
  `LESSON 2 SOON.` until there is a lesson 2.
- **A question to settle: could the lessons run on the FM-1 with no USB cable,** with an AI mode when it is
  connected? First answer: yes. It needs a small lesson player inside the firmware, and the lessons and songs
  stored on the FM-1. Measured room: about 6 KB free for code, which should hold the player. The lessons and
  songs would not fit there; the four user-sample slots are 80 KB each (corrected: not 150 KB), so the course
  would take one or two of them, without cutting anything out of SLOOP. The lessons have to become data first, which was planned anyway.

- **Decided the same day:** keep building lessons on the computer for now. In the end the version without AI
  lives entirely on the FM-1, with no cable. The connected mode with Claude stays alongside it.

- **The logo at the start, on white,** with WITH SLOOP 3 under it, in place of SLOOP's sailboat. Stored in
  nine flat colours, about 3.3 KB. SLOOP's own tests caught a colour bug in the first version.
- **"I still want the buttons... the computer should see that I pressed start."** The start card now has
  `START` on OCT+ and `JUST PLAY` on OCT−. START tells the computer, and the waiting program starts the lesson.
  It worked on the real FM-1 the first time.
- **The plan was rewritten as draft 3** at the end of the day.

## 2026-11-18: all thirty lessons drafted

- **The lessons are Markdown files now,** in `lessons/`, one each. `lessons/README.md` says how to go through
  them on the FM-1, where to write notes, and how a lesson file is written. Lesson 1 was moved over and plays as
  before.
- **Each lesson was played through on the simulated FM-1** by the robot learner, which does what each step asks.
  Most steps are checked against the FM-1 itself: the lesson reads that the kick really is in box 1, the track
  really is muted, the page really is the piano roll. So the button moves in the lessons are known to work on
  this firmware.
- **None of the twenty-nine new lessons has been played by a person.** Wording, pace and how it feels are all
  still to judge.
- **The song question:** a lesson names its song and section, so a song can be swapped later. Every lesson uses
  HOUSE ONE for now, and each file says what in it is tied to the song.

### What building them showed about SLOOP 3

- **The white keys mean two things on the drum track:** drums when played, boxes while SEQ is held. The drum
  that boxes write is the drum played last. Tapping a box with the wrong drum picked can take a note out. The
  lessons say "play its key first" each time, and the hints repeat it.
- **Black keys also pick a drum, but each press flips between two.** The lessons use the white keys instead.
- **While the FM-1 saves, it misses presses** and answers nothing for a moment. Copying a page saves too. This
  is very likely what the "lost answers" of 2026-11-17 were. The connector now asks again and keeps what a late
  answer brings.
- **SLOOP saves by itself** when the music has been stopped for a few seconds. Lesson 29 wrecks the project
  while it plays for that reason.
- **Opening the saved project stops the music.**
- **HOME on the TRACKS page opens the visualiser.** The lesson cannot see that yet.
- **The echo knob on the SENDS page is labelled ECHO,** so the lessons say echo there.
- **A key played on a muted track still sounds.** Only what the sequencer plays is muted. Lessons 11, 21 and 22
  use that: the lead is muted and the learner is the lead.

### What the lessons cannot do yet

- **Read the key, scale, chord, arpeggiator, the drum that is picked, or the cursor** in the piano roll and the
  song's lines. Lessons 11, 12, 13, 22 and 26 count knob clicks in those places. A small addition to the guide
  in the firmware would let them check the result.
- **OCT+ is the "done" button on free steps,** so the octave can't go up during them, and lesson 23 had to be
  written around it. OCT- is left alone, so undo works.
- **Lesson 29 only describes** recording a song as a sound file.
- **The path for people who know grooveboxes** is only started: few steps are marked *(beginners only)* so far.

### The cards change colour again

- **"I just think it will better separate the tutorial from Sloop."** Cards and strips are now a creamy peach,
  `#FFCBA4`, with dark red words, `#960000`, in place of white and black. The frames round button names and the
  answers by the OCT buttons take the same red.
- **The first peach tried, `#F4A834`, looked "almost yellow ... or orange"** on the simulated FM-1. Three peaches
  were shown side by side on a real card, and `#FFCBA4` was kept. A colour is best chosen from a picture of a
  card, not from its number.
- **The logo at the start moved onto the same peach,** with WITH SLOOP 3 in the same red. The logo itself was
  redrawn (`assets/logo/teachie-peachie.jpg`; the same art on white is beside it). It is stored in eight flat
  colours and takes about 2.9 KB, a little less than the old one. About 3.8 KB of code room is left.
- These are changes to the firmware, so it has to be installed again to show on the FM-1.

### To check first on the real FM-1

- That the waiting program still finds the FM-1 and START still works. The part of the connector that listens
  for answers was changed and has only run on the simulated FM-1 since.
- That a lesson's start feels quick enough. A new project plus the song takes about four seconds after BEGIN,
  with the begin card still showing.
- That the top strips leave the dials readable (lessons 11, 13, 14, 19, 21, 22, 26, 28).

## To do next

The order of work is in `PLAN.md`, section 13.
