# The lessons

Thirty lessons, one Markdown file each. They are first drafts: every one has been played from its first card to
its last on the simulated FM-1 by a robot learner, and none has been played by a person yet.

## Going through them on the FM-1

1. Plug the FM-1 into the computer and switch it on. Close Chrome pages that talk to the FM-1 (SLOOP's editor,
   the installer).
2. Start the waiting program, from the repository's top folder:

   ```bash
   tutorial/.venv/bin/python -u tutorial/run.py --fm1 --wait
   ```

   It keeps running until you stop it (Ctrl+C), and finds the FM-1 again after it has been off.
3. On the FM-1's start card press **OCT+** (START). The next lesson's first card comes up.
4. On that card:
   - **OCT+** (BEGIN) starts the lesson.
   - **OCT−** (LESSONS) opens the list. Turn **SELECT** to choose a lesson, then **OCT+** to begin it, or
     **OCT−** to go back to the start card.
5. At the end of a lesson, **OCT−** plays it again and **OCT+** goes on to the next one's first card.

To start from a particular lesson, put its number in the command: `tutorial/run.py 5 --fm1 --wait`.

**Where you are is remembered** in `tutorial/progress.json`. Each run's log goes to `tutorial/logbook/`, and a
copy of what was on the FM-1 before each lesson goes to `tutorial/backups/`.

**A lesson replaces what is on the FM-1.** It starts a new project and loads its own song. The copy in the
backups folder is the way back.

## On the FM-1 alone, with no computer

The same lessons can run on the FM-1 by itself. The firmware has its own lesson player, and the lessons are packed
into one of the FM-1's four sample slots.

**Putting the lessons on the FM-1** (once, and again whenever a lesson file changes), with the FM-1 switched on
normally and plugged in:

```bash
tutorial/.venv/bin/python tutorial/device.py --install
```

It uses the last empty sample slot, and says which. Add `1-15` to put only lessons 1 to 15 on.

The firmware has to be on the FM-1 first (`build/fw/installer-fm1.html`, opened in Chrome), and the lessons and the
firmware have to come from the same build. On a computer that does not have `tutorial/.venv` yet, make it once:

```bash
python3 -m venv tutorial/.venv && tutorial/.venv/bin/pip install mido python-rtmidi
```

**Taking a lesson:** press **OCT+** (START) on the start card. With no computer answering, the FM-1 begins its own
course after a second. Everything else is as above: BEGIN, LESSONS and SELECT, REPLAY and NEXT.

**Leaving a lesson:** hold **OCT−** and **OCT+** together for two seconds. The lesson's first card comes back.

**What is different without a computer:**
- The FM-1 does not remember which lesson was next once it is switched off. Use LESSONS to go to it.
- SHOW ME does the step directly (the notes appear, the page opens); it does not press the buttons one by one.
- There is no log, and no copy is kept of what was on the FM-1 before a lesson.
- If the lesson program on a computer is running and the FM-1 is plugged in, the computer leads, as before.

## Making notes

Each lesson file ends with a `## Tester notes` section. Write there, in any form. The section is for people only;
the lesson player doesn't read it.

Each file also has a picture of the lesson's screens, in order, taken from the robot's run.

## The thirty

### Part 1: your first beat

| # | Lesson | The one idea |
|---|---|---|
| 1 | [Hello](01-hello.md) | A blinking light means "press this". SLOOP has five tracks. |
| 2 | [Sixteen boxes](02-sixteen-boxes.md) | A sequencer is a row of boxes and a walking light. |
| 3 | [Kick, snare, hat](03-kick-snare-hat.md) | A beat is three simple layers. |
| 4 | [The drum kit](04-the-drum-kit.md) | A kit has 16 drums to pick from, and a new kit swaps them all. |
| 5 | [Meet the tracks](05-meet-the-tracks.md) | Each track plays its own instrument. |
| 6 | [Faster, slower, oops](06-faster-slower-oops.md) | Tempo, undo and save. |
| 7 | [Your own beat](07-your-own-beat.md) | A beat made alone. |

### Part 2: notes and chords

| # | Lesson | The one idea |
|---|---|---|
| 8 | [A bass line in boxes](08-a-bass-line-in-boxes.md) | The other tracks put notes in boxes. |
| 9 | [Pick your sound](09-pick-your-sound.md) | Each track can play any instrument. |
| 10 | [Record it live](10-record-it-live.md) | REC writes down what you play. |
| 11 | [No wrong notes](11-no-wrong-notes.md) | A key is a family of notes that fit together. |
| 12 | [One-finger chords](12-one-finger-chords.md) | A chord is several notes at once. |
| 13 | [The piano roll](13-the-piano-roll.md) | A note can be moved, stretched or removed. |
| 14 | [Mix the tracks](14-mix-the-tracks.md) | If everyone shouts, nobody is heard. |
| 15 | [Your own loop](15-your-own-loop.md) | A loop made alone. |

