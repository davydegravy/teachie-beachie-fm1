# Teachie Beachie: the plan

A course that teaches the FM-1 and SLOOP 3 from zero, on the FM-1 itself.

Draft 3, 2026-11-17, brought up to date on 2026-11-18 when all thirty lessons were drafted. It replaces draft 2
(written as BOATSWAIN) and takes in every decision since. What has been tried so far is in `NOTES.md`, and the
lessons themselves are in `lessons/`.

## 1. The goal

**A 10-year-old beginner finishes the course able to make their own songs on the FM-1, with the tutorial
switched off.**

That means they can:
- make a drum beat from an empty track;
- add a bass line, chords and a melody on the other tracks;
- pick and shape sounds;
- use the effects, both the ones that stay on and the ones you play;
- build several patterns and join them into a song with a beginning, a middle and an end;
- save their work and fix their own mistakes.

**The final test is simple:** an empty project, no lit buttons, no hints. The learner makes a whole song and
saves it.

**The author plays every lesson first,** as a real beginner with a new FM-1. What goes wrong is fixed before a
child sees it.

## 2. What has been decided

- **The name is Teachie Beachie,** for the project and its logo only (Teachie Peachie until 2026-11-20). The
  repository is `davydegravy/teachie-beachie-fm1`, named the way SLOOP's own are.
- **SLOOP 3 only, for now.** The course uses as much of SLOOP 3 as it can. Other FM-1 firmware is left for
  later (section 15).
- **The course is taken on the FM-1 itself, with no cable** (section 3). An AI assistant can sit beside it on a
  computer, as a tutor and a lesson writer. It is any assistant that works with the project's files, and it is
  asked: it does not cut in (section 9).
  - Until 2026-11-22 the plan had two ways to take the course, one of them led from the computer with an
    assistant cutting in. The lessons were built that way first. The program that leads from the computer stays,
    as a tool for building and testing lessons.
- **Keep it simple.** A feature that adds a way to fail needs a strong reason. An assistant teaches; it doesn't
  compose.
- **The lesson never waits for an assistant.** It runs by itself (section 9).
- **The character is a musician, not a mascot,** and nothing about it is cute or childish (section 5).
- **The look:** peach cards with dark red words, button names in a thin frame, and no pictures on cards in the
  first version. The logo shows only at the start (section 5).
- **All music is original,** written in the styles a groovebox is made for. Nothing under copyright goes into
  the course (section 8).
- **A lesson's song stays on the FM-1** when the lesson ends. A copy of what was there before is saved on the
  computer first.
- **A lesson simply starts.** A skip button leads to "I know grooveboxes, just SLOOP 3" or to a list of
  lessons (section 6).
- **Lessons are built on the computer first,** where changes are quick. The lesson player moves into the
  firmware once the lesson format has settled.
- **A lesson is a Markdown file,** written to be read by a person: its steps as a numbered list, with notes for
  an AI assistant and the tester's notes under them (`lessons/README.md`).

## 3. On the FM-1, and an assistant beside it

The course is whole on the FM-1. An AI assistant on a computer adds to it and replaces nothing.

| | On the FM-1 | With an AI assistant beside it |
|---|---|---|
| **What it needs** | Only the FM-1 | This project on a computer, and an assistant that works with its files (`AI.md`) |
| **Status** | Built on 2026-11-19. All thirty play through on the simulated FM-1, and the course is on a real FM-1, being played through. | How the course was built and tested from the start. Its guides were written on 2026-11-22. |
| **Lit buttons, cards, questions, the melody game** | Yes | The lesson's own, on the FM-1 |
| **Checks the learner's work** | Yes | The lesson's own, on the FM-1 |
| **Do it for me** | Yes, from the song as written | The lesson's own, on the FM-1 |
| **Hints** | The written hints, in order | Another way of saying it, when asked |
| **A fuller explanation** | A longer card (`MORE`, not built) | The assistant explains, on the computer, when asked |
| **Talks about the learner's own music** | No | Yes: with the FM-1 on its cable it reads the notes, the sounds and the mix. It cannot hear. |
| **Answers questions** | No | Yes |
| **Controls the FM-1** | The lesson's own setups and "do it for me" | Yes, when asked: on the cable it loads a song, plays, sets the tempo and the mix, writes notes, lights buttons |
| **Changes a lesson, writes a new one** | No | Yes: written, checked, tested on the simulated FM-1, sent to the FM-1 |

