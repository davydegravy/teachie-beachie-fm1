# Lesson 1: The frames

A test of the guide and the lesson player, not a lesson of the course: `tutorial/upgrade/framecheck.py` plays it, on
the FM-1's own course and with the computer leading. The framed names on a card or a strip answer the real controls:
a button's or a key's frame is filled while it is held; a knob's frame fills as far as the turn the step asks for is
done, from the left for a turn to the right and from the right for a turn to the left. A turn of a set number of
clicks fills by clicks (step 4: three, then two); a turn to a value fills as the value nears it (step 6: the tempo,
from where it stands to below 110). A name that is no control the firmware can see ([MASTER]) stays as it is.
In every step the robot does nothing (`idle`): the test presses and turns.

- **Part:** a test
- **The idea:** a framed name answers its control.
- **The learner does:** holds a button and a key, turns two knobs by clicks, turns the tempo down.
- **Got it when:** all three steps end.
- **Song:** HOUSE ONE, section C, its first bar.

## Steps

1. **Setup:** song HOUSE ONE section C, 1 bar; track drums; pick HAT; page TRACKS; play
2. **Strip:** Hold [GLO] and hold [11]. / [MASTER] stays plain.
   - **Do:** hold GLO, long box 11
   - **Until:** stopped
   - **Robot:** idle
3. **Setup:** page STEPS
4. **Strip top:** [KNOB2] to STEP 4. / Then [KNOB4] to HARD.
   - **Do:** turn KNOB 2 right 3, turn KNOB 4 right 2
   - **Until:** drums has 1 hard notes or more
   - **Page:** STEPS
   - **Robot:** idle
5. **Setup:** page TRACKS
6. **Strip:** [SELECT] left: slower, / below 110.
   - **Do:** turn SELECT left 3
   - **Until:** tempo below 110
   - **Robot:** idle
7. **Card:** Done.
   - **OCT+:** Done
