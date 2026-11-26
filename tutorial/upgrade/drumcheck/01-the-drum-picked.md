# Lesson 1: The drum picked

A test of the lesson player, not a lesson of the course: `tutorial/upgrade/drumcheck.py` plays it, on the FM-1's own
course and with the computer leading. It checks the `Until` word `drums picked SNARE`, and what a lesson does when the
drum picked is not the one a step's boxes ask for (SLOOP 3.4.2's drum grid: a black key picks, a white key is a box).
In step 3 the robot does nothing (`idle`): the test presses the keys itself, the wrong drum first.

- **Part:** a test
- **The idea:** the lesson knows which drum is picked.
- **The learner does:** picks the snare with its black key; then picks it by mistake where the kick is wanted.
- **Got it when:** both steps end.
- **Song:** HOUSE ONE, section C.

## Steps

1. **Setup:** song HOUSE ONE section C, 1 bar; drums empty; track drums; page STEPS
2. **Strip:** Pick the snare.
   - **Do:** tap G#3
   - **Until:** drums picked SNARE
   - **Hint:** Tap the blinking black key once.
   - **Robot:** tap A#3, wait 1, tap G#3
3. **Strip:** Kicks on [1] and [5].
   - **Do:** tap box 1 5
   - **Until:** drums has KICK on boxes 1 5
   - **Robot:** idle
4. **Card:** Done.
   - **OCT+:** Done
