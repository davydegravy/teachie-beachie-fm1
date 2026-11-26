# Lesson 1: The key to play

A test of the lesson player, not a lesson of the course: `tutorial/upgrade/rollcheck.py` plays it, on the FM-1's own
course and with the computer leading. On SLOOP 3.4.2 the white keys are boxes on the drum grid only; on another track
the piano roll (NOTES) writes the key played at the step shown. So a step that waits for notes in boxes on such a
track blinks the key its Do names, not the white keys of the boxes still empty, and only while STEP is on a step that
still lacks its note (the key waits for the knob). SHOW ME on the FM-1 writes the track's last note into those steps.
In steps 2 and 4 the robot does nothing (`idle`): the test presses the keys.

- **Part:** a test
- **The idea:** on the piano roll the key to play blinks, not a box's key.
- **The learner does:** writes an A on steps 1, 4 and 7 of the bass.
- **Got it when:** all three steps end.
- **Song:** HOUSE ONE, section C.

## Steps

1. **Setup:** song HOUSE ONE section C, 1 bar; bass empty; track bass; page NOTES
2. **Strip top:** STEP says 1. Play [KEY 3].
   - **Do:** tap A3
   - **Until:** bass has notes on box 1
   - **Page:** NOTES
   - **Robot:** idle
3. **Strip top:** [KNOB1] to step 4: the A.
   - **Do:** turn KNOB 1 right 2, tap A3
   - **Until:** bass has notes on boxes 1 4
   - **Page:** NOTES
4. **Strip top:** Step 7: the A again.
   - **Do:** turn KNOB 1 right 2, tap A3
   - **Until:** bass has notes on boxes 1 4 7
   - **Page:** NOTES
   - **Robot:** idle
5. **Card:** Done.
   - **OCT+:** Done
