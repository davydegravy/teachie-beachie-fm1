# Replays one lesson on a simulated FM-1 that leads itself (as tutorial/device.py does), up to a step, then makes that
# step's robot moves ONE AT A TIME, reading the drum pattern, the page and the screen after each.
#   cd <a copy of the project> && python3 replay.py LESSON STEP OUTDIR
import os, sys, time
sys.path.insert(0, "tutorial"); sys.path.insert(0, "tools")
import mkcourse, device
from tp.fm1 import FM1, Twin, BTN, KNOB
from tp import lesson as L
les_n, step_n, out = int(sys.argv[1]), int(sys.argv[2]), sys.argv[3]
lessons = L.lessons(); idx = next(i for i, l in enumerate(lessons) if l.number == les_n); les = lessons[idx]
pack = mkcourse.build()[0]; seed = device.seed_flash(pack)
os.makedirs(out, exist_ok=True); os.environ["TP_HOME"] = "1"
link = Twin(out=out, flash=seed); f = FM1(link); PH = device.PH
def state(tag):
    ph, num, step, _ = f.course(); n, notes = f.pattern(3)
    s = f.status()
    print("%-34s phase %-2d step %-2d page %-2d track %d | drums: %d notes %s | screen says: %r" % (tag, ph, step + 1, f.page, f.track, len(notes), sorted({(int(x[0]) + 1, x[2]) for x in notes})[:10], s.get("message", "")[:30]), flush=True)
try:
    time.sleep(3.2); device.tap(link, "button", BTN["OCT_UP"])
    for _ in range(60):
        if f.course()[0] == PH["TOC"]: break
        time.sleep(0.1)
    device.tap(link, "button", BTN["OCT_DOWN"])
    for _ in range(idx): link.knob(KNOB["SELECT"], 1); time.sleep(0.12)
    device.tap(link, "button", BTN["OCT_UP"]); device.tap(link, "button", BTN["OCT_UP"])
    for _ in range(400):                                    # through the steps before, as the robot would
        ph, num, step, _ = f.course()
        if step + 1 >= step_n and ph in (PH["DO"], PH["ANSWER"]): break
        if ph == PH["ANSWER"]: device.tap(link, "button", BTN["OCT_UP"])
        elif ph == PH["DO"]:
            s = les.steps[step]; device.hands(link, list(L.parse_do(s.sub("Robot") or s.sub("Do") or "", les.phrases)))
        time.sleep(0.4)
    time.sleep(0.5); state("at the step"); link.shot("a-at-step")
    s = les.steps[step_n - 1]
    ops = [o for o in L.parse_do(s.sub("Robot") or s.sub("Do") or "", les.phrases) if o[0] not in ("answer", "idle")]
    NAME = {v: k for k, v in BTN.items()}
    for k, m in enumerate(L.moves(ops)):                    # the robot's own presses and releases, in order, its timing kept
        if m[0] == "button": link.button(m[1], m[2])
        elif m[0] == "key": link.key(m[1], m[2])
        elif m[0] == "knob": link.knob(m[1], m[2])
        time.sleep(m[3])
        if m[0] in ("key", "button") and m[2] == 0 or m[0] == "knob":   # after each release: what the FM-1 holds
            state("%s %s up" % (m[0], NAME.get(m[1], m[1]) if m[0] == "button" else "key %d" % m[1])); link.shot("b-%02d" % k)
        elif m[0] == "button" and m[2] == 1: print("%-34s" % ("  (%s down)" % NAME.get(m[1], m[1])), flush=True)
finally:
    link.close()
