# Lesson 1: Hello

- **Part:** 1, your first beat
- **The idea:** a blinking light means "press this". SLOOP has five tracks.
- **The learner does:** starts and stops a song with PLAY, plays the melody game, and meets a run nobody can play by hand.
- **Got it when:** they start and stop the song without help.
- **Song:** HOUSE ONE, section C (the whole band).
- **Phrase RUN:** A3 C4 E4 A4 C5 E5 C5 A4 E4 C4 A3 C4 E4 A4 C5 E5 C5 A4 E4 C4 A3 E4 A4 E5

## Steps

1. **Setup:** song HOUSE ONE section C; track lead
2. **Card:** A blinking light means: / press this. // Try it now.
   - **OCT+:** Go
3. **Card:** _How to read the cards / A name with a border is / a button or knob: [OCT+] / Press: down and up. / Hold: keep it down. / [OCT+] is blinking. // Press it.
   - **OCT+:** Next
4. **Card:** A number with a border / is a white key. / Count from the left. / [KEY 5], or just [5], / is blinking. // Tap it.
   - **Echo:** box 5
5. **Reply:** Good.
6. **Card:** This picture is / a black key: [BLACK 2] / Count from the left too. / [BLACK 2] is blinking. // Tap it.
   - **Echo:** black 2
7. **Reply:** Good.
8. **Card:** _Your FM-1 / [MASTER] is the volume. // We recommend using / headphones. / The FM-1 has good / sound, but the speaker / is not great.
   - **OCT+:** Next
9. **Card:** SLOOP is a groovebox: / 5 tracks, a sequencer, / synths, effects, a mixer. / 153 sounds, 37 drum kits. // Write beats, record live, / add effects, mix, save.
   - **OCT+:** Next
10. **Strip:** SLOOP! Press [PLAY].
    - **Do:** press PLAY
    - **Until:** playing
11. **Strip:** 5 tracks, 1 song. Listen.
    - **Wait:** 4 bars
12. **Strip:** Press [PLAY] to stop.
    - **Do:** press PLAY
    - **Until:** stopped
13. **Reply:** Good.
14. **Card:** _Repeat after me / Listen. Then play it back. // The blinking key shows / which key to press next.
    - **Play:** A4 C5 E5
    - **Echo:** A4 C5 E5
15. **Reply:** Good.
16. **Card:** _Repeat after me / Listen. Then play it back.
    - **Play:** A4 B4 C5 D5 E5
    - **Echo:** A4 B4 C5 D5 E5
17. **Reply:** Good.
18. **Card:** _One more / Listen. / Then play it back.
    - **Play:** RUN fast
19. **Card:** _Your turn / Play it back.
    - **Echo:** A3 C4
20. **Big:** Faster / Faster / FASTER
    - **Keys:** silent
    - **Do:** tap E4 or any key
21. **Play:** RUN fast
    - **Keys:** silent
22. **Reply:** Nice.
23. **Card:** Ha, sorry, I'm just / messing with you. / That was way too fast. / Nearly impossible / to play by hand. // But a sequencer can.
    - **OCT+:** Next

## Screens

The lesson's cards and strips in order, from the robot's run on the simulated FM-1.

![The screens of this lesson](screens/01-hello.png)

## Notes for the AI assistant

- The first two keys of the fast run are real: the learner plays them and hears them. From FASTER on, the keys are
  silent and any key sets the run off, so it sounds as if the learner played it.
- If the learner laughs or looks puzzled at the fast run, that is the point. Don't explain the joke before step 22.
- Step 6 shows the picture the cards use for a black key: two white keys in outline and the black one between them,
  filled, with its number. `[BLACK 2]` in this file is that picture and a 2: the second black key from the left (there
  are eleven). Here it plays a note of the lead. From the drum lessons on, on the drum page of boxes, a black key
  picks the drum.
- Lesson 22 pays this off: the arpeggiator plays the same run.

## Tester notes

2026-11-20, the round from the notes ("go back and make adjustment based on all of the notes").

- A new card, step 3, right after the blinking light: `[PLAY] IS A BUTTON. / [KEY 5] OR JUST [5]: / WHITE KEY 5
  FROM THE LEFT. / PRESS OR TAP: DOWN AND UP. / HOLD: KEEP IT DOWN.` Asked for the same day: "in the very beginning
  we should have one card that explains the buttons and holding and pressing and also what we are going to call
  them." A first draft. 18 steps: from old step 3 on, add one.
- The same evening, after seeing it: "I think we should blink key 5, also PLAY is a button, maybe that should be
  [THIS] is a button. That page is a little confusing, it's also very early in the tutorial so I think [we] should
  make it so there is no confusion. If it doesn't fit on one page we should make two." Now two cards, each with
  something to press at once, so that a blinking light still means "press this":
  - step 3, buttons: `A NAME IN A FRAME / IS A BUTTON: [OCT+] / PRESS: DOWN AND UP. / HOLD: KEEP IT DOWN. / [OCT+] IS
    BLINKING. / PRESS IT.` The example is OCT+, the button that is blinking on the card anyway. PLAY is not
    used here: a blinking PLAY would be pressed and start the song under the card.
  - step 4, keys: `A NUMBER IN A FRAME / IS A WHITE KEY. / COUNT FROM THE LEFT. / [KEY 5], OR JUST [5], / IS BLINKING. /
    TAP IT.` Key 5 blinks and the step ends when it is tapped (an Echo of one key), then `GOOD.`
  20 steps: from the old step 4 on, add two.

2026-11-25, the move to SLOOP 3.4.2.

- Nothing in this lesson touches what SLOOP 3.4.2 changed: it played through both ways as it was, and its counts (153
  sounds, 37 drum kits) are 3.4.2's too.
- A new card, step 6, and its `Good.`, step 7: `This picture is / a black key: [BLACK 2] / Count from the left too. /
  [BLACK 2] is blinking. // Tap it.` On 3.4.2's drum grid a black key picks the drum, so the cards need a way to name
  one: a small picture of a black key, with its number. Of the picture: "The first one shows three keys but its
  easier to see that they are keys ... at the beginning we can just tell the user that this is supposed to be a black
  key". Put here, with the cards' other rules (a name with a border, a number with a border), and tapped at once, as
  the white key is ("Yes, add it"). 23 steps: from old step 6 on, add two.