The program that leads lessons from the computer (`run.py`) is where the lessons were built first, and the
computer's own tests still play them that way. It is not offered to learners: next to the FM-1 alone it adds
nothing a learner would notice.

### Four rules that keep the version on its own strong

1. **Every lesson has a complete written path:** every word, light, check and hint. Nothing a learner needs
   lives only in an assistant's head.
2. **An assistant starts from the written path** and leaves it only when the learner needs something else.
3. **What works goes back into the lesson.** When an assistant explains a step another way and it lands, that
   becomes a written hint. Every session with an assistant improves the course for those who have none.
4. **The written path is tested with no AI,** on the simulated FM-1, by a robot learner (section 11).

## 4. Lesson 1, as it runs today

Lesson 1, HELLO, has run on the real FM-1 several times. It takes about a minute. The other twenty-nine are
first drafts, each played through on the simulated FM-1 and not yet by a person.

1. **The start.** The FM-1 shows the peach logo on the cards' peach background with WITH SLOOP 3 under it, then
   the start card:
   `TEACHIE BEACHIE / READY FOR A LESSON.`, with `JUST PLAY` on OCT− and `START` on OCT+.
2. **START.** The card says `STARTING...`. The waiting program on the computer sees the press and starts the
   lesson. If no computer answers within 4 seconds, the card says so and offers START again.
3. **The begin card:** `LESSON 1 / HELLO`, with `BEGIN` on OCT+ and `LESSONS` on OCT−, which opens the list of
   lessons (SELECT chooses). Nothing on the FM-1 changes before BEGIN.
4. **The lesson starts a new project and loads its song,** HOUSE ONE, after saving a copy of what was on the
   FM-1.
5. `A BLINKING LIGHT MEANS: PRESS THIS. TRY IT NOW.`, with `GO` blinking on OCT+.
6. `WE MAKE OUR SONGS IN SLOOP. SLOOP HAS 5 TRACKS WITH DIFFERENT INSTRUMENTS. PRESS [PLAY] TO OPEN A SONG IN
   SLOOP.` PLAY blinks.
7. **The song plays on SLOOP's own screen,** with a strip along the bottom: `FIVE TRACKS. ONE SONG. LISTEN.`
   After one time round: `[PLAY] STOPS IT TOO. PRESS [PLAY].` Then `GOOD.`
8. **The melody game.** The FM-1 plays three notes, lighting each key, and the learner plays them back. Then
   five.
9. **The joke.** The FM-1 plays a very fast run. `YOUR TURN.` The first two keys blink and the learner plays
   them. The screen says `FASTER FASTER FASTER`, and the next key pressed sets the whole run off. `NICE.`
10. `HA, THAT WAS TOO FAST. NOBODY CAN PLAY THAT BY HAND. A SEQUENCER CAN.`
11. **The end card:** `LESSON 1 / DONE`, with `REPLAY` on OCT− and `NEXT` on OCT+. NEXT brings up lesson 2's
    begin card. The song stays on the FM-1.

**Every press is the real SLOOP press.** There is no pretend mode, so what the learner learns still works
with the tutorial switched off.

## 5. The look and the character

### The look

- **The start:** the peach logo on the same peach background as the cards, WITH SLOOP 3 under it in the same
  dark red. It replaces SLOOP's sailboat. (The logo was redrawn on 2026-11-18, flatter and cleaner; it is stored
  in eight flat colors, about 2.9 KB.)
- **Cards and strips are a creamy peach (`#FFCBA4`) with dark red words (`#960000`),** and nothing else: no
  edge bar, no second color. SLOOP's own screens are dark, so peach always means "this is the tutorial". (Changed
  on 2026-11-18 from white with black words, to set the tutorial further apart from SLOOP.)
