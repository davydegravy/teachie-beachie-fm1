# The steps to look at for SLOOP 3.4.2, lesson by lesson

Made by `tutorial/upgrade/worksheet.py` from `tutorial/lessons/`. Each step below touches something SLOOP 3.4.2
changed; its flags say what. A flag says what to check, not that the step is wrong: the runs and the review
in `tutorial/UPGRADE-3.4.2.md` say which ones break. Today's words are as the lesson file has them (` / ` a
new line, ` // ` a gap).

## The flags

- **SAVE**: SAVE changed: SAVE on the tracks page opens SONG 2/3; the project is saved from PROJECT (SAVE twice no longer saves); SAVE + a key held 1.5 s saves into that pattern and does not switch; SAVE + a tap on the playing pattern puts back its saved copy.
- **SEQ**: SEQ on a synth track opens NOTES (the piano roll), then PATTERN, then SONG: the page of boxes is the drums' only.
- **GRID**: the drum grid: a white key sets or clears a box (with or without SEQ) and makes no sound; a black key or KNOB1 picks the drum. Playing a drum's key there no longer picks it.
- **GRIDSEQ**: works on 3.4.2, but [SEQ] is no longer needed on the drum grid: a white key sets or clears a box by itself. Keep teaching [SEQ] + a box, or not: a decision.
- **GRIDKN**: the drum grid's knobs are SOUND, STEP, HIT, LEVEL (were SOUND, QUANT, GATE, STEPS).
- **HITS**: on the drum grid OCT+ held opens the hit SHEET and OCT- turns the page: OCT held + a drum no longer gives soft or hard hits.
- **SEL**: SEL opens SEL 1/2, a knob page with no tiles: ROOT, SCL, QNT, CHORD. KNOB4 turns chords on (TRIAD); KNOB3 is SNAP.
- **HOME**: HOME held 0.7 s opens a MENU (SCREEN, LIGHTS ...); there is no PROJECT page (keys 1, 3, 5 still work, unlabelled).
- **NAMES**: names on screen: SENDS is FX 1/7 (ECHO is DLY), DELAY is DLY 6/7, FILT is FILTER, SCALE is SCL; EDIT has FLT 2/6 and FILTER 3/6; PLUS is gone; YOURS is under SAVE; MASTER is under GLO.
- **LAYOUT**: sound pages are drawn the other way up: knob labels at the top, the page's title small at the bottom right. A bottom strip hides the title; a one-line top strip covers the knob labels.
- **ORDER**: the order of a button's pages changed (EDIT: OSC, FILTER, the analog filter, VOICE, VOICE 2, SOUNDS; FX: SENDS, a track filter, SLICER, INSERT, INSERT 2, DLY, REV/CHO; LFO: LFO, LFO 2, LFO DEST).
- **KEEP**: each pattern keeps a saved copy and leaving a pattern puts it back: what is not saved into a pattern is lost on a pattern change (and the setups' songs need keep_all() after c_setup).
- **CHAIN**: [15] is no longer CHAIN: it switches THIS TRACK ONLY / every track and stays; [16] opens SONG; a chain is pattern keys tapped in one SAVE hold; CHAIN DONE is never said.
- **LINE**: SAVE + [13] plays the song from the LINE shown, not from the top.
- **UNDO**: undo keeps 3 moves (was 8).
- **LOCK**: the FX + HOME lock happens on HOME's release, and only if FX is still held.
- **P1**: the tracks page shows the pattern as a number in a square at the top right, not P1.

25 lessons have steps to look at. Steps flagged, by flag: SAVE 23, SEQ 17, GRID 18, GRIDSEQ 26, GRIDKN 4, HITS 5, SEL 24, HOME 8, NAMES 14, LAYOUT 23, ORDER 11, KEEP 9, CHAIN 5, LINE 2, UNDO 1, LOCK 3, P1 2.

## Lesson 2: Sixteen boxes

- **Step 6, card** · `GRIDSEQ`
  - Today: 16 boxes across. / That is one bar of music. // Hold [SEQ], and the / 16 white keys are / the 16 boxes.
- **Step 7, strip** · `GRID`
  - Today: Play [KEY 1], the lowest white key: the kick.
  - Do: tap box 1
  - Page: STEPS
- **Step 8, strip** · `GRIDSEQ`
  - Today: Now hold [SEQ] and tap [1].
  - Do: hold SEQ, tap box 1
  - Until: drums has KICK on box 1
  - Hint: Keep holding [SEQ]. / Tap the blinking key.
- **Step 10, strip** · `GRIDSEQ`
  - Today: One kick is lonely. / Hold [SEQ] and tap / [5], [9] and [13].
  - Do: hold SEQ, tap box 5 9 13
  - Until: drums has KICK on boxes 1 5 9 13
  - Hint: Hold [SEQ] and tap / each blinking key.
- **Step 13, strip** · `GRIDSEQ`
  - Today: Hold [SEQ] and tap a box / to add or remove a kick. / [OCT+] when done.
  - Robot: hold SEQ, tap box 7, let go

## Lesson 3: Kick, snare, hat

**Stopped on 3.4.2:** the computer-led run, step 6 (drum grid).

- **Step 4, card** · `GRID`
  - Today: Each key is / a different drum. / Play one to pick it. / The selected drum / can then be put / into the boxes.
- **Step 5, strip** · `GRID`
  - Today: Pick the snare: / play [KEY 3].
  - Do: tap box 3
  - Page: STEPS
- **Step 6, strip** · `GRIDSEQ`
  - Today: Now hold [SEQ] and tap [5] and [13].
  - Do: hold SEQ, tap box 5 13
  - Until: drums has SNARE on boxes 5 13
  - Hint: If the top says KICK, play [3] first.
- **Step 9, strip** · `GRID`
  - Today: Now pick the hat: / play [5].
  - Do: tap box 5
  - Page: STEPS
- **Step 10, strip** · `GRIDSEQ`
  - Today: Hold [SEQ]. Hats on [3], [7], [11], [15]: between the kicks.
  - Do: hold SEQ, tap box 3 7 11 15
  - Until: drums has HAT on boxes 3 7 11 15
  - Hint: If the top does not say HAT, play [5] first.
- **Step 14, strip** · `GRIDSEQ`
  - Today: Hold [SEQ]. Hats on [1], [5], [9], [13] too.
  - Do: hold SEQ, tap box 1 5 9 13
  - Until: drums has HAT on boxes 1 3 5 7 9 11 13 15
  - Hint: If the top does not say HAT, play [5] first.
  - Robot: tap box 5, hold SEQ, tap box 1 5 9 13
- **Step 17, strip** · `GRIDSEQ`
  - Today: Wrong box? Hold [SEQ] / and tap it again. / [OCT+] when done.

## Lesson 4: The drum kit

- **Step 3, strip** · `GRID`
  - Today: Play every white key. The top names each drum.
- **Step 6, strip** · `GRID`
  - Today: Play along. Try [KEY 4], the clap. / [OCT+] when done.
  - Until: played 4 keys
  - Hint: Not yet. Play a few drums first.
  - Robot: tap box 4 10 11 4

## Lesson 6: Faster, slower, oops

**Stopped on 3.4.2:** both runs, step 18 (SAVE twice).

- **Step 10, strip** · `GRIDSEQ`
  - Today: Hold [SEQ]. Tap [KEY 2], [6] and [10].
  - Do: hold SEQ, tap box 2 6 10
  - Until: drums has KICK on boxes 2 6 10
- **Step 16, card** · `UNDO`
  - Today: Undo will remember / your last 8 moves. / Went too far? / To redo hold [EDIT] / and press [OCT+].
- **Step 18, strip** · `SAVE` `HOME`
  - Today: Press [SAVE] twice, quickly.
  - Do: press SAVE twice
  - Until: says PROJECT SAVED
  - Hint: Two quick taps on [SAVE].
- **Step 21, card** · `SAVE`
  - Today: Tempo: [SELECT]. / Undo: [EDIT] + [OCT-]. / Redo: [EDIT] + [OCT+]. / Save: [SAVE] twice. // Now nothing can / go wrong.

## Lesson 7: Your own beat

**Stopped on 3.4.2:** both runs, step 4 (drum grid).

- **Step 3, card** · `GRID`
  - Today: Pick a drum: play its key. / [SEQ] + a key: a box fills. / Press again: it empties. / [PLAY]: hear it. / [EDIT] + [OCT-]: undo.
- **Step 4, strip** · `GRIDSEQ`
  - Today: Make your beat. / [OCT+] when it is ready.
  - Until: drums has 6 notes or more
  - Hint: Not yet. A beat needs 6 hits or more.
  - Page: STEPS
  - Robot: tap box 1, hold SEQ, tap box 1 5 9 13, let go, tap box 3, hold SEQ, tap box 5 13, let go
- **Step 8, strip** · `SAVE` `HOME`
  - Today: Save it: press [SAVE] twice.
  - Do: press SAVE twice
  - Until: says PROJECT SAVED

## Lesson 8: A bass line in boxes

**Stopped on 3.4.2:** the FM-1-led run, step 4 (SEQ on the bass).

- **Step 4, strip** · `SEQ`
  - Today: Press [SEQ]. The same 16 boxes.
  - Do: press SEQ
  - Until: page is STEPS
- **Step 5, strip** · `SEQ`
  - Today: Play the blinking key. It is an A.
  - Do: tap A3
  - Page: STEPS
- **Step 6, strip** · `SEQ`
  - Today: Hold [SEQ] and tap [KEY 1]. The A drops in.
  - Do: hold SEQ, tap box 1
  - Until: bass has notes on box 1
  - Hint: Keep holding [SEQ] and tap the blinking key.
- **Step 7, strip** · `SEQ`
  - Today: Press [PLAY].
  - Do: press PLAY
  - Until: playing
- **Step 8, strip** · `SEQ`
  - Today: Hold [SEQ]. The same A on [4], [7], [11], [14].
  - Do: hold SEQ, tap box 4 7 11 14
  - Until: bass has notes on boxes 1 4 7 11 14
- **Step 9, reply** · `SEQ`
  - Today: Good.
- **Step 10, card** · `SEQ`
  - Today: One note in five boxes is / already a bass line. // Now a second note.
- **Step 11, strip** · `SEQ`
  - Today: Let go of [SEQ]. Play the / higher A: [KEY 10].
  - Do: tap A4
  - Page: STEPS
- **Step 12, strip** · `SEQ`
  - Today: Hold [SEQ]. Put it on [16].
  - Do: hold SEQ, tap box 16
  - Until: bass has notes on box 16
- **Step 13, card** · `SEQ`
  - Today: _Bass and kick / The kick is on / 1, 5, 9 and 13. / Your bass is on 1 too, / then lands in between. // That is what makes / it groove.
- **Step 14, card** · `SEQ` `GRID`
  - Today: Make it your own. / Pick a note: play its key. / Hold [SEQ], tap a box: / it is added. / Tap again: it is removed.
- **Step 15, strip** · `SEQ`
  - Today: Your bass line. / [OCT+] when done.
  - Robot: tap C4, hold SEQ, tap box 9, let go
- **Step 17, strip** · `SEQ`
  - Today: Your bass line, in the song.
  - Wait: 4 bars
- **Step 18, strip** · `SEQ`
  - Today: Press [PLAY] to stop.
  - Do: press PLAY
  - Until: stopped
- **Step 19, card** · `SEQ`
  - Today: A key picks the note. // [SEQ] and a box writes it.

## Lesson 9: Pick your sound

- **Step 8, strip** · `ORDER`
  - Today: Tap [EDIT] until you see / sound names.
  - Do: press EDIT
  - Until: page is SOUNDS
  - Hint: It is the last of the edit pages. Keep tapping.
  - Robot: press EDIT, press EDIT, press EDIT, press EDIT, press EDIT, press EDIT

## Lesson 11: No wrong notes

- **Step 9, strip** · `SEL`
  - Today: Hold [SEL] and tap / the A: [KEY 10].
  - Do: hold SEL, tap A4
  - Until: says KEY A
- **Step 10, strip** · `SEL`
  - Today: Press [SEL]: / the KEY page.
  - Do: press SEL
  - Until: page is KEY
- **Step 11, strip top** · `SEL`
  - Today: [KNOB2] right until / SCALE says MIN.
  - Do: turn KNOB 2 right 2
  - Page: KEY
  - Hint: Two clicks right. SCALE is the 2nd dial.
- **Step 12, strip top** · `SEL`
  - Today: [KNOB4]: one click right. SNAP.
  - Do: turn KNOB 4 right 1
  - Page: KEY
- **Step 13, card** · `SEL`
  - Today: MIN is the minor scale. / With SNAP, a note that / doesn't fit moves down / to one that does. / The black keys can't / go wrong now.
- **Step 14, strip** · `SEL` `LAYOUT`
  - Today: Play the black / keys again.
  - Until: played 4 keys
  - Robot: tap F#3 G#3 A#3 C#4
- **Step 15, reply** · `SEL`
  - Today: No sour notes.
- **Step 16, strip** · `SEL` `LAYOUT`
  - Today: Follow the lights: a tune in A minor.
- **Step 17, strip** · `SEL` `LAYOUT`
  - Today: Now make up your own. / [OCT+] when done.
- **Step 18, strip** · `SEL` `LAYOUT`
  - Today: Press [HOME].
  - Do: press HOME
  - Until: page is TRACKS
- **Step 20, card** · `SEL`
  - Today: A song's key is the family / of notes that fit it. / Hold [SEL] and tap a note / to set the key. / With SNAP on, every / note you play fits.

## Lesson 12: One-finger chords

**Stopped on 3.4.2:** the computer-led run, step 10 (SEL's knobs).

- **Step 3, strip** · `SEL`
  - Today: Press [SEL]: / the KEY page.
  - Do: press SEL
  - Until: page is KEY
- **Step 4, strip top** · `SEL`
  - Today: [KNOB3]: 1 click right. / Tiles show chords.
  - Do: turn KNOB 3 right 1
  - Page: KEY
- **Step 5, strip** · `SEL` `LAYOUT`
  - Today: One key, one chord. Play the blinking keys.
- **Step 6, card** · `SEL`
  - Today: Those four are this / song's chords: A minor, / F, C and G. // One for each bar.
- **Step 7, strip** · `SEL` `LAYOUT`
  - Today: Press [PLAY].
  - Do: press PLAY
  - Until: playing
- **Step 8, strip** · `SEL` `LAYOUT`
  - Today: Follow the light: a chord for each bar. / [OCT+] when ready.
- **Step 9, strip** · `SEL` `LAYOUT`
  - Today: Press [REC].
  - Do: press REC
  - Until: recording
- **Step 10, strip** · `SEL` `LAYOUT`
  - Today: Play the four chords, one each bar.
  - Until: chords has 12 notes or more
  - Robot: tap C4, wait 1.5, tap A4, wait 1.5, tap E4, wait 1.5, tap B4
- **Step 11, strip** · `SEL` `LAYOUT`
  - Today: Press [REC] to stop recording.
  - Do: press REC
  - Until: not recording
- **Step 12, strip** · `SEL` `LAYOUT`
  - Today: Your chords. Listen.
  - Wait: 4 bars
- **Step 13, card** · `SEL`
  - Today: Keep them?
- **Step 16, card** · `SEL`
  - Today: [SEL], then [KNOB3]: / one-finger chords. // Four chords in a loop: / most pop songs work / this way.

## Lesson 13: The piano roll

**Stopped on 3.4.2:** the FM-1-led run, step 8 ([SEQ] twice lands on PATTERN).

- **Step 4, strip** · `SEQ`
  - Today: Press [SEQ] twice: / the piano roll.
  - Do: press SEQ, press SEQ
  - Until: page is NOTES
  - Robot: press SEQ, wait 0.3, press SEQ
- **Step 15, strip top** · `ORDER`
  - Today: Press [EDIT].
  - Do: press EDIT
  - Until: lead has nothing on box 13
  - Page: NOTES
  - Hint: Step 13, then [EDIT].
- **Step 20, card** · `SEQ`
  - Today: Piano roll page: / [SEQ] twice. / STEP: [KNOB1]. / NOTE: [KNOB2]. / LEN: [KNOB3]. / Delete: [EDIT].

## Lesson 15: Your own loop

**Stopped on 3.4.2:** both runs, step 4 (drum grid).

- **Step 3, card** · `GRIDSEQ`
  - Today: Boxes: [SEQ] + a key. / Live: [REC]. / A track: [ALGORITHM]. / Undo: [EDIT] + [OCT-].
- **Step 4, strip** · `GRIDSEQ`
  - Today: Drums first. / [OCT+] when ready.
  - Until: drums has 4 notes or more
  - Hint: Not yet. The beat needs 4 hits or more.
  - Robot: tap box 1, hold SEQ, tap box 1 5 9 13, let go
- **Step 5, strip** · `GRIDSEQ`
  - Today: Now add another track. / [OCT+] when ready.
  - Until: 2 tracks or more
  - Hint: Not yet. Write something on a second track first.
  - Robot: turn ALGORITHM right 1, tap A3, hold SEQ, tap box 1 4 7, let go
- **Step 10, strip** · `SAVE` `HOME`
  - Today: Save it: / press [SAVE] twice.
  - Do: press SAVE twice
  - Until: says PROJECT SAVED

## Lesson 16: Eight pages

**Stopped on 3.4.2:** both runs, step 6 (the copy of a page: SAVE).

- **Step 3, strip** · `KEEP` `P1`
  - Today: Press [PLAY]. This is / page 1: P1, top right.
  - Do: press PLAY
  - Until: playing
- **Step 4, card** · `SAVE`
  - Today: _Pages: hold [SAVE] / Keys [1] to [8] / are the 8 pages. // Tap a key: go there. / Hold a key: copy / this page onto it.
- **Step 6, strip** · `SAVE`
  - Today: Hold [SAVE] + [2] until / the top says COPY.
  - Do: hold SAVE, long box 2
  - Until: drums pattern 2 has notes and drums on pattern 2
  - Hint: Keep holding both until the top says COPY.
- **Step 8, card** · `KEEP` `P1`
  - Today: You are on page 2: / see P2, top right. / It is a copy of page 1. // Change page 2. / Page 1 stays as it was.
- **Step 10, strip** · `GRID`
  - Today: Play [1]: the kick.
  - Do: tap box 1
  - Page: STEPS
- **Step 11, strip** · `GRIDSEQ`
  - Today: Hold [SEQ]. Extra kicks on [8] and [16].
  - Do: hold SEQ, tap box 8 16
  - Until: drums has KICK on boxes 8 16
  - Hint: If the top does not say KICK, play [1].
- **Step 12, card** · `SAVE` `CHAIN`
  - Today: _Two pages / Page 1: the beat. / Page 2: the beat, plus / kicks on [8] and [16]. // Switch between them: / [SAVE] + [1] or [2].
- **Step 13, strip** · `SAVE` `GRID`
  - Today: Hold [SAVE], tap [1]. / It waits for the bar's end.
  - Do: hold SAVE, tap box 1
  - Until: drums on pattern 1
- **Step 14, strip** · `SAVE` `GRID`
  - Today: And back: [SAVE] + [2].
  - Do: hold SAVE, tap box 2
  - Until: drums on pattern 2
- **Step 17, card** · `SAVE`
  - Today: A page is also called / a pattern. // Go to a page: / [SAVE] + a key. / Copy this page: / [SAVE] + hold a key.

## Lesson 17: Fills and rolls

**Stopped on 3.4.2:** the computer-led run, step 12 (drum grid: [11] wrote a box).

- **Step 8, strip** · `GRID`
  - Today: Hold [GLO], tap [12].
  - Do: hold GLO, tap box 12
  - Wait: 2 bars
- **Step 10, strip** · `GRID`
  - Today: Let go of [GLO]. / Pick the high tom: [11].
  - Do: tap box 11
  - Page: STEPS
- **Step 11, card** · `GRIDSEQ`
  - Today: _Three at once / Hold [SEQ], hold [12], / then press [OCT+]. // Use both hands. Keep / the first two down.
- **Step 12, strip** · `GRIDSEQ`
  - Today: Hold [SEQ] + hold [12]. / The top says STEP 12.
  - Do: hold SEQ, hold box 12
  - Until: drums has TOM H on box 12
  - Hint: Hear a crash? [SEQ] was not held. Hold [SEQ] first.
- **Step 13, strip** · `GRIDSEQ`
  - Today: Keep holding both. / Press [OCT+] once.
  - Do: hold SEQ, hold box 12, press OCT+
  - Until: drums has 5 fill notes or more
  - Hint: Hold [SEQ] + [12], then [OCT+] once. The top says FILL.
  - Hint: Top says NO FILL? [OCT-] puts it back. Then [OCT+] once.
- **Step 15, strip** · `GRID`
  - Today: Now [GLO], not [SEQ]: / hold [GLO], tap [12].
  - Do: hold GLO, tap box 12
  - Wait: 2 bars
- **Step 18, strip top** · `GRIDKN`
  - Today: Speed: [ARP] + [KNOB1]. / Hear it: [ARP] + [5]. / [OCT+] when done.
  - Robot: hold ARP, turn KNOB 1 right 1, long box 5, let go
- **Step 20, card** · `GRIDSEQ`
  - Today: _Fills and rolls / Fill: [GLO] + [11] or [12]. / Fill note: / [SEQ] + a key + [OCT+]. / Roll: [ARP] + a key.

## Lesson 18: Play the effects

- **Step 8, strip** · `NAMES`
  - Today: Hold [FX] + [15]: ECHO.
  - Do: hold FX, long box 15
  - Until: punch on
- **Step 11, card** · `LOCK`
  - Today: Tired of holding [FX]? / Lock it, and the keys / stay effects. // To lock: [FX] + [HOME].
- **Step 12, strip** · `LOCK`
  - Today: Hold [FX], tap [HOME], / then let go of both.
  - Do: hold FX, press HOME
  - Until: says LOCK
- **Step 14, strip** · `ORDER`
  - Today: Press [FX] once / to unlock.
  - Do: press FX
  - Until: pressed FX
- **Step 16, card** · `LOCK`
  - Today: Effect: hold [FX] + a key. / Lock: [FX] + [HOME]. / Unlock: [FX] again.

## Lesson 19: Echo and space

- **Step 4, strip** · `NAMES` `ORDER`
  - Today: Tap [FX] until / the screen says SENDS.
  - Do: press FX
  - Until: page is SENDS
  - Robot: press FX, press FX, press FX, press FX, press FX, press FX, press FX
- **Step 5, strip top** · `NAMES`
  - Today: [KNOB3], echo: up.
  - Do: turn KNOB 3 right 5
  - Until: lead delay above 50
  - Page: SENDS
- **Step 6, strip top** · `NAMES`
  - Today: [KNOB4], reverb: up.
  - Do: turn KNOB 4 right 5
  - Until: lead reverb above 55
  - Page: SENDS
- **Step 8, strip top** · `NAMES`
  - Today: [KNOB4] down past 30.
  - Do: turn KNOB 4 left 5
  - Until: lead reverb below 30
  - Page: SENDS
- **Step 9, card** · `NAMES`
  - Today: The echo has a page / of its own. // It is called DELAY.
- **Step 10, strip** · `NAMES` `LAYOUT` `ORDER`
  - Today: Tap [FX] until / the screen says DELAY.
  - Do: press FX
  - Until: page is DELAY
  - Robot: press FX, press FX, press FX, press FX
- **Step 11, strip top** · `NAMES`
  - Today: [KNOB1]: the echo time.
  - Do: turn KNOB 1 left 1
  - Page: DELAY
- **Step 12, strip top** · `NAMES`
  - Today: [KNOB2]: more repeats.
  - Do: turn KNOB 2 right 5
  - Page: DELAY
- **Step 14, strip** · `LAYOUT`
  - Today: Hold [FX] + hold [OCT+].
  - Do: hold FX, press OCT+
- **Step 15, strip** · `LAYOUT`
  - Today: Press [HOME].
  - Do: press HOME
  - Until: page is TRACKS

## Lesson 20: Color a track

- **Step 4, strip** · `ORDER`
  - Today: Tap [FX] until / the screen says INSERT.
  - Do: press FX
  - Until: page is INSERT
  - Robot: press FX, press FX, press FX, press FX, press FX, press FX, press FX
- **Step 13, strip top** · `NAMES`
  - Today: [FX] and [KNOB1] left: / FILT goes past LP 40.
  - Do: hold FX, turn KNOB 1 left 5
  - Until: master filter below -40
  - Hint: Hold [FX]. FILT is / the first dial below.
- **Step 14, strip top** · `NAMES`
  - Today: [FX] and [KNOB1] right / until FILT says OFF.
  - Do: hold FX, turn KNOB 1 right 3
  - Until: master filter is 0
- **Step 15, strip top** · `NAMES`
  - Today: Keep turning right: / FILT goes past HP 40.
  - Do: hold FX, turn KNOB 1 right 5
  - Until: master filter above 40
- **Step 16, strip top** · `NAMES`
  - Today: [FX] and [KNOB1] left / until FILT says OFF.
  - Do: hold FX, turn KNOB 1 left 3
  - Until: master filter is 0
- **Step 17, strip** · `LAYOUT`
  - Today: Press [HOME].
  - Do: press HOME
  - Until: page is TRACKS

## Lesson 21: Shape a sound

- **Step 4, strip** · `ORDER`
  - Today: Press [ENV].
  - Do: press ENV
  - Until: page is ENV
- **Step 11, strip top** · `ORDER`
  - Today: Tap [EDIT] until FLT.
  - Do: press EDIT
  - Until: page is FILTER
  - Robot: press EDIT, press EDIT, press EDIT, press EDIT, press EDIT, press EDIT
- **Step 16, strip** · `LAYOUT`
  - Today: Press [HOME]. / Then [PLAY].
  - Do: press HOME, press PLAY
  - Until: page is TRACKS and playing

## Lesson 22: Wobble and arps

- **Step 4, strip** · `ORDER`
  - Today: Tap [LFO] until / it says LFO DEST.
  - Do: press LFO
  - Until: page is LFO 2
  - Robot: press LFO, press LFO, press LFO
- **Step 7, strip** · `LAYOUT` `ORDER`
  - Today: Tap [LFO] until / it says LFO 1/3.
  - Do: press LFO
  - Until: page is LFO
  - Robot: press LFO, press LFO, press LFO
- **Step 10, strip** · `LAYOUT`
  - Today: [ALGORITHM] to track 2.
  - Do: turn ALGORITHM left 2
  - Until: track is lead
- **Step 11, strip** · `LAYOUT` `ORDER`
  - Today: Press [ARP].
  - Do: press ARP
  - Until: page is ARP
- **Step 13, strip** · `LAYOUT`
  - Today: Hold the three / blinking keys together.
  - Do: hold A3, hold C4, hold E4
  - Until: played 3 keys
  - Page: ARP
- **Step 17, strip** · `LAYOUT`
  - Today: Hold the same / three keys again.
  - Do: hold A3, hold C4, hold E4
  - Until: played 3 keys
  - Page: ARP
- **Step 19, strip** · `LAYOUT`
  - Today: Hold other keys. / Try chords. / [OCT+] when done.
- **Step 20, strip** · `LAYOUT`
  - Today: Press [HOME].
  - Do: press HOME
  - Until: page is TRACKS

## Lesson 23: Your own piece, live

**Stopped on 3.4.2:** both runs, step 4 (the copy of a page: SAVE).

- **Step 3, card** · `SAVE` `GRIDSEQ`
  - Today: _Your show controls / Page: [SAVE] + a key. / Fill note: / [SEQ] + a key + [OCT+]. / Fill: [GLO] + [12]. / Effects: [FX] + a key.
- **Step 4, strip** · `SAVE`
  - Today: Hold [SAVE], hold [2]. / Let go when / the top says COPY.
  - Do: hold SAVE, long box 2
  - Until: 2 patterns or more
  - Hint: A short tap only goes / to page 2. Hold longer.
  - Robot: hold SAVE, long box 2, let go, wait 1.5
- **Step 5, strip** · `GRIDSEQ` `KEEP`
  - Today: You are on page 2. Change it. / [OCT+] when ready.
  - Robot: tap box 1, hold SEQ, tap box 3, let go
- **Step 6, card** · `GRIDSEQ`
  - Today: _Three at once / Hold [SEQ], hold a key, / then press [OCT+]. // Use both hands. Keep / the first two down.
- **Step 7, strip** · `GRIDSEQ`
  - Today: Now mark one note / as a fill note.
  - Until: drums has 1 fill notes or more
  - Hint: Hold [SEQ] + hold a key, then press [OCT+] once.
  - Robot: tap box 11, hold SEQ, hold box 12, press OCT+, let go
- **Step 12, strip** · `SAVE` `HOME`
  - Today: Save it: / press [SAVE] twice.
  - Do: press SAVE twice
  - Until: says PROJECT SAVED

## Lesson 25: Chain the pages

**Stopped on 3.4.2:** both runs, step 3 (SAVE opens SONG 2/3).

- **Step 3, strip** · `SAVE`
  - Today: Press [SAVE] once: / the SONG page.
  - Do: press SAVE
  - Until: page is SONG
  - Hint: One tap only. / Two quick taps save.
- **Step 5, strip** · `KEEP`
  - Today: Keys are pages here. / Tap [KEY 3].
  - Do: tap box 3
  - Until: drums on pattern 3
  - Page: SONG
- **Step 6, strip** · `KEEP`
  - Today: [1]: back to the intro.
  - Do: tap box 1
  - Until: drums on pattern 1
  - Page: SONG
- **Step 8, strip** · `CHAIN`
  - Today: Tap [15]: CHAIN.
  - Do: tap box 15
  - Until: says CHAIN
  - Page: SONG
- **Step 11, strip** · `CHAIN`
  - Today: [15] again: / the chain ends.
  - Do: tap box 15
  - Until: says CHAIN DONE
  - Page: SONG
- **Step 20, card** · `SAVE` `CHAIN` `LINE`
  - Today: SONG page: [SAVE]. / Chain: [15]. / Record a song: [14]. / Play it: [13].

## Lesson 26: Write it line by line

**Stopped on 3.4.2:** both runs, step 3 (SAVE opens SONG 2/3).

- **Step 3, strip** · `SAVE`
  - Today: Press [SAVE] once: / the SONG page.
  - Do: press SAVE
  - Until: page is SONG
  - Hint: One tap only.
- **Step 7, card** · `KEEP`
  - Today: Four lines, / all playing page 1. // You are on line 4. / Make it page 4, / the break.
- **Step 8, strip top** · `KEEP`
  - Today: [KNOB2], PAT: P4.
  - Do: turn KNOB 2 right 1
  - Until: song line 4 is pattern 4
  - Page: SONG 2
  - Hint: LINE must say 4/4.
- **Step 10, strip top** · `KEEP`
  - Today: [KNOB2], PAT: P3.
  - Do: turn KNOB 2 right 1
  - Until: song line 3 is pattern 3
  - Page: SONG 2
  - Hint: LINE must say 3/4.
- **Step 13, strip top** · `KEEP`
  - Today: [KNOB2], PAT: P2.
  - Do: turn KNOB 2 right 1
  - Until: song line 2 is pattern 2
  - Page: SONG 2
  - Hint: LINE must say 2/4.
- **Step 19, card** · `SAVE`
  - Today: _Song lines / Get there: [SAVE], / then [SELECT] right. // Each line is a part: / a page and its bars.

## Lesson 27: Build-ups and drops

- **Step 4, strip top** · `NAMES`
  - Today: [FX] and [KNOB1] right until FILT says OFF.
  - Do: hold FX, turn KNOB 1 right 3
  - Until: master filter is 0
  - Hint: Hold [FX]. FILT is / the first dial below.

## Lesson 28: Swing and feel

- **Step 7, card** · `HITS`
  - Today: Soft and hard hits / make a beat groove. // A drummer never hits / twice the same way. // If every hit is the same, / it sounds like a robot.
- **Step 9, strip** · `GRID`
  - Today: Play [5]: the hat.
  - Do: tap box 5
  - Page: STEPS
- **Step 10, card** · `GRIDSEQ` `GRIDKN`
  - Today: _Three at once / Hold [SEQ], hold [4], / then turn [KNOB2]. // Use both hands. Keep / the first two down.
- **Step 11, strip top** · `GRIDSEQ` `GRIDKN` `HITS`
  - Today: Hold [SEQ] + [4]: / [KNOB2] to HARD.
  - Do: hold SEQ, hold box 4, turn KNOB 2 right 2
  - Until: drums has 1 hard notes or more
  - Hint: Keep holding both. LEVEL is the 2nd dial.
- **Step 12, strip top** · `GRIDSEQ` `HITS`
  - Today: Now [8], [12], [16].
  - Do: hold SEQ, hold box 8, turn KNOB 2 right 2, let go, hold SEQ, hold box 12, turn KNOB 2 right 2, let go, hold SEQ, hold box 16, turn KNOB 2 right 2
  - Until: drums has 4 hard notes or more
  - Hint: [SEQ], hold the box, / [KNOB2] right. / One box at a time.
- **Step 14, card** · `HITS`
  - Today: Playing by hand has / soft and hard too. // Hold [OCT-] and play / a drum: soft. / Hold [OCT+]: hard.
- **Step 15, strip** · `GRID`
  - Today: Hold [OCT-], play [3].
  - Do: hold OCT-, tap box 3
- **Step 16, strip** · `GRID`
  - Today: Now hold [OCT+], / play it again.
  - Do: hold OCT+, tap box 3
- **Step 18, card** · `GRIDSEQ` `GRIDKN` `HITS`
  - Today: _Swing and groove / Swing: [KNOB1] on / the TRACKS page. / Loud or soft box: / [SEQ] + a box, [KNOB2]. / Soft hit: hold [OCT-]. / Hard hit: hold [OCT+].

## Lesson 29: Keep it and share it

- **Step 3, card** · `SAVE` `HOME`
  - Today: Hold [HOME]: / the PROJECT page. // [1]: OPEN. / [3]: NEW. / [5]: SAVE.
- **Step 4, strip** · `SAVE` `HOME`
  - Today: Hold [HOME], / tap [5]: SAVE.
  - Do: hold HOME, tap box 5
  - Until: says PROJECT SAVED
- **Step 9, card** · `HOME`
  - Today: OPEN brings back / what you saved. // It asks if you are sure, / so you tap twice.
- **Step 15, card** · `SAVE` `HOME`
  - Today: _Keep your song / Save: hold [HOME] + [5], / or [SAVE] twice. / Open: hold [HOME] + [1], / then [1] again. // More songs: as files, / with the web editor.

## Lesson 30: The final test

- **Step 4, card** · `GRID`
  - Today: _Your first beat / Play, stop: [PLAY]. / Pick a drum: play its key. / Boxes: [SEQ] + a key. / Tempo: [SELECT]. / Undo: [EDIT] + [OCT-]. / Mute: [GLO] + [1] to [5].
- **Step 5, card** · `SEL`
  - Today: _Notes and chords / Track: [ALGORITHM]. / Sound: [PRESETS]. / Record live: [REC]. / Key and chords: [SEL]. / Piano roll: [SEQ] twice. / Level: [KNOB2].
- **Step 7, card** · `SAVE` `CHAIN` `LINE`
  - Today: _Make a song / Page: [SAVE] + a key. / Copy: [SAVE], hold a key. / SONG page: [SAVE]. / Chain: [15]. / Record a song: [14]. / Play the song: [13].
- **Step 8, card** · `SAVE`
  - Today: When your song is done, / press [SAVE] twice. // The corner of the / screen reminds you: / [SAVE] x2
- **Step 9, strip corner** · `SAVE`
  - Today: [SAVE] x2
  - Until: pressed SAVE twice
  - Robot: press SEQ, tap A3, hold SEQ, tap box 1 5, let go, turn ALGORITHM right 1, tap A4, hold SEQ, tap box 3, let go, turn ALGORITHM right 1, tap C5, hold SEQ, tap box 1, let go, turn ALGORITHM right 2, tap box 1, hold SEQ, tap box 1 5 9 13, let go, press HOME, hold SAVE, long box 2, let go, wait 2, hold SAVE, long box 3, let go, wait 2, press SAVE, wait 0.8, turn SELECT right 1, turn KNOB 4 right 3, wait 0.3, press HOME, wait 0.6, press SAVE twice, wait 1.5
