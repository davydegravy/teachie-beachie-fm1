# SPDX-License-Identifier: GPL-3.0-only
"""Lesson 1, HELLO: a blinking light means press it; PLAY starts and stops a song; and the melody game, which ends
with a run nobody can play by hand. (The lesson is written in Python for now; the plain text form comes later.)"""
import time
from tp.fm1 import BTN, light_of_key
from tp.song import load_spec, TRACK

SONG = load_spec("house-one", "C")                           # the whole band

A3, C4, E4, A4, B4, C5, D5, E5 = 4, 7, 11, 16, 18, 19, 21, 23   # keys, counted from the lowest (F3 = 0); the song is in A minor
UP = [A3, C4, E4, A4, C5, E5]
RUN = UP + UP[-2:0:-1] + UP + UP[-2:0:-1] + [A3, E4, A4, E5]    # up and down the chord, twice, and out

def lesson(t):
    """returns "left" when the learner asks to play the lesson again"""
    f = t.fm1; play = {("button", BTN["PLAY"])}
    t.ask("LESSON 1\nHELLO", "", "BEGIN", shot="0-begin", big=True)             # nothing on the FM-1 changes before this
    t.load_song(SONG); f.select(TRACK["lead"])
    t.ask("A BLINKING LIGHT MEANS: PRESS THIS.\nTRY IT NOW.", "", "GO", shot="1-hello")

    t.say("WE MAKE OUR SONGS IN SLOOP.\nSLOOP HAS 5 TRACKS WITH DIFFERENT INSTRUMENTS.\n"
          "PRESS [PLAY] TO OPEN A SONG IN SLOOP.", shot="2-sloop")
    t.point([BTN["PLAY"]]); t.wait_for(play)
    t.note("check", "playing: %s" % f.playing())
    f.lights(); t.strip("FIVE TRACKS.\nONE SONG. LISTEN.", where=2, shot="3-song")
    time.sleep(8.0)                                              # (the four bars, once round)
    t.strip("[PLAY] STOPS IT TOO.\nPRESS [PLAY].", where=2, shot="4-stop")
    t.point([BTN["PLAY"]], only=False); t.wait_for(play)
    t.note("check", "stopped: %s" % (not f.playing()))
    t.reply("GOOD.", shot="5-good")

    for phrase in ([A4, C5, E5], [A4, B4, C5, D5, E5]):
        t.say("THE MELODY GAME\nLISTEN.\nTHEN PLAY IT BACK.")
        t.play(phrase, 0.38); t.echo(phrase); t.reply("GOOD.")
    t.say("ONE MORE\nLISTEN.\nTHEN PLAY IT BACK.")
    t.play(RUN, 0.075)
    t.say("YOUR TURN\nPLAY IT BACK.", shot="6-your-turn")
    t.echo(RUN[:2])                                             # the first two notes, pointed at like the phrases before
    f.take(keys=range(27))                                      # from here every key is the guide's: none sounds by itself
    t.say("FASTER\nFASTER\nFASTER", big=True, shot="7-faster")
    t.point([light_of_key(RUN[2])]); t.wait_for({("key", k) for k in range(27)})
    t.play(RUN, 0.075); f.take()
    t.reply("NICE.", hold=1.4)
    t.ask("HA, THAT WAS TOO FAST. NOBODY CAN PLAY THAT BY HAND.\nA SEQUENCER CAN.", "", "NEXT", shot="8-ha")
    if t.ask("LESSON 1\nDONE", "REPLAY", "NEXT", shot="9-end", big=True) == "left": return "left"   # (a menu will come here)
    t.reply("LESSON 2\nSOON.", hold=1.8)                          # (there is no next lesson yet; the song stays on the FM-1)