- **A button's name sits in a thin frame,** in the words (`PRESS [PLAY]`) and by the answers (`[OCT+]`).
- **Two sizes of text:** a dry reply alone in large letters in the middle, an instruction smaller and to the
  left.
- **No pictures on cards** in the first version, unless one has an obvious value.
- **The lessons have a font of their own, with small letters** (since 2026-11-20: Google Sans SemiBold, in the lesson
  pack), so their words are sentences. SLOOP's own fonts have capitals only; with no pack on the FM-1 the cards fall
  back to those, in capitals.

### The character

A musician sitting across from the learner, who listens and answers. There is no figure on the screen; the
personality shows in what the FM-1 plays, how its lights move, and a few words.

- **It answers in music.** When something goes right, the band comes in on the next downbeat.
- **It quotes the learner.** After a kick pattern is written, it can tap that rhythm back on the hi-hat.
- **It plays games.** The melody game, the impossible run, a phrase left one note short for the learner to
  finish.
- **Its timing is exact.** What it plays lands on the beat and in the song's key.
- **It knows when to stay quiet.** While the learner works something out, nothing interrupts.
- **Its words are short and dry:** `GOOD.` `NICE.` `AGAIN?` `LISTEN.` No exclamation marks, no praise for
  nothing.
- **It explains fully when asked.** `MORE` gives a complete explanation in plain sentences (section 3).
- **It never talks down.** It treats a ten-year-old as a musician, which also suits adults.

## 6. The controls

These are the only things a learner has to learn about the tutorial itself.

| Control | What it means | Built |
|---|---|---|
| **A blinking button or key** | Press this now. A quick double blink. | Yes |
| **A faint key** | Part of the picture, such as the boxes that already hold a drum. | Yes |
| **The card** | A full screen of words, for an idea, a question or a reply. | Yes |
| **The strip** | One or two lines over SLOOP's own screen, which stays live. | Yes |
| **The strip at the top** | One line, when SLOOP's four knob dials at the bottom have to stay in view. | Yes |
| **A framed name that fills** | The button or key named is held. For a knob: how far the turn the step asks for is done; full means done. | Yes (SLOOP 3.4.2) |
| **The black key's picture** | A black key, counted from the left: on the drum grid it picks the drum. | Yes (SLOOP 3.4.2) |
| **The half page** | Words on top, SLOOP's four knob dials live below. | No (the strip at the top does the job so far) |
| **OCT− and OCT+, both blinking** | A question with two answers, one by each button. While it is up, they don't change the octave. | Yes |
| **White keys lit with numbers** | A choice with three answers or more. | No |
| **Both OCT buttons together** | "I'm stuck." A hint, then the choices below. | No. For now a hint comes by itself when a step takes long, and after the last hint the lesson offers `SHOW ME`. On the FM-1 alone, both OCT buttons held for two seconds leave the lesson. |
| **START and JUST PLAY** | On the start card: begin a lesson, or go to SLOOP. | Yes |
| **REPLAY and NEXT** | On a lesson's end card. | Yes |
| **LESSONS** | On a lesson's begin card: the list of lessons, chosen with SELECT. | Yes |
| **SKIP** | Leads to "I know grooveboxes, just SLOOP 3", or to the list of lessons. | No |
| **MORE** | A fuller explanation, on the computer. | No |

**"Do it for me" comes in three sizes,** all from the song as written, none counting against the learner:
- **SHOW ME:** the FM-1 does the next move itself, slowly, with the button lit. Then it undoes the move and
  the learner tries. (Built in part: offered after the hints run out, and the move is not undone.)
- **FINISH IT:** the rest of the part being worked on is written in, for example the hi-hats. (Built: lesson 3.)
- **BRING THE BAND:** the other tracks come in, so the learner's part plays inside a finished song. (Built:
  lesson 15, and the band is unmuted at the end of most lessons.)

**"Just SLOOP 3."** Every step in a lesson file is marked "everyone" or "beginners only". The path for people
who know grooveboxes plays the same lessons with the beginner-only steps left out.

