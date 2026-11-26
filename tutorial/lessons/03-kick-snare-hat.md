# Lesson 3: Kick, snare, hat

- **Part:** 1, your first beat
- **The idea:** a beat is three simple layers.
- **The learner does:** adds the snare on boxes 5 and 13 and the hats between the kicks, picking each drum with
  its black key first.
- **Got it when:** they can name the three layers and pick a drum without being told which key.
- **Song:** HOUSE ONE, section C, its first bar. The drums start with only the four kicks from lesson 2.
- **Style in the end:** house, then hip-hop from lesson 4 on
- **Tied to the song:** the boxes named for the snare and the hats.

## Steps

1. **Setup:** song HOUSE ONE section C, 1 bar; drums empty; drums KICK on 1 5 9 13; mute bass lead chords pad; track drums; pick KICK; page STEPS
2. **Card:** Last lesson had / four kicks. / A beat has three layers: / kick, snare and hat. // Now let's take a look / at the other two.
   - **OCT+:** Go
3. **Strip:** Press [PLAY]. / Your four kicks.
   - **Do:** press PLAY
   - **Until:** playing
4. **Card:** Each black key is / a different drum. / Tap one to pick it. / The selected drum / can then be put / into the boxes.
   - **OCT+:** Next
5. **Card:** The top names / the selected drum. / Now it says KICK. // Some black keys hold / two drums. Tap again / for the other one.
   - **OCT+:** Next
6. **Strip:** Pick the snare: tap [BLACK 2].
   - **Do:** tap black 2
   - **Until:** drums picked SNARE
   - **Hint:** Tap [BLACK 2] until / the top says SNARE.
   - **Page:** STEPS
7. **Strip:** Now tap [5] and [13].
   - **Do:** tap box 5 13
   - **Until:** drums has SNARE on boxes 5 13
   - **Hint:** Tap each blinking key.
8. **Reply:** Good.
9. **Card:** Every 4 boxes is a beat. / Kick on every beat. / Snare on beats 2 and 4: / the backbeat. / Most songs you know / have a backbeat. *(beginners only)*
   - **OCT+:** Next
10. **Strip:** Pick the hat: tap [BLACK 5].
    - **Do:** tap black 5
    - **Until:** drums picked HAT
    - **Hint:** Tap [BLACK 5] until / the top says HAT.
    - **Page:** STEPS
11. **Strip:** Hats on [3], [7], [11], [15]: / between the kicks.
    - **Do:** tap box 3 7 11 15
    - **Until:** drums has HAT on boxes 3 7 11 15
    - **Hint:** Tap each blinking key.
12. **Reply:** Nice.
13. **Card:** The grid has a row / for each drum. // The keys for the selected / drum will light up. // [KNOB1] picks one too.
    - **OCT+:** Next
14. **Card:** More hats? / One in every odd box / makes it rush along. // Write them, or / have them written.
    - **OCT-:** I will
    - **OCT+:** Finish it → drums HAT on 1 5 9 13
15. **Strip:** Hats on [1], [5], / [9] and [13] too.
    - **Do:** tap box 1 5 9 13
    - **Until:** drums has HAT on boxes 1 3 5 7 9 11 13 15
    - **Hint:** Tap each blinking key.
    - **Robot:** tap box 1 5 9 13
16. **Setup:** unmute; play
17. **Strip:** Kick, snare, hat. / And the band.
    - **Wait:** 4 bars
18. **Strip:** Wrong box? Tap it again. / [OCT+] when done.
    - **OCT+:** Done
19. **Strip:** Press [PLAY] to stop.
    - **Do:** press PLAY
    - **Until:** stopped
20. **Card:** Kick, snare, hat. / Three layers, / one drum beat. // And you made it.
    - **OCT+:** Done

## Screens

The lesson's cards and strips in order, from the robot's run on the simulated FM-1.

![The screens of this lesson](screens/03-kick-snare-hat.png)

## Notes for the AI assistant

- The one trap: the boxes write the drum selected, the one named at the top of the grid. If the kick is selected and
  the learner taps box 5, the kick in box 5 is removed. The lesson notices a wrong drum by itself: it says `Another
  drum is picked. / Tap the blinking key.` and blinks the right drum's black key, not the boxes. If kicks have gone
  missing, say so and help put them back (`[BLACK 1]`, then the box).