### Part 3: effects and sound

| # | Lesson | The one idea |
|---|---|---|
| 16 | [Eight patterns](16-eight-patterns.md) | A song is made of patterns, and there are eight. |
| 17 | [Fills and rolls](17-fills-and-rolls.md) | A fill says "something new is coming". |
| 18 | [Play the effects](18-play-the-effects.md) | Some effects are played like notes. |
| 19 | [Echo and space](19-echo-and-space.md) | A delay repeats a sound; a reverb puts it in a room. |
| 20 | [Color a track](20-color-a-track.md) | An insert changes one track; a filter makes it dark or bright. |
| 21 | [Shape a sound](21-shape-a-sound.md) | A sound has a beginning, an end and a brightness. |
| 22 | [Wobble and arps](22-wobble-and-arps.md) | The FM-1 can move a sound, and play a chord note by note. |
| 23 | [Your own piece, live](23-your-own-piece-live.md) | A performance. |

### Part 4: make a song

| # | Lesson | The one idea |
|---|---|---|
| 24 | [Songs have parts](24-songs-have-parts.md) | Intro, verse, chorus, break. |
| 25 | [Chain the pages](25-chain-the-pages.md) | Pages played in a row make a song. |
| 26 | [Write it line by line](26-write-it-line-by-line.md) | A song can be written down and edited. |
| 27 | [Build-ups and drops](27-build-ups-and-drops.md) | Taking tracks away is as strong as adding them. |
| 28 | [Swing and feel](28-swing-and-feel.md) | The same beat can march or dance. |
| 29 | [Keep it and share it](29-keep-it-and-share-it.md) | A finished song deserves to be kept. |
| 30 | [The final test](30-the-final-test.md) | A whole song from nothing. |

## What is true of all of them for now

- **Every lesson uses HOUSE ONE.** Each file says which style its song is meant to have in the end, and what in
  the lesson is tied to the song (usually a few box numbers). Swapping the song later means changing those lines.
- **HOUSE ONE has not been approved by ear yet.**
- **The lessons run by themselves.** An AI assistant beside them (`../AI.md`) finds what it should know about a
  lesson in the file's "Notes for the AI assistant".
- **A few steps count clicks instead of checking the result,** because the FM-1 cannot yet tell the lesson its
  key, scale, chord, arpeggiator or cursor. Lessons 11, 12, 13, 22 and 26 say where.
- **The path for people who know grooveboxes** leaves out the steps marked *(beginners only)*. So far only a few
  steps are marked. It is started with `--advanced` on the command, not from the FM-1.

## How a lesson file is written

A lesson is meant to be read by a person first. The player reads only two things in it: the list at the top, and
the numbered list under `## Steps`.

```markdown
# Lesson 2: Sixteen boxes

- **Part:** 1, your first beat
- **The idea:** a sequencer is a row of boxes and a walking light.
- **The learner does:** ...
- **Got it when:** ...
- **Song:** ...

## Steps

1. **Setup:** song HOUSE ONE section C, 1 bar; drums empty; track bass
2. **Card:** A sequencer is a row of boxes. / A light walks along them.
   - **OCT+:** Next
3. **Strip:** Now hold [SEQ] and tap [KEY 1].
   - **Do:** hold SEQ, tap box 1
   - **Until:** drums has KICK on box 1
   - **Hint:** Keep holding [SEQ] and tap the blinking key.
```

### A step

One numbered line: the kind of step in bold, then its words. In the words, ` / ` (a slash with a space on each
side) starts a new line on the screen; a slash inside a word, as in `LFO 1/3`, is printed. A button's or knob's
name in `[ ]` is drawn in a thin border, as the cards say it (`A name with a border is a button or knob`). A line
that starts with `_` is a heading: small, and underlined (`_Repeat after me`); on a Big card, a line that starts
with `^` is small without the line. The mark itself is not drawn.

**A black key** is written `[BLACK 2]`: the second black key from the left (there are eleven). The FM-1 draws a small
picture of a black key between two white ones, and the number, in the border; a white key stays `[5]`. On SLOOP
3.4.2's drum grid a black key picks the drum: `[BLACK 1]` the kick, `[BLACK 2]` the snare, `[BLACK 5]` the hat. The
first card that uses the picture says what it is. The picture is one letter of the lessons' font
(`tools/mklessonfont.py`), measured like any other; an FM-1 with no lessons on it, led by a computer, shows the words
`[BLACK 2]` in its place.