**A wrong press never loses anything.** The screen says `NOT THAT ONE.` and the right button blinks. If the
learner wanders to another screen, the strip says how to come back. (Built differently so far: most steps are
done when the FM-1 is in the right state, however the learner got there, and a step that needs a certain page
brings the learner back to it after a moment.)

## 7. The rules of teaching

- **One idea per lesson.** Five to ten minutes. Many short lessons beat a few long ones.
- **Hear it, do it, change it, own it,** in that order:
  - the FM-1 plays or shows the idea;
  - the learner does it with the lights guiding;
  - the learner changes it freely;
  - the learner does it again from memory in a later lesson.
- **The real instrument.** The same buttons SLOOP uses, on SLOOP's own screens.
- **The lights fade.** Help comes in three levels, and each skill moves down them:
  1. the button blinks and the screen names it;
  2. the screen names it, no light;
  3. the screen gives only the goal (`ADD A SNARE ON 5 AND 13`).
- **Every lesson ends in music.** The learner's part sits inside a full song, never alone.
- **Nothing can be broken.** Undo comes early. Any lesson can be started again.
- **The same picture every time.** A step is always a "box", a pattern always a "page" (the table below).
- **Few words on the screen.** About eight lines of twenty letters. The FM-1 shows, and the words only point.
- **A short review to open each lesson,** on something from two or three lessons back.

### The big ideas, each with one picture

| Word | What we say | How the FM-1 shows it |
|---|---|---|
| **Groovebox** | Five tracks, each with its own instrument, and a sequencer that remembers what each one plays. | Mute the five tracks one by one while a song plays. |
| **Track** | One instrument. There are five: four instruments and the drums. | Each has its own color on the screen. |
| **Step** | A box. A bar has 16 boxes. | On the drum grid the 16 white keys are the 16 boxes; on the piano roll a knob walks along them. |
| **Sequencer** | A light walks along the boxes, and a full box plays. | The walking light on the keys. |
| **Tempo** | How fast the light walks. | Turn SELECT and watch it speed up. |
| **Loop** | When the light reaches the end, it starts again. | It simply never stops. |
| **Pattern** | One version of the music, all five tracks. There are eight, and each keeps what was saved into it. | Hold SAVE: eight tiles, one for each pattern. |
| **Song** | The order the patterns are played in. | The SONG page, line by line. |
| **Sound** | Which instrument a track plays. | Turn PRESETS while a part plays. |
| **Effect** | A room or a costume for the sound: an echo, a fuzz, a big hall. | Hold FX and press a key. |
| **Mix** | How loud each track is. | Hold GLO and turn the four knobs. |

## 8. The songs

- **All original,** written for the course in the styles a groovebox is made for. A style belongs to
  everyone; a particular song belongs to someone, so the course borrows the feel of a style and never a song.
- **Each skill is taught in the style that is built on it,** so a lesson explains how the FM-1 works and how
  that kind of music works at once.
- **Five tracks,** because SLOOP 3 has five: bass, lead, chords, pad and drums.
- **Written the same way as the FM-1 song library:** a `songs/<song>.py` in the library writes
  `songs/<song>.json`, which builds the song in Ableton Live and feeds the lessons (a copy lives in
  `tutorial/songs/`).
- **About ten songs.** Each is approved by ear, on the FM-1, before a lesson uses it.
- **A song grows over several lessons.** The learner puts the kick into the house song in lesson 2 and its
  snare and hats in lesson 3.
- **For their own pieces, the learner picks the style.**

**HOUSE ONE is written:** A minor, 122 BPM, the chords Am F C G, five tracks, and four sections (A the beat
and the bass, B adds the piano and pad, C the whole band, D a break). Lesson 1 plays section C. It is not yet
built in Ableton, and not yet approved by ear.

