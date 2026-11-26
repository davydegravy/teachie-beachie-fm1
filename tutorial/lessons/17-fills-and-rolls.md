# Lesson 17: Fills and rolls

- **Part:** 3, effects and sound
- **The idea:** a fill says "something new is coming".
- **The learner does:** plays a fill by hand, marks a note so it only plays in a fill, and plays a drum roll.
- **Got it when:** they drop a fill in at the end of a bar, on purpose.
- **Song:** HOUSE ONE, section C, its first bar. Four snares have been added that only play in a fill.
- **Style in the end:** trap (hat rolls and fills)
- **Tied to the song:** the fill snares in boxes 13 to 16, and box 12 for the learner's own.

## Steps

1. **Setup:** song HOUSE ONE section C, 1 bar; drums fill SNARE on 13 14 15 16; track drums; pick KICK; page STEPS
2. **Card:** A fill is a short / burst of drums. // It says: something / new is coming.
   - **OCT+:** Go
3. **Strip:** Press [PLAY].
   - **Do:** press PLAY
   - **Until:** playing
4. **Card:** This beat hides four / snares, on [13] to [16]. // They only play / in a fill.
   - **OCT+:** Next
5. **Strip:** Hold [GLO] and hold [11]: / the snares play.
   - **Do:** hold GLO, long box 11
   - **Until:** fill on
6. **Strip:** Again, when the light / reaches [13].
   - **Do:** hold GLO, long box 11
   - **Until:** fill on
7. **Card:** [GLO] + [12] is easier. // It plays the fill for / one whole bar, then / stops by itself.
   - **OCT+:** Next
8. **Strip:** Hold [GLO], tap [12].
   - **Do:** hold GLO, tap box 12
   - **Wait:** 2 bars
9. **Card:** Now a fill note / of your own. // A note can be marked / to play only in a fill.
   - **OCT+:** Next
10. **Strip:** Let go of [GLO]. / Pick the high tom: tap / [BLACK 7] until TOM H.
    - **Do:** tap black 7
    - **Until:** drums picked TOM H
    - **Hint:** Tap [BLACK 7] until / the top says TOM H.
    - **Page:** STEPS
11. **Strip:** Tap [12]: a tom in box 12.
    - **Do:** tap box 12
    - **Until:** drums has TOM H on box 12
    - **Page:** STEPS
12. **Card:** _The note's sheet / Hold [OCT+]: a sheet / opens for the tom in / box 12. // [KNOB1] picks a line. / [OCT+] changes it. / [OCT-] closes the sheet.
    - **OCT+:** Next
13. **Strip:** Hold [OCT+] until / the top says SHEET.
    - **Do:** hold OCT+, wait 1
    - **Until:** says SHEET
    - **Page:** STEPS
14. **Strip top:** Turn [KNOB1] until / LINE says FILL. / Then [OCT+] once.
    - **Do:** turn KNOB 1 right 2, press OCT+
    - **Until:** drums has 5 fill notes or more
    - **Hint:** [KNOB1]: LINE says FILL. / Then press [OCT+] once.
    - **Hint:** Top says NO FILL? / [OCT+] twice more.
    - **Page:** STEPS
15. **Card:** FILL. It worked. // The tom now plays / only in a fill.
    - **OCT+:** Next
16. **Strip top:** [OCT-] closes the sheet.
    - **Do:** press OCT-
    - **Page:** STEPS
17. **Strip:** Hear it: hold [GLO], / tap [12].
    - **Do:** hold GLO, tap box 12
    - **Wait:** 2 bars
18. **Card:** A roll is one drum, / repeated fast. // Hold [ARP]: the white / keys play the drums. / Hold one: it rolls.
    - **OCT+:** Next
19. **Strip:** Hold [ARP], hold [5]: / a hat roll.
    - **Do:** hold ARP, long box 5
20. **Strip top:** Speed: [ARP] + [KNOB1]. / Hear it: [ARP] + [5]. / [OCT+] when done.
    - **OCT+:** Done
    - **Robot:** hold ARP, turn KNOB 1 right 1, long box 5, let go
21. **Strip:** Press [PLAY] to stop.
    - **Do:** press PLAY
    - **Until:** stopped
22. **Card:** _Fills and rolls / Fill: [GLO] + [11] or [12]. / Fill note: hold [OCT+], / LINE to FILL, [OCT+]. / Roll: [ARP] + a key.
    - **OCT+:** Done

## Screens