Two more marks, for a card (2026-11-22):
- **A gap, ` // `** (two slashes, a space on each side): a new line with a little air before it, where a second
  thought begins (`This tune has / one wrong note. // No need to start over.`). The FM-1 draws it 8 px high, and 16
  on a card of 4 lines or fewer; it is not one of the card's lines. With its gaps a card has to end above the
  answers: 7 lines take one gap, 6 take two. `check.py` says when it does not fit.
- **A line in the middle, `~`** at its start (`~Each lesson teaches`): centered, as a big card's lines are.

The last card of a lesson is a cheat sheet where there is something to look up: `Tempo: [SELECT].`, one line for
each thing, under an underlined heading when it has room.

**The words are written as sentences,** as they will show: the FM-1 draws the lessons in a font of their own, with
small letters (Google Sans SemiBold, kept in the lesson pack). Capitals are for three things only: a name in a border
(`[PLAY]`, `[KEY 12]`), a word quoted from SLOOP's own screen (`until FILT says OFF`, `the SONG page`, `the top says
COPY`), and names (SLOOP, FM-1, A minor). A line goes on in small letters after a colon. The four knobs are `[KNOB1]`
to `[KNOB4]`, with no space, as the FM-1's own labels have them (a key has no label: `[KEY 12]` keeps its space); in
Do, Until and Robot they stay `KNOB 1` to `KNOB 4`. One shout is allowed: lesson 1's `Faster / Faster / FASTER`.
A step marked `*(beginners only)*` is left out on the path for people who know grooveboxes.

| Kind | What the learner gets |
|---|---|
| **Card** | A full peach screen of words: an idea, or a question. About seven short lines. |
| **Big** | A few words, large and centered. |
| **Strip** | A band of words along the bottom, over SLOOP's own screen, which stays live. Three lines at most. **Strip top** puts it along the top, for when the dials at the bottom matter. **Strip corner** is a small tag in the top left corner, as wide as its words, for a step where the learner needs the whole screen (lesson 30). |
| **Reply** | The dry voice: a word or two, large, for a moment. `GOOD.` |
| **Setup** | Nothing to see. The FM-1 is made ready: a song, an empty track, a page. |
| **Wait** | Nothing new for a while: `4 seconds`, `2 bars`, `until bar 9`. |
| **Play** / **Echo** | The FM-1 plays a phrase, lighting each key. / The learner plays it back, key by key. |

### The bullets under a step

| Bullet | What it means |
|---|---|
| **OCT−** / **OCT+** | An answer on that button. After an arrow `→`, what it does: `go to 4`, a setup such as `drums HAT on 1 5 9 13`, or `right` / `wrong` for a quiz. On a strip, the words have to name the button, because a strip shows no answers. |
| **Do** | What the learner does. Its buttons and keys blink. |
| **Until** | How the lesson knows the step is done. It is read from the FM-1 itself, so any way of getting there counts. With no Until, the step is done when the Do has been seen. |
| **Hint** | Words shown when a step takes long (25 seconds, then each 25 seconds after). After the last one the lesson offers to do the move itself. With OCT− or OCT+ and an Until, the hint shows when the answer comes too early. |
| **Play**, **Echo** | As above, on a step that also has words. |
| **Follow** | Keys to play along with in time: one for each bar, blinking while its bar plays. |
| **Wait** | How long the step stays up. |
| **Keys** | `silent`: the keys belong to the lesson and play nothing. Used on the piano roll, where every key played is written: on its cards, and on steps that need no key (lessons 8, 13). |
| **Lights** | Keys lit faintly, as part of the picture. |
| **Page** | The page of SLOOP the step needs. If the learner wanders off, the lesson brings them back. |
| **Right**, **Wrong** | A quiz's replies. |
| **Robot** | For the tests only: what the robot learner does when the Do is not enough to go by. |

### The words of Do

`press PLAY` · `press SAVE twice` · `hold SEQ, tap box 1 5 9 13` · `hold GLO, tap box 6` ·
`hold SAVE, long box 2` (a key held about a second) · `hold SEQ, hold box 12, press OCT+` ·
`turn SELECT right 3` · `turn KNOB 2 left 5` · `tap A4 C5 E5` · `tap E4 or any key` · `let go` · `wait 2`

Buttons: `OCT-` `OCT+` `FX` `SEL` `ENV` `LFO` `EDIT` `GLO` `HOME` `SAVE` `ARP` `SEQ` `PLAY` `REC`.
Knobs: `SELECT` `ALGORITHM` `PRESETS` `KNOB 1` to `KNOB 4`.
Keys: a note's name from `F3` to `G5`, `box 1` to `box 16` for the white keys from the lowest up, or `black 1` to
`black 11` for the black keys.

**Which keys blink** when the step waits for notes (`Until: ... has ... on boxes ...`):

- **On the drums,** the boxes still empty: on SLOOP 3.4.2's drum grid a white key is a box. If another drum is picked
  than the boxes ask for, that drum's black key blinks instead, and the lesson says so.
- **On another track** there are no boxes on the keys: the piano roll writes the key played at the step its STEP dial
  shows. There the keys the Do names blink, and only while STEP is on a step that still lacks its note. So with
  `Do: turn KNOB 1 right 2, tap A3` and `Until: bass has notes on boxes 1 4`, the A's key waits until [KNOB1] has
  brought STEP to 4: a blinking key always means "press this". A knob has no light. Off the piano roll the Do's keys
  blink at once.

### The words of Until

Joined with `and`. Tracks are `bass`, `lead`, `chords`, `pad`, `drums`.

- **The transport:** `playing` · `stopped` · `recording` · `not recording` · `rec ready` · `song playing` ·
  `song recording` · `fill on` · `punch on`
- **Where the learner is:** `track is drums` · `page is STEPS` (SLOOP 3.4.2's pages too: `page is PROJECT`, `page is
  PATTERN`) · `drums picked SNARE` (the drum that boxes write: on the
  grid a black key or [KNOB1] picks it, on the tracks page playing it does)
- **What is written:** `drums has KICK on boxes 1 5 9 13` · `drums has no KICK on boxes 2 6 10` ·
  `bass has notes on boxes 1 4 7` · `lead has C5 on box 7` · `lead has nothing on box 13` ·
  `drums has 6 notes or more` · `lead is empty` · `lead longest note above 4.5` ·
  `drums has 1 fill notes or more` · `drums has 4 hard notes or more`
- **Pages and the song:** `drums on pattern 2` · `drums pattern 2 has notes` · `3 patterns or more` ·
  `4 tracks or more` · `song has 3 lines or more` · `song line 3 is pattern 3` · `song line 3 has 8 bars`
- **The mix:** `bass is muted` · `drums is solo` · `nothing muted` · `nothing solo` · `pad level below 46` ·
  `lead delay above 50` · `lead reverb below 30` · `master filter is 0` · `tempo above 135` · `swing above 45`
- **Sounds:** `lead sound changed` · `drums sound is HOUSE` · `bass insert is FUZZ` · `lead ATK above 50` ·
  `lead E4 below 45` (the sound's values by SLOOP's names; E0 to E7 are the eight on the OSC and FLT pages)
- **What the screen said:** `says PROJECT SAVED` · `says only PROJECT SAVED`
- **What was pressed:** `played 4 keys` · `pressed SAVE twice` · `turned KNOB 3`
- Any number can be tested with `above`, `below`, `is`, `is not`, `at least`, `at most`, or `changed`.

### The words of Setup

Joined with `;`.

- **A song:** `song HOUSE ONE section C` (a new project, then the section on page 1) · add `, 1 bar` for its
  first bar only · `patterns A B C D` (sections on pages 1 to 4) · `pattern 2: section C` ·
  `song lines 1 2 3 3 4 3` · `song mode on` · `new project`
- **Tracks:** `drums empty` · `drums 16 boxes` · `drums KICK on 1 5 9 13` · `drums fill SNARE on 13 14 15 16` ·
  `lead E5 on 1` · `fill empty tracks` · `lead sound FAT LEAD` · `pad level 40` · `lead delay 30`
- **The mix:** `mute lead chords pad` · `solo drums` · `unmute` · `master filter -70` · `tempo 100` · `swing 30`
- **Where the learner starts:** `track drums` · `pick KICK` (the drum that boxes write) · `page STEPS` ·
  `key A minor` · `play` · `stop` · `rec off`

## Checking a lesson after changing it

(These check the lessons as a computer plays them. `python3 tutorial/device.py` plays them on a simulated FM-1 that
leads itself, the way the FM-1 does with no cable; add lesson numbers for only those, and `--clumsy` for the robot
that slips.)

From the repository's top folder:

```bash
python3 tutorial/check.py 5
```

reads lesson 5 and names anything it can't understand, and any words that won't fit the FM-1's screen.

```bash
python3 tutorial/check.py --run 5
```

also plays it on the simulated FM-1 with the robot learner, and saves the picture of its screens. With no
number, both do every lesson.