| Style | What it teaches | Lessons |
|---|---|---|
| **House** | A kick in boxes 1, 5, 9 and 13 is the house beat. | 1 to 3 |
| **Hip-hop** | A steady snare, and a kick that moves around it. | 3 to 7 |
| **Funk and disco** | A bass line that dances with the kick. | 8, 10 |
| **Pop** | Four chords in a loop, and a tune over them. | 11, 12 |
| **Lo-fi** | Soft chords, mellow sounds, and a gentle mix. | 9, 13, 14 |
| **Trap** | Fast hat rolls and fills. | 17 |
| **Dub** | Echo and reverb used as part of the music. | 19 |
| **Synthwave** | Slow, wide sounds that fade in and wobble. | 21, 22 |
| **Video-game music** | Arpeggios. | 22 |
| **Dance music** | Verse and chorus, build-ups and drops. | 24 to 27 |
| **Hip-hop again** | Swing, and soft and hard hits. | 28 |

SLOOP already has drum kits named for most of these (HOUSE, BOOMBAP, DISCO, LO-FI, TRAP, SYNTHWV, CHIP).

## 9. An AI assistant beside a lesson

- **The lesson runs by itself, on the FM-1.** It never waits for an assistant.
- **The assistant is beside it, on a computer, and is asked.** It is any assistant that can read this project's
  files, change them and run commands. `AGENTS.md`, at the top of the project, is what it reads first; `AI.md` is
  the guide for the person.
- **As a tutor** it answers a question about a step, or explains an idea another way. With the FM-1 on its
  cable it reads the notes, the sounds and the mix, and says one true thing about them. It cannot see the screen
  or hear the music, so it asks which lesson and step the learner is on.
- **It can control the FM-1,** on its cable and when asked: put a lesson's song on it, play and stop, set the
  tempo and the mix, choose sounds, write notes, light a button, put a line of words on the screen. It says what
  it is about to do first, and saves a copy before it replaces the song (`AGENTS.md`).
- **As a lesson writer** it changes a lesson or writes a new one, checks that the words fit the screen, plays the
  lesson on the simulated FM-1, and sends the lessons to the FM-1 when told to.
- **An assistant teaches; it doesn't compose.** All music in a lesson comes from the song as written and
  approved.
- **What works goes back into the lesson** (section 3, rule 3).
- **Set aside on 2026-11-22: an assistant that cuts in by itself.** Until then the plan had an assistant watching
  every press and speaking on the FM-1's screen, with four cases where the lesson would wait for it. It was never
  built. The author: "we don't need it to answer on the FM-1 I don't even know if thats a good idea really."

## 10. The course

Four parts, thirty lessons. Each part ends with **a piece of the learner's own,** made with no lit buttons.

The button moves come from SLOOP's guides and its code. Each has been checked on the simulated FM-1: the robot
learner makes the move, and the lesson reads from the FM-1 that it did what the lesson says. All thirty are
drafted in `lessons/`.

### Part 1: your first beat

| # | Lesson | The one idea | What the learner does |
|---|---|---|---|
| 1 | Hello | A blinking light means "press me". SLOOP has five tracks. | Starts and stops a song with PLAY, plays the melody game, meets the run nobody can play by hand. |
| 2 | Sixteen boxes | A sequencer is a row of boxes and a walking light. | Picks the drum track, plays the kick, opens the drum grid with SEQ, taps a kick into boxes 1, 5, 9 and 13: a house beat. |
| 3 | Kick, snare, hat | A beat is three simple layers. | Adds the snare on 5 and 13 and the hats, picking each drum with its black key. |
| 4 | The drum kit | A kit has 16 drums, and a new kit swaps all 16 at once. | Adds a clap and two toms to the beat, tries other drums with KNOB 1, tries other kits with PRESETS. |
| 5 | Meet the tracks | Each track plays its own instrument. | Holds GLO and taps keys 1 to 5 to silence each track, then solos one. |
| 6 | Faster, slower, oops | Tempo is how fast the light walks. Undo takes a mistake back. | Changes the tempo with SELECT, makes a mess on purpose, undoes it (EDIT + OCT−), saves (SAVE twice, then KNOB 1). |
| 7 | **Your own beat** | | Makes a beat on an empty drum track. The band joins in. |

### Part 2: notes and chords