- On the grid a black key picks the drum and makes no sound; a white key is a box (SLOOP 3.4.2). The picture in
  `[BLACK 2]` is a black key, the second from the left; lesson 1 introduced it.
- Five black keys hold two drums in turns: `[BLACK 1]` KICK and KICK2, `[BLACK 2]` SNARE and SNR 2, `[BLACK 5]` HAT and
  PEDAL, `[BLACK 7]` TOM L and TOM H, `[BLACK 10]` SHAKE and CONGA. Tapped again, the key gives the other; step 5's
  card says so, and the pick steps wait until the top names the right one.
- [KNOB1] on the grid (SOUND) goes through all 16 drums by name. Step 13's card names it once; the lessons use the
  black keys.
- "Backbeat" is the only new music word. Use it again in later lessons.
- If they choose I WILL in step 13 and then struggle, FINISH IT is still there next time round.

## Tester notes

2026-11-19, on the real FM-1. "Lesson 3 was great."

- **Say more plainly what picking a drum is.** "How to select the drum, the selection phase of picking the snare:
  be more explicit about what that step is." Today the strip only says `PLAY WHITE KEY 3: THE SNARE`. It does not
  say that playing a drum's key is what chooses the drum the boxes will write. Not changed yet.
- **Later in the lesson, say that a wrong box can be undone by tapping it again.** "If they pick the wrong step
  they can just press it again and it goes away." Lesson 2 says `A FULL BOX EMPTIES` once; lesson 3 never says
  it. Not changed yet.

2026-11-20, the round from the notes ("go back and make adjustment based on all of the notes").

- Both notes above are applied. A card before the snare says what picking is (`PLAYING A DRUM'S KEY PICKS THAT
  DRUM. / THE BOXES THEN WRITE IT. / SO FIRST PICK THE SNARE.`), and the strips say `PICK THE SNARE: / PLAY [KEY
  3].` and `NOW PICK THE HAT: / PLAY [5].`
- The free step near the end says `WRONG BOX? TAP IT AGAIN.` 19 steps: from old step 4 on, add one.

2026-11-25, the move to SLOOP 3.4.2. On its drum grid a black key picks the drum (in silence), a white key is a box
with no [SEQ] held, and playing a drum's white key there no longer picks it: this lesson stopped at step 6 when a
computer led it. The whole lesson was shown at once ("can you show me all of the next lesson all at once?").

- **Step 4, the explanation, kept** ("for step 4 we have what amounts to an explanatory step. I'm worried we lose that
  with this suggestion. maybe we can add a card and then have the suggestion?"): `Each black key is / a different drum.
  / Tap one to pick it. / The selected drum / can then be put / into the boxes.` Two words changed: "black key" for
  "key", "Tap" for "Play". The first suggestion had replaced it with a shorter card.
- **A new card after it, step 5:** `The top names / the selected drum. / Now it says KICK. // Some black keys hold /
  two drums. Tap again / for the other one.` Its second half is the course's author's: "if they hit the black key
  again its another drum right? shouldn't we mention that too?"
- **The pick steps, on one line** ("for that strip can it all fit on one line?"): `Pick the snare: tap [BLACK 2].` (was
  `Pick the snare: / play [KEY 3].`) and `Pick the hat: tap [BLACK 5].` (was `Now pick the hat: / play [5].`; with
  "Now" it is 223 px of 220). Each waits for the drum picked (`drums picked SNARE`) and has a hint: `Tap [BLACK 2]
  until / the top says SNARE.`
- **The boxes without [SEQ]:** `Now tap [5] and [13].`, `Hats on [3], [7], [11], [15]: / between the kicks.`, `Hats on
  [1], [5], / [9] and [13] too.`, `Wrong box? Tap it again. / [OCT+] when done.` Their hints are `Tap each blinking
  key.`: the old ones (`If the top says KICK, play [3] first.`) are the lesson's own doing now.
- **Step 13, the card about the grid,** one line more: `[KNOB1] picks one too.` (decision 2: [KNOB1] shown once as the
  other way).
- The setup picks the kick, so the lesson starts with KICK at the top. 20 steps: from old step 5 on, add one.
