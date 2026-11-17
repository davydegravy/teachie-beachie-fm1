# Teachie Peachie

Teachie Peachie teaches the FM-1 and SLOOP 3 on the FM-1 itself. This folder holds the connector, the lessons
and their songs. `PLAN.md` was written when the project was still called BOATSWAIN and before several later
decisions, so it is due a rewrite.

## What works now

- **The guide, inside the firmware** (`fw/main.c`, the command `ED_GUIDE`):
  - a dark card with words, in two sizes;
  - a strip: a line or two over SLOOP's own screen, which stays live;
  - lights on any button or key, at four levels or the double blink that means "press this";
  - buttons, keys and knobs the lesson takes for itself, so they don't act while a question is up;
  - every press and turn reported back.
- **The start screen** says TEACHIE PEACHIE, with SLOOP 3 credited (`app/vis.c`).
- **The connector** (`tp/fm1.py`) speaks to the simulated FM-1 and, untried so far, to a real one over USB.
- **Lesson 1, HELLO** (`lessons/welcome.py`): the first card, PLAY starting and stopping a song, and the
  melody game.
- **The song HOUSE ONE** (`songs/house-one.json`): original music, four players, sections A to D.
- **A copy of the project that was on the FM-1** goes to `seachest/` before a lesson. The lesson's song then
  stays on the FM-1.
- **A log of each run** goes to `logbook/`.

## The songs

A song is a spec file in the same form as the FM-1 song library's: four tracks, sections A to D, the notes of
each, and each track's SLOOP sound. The library's `songs/house_one.py` writes the spec, its
`tools/build_song.py` builds it in Ableton Live, and a copy of the spec lives here for the lessons
(`tp/song.py` reads it).

## Trying it on the simulated FM-1

From the repository's top folder:

```sh
make build/twin
python3 tutorial/run.py welcome --twin
python3 tutorial/preview.py house-one C
```

- **The first run plays lesson 1.** A robot learner looks at the lights and presses whatever blinks. The run
  prints a log of every step and saves the screens in `build/tw/lesson/`. `lesson-1-screens.png` shows them.
- **The second records a section of the song** through SLOOP 3's own sound engine, as
  `build/tw/lesson/house-one.wav`.

## Not built yet

- A run on a real FM-1.
- Claude cutting in during a lesson.
- `MORE`, the skip menu and the lesson list.
- The half page (words on top, SLOOP's dials live below).
- The web page that plays lessons with no AI.
- The lesson files as plain text: lesson 1 is written in Python for now.
