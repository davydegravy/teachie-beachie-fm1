# Notes for the next session

Written 2026-11-17, after lesson 1 first ran on a real FM-1.

## Where things stand (end of 2026-11-17)

- **The plan is `PLAN.md`, draft 3.** It holds every decision so far, the course, and the order of work.
  These notes are the record of what was tried and what the tester noticed.
- **The FM-1:**
  - It has the old flash chip (`0x856014`) and m-bootloader 2.4.
  - It runs Teachie Beachie: SLOOP 3.3.7 plus the guide, the logo at the start and the START card.
  - HOUSE ONE is on it, left by the last lesson.
  - The way back to SLOOP 2.8.5 is M-Flasher's RESTORE FULL FLASH with `fm1-20261117-113255.bin`, in
    `Documents/M-Flasher/backups`.
- **The Mac:**
  - Its two old MIDI names were changed to "FM-1 Update" and "FM-1 SLOOP 3".
  - The two MIDI packages are in `tutorial/.venv`.
  - The installer: `build/fw/installer-fm1.html`, opened in Chrome
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
  Teachie Beachie, not for SLOOP 3 work of its own.

- **"When Teachie Beachie loads I want our card to show, not SLOOP."** Built the same day: after the sailboat the
  FM-1 shows a white card, `TEACHIE BEACHIE / READY FOR A LESSON. / START ONE FROM THE COMPUTER.`, with `JUST PLAY`
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
  nine flat colors, about 3.3 KB. SLOOP's own tests caught a color bug in the first version.
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
- **HOME on the TRACKS page opens the visualizer.** The lesson cannot see that yet.
- **The echo knob on the SENDS page is labeled ECHO,** so the lessons say echo there.
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

### The cards change color again

- **"I just think it will better separate the tutorial from Sloop."** Cards and strips are now a creamy peach,
  `#FFCBA4`, with dark red words, `#960000`, in place of white and black. The frames round button names and the
  answers by the OCT buttons take the same red.
- **The first peach tried, `#F4A834`, looked "almost yellow ... or orange"** on the simulated FM-1. Three peaches
  were shown side by side on a real card, and `#FFCBA4` was kept. A color is best chosen from a picture of a
  card, not from its number.
- **The logo at the start moved onto the same peach,** with WITH SLOOP 3 in the same red. The logo itself was
  redrawn. It was stored in eight flat colors and took about 2.9 KB, a little less than the one before it.
  About 3.8 KB of code room was left then. (That logo was replaced on 2026-11-20: see the renaming below.)
- These are changes to the firmware, so it has to be installed again to show on the FM-1.

### To check first on the real FM-1

- That the waiting program still finds the FM-1 and START still works. The part of the connector that listens
  for answers was changed and has only run on the simulated FM-1 since.
- That a lesson's start feels quick enough. A new project plus the song takes about four seconds after BEGIN,
  with the begin card still showing.
- That the top strips leave the dials readable (lessons 11, 13, 14, 19, 21, 22, 26, 28).

## Notes from the play-through on the real FM-1, from 2026-11-18

Noted as they come. Nothing here is done until it is marked done.

- **The list of lessons: show the lesson's name in the large font.** "Will 2 lines fit in the larger font? If
  they can't fit just keep it how it is." Measured: in the large font 26 of the 30 names take one or two lines,
  and four take three (lessons 6, 24, 26 and 29). Three large lines still fit between a small heading and the
  answers, so every name fits. The catch: a card is one size throughout today. Either the list card becomes all
  large and loses its `TURN [SELECT] TO CHOOSE` line, or the firmware learns to draw one card in two sizes, which
  means installing it again. **Done 2026-11-20** the second way: a large
  card's line that starts with `^` is drawn small, so the list is `TURN [SELECT] TO CHOOSE.` small, then `LESSON 6`
  and the name large.
- **American spelling: "color", not "colour".** On the FM-1's screen that is lesson 20's name and one of its
  cards, and FAVOURITE on three cards of lesson 9. The lesson files' notes and the other documents use British
  spelling throughout (colour, favourite, centred, neighbouring) and change with it, as does the name of lesson
  20's file. SLOOP's own messages say FAVOURITE; those are SLOOP's words and stay (decided 2026-11-19: "Leave SLOOP's"). Lesson 9's three cards say
  FAVORITE since 2026-11-20 (one of them was being rewritten). Lesson 20 is "Color a track" since the same day
  ("Lesson 20 says Colour"): its name, its last card, its file and its picture. With that, nothing on the
  FM-1's screen has British spelling but SLOOP's own messages. The prose of the documents and of the lessons'
  notes was changed on 2026-11-20 too; what is left is this entry, the notes that quote the spelling, SLOOP's
  messages, and the comments in the code.

- **Stuck in lesson 2 at "ONE KICK IS LONELY", and SHOW ME did not help** ("I am stuck"; "if I do it correctly
  it still does not progress"; "I press show me [and] it removes the drum that I put in"). Read from the FM-1
  itself: the drum picked was the hat, and boxes 5, 9 and 13 held hats. A blinking box key tapped with SEQ not
  held plays that drum and makes it the drum that boxes write, and the lesson said nothing. After that, correct
  taps wrote hats, and SHOW ME tapped the same boxes and took them out again. The robot learner never slips and
  never gets stuck, so neither fault had shown.
  **Fixed the same day,** in the lesson player (no firmware change):
  - a step that writes a drum into boxes makes sure that drum is the one picked when it starts;
  - when a key is played with SEQ not held during such a step, the strip says `THAT PLAYED A DRUM. HOLD [SEQ]
    FIRST.` and the lesson quietly picks the right drum again;
  - SHOW ME plays the drum's own key first, then makes the move, and writes in whatever is still missing, so the
    step always ends;
  - the checker can play every lesson with a clumsy robot that taps a blinking box before holding SEQ
    (`check.py --run --clumsy`).
  Still open: the same slip on the bass track changes which note is written (the lesson goes on, with another
  note than the one it named).
- **The `STUCK?` card also comes up when the learner has only stepped away.** Whether it should wait longer, or
  only come when asked for, is to decide.
- **Lesson 17 (fills and rolls) is the most confusing so far; to revisit.** Played twice on 2026-11-19. Its
  step 10, the fill note (`HOLD [SEQ], HOLD BOX 12, PRESS [OCT+]`), took 134 and 154 seconds, with the hint and
  `STUCK?` twice each time. The hint `KEEP [SEQ] AND THE BOX DOWN. THE TOP SAYS FILL.` was not understood
  ("what does the box down mean?"). Their wording: `HOLD [SEQ] + HOLD KEY 12 + PRESS [OCT+]`. Their idea: teach
  that some moves mean holding two buttons and then pressing a third. Also seen on the FM-1: marking a box
  marks every drum in it, and one press too many gives NO FILL. The whole account and a sketch are in the
  lesson's own tester notes. **Done 2026-11-20** from the sketch: see "The round from the
  notes" below.
- **A framed name for the keys, like the buttons have.** "Maybe we could create a way to reference the keys,
  like put a border around [12]. Start by saying [key 12] and then after they have seen it a few times"
  (the shorter form, `[12]`, by itself). Came out of lesson 17, where the same key was KEY 12 in one step and
  BOX 12 in the next. What it gives: one name for a key whatever it is doing (a drum, a box while SEQ is held,
  a mute while GLO is held), drawn the way `[SEQ]` and `[OCT+]` are, so a move reads as a row of frames:
  `HOLD [SEQ] + HOLD [12] + PRESS [OCT+]`.
  The screen can already do it: the guide frames any words between square brackets, so no firmware change and
  nothing to install. The work is in the lessons: about 68 lines on screen name a key by number today (WHITE
  KEY n 19 times, KEY n 38, BOX n and BOXES 11), plus the hints, and the checker measures each one again since
  a frame is a little wider than bare words.
  To decide: where `[KEY 12]` gives way to `[12]` (a fixed lesson, or its first use in each lesson); whether
  the picture of the grid still says "box".
  The black keys ("how do you usually reference the black keys?"): no lesson asks for one black key today.
  Lesson 11 speaks of them as a group (`PLAY ONLY BLACK KEYS FOR A WHILE`); lessons 12 and 18 mention them
  only in their notes. So the numbers can mean white keys counted from the left, said once where lesson 2
  teaches the keys as boxes. If a lesson ever needs one black key: by its neighbor (`THE BLACK KEY AFTER [3]`),
  not a second set of numbers.
  **Done 2026-11-20:** `[KEY 12]` the first time a lesson names a key, `[12]` after; "box" stays for the squares on
  the screen. A card at the start of lesson 1 says what the frames mean.
- **The buttons are hard to read on a black FM-1 unless they are lit.** SLOOP has the answer already: hold HOME,
  turn SELECT to `LIGHTS 2/4`; KNOB 1 LIGHTS (OFF, LOW, MID, HIGH) makes every button glow, KNOB 2 KEYS (OFF,
  C KEYS, WHITE, ALL) the keys too. "Oh that is so much better. ... can we just make the default for all of our
  users to have the button lights on?"
  - The battery: a glowing light is on for 3, 6 or 12 millionths of a second each time its column comes round
    (LOW, MID, HIGH), where a fully lit one stays on for 250 (`hal/fm1/panel.c`, `panel_tick`). All fourteen
    buttons on HIGH together use about two thirds of what one lit button uses. Worked out from the code, not
    measured.
  - Where it starts from: `hs[HS_N]` in `fw/main.c`, LIGHTS 0 (OFF). The settings are kept with the saved
    project and read at power-on, so a new starting value reaches only an FM-1 with nothing saved yet. For
    everyone else the lessons can switch the glow on when the start card comes up, if it is OFF.
  - KEYS is best left OFF in the lessons: a glow on every key would compete with the key the lesson blinks.
  **Done in the firmware the same evening (2026-11-19), not installed yet.** "Sorry I meant full lit mode. I think
  for beginners it might be good to have the buttons backlight all the time so they can clearly identify each
  key." Taken as LIGHTS on HIGH, SLOOP's brightest glow (a light lit in full would hide the ones SLOOP and the
  lessons use to point). Two lines in `fw/main.c`: `hs[]` starts with LIGHTS 3, and START on the start card
  sets LIGHTS to HIGH if it was OFF (LOW and MID are left as chosen; KEYS is not touched). 16 bytes; 448 bytes
  of room left. Someone who wants the glow off after a lesson turns it off in HOME > LIGHTS; the next START
  puts it back.