| # | Lesson | The one idea | What the learner does |
|---|---|---|---|
| 8 | A bass line in boxes | The other tracks put notes in boxes, and low notes are the bass. | On track 1, opens the piano roll with SEQ, turns KNOB 1 to a step and plays a key: its note is written there. |
| 9 | Pick your sound | Each track can play any instrument. | Turns PRESETS, opens SOUNDS, picks by category, keeps a favorite. |
| 10 | Record it live | REC writes down what you play. | Presses REC and plays over the loop. Undoes a bad take and tries again. |
| 11 | No wrong notes | A key is a family of notes that sound good together. | Sets the scale and SNAP on the SEL page (KNOB 2, KNOB 3), then plays a tune on the white keys by following the lights. |
| 12 | One-finger chords | A chord is several notes at once. | Turns chords on (SEL, then KNOB 4) and records four chords. |
| 13 | The piano roll | Notes can be moved, stretched and removed. | Opens the piano roll (SEQ), fixes a wrong note, makes one longer, deletes one. |
| 14 | Mix the tracks | If everyone shouts, nobody is heard. | Sets the level of each track (KNOB 2 on the TRACKS page), first with a target and then by ear. |
| 15 | **Your own loop** | | Drums, bass, chords and a melody. The learner makes at least two of the parts and may ask for the rest. |

### Part 3: effects and sound

| # | Lesson | The one idea | What the learner does |
|---|---|---|---|
| 16 | Eight patterns | A song is made of patterns, and there are eight. | Changes the beat, holds SAVE and saves it into pattern 2, switches between the two: a verse beat and a chorus beat. |
| 17 | Fills and rolls | A fill says "something new is coming". | Plays a fill by hand (GLO + a key), marks a note on its sheet so it only plays in a fill (OCT+ held), makes a hat roll (ARP + a key). |
| 18 | Play the effects | Some effects are played like notes. | Holds FX and presses keys for the punch-in effects, then locks them on (FX + HOME). |
| 19 | Echo and space | Delay repeats a sound. Reverb puts it in a room. | Sends the melody to the echo and the reverb, finds what too much sounds like, changes the echo itself. |
| 20 | Color a track | An insert changes one track. A filter makes it dark or bright. | Puts FUZZ or CRUSH on the bass, sweeps the filter on the whole mix (FX + KNOB 1). |
| 21 | Shape a sound | A sound has a beginning and an end. | On the ENV and EDIT pages, makes one sound start slowly, ring longer, get brighter. |
| 22 | Wobble and arps | The FM-1 can move a sound, or play a chord's notes by itself. | Adds a wobble with the LFO page, then holds a chord and lets the arpeggiator play it. **The payoff:** the arpeggiator plays lesson 1's impossible run. |
| 23 | **Your own piece, live** | | Two patterns, a fill and effects, played live. |

### Part 4: make a song

| # | Lesson | The one idea | What the learner does |
|---|---|---|---|
| 24 | Songs have parts | Intro, verse, chorus, ending. | Listens to a full song and names each part as it comes, with the OCT buttons. |
| 25 | Chain the patterns | Patterns played in a row make a song. | Holds SAVE and taps patterns in order, then records a song live (SONG REC). |
| 26 | Write it line by line | A song can be written down and edited. | Opens SONG 2/3 and sets each line: which pattern, how many bars. |
| 27 | Build-ups and drops | Taking tracks away is as strong as adding them. | Arranges with mutes, fills and filter sweeps: a quiet intro, a build, a drop. |
| 28 | Swing and feel | The same beat can march or dance. | Adds swing, makes some hats louder than others (KNOB 2 and KNOB 4 on the drum grid), plays soft and hard hits (OCT− and OCT+ while drumming). |
| 29 | Keep it and share it | A finished song deserves to be kept. | Saves the project on the PROJECT page, wrecks it on purpose and loads the saved one again. Recording the song as a sound file is only described so far. |
| 30 | **The final test** | | An empty project and no lights. A whole song: three parts or more, four tracks or more, effects, saved. Help comes only when asked. |

### After the course: deeper series, in any order

Each is a short series of lessons inside SLOOP 3, covering what the thirty lessons only touch:
- **How a sound is made:** one sound built from nothing on SLOOP's ANALOG engine, then kept as the learner's
  own.
