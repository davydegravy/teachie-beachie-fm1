# Teachie Beachie: the tutorial

Teachie Beachie teaches the FM-1 and SLOOP 3 on the FM-1 itself. This folder holds the plan, the lessons, their
songs, and the programs that run them.

## Where to look

| | |
|---|---|
| `PLAN.md` | The plan: the goal, every decision, the course, the order of work. |
| `NOTES.md` | What was tried and what the tester noticed, day by day. |
| `AI.md` | Teachie Beachie with an AI assistant: asking about a lesson, changing one, writing a new one. `AGENTS.md`, at the top of the project, is what the assistant itself reads first. |
| `lessons/` | The thirty lessons, one Markdown file each. `lessons/README.md` says how to go through them on the FM-1, how to leave notes, and how a lesson file is written. |
| `songs/` | The songs the lessons use. So far: HOUSE ONE. |
| `run.py` | The waiting program: it waits for START on the FM-1 and plays the lessons. |
| `device.py` | The course on the FM-1 alone: puts the packed lessons on the FM-1, and tests them on a simulated FM-1 that leads itself. |
| `check.py` | Reads every lesson, measures its words against the FM-1's screen, and plays it on the simulated FM-1. |
| `tp/` | The connector to the FM-1 (`fm1.py`), the lesson reader and player (`lesson.py`), cards and the robot learner (`teach.py`), and the song reader (`song.py`). |
| `logbook/`, `backups/`, `progress.json` | Kept on this computer only: each run's log, a copy of what was on the FM-1 before each lesson, and where the learner has got to. |

## What works now

- **The guide, inside the firmware** (`fw/main.c`, the command `ED_GUIDE`):
  - a peach card with dark red words, in two sizes, and a strip over SLOOP's own screen, at the top or the bottom;
  - a button's name drawn in a thin frame;
  - lights on any button or key, at four levels or the double blink that means "press this";
  - buttons, keys and knobs the lesson takes for itself;
  - every press and turn reported back.
- **The start:** the logo on the cards' peach background, then the start card with START and JUST PLAY.
- **The connector** speaks to the simulated FM-1 and to a real one over USB. It reads what the FM-1 is doing:
  the transport, the tempo, the mix, each pattern's notes, each sound's values, the song, and what the screen
  just said.
- **Thirty lessons,** as Markdown files. Lesson 1 has run on the real FM-1. The others are first drafts, played
  through on the simulated FM-1 by the robot learner.
- **The lesson player:** cards, strips, questions on the OCT buttons, steps that are done when the FM-1 is in the
  right state, hints when a step takes long, an offer to show the move, phrases to play back, lights to play
  along with in time, a quiz, and the list of lessons on a lesson's first card.
- **The course on the FM-1 alone:** `tools/mkcourse.py` packs the lessons, the firmware plays them itself
  (`fw/course.inc`), and START with no computer answering begins it. All thirty play through on the simulated
  FM-1; none has been tried on a real one yet.
- **The song HOUSE ONE:** original music, five tracks, sections A to D.

## The songs

A song is a spec file in the same form as the FM-1 song library's, with a fifth track for SLOOP 3: sections A to
D, the notes of each, and each track's SLOOP sound. The library's `songs/house_one.py` writes the spec and a copy
of it lives here (`tp/song.py` reads it). A lesson names its song and section, so a song can be changed or
replaced without rewriting the lesson; each lesson file says what in it is tied to the song.

## Running it

From the repository's top folder.

On the real FM-1 (needs the two MIDI packages in `tutorial/.venv`):

```sh
tutorial/.venv/bin/python -u tutorial/run.py --fm1 --wait
```

On the simulated FM-1 (needs `build/twin`: `make build/twin`):

```sh
python3 tutorial/check.py                 # every lesson read and its words measured
python3 tutorial/check.py --run 5         # lesson 5 played by the robot learner; its screens saved as a picture
python3 tutorial/check.py --run           # all thirty, several at once (about four minutes)
python3 tutorial/preview.py house-one C   # a section of the song recorded through SLOOP 3's own sound engine
```

## Not built yet

- `MORE`, "I'm stuck" on both OCT buttons, and the skip menu on the FM-1.
- The FM-1 telling the lesson its key, scale, chord, arpeggiator and cursor. Until then a few steps count clicks.
- Recording the FM-1's sound to a file from a lesson.
- The other songs. Every lesson uses HOUSE ONE for now.
- On the FM-1 alone: remembering the next lesson after it is switched off, and SHOW ME pressing the buttons one by one.