- **No way back to the lessons from SLOOP.** After JUST PLAY (to change LIGHTS in SLOOP's menu): "I can adjust it
  in SLOOP, but I can't get back to the lesson from there. I feel like we need to have a button that users can
  press to go back to the lesson menu." Today the start card comes only at power-on, or when the computer's
  program asks for it (done by hand for them this time). With no computer that means switching off and on.
  Idea, not built: both OCT buttons held two seconds brings the start card back from anywhere in SLOOP. It is
  already the way out of a lesson on the FM-1 alone, so one move would mean "to the lessons" everywhere. A tap
  of both stays SLOOP's own (the octave back to 0). A few dozen bytes of firmware; 448 are free.
  **Built 2026-11-20** (`course_tick` in `fw/course.inc`): with no card up, both OCT buttons held two seconds
  bring the start card back.
- **A lesson's cards switched the glow off.** In lesson 18, on its first card (`HOLD [FX], AND EVERY KEY IS AN
  EFFECT ...`): "I can't see the keys, I'm trying to see the FX key but it is very dark. I need the lights to
  stay on." Their settings read from the FM-1: LIGHTS HIGH, KEYS ALL. The cause: a card asks the guide for "only
  these lights", which puts out SLOOP's own lights and the glow with them; a strip does not.
  - **Done at once, in the computer's program** (`tutorial/tp/fm1.py`, `lights`): with "only", the fourteen
    buttons are asked for at level 1, the glow, as bright as HOME > LIGHTS says. No firmware change.
  - **Done in the firmware the same evening, not installed yet:** the glow (the buttons, and the keys that KEYS
    names) is put back under a card. `leds_glow` in `fw/main.c`, called from `leds` and from `guide_leds` when
    the guide asks for "only". It serves the FM-1 alone and the computer's program alike. 32 bytes; 416 bytes
    of room left.
  - **Left as it is:** on the firmware of before, the keys still go dark on a card when the computer leads
    (the buttons do not). Not worth another change to the computer's program: the new firmware does it.
- **A card that asks for a move hides what the move does.** Found in lesson 19 (step 9, `TAP [FX] UNTIL THE
  SCREEN SAYS DELAY` on a card that covers the screen: 119 seconds, ended by SHOW ME). A card fills the screen,
  so the page's name, SLOOP's messages (UNDONE, PROJECT SAVED, LOCK: LAYER OPEN, AGAIN: CHANGES LOST) and
  everything a turn or a press changes are out of sight, and with no answer at its bottom the card looks
  stuck. Fourteen steps are built this way:
  1/3, 1/13 (big), 6/12, 6/16, 10/10, 11/8, 17/4, 18/9, 19/9, 19/12, 20/10, 21/10, 29/6, 29/8 (lesson/step).
  The slow ones of this play-through are among them: 17/4 (18 s), 18/9 (55 s, STUCK?), 19/9 (119 s, STUCK?
  twice). Two tell the learner to read the screen they cover: 19/9 and 21/10 (`TAP [EDIT] UNTIL THE SCREEN
  SAYS FILTER`).
  The rule to apply: a card explains and ends with an answer; the move that follows is a strip. The checker
  can refuse a card that has a Do and no answer.
  **Done for lessons 21 to 30 the same evening ("Yes, fix them"):** 21/10, 29/6 and 29/8 are each a card with an
  answer and then a strip (lesson 21 has 19 steps now, lesson 29 has 15). Lessons 1 to 20 are as they were:
  1/3, 1/13, 6/12, 6/16, 10/10, 11/8, 17/4, 18/9, 19/9, 19/12, 20/10. The checker's rule is not written yet.
  **Done for lessons 1 to 20 on 2026-11-20 ("Yes, fix them"):** ten of the eleven are each a card with an answer
  and then a strip (1/3, 6/12, 6/16, 10/10, 11/8, 17/4, 18/9, 19/9, 19/12, 20/10; step numbers as they were).
  1/13 stays: it is the FASTER card of the melody game, large, with nothing to read under it. So the checker's
  rule, when written, is for cards and leaves the large ones alone.
  **Written 2026-11-20:** a card with a Do or an Until and no answer is refused.
- **A strip at the bottom covers the dials.** Found in lesson 20 (steps 11 and 12: the song's filter, `[FX]` and
  KNOB 1; 84 and 57 seconds, STUCK? each time): "perhaps something is being obscured?" The four dials and their
  values sit along the bottom of SLOOP's screen, under the strip. Lesson 19 already puts its strips at the top
  for this reason. Nineteen bottom strips ask for a knob: 6/4, 6/5, 6/7, 9/10, 9/12, 11/10, 12/4, 17/14, 20/5,
  20/7, 20/8, 20/11, 20/12, 20/13, 26/4, 26/15, 27/4, 28/10, 28/11 (lesson/step). Each is to look at on the
  screen: where the value shows, and whether the strip is on it.
  The rule to apply: a step that turns a knob gets its strip at the top, and names what to read (`UNTIL FILT
  SAYS OFF`). The checker can warn on a bottom strip whose Do turns a knob.
  **Done for lessons 21 to 30 the same evening,** after looking at each lesson's screens:
  - 27/4 and 27/13 (the song's filter) and 28/10 and 28/11 (a held box's LEVEL) are strips at the top, and say
    what to read: `[FX] AND KNOB 1 RIGHT UNTIL FILT SAYS OFF.`, `HOLD [SEQ] + BOX 4: / KNOB 2 TO HARD.`
  - 26/4 and 26/15 stay at the bottom: SELECT changes the page there, and the page's name is at the top.
  - 25/5, 25/14 and 25/17 were three lines high and sat on the LOOP REC CHAIN ALL row; they are two lines now.
  - Where things are on SLOOP's screen, for the next pass: the page's name and SLOOP's messages at the top; with
    a box held, its rule (ALWAYS, FILL) at the top and SOUND, LEVEL, RATCH, NUDGE on the dials; with FX held,
    the effects' names in the middle and FILT, DUST, DUCK, FILT P on the dials.
  Lessons 1 to 20 are as they were: 6/4, 6/5, 6/7, 9/10, 9/12, 11/10, 12/4, 17/14, 20/5, 20/7, 20/8, 20/11,
  20/12, 20/13.
  **Done for lessons 1 to 20 on 2026-11-20,** after looking at each page on the simulated FM-1 with the button
  held (step numbers as they were):
  - At the top now, naming the dial: 11/10 (`KNOB 2 RIGHT UNTIL / SCALE SAYS MIN.`), 12/4 (`KNOB 3: 1 CLICK
    RIGHT. / TILES SHOW CHORDS.`), 17/14 (`[ARP] + KNOB 1: RATE, / HOW FAST IT ROLLS.`), 20/5, 20/7, 20/8, 20/9
    (the insert page) and 20/11 to 20/13 (`FILT GOES PAST LP 40.`, `UNTIL FILT SAYS OFF.`).
  - Left at the bottom: 6/4, 6/5, 6/7 (the tempo is the big number at the top) and 9/10 (the family's name is
    at the top left).
  - Three lines made two, because the third line sat on something to read: 9/5, 9/11, 9/12 (the fourth row of
    sound names) and 18/10 (the lowest row of effect names).
  - More of where things are: with GLO held the screen is the MIX page and its FILL tile lights during a fill;
    with ARP held it is the ROLL page, one dial, RATE; on the KEY page the dials are ROOT, SCALE, CHORD, KEYS;
    on an effects page (INSERT, SENDS, DELAY) the page's name is a third of the way down, so a strip at the
    top, up to three lines, leaves the name, the values and the dials in view.
- **A slash inside a word was taken for a new line.** `LFO 1/3` came out as `LFO 1` and `3.` on the next line
  (22/7, 22/15 and three hints of lesson 26). Now only a slash with a space on each side starts a line
  (`words` in `tutorial/tp/lesson.py`; the packer for the FM-1 alone uses the same). Done.
- **The computer's program reads a lesson's file again when the lesson begins** (`tutorial/run.py`), so a
  lesson can be mended while another is being played, with no restart. Done.

### What the log of 2026-11-19 shows (lessons 3 to 15 on the real FM-1)

- **Thirteen lessons in about 55 minutes,** pauses for note-taking included. Seconds each: lesson 3, 293; 4, 109;
  5, 239; 6, 193; 7, 203; 8, 241; 9, 337; 10, 148; 11, 306; 12, 139; 13, 118; 14, 302; 15, 678. Most are shorter
  than the five to ten minutes the plan aims for.
- **No answer from the FM-1 was lost or late** in any of them.
- **A fault: lesson 4 was cut off on its end card.** The card had been up for two minutes while a note was being
  written, the program gave up waiting, and the lesson had to be played again. Fixed the same day: a person may
  leave any card or step up for as long as they like.
- **A fault: `THE WHOLE BAND, WITH YOUR KIT` played silence** in lesson 4, because the music had been stopped
  earlier. Fixed the same day: the steps where the band joins start the music themselves (lessons 2, 3, 4, 8, 10).
- **The drum fix worked for real:** in lesson 3 a key was twice played with SEQ not held, the strip said so, the
  right drum was picked again, and the lesson went on.
- **Brought back to the right page** three times: twice in lesson 7 (off the drum grid), once in lesson 9 (off
  the SOUNDS page).
- **`STUCK?` came up once,** in lesson 11 at the KNOB 4 step, the one whose strip gives no direction.
- **READY was pressed too early three times** (lesson 7 once, lesson 15 twice) and the lesson said what was
  still needed.
- The tester's notes on single lessons are in each lesson's own file (3, 4, 6, 8, 9, 11, 14).

## 2026-11-19: the course on the FM-1 alone

Asked for the same day: "I was hoping we could try to have it on the FM-1 only", for two children to try with no
computer.

- **The lessons already ran with no AI and no internet;** what they needed was the computer, as the thing that
  plays them. That part is now inside the firmware too.
- **How it works:**
  - `tools/mkcourse.py` packs the same Markdown lessons into one pack. All thirty with their songs are about 35 KB.
  - The pack is kept in one of the FM-1's four sample slots, behind a header SLOOP's own samples never have, so
    SLOOP leaves it alone and shows the slot as empty. `tutorial/device.py --install` puts it there over USB.
  - `fw/course.inc` is the lesson player in the firmware. START on the start card, with no computer answering for
    a second, begins it.
- **Tested with no computer leading:** a robot only presses, on a simulated FM-1 running the real firmware, and
  the FM-1's own course does the rest. All thirty lessons play through, also with the robot that slips.
- **Not yet tried on a real FM-1.**
- **Room:** the player is about 8.3 KB of code and the firmware had 3.8 KB free. SLOOP's import of an old SLOOP
  2.8 project at the first start was left out to make room; about 450 bytes are free now (416 after the changes for the buttons' glow; 1,156 after the
  new logo of 2026-11-20, which is packed smaller; 1,040 after the corner tag of the same day).
- **A way out of a lesson:** both OCT buttons held for two seconds bring back the lesson's first card. (With a
  computer leading there is no such way out yet.)
- **What it does not do yet:** remember the next lesson after the FM-1 is switched off; SHOW ME does the step
  directly and does not press the buttons one by one; there is no log.

## 2026-11-20: Teachie Beachie

- **The project is called Teachie Beachie now** ("rename everything from Teachie Peachie to Teachie Beachie"), with
  a new logo: the peach, with the name and a beach sunset inside it.
  - Changed: the words on the FM-1 (the START cards, lesson 29's card), the installer page, the documents, the
    lessons' files, the song's credit, `TEACHIE-BEACHIE.md` (the file's name too), the logo's picture
    (`assets/logo/teachie-beachie.jpg`; the two old pictures are removed).
  - The repository and the folder too, the same day ("lets change the repo and the folder so that task is
    complete"): `davydegravy/teachie-beachie-fm1` on GitHub (the old address still leads there) and
    `~/Development/teachie-beachie-fm1` on the Mac, with the two addresses in `tools/teachie_page.py` and the
    paths inside the Python environment (`tutorial/.venv`) put right.
  - Not changed, on purpose: the code's own short names (`tutorial/tp`, `app/tp_logo.h`, `TP_PAPER`, the lesson
    pack's `TPCO` mark), which nobody sees.
- **The logo is packed smaller** ("make this as small as possible to save space"). Same size on the screen as the
  old one (168 wide, 197 high, eight flat colors).
  - Written as runs of one color, as before, it would take 2,915 bytes (the old logo: 2,946).
  - Now most rows are written as "the row above, each run's start moved a little" (half a byte a run):
    2,118 bytes. The FM-1 reads it back with a few more lines of code (`tp_logo_draw` in `app/vis.c`).
    `tools/mklogo.py` reads its own output back the FM-1's way before it writes anything, and on the simulated
    FM-1 the start screen was compared with the picture: all 24,906 pixels the same.
  - Tried and worse: writing each pixel as "same as the one above" before making runs (3,550 bytes: in a drawing
    of curves nearly every edge moves a little on every row).
  - The firmware has 1,156 bytes free (416 before).
  - What would make it smaller still costs the picture: a narrower logo (152 wide: about a sixth less), or each
    row drawn twice (about half, with coarser edges). Not done.
- **"Why don't we take some of the sample space?"** Nothing stands in the way; it was not needed until now.
  - The FM-1's memory, in order: the firmware (up to 0x48000), SLOOP's built-in drum samples right behind it
    (194 KB, `assets/samples.bin`), then at 0x78000 the user's own FM6 sounds and kits, then the projects. The
    installer writes the firmware and the built-in samples as one block, and that block must end before 0x78000.
  - 2,478 bytes at the end of that block are unused. Moving the samples up by that much gives the firmware that
    room and no sound changes. It is a change of the samples' address in three places (`tools/mkfw.py`,
    `fw/main.c`, `app/store.h`), then the full tests. Not done, the tester's call the same day: "lets not remove drum
    samples if we do not need the space yet." The samples stay where they are until the room is needed.
  - More than that means shorter or fewer built-in drum samples, which SLOOP's kits use: every KB taken is a
    little drum sound gone, and the songs from SLOOP's library would sound different.
  - "Why can't the four slots hold firmware code?" They could be made to, but it would be a firmware in two
    parts that must always match. The FM-1's installer puts in one checked block (the firmware and the built-in
    samples) and starts it; the slots are outside that block and only the running firmware writes them. Code in
    a slot would arrive in a second step, could be wiped by loading a sample into that slot, and after a
    firmware update would be the wrong version: with data the FM-1 can check and say "no lessons", with code
    it crashes. So the slots hold what the firmware reads (lessons, songs, a picture), not what it runs. If
    room gets tight, data moves there first: the logo's 2.1 KB could ride in the lesson pack.
- **The `seachest` folder is `backups` now** (the tester, at lesson 29's card: "is that a remnant of boatswain? can
  we change that to something else?", then "Yes, BACKUPS"). The card says `IN ITS BACKUPS FOLDER`; the folder is
  `tutorial/backups/`, with the copies kept so far. `logbook/` keeps its name: it never shows on the FM-1.
- **A strip in the corner, for lesson 30** (the tester, while making the song: "for the final lesson I think we
  should try to minimize the real estate we are taking up maybe just the top left corner [save]x2").
  - The strip there was `MAKE YOUR SONG. / WHEN IT IS DONE: [SAVE] TWICE.`, which wraps to three lines: 81 of the
    screen's 240 rows, over the dials, for the whole of the song-making.
  - New kind of step, **Strip corner**: a tag in the top left corner, as wide as its words (`[SAVE] X2` is 97 by
    26 pixels). In the firmware's guide it is strip number 3 (`guide_draw` in `fw/main.c`), in the pack
    `CK_STRIP_CORNER` (the three strips now sit next to each other, 3 4 5, and REPLY, WAIT, PLAY, ECHO moved up
    by one: a pack and a firmware must be from the same build, as before). 116 bytes; 1,040 are free.
  - Lesson 30: a card before it says what the tag means (`WHEN YOUR SONG IS DONE, PRESS [SAVE] TWICE. / THE
    CORNER OF THE SCREEN REMINDS YOU: / [SAVE] X2`), then the tag. Ten steps now.
  - What the tag hides: SLOOP uses every corner. Top left is the tempo (TRACKS and the sound pages) or the
    page's name (STEPS, SONG, KEY, SOUNDS). Top right would hide `P1` (which page) or the small print next to the
    name. The picture `tutorial/lesson-30-corner-tag.png` shows today's strip, the tag top left and top right
    on four pages. Top left is built, as asked; left or right is the tester's call.
  - Checked the same day on the simulated FM-1: lesson 30 played through connected and alone; lessons 1, 13 and
    24 alone (for the moved numbers); `make test`. An FM-1 with the firmware from before this day draws the tag
    as a one-line strip along the bottom.
- **Lesson 30 shut the tester in, and its ending is changed.** After SAVE twice the card `IS YOUR SONG
  FINISHED?` held back both answers until the song had 3 pages, 4 tracks and 3 lines: `NOT YET` did nothing, and
  SLOOP's screen (needed to chain the pages) could not be reached. The lesson program was stopped to get out; the
  song stayed on the FM-1.
  - The cause: an `Until` on a card holds back every answer, in the lesson program (`Player.answer`) and in the
    course on the FM-1 alone (`c_answered`). No other lesson has an `Until` on a card with two answers, and
    `tutorial/check.py` now refuses one. The engines are as they were.
  - The tester's call on the ending: say what the goal was, let the learner finish when they choose, and
    congratulate them even if the song is not all there. The card is now `IS YOUR SONG FINISHED? / THE GOAL WAS
    3 PAGES, 4 TRACKS AND A SONG OF 3 LINES. / YOU DECIDE.` with no check. With no chain, `song mode on; play`
    loops the page that is up (tried on the simulated FM-1).
  - Seen on the way: to let the tester carry on with the song they had, lesson 30 was started once more with its
    first step changed so that it did not make a new project. A way to rejoin a lesson without losing what is on
    the FM-1 does not exist yet.
  - The new ending was played on the real FM-1 the same day, with the tester's song: no fault. On the simulated
    FM-1 lesson 30 played through connected and alone.
- **All thirty lessons have now been played on the real FM-1 by the tester** (lessons 17 to 30 on this day). The
  robot replay of all thirty on this day's build has not been run: it waits for the tester's word.

## 2026-11-20: the round from the notes

After the thirtieth lesson: "go back and make adjustment based on all of the notes. The next thing I want to test
is all of the lessons on device, unplugged. Please run the robot replay after the changes have been made. When
everything passes I want to put the updated firmware back on my FM-1 and try it again."

**What was asked and answered.** Four questions went to them; they answered with one message: "Yes I like that I think
in the very beginning we should have one card that explains the buttons and holding and pressing and also what we
are going to call them." Taken as: framed key names, and the new card. The three questions they did not answer
were settled as follows, and said to them at once:
- the way back to the lessons: built (it was their own wish of 2026-11-19);
- lesson 30's tag: left in the top left corner, as built;
- of the extras, the three that come from their notes are in (`3 PAGES IN A ROW`, the erase step in lesson 10,
  large names in the list of lessons); when `STUCK?` comes up is left as it was.

**The lessons.**
- **Two cards about names, at the very start** (lesson 1, steps 3 and 4). The first try was one card (`[PLAY] IS A
  BUTTON. / [KEY 5] OR JUST [5]: ...`); they found it confusing so early and asked for key 5 to blink. Now: `A NAME IN
  A FRAME / IS A BUTTON: [OCT+] / PRESS: DOWN AND UP. / HOLD: KEEP IT DOWN. / [OCT+] IS BLINKING. / PRESS IT.`, then
  `A NUMBER IN A FRAME / IS A WHITE KEY. / COUNT FROM THE LEFT. / [KEY 5], OR JUST [5], / IS BLINKING. / TAP IT.` Each
  has something to press at once, so a blinking light still means "press this". No firmware change: the key card
  is an Echo of one key.
- **Framed key names everywhere:** 75 lines in 16 lessons. `[KEY 12]` the first time a lesson names a key, `[12]`
  after. KEY, WHITE KEY and BOX with a number are all the same framed name now; "box" is only the square on the
  screen. Lists read `[5], [9] AND [13]`.
- **Lesson 2:** the free step says what can be done (`ADD KICKS, OR TAP A FULL BOX: IT EMPTIES.`).
- **Lesson 3:** a card says what picking a drum is (`PLAYING A DRUM'S KEY PICKS THAT DRUM. / THE BOXES THEN WRITE
  IT.`), the strips say `PICK THE SNARE`, `NOW PICK THE HAT`, and the free step says `WRONG BOX? TAP IT AGAIN.`
- **Lesson 6:** `TURN [SELECT] UP PAST 135.`
- **Lesson 7:** the card of moves says a full box empties.
- **Lesson 8:** a card before the free step (`MAKE IT YOUR OWN. / PLAY A KEY TO CHOOSE A NOTE. / HOLD [SEQ], TAP A
  BOX: IT GOES IN. / TAP AGAIN: IT GOES OUT.`).
- **Lesson 10:** after undo, redo brings the take back, then `HOLD [REC] FOR TWO SECONDS` erases the track (asked
  for in the final test: "how do I delete a whole track?"). 19 steps.
- **Lesson 11:** `KNOB 4: ONE CLICK RIGHT. SNAP.`
- **Lesson 17, the fill note, in stages** (steps 10 to 15): pick the tom; a card (`THE NEXT MOVE TAKES THREE
  FINGERS. / HOLD TWO THINGS DOWN, / THEN PRESS A THIRD.`); `HOLD [SEQ] + HOLD [12]. / THE TOP SAYS STEP 12.` (ends
  when the tom is in the box; hint `HEAR A CRASH? [SEQ] WAS NOT DOWN. HOLD [SEQ] FIRST.`); `KEEP BOTH DOWN. /
  PRESS [OCT+] ONCE.` (hints: the move again, and `TOP SAYS NO FILL? [OCT-] PUTS IT BACK. THEN [OCT+] ONCE.`); a
  card with an answer that says it worked and to let go (it takes OCT+, so one press too many does nothing);
  `NOW [GLO], NOT [SEQ]: / HOLD [GLO], TAP [12].` 20 steps. Seen on the simulated FM-1 on the way: the tom is
  written when the key goes down; OCT- with the box held puts its rule back to ALWAYS; a full box held and let
  go with nothing changed empties.
- **Lesson 18:** the unlock step ends on FX pressed, locked or not. The four effects can no longer go by unseen
  (the engines, below).
  Later the same evening, at their word, the lesson teaches the screen that FX brings up: a card says it is a map of
  the keys (big names the white keys, four in a row; small names the black keys), the effect strips name what the
  map says (`[FX] AND [3]: 1/16. / A STUTTER.`), and one step asks the learner to find CRUSH alone. 16 steps.
  Asked on the way: SLOOP has no help page on the FM-1; the screens that held buttons bring up are its only guide.
- **Lesson 23:** the copy is a step of its own (`HOLD [SAVE], HOLD [2]. / LET GO WHEN THE TOP / SAYS COPY.`, hint `A
  SHORT TAP ONLY GOES TO PAGE 2. KEEP BOTH DOWN.`), then `YOU ARE ON PAGE 2. CHANGE IT.` The card of moves says
  `GO TO A PAGE: [SAVE] + A KEY.` 13 steps.
- **Lesson 30:** `A SONG: 3 PAGES IN A ROW, OR MORE.` in place of `A SONG OF 3 LINES OR MORE.`
- **The "not yet" hints begin with `NOT YET.`** (lessons 4, 5, 7, 10 and 15; lesson 23's went with its step).

**The engines** (the lesson program, and `fw/course.inc` for the FM-1 alone; the same rule in both).
- **A fill or an effect held from the step before no longer ends the next step.** A step whose Until is true
  already is still passed over, unless a fill or an effect is on at that moment: then the learner has to let go
  first, and do it again. Lesson 18's four effects and lesson 17's two fills went by unseen this way.
  `tutorial/device.py --sticky` plays with a robot that is slow to let go, and fails a lesson where that happens.
- **A "not yet" stays up six seconds** (two and a half before).
- **Both OCT buttons, held two seconds, bring back the start card** from SLOOP by itself (no card up). In a lesson
  on the FM-1 alone they still go to the lesson's first card.
- **The list of lessons shows the name large,** under a small line `< TURN [SELECT] >`. A large card's line
  that starts with `^` is drawn small. The lesson program's list does the same.
  - Seen on the real FM-1 the same evening: "on the lesson select list CHOOSE is in the larger font size I think
    it should be smaller. Is there another way to say that turn [select] to". The heading was `TURN [SELECT] TO
    CHOOSE.`, 264 pixels where a line holds 212, so `CHOOSE.` wrapped onto a line of its own, and a line that
    does not start with `^` is large. The list was never photographed by the robot, so it had not shown.
  - The heading is one short line now. What fits: `< TURN [SELECT] >` (179), `PICK WITH [SELECT]` (192), `TURN
    [SELECT]` (147). `TURN [SELECT] TO PICK` (224) does not. Photographed on the simulated FM-1 for lessons 1, 6
    and 26 (`tutorial/lesson-list.png`). A firmware change: it reaches the FM-1 with the next install.
- **A framed name is never cut in two** by the end of a line (`[KEY 12]` has a space in it). The checker wraps the
  same way.
- **The checker refuses** an Until on a card with two answers, and a card that asks for a move with no answer.
- **SHOW ME and the lock** (lesson 18's note): the computer's own presses do lock the layer (tried on the simulated
  FM-1: after them, FX says LAYER FREE). The log's "does not see: says LOCK" was only SHOW ME not reading the
  message while it moved. Nothing to change in the firmware.
- **Room:** 820 bytes free (1,040 before this round).

**The installer page** shows the logo: by the name and as the tab's icon (in place of SLOOP's sailboat), and above
the title. `tools/mkpagelogo.py` cuts the logo out of its paper once (`assets/logo/teachie-beachie-page.png`,
`-mark.png`); `tools/teachie_page.py` puts the two pictures in. The credit to SLOOP is as it was.

**The installer page's words** ("The copy for the install page does not match what Teachie Beachie is. We need to
update that"). The page was SLOOP's with the name changed, so it described a groovebox and things Teachie Beachie
does not do. Changed in `tools/teachie_page.py`, in English and French, first drafts:
- what it is, in their own voice ("I want it to say that I built this tutorial for my children. The intent is for
  them to learn about sequencers and groove box and be able to make their own songs on an FM-1 (which will be a
  Christmas gift)"): `I built TEACHIE BEACHIE for my children, so they can learn what a sequencer and a groovebox
  are and make their own songs on an FM-1, their Christmas gift. It teaches the M-VAVE FM-1 on the FM-1 itself:
  thirty short lessons, from a first beat to a whole song. It is built on SLOOP 3, a complete groovebox, which
  stays when the lessons are done.` No names and no ages on the page. The page's description for search engines
  stays plain (`free firmware that teaches the M-VAVE FM-1 in thirty lessons, built on SLOOP 3`).
- the picture's caption: `The M-VAVE FM-1, with SLOOP 3 on its screen (3D render)` (the render is SLOOP's).
- after the install: the peach logo and the READY FOR A LESSON card, not the sailboat; OCT+ and OCT-; and that the
  thirty lessons are a pack of their own which the page does not install yet.
- "Coming from SLOOP 2.8?": Teachie Beachie does not open the SLOOP 2.8 project (that part of SLOOP 3 is left out
  for room); the presets, favorites, samples, kits, bank and settings still come along.
Still SLOOP's: the Guide and Web editor links, the steps of the install itself, the picture.

**Left as they are, on purpose or for later.**
- Lesson 4's note about the orange notes ("maybe we should keep it the same"), the glow on HIGH, when `STUCK?`
  comes up, the tag's corner.
- Lesson 17: marking box 12 also marks the hat that is in it (SLOOP's rule; a setup cannot take one note out yet).
- The same slip on the bass track (a key played with SEQ not held changes the note that boxes write).
- The FM-1 alone does not remember the next lesson after it is switched off (the list of lessons is the way).
  Writing that to flash is not something to add two days before it has to work.
- A way to rejoin a lesson without losing what is on the FM-1.

**Checked the same evening, on this round's build.**
- The build, `make test` and `make twin` (SLOOP's own scenarios) pass.
- The robot replay: all thirty lessons, five ways, 150 runs, none stopped. With the computer leading and a clumsy
  robot; with the FM-1 leading itself; the same with the clumsy robot; the same with the robot that is slow to let
  go; and with the computer leading once more, for the pictures in `lessons/screens/`.
- Looked at afterwards: the pictures of lessons 1, 3, 10, 17, 23 and 30, and the installer page.
- **On the real FM-1 the same evening:** the tester installed this round's firmware from the installer page, and
  the lesson pack was put on over USB (`tutorial/device.py --install`): all four sample slots were empty, the pack
  went to slot 4 in 2 seconds, and the FM-1 finds 30 lessons. Next: the lessons played on the FM-1 alone, with no
  cable. The lesson program on the computer is stopped for that.
- **Asked on the way: "how will other users get the lessons on their devices?"** Today they cannot: the pack goes
  on only with that command. Proposed, not built: a last step on the installer page that sends the pack from the
  browser once the FM-1 has restarted (the page says today that it does not install the lessons yet).

## 2026-11-20: the look of the cards

"I want it to look good. Maybe a little stylish, but it needs to be super functional, simple." Asked after the
question of lowercase came up (the fonts in the firmware have capitals only: 77 characters each, 4.3 KB, 5.0 KB
and 6.4 KB; a font for the lessons alone could sit in the lesson pack, which has over 40 KB to spare).

- **Chosen: "Modern", with outlined button names.** Sentences in small letters, in an open, even sans-serif;
  button and key names stay in capitals inside the thin frame they have today.
- **How it was chosen:** three sheets of mock-ups, the same card, a full seven-line card and a strip, drawn at the
  FM-1's size (`tutorial/card-looks.png`, `card-looks-modern.png`, `card-looks-stylish.png`).
  - Tried and set aside: today's capitals with filled buttons; Clean (the family the FM-1 uses now); Round;
    Modern bolder; Geometric (its a looks like an o); Technical; Soft; Retro; Typewriter; Tech.
  - Filled buttons (a solid dark-red tab) were in every first mock-up; they were not convinced, and side by side the
    outlined ones are calmer on a card with two or three names. Outlined it is.
- **The mock-ups use typefaces that come with the Mac** (Modern is Avenir Next Demi Bold). Those cannot go into the
  project. The real one has to be a free, open typeface that looks the same; none is on this Mac yet, and
  SLOOP's own Inter is not here either (its tool points at a Linux path).
- **Two open typefaces downloaded at their word** ("can you download Google Sans and Lato"), from Google Fonts'
  repository on GitHub (`github.com/google/fonts`, folder `ofl`), each with its license, into `assets/fonts/`:
  Google Sans (one file with every weight, 5 MB) and Lato in Medium, SemiBold and Bold (about 650 KB each). Not
  committed yet.
  - "I'm allowed to use these fonts on an FM-1?" Both are under the SIL Open Font License 1.1, as SLOOP's Inter
    is: they may be built into a product and passed on, the license text goes with the project, the font is
    not sold by itself. Lato's name is reserved for the unchanged font. Google Sans has no reserved name, but
    "Google" and "Google Sans" are trademarks: not in the project's name, and nothing that suggests Google is
    behind it (`assets/fonts/google-sans/TRADEMARKS.md`). A reading of the license files, not legal advice.
  - Drawn next to Modern with outlined buttons: `tutorial/card-looks-fonts.png` (Google Sans Medium and SemiBold,
    Lato SemiBold and Bold). Google Sans SemiBold is the closest to Modern.
- **Chosen: Google Sans Medium** ("Google sans medium"), over SemiBold, which had been the suggestion.
- **Built the same night.**
  - **The font.** `tools/mklessonfont.py` renders Google Sans Medium the way SLOOP's `mkfont.py` renders Inter: a
    small size (18 pixels, capitals 13 high, as SLOOP's small font) and a large one (34, capitals 25), each with
    all of ASCII, so with small letters. 4.8 KB and 9.5 KB, kept as `assets/fonts/lessons-font.bin`.
  - **In the pack.** The two fonts follow the lessons and the clips (`tools/mkcourse.py`); the pack is `TPC2` now
    and 51 KB (a slot holds 80; the pack's own limit is 64). A pack and a firmware must be from the same build.
  - **In the firmware.** When the pack is found, the guide draws cards and strips with its fonts (`course_find`
    in `fw/course.inc`, `GF_S` and `GF_L` in `fw/main.c`); the pack is looked for at power-on, so the start card
    has them too. In `app/gfx.c` a font with more than 77 glyphs is read as all of ASCII. With no pack the
    guide uses SLOOP's fonts and puts the words in capitals first, as before. 308 bytes; 516 are free.
    The build's own check refuses a pointer into the flash window kept in ready-made data, so which fonts
    to use is a flag, not a pointer.
  - **The words.** A script turned the 691 texts into sentences; every line was read afterwards and about sixty
    were set by hand. The rules that came out of it are in `lessons/README.md`: capitals only for a name in a
    frame, a word quoted from SLOOP's screen (`FILT says OFF`, `the SONG page`, `the top says COPY`) and names
    (SLOOP, FM-1, A minor); small letters after a colon; `Knob 1` to `Knob 4`. `FASTER / FASTER / FASTER` in
    lesson 1 stays in capitals. The start cards, the list of lessons, `Stuck?`, `Not yet.`, `Lesson 6 / Done`
    and the other built-in words follow, in the firmware and in the lesson program.
  - **The tools.** The checker measures with the lessons' font and allows small letters. The simulated FM-1s of
    the robot runs start with the pack on them, so the pictures show the new look.
- **How it really draws, and SemiBold after all.** On the simulated FM-1 the letters come out a little thinner than
  in the mock-ups. `tutorial/card-looks-real.png` shows Medium and SemiBold as the FM-1 draws them; seeing that:
  "lets go with semibold". The font file is made with weight 600 now (the tool's default). A weight is a change to
  the font file and the lesson pack only, not to the firmware. Where this section says Medium for what was
  built, read SemiBold.

- **Checked the same night, with SemiBold.** The build, `make test` and `make twin` pass. The robot replay again:
  all thirty lessons, five ways, 150 runs, none stopped (computer-led with the clumsy robot; the FM-1 leading
  itself, plain, clumsy and slow to let go; computer-led once more for the pictures). Looked at: the pictures of
  lessons 1, 17, 18 and 26, the start card, the list of lessons, and a card on a simulated FM-1 with no pack
  (capitals, SLOOP's font, as before). Not yet on a real FM-1: it needs the firmware and the lesson pack again,
  both from this build.

- **2026-11-21: on the real FM-1, and the start card.** The firmware of the night before went on the tester's
  FM-1, then the pack (`device.py --install`: slot 4, 30 lessons found). First word back: the start card. Asked
  for: the name larger, `Ready for a lesson` in the regular size, and better words ("Start the lessons or just
  play in Sloop"). Built: with the pack on the FM-1 the card is a large one, `Teachie / Beachie` in the large
  font, then two small lines, `Ready for a lesson?` and `Or just play in SLOOP.`; `Starting...` the same way.
  A small line under a large one gets 6 px of air. With no pack (SLOOP's wide capitals) those small lines do not
  fit one line, so there the card stays a plain one with the same words. `The computer did not answer` stays
  plain too. Picture: `tutorial/home-card.png`. Checked: the build (277,896 bytes, 344 to spare), `make test`,
  and on the simulated FM-1 the card with the pack, START through to the list of lessons, and the three cards
  with no pack. Not replayed ("don't test everything ... I'm sure there will be more edits"). The lesson pack is
  unchanged, so only the firmware has to go on the FM-1 again.
- **The same morning: a lesson's name.** "That looks much better", then: on the lesson cards `Lesson 1` in the
  smaller size and `Hello` in the larger. Built in the firmware and in the lesson program: `Lesson N` is a small
  line (`^`) on a lesson's first card, on the list of lessons (under `< Turn [SELECT] >`) and on `Lesson N / Done`.
  Picture: `tutorial/lesson-list.png`. Checked: the build, `make test`, the lint of the thirty lessons, and the
  cards on the simulated FM-1 (lessons 1, 8, 26 and 29 on the list).
- **Waiting, to be made all at once** ("can you just take notes and we will make the changes all at once"):
  1. **An underline.** "Yes I like the underline": the small `Lesson N` line, on a lesson's first card, the list
     and `Lesson N / Done`, underlined as in the mock-up. The cards cannot underline yet: it needs a mark of its
     own (my plan: a line starting with `_` is drawn small and underlined, on any card; `^` stays small without
     a line), in the firmware's wrap and draw, and in `check.py`'s measuring.
  2. **A heading on lesson 1's buttons card (step 3), underlined.** The tester's word was `Conventions`; they asked
     for suggestions. Not chosen yet. All fit one small line (212 px): `How to read the cards` (186), `Reading the
     cards` (151), `How the cards work` (169), `Buttons and keys` (146), `Good to know` (121), `Conventions`
     (108). The card has 6 lines and an answer; with the heading 7, its limit.
  3. **"Border", not "frame".** Step 3 to say `A name with a border / is a button: [OCT+]`; then, with the knobs
     in borders (item 39), "is a button or a knob". `a button or a knob: [OCT+]` is 224 px, too wide, so: `A name
     with a border is / a button or knob: [OCT+]` (197, 210). Still 7 lines with the heading. Step 4 the same way,
     asked too ("please change that to A number with a border is a white key"): `A number with a border / is a
     white key.` (199 px, fits). My suggestion, not asked: the word "frame" in `lessons/README.md` where it means
     this border.
  4. **Lesson 1, step 6: what SLOOP is.** Today: `We make our songs in SLOOP. / SLOOP has 5 tracks with different
     instruments.` Asked: say SLOOP is a groovebox, with how many instruments and samples it has. And for all the
     words from now on: "With this new font we have more space so I think we should improve our copy so it is
     more informative--but more is not always better." My draft (6 lines, the widest 211 of 212 px):
     `SLOOP is a groovebox: / a whole band in one box. / We make our songs in it. / 153 sounds, 37 drum kits. /
     Piano, strings and horns / are real recordings.` The facts: 153 sounds (`docs/sloop/facts.json`); 37 drum
     kits ready to play, ACOUSTIC to JAZZ (SLOOP's own count, 46, adds the 9 slots for kits of your own); the
     factory samples are a piano, vibes, a bass, horns, strings, a flute, a scratch and percussion
     (`assets/CREDITS-SAMPLES.txt`). Asked later ("aren't some of the drums real recordings too?"): yes, PERC is
     VCSL's recorded acoustic drum kit, and 5 of the 37 kits play it (ACOUSTIC, and DEEP, TIGHT, BRIGHT, DUST: the
     same kit treated, `engine/drum.h` `GB_KIT_SAMPLED`); the rest are synthesized. The last sentence then, 3
     lines, the card 7 (its limit): `Piano, strings, horns / and acoustic drums / are real recordings.` The 5 tracks
     leave this card: the strip two steps later says `Five tracks. /
     One song.` Lesson 5 (step 18) says what a groovebox is in full, with the sequencer, and lesson 9 (step 2)
     says 153 again; both stay as they are. Then, on 2026-11-22: "I don't think 'Piano, strings and horns are real
     recordings.' is the most important thing", and "just say what it is don't use weird vague stuff, say the
     sequencer and what you can do, lets call out tracks and not use band in a box". Chosen ("yeah that is good"),
     their two lines in it, 6 lines, the widest 211: `SLOOP is a groovebox: / 5 tracks, a sequencer, / synths,
     effects, a mixer. / 153 sounds, 37 drum kits. / Write beats, record live, / add effects, mix, save.`
  5. **Lesson 1, step 7: one line, a smaller strip.** Asked: "This is SLOOP / Press [PLAY]" on one line, so the
     strip covers less of the screen (one line: 35 px high instead of 58). As written it does not fit: `This is
     SLOOP. Press [PLAY].` is 233 px of 212. Wordings that fit: `SLOOP! Press [PLAY].` (178, my pick: it still
     names what they see), `Now press [PLAY].` (151), `This is SLOOP: [PLAY]!` (183). Not chosen yet. My
     suggestion, not asked: steps 8 and 9 on one line too, `5 tracks, 1 song. Listen.` (195) and `Press [PLAY]
     to stop.` (172).
  6. **Lesson 1, the melody game (steps 11 and 13).** Today: `The melody game / Listen. / Then play it back.`
     Asked: an underlined heading `Repeat after me`, then "Listen. then play it back, the blinking keys will help
     you know what buttons to press." My draft (4 lines): `_Repeat after me / Listen. Then play it back. / The
     blinking key shows / which key to press next.` Two words changed from theirs, to say so when it is made:
     "keys", not "buttons" (lesson 1's card 3 and 4 just taught that buttons and keys are different), and one
     key, the next one, because during an Echo the FM-1 blinks only the next key to play (`course.inc`,
     `PH_ECHO`); while it plays the phrase itself it lights each key in turn. My suggestions, not asked: step 13,
     the second round, could be just `_Repeat after me / Listen. Then play it back.`; and step 15's `One more` and
     step 16's `Your turn` underlined the same way, so all the game's cards look alike. All of item 6 accepted
     ("Yeah I like your changes").
  7. **Lesson 1, the end of the game.** Step 17 (big): `Faster / Faster / FASTER`, the last one in capitals
     (asked; the three lines fit, 103 to 123 px). Step 20, in the tester's words: `Ha, sorry, I'm just messing with you. /
     That was way too fast. / Nearly impossible to play by hand. / But with a sequencer you can.` It fits exactly:
     7 lines, the most a card with an answer holds. Cut at the breaks I would choose, the words unchanged:
     `Ha, sorry, I'm just / messing with you. / That was way too fast. / Nearly impossible / to play by hand. /
     But with a sequencer / you can.` Then the last sentence changed: "But a sequencer can." The card as it is
     to be made, 6 lines: `Ha, sorry, I'm just / messing with you. / That was way too fast. / Nearly impossible /
     to play by hand. / But a sequencer can.` (`lessons/README.md` says `FASTER / FASTER / FASTER` stays in capitals:
     that line changes with it.)
  8. **Lesson 2, step 2** (beginners only). Today: `In lesson 1 the FM-1 played a run too fast for hands. / A
     sequencer did that. / Today you get one.` Asked: "In lesson 1 the FM-1 played a sequence too fast for hands. A
     sequencer played it. Let's learn how to use a sequencer." Their line `played a sequence too fast for` is too wide
     (it wraps), so cut at my breaks, the tester's words unchanged, 6 lines: `In lesson 1 the FM-1 / played a sequence / too
     fast for hands. / A sequencer played it. / Let's learn how / to use a sequencer.`
  9. **Lesson 2, step 12.** Asked: `Kick on 1, 5, 9, 13. / They call this four on the floor. / This is the beat
     of house music.` Two of those lines are too wide and would leave `floor.` and `music.` alone, so cut at my
     breaks, the tester's words unchanged, 5 lines: `Kick on 1, 5, 9, 13. / They call this / four on the floor. / This is
     the beat / of house music.`
  10. **Lesson 2, step 13 (the free strip).** The tester's draft: `Add kicks, or tap a full / box again to remove / [OCT+]
      when done.`, and they asked for another word than "full". Not chosen yet. My suggestions: no adjective at all,
      `Tap a box to add a kick. / Tap it again to remove it. / [OCT+] when done.` (3 lines, 206 px at most; my
      pick); or "filled" (`a filled box`); or "a kick" (`tap a kick again to remove it`). Not "lit": step 3 says
      a light walks along the boxes. Step 3 says "a full box" too, so the same word goes there. (Lesson 3's tester
      note, 2026-11-20: they did not know a box could just be pressed again to empty it.) Chosen: my pick, no
      adjective ("yes that is much better"). Step 3 without "full", chosen too ("yes that is much better"), 6
      lines: `A sequencer is / a row of boxes. / A light walks along them. / When it steps on a box / with a sound
      in it, / the sound plays.`
  11. **Lesson 2, step 17 (its last card).** Today: `A sequencer: boxes, and a light that walks. / You just
      programmed one.` Chosen, my draft ("lesson 2. Yes I like your changes"), 5 lines: `That's a sequencer: /
      boxes, and a light / that walks along them. / You just programmed / your first beat.`
  12. **Lesson 3, step 2.** Today: `Last time: four kicks. / A beat has three layers: kick, snare and hat. / Today:
      the other two.` Asked: "Last lesson had four kicks. / A beat has three layers: / kick, snare and hat. / Now
      lets take a look at the other two." `Last lesson had four kicks.` is too wide for one line, so cut at my
      breaks, the tester's words unchanged (with the apostrophe in "let's"), 6 lines: `Last lesson had / four kicks. / A
      beat has three layers: / kick, snare and hat. / Now let's take a look / at the other two.`
      Step 3 stays as it is (`Press [PLAY]. Your four kicks.` is too wide for one line; "lets just keep it as is").
  13. **Lesson 3, step 4.** Today: `Playing a drum's key / picks that drum. / The boxes then write it. / So first
      pick the snare.` The tester does not like it; their gist: "Each key represents a different kind of drum. press a
      drum's key to select that drum. The selected drum can then be put into the boxes. Pick the snare." My draft,
      4 lines: `Each key is a drum. / Play one to pick it. / The boxes you fill next / get the drum you picked.`
      "Pick the snare" left to step 5, which says `Pick the snare: / play [KEY 3].` Their answer, the card to make
      (their words, my breaks; 6 lines): `Each key is / a different drum. / Play one to pick it. / The selected drum /
      can then be put / into the boxes.`
  14. **Lesson 3, step 8** (beginners only). The tester's question: "is there supposed to be a kick on every beat? or is
      that referencing the first beat?" The card is right (kicks on boxes 1, 5, 9, 13 are beats 1 to 4; snares
      on 5 and 13 are beats 2 and 4), but no lesson says a beat is 4 boxes, and "beat" also means the whole
      drum part (step 2 `A beat has three layers`, step 19 `That is a drum beat`). My draft, 6 lines, chosen ("Ok
      that looks good"): `Every 4 boxes is a beat. / Kick on every beat. / Snare on beats 2 and 4: / the
      backbeat. / Most songs you know / have one.`
  15. **Lesson 3, step 12.** Today: `The grid has a row for each drum. / The keys light up only for the drum you
      picked last.` The tester does not love the second sentence; their idea: "The keys for the selected drum will light
      up." What SLOOP does (`fw/main.c`, `leds()`): with SEQ held, a key is lit where the selected drum plays in
      that box (the playhead flips it; keys 1, 5, 9, 13 glow faintly). Their line could read as "the drum's own key
      lights". My draft, 6 lines: `The screen shows / every drum. / The keys show only / the selected drum: / a key
      lights up / where it plays.` Not wanted ("I don't like your edit"). The card to make, their words, 4 lines:
      `The grid has a row / for each drum. / The keys for the selected / drum will light up.` (The tester thought SEQ is
      not held at this point; on the drum grid the keys show the selected drum's boxes whether SEQ is held or
      not: `steps_on_keys() || sticky == PG_STEPS`.)
  16. **Lesson 3, step 19 (its last card).** Today: `Three layers: kick, snare, hat. / That is a drum beat.` Chosen
      from three drafts ("lets go with the first"), 4 lines: `Kick, snare, hat. / Three layers, / one drum beat. /
      And you made it.`
  17. **Lesson 4, step 2.** Today: `The drum track has 16 sounds. / One on each white key. / Together they are a
      kit.` Asked, 4 lines: `The drum track has 16 / different drum sounds. / One on each white key. / Together they
      are a kit.`
  18. **Lesson 4, step 7.** Today: `SLOOP has many kits. / Each one puts 16 new sounds on the same keys. / Your beat
      stays the same.` Asked: "many" becomes a number ("more than #"). 37 is the count of ready kits, as in lesson
      1's SLOOP card. The words otherwise unchanged, at my breaks, 6 lines (offered): `SLOOP has 37 drum kits. /
      Each one puts / 16 new sounds / on the same keys. / Your beat / stays the same.` Shorter, offered too, 4
      lines: `SLOOP has 37 drum kits. / Each one puts 16 new / sounds on the keys. / Your beat stays as it is.`
      Their answer, the card to make, 5 lines: `SLOOP has 37 drum kits. / Each one puts 16 new / sounds on the keys. /
      Your programmed beat / stays the same.`
  19. **Lesson 4, steps 8, 9 and 11: "drum kit", not "kit".** Asked. `Turn [PRESETS] one click. / A new drum kit.`,
      `Try more drum kits. / [OCT+] on one you like.`, `The whole band, / with your drum kit.` All fit as they
      are.
  20. **Lesson 4, step 13 (its last card).** Today: `16 sounds on 16 white keys. / [PRESETS] changes the kit.` The
      tester wants it redone: 37 drum kits, 16 sounds on the white keys, PRESETS changes the drum kit. Three drafts offered,
      5 lines each; my pick, chosen ("yeah lets go with the first"): `A drum kit: 16 sounds, / one on each white key. / SLOOP has 37 of them. / [PRESETS]
      changes / the drum kit.` The others: `37 drum kits, / 16 sounds each, / one on each white key. / [PRESETS]
      changes / the drum kit.` and `37 drum kits. / 16 sounds in each, / one per white key. / Turn [PRESETS] / to
      change the drum kit.`
  21. **Lesson 5:** shown, nothing asked at first ("Lets move on"); my note for later: step 18's groovebox card
      could build on lesson 1's new SLOOP card instead of saying it again.
  22. **Lesson 5, step 2.** Today: `Five tracks. / Five instruments. / Today you meet each one, by taking it away.`
      Asked, then "one at a time" moved to its own line, 5 lines: `Five tracks. / Five instruments. / Now let's
      focus on / each track, / one at a time.`
  23. **Lesson 5, step 9.** Today: `Only bass and drums now. / They are the rhythm section. Every band leans on
      them.` The tester did not like "Every band leans on them". Three endings offered; chosen, 5 lines: `You can now
      hear only / the bass and drums. / They are the rhythm / section. They keep the / beat for the others.`
  24. **Lesson 5, step 11: light keys 6 to 10 while the card names them.** Asked; the words stay (`Solo is the
      opposite of mute: only that track plays. / [6] to [10] are the five solos.`). Lit steady and faint, not
      blinking, since a blink means "press this". The lesson format has it already: `- **Lights:** box 6 7 8 9 10`
      (README, "Keys lit faintly, as part of the picture"; no lesson uses it yet). The computer's lesson program
      draws it (`tp/lesson.py`, level 2) and `tools/mkcourse.py` puts it in the pack (`LIGHTS`), but the FM-1's own
      player never reads it: `CT_LIGHTS` is in `app/tp_course.h` and nowhere in `fw/course.inc`. To make: in
      `course.inc`'s `PH_ANSWER`/`PH_DO` case, next to the Follow light, `G.led[14 + key] = 2` for each key of
      `c_field(CT_LIGHTS, ...)`. A few lines; the flash has 344 bytes left. Check it on a card and on a strip.
  25. **Lesson 6, step 2.** Today: `Tempo is how fast the light walks. / It is counted in beats per minute: BPM.`
      Asked, then a last line added on my offer ("Yes, add it"), to tie BPM to lesson 3's "Every 4 boxes is a beat".
      7 lines, the card's limit, all fit: `Tempo is the speed / of the song. It's / how fast the light / walks in the
      sequencer. / It is counted in beats / per minute: BPM. / A beat is 4 boxes.`
  26. **Lesson 6, step 8.** Today: `Now the oops. / Make a mess on purpose. / Then take it back.` Asked, 4 lines,
      all fit: `Now the oops. / We will make a mess / on purpose. / Then take it back.`
  27. **Lesson 6, step 12.** Today: `Undo takes back your last move. / It is [EDIT] and [OCT-] together.` Asked, 4
      lines, all fit: `Undo takes back your / last move. To undo / hold [EDIT] and press / [OCT-] together.`
  28. **Lesson 6, step 16.** Today: `Undo remembers your last 8 moves. / Went too far? [EDIT] and [OCT+] is redo.`
      Asked, 5 lines, all fit: `Undo will remember / your last 8 moves. / Went too far? / To redo hold [EDIT] /
      and press [OCT+].`
  29. **Lesson 6, step 21 (its last card).** Today: `Tempo: [SELECT]. / Undo: [EDIT] and [OCT-]. / Save: [SAVE]
      twice. / Now nothing can go wrong.` The tester's first version, with "Hold ... press" and a redo line, was 8
      lines, one over. Chosen, 6 lines, all fit: `Tempo: [SELECT]. / Undo: [EDIT] + [OCT-]. / Redo: [EDIT] +
      [OCT+]. / Save: [SAVE] twice. / Now nothing can / go wrong.`
  30. **Lesson 7, step 3.** Today: `Pick a drum: play its key. / [SEQ] + a key: a box fills. Again: it empties. /
      [PLAY]: hear it. / [EDIT] + [OCT-]: undo.` Asked, 5 lines, all fit (the first is 205 px): `Pick a drum: play
      its key. / [SEQ] + a key: a box fills. / Press again: it empties. / [PLAY]: hear it. / [EDIT] + [OCT-]:
      undo.`
  31. **Lesson 7, step 10 (its last card).** Today: `Part 1 is done. / You can make a beat. / Next: the rest of the
      band.` Asked, 5 lines, all fit; "band." would sit alone on the last line, so the break is mine, and "Now you
      can" on my offer: `Part 1 is done. / Now you can make / a beat on your own. / Next: the rest / of the band.`
  32. **Lesson 8, step 2.** Today: `Drums have boxes. So do the other tracks. / But their boxes hold notes.` Asked,
      5 lines, all fit; "notes." would sit alone, so the last break is mine: `In the sequencer, / drums have boxes. /
      So do the other tracks. / But their boxes / hold notes.`
  33. **Lesson 8, step 11.** Today: `Let go of [SEQ]. Play the higher A.` Asked: name the key. Checked: the white
      keys run F3 (1) to G5 (16), so the A of step 5 (A3) is key 3 and the higher A (A4, the step's `tap A4`) is
      key 10. Strip, 2 lines, both fit: `Let go of [SEQ]. Play the / higher A: [KEY 10].`
  34. **Lesson 8, step 14.** Today: `Make it your own. / Play a key to choose a note. / Hold [SEQ], tap a box: it
      goes in. / Tap again: it goes out.` Asked: "it is added", "it is removed". Chosen, 5 lines, all fit, the note
      line matching lesson 7 step 3's `Pick a drum: play its key.`: `Make it your own. / Pick a note: play its key.
      / Hold [SEQ], tap a box: / it is added. / Tap again: it is removed.`
  35. **Where the review stopped (2026-11-21):** lessons 1 to 8 gone through ("Done with Lesson 8"); lesson 8 step
      19 left as it is. Next time: lesson 9, then on to 30. Nothing above is made yet. (Later the same day: lesson 9
      begun, below.)
  36. **Lesson 9, step 2.** Today: `Every track can play any instrument. / SLOOP has 153 of them.` Asked, 4 lines,
      all fit; "instrument." would sit alone, so the first break is mine: `Every track can play / any instrument. /
      SLOOP has 153 / different instruments.`
  37. **Lesson 9, step 13.** Today: `Favorites live in the first family: FAVS. / Your best sounds, one tap away.`
      Asked: `Your best sounds, are just one tap away.` The comma dropped, as the sentence now has its verb; the
      first break mine, so "the first family" stays together. 4 lines, all fit: `Favorites live in / the first
      family: FAVS. / Your best sounds are / just one tap away.`
  38. **Lesson 9, step 15:** `The tune, in your sound.` The tester asked what it meant (the song plays 4 bars with
      the lead in the sound just picked). Two clearer wordings offered; kept as it is ("lets just keep it").
  39. **Knobs get a border: `[KNOB1]` to `[KNOB4]`.** Asked ("I think it should be [KNOB4]"), for lesson 9 steps
      10, 12 and 17. Written as the FM-1's own labels have it, with no space (the tester: "there is no space so
      knob 1 is [knob1]"); a key, with no label on the FM-1, keeps its space: `[KEY 10]`. Until now no lesson
      framed the four knobs (`Knob 4`, plain), while `[SELECT]`, `[PRESETS]` and `[ALGORITHM]` had borders. Then
      for every card ("can you put borders on all of the knobs and measure that they fit"): `Knob N` becomes
      `[KNOBN]` in the words of every card, strip and hint (not in Do, Until or Robot, which say `KNOB 1` for the
      lesson program). Measured on today's words (scratch script `knobs.py`): 56 texts in lessons 9 to 28, none
      over its limit. Five grow a line:
      - Lesson 9 step 17: rewritten anyway (item 42).
      - Lesson 21 step 5 (card): 5 lines to 6, of 7. Fits; `hear it.` ends alone, for its own review.
      - Three strips at the top go from one line to two, a word left alone. My one-line fixes, to make with the
        borders: lesson 19 step 8 `Knob 4 down, below 30.` → `[KNOB4] down past 30.` (198 px); lesson 19 step
        11 `Knob 1: the echo's time.` → `[KNOB1]: the echo time.` (200); lesson 28 step 4 `Knob 1, SWING: up to
        50.` → `[KNOB1], SWING: to 50.` (199).
      When the review reaches lessons 10 to 28, their new words get the border too, and are measured with it.
      Lesson 1 step 3 says it: item 3.
  40. **Lesson 9, step 10.** Today: `Knob 1 changes the family. Turn it.` With the border, 2 lines, both fit:
      `[KNOB1] changes the / family. Turn it.`
  41. **Lesson 9, step 12.** Today: `Knob 4 right: / now a favorite.` With the border, 2 lines, both fit: `[KNOB4]
      right: / now a favorite.`
  42. **Lesson 9, step 17 (its last card).** Today: `[PRESETS] steps through sounds. / The SOUNDS page shows a
      family. / Knob 4 keeps a favorite.` Asked: directions to the SOUNDS page (it is the last of [EDIT]'s six
      pages on a melody track, so up to 6 taps from wherever the learner was). Their words, fitted: "to get to
      the" became "for the" (too wide even alone), and the knob line takes two with its border. 7 lines, the
      card's limit: `[PRESETS] steps through / different sounds. / Tap [EDIT] several times / for the SOUNDS page.
      / It shows a whole family. / [KNOB4] keeps / a favorite.`
  43. **Where the review stopped (2026-11-21, evening):** lessons 1 to 9 gone through. Next time: lesson 10, "Record
      it live", then on to 30. Nothing above is made yet.
  44. **Lesson 10, step 2.** Today: `Boxes are one way to write music. / The other way: just play, and the FM-1 writes
      it down.` Asked, then "just" put back before "play", 5 lines, all fit: `Boxes are one way to / write music. /
      The other way is to / just play, and the FM-1 / writes it down.`
  45. **Lesson 10, step 10.** Today: `Not happy with a take? / Undo takes the whole take back.` The second sentence
      redone ("we need to improve this"). Chosen, 3 lines, all fit: `Not happy with a take? / One undo removes / the
      whole take.`
  46. **Lesson 10, step 19 (its last card).** Today: `[REC] writes down what you play. / Undo takes a take back. /
      [REC] held: the track is erased.` Redone as a cheat sheet like lesson 6's last card, with redo added (taught in
      step 12). The tester's words, 5 lines, all fit: `Record: [REC], then play. / Undo: [EDIT] + [OCT-]. / Redo:
      [EDIT] + [OCT+]. / Erase track: hold [REC] / for 2 seconds.`
  47. **Lesson 10: a new card on recording in a loop.** The tester: "I wonder if we should say how it records in a
      loop." What SLOOP does (docs/GUIDE.md, Recording): while REC is on, the bar keeps looping, and each pass adds the
      notes played (a note played again where it already is takes its place). The lesson never says it. In the
      words they liked best ("the bar keeps looping, and each time round, what you play is added on top ... builds
      up a tune bit by bit"), a card with `[OCT+] Next`, 6 lines, all fit: `While [REC] is on, / the bar keeps
      looping. / Each time round, what / you play is added on top. / That's how you build up / a tune, bit by bit.`
      Where: just before step 15 (`Record your own tune.`), the first free recording; my placement. The steps after
      it move up by one.
  48. **Lesson 10 done** ("thats good"). On to lesson 11.
  49. **Lesson 11, step 9: name the A.** Today: `Hold [SEL] and tap an A.` The step's Do is `tap A4`, which is white
      key 10 (lesson 8's count). Asked (my note: "it could name it like lesson 8 does"). Strip, 2 lines, both fit:
      `Hold [SEL] and tap / the A: [KEY 10].` (Its Until stays `says KEY A`: any A still counts.)
  50. **Lesson 11, step 13: "note", not "key".** Today: `MIN is the minor scale. / SNAP pulls every key onto it. /
      The black keys can't go wrong now.` "Key" means the song's key in this lesson (step 2) and the keys pressed;
      "pulls every key onto it" mixes them. Asked: "every note" (5 lines: `MIN is the minor scale. / SNAP pulls
      every / note onto it. / The black keys / can't go wrong now.`). Then, once SNAP was explained (KNOB4 on the
      KEY page, KEYS: OFF / SNAP / WHITE; with SNAP a key outside the scale plays the nearest scale note below it,
      app/harmony.c), redone to say what it does. Chosen, 6 lines, all fit: `MIN is the minor scale. / With SNAP, a
      note that / doesn't fit moves down / to one that does. / The black keys can't / go wrong now.`
  51. **Lesson 11: line breaks only, the words kept.** So no word sits alone on a strip's last line: step 4 `Play
      only black keys / for a while.`; step 10 `Press [SEL]: / the KEY page.`; step 14 `Play the black / keys
      again.` (the tester kept "the": `Play the black keys again.` is 214 px, 2 over one line). Step 12 stays
      (`[KNOB4]: one click right. SNAP.`: SNAP alone is the word on the screen).
  52. **Lesson 11, step 20 (its last card).** Today: `A key is a family of notes. / [SEL] and a key sets it. / SNAP
      keeps you inside it.` "[SEL] and a key sets it" used a key (the keyboard's) to set a key (the song's). Chosen,
      the tester's last two lines, 6 lines, all fit: `A song's key is the family / of notes that fit it. / Hold
      [SEL] and tap a note / to set the key. / With SNAP on, every / note you play fits.`

- **2026-11-22: lessons 1 to 11 made** ("can you make the changes for lessons 1 to 11 now"), items 1 to 52 above.
  The open choices took my picks: lesson 1's heading `How to read the cards` (item 2), step 7 `SLOOP! Press [PLAY].`
  and steps 8 and 9 on one line, `5 tracks, 1 song. Listen.` and `Press [PLAY] to stop.` (item 5).
  - **The underline** (item 1): a line that starts with `_` is small and underlined, on any card (`fw/main.c`,
    `guide_wrap`, `guide_line`, `guide_draw`; `^` and `_` differ by one bit, so one test finds both). `Lesson N` is
    underlined on a lesson's first card, the list and the Done card, in the firmware and in the lesson program
    (`run.py`, `tp/lesson.py`). Lesson 1's headings use it: steps 3, 11, 13, 15 and 16.
  - **Lights** (item 24): the FM-1's own player now reads them (`fw/course.inc`, the PH_ANSWER/PH_DO tick): each key
    lit faintly (level 2), unless the step already lights it. Lesson 5 step 11 has `box 6 7 8 9 10`.
  - **The knobs in borders** (item 39): 57 in lessons 9 to 28, with the three one-line fixes.
  - **Lesson 10's new card** (item 47) is step 15; the steps after it moved up by one (20 steps now).
  - `lessons/README.md`: "border", not "frame"; the `_` and `^` marks; `[KNOB1]` to `[KNOB4]`; `Faster / Faster /
    FASTER`.
- **Waiting again, to be made all at once** (from looking at lessons 1 to 11 on the FM-1, 2026-11-22):
  53. **Lesson 2, step 13: say to hold [SEQ].** Today: `Tap a box to add a kick. / Tap it again to remove it. /
      [OCT+] when done.` The tester: "you can't just press you need to hold [seq] then tap a box. this is on a strip
      so there is limited space." Chosen, my pick ("Those look good"), 3 lines, all fit (207, 201, 163): `Hold [SEQ] and tap a box / to add or
      remove a kick. / [OCT+] when done.` The other: `Hold [SEQ], tap a box: / a kick. Tap again: gone. / [OCT+]
      when done.`
  54. **Lesson 3, step 17: the same.** Today: `Wrong box? Tap it again. / [OCT+] when done.` Chosen too, 3 lines, all
      fit (196, 134): `Wrong box? Hold [SEQ] / and tap it again. / [OCT+] when done.`
  55. **Lesson 3, step 8: the last sentence.** Today (item 14): `... / the backbeat. / Most songs you know / have one.`
      Asked: "Most songs you know have a backbeat." Four endings shown; chosen, theirs ("lets do option 1"), 6
      lines, all fit (the last two 185, 143): `Every 4 boxes is a beat. / Kick on every beat. / Snare on beats 2 and
      4: / the backbeat. / Most songs you know / have a backbeat.`
  56. **Lesson 12, step 2.** Today: `A chord is several notes played together, usually three. / SLOOP can play a
      whole chord from one key.` Asked: "SLOOP has a mode that can play a whole chord with one key" ("with", not
      "from": one key is what you play it with). The first sentence cut at my breaks, so `three.` is not left alone;
      6 lines, all fit (the widest 200): `A chord is several notes / played together, / usually three. / SLOOP has a
      mode that / can play a whole chord / with one key.`
  57. **Lesson 12, step 3: a line break only,** as lesson 11 step 10 (item 51): `Press [SEL]: / the KEY page.` (asked:
      "yes for step 3 fix it"). The rest of lesson 12 stays as it is.
  58. **Lesson 13, step 2.** Today: `One note in this tune is wrong. / You don't start over. / You fix that note.`
      (`wrong.` alone on its line). The tester's gist: "One note in this tune is wrong. But you don't want to start
      over, you just fix that note." Three shown; chosen, mine ("option 2"), 4 lines, all fit (the widest 183):
      `This tune has / one wrong note. / No need to start over. / Just fix that note.`
  59. **Lesson 13, step 5: the piano roll, with a heading.** Today: `The piano roll. / Every bar is a note. / Left to
      right is time. / Higher up is a higher note.` (`note.` alone; and "bar" is 16 boxes since lesson 2, while the
      roll draws each note as a short green line). Asked: the heading underlined and capitalized, `The Piano Roll`,
      and options for the rest. Chosen ("lets go with option 3"), 7 lines with the heading, the card's limit, all fit
      (the widest 207): `_The Piano Roll / It shows your notes / as green lines. / Left to right is time. / Higher
      up: a higher note. / The longer the line, / the longer the note.` (Capitals in a heading: a name, as the
      README allows.)
  60. **Lesson 13, step 6.** Today: `[KNOB1] walks along the steps. Its dial says where you are. / The wrong note is
      at step 7.` (`7.` alone). Asked: fix the breaks, and say it walks left and right. Chosen, my pick ("Option 1"),
      6 lines, all fit (the widest 191): `[KNOB1] walks left and / right along the steps. / Its dial says which /
      step you are on. / The wrong note is / at step 7.`
  61. **Lesson 13, step 10.** Today: `[KNOB2] changed which note it is. / [KNOB3] changes how long it lasts. / The
      last note is at step 9.` Asked to improve it. Chosen, my pick ("Option 2"): the knobs tied to the words the
      piano roll prints under them (STEP, NOTE, LEN, FORCE), and why step 9. 6 lines, all fit (the widest 199):
      `[KNOB2] is NOTE: / higher or lower. / [KNOB3] is LEN: / how long a note lasts. / Let's make the last note /
      longer. It's at step 9.`
  62. **Lesson 13, step 13.** Today: `One note does not belong at all. It is at step 13. / Go there, then press [EDIT] to
      delete it.` (`13.` alone). Asked, 5 lines, all fit (the widest 209): `One note sounds wrong, / it doesn't belong. /
      It is at step 13. / Go there, then press / [EDIT] to delete it.`
  63. **Lesson 13, step 12: name the length.** Today: `[KNOB3] right: longer.` Asked: "tell them the length we want".
      What the FM-1 does: the note at step 9 starts at LEN 2, a click adds a step (`fw/main.c`, the piano roll's
      KNOB 3), and the step's Until (`lead longest note above 4.5`) is met at LEN 5, three clicks. Chosen, my pick
      ("Option 1"), 2 lines, both fit (160, 135): `Turn [KNOB3] right / until LEN says 5.` With it, the Do becomes
      `turn KNOB 3 right 3` (today 4, which would leave LEN at 6 when the FM-1 does the step for the learner).
  64. **Lesson 13, step 20 (its last card): a cheat sheet,** as lessons 6 and 10, with the knobs named as the screen
      names them (item 61). Today: `[SEQ] twice: the piano roll. / [KNOB1] where. [KNOB2] which note. [KNOB3] how long. /
      [EDIT] deletes.` The tester's words; `(left/right)` after STEP made that line too wide, so taken out ("lets just
      remove the left/ right"). 6 lines, all fit: `Piano roll page: / [SEQ] twice. / STEP: [KNOB1]. / NOTE: [KNOB2]. /
      LEN: [KNOB3]. / Delete: [EDIT].`
  65. **Lesson 13, steps 3 and 4: line breaks only** ("Yes, note them"): `Press [PLAY]. / Hear the wrong one?` and
      `Press [SEQ] twice: / the piano roll.`
  66. **Lesson 14, step 4.** Today: `The pad is a cushion. It belongs at the back. / Choose it, then turn [KNOB2]: its
      level.` The tester does not like the cushion ("I don't like the cushion reference"). Chosen, my pick ("Option 1"),
      5 lines, all fit (the widest 206): `The pad plays soft, long / notes behind the others. / It belongs at the back, /
      so let's turn it down. / [KNOB2] sets the level.` (Picking the track is the next strip's: `[ALGORITHM] to track
      4: / the pad.`) Lesson 5 step 8 says `A soft cushion.` too: first "Leave it", then "actually lets change that to [4]: the pad.
      Soft, long notes." On one line with `Mute it.` it is too wide (243 px) and would leave `notes.` alone, so at my
      breaks, 2 lines, both fit (101, 206): `[4]: the pad. / Soft, long notes. Mute it.`
  67. **Lesson 14, step 3.** Today: `Press [PLAY]. Every track is at full.` (it breaks after "track"). Asked: the level
      as the screen shows it, `Every track's level is at 100.` (229 px, too wide). Chosen, my pick ("Option 1"), 2
      lines, both fit (112, 196): `Press [PLAY]. / Every track's level: 100.` (The setup puts every level at 100; the
      TRACKS page shows LEVEL 100.)
  68. **Lesson 14, step 2.** Today: `If everyone shouts, nobody is heard. / A mix gives each track its own loudness.`
      (`own loudness.` cut off from "its"). Asked, 4 lines, all fit: `If everyone shouts, / nobody is heard. / A mix
      gives each track / its own volume level.`
  69. **Lesson 14, step 11 (its last card).** Today: `A good mix: drums and bass strong, the tune clear, chords and pad
      behind.` Liked: an underlined heading and the cheat sheet's two lines, but not "behind", nor `Drums and bass:
      strong. / The tune: clear.` (not "lead" either: track 2 is the lead). Chosen ("Option A"), 6 lines, all fit (the
      widest 191): `_A good mix / Loud drums and bass, / a tune that stands out, / softer chords and pad. / Track:
      [ALGORITHM]. / Level: [KNOB2].`
  70. **Lesson 14, step 9.** Today: `Set all five by ear. / [OCT+] when it sounds right.` (`right.` alone). The
      controls named in it did not fit a strip's 3 lines with [OCT+]; the last card names them. Asked, 2 lines, both
      fit (145, 188): `Set all five by ear. / [OCT+] when it's right.`
  71. **A card for the whole course, after the start card.** Asked: "we don't really have a table of contents like card
      in the beginning ... right after the teachie beachie ready for a lesson card ... [oct-] lessons and [oct+] begin
      (which would lead to the lesson 1 card) ... the parts and lessons numbers--it should fit on one page." START,
      then this card; [OCT-] Lessons opens the list, [OCT+] Begin goes on to the lesson's first card. Part 3 renamed
      on the way: "make it move" was vague ("what does make it move mean?"), now **Effects and sound** (asked), in the
      eight lesson files (16 to 23), `lessons/README.md` and `PLAN.md`. Part 2, "add the band", renamed too: **Notes and
      chords** (asked), in lessons 8 to 15 and the same two files. Mocks drawn on the simulated FM-1 and compared:
      the part numbers first needs "&" to fit (`3. Effects & sound: 16-23`), so the lesson numbers first; the last
      sentence asked for ("something that it teaches you how to use Sloop"), chosen from three, `Each lesson teaches /
      one part of SLOOP.`; not under the heading ("too much prominence"); set apart from the list by a gap, 8 px (a
      whole line would run into [OCT-]; 12 px nearly touches it), and centered ("Centered"). The card, 7 lines, the
      card's limit, all fit (the widest 208): `_30 lessons in 4 parts / 1-7: Your first beat / 8-15: Notes and chords /
      16-23: Effects and sound / 24-30: Make a song /` an 8 px gap `/ Each lesson teaches / one part of SLOOP.` (the
      last two centered). To build:
      - Two small marks in the firmware's card drawing (`fw/main.c`): an empty line draws as an 8 px gap, not a whole
        line, and does not count as one of the 7; a line starting with a mark of its own (my plan: `~`) is centered
        on a normal card, as on a big one. `check.py` measures both the same way; `lessons/README.md` says so.
      - The card's words in the lesson pack, not the firmware (244 bytes free); the computer-led program (`run.py`)
        shows it too. [OCT-] Lessons opens the list, [OCT+] Begin the lesson's first card.
  72. **The 8 px gap on the long cards** (item 71's gap; the sheets of every card of 5 lines or more in lessons 1 to 14,
      drawn on the simulated FM-1). Asked, a gap before the line that starts:
      - lesson 1: step 3 `Press it.`; step 4 `Tap it.`; step 6 `Write beats, ...`; step 20 `But a sequencer can.`
      - lesson 2: step 2 `Let's learn how`; step 6 `Hold [SEQ], ...`; step 12 `They call this` and `This is the beat`;
        step 17 `You just programmed`
      - lesson 3: step 2 `Now let's take a look`; step 13 `Write them, ...`
      - lesson 4: step 7 `Each one puts 16 new` and `Your programmed beat`; step 13 `SLOOP has 37 of them.` and
        `[PRESETS] changes`
      - lesson 5: step 11 `[6] to [10] ...`
      - lesson 6: step 21 `Now nothing can`
      - lesson 7: step 2 `An empty drum track ...` and `Make a beat ...`; step 10 `Now you can make` and `Next: the rest`
      - lesson 10: step 2 `The other way is to`; step 13 `Hold [REC] ...` and `Undo brings it back.`; step 15 `That's how
        you build up`
      - lesson 12: step 2 `SLOOP has a mode that`; step 16 `Four chords in a loop:`
      - lesson 13: step 6 `The wrong note is`; step 10 `Let's make the last note`; step 13 `It is at step 13.` and `Go
        there, then press`
      Every one found at the start of a line, with the cards as the notes have them. Every card still fits: the lowest
      is lesson 1 step 3 (7 lines and a gap), its words ending at y 181, the buttons' frames at 188. In the lesson files
      the gap needs a way to be written (my plan: ` // `, as ` / ` is a new line).
  73. **Better breaks on four of those cards** (asked: "suggest better breaks for those"; "Yes, all four"). The gaps of
      item 72 stay where asked (`//` here):
      - lesson 2 step 6, words unchanged: `16 boxes across. / That is one bar of music. // Hold [SEQ], and the / 16
        white keys are / the 16 boxes.`
      - lesson 3 step 13, words unchanged: `More hats? / One in every odd box / makes it rush along. // Write them, or /
        have them written.`
      - lesson 5 step 11, the colon after "mute" a full stop (with it, "only that / track plays." breaks mid-phrase):
        `Solo is the opposite / of mute. / Only that track plays. // [6] to [10] are / the five solos.`
      - lesson 10 step 13, "two" as "2" (as its last card says; `Hold [REC] for 2 seconds:` is 212 px, exactly the
        width): `There is a bigger eraser. // Hold [REC] for 2 seconds: / the whole track is empty. // Undo brings it
        back.`
  74. **The gap on the shorter cards too** ("I just think the gap makes it more legible"; mocks of 19 cards of fewer
      than 5 lines; "All 19"). A gap before the line that starts:
      - lesson 1: step 2 `Try it now.`; step 11 `The blinking key shows`
      - lesson 3: step 12 `The keys for the selected`; step 19 `And you made it.`
      - lesson 4: step 2 `Together they are a kit.`
      - lesson 6: step 8 `We will make a mess`; step 12, new breaks, "together" dropped (it would sit alone): `Undo
        takes back / your last move. // To undo, hold [EDIT] / and press [OCT-].`
      - lesson 8: step 10 `Now a second note.`; step 19 `[SEQ] and a box writes it.`
      - lesson 9: step 2 `SLOOP has 153`; step 13 `Your best sounds are`
      - lesson 10: step 9 `That is called quantize.`; step 10 `One undo removes`
      - lesson 11: step 2 `The ones that fit are`; step 7, new breaks: `They all fit. // This song is in the key / of A
        minor, and A minor / is exactly the white keys.`; step 8 `First tell it which key.`
      - lesson 12: step 6 `One for each bar.`
      - lesson 13: step 2 `No need to start over.`
      - lesson 14: step 2 `A mix gives each track`
      From now on, the review offers a gap wherever a card has two thoughts.
  75. **Lesson 15, all my suggestions** ("Those are great lets go with your suggestions"); `//` is the gap. All fit:
      - step 2: `Part 2 ends with a loop / of your own. // Drums first, then / at least one more track.`
      - step 4 (strip): `Drums first. / [OCT+] when ready.`
      - step 5 (strip): `Now add a bass line, / chords or a tune. / [OCT+] when ready.`
      - step 6: `Want the rest / of the band? // Empty tracks can be / filled in for you.`
      - step 10 (strip): `Save it: / press [SAVE] twice.`
      - step 12 (its last card), as lesson 7's end of part 1, and naming part 3: `Part 2 is done. // Now you can build
        / a whole loop: / drums, bass, chords / and a tune. // Next: effects and sound.`
      Steps 3, 8 and 9 stay as they are.
  76. **Lesson 16, all my suggestions** ("This all look good lets go with your suggestions"); `//` is the gap. All fit:
      - step 2: `So far your music is / one bar, on a loop. / A song needs more. // Each track has 8 pages / to write on.`
      - step 3 (strip): `Press [PLAY]. / This is page 1.`
      - step 4: `Hold [SAVE]: [1] to [8] / are the pages. // Tap one to go there. / Hold one to copy / this page onto
        it.` (`[KEY 1]` as `[1]`: with it the first line is 215 px)
      - step 7: `You are on page 2 now. / It is a copy of page 1. // Change it, and page 1 / stays as it was.`
      - step 8 (strip): `Press [SEQ] for / the drum grid.`
      - step 11: `Two pages. // Page 1: the beat. / Page 2: the same beat / with a push.`
      - step 12 (strip): `Hold [SAVE], tap [1]. / It comes on the next bar.`
      - step 13 (strip): `And back: [SAVE] + [2].`
      - step 16 (its last card), a cheat sheet: `A page is also called / a pattern. // Go to a page: / [SAVE] + a key. /
        Copy this page: / [SAVE] + hold a key.` Asked: "is there a paste step?" No: holding the key copies the page
        onto that one and goes there, in one move (`fw/main.c`, `pat_go`: the top says `PATTERN 2: COPY OF 1 ON THE BAR`).
      Steps 5, 9, 10, 14 and 15 stay as they are.
  77. **Lesson 17, all my suggestions** ("lets go with those"); `//` is the gap. All fit:
      - step 2: `A fill is a short / burst of drums. // It says: something / new is coming.`
      - step 4: `This beat hides / four snares. // They only play / when you ask.`
      - step 5 (strip): `Hold [GLO] and hold [11].`
      - step 6 (strip): `Again. Aim for / the end of the bar.`
      - step 7: `[12] is easier. // It plays the fill for / one whole bar, then / stops by itself.`
      - step 9: `Now a fill note / of your own. // A note can be marked / to play only in a fill.`
      - step 11: `The next move takes / three fingers. // Hold two things, / then press a third.`
      - step 14: `FILL. It worked. / Let go of everything. // The tom now plays / only in a fill.`
      - step 16: `A roll is one drum, / repeated fast. // Hold [ARP] and hold / a drum key.`
      - step 17 (strip): `Hold [ARP], hold [5]: / a hat roll.`
      - step 20 (its last card), a cheat sheet: `Fill: [GLO] + [11] or [12]. / Fill note: / [SEQ] + a key + [OCT+]. /
        Roll: [ARP] + a key.`
      Steps 8, 10, 12, 13, 15, 18 and 19 stay as they are.
  78. **The gap's size: 16 px on short cards** ("so if we wanted to on all cards that have 4 lines or less we could make
      each gap a double gap?"; mocks of 8 and 16 px; "Yes, 16 px"). A card of 4 lines or fewer draws each gap 16 px
      high; 5 lines or more, 8 px. The FM-1 decides it from the card's own line count, so the lesson files keep one
      mark (`//`). Even 4 lines and two 16 px gaps end well above the buttons (y 136 of 184).
  79. **Lesson 18, my suggestions, step 11 in the tester's words**; `//` is the gap. All fit:
      - step 2: `Some effects are / played like notes. // Hold [FX], and every key / is an effect for as long / as you
        hold it.`
      - step 4 (strip): `Hold [FX], hold [7]: STOP.`
      - step 5: `While [FX] is held, / the screen is a map. // 4 bright rows of 4: / the 16 white keys. / The dim names
        between / the rows: the black keys.`
      - step 11: `Tired of holding [FX]? / Lock it, and the keys / stay effects. // To lock: [FX] + [HOME].` ("To lock",
        asked, for my "The lock")
      - step 14 (strip): `Press [FX] once / to unlock.`
      - step 16 (its last card), a cheat sheet: `Effect: hold [FX] + a key. / Lock: [FX] + [HOME]. / Unlock: [FX]
        again.`
      Steps 3, 6 to 10, 12, 13 and 15 stay as they are.
  80. **Lesson 19, my suggestions** ("Lets go with all of these"), steps 2 and 14 still being worked on; `//` is the
      gap. All fit:
      - step 4 (strip): `Tap [FX] until / the screen says SENDS.`
      - step 7: `Too much of both / turns to mud. // Pull the reverb back / until the tune is clear.`
      - step 9: `The echo has a page / of its own. // It is called DELAY.`
      - step 10 (strip): `Tap [FX] until / the screen says DELAY.`
      - step 13: `One more trick: a throw. // Hold [FX] and [OCT+]: / everything echoes / while you hold.`
      - step 17 (its last card), a cheat sheet: `Sends page: [FX]. / Echo: [KNOB3]. / Reverb: [KNOB4]. // A little
        goes a long way.`
      Steps 3, 5, 6, 8, 11, 12, 15 and 16 stay as they are.
  81. **Lesson 19, steps 2 and 14.** Step 2, my option 1 (a picture for each, which children know), 6 lines, two gaps,
      all fit (the widest 203): `Echo repeats a sound, / like a shout in a canyon. // Reverb puts it in a room, / like
      singing in a church. // You decide how much / each track sends.` Step 14 (strip), asked ("They need to hold
      both"), 1 line, 201 px: `Hold [FX] + hold [OCT+].`
  82. **Lesson 20, my suggestions** ("lets go with all of these suggestions"), step 2 still being talked over; `//` is
      the gap. All fit:
      - step 3 (strip), as lesson 19's: `Press [PLAY]. Track 1, / the bass, is chosen.`
      - step 4 (strip): `Tap [FX] until / the screen says INSERT.`
      - step 10: `Now a filter, / on the whole song. // Hold [FX], and [KNOB1] / is the filter knob.`
      - step 11, an underlined heading: `_The filter knob / Left is LP: only / the low sounds stay. / Right is HP: only /
        the high sounds stay. / Middle is OFF.`
      - step 12's hint: `Hold [FX]. FILT is / the first dial below.`
      - step 18 (its last card), a cheat sheet: `Insert page: [FX]. / It colors one track. // Filter: [FX] + [KNOB1]. /
        It colors the whole song.`
      Steps 5 to 9 and 13 to 17 stay as they are.
  83. **Lesson 20: two cards, the insert and the filter,** each saying what it does and why musicians use it ("Let's do
      two cards one for the insert and one for the filter ... I just think filters are a big deal but kids wont know
      that"). Both fit:
      - step 2, the insert (the tester's first lines, "like distortion from a rock guitar pedal", cut to fit): `An
        insert changes / one track's sound, / like a rock guitar pedal. // It makes a bass growl / or a beat crunch: /
        fuzz, crush and more.`
      - a new card just before step 10 (where the filter part begins), with `[OCT+] Next`: `A filter is a DJ's /
        favorite trick. // Close it: the song sinks / underwater. Open it: / it bursts back to life.` (Chosen from two;
        the other added "DJs use it to build up to the drop", which is lesson 27's.) The steps after it move up by one;
        item 82 names them as they are numbered today.
  84. **Lesson 21, my suggestions** ("lets go with all of these"), steps 10 and 19 still being worked on; `//` is the
      gap. All fit:
      - step 2: `Every sound has a shape: / how it starts, / how long it rings, / how bright it is. // You can change
        all three.`
      - step 3 (strip): `Play a few keys. / A plain, sharp sound.`
      - step 5: `ATK is the attack: / how fast a note starts. // Turn [KNOB1] up and / keep playing to hear it.`
      - step 8: `REL is the release: / how long a note rings / after you let go.`
      - step 16 (strip): `Press [HOME]. / Then [PLAY].`
      - step 17 (strip): `Play your sound / over the song. / [OCT+] when done.`
      Steps 4, 6, 7, 9, 11 to 15 and 18 stay as they are.
  85. **Lesson 21, step 19 (its last card), a cheat sheet.** Asked: make the brightness line clear, since [EDIT] pages
      through to the filter. What the FM-1 does: [EDIT] steps through the sound's six pages, OSC, FLT, VOICE, PLAY,
      USER, SOUNDS (`fw/main.c`, `EDIT_PG`), and opens on the one used last, so "until FLT", not a number of taps.
      Chosen ("Option 1"), 6 lines, all fit: `Start: [ENV], [KNOB1]. / End: [ENV], [KNOB4]. // Brightness: / tap [EDIT]
      until FLT, / then turn [KNOB1].`
  86. **Lesson 21, step 10.** Today: `Now brightness. That is the filter. / Its page is called FLT. / [EDIT] goes from
      page to page.` Redone ("I still don't like it I prefer Option 2 but can we make it better"); the paging left to
      step 11's strip (`Tap [EDIT] until FLT.`). Chosen ("Option 1"), 5 lines, two gaps, all fit (the widest 202):
      `Brightness comes from / the sound's own filter. // Turn it down: dark, soft. / Turn it up: bright, sharp. // Its
      page is called FLT.`
  87. **Lesson 22, my suggestions** ("lets go with all of these suggestions"), steps 2 and 22 still being worked on;
      `//` is the gap. All fit:
      - step 4 (strip): `Tap [LFO] until / it says LFO DEST.`
      - step 7 (strip): `Tap [LFO] until / it says LFO 1/3.`
      - step 9: `Now the arpeggiator. // Hold a chord, and it plays / its notes one by one. // It's the sparkle in /
        video-game music.`
      - step 13 (strip): `Hold the three / blinking keys together.`
      - step 14: `Remember the run / from lesson 1, too / fast for hands? // Make the arp / faster and wider.`
      - step 17 (strip): `Hold the same / three keys again.`
      - step 18: `Nobody can play that / by hand. // You just did.`
      - step 19 (strip): `Hold other keys. / Try chords. / [OCT+] when done.`
      Steps 3, 5, 6, 8, 10 to 12, 15, 16, 20 and 21 stay as they are.
  88. **Lesson 22, step 2.** The tester's version (my first suggestion and "It makes pads breathe / and basses wobble.")
      is 7 lines; with two gaps its words would end at y 189, on the buttons' frame (188). Chosen ("Option 2"): "slow
      wave" and "for you" left out, 6 lines, two gaps, all fit: `The FM-1 can move / a sound by itself. // An LFO
      turns a knob / up and down, forever. // It makes pads breathe / and basses wobble.`
  89. **Lesson 22, step 22 (its last card), a cheat sheet.** Asked: "do musicians call it anything other than wobble?"
      The general word is modulation (or movement); on the pitch it is vibrato, on the volume tremolo, on the filter
      wobble (as here: the LFO moves the pad's filter). Kept ("Lets keep Wobble"), 4 lines, all fit: `Wobble: [LFO]. /
      Arpeggio: [ARP]. // A sequencer never / gets tired.`
  90. **Lesson 23, my suggestions** ("Lets go with these"), steps 3 and 9 still being talked over; `//` is the gap.
      All fit:
      - step 2: `Part 3 ends with a show. // You build a second page / and a fill. / Then you play it all live.`
      - step 4 (strip): `Hold [SAVE], hold [2]. / Let go when / the top says COPY.`; its hint: `A short tap only goes
        / to page 2. Hold longer.`
      - step 6 (strip): `Now mark one note / as a fill note.`
      - step 7: `Now play it live. // Change pages, drop in / fills, play the effects. / A minute or so, / like a show.`
      - step 11 (strip), as lesson 15's: `Save it: / press [SAVE] twice.`
      - step 13 (its last card), as lesson 15's, naming part 4: `Part 3 is done. // Now you can play / the FM-1 live. //
        Next: make a song.`
      Step 5 stays as it is.
  91. **Lesson 23, step 9 (strip, while the learner plays).** Today: `You are on. / [OCT+] when the show is over.`
      ("You are on" may not be clear to children). Asked, 2 lines, both fit: `Play your live show. / [OCT+] when it's
      over.`
  92. **Lesson 23, step 3 (the cheat sheet before the show), with an underlined heading** ("Lets go with the heading
      version"), 6 lines, all fit: `_Your show controls / Page: [SAVE] + a key. / Fill note: / [SEQ] + a key + [OCT+]. /
      Fill: [GLO] + [12]. / Effects: [FX] + a key.`
  93. **Lesson 24, my suggestions, steps 2 and 23 in the tester's words**; `//` is the gap. All fit:
      - step 2 (asked): `A song is not one loop. // It has parts: / an intro, a verse, / a chorus, and a break.`
      - step 3: `First, listen to / a whole song. // The strip names / each part as it comes.`
      - step 5 (strip), as step 13's answer says it: `Intro. / Only beat and bass.`
      - step 6 (strip): `Verse. / In come the piano / and the pad.`
      - step 7 (strip): `Chorus. / The whole band, / with the tune.`
      - step 11: `Now you name them. / The song plays again. // At each part, answer / with [OCT-] or [OCT+].`
      - step 19's wrong answer: `No. The bass left: / that is the break.`; step 21's: `No. Everyone is back: / the
        chorus.`
      - step 23 (its last card, asked): `Intro, verse, chorus, / break, and chorus. // Each part changes / something,
        and that / keeps people listening.`
      Steps 8, 9 and the other questions stay as they are.
  94. **Lesson 25, my suggestions, step 2 in the tester's words**; `//` is the gap. All fit:
      - step 2 (asked): `You have four pages: / intro, verse, chorus, / and break. // Played in a row, / the pages make
        a song.`
      - step 3 (strip): `Press [SAVE] once: / the SONG page.`; its hint: `One tap only. / Two quick taps save.`
      - step 7: `A chain plays pages / in a row by itself. // Tap [15], then / the pages in order.`
      - step 9 (strip): `Now [1], [2], [3], / in that order.`
      - step 10 (strip): `The chain plays: / 1, 2, 3, round and round.`
      - step 11 (strip): `[15] again: / the chain ends.`
      - step 12: `A chain is gone / once you end it. // To keep an order, / record it. / [14] is SONG REC.`
      - step 13 (strip): `Tap [14]. / It starts on the bar.`
      - step 15 (strip): `[14] again: / the song is kept.`
      - step 16 (strip): `[13] plays the song / you recorded.`
      - step 18 (strip): `[13] again: / back to looping.`
      - step 19 (strip): `Press [HOME], / then [PLAY] to stop.`
      - step 20 (its last card), a cheat sheet: `SONG page: [SAVE]. / Chain: [15]. / Record a song: [14]. / Play it:
        [13].`
      Steps 4 to 6, 8, 14 and 17 stay as they are.
  95. **Lesson 26, my suggestions** ("Lets go with all of these suggestions"); `//` is the gap. All fit:
      - step 2: `A song can also be / written down, / line by line. // Each line: a page, / and how many bars.`
      - step 3 (strip), as lesson 25's: `Press [SAVE] once: / the SONG page.`
      - step 4 (strip): `[SELECT] one click right: / the lines.`
      - step 5, an underlined heading: `_Four knobs / LINE: which line. / PAT: its page. / BARS: how long. / LINES: how
        many.`
      - step 7: `Four lines, / all playing page 1. // You are on line 4. / Make it page 4, / the break.`
      - step 14: `Your song, written out: // intro, verse, / a long chorus, break.` (`written down:` is 213 px)
      - step 16 (strip): `Tap [13], then [PLAY].`
      - step 17 (strip): `Your song. / [OCT+] when it's done.`
      - step 19 (its last card), a cheat sheet with an underlined heading (asked), each line named as a part of the
        song (lesson 24's parts; "Option 2"): `_Song lines / Get there: [SAVE], / then [SELECT] right. // Each line is a
        part: / a page and its bars.`
      Steps 6, 8 to 13 and 15 stay as they are.
  96. **Lesson 27, step 2**, the tester's words, 7 lines, one gap, all fit (the widest 197): `A build-up adds / a little
      at a time. / A drop is when / it all arrives. // Taking tracks away can / be just as powerful / as adding them.`
  97. **Lesson 27, step 15 (its last card), a cheat sheet with an underlined heading, without the gap** ("lets remove
      the gap if its not defined appropriately it will confuse the user"; the card never said what a gap is), then the
      idea said plainly, with no name for it (the tester's words). 6 lines, one gap, all fit: `_Build-ups and drops /
      Build: a little at a time. / Drop: all at once. // Mute a track, / then bring it back / with a bang.` (Steps 10
      to 12 take the drums out and bring them back; step 2 says why.)
  98. **Lesson 27, my suggestions for the other steps** ("yes go with your suggestions"); `//` is the gap. All fit:
      - step 3 (strip), as lesson 20's filter card says it: `Press [PLAY]. / Bass and drums, / underwater.`
      - step 4's hint, as lesson 20's: `Hold [FX]. FILT is / the first dial below.`
      - step 5 (strip): `Bring in the pad: / [GLO] and [4].`
      - step 6 (strip): `The chords: / [GLO] and [3].`
      - step 9 (reply): `That is / a drop.`
      - step 10: `Now the other way. // Take the drums out / for a moment, then / bring them back / on a strong beat.`
      - step 12 (strip): `Wait for it ... / then [5] again.`
      Steps 4, 7, 8, 11, 13 and 14 stay as they are. "Gap" was only on the last card (and in the lesson's own
      description at the top, "then tries a gap", for Claude, not shown): with it gone, no card says it.
  99. **Lesson 28, my suggestions** ("I like all of the other suggestions"), steps 2, 7 and 17 still being worked on;
      `//` is the gap. All fit:
      - step 3 (strip): `Press [PLAY]. / Straight, like a march.`
      - step 8 (strip), as lesson 16's: `Press [SEQ] for / the drum grid.`
      - step 9 (strip): `Play [5]: the hat.`
      - step 11's hint: `[SEQ], hold the box, / [KNOB2] right. / One box at a time.`
      - step 13: `Playing by hand has / soft and hard too. // Hold [OCT-] and play / a drum: soft. / Hold [OCT+]: hard.`
      - step 15 (strip): `Now hold [OCT+], / play it again.`
      Steps 4 to 6, 10, 12, 14 and 16 stay as they are.
  100. **Lesson 28, step 2.** Today: `The same beat can march or dance. / Swing plays every second box a little late.`
       Worked over: what swing is, how, and why; not "swung" ("I just don't like the word swung"). Chosen, 6 lines,
       two gaps, all fit (the widest 207): `A straight beat marches. / With swing, it dances. // Swing plays every /
       second box a little late. // It's the bounce in / hip-hop and jazz.`
  101. **Lesson 28, step 7.** Today: `Feel also comes from soft and hard hits. / Hats that are all the same sound like a
       machine.` Not "Feel also comes from" (asked); the robot line tweaked (asked). Chosen, 6 lines, two gaps, all
       fit (the widest 192): `Soft and hard hits / make a beat groove. // A drummer never hits / twice the same way. //
       If every hit is the same, / it sounds like a robot.`
  102. **Lesson 28, step 17 (its last card), a cheat sheet with an underlined heading** ("groove", as step 7, not
       "feel"; "Levels" as what it is for). Chosen ("Option 1"), 7 lines, all fit (the widest 197): `_Swing and groove /
       Swing: [KNOB1] on / the TRACKS page. / Loud or soft box: / [SEQ] + a box, [KNOB2]. / Soft hit: hold [OCT-]. /
       Hard hit: hold [OCT+].`
  103. **Lesson 29, step 13: SLOOP's web editor, not Teachie Beachie's backups.** Today: `A copy on the computer is
       safer still. / Teachie Beachie keeps one before every lesson, in its backups folder.` That backup is made only
       when the computer runs the lessons (`run.py`), never with the FM-1 alone. Asked: "If you are working on multiple
       songs you should save it to your computer ... remove any reference to Teachie Beachie ... how sloop manages
       songs on a computer." What SLOOP does: the FM-1 keeps one project; the web editor's Projects tab saves it as a
       file on the computer and puts any file back (`docs/GUIDE.md`, The web editor). Chosen ("option 2"), 6 lines,
       all fit: `The FM-1 holds one / song at a time. // Working on more? / Keep each one as a file / on a computer, with
       / SLOOP's web editor.`
  104. **Lesson 29, my suggestions for the other steps** ("go with the rest of your suggestions"); `//` is the gap. All
       fit:
       - step 2: `A finished song / deserves to be kept. // The FM-1 holds one / project: all your pages, / sounds and
         the song.`
       - step 3: `Hold [HOME]: / the PROJECT page. // [1]: OPEN. / [3]: NEW. / [5]: SAVE.`
       - step 4 (strip): `Hold [HOME], / tap [5]: SAVE.`
       - step 6: `Saved. Now let's see / what that is worth. // We wreck it on purpose: / the drums get erased.`
       - step 7 (strip), as lesson 10's: `Hold [REC] for 2 seconds.`
       - step 8 (strip): `No drums. / They are gone.`
       - step 9: `OPEN brings back / what you saved. // It asks if you are sure, / so you tap twice.`
       - step 12: `The FM-1 also saves / by itself when the / music is stopped. // So save before you / try something
         risky.`
       - step 14: `To share a song, / record it as sound. // The FM-1 sends its sound / down the USB cable / to a
         computer. / Ask a grown-up.`
       - step 15 (its last card), a cheat sheet with an underlined heading, "hold" [HOME] as it works, and the web
         editor of step 13 ("Option 1"), 7 lines, one gap: `_Keep your song / Save: hold [HOME] + [5], / or [SAVE]
         twice. / Open: hold [HOME] + [1], / then [1] again. // More songs: as files, / with the web editor.`
       Steps 5, 10 and 11 stay as they are.
  105. **Lesson 30: a cheat sheet of four cards** ("we should put in a comprehensive cheat sheet in the beginning maybe
       even two cards ... on step 3 at the bottom the right is continue and the left is Cheat Sheet"; "Yes this looks
       good"). Step 3 (the goals) gets `[OCT-] Cheat sheet` and `[OCT+] Next`; Cheat sheet goes to four new cards, one
       per part of the course (as the course card, item 71), each `[OCT-] Back` and `[OCT+] Next`, the last `[OCT+]
       Start` (to step 4); Next on step 3 skips them. Step 6's `Not yet` goes back to step 3 (today to step 5), so the
       cheat sheet is one press away whenever they are stuck. Each card 7 lines, all fit:
       - `_Your first beat / Play, stop: [PLAY]. / Pick a drum: play its key. / Boxes: [SEQ] + a key. / Tempo:
         [SELECT]. / Undo: [EDIT] + [OCT-]. / Mute: [GLO] + [1] to [5].`
       - `_Notes and chords / Track: [ALGORITHM]. / Sound: [PRESETS]. / Record live: [REC]. / Key and chords: [SEL]. /
         Piano roll: [SEQ] twice. / Level: [KNOB2].`
       - `_Effects and sound / Effects: hold [FX] + a key. / Filter: [FX] + [KNOB1]. / Echo, reverb: [FX]. / Fill:
         [GLO] + [12]. / Wobble: [LFO]. / Arpeggio: [ARP].`
       - `_Make a song / Page: [SAVE] + a key. / Copy: [SAVE], hold a key. / SONG page: [SAVE]. / Chain: [15]. / Record
         a song: [14]. / Play the song: [13].`
       Saving is left off (step 4 and the corner tag say it); left out for room: redo, solo, erasing a track, the
       insert, rolls, swing, [ENV].
  106. **Lesson 30, my suggestions for its cards** (taken as yes with the cheat sheet; to check with the tester);
       `//` is the gap. All fit:
       - step 2: `_The final test / An empty project. / No blinking lights. / No hints.`
       - step 3: `Make a whole song: / 3 pages or more. / 4 tracks or more. / Some effects. / A song that plays / 3 pages
         in a row.`
       - step 4: `When your song is done, / press [SAVE] twice. // The corner of the / screen reminds you: / [SAVE] x2`
       - step 6: `Is your song finished? // The goal: 3 pages, / 4 tracks, some effects, / and 3 pages in a row. // You
         decide.`
       - step 8 (strip), as lesson 26's: `Your song. / [OCT+] when it's done.`
       - step 10 (the course's last card), as the ends of parts 1 to 3: `The course is done. // Now you can make / a
         song from nothing, / on your own. // The FM-1 is yours now.`
       Step 5 (the corner tag) stays as it is.
  107. **The review is done (2026-11-22):** all thirty lessons gone through. Next: make items 53 to 106 (with the gap,
       the centered line and the course card in the firmware), build, check, and put it on the FM-1.
- **2026-11-22, evening: the second round made** ("yes make them"), items 53 to 106.
  - **The gap** (items 71, 72 to 74, 78): in a lesson file ` // `; `words()` (`tp/lesson.py`) turns it into an empty
    line, two on a card of 4 lines or fewer; the FM-1 draws an empty line 8 px high (`fw/main.c`, `guide_draw`). So the
    16 px is decided by the tools, not the firmware as first planned: fewer bytes. `check.py` counts gaps apart from
    lines and says when a card would end on its answers.
  - **A centered line** (item 71): `~` at its start (`guide_line`).
  - **The course's card** (item 71): after START, before the lesson's first card; its words are `COURSE` in
    `tp/lesson.py`, packed after the fonts (`tools/mkcourse.py`); the pack is `TPC3` now, so a firmware and a pack
    must again be from the same build. A new phase in `fw/course.inc`, PH_TOC (13): BEGIN goes to the lesson's first
    card, LESSONS to the list. The computer-led program shows it too (`run.py`, `begin`).
  - **The lessons:** every card of items 53 to 106, lesson 20's filter card (19 steps now) and lesson 30's cheat sheet
    (14 steps: the goals' card has `Cheat sheet` and `Next`, four cards with `Back` and `Next`, `Not yet` goes back
    to the goals). The last cheat card says `Next`, not `Start`: the card after it says `Start`. Lesson 7 step 2's
    last line cut in two (`Make a beat / of your own.`: it wrapped). Parts 2 and 3 renamed in the lesson files,
    `lessons/README.md` and `PLAN.md`; the README has the two new marks.
  - **Sizes:** the firmware 278,148 bytes (152 more; 92 free), the pack 53,850.
  - **Checked:** the build and its own checks; the lint of the thirty lessons; every break where it was written; on the
    simulated FM-1 leading itself, START to the course's card, to lesson 1 and to the list, and lesson 30's cheat
    sheet walked forward and back (pictures looked at); all thirty lessons replayed on simulated FM-1s that lead themselves: 30 played through, none stopped.
  - **Left as they are, seen on the way** (old cards that still wrap on their own, not in the notes): lesson 4 step
    6, lesson 5 steps 4, 7 (`piano.` alone) and 14 (`like.` alone), lesson 9 step 6, lesson 10 step 12 (`[OCT+].`
    alone), lesson 12 step 8.
- **2026-11-22: room in the firmware, and what could come out one day** ("Sloop is in beta, its highly likely it will end
  up taking more space so we should think about what we want to trim"; "don't actually trim anything now we just
  might want to identify it"). Nothing trimmed. The firmware's code is 278,148 bytes of 278,240: 92 free. Teachie
  Beachie's own part of it is small: about 6.2 KB of code (the guide and the course in `fw/main.c` and `fw/course.inc`)
  and the 2.1 KB logo. The lessons' words are in the pack, not there. The sizes below are each part's object file
  (`build/fw/o/*.o`), before the build packs it: what a cut really frees has to be measured by building without it.
  - **SLOOP parts the lessons never use, most promising first:**
    - What is left of `sloopimport.o`, about 2.4 KB (not the 6.6 KB of its object file, as first said here): the
      import of a SLOOP 2.8 project at the first start is out already, since 2026-11-19 (`TP_SLOOP28_PROJECT` in
      `fw/main.c`; the tester remembered it: "I thought we removed that"). Still in: SLOOP 2.8's settings, favorites,
      user sounds, SYN kits and MIDI settings brought over (`sloop_upreset`, `sloop_synkits`, `sloop_midi`,
      `sloop_settings`, `sloop_favs`, `sloop_theme`; measured in `build/fw/ram.elf`). A new FM-1 owner never has
      those. The cleanest cut. And the lesson for the rest of this list: an object file's size says more than is
      really in the firmware.
    - `slice.o` and `slice_scan.o`, 5.0 KB: the SLICE engine (a drum break cut into slices, made with the web editor).
    - `grain.o` 5.7 KB and `texture.o` 4.1 KB: the granular and texture sound engines. Some of SLOOP's 153 sounds use
      them (STR CLOUD is granular), so the count on lesson 1's card would change.
    - `fm6.o` and `fm6_patches.o`, 8.3 KB: the six-operator engine and its DX7 bank.
    - `phys.o`, 29.3 KB: the physical-modelling engine, the biggest single part after the main program. Its sounds
      would go.
    - `vis.o`, 9.0 KB: the 3D picture of the sound on the start page (the logo, 2.1 KB of it, is ours and stays).
    - Small ones: `midi.o` 1.6 KB, `usersnd.o` 1.6 KB, `demo_song.o` 1.5 KB.
  - **On our own side:** the logo and the built-in cards' words could move into the lesson pack, as the course's card
    did (about 2.5 KB); a card's words there cost the firmware nothing.
  - **Not to touch:** `edproto.o` and the editor's part of the main program (the lessons are sent to the FM-1 that
    way), the fonts SLOOP's own pages use, the sequencer, the drums.
  - **How, when it is needed:** merge the new SLOOP, build, read how many bytes it is over, then cut from this list
    in order, each behind a switch of our own in the build, so SLOOP's files stay as they are and the next merge
    stays easy. The recorded samples (the drum kits spoken of before) sit outside this code, with their own room:
    whether cutting them helps the code has to be checked first.
- **2026-11-22, late: the installer page sends the lessons** ("how are we going to make it easy for others to get the
  lessons when my github page is made public?", then "Yes, build it").
  - **Before:** the page installed the firmware only. The lessons went on with `tutorial/device.py --install`, which
    needs Python and this whole project: nobody else could do it.
  - **Now:** the page's sixth step is "Lessons", with one button, **Send the lessons**. After the install the FM-1
    restarts on Teachie Beachie; the page finds it by itself (it waits up to 30 seconds for it), sends the thirty
    lessons, and checks that the FM-1 finds all of them. The FM-1 says LESSONS RECEIVED and shows its start card.
  - **Where they go:** the last sample slot that holds no samples (USR4 on an FM-1 with no samples of its own),
    the same rule as `device.py`. If all four slots hold samples, the page lists the four and asks which one to
    replace; nothing is written before a yes.
  - **The lessons alone:** in step 1, "Teachie Beachie already on your FM-1? ... send the lessons only" opens the
    last step by itself. This is how newer lessons go on later, without installing again.
  - **What the page says when something is wrong:** no FM-1 found; the FM-1 is still in MIDI MODE; the FM-1 runs
    SLOOP 3 without Teachie Beachie; the FM-1 took the lessons but cannot read them (an older Teachie Beachie:
    install from this page first).
  - **How it is made:** `tools/teachie_page.py` packs the lessons (`tools/mkcourse.py`) and puts them inside the
    page with `web/teachie_lessons.js`, which sends them the way SLOOP's web editor sends samples. So one page
    always holds a firmware and lessons of the same build. SLOOP's own page template is untouched. **Nothing in the
    firmware changed:** it is byte for byte what was installed this evening, and its 92 bytes are still free.
  - **Checked:** a new test in the build, `tests/lessons_sim.js`: a fake FM-1 (the last empty slot, a busy FM-1
    asked again, a full one, each thing that can be wrong, the lessons alone), then the simulated FM-1 with the real
    firmware: thirty lessons in 2 seconds, found by the FM-1, its flash byte for byte the pack, sent twice into the
    same slot. In a real browser with a pretend FM-1 inside the page: sending, the full-FM-1 choice and the
    messages, with no error.
  - **Not checked yet:** a real FM-1 in Chrome. `tests/installer_e2e.js` (it needs Playwright, which is not on this
    computer) was not run either.
  - **To know:** the lessons alone work as long as the FM-1's firmware reads this pack's format (its mark is TPC3).
    When the format changes the mark must change too: an older firmware then finds no lessons and the page says to
    install first.
  - **Before the repository is public** ("Is there any risk to just making it public?"). Looked through every file
    and the whole history: no real name, email or home folder in any file or picture; every commit of ours under
    the project's name and GitHub's no-reply address; the logs, backups and progress file are not in the
    repository.
    - **Done:** a README of Teachie Beachie's own ("can you create a new readme"; before, it was SLOOP 3's and
      pointed to SLOOP's page), with a picture of the course's card and lesson 1's
      (`docs/teachie-beachie-lessons.png`). A personal detail about the first deadline is taken out of these
      notes and the plan, as asked.
    - **Left:** old commits still hold those lines, and one old line with the computer's home folder in a path;
      only rewriting the history removes them, which is to be asked for first. The README's install link is the
      GitHub Pages address, which works only once Pages serves the installer page there. The page tried on a real
      FM-1.
    - **Who it was made for stays, and goes on the page** ("its fine to say that I made this as part of a
      Christmas present, and that I made it for kids. I think we should probably mention on the website that it
      was made for kids because I think other people will want their kids to use this tutorial"). Option 1 of
      three: the page's introduction now begins `Teachie Beachie is a tutorial made for kids, and for any beginner
      new to the M-VAVE FM-1 and SLOOP.` and ends `I made it for my own kids, as part of a Christmas present.`; the
      rest is as it was. The same two sentences are in the README, and the page's description for search engines
      says `made for kids and beginners`. No names and no ages, as before.
    - **The introduction from the left** ("I don't like how it is centered can we make it left aligned without it
      looking too weird"). The logo and the title stay in the middle. The words under them, and the three checks,
      now start at the title's left edge and are exactly as wide as the title, at every width (on a phone, the
      whole width). Looked at in a browser at three widths, and with the "this browser can't install" box shown.
      Seen in passing: on a phone the title broke as `FM-` / `1`, and so did `FM-1's` in the words. Fixed ("yes fix
      it"): every FM-1 in the title and the introduction has the hyphen that does not break.
    - **Two paragraphs** ("can you start a new paragraph at Teachie Beachie can light up the FM-1's buttons and
      keys ..."): the introduction's second paragraph starts there, on the page and in the README.
    - **A third paragraph, in the author's words** ("can you separate the last sentence about making it for my kids
      into a new paragraph and fix it up"). Given: "I made Teachie Beachie for my kids, nieces and nephews as part
      of a Christmas present. If you have kids that would like to learn how to use a groovebox and the FM-1 I'd
      encourage you to give Teachie Beachie a try." On the page and in the README, with two commas added and
      "kids that" made "kids who": `I made Teachie Beachie for my kids, nieces and nephews, as part of a Christmas
      present. If you have kids who would like to learn how to use a groovebox and the FM-1, I'd encourage you to
      give Teachie Beachie a try.` It replaces `I made it for my own kids, as part of a Christmas present.`
- **2026-11-22, late: the installer page for a newcomer** ("if you have any other suggestions on how to make this
  look better or the content more compelling please let me know", then of the eight offered: "Lets do all of those
  things"). All in `tools/teachie_page.py`; SLOOP's template is untouched.
  1. **What a lesson looks like.** Six screens in a row between the introduction and the steps (three across on a
     tablet, two on a phone): the course's card, then lessons 3 (drums), 13 (the piano roll), 20 (a filter), 27
     (the drop) and 30 (Done). They are from the run on the simulated FM-1, kept in `assets/shots/`. The README's
     picture is the same six.
  2. **Step 1 for a newcomer.** The beta note in plain words: `SLOOP 3 is new and still being tested on real FM-1s,
     and so is Teachie Beachie, so you may meet bugs. This page backs up your FM-1 before it changes anything, so
     you can always go back.` SLOOP's two notes for its own users (the tried and tested version; updating from 3.0
     to 3.3.4) are folded into one closed section, `Already use SLOOP?`.
  3. **What the learner ends up with**, under the six screens: `Thirty lessons in four parts: Your first beat,
     Notes and chords, Effects and sound, Make a song.` / `By lesson 7 you have made a beat of your own. By lesson
     30, a whole song.`
  4. **Free, and no computer afterwards**, in the second paragraph: `It is free, and once it is installed no
     computer is needed.`
  5. **A fact where the page sounded like an ad:** `It's the best way to learn the basics of sequencers and
     grooveboxes as well as specific workflows in SLOOP.` is now `Each lesson takes a few minutes and builds on the
     one before.`
  6. **The steps are counted to 6** (`Step 1 of 6` ...), and the lessons' step says `Step 6 of 6`.
  7. **The browser's three checks** (and what the page says when a browser cannot install) moved from the top into
     step 1, so the top ends on the author's words.
  8. **A line under the title:** `Thirty lessons that teach the FM-1, on the FM-1 itself.` Because of it, `on the
     FM-1 itself` came out of the first paragraph, which said it too.
  - Looked at in a browser at desktop, tablet and phone widths; both page tests pass.
  - **The six screens in dark frames** ("do you think we should change the website background so it doesn't
    conflict with the screenshots?"). A peach card on the peach page was lost: only a thin line set it apart. The
    page keeps its color; each screen now sits in a near-black frame with round corners and a soft shadow, as the
    FM-1's screen sits in its black panel ("Yes, frame them").
  - **Fatter, gray frames; the four parts in the introduction; the introduction rewritten** ("Can you make the
    frames even fatter so they are closer to an FM-1 maybe make them a little bit gray. Can you move Thirty Lessons
    in four parts into the intro paragraphs. can you rewrite the intro paragraphs so they are more compelling?").
    - The frames: a fat dark gray edge, the FM-1's body, with the screen's black edge inside it.
    - The introduction, five short paragraphs now (the last is the author's own, unchanged), on the page and in the
      README:
      `Teachie Beachie teaches kids, and any beginner, to make music on the M-VAVE FM-1. You don't need to read
      music or open a manual: every lesson shows you what to press, right on the FM-1's screen.`
      `A lesson puts a short card on the screen, lights up the button or key to press, and waits while you try it.
      If you get stuck it offers a hint, and it can even do the step for you. Each lesson takes a few minutes and
      builds on the one before.`
      `There are thirty lessons in four parts: Your first beat, Notes and chords, Effects and sound, Make a song.
      By lesson 7 you have made a beat of your own. By lesson 30, a whole song.`
      `Teachie Beachie is free, and once it is installed no computer is needed. It is built on SLOOP 3, a complete
      groovebox, which stays when the lessons are done.`
      `I made Teachie Beachie for my kids, nieces and nephews, as part of a Christmas present. If you have kids who
      would like to learn how to use a groovebox and the FM-1, I'd encourage you to give Teachie Beachie a try.`
    - The line under the six screens is gone: its words are the third paragraph.
  - **A section about an AI assistant, under the six screens** ("Can you add a section under the screenshots for
    the AI connected functionality?").
    - First written as two boxes: the program on the computer that leads the lessons (`Works today`), and an AI
      tutor that joins in, marked `Not built yet` from the plan's own words ("Claude does not cut in yet"). The
      tester: "Why is that not built? I have been doing that for a week now". Both were true, of two things: an
      assistant open on this project beside a lesson works and was used all week; an assistant joining in by
      itself, with its own words on the FM-1's screen (`PLAN.md`, section 9), is not built.
    - Then, the tester's direction: "Can you not make it specific to Claude. As far as the computer leads the
      lesson, I don't think we want to include that because it doesn't really add value over the on device
      lessons. as far as the AI, we don't need it to answer on the FM-1 I don't even know if thats a good idea
      really. can you rewrite this section based on what is possible now don't have a section that says not built
      yet and don't make it specific to claude. if we don't have any documentation that is specifically for AI can
      you write some."
    - **The section now:** `Take it further with an AI assistant`, a few words (the lessons are plain text files,
      written for people and for AI; a tutor for the learner and a lesson writer for you; built and tested this
      way), then three boxes: `Ask about any step`, `Change a lesson`, `Write your own`, and a link to the new
      guide. No assistant is named, nothing is marked as not built, and the program that leads lessons from the
      computer is not on the page (it stays in `tutorial/README.md`). The README has the same section.
    - **New documentation, for any assistant:** `tutorial/AI.md` for the person (what is needed, how to start,
      what to ask, what the assistant sets up, what it cannot do: see the screen, hear the FM-1) and `AGENTS.md`
      at the top for the assistant itself (where things are, how to be a tutor, how to change and check a lesson,
      what to ask before doing). Its commands were tried: the lesson check, and the connector's reads on the
      simulated FM-1.
    - **The lesson files' notes renamed** ("Yes, rename them"): `## Notes for Claude` is `## Notes for the AI
      assistant` in all thirty. Four notes that spoke of the lesson waiting for Claude (lessons 7, 15, 23, 30) now
      speak of a moment for the assistant, and say to read the FM-1 or to ask, since it cannot hear. No program
      reads that heading: the packed lessons are byte for byte as before.
    - **The assistant controls the FM-1** ("The part that is missing is AI can control the device"). The guides
      had it reading the FM-1 only. On the cable it can also put a lesson's song on the FM-1 (a lesson's Setup
      line, run by hand), play and stop, set the tempo and the mix, choose sounds, write notes, turn a knob, light
      a button and put words on the screen; and it can save the project and put it back. All of it is in the
      connector already: this is how the lessons were first built. Now in `AGENTS.md` ("The FM-1 on its cable":
      how to connect, read, control, and the rules: say what is about to happen and wait for a yes, save a copy
      before the song is replaced, show and do not do the learner's work, leave no card or light behind), in
      `AI.md` with things to ask, in the plan, and on the page and the README as a fourth box, `Control the FM-1`.
      Tried on the simulated FM-1: the copy saved, a song set up and played, stop, the tempo, notes written, a
      light and a strip, a knob, a key, the copy put back. Not tried from these guides on a real FM-1.
    - **The plan brought up to date** ("Yes, update it"): `PLAN.md` names no assistant now. Section 3 is the
      course on the FM-1 with an assistant beside it, in place of two ways to take the course; section 9 is what
      an assistant beside a lesson does, and records that an assistant cutting in by itself was set aside on
      2026-11-22, never built. The program that leads lessons from the computer is kept as a tool for building
      and testing. The tutorial's README no longer lists the cutting in as not built yet.
  - **The top as wide as the sections under it** ("Is there any way we can redesign the top section (above the
    screenshots) so that it has the width of the sections below. but do it in a way so its not spread all the way
    across the screen ... it just doesn't look that good right now"). It was a narrow column in the middle, over
    wide sections. Tried in turn, each looked at on the page:
    - two columns, the words at the left and a large logo at the right, level with the middle of the words;
    - the logo moved to the top of its column ("can you move the logo up");
    - the logo at the left and the words at the right ("can I see what it looks like if the logo is on the left");
    - the logo small at the top left, the words under it, and six more screens filling the right column ("what
      if we put it at the top put the text back on the left and filled the other column with 6 more screenshots
      you can make the logo smaller");
    - **and the one kept** ("I think the logo should be on top of everything then below it in the two columns on
      the left is the text, then the column on the right should be the screenshots but they need to be the same
      size as the ones below ... can you make it so the screenshots on the top go in order top is first then the
      next six are after don't use the ready for a lesson slide don't use faster faster faster I like shots that
      use the overlays in a smart way"):
      - the logo (124 px) on a row of its own, above everything, at the left edge;
      - under it, on the six columns of the screens below: the title, the line and the five paragraphs over the
        first four columns, and six screens over the last two, two across. So each of those is exactly as large
        as the six under them, and stands right over the fifth and sixth of them;
      - twelve screens in the lessons' order, most of them a strip or a tag of the lesson's words over SLOOP's
        own page. First chosen with no plain cards at all; then ("can you put the course one in first ... you can
        remove the pluck one to fit it in. put lesson 20 filter one back in, remove the fx one. keep everything in
        order"): beside the words the course's card, then lessons 1 (the five tracks), 3 (the drum grid), 8 (the
        bass line's boxes), 12 (chords on the keys), 13 (the piano roll); under them, after one more change ("remove
        the last lesson 27 screenshot and the current lesson 30 one and put in lesson 14 the mix that we had
        before and the done slide from lesson 30. keep everything in order"): first lesson 14's card about the mix;
        then a cheat sheet in its place ("can you find a 'Cheat Sheet' between lesson 13 and lesson 20 and swap it
        for the mix one?"), lesson 14's `A good mix` offered and lesson 16's chosen ("can you use the page is
        called a pattern one instead"): `A page is also called / a pattern. // Go to a page: / [SAVE] + a key. /
        Copy this page: / [SAVE] + hold a key.`; then 20 (the card about the filter), 21 (a sound's envelope), 22 (a slow wave), 25 (the song page) and the last card,
        `Lesson 30 Done`. `assets/shots/`; the README's picture is the same twelve.
    - On a tablet the same, smaller. On a phone: the logo and the title in the middle as before, the words under
      them, and the six screens beside them left out (the six below follow at once). The title breaks only
      between `Install`, `Teachie Beachie` and `on your FM-1`.
    - The `AI assistant` boxes' order is Ask, Control, Change, Write ("can you put control the FM-1 after Ask about
      any step").
  - **A short film of a lesson, for the page: wanted, not tonight** ("we can do the clip but not tonight. I guess
    we should find a compelling part to film"). Candidates: the end of lesson 1 (the repeat-after-me game with its
    lit keys, then `Faster / Faster / FASTER` and `Ha, sorry, I'm just messing with you`), which works without
    sound; lesson 27's build-up and drop, or lesson 20's filter (`the song sinks underwater`), which need sound.
    A real FM-1 filmed from above shows the lit keys, which the simulated FM-1's screen does not.
- **2026-11-24: notes after two play-throughs on the FM-1 alone.** The first young learner played lessons 1 to 13,
  and the tester played all thirty again. To be made all at once, when the notes are all in ("Lets add these notes
  once we have all the notes we can implement them").
  108. **Lesson 1: a short word about the FM-1 itself.** "Maybe I should give a short overview of the FM-1, the two
       things that I think we should mention is master is the volume. And we recommend using headphones, the FM-1
       has good sound but the speaker is not great." No card says either today: MASTER is on the installer page
       only, and the speaker only in lesson 14's notes for the assistant. Suggested: one new card after `Good.`
       (step 5) and before `SLOOP is a groovebox` (step 6), so it is read before the first sound (step 7). No step
       in lesson 1 jumps to a number, so adding one is safe. Measured, both fit (5 lines and a gap):
       - `_Your FM-1 / [MASTER] is the volume. // Headphones are best. / The FM-1 sounds good, / but its speaker
         is small.`
       - nearer the tester's words: `_Your FM-1 / [MASTER] is the volume. // Use headphones. / The sound is good.
         / The speaker is not great.`
       `We recommend headphones.` does not fit on one line.
       The tester's own card: `_Your FM-1 / [MASTER] is the volume. // We recommend using / headphones. / The FM-1
       has good sound. / but the speaker is not great.` Its last two lines are too wide (221 and 236 of 212), so
       the screen would break them and the card would run to 8 lines and a gap, too tall with an answer under it.
       Nearest that fit, 7 lines and a gap or fewer: `... / We recommend using / headphones. / The FM-1 sounds
       good, / but the speaker / is not great.`; with the exact words, `... / The FM-1 has good / sound, but the
       speaker / is not great.`; one line shorter, `... / The sound is good. / The speaker is not great.`
       **Chosen** ("Option 2", the tester's exact words): `_Your FM-1 / [MASTER] is the volume. // We recommend
       using / headphones. / The FM-1 has good / sound, but the speaker / is not great.` 7 lines and a gap, the
       most a card with an answer holds; its answer `OCT+: Next`.
  109. **Lesson 5, step 14 (a strip): it wraps.** The tester (who named it lesson 4): "Mute and solo as you like.
       It's wrapping. Maybe: Mute/solo as you like." Today the screen breaks it as `Mute and solo as you` / `like.`
       / `[OCT+] when done.` (it was on the list of old cards that still wrap awkwardly). Measured:
       - the tester's: `Mute/solo as you like. / [OCT+] when done.` 2 lines, 181 px of 212: fits;
       - keeping "and": `Mute and solo / as you like. / [OCT+] when done.` 3 lines, the most a strip holds: fits.
       **Chosen** ("Option 1", the tester's): `Mute/solo as you like. / [OCT+] when done.`
  110. **Lesson 8, step 13 (a card): what is it for?** The tester: "'Listen for where the bass and the kick meet and
       where the bass slips between', what is the intent of this card?" The intent: the song plays while the card
       is up, and it points at why the bass line grooves. The kick is on boxes 1, 5, 9 and 13; the learner's bass is
       on 1, 4, 7, 11, 14 and 16. They hit together on box 1 only, and the rest of the bass lands between the
       kicks. That is the push of funk and disco, the style the lesson names. But the card says it in the abstract
       ("meet", "slips between"), gives the learner nothing to listen for, and is one sentence the screen breaks
       on its own. Suggested, all measured and fitting:
       - `The bass and the kick / hit together on [1]. / The rest of the bass / lands between kicks. // That is
         the groove.`
       - `_Bass and kick / The kick is on / 1, 5, 9 and 13. / Your bass is on 1 too, / then lands in between. //
         That is what makes / it groove.`
       - `Listen. The bass hits / with the kick on [1], / then lands between / the kicks. // That bounce is / the
         groove.`
       **Chosen** ("Option 2"): `_Bass and kick / The kick is on / 1, 5, 9 and 13. / Your bass is on 1 too, /
       then lands in between. // That is what makes / it groove.` 7 lines and a gap; its answer `OCT+: Next`.
  111. **Lesson 9, steps 8 and 9: the SOUNDS page does not always show 16 sounds.** The tester: "Instrument select
       step can go weird because it references 16 sounds and some families don't have 16 sounds." True: the page
       shows a family 16 at a time, and the families hold 10 to 38 sounds, so a family's last page is part empty,
       and FAVS holds only the learner's favorites (none, at first). The learner arrives on whatever family their
       PRESETS clicks (steps 4 and 5) left the lead in. Step 8 tells them to tap EDIT "until you see 16 sound
       names": on a short page they keep tapping past it. Step 9 says "Each white key is a sound": on a short page
       some are not. Suggested, measured (a strip holds 3 lines):
       - step 8: `Tap [EDIT] until you see / sound names.` (today `Tap [EDIT] until you see / 16 sound names.`)
       - step 9: `Each name is on / a white key. Tap one.` (today, as the screen breaks it, `Each white key is a /
         sound. Tap one.`)
       **Chosen** ("Yes, use them"): both, as suggested.
  112. **Lesson 14, step 9: the strip covers the levels.** The tester: "it says set all five by ear [oct+] when it's
       right. But the strip is at the bottom covering up the levels. I think the strip should be at the top. Would
       be best if it was all one line." Steps 6 and 8, just before it, are at the top for that reason already.
       Suggested: `Strip top`, one line. Today `Set all five by ear. / [OCT+] when it's right.` (2 lines); on one
       line it is 333 px of 212. Measured, one line each:
       - `Mix by ear, then [OCT+].` (202 px)
       - `Mix all five, then [OCT+].` (203 px)
       - `Set all five, then [OCT+].` (201 px)
       **Chosen** ("Option 1"): `Strip top: Mix by ear, then [OCT+].`
  113. **Lesson 15, step 5: a smaller strip.** The tester: "'Now add a bass line chords or a tune [oct+] when ready.'
       would be better if this was smaller. Now add another track or something." Today 3 lines, the most a strip
       holds, over the page being written on: `Now add a bass line, / chords or a tune. / [OCT+] when ready.`
       Suggested, 2 lines, in the tester's words and the same shape as step 4 (`Drums first. / [OCT+] when
       ready.`): `Now add another track. / [OCT+] when ready.` (199 px). Shorter still: `Add another track. /
       [OCT+] when ready.` **Chosen** ("Option 1"): `Now add another track. / [OCT+] when ready.`
  114. **Lesson 16, the whole lesson** ("Lesson 16 is rough I think maybe we need one more card to explain what is
       happening, I'd like to go through the whole lesson because I think of all of the lessons this is the most
       confusing"). What is hard in it, from its screens and the firmware:
       - step 2 says "Each track has 8 pages", but `[SAVE]` + a key moves all five tracks to that page together,
         and a copy copies all five (`pat_go` in `fw/main.c`): a page is really the whole band's;
       - nothing says where to see which page is playing: `P1` / `P2` at the top right of the tracks page;
       - a copy sounds exactly like the page it came from, so after it nothing seems to have happened, and nothing
         says why one would copy at all (a short tap goes to an empty page instead: the trap of lesson 23);
       - on the drum grid there is no `P1` / `P2`; the only sign of the page is the extra kicks on 8 and 16;
       - a page change waits for the bar to end (`PATTERN 1 ON THE BAR`), so a press seems to do nothing at first.
       Suggested, every one measured and fitting (no step of the lesson jumps to a number, so a new card is safe):
       - step 2: `So far your music is / one bar, on a loop. / A song needs more. // You have 8 pages. / Each page
         holds / all five tracks.`
       - step 3: `Press [PLAY]. This is / page 1: P1, top right.`
       - step 4: `_Pages: hold [SAVE] / Keys [1] to [8] / are the 8 pages. // Tap a key: go there. / Hold a key:
         copy / this page onto it.`
       - a new card after step 4, the one more card asked for: `_Why copy a page? / A new page is empty. / A copy
         starts with / your music on it. // Then change a little: / a new part of the song.`
       - step 5, the same words as lesson 23's copy: `Hold [SAVE], hold [2]. / Let go when / the top says COPY.`
       - step 7: `You are on page 2: / see P2, top right. / It is a copy of page 1. // Change page 2. / Page 1
         stays as it was.`
       - step 11: `_Two pages / Page 1: the beat. / Page 2: the beat, plus / kicks on [8] and [16]. // Switch
         between them: / [SAVE] + [1] or [2].`
       - step 12: `Hold [SAVE], tap [1]. / It waits for the bar / to end, to stay in time.`
       - unchanged: steps 6 (`Copied.`), 8, 9, 10, 13, 14, 15, and the last card (the one on the installer page).
       **Chosen:** all of these except steps 5 and 12 ("I like all of your suggestions except for Step 5 and Step
       12--I don't like increasing the lines. can we make them clearer and keep them on two lines"). Two lines
       each, measured:
       - step 5: `Hold [SAVE] + [2] until / the top says COPY.` or `Keep [SAVE] + [2] down / until the top says
         COPY.`
       - step 12: `Hold [SAVE], tap [1]. / Wait for the bar to end.` or `Hold [SAVE], tap [1]. / It waits for the
         bar's end.`
       **Chosen** ("For Step 5 lets do option 1 for step 12 Option 2"): step 5 `Hold [SAVE] + [2] until / the top
       says COPY.`; step 12 `Hold [SAVE], tap [1]. / It waits for the bar's end.`
       Then lesson 17, gone through the same way ("Lesson 17 is the lesson that I remember being really rough").
  115. **A card before every three-at-once move** ("For Lesson 23 its a 3 button press these are difficult so
       whenever there is a 3 button or 2 buttons and a knob we should have a card that warns them and suggests a
       strategy"). Found in the course, by the lessons' `Do` lines and words:
       - lesson 17, steps 12 and 13: hold `[SEQ]`, hold `[12]`, press `[OCT+]` (a fill note);
       - lesson 23, step 6: the same move, on a key of the learner's choosing (the one the tester means);
       - lesson 28, steps 10 and 11: hold `[SEQ]`, hold a box, turn `[KNOB2]` (a hard hit);
       - perhaps lesson 17, step 18: `[ARP]` + `[KNOB1]` sets how fast a roll goes, and hearing it while turning
         means a drum key held too;
       - not counted: lesson 22's three keys held together (a chord, one hand), and moves of two.
       Suggested, one card before each, the same shape every time, measured: `_Three at once / Hold [SEQ], hold
       [12], / then press [OCT+]. // Use both hands. Keep / the first two down.` (lesson 23: `hold a key`; lesson
       28: `Hold [SEQ], hold [4], / then turn [KNOB2].`). Lesson 17's is chosen with item 116 (its step 11), and
       its step 18 is split into two moves of two there, so it needs no card. Lessons 23 and 28 take the same card,
       in the same shape, before their moves.
  117. **Lesson 27, step 12: name [GLO] again.** The tester: "Wait for it then [5] again. I think this should be [Glo]
       + [5]". Today `Wait for it ... / then [5] again.`: the learner has let go of [GLO] since step 11, and [5]
       alone plays a drum. **Chosen:** `Wait for it ... / then [GLO] + [5].` (2 lines, as today; measured.)
  118. **The FM-1 remembering the lesson after it is switched off** ("how hard is it for us to save what lesson they
       are on. so if they turn off the FM-1 it will come back on the lesson they were on"). Today the lesson
       number lives in RAM only (`C.les`, `fw/course.inc`), and after a power-off START begins at lesson 1. Looked
       at, nothing built:
       - **Not through the project.** HOME's settings are kept in the project (`x.home`, 80 bytes, some free), but
         the project is written only when it is saved: keeping the lesson there means saving the lesson's song over
         the learner's own saved song.
       - **Next to the lessons instead.** In the sample slot that holds the pack, the rest of the pack's last 4 KB
         sector is blank (about 3 KB after today's pack). Blank flash can be written a byte at a time without
         erasing anything: each lesson's start writes its number into the next blank byte, and at START the FM-1
         reads the last one written. The learner's song is not touched. Sending lessons again rewrites the slot, so
         the course starts from lesson 1 again.
       - **The cost:** a new firmware, perhaps 70 to 100 bytes of code, and 92 are free: it may need room made
         first (the start card's words could move into the pack, as the course's card did). The write runs the way
         the editor's sample writes run, while the music plays; the simulated FM-1 can test it, power cuts
         included.
       Set for its own session, on 2026-11-25 ("I will have more fable tomorrow at noon").
- **2026-11-24: the third round made** ("All of my notes are in"), items 108 to 117, in eleven lessons:
  - lesson 1: the card `_Your FM-1` after `Good.` (21 steps);
  - lesson 5, step 14; lesson 8, step 13 (`_Bass and kick`); lesson 9, steps 8 and 9; lesson 14, step 9 (a strip
    at the top, one line); lesson 15, step 5; lesson 27, step 12;
  - lesson 16: steps 2, 3, 4, 5, 7, 11 and 12, and the card `_Why copy a page?` after step 4 (17 steps);
  - lesson 17: steps 4, 5, 6, 7, 10, 11 (`_Three at once`), 18 (the roll's speed as two moves of two) and 20;
  - lessons 23 and 28: the card `_Three at once` before their three-at-once moves (14 and 18 steps).
  - No step of these lessons jumps to a number, so the steps after a new card were simply renumbered.
  - In the notes and the lessons' tester notes the tester is now "they" everywhere (some places said "he").
  - The chip ("I think we might want to add something that says I have never tested this on a new chip--mine has
    the old chip. the developer for Sloop says something similar"): a note in the installer page's first step,
    after the beta note, and in the README's warning: `Tested on the original flash chip only. Teachie Beachie has
    been tested on one FM-1, which has the original flash chip. FM-1s sold since fall 2026 have a newer chip, the
    UCUN UC25HQ80, and neither Teachie Beachie nor SLOOP 3 has been tried on one yet. If yours is one of them, you
    would be among the first: keep the backup this page makes.` SLOOP's own note on the chip is in step 2, seen only
    by those who set up m-bootloader. Then "one FM-1" became "my FM-1", in the author's voice ("can you just change
    the wording to my fm-1 instead of one FM-1"). And: "have a newer chip" became "can have a newer chip", and the
    page's ": keep the backup this page makes" came out (the tester's own edits). A stronger note, warning of the
    one new-chip FM-1 that did not start after m-bootloader was installed (m-bootloader's release notes, 1.3 to
    3.0, which still list the chip as not tested), was offered and not wanted.
  - The page installed on the real FM-1 and sent the lessons ("It is installed and it says the lessons are on it").
    Then: "I don't like how it says installed before the lessons are on there. I basically want to give it less of
    an emphasis on success until the lessons are one there", and of the step's words "the peach logo, then a card
    that asks 'Ready for a lesson?' OCT+ starts a lesson, OCT- goes straight to SLOOP 3. One step is left: the
    thirty lessons, just below. can we remove that". Step 6 now has two states:
    - before the lessons: no check mark, `One step left: the lessons`, `The firmware is installed. Now send the
      thirty lessons: it takes a few seconds.`, the two things to do and the button; SLOOP's tips, its links and
      "Coming from SLOOP 2.8?" wait; the step is still the current one in the list at the left (SLOOP's page marked
      it done when the install ended);
    - once the FM-1 finds the lessons: the check mark, `Teachie Beachie is ready`, `The thirty lessons are on your
      FM-1. Turn MASTER up gently, and have fun.`, what was sent, then the tips and links; the step marked done.
    `tests/lessons_sim.js` checks that it says ready only then. Looked at in a browser, both states.
  - **Public** ("ok lets make my github repo public and turn on github pages so I can send it to my friend"),
    after a full install from the page on the real FM-1. The page is `docs/index.html` (and `docs/.nojekyll`, so
    GitHub serves the folder as it is); `fw/build.sh` copies it there at each build. GitHub Pages serves `docs/` of
    `main` at davydegravy.github.io/teachie-beachie-fm1. SLOOP's own page, `index.html` at the top, is left as it
    is. The issue forms were still SLOOP's; then ("yes make the bug report form go to Teachie Beachie") they are
    Teachie Beachie's own: a bug report (where, lesson and step, what happened, the screen's words, the flash chip,
    the version, the computer), an idea, and a help link to the page's own help. Nothing points people with an old
    SLOOP 3.3.7 to SLOOP's issues.
  - **All twelve screens on a phone** ("on Mobile it hides the first six screenshots. do we have to do that? I'd
    prefer if we didnt"). On a phone and a tablet the six beside the words now come under them, in one grid with
    the six below, in the lessons' order (two across on a phone, three on a tablet). Looked at in a browser at the
    three widths.
  - The tester on what goes public: "Whatever I'm fine with those notes. and my github is davydegravy so david is
    fine." So the history is left as it is: the old commits keep the trip's lines and the home folder's path.
  116. **Lesson 17, the whole lesson** ("Lesson 17 is the lesson that I remember being really rough ... I'd like to go
       over all of Lesson 17 like we just did for 16"). The fill note was rebuilt on 2026-11-20 (steps 10 to 15);
       what is still hard, from its screens: the same keys mean three things by the button held ([GLO] + [11] a
       fill while held, [GLO] + [12] a fill for a bar, [SEQ] + [12] + [OCT+] a fill note); "aim for the end of the
       bar" gives nothing to aim at; step 10's `play [11]` comes right after [GLO] + [11] meant a fill; and the
       roll's speed (step 18) is three at once if it is to be heard while turning. Suggested, all measured:
       - step 4: `This beat hides four / snares, on [13] to [16]. // They only play / in a fill.`
       - step 5: `Hold [GLO] and hold [11]: / the snares play.`
       - step 6: `Again, when the light / reaches [13].`
       - step 7: `[GLO] + [12] is easier. // It plays the fill for / one whole bar, then / stops by itself.`
       - step 10: `Let go of [GLO]. / Pick the high tom: [11].`
       - step 11, the three-fingers card, as item 115: `_Three at once / Hold [SEQ], hold [12], / then press
         [OCT+]. // Use both hands. Keep / the first two down.`
       - step 18, two moves of two in place of one of three: `Speed: [ARP] + [KNOB1]. / Hear it: [ARP] + [5]. /
         [OCT+] when done.`
       - step 20, a heading as the other cheat sheets have: `_Fills and rolls / Fill: [GLO] + [11] or [12]. /
         Fill note: / [SEQ] + a key + [OCT+]. / Roll: [ARP] + a key.`
       - unchanged: steps 2, 3, 8, 9, 12, 13, 14, 15, 16, 17, 19.
       **Chosen:** all of them ("Yes, all of them").
- **2026-11-21, lesson 12 on the real FM-1: "Again" once seemed to go to the end.** At `Keep them?` (step 13), the tester
  saw `Press [PLAY] to stop` and then the last card, which is the Keep path; Again should go back to `Press [REC]`
  (step 9). Not reproduced on the simulated FM-1: Again, and Replay on the Done card, worked every time, and every
  lesson replayed from its Done card showed all its steps again (scratch scripts `again.py`, `sweep.py`). Working on
  the FM-1 again afterwards; perhaps the other button. Left as it is ("lets not worry about it").

## 2026-11-24 to 27: the move to SLOOP 3.4.2 (a trial, on its own branch)

SLOOP went from 3.3.7 to 3.4.2 and changed pages the lessons stand on. The move is a trial on the branch
`trial/sloop-3.4.2`: nothing of it is on `main`, on the published page or on a real FM-1. Its own record is
`tutorial/UPGRADE-3.4.2.md`: every step, what it cost in the firmware's room, how it was checked, what is not
checked. The course's author's answers, in their own words, are in `tutorial/upgrade/decisions.md` (1 to 13). This
entry only says what changed for the course.

- **What SLOOP changed under the lessons.** A pattern only changes when it is saved, with [SAVE] held and its key
  held; a chain is several pattern keys in one hold of [SAVE]; the song is recorded ([SAVE] + [14]) and has a SONG
  page of lines; the project is saved on PROJECT 3/3; the drum grid has four dials (SOUND, STEP, HIT, LEVEL); the
  pages behind [FX], [EDIT], [LFO], [ARP] and [ENV] are other pages, each with its name in the bottom corner.
- **The lessons: all thirty gone through again,** in the course's four parts, each lesson shown whole as Today /
  Suggested after it had played both ways on the simulated FM-1 (`tutorial/UPGRADE-3.4.2.md`, "Step 4: the
  lessons", part by part). First asked of each: what the learner needs it for now. Lessons 16 and 25 have new names
  (Eight patterns, Chain the patterns). The learners' words from the three rounds above still stand where the step
  still exists.
- **New for a learner, in the firmware:**
  - **The framed names answer** (decision 11): a button's or a key's name on a card fills in while it is held; a
    knob's fills as far as the step's turn is done, from the side the turn comes from.
  - **A black key has a picture** (decision 12): `[BLACK 2]` in a lesson file.
  - **The lesson is remembered** over a power-off (item 118, decision 9).
  - **The screen's colors on newer FM-1s** (decision 8): M-VAVE's own start-up table, sent again.
  - **The logo** is drawn from the lessons' slot, in full color (decision 1), and since 2026-11-27 it is the new
    round one, the name over a sunset on the sea ("I want you to change to this logo"): on the start screen, on
    the page and in the README. The peach was its stand-in.
- **Left out to make room** (decision 1): SLOOP's visualizer, and what SLOOP 3 brings over from SLOOP 2.8 at its
  first start.
- **2026-11-27, the words around the course** (decision 13):
  - **All the pictures remade** ("I think we need to update all of the pictures"): the thirty sheets of the lessons'
    screens, the twelve screens on the page and in the README ("Yes, use them"), the start screen. Five old
    pictures in `tutorial/` that nothing linked to are removed ("yes remove them").
  - **The README and the page say SLOOP 3.4.2, version 0.2** ("Yes, 0.2"), and what is new: the names answer, it
    remembers.
  - **No paragraph for someone with an earlier Teachie Beachie** ("don't say coming from an earlier teachie beachie
    paragraph we don't need that").
  - **Nothing about SLOOP 2.8** on the page or in the README ("Lets remove anything about Sloop 2.8 those people
    are unlikely to use this. they need to flash, I don't think they are going to expect to bring anything over").
    The closed note for someone who has SLOOP 3 stays ("Keep it").
  - **The guides for an AI assistant** (`AGENTS.md`, `tutorial/AI.md`) name what changed ("Yes, all of it").
  - **The page's top says why the lessons are inside the FM-1** ("can we add some of this without making it too
    long?", of a draft from another AI assistant): `Because the lessons run inside the FM-1, they can do what a
    video or a manual can't. They know which page you are on, which notes you put in and which button you are
    holding, so a lesson moves on when you have done the step.` Three trims keep the text beside the six screens
    ("right now the pictures and text are perfect"; "Three edits"); the last paragraph is now `I made Teachie
    Beachie as a gift to my kids. If you have kids who would like to learn how to use a groovebox and the FM-1,
    I'd encourage you to give it a try.` ("we can shorten it from I made this as a gift to my kids").
  - **The page tried in a browser with a pretend FM-1:** the lessons sent to an empty slot, to a full FM-1 after its
    question, and each thing that can be wrong said in its own words.
- **Still to do:** a real FM-1 (decision 10), after one run on Linux. A newer FM-1, for the screen's colors, is with
  the course's author around late December 2026.

## 2026-11-27: the trial on the real FM-1 (commit `719e837`)

The course's author installed the trial from its page (served on their computer) on their own FM-1, the original
screen and chip, and is going through the lessons. Notes as they come, nothing changed yet:

1. **Lesson 3, boxes and beats** ("in lesson 3 it tells you to put snare on 5 and 13, but then it tells you the
   snare is on beats 2 and 4. I think the 5 and 13 are incorrect right?"). They are the same places: box 5 begins
   beat 2 and box 13 begins beat 4. Step 9's card says `Every 4 boxes is a beat.` but never which boxes those beats
   are, and the two numbers read as a contradiction even to the author. To reword: name the boxes with the beats.
2. **Lesson 3 sounds different from the same lesson on 3.3.7** ("It sounds pretty different from before maybe its
   just the default kick"). SLOOP's own notes give the likely cause: 3.4.1 evened out the kits' snares, claps and
   hats and brought every sound to the same loudness, and 3.4.2 made the whole mix 6 dB louder. Not yet checked
   on the FM-1 whether a kick was tapped away as well. Heard on a child's headphones ("kid headphones not high quality"), which
   carry little bass: to hear again on the usual ones before calling it a change. On headphones SPEAKER LOWCUT
   (HOME held, AUDIO) belongs OFF: ON, it cuts the lows on the jack too.

3. **Lesson 5, the mutes: name [GLO] each time** ("when we are muting the tracks it just tells you the number like
   2, 3, 4. These should be [Glo]+ 2, [Glo]+ 3, [Glo]+ 4"). Steps 6, 7 and 8 say `[2] is the lead`, `[3]: the
   chords`, `[4]: the pad`; the hold is only named in steps 4 and 5. As item 117 said of lesson 27. Measured, two
   lines each:
   - step 6: `[GLO] + [2] is the lead: / the tune. Mute it.`
   - step 7: `[GLO] + [3]: the chords, / a piano. Mute them too.`
   - step 8: `[GLO] + [4]: the pad. / Soft, long notes. Mute it.`
   And the solo's way back, step 13 ("for the solos when the you are turning the solo off it says [10] it needs to
   be [Glo]+ [10]"): `[10] again: / everyone back.` to `[GLO] + [10] again: / everyone back.`, measured, two lines.
4. **The song's mix on SLOOP 3.4.2** ("wow I do not like how this sounds, the drum dominates and I can barely hear the
   bass"), in lesson 5, where the whole band plays, on the same child's headphones. HOUSE ONE's track levels
   (`tutorial/songs/house-one.json`: bass 100, lead 92, chords 78, drums 52, pad 104) were set by ear on 3.3.7; since
   then SLOOP evened every sound's loudness and the kits' snares, claps and hats (3.4.1) and made the mix 6 dB
   louder with its peaks limited (3.4.2). To measure on the simulated FM-1, each track alone, old against new, and
   to hear on other headphones; then the song's levels set again if it is the mix. The levels are the song's, in
   the lesson pack: no firmware.

5. **The drums drown the band: measured, and it is the new SLOOP** ("yeah the next step it says to play along with
   the keys but the drum is so powerful I can't hear the keys at all"). Each track of HOUSE ONE alone, as lesson 5
   sets it up, played by the simulated FM-1 of each version and its sound measured (RMS, dB below full scale):

   | | 3.3.7 | 3.4.2 | change |
   |---|---|---|---|
   | drums | -38.9 | -26.4 | +12.5 |
   | bass | -48.4 | -46.8 | +1.6 |
   | lead | -49.6 | -46.0 | +3.6 |
   | chords | -46.2 | -45.6 | +0.6 |
   | pad | -49.6 | -49.7 | -0.1 |
   | the whole band | -37.2 | -26.2 | +11.0 |

   The bass was 9.5 dB under the drums and is now 20.4 under. Nothing of ours changed: the song's levels are the
   same numbers. SLOOP's kits are that much louder against its synth sounds since 3.4.1 and 3.4.2. A track's level
   is plain (half the number, 6 dB less); the drums alone come out at -37.2 dB at level 30, so about 31 in place of
   the song's 104 gives the old balance back. Lesson 14 sets every track to 100 by its own Setup and would need its
   drums lower in the same way.

6. **A scratchy sound on the drum grid, on the real FM-1** ("oh yeah I can hear the scratchy sound in the drum
   grid--it is bad"). Two causes are possible and not yet told apart: SLOOP 3.4.2's own drum grid, which reads the
   whole pattern 256 times a frame so that the sound comes late (what SLOOP 3.4.4 repairs; our strips on that page
   may add to it), or the drums of item 5 driven into the mix's limit. To ask: with the same beat playing, is it
   gone on the main screen? Gone there: the page. Still there: the loudness.
   **Answered: "its the page".** So it is SLOOP 3.4.2's drum grid, the page lessons 2 to 7 are taught on, and the
   Teachie Beachie on 3.3.7 did not have it. The trial should not go out on 3.4.2: it needs the SLOOP release that
   has 3.4.4's repair without 3.4.4's crashes (`tutorial/UPGRADE-3.4.2.md`, "News looked up on 2026-11-27").

7. **Lesson 8 on the real FM-1: "did not go well"** ("I don't liek same on 7,11 and 14, I think we should do each
   step with the user so it should say use knob1 to step 7 [3], etc etc.the hint says use knob1 to go to the step
   but it doesn't tell you the step & the directional knob fill seems incorrect. This is a bad experience for me it
   would be really bad for a kid"). Three things:
   - **Step 11 asks for three notes at once** (`The same on 7, 11 and 14.`). To be one step for each note, each
     naming its step, as step 10 does (`[KNOB1] to step 4: [3].`).
   - **Its hint names no step** (`[KNOB1] to the step, / then play [KEY 3].`). Each hint to name its own.
   - **The knob's frame fills wrongly here, and the cause is in how it is built.** On these steps the lesson cannot
     read the STEP dial (the FM-1 does not tell the lesson its cursor: `tutorial/lessons/README.md`), so the frame
     counts clicks of the knob against the clicks the step expects: all three turns of step 11 added up (7), and
     right less left. On a real knob that is not the way to the step: turned quickly it moves more than a step a
     click, a learner who overshoots and comes back empties the frame while getting nearer, and one who starts from
     another step needs other clicks, perhaps the other way. The robot on the simulated FM-1 always turned exactly
     the clicks written, so no check showed it. Two repairs: with words only, leave the turn out of these steps'
     Do (the frame then stays plain, the key still blinks when STEP is on its step, the robot's turn goes in
     `Robot:`); or, in the firmware, let the lesson read the STEP dial, so the frame fills as STEP nears its step
     from either side. The same holds wherever a step counts clicks (lessons 11, 12, 13, 22, 26).
   - **And step 14** ("for knob1 to step 16 the directional button isn't working"): its Do is one click, and by the
     rule of step 6 one click fills nothing, so the frame stays empty however far the knob is turned. The rule was
     meant for a step that is over at the click; this one goes on until the key is played, and from another step
     it takes more than one click. The same root: clicks counted in place of the dial read.
8. **The sound of SLOOP 3.4.2 as a whole** ("and the sound is just bad. I really hope he fixes the sound, this is so
   much worse than the last Sloop version"). So far two causes are known, items 5 and 6: the kits about 12 dB
   louder against everything else, and the drum grid's crackle. The first is ours to balance in the song; the
   second is SLOOP's, repaired in 3.4.4. Whether more is wrong is to be heard again once both are out of the way.

**Made the same day, before the play-through went on** ("I think I want to make those changes and get them on my FM-1
before continuing"), each lesson shown as Today / Suggested after it had played both ways on the simulated FM-1:

- **The song's drums: level 104 to 31** (item 5; heard first as three sound files from the simulated FM-1: "yeah it
  sounds better with the drums lowered").
- **Lesson 8** (item 7), "Yes, all of it"; **lesson 3** (item 1), "Yes, both"; **lesson 5** (item 3), "Yes, all of
  it"; **lesson 14**, its own drums from 100 to 30 and one strip, "Yes, use it". What each says now is in its file,
  under Tester notes.
- **Checked afterwards** on the simulated FM-1, by a helper on a cheaper model, its logs read: all thirty lessons
  read and measured; the six upgrade checks that play lessons or start the FM-1; and the five lesson runs, 30 of 30
  played through in each. The firmware is unchanged: only the lessons were sent again. The thirty sheets of the
  lessons' screens are the ones this run made.
- **Not made:** the knob's frame following the STEP dial (firmware); the drum grid's crackle (SLOOP's, item 6).
- **Still to redo:** the page's picture of lesson 1 (`assets/shots/lesson-01.png`), where the drums' level bar is now
  shorter.

## To do next

The order of work is in `PLAN.md`, section 13.