- **Keys and chords in depth:** SLOOP's key, scale and chord settings.
- **Sampling:** the learner's own drum sounds and chopped breakbeats, made with SLOOP's web editor.
- **Motion:** a sound changing step by step, and the "pumping" effect where the kick pushes the other sounds
  down.
- **Extras:** the visualizers, color themes, the DX7 sounds, and playing the FM-1 from another keyboard.

## 11. Learning from every play-through

- **The logbook.** Each run records every step, how long it took, every wrong press and every call for help.
  A step where the author sat for forty seconds is a step to rewrite.
- **Two questions when a lesson ends,** answered with the OCT buttons: `TOO EASY` or `TOO HARD`, and
  `PLAY IT AGAIN` or `NEXT ONE`. (Not built.)
- **Notes in plain words.** After a session the author tells an assistant what felt wrong, and it proposes
  changes from the notes and the logbook. `NOTES.md` holds these.
- **What an assistant tried goes into the lesson,** when it worked (section 3, rule 3).
- **Every lesson is tested with no person at all.** The simulated FM-1 runs the real firmware on the computer,
  and a robot learner presses whatever blinks, from the first card to the last. SLOOP's own tests run too.
  When SLOOP 3 changes a button, the lesson that breaks is found the same day.
- **A copy of the project** that was on the FM-1 is saved on the computer before every lesson.

## 12. How it is built

| Piece | What it is | Status |
|---|---|---|
| **The guide** | The addition to SLOOP 3's firmware: cards, strips, framed button names, lights at four levels or the double blink, buttons and keys the lesson takes for itself, every press and turn reported. | Built |
| **The start** | The logo on the cards' peach, then the start card with START and JUST PLAY. | Built |
| **The connector** | The computer's side of the USB link: SLOOP's own editor commands plus the guide. | Built |
| **The waiting program** | Waits for START on the FM-1, plays the lessons from where the learner is, then waits again. It finds the FM-1 again after it is unplugged. | Built; runs while a terminal or an assistant's session keeps it running |
| **The lessons** | Thirty Markdown files, read by a person, by an AI assistant and by the lesson player on the computer (`tp/lesson.py`). | Drafted; to be packed for the FM-1 later |
| **The checker** | Reads every lesson, measures its words against the FM-1's screen, and plays it on the simulated FM-1 with the robot learner (`check.py`). | Built |
| **The songs** | Spec files in the song library's form. | HOUSE ONE written |
| **The logbook and the copies** | Folders on the computer, kept out of the repository. | Built |
| **The lesson player in the firmware** | Plays the packed lessons with no computer (`fw/course.inc`): the same cards, lights, checks, hints and SHOW ME. START with no computer answering begins it. | Built; tested on the simulated FM-1 only |
| **The packer** | Turns the Markdown lessons into one small pack for the FM-1 (`tools/mkcourse.py`); `tutorial/device.py` puts it on the FM-1 and tests it. | Built |

**The room left in the firmware is small** (on SLOOP 3.4.2, 2026-11-26; `tutorial/UPGRADE-3.4.2.md`, "The room"):
- **2,720 bytes are free** before the sample bank, and 1,880 of RAM. To make that room, SLOOP's visualizer and what
  SLOOP 3 brings over from SLOOP 2.8 at its first start are left out, and the logo moved out of the firmware.
- **The lessons, their song, their fonts and the logo are in a sample slot.** The pack is about 57 KB and the logo
  17 KB, of a slot's 80 KB: room for two or three more lessons. The blank bytes between them hold the lesson the
  learner was on. Installing new firmware doesn't erase the slot, but a firmware only takes the lessons made for it.

## 13. The order of work

**Done (2026-11-17):** the guide, the start, the connector, the robot learner, lesson 1, HOUSE ONE, the
waiting program, and Teachie Beachie installed and run on the real FM-1.

**Done (2026-11-18):** lessons as Markdown files, the player that runs them, the checker, the list of lessons on
the begin card, and first drafts of all thirty lessons, each played through on the simulated FM-1.