The lesson's cards and strips in order, from the robot's run on the simulated FM-1.

![The screens of this lesson](screens/17-fills-and-rolls.png)

## Notes for the AI assistant

- A fill does nothing unless some notes are marked FILL. That is why the lesson hides four snares in the beat
  first.
- With [GLO] held the screen is the MIX page: key 11 is the fill while it is held (its FILL tile lights), key 12 the
  fill for one whole bar. The other keys there mute and solo, which is why step 10 starts with "Let go of [GLO]".
- The fill note is made on the note's sheet (SLOOP 3.4.2), one move at a time, with nothing held while something
  else is pressed (steps 10 to 16):
  - `[BLACK 7]` picks the tom: TOM L on the first tap, TOM H on the second. The top of the screen names it.
  - A tap on key 12 puts the tom in box 12, in silence, and leaves the cursor there.
  - [OCT+] held for half a second opens the sheet of the note under the cursor. The top says `SHEET 12 TOM H`; the
    bottom shows LINE, the line's value, OCT+ and OCT- CLOSE.
  - [KNOB1] picks the line: RATCHET, NUDGE, FILL, MOVE, COPY, DELETE. FILL is two clicks to the right.
  - [OCT+] changes the line shown. On FILL each press moves on: NORMAL, FILL (`FILL: ONLY IN A FILL`), NO FILL, and
    round again. Pressed twice it says NO FILL, the note plays except in a fill; two more presses bring FILL back.
  - [OCT-] closes the sheet.
- The sheet has lines past FILL: MOVE, COPY, DELETE. [OCT+] on one of those changes the note itself. If the tom is
  gone or has moved: [OCT-] to close, tap [12] again with TOM H picked, and start again from step 13. The FM-1's
  SHOW ME only moves the lesson on here; it does not make the mark.
- The sheet belongs to the box tapped last. If the learner tapped another box after key 12, the sheet is that
  box's: the top names the box and the drum.
- The card in step 15 takes [OCT+] for itself, so a press too many does not move FILL on to NO FILL. The sheet is
  still open under it; step 16 closes it.
- Marking a box marks every drum in it: the hat on box 12 becomes a fill note with the tom. That is SLOOP's rule.
- A step that waits for a fill or an effect does not go by on one still held from the step before: the learner
  has to let go and do it again.
- The lesson cannot see key 12's one-bar fill, only that the key was pressed.
- With [ARP] held on the drums the screen is the ROLL page: the drums' names on sixteen tiles. The white keys play
  the drums there and write no box; one held rolls. RATE, the roll's speed, is [KNOB1]. The roll is live: it is
  heard, and it is written down only if REC is on.

## Tester notes


2026-11-19, on the real FM-1.

- **Step 10, the fill note, was not understood.** "Keep SEQ and the box down. The top says FILL. Can you explain
  this, I am stuck." The step ran 134 seconds: the hint, STUCK? twice (TRY AGAIN, then SHOW ME). What went wrong
  with the hands is not in the log. Three things make it hard:
  - It is three holds at once and in order (SEQ, then the box, then OCT+), the only step so far that asks that.
    Letting the box go before OCT+ does nothing, and nothing on the screen says why.
  - The hint reads as a statement ("THE TOP SAYS FILL") when it means "when it works, the top of the screen
    shows FILL". It does not say what to do differently.
  - A second press of OCT+ goes past FILL to NO FILL, and the step then never ends.
  Ideas, none applied: split it in two steps (HOLD [SEQ] AND HOLD BOX 12, then KEEP BOTH DOWN, PRESS [OCT+] ONCE);
  hint `PRESS [OCT+] ONCE WHILE / BOTH ARE HELD. THE TOP / OF THE SCREEN SHOWS FILL.`; let the step also end,
  with a message, when the box says NO FILL.
- **Marking the box marks every drum in it.** After the step, box 12 held the high tom and the hat that was
  already there, both FILL: the hat on box 12 no longer plays outside a fill. That is SLOOP's rule (the mark is
  the box's). An empty box would avoid it; in this beat every box holds something, so the lesson could clear
  box 12 in its setup.
- **Second play, the same day: step 10 again, 154 seconds.** The hint, STUCK? twice (TRY AGAIN both times), then
  done by hand after the step was explained in chat. "What does the box down mean?" The words that failed are
  THE BOX (the two steps before call the same key KEY 12) and DOWN (for "still held").
