# Teachie Beachie with an AI assistant

Teachie Beachie's lessons are plain text files, written to be read by people and by an AI assistant. Put this
project on a computer, open it in an assistant that can read files, change them and run commands, and you have a
tutor for the learner, a lesson writer for you, and a second pair of hands on the FM-1. Teachie Beachie itself was built and tested this way.

Any assistant of that kind will do. Nothing here needs a particular one.

## What you need

- **The lessons on the FM-1.** The installer page puts them there.
- **This project on the computer.** Download it from GitHub, or `git clone` it.
- **An AI assistant that works in a folder on your computer:** it reads and changes the files there and runs
  commands.
- **Python 3.** Asking about lessons needs nothing more. Changing lessons and sending them to the FM-1 needs a few
  small things, which the assistant can set up for you (see "What it sets up", below).

## Start

Open the project's folder in your assistant and say:

> Read AGENTS.md, then help me with Teachie Beachie.

[AGENTS.md](../AGENTS.md), at the top of the project, tells the assistant where everything is, how a lesson is
written and checked, and what it must ask you before doing.

## What you can ask

### About a lesson

The learner plays on the FM-1. You, or they, ask the assistant. It knows every lesson: each step's words, its
hints, what the step checks, and notes on what usually goes wrong.

- "In lesson 13, step 7, what am I supposed to do?"
- "Lesson 20 talks about a filter. What is a filter, in words a ten-year-old gets?"
- "Why does lesson 3 put the snare on boxes 5 and 13?"
- "We are stuck on lesson 16. Explain copying a page another way."

The assistant cannot see the FM-1's screen or hear it. Tell it which lesson you are in and what the card says.

### To change a lesson

- "The card in lesson 9, step 4 is confusing. Suggest three clearer ones."
- "Lesson 6 is too long. What could go?"
- "Add a hint to lesson 12, step 9."

The assistant changes the lesson's file, checks that every line fits the FM-1's small screen, and plays the lesson
through on a simulated FM-1 to be sure it still works from the first card to the last. Then, when you say so, it
sends the lessons to your FM-1.

### To write a new lesson

- "Write a lesson 31 where the learner makes a second beat for the chorus."
- "Make an easier version of lesson 21."

A new lesson is one more text file. It is checked and tested the same way, and goes to the FM-1 with the others.
The lessons share one song, HOUSE ONE, and there is room on the FM-1 for about half as many lessons again.

### To look at the FM-1, and to control it

With the FM-1 on its USB cable, the assistant can read what is on it: the tempo, whether it is playing, each
track's notes and sound, the mix, the song's lines. And it can control it: put a lesson's song on it, press play,
change the tempo, mute tracks, choose a sound, write notes into a pattern, light up a button, put a line of words
on the screen.

- "Look at my drum pattern. Why does it sound uneven?"
- "Show me where the snare goes: put it in and play it."
- "Set the FM-1 up the way lesson 20 starts, so I can try the filter again."
- "Mute everything but the drums and slow it down to 100."
- "Light up the button I should press next."
- "I broke my bass line. Put the song's own back."

It cannot hear the result, so it tells you what the notes and values say, not how it sounds.

Before it changes anything on the FM-1 it should say what it is about to do and wait for your yes. Before it
replaces the song on the FM-1 it saves a copy on the computer, and it can put that copy back.

## What it sets up

Ask the assistant to do these when they are needed; it should tell you before installing anything.

| For | What |
|---|---|
| Checking a lesson's words | Nothing more: `python3 tutorial/check.py 5` |
| Playing a lesson on the simulated FM-1 | A C compiler (gcc or clang), then `make build/twin` once |
| Sending the lessons to the FM-1, reading it or controlling it | Two MIDI packages for Python, in a folder of their own: `python3 -m venv tutorial/.venv`, then `tutorial/.venv/bin/pip install mido python-rtmidi` |

To send the lessons, the FM-1 is on and plugged in, running Teachie Beachie (not the menu with MIDI MODE):

```sh
tutorial/.venv/bin/python tutorial/device.py --install
```

It replaces the lessons in their sample slot and says how many lessons the FM-1 finds. Samples in the other slots
are not touched. Changing lessons never changes the firmware.

## Good to know

- **Check what it tells you on the FM-1.** An assistant can be wrong about a button or a page. The lesson files are
  what the FM-1 actually follows.
- **It runs commands on your computer, and it can change what is on the FM-1.** A good assistant asks before it
  installs something, sends lessons or touches the FM-1; `AGENTS.md` tells it to.
- **Keep the backup** the installer made. Sending lessons does not need it, but it is what brings an FM-1 back.
- **What works goes back into the lessons.** When an explanation lands, ask the assistant to make it a hint in the
  lesson. The next learner gets it with no assistant at all.