**Next, in order:**
1. **The author plays the lessons** on the real FM-1, from lesson 1, and writes notes in each file. The lessons
   are revised from the notes and the logbook.
2. **Build HOUSE ONE in Ableton** and approve it by ear. Check first that the library's tools handle five
   tracks.
3. **The other songs,** one style at a time, each approved by ear before a lesson uses it (section 8).
4. **A small addition to the firmware:** the guide tells the lesson the key, scale, chord, arpeggiator, the drum
   that is picked, and where the cursor is. Then the steps that only count clicks can check the result.
5. **What the lessons still lack:** `MORE`, "I'm stuck" on both OCT buttons, the skip menu on the FM-1, and the
   two questions at a lesson's end.
6. **(Set aside on 2026-11-22:** an assistant cutting in during a lesson. Section 9.)
7. **The course on the FM-1 alone, on a real FM-1.** It is built and passes on the simulated FM-1 (2026-11-19);
   it has to be installed and played by a person.
8. **What the FM-1 alone still lacks:** room to spare in the firmware. Remembering the lesson after it is switched
   off is built (the move to SLOOP 3.4.2, step 8).
9. **The first young learner** plays the course from lesson 1, on the FM-1.
10. **Going public:** the public repository, the installer page and the lessons. Then showing it to SLOOP's
    developer, and offering the guide to SLOOP itself.

## 14. Things to watch

- **SLOOP 3 is a beta and changes often.** Lessons name what to do ("open the drum grid") as well as which
  button does it, and everything is re-tested after each new SLOOP.
- **Room in the firmware.** The guide and the player have to stay small.
- **Late answers.** While the FM-1 saves, it answers nothing for a moment, and it misses presses too. The
  connector asks again and keeps whatever a late answer brings. This looks like the whole of the "lost
  answers" seen on 2026-11-17, though that is not proven on the real FM-1.
- **SLOOP saves by itself** whenever the music has been stopped for a few seconds, so a lesson's song becomes
  the saved project. That is as decided, and lesson 29 has to work around it.
- **Other programs talking to the FM-1.** SLOOP's web editor or the installer page open at the same time can
  get in the way. Close them during a lesson.
- **Old MIDI names on a Mac.** A Mac that knew SLOOP 2.8 calls the FM-1 "Felucca", and the installer can't find
  it until the names are changed. Other owners will meet this too.
- **The FM-1's small speaker can't play low notes.** SLOOP's SPEAKER setting helps; headphones are better.
- **The song library's tools were written for four tracks,** and SLOOP 3 songs have five.
- **An FM-1 coming straight from SLOOP 2.8** no longer has its old project imported at the first start; the room
  went to the lesson player. Its old sounds, favorites and settings are still taken over. Installing SLOOP 3
  itself first does the import.

## 15. Left for later

Set aside to keep the first version simple:
- **A band written to fit the learner's own beat.**
- **Lessons made on request,** such as "show me how to make a spooky beat".
- **Extra practice made up** from what the learner found hard.
- **Other FM-1 firmware:** FMini-1 (a Minimoog-style synth) for how a sound is made, ChoralRoot for chords,
  PIGEON for sampling, X0X for classic drum machines. Each is open, so the guide could go in. SHRUB is closed,
  so it can't.
- **A tutorial for SLOOP's web editor,** as a separate project.

## 16. Questions to settle

1. **Is the computer screen part of the lesson?** Settled on 2026-11-22: no. Everything needed is on the FM-1. A
   computer is only where an assistant is asked.
2. **Does the learner already know the names of the notes, or play the piano?** That sets the pace of part 2.
3. **Which kinds of music does the learner like most?** The songs can lean that way.
4. **Is thirty lessons in four parts the right size?**
5. **The parts' names:** plain, as here, or the sea names from draft 2 (greenhand, deckhand, able hand)?
6. **Voice samples in a slot:** "Beachie!" and other ad libs as a 16-pad kit, recorded by the family. Yes or no?
7. **Should the waiting program start by itself** whenever the Mac starts? That changes a setting on the Mac.