- **Their wording:** "Hold Glo + key2 + Oct+", then "hold Glo+ Hold key12 + press OCT+". Read here as the form they
  wants for this step, with SEQ meant ("yes I meant seq"): each thing with its own verb, joined by +, and KEY 12
  where the strip says BOX 12. A strip in that form:
  `HOLD [SEQ] + / HOLD KEY 12 + / PRESS [OCT+] ONCE.` Not applied, not measured yet.
  They wrote GLO both times. If GLO is what the hand goes to, that is a finding too: steps 5, 7 and 11 are all
  [GLO] and key 12, and this one is [SEQ] and the same key, so the step could say "THIS TIME [SEQ], NOT [GLO]".
- **After the step ended the note went on to NO FILL.** At the lesson's end boxes 11 and 12 (open hat, high tom,
  hat) all said NO FILL: OCT+ was pressed again after the step had ended, with key 11 held too at some point.
  The step ends the instant FILL is reached, while the fingers are still down, and nothing says to stop.
- **To revisit: "it is still a bit confusing", the most confusing lesson so far.** Their idea: "introduce the idea
  of holding multiple buttons, and that sometimes you might need to hold in two buttons and then press a
  third." This is the first move in the course that takes three fingers; everything before is one hold and
  one tap. A sketch, nothing applied:
  - a card before the step: `SOME MOVES TAKE THREE FINGERS. / HOLD TWO THINGS DOWN, / THEN PRESS A THIRD.`
  - the move in two steps: first `HOLD [SEQ] + HOLD KEY 12.` (the tom goes in), then
    `KEEP BOTH DOWN. / PRESS [OCT+] ONCE.` To check: whether the lesson can see that the key is still held.
  - when FILL is reached: `FILL. LET GO NOW.`, so that no further press moves it on to NO FILL.
  - worth saying once, early in the course, as a rule of the FM-1 (hold one thing, press another), and here
    as its bigger form. Lessons 23 and 27 come back to the same move and take the same wording.

2026-11-20, the hidden-screen pass ("Yes, fix them"). Only the two steps that hid the screen; the rework of the
fill note above is still open.

- Old step 4 is a card with an answer and then the strip `HOLD [GLO] AND HOLD WHITE KEY 11.` With GLO held the
  screen is the MIX page, and its FILL tile lights while the fill is on: that was under the card.
- The roll's speed (old step 14) is a strip at the top: `[ARP] + KNOB 1: RATE, / HOW FAST IT ROLLS.` With ARP
  held the screen is the ROLL page, with one dial, RATE (1/16, 1/32 ...), at the bottom left.
- 17 steps: from old step 5 on, add one to the numbers above (the fill note is step 11 now).

2026-11-20, third play on the real FM-1 (the steps as renumbered: the fill note is step 11).

- **Stuck at the fill note again, more than four minutes:** the hint, then STUCK? three times, TRY AGAIN each time.
  "It is asking me to put a fill on 12 but when I press 12 it turns to crash, no longer fill."
  - What the FM-1 held at that moment: box 12 with its hat only, nothing written. So SEQ was not down when key
    12 went down: the key then plays its drum, the crash, and the drum's name at the top right changes to
    CRASH.
  - They were reading the top of the screen for FILL, as the hint says (`THE TOP SAYS FILL`), and saw CRASH there.
    The hint never says that hearing a drum means SEQ is not held.
  - The steps before use the same key with GLO for a fill ("fill on 12"); here the word FILL is on the screen
    only after OCT+, and nothing says what the screen shows on the way (with SEQ and the key down, the top
    says `STEP 12` and `ALWAYS`).
  - For the rework: say what each stage looks like. `HOLD [SEQ] + HOLD KEY 12.` (the top says STEP 12, no drum
    sounds), then `KEEP BOTH DOWN. / PRESS [OCT+] ONCE.` (the top says FILL), and a hint for the wrong turn:
    `HEAR A CRASH? [SEQ] WAS NOT DOWN. HOLD [SEQ] FIRST.`
- **The same play, what the log shows.** The first time through, the fill note took 292 seconds and was ended by
  SHOW ME. They played the lesson again at once (AGAIN on the DONE card):
  - The fill note was done by hand in 40 seconds, after the order was spelled out in chat (SEQ first and kept
    down, then the key, then OCT+ once).
  - They did not know it had worked: "I'm still having issues with the step when I am supposed to hold seq hold
    key 12 and then press Oct+." The step ended the instant FILL was reached, with no word, and the next strip,
    `HEAR IT: [GLO] AND KEY 12.`, looks like the same move. That step then took 96 seconds and was ended by
    SHOW ME.
  - They went on doing the move: box 13 was marked too (its kick, clap and snare all FILL), so the beat lost a
    kick outside the fill.
  - So the rework needs a plain stop after the mark (`FILL. LET GO NOW.` as a reply, or a card with an answer),
    and the next step has to say that the hand changes: `NOW [GLO], NOT [SEQ]: / HOLD [GLO], TAP KEY 12.`
  "I know there was a problem with this before, did we fix it?" No: only the two steps that hid the screen.

