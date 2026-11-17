# Notes for the next session

Written 2026-11-17, after lesson 1 first ran on a real FM-1.

## Where things stand

- **The FM-1:**
  - It has the old flash chip (`0x856014`) and m-bootloader 2.4.
  - It runs the Teachie Peachie build: SLOOP 3.3.7 plus the guide.
  - The way back to SLOOP 2.8.5 is M-Flasher's RESTORE FULL FLASH with `fm1-20261117-113255.bin`, in
    `Documents/M-Flasher/backups`.
- **The Mac:**
  - Its two old MIDI names were changed to "FM-1 Update" and "FM-1 SLOOP 3".
  - The two MIDI packages are in `tutorial/.venv`.
- **Lesson 1 ran on the real FM-1 from start to finish,** in 63 seconds. The project that was on the FM-1 was
  put back afterwards, the same byte for byte.
- **Nothing is committed,** here or in the song library (`songs/house_one.py`, `songs/house-one.json`).
- **The GitHub repository is now `teachie-peachie-fm1`,** following SLOOP's own naming (`sloop3-fm1`). Still
  under the old name: this folder on the Mac (`boatswain-fm1`) and `PLAN.md`.
- **HOUSE ONE is not built in Ableton yet.** It was heard for the first time on the FM-1, in the lesson.

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
  songs would not fit there; one of the four user-sample slots (about 150 KB) would hold the whole course
  without cutting anything out of SLOOP. The lessons have to become data first, which was planned anyway.

- **Decided the same day:** keep building lessons on the computer for now. In the end the version without AI
  lives entirely on the FM-1, with no cable. The connected mode with Claude stays alongside it.

(More to be filled in.)

## To do next, in an order still to be decided

1. Change lesson 1 from the tester's notes.
2. Build HOUSE ONE in Ableton, approve it by ear, export it like the other songs.
3. Rename this folder on the Mac to `teachie-peachie-fm1`. (The work is committed and pushed, and the GitHub
   repository is renamed.)
4. Rewrite `PLAN.md` with everything decided since draft 2:
   - Claude leads, and the lesson never waits for it except in four cases.
   - The character is a musician with no mascot, dry replies, and detail on request.
   - The dark screen means Teachie Peachie is speaking.
   - No pictures on cards in the first version.
   - SLOOP 3 only for now.
   - A lesson simply starts, and skip leads to "I'm advanced" or a list of lessons.
   - The melody game is in.
5. Build the next pieces:
   - the half page;
   - the skip menu and lesson list;
   - `MORE`;
   - Claude cutting in;
   - lesson 2;
   - lessons as plain text files;
   - the web page.
6. Check on the FM-1 that the start screen shows TEACHIE PEACHIE under the sailboat.
7. Look for a reply on SLOOP's issue 10 (the Mac build fixes).