2026-11-20, the round from the notes ("go back and make adjustment based on all of the notes").

- The fill note is rebuilt from the sketch above (steps 10 to 15): `PICK THE HIGH TOM: / PLAY [11].`; a card `THE
  NEXT MOVE TAKES THREE FINGERS. / HOLD TWO THINGS DOWN, / THEN PRESS A THIRD.`; `HOLD [SEQ] + HOLD [12]. / THE TOP
  SAYS STEP 12.` (it ends when the tom is in the box; hint `HEAR A CRASH? [SEQ] WAS NOT DOWN. HOLD [SEQ] FIRST.`);
  `KEEP BOTH DOWN. / PRESS [OCT+] ONCE.` (hints: `[SEQ] + [12] DOWN, THEN [OCT+] ONCE. THE TOP SAYS FILL.` and `TOP
  SAYS NO FILL? [OCT-] PUTS IT BACK. THEN [OCT+] ONCE.`); a card with an answer, `FILL. IT WORKED. / LET GO OF
  EVERYTHING. / THE TOM NOW PLAYS ONLY IN A FILL.`, which takes OCT+ so a press too many does nothing; then `NOW
  [GLO], NOT [SEQ]: / HOLD [GLO], TAP [12].`
- The two fills in a row (steps 5 and 6) can no longer be ended by one long hold: a step that waits for a fill does
  not go by on one still held.
- Still as it was: marking box 12 also marks the hat in it. 20 steps.

2026-11-26, the move to SLOOP 3.4.2. Its goal is the same: a fill by hand, a note that plays only in a fill, a roll.
The lesson stopped at the fill note both ways: on 3.4.2's drum grid a white key writes a box, so key 11 no longer
picks the tom, and the three-finger move ([SEQ] + a key + [OCT+]) is gone. The mark is on the note's sheet now.
Shown whole ("Yes, all of it").

- **The fill note, steps 10 to 17, with nothing held while something else is pressed.** This replaces the move the
  notes above call the most confusing of the course:
  - step 10 `Let go of [GLO]. / Pick the high tom: tap / [BLACK 7] until TOM H.` (was `... Pick the high tom: [11].`),
    hint `Tap [BLACK 7] until / the top says TOM H.`;
  - step 11 `Tap [12]: a tom in box 12.` (was `Hold [SEQ] + hold [12]. / The top says STEP 12.`);
  - step 12, a new card: `_The note's sheet / Hold [OCT+]: a sheet / opens for the tom in / box 12. // [KNOB1] picks a
    line. / [OCT+] changes it. / [OCT-] closes the sheet.` The card `_Three at once` is gone;
  - step 13 `Hold [OCT+] until / the top says SHEET.`;
  - step 14, along the top, the sheet's dials being at the bottom: `Turn [KNOB1] until / LINE says FILL. / Then
    [OCT+] once.` (was `Keep holding both. / Press [OCT+] once.`), hints `[KNOB1]: LINE says FILL. / Then press [OCT+]
    once.` and `Top says NO FILL? / [OCT+] twice more.`;
  - step 15 `FILL. It worked. // The tom now plays / only in a fill.` (`Let go of everything.` is gone);
  - step 16, along the top: `[OCT-] closes the sheet.`;
  - step 17 `Hear it: hold [GLO], / tap [12].` (was `Now [GLO], not [SEQ]: / ...`).
- **Step 18, the roll's card:** `A roll is one drum, / repeated fast. // Hold [ARP]: the white / keys play the drums. /
  Hold one: it rolls.` (was `... // Hold [ARP] and hold / a drum key.`). With [ARP] held the screen names the drums.
- **The last card:** `Fill note: hold [OCT+], / LINE to FILL, [OCT+].` (was `Fill note: / [SEQ] + a key + [OCT+].`).
- The setup picks the kick, so the top says KICK at the start. Still as it was: marking box 12 also marks the hat in
  it. 22 steps (were 20).
