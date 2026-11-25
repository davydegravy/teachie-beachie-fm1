#!/usr/bin/env python3
"""The worksheet for moving the lessons to SLOOP 3.4.2: every step whose words or checks touch something 3.4.2 changed,
with today's words, so the rewrite can be done lesson by lesson without reading all thirty files first.
    python3 tutorial/upgrade/worksheet.py > tutorial/upgrade/worksheet.md
Found by words and by where the step is (the track and the page it is on), from the changes in UPGRADE-3.4.2.md. It
flags more than breaks: each flag says what to check, not that the step is wrong. Written for this project."""
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, ".."))
from tp import lesson as L

# what 3.4.2 changed, by a short name (UPGRADE-3.4.2.md has the whole of each)
WHY = {
    "SAVE":   "SAVE changed: SAVE on the tracks page opens SONG 2/3; the project is saved from PROJECT (SAVE twice no longer saves); SAVE + a key held 1.5 s saves into that pattern and does not switch; SAVE + a tap on the playing pattern puts back its saved copy",
    "SEQ":    "SEQ on a synth track opens NOTES (the piano roll), then PATTERN, then SONG: the page of boxes is the drums' only",
    "GRID":   "the drum grid: a white key sets or clears a box (with or without SEQ) and makes no sound; a black key or KNOB1 picks the drum. Playing a drum's key there no longer picks it",
    "GRIDSEQ": "works on 3.4.2, but [SEQ] is no longer needed on the drum grid: a white key sets or clears a box by itself. Keep teaching [SEQ] + a box, or not: a decision",
    "GRIDKN": "the drum grid's knobs are SOUND, STEP, HIT, LEVEL (were SOUND, QUANT, GATE, STEPS)",
    "HITS":   "on the drum grid OCT+ held opens the hit SHEET and OCT- turns the page: OCT held + a drum no longer gives soft or hard hits",
    "SEL":    "SEL opens SEL 1/2, a knob page with no tiles: ROOT, SCL, QNT, CHORD. KNOB4 turns chords on (TRIAD); KNOB3 is SNAP",
    "HOME":   "HOME held 0.7 s opens a MENU (SCREEN, LIGHTS ...); there is no PROJECT page (keys 1, 3, 5 still work, unlabelled)",
    "NAMES":  "names on screen: SENDS is FX 1/7 (ECHO is DLY), DELAY is DLY 6/7, FILT is FILTER, SCALE is SCL; EDIT has FLT 2/6 and FILTER 3/6; PLUS is gone; YOURS is under SAVE; MASTER is under GLO",
    "LAYOUT": "sound pages are drawn the other way up: knob labels at the top, the page's title small at the bottom right. A bottom strip hides the title; a one-line top strip covers the knob labels",
    "ORDER":  "the order of a button's pages changed (EDIT: OSC, FILTER, the analog filter, VOICE, VOICE 2, SOUNDS; FX: SENDS, a track filter, SLICER, INSERT, INSERT 2, DLY, REV/CHO; LFO: LFO, LFO 2, LFO DEST)",
    "KEEP":   "each pattern keeps a saved copy and leaving a pattern puts it back: what is not saved into a pattern is lost on a pattern change (and the setups' songs need keep_all() after c_setup)",
    "CHAIN":  "[15] is no longer CHAIN: it switches THIS TRACK ONLY / every track and stays; [16] opens SONG; a chain is pattern keys tapped in one SAVE hold; CHAIN DONE is never said",
    "LINE":   "SAVE + [13] plays the song from the LINE shown, not from the top",
    "UNDO":   "undo keeps 3 moves (was 8)",
    "LOCK":   "the FX + HOME lock happens on HOME's release, and only if FX is still held",
    "P1":     "the tracks page shows the pattern as a number in a square at the top right, not P1",
}
SOUNDPAGES = {"ENV", "FILTER", "SENDS", "DELAY", "INSERT", "ARP", "LFO", "KEY", "FILT"}

# where each lesson stopped in the runs on 3.4.2 (UPGRADE-3.4.2.md): (the FM-1-led run, the computer-led run)
STOPPED = {
    3: "the computer-led run, step 6 (drum grid)", 6: "both runs, step 18 (SAVE twice)", 7: "both runs, step 4 (drum grid)",
    8: "the FM-1-led run, step 4 (SEQ on the bass)", 12: "the computer-led run, step 10 (SEL's knobs)",
    13: "the FM-1-led run, step 8 ([SEQ] twice lands on PATTERN)", 15: "both runs, step 4 (drum grid)",
    16: "both runs, step 6 (the copy of a page: SAVE)", 17: "the computer-led run, step 12 (drum grid: [11] wrote a box)",
    23: "both runs, step 4 (the copy of a page: SAVE)", 25: "both runs, step 3 (SAVE opens SONG 2/3)",
    26: "both runs, step 3 (SAVE opens SONG 2/3)",
}

def flags(step, track, page):
    """the changes a step touches"""
    txt = step.text
    sub = " | ".join("%s: %s" % (k, v) for k, v in step.subs)
    allw = txt + " | " + sub
    do = " ".join(step.all("Do") + step.all("Robot"))
    f = []
    if re.search(r"\bSAVE\b", allw): f.append("SAVE")
    if re.search(r"\bCHAIN\b", allw) or "SAVE" in f and re.search(r"\[1[56]\]", allw): f.append("CHAIN")
    if "SAVE" in f and re.search(r"\[13\]|\bbox 13\b", allw): f.append("LINE")
    synth = track not in (None, "drums")
    if synth and (re.search(r"\bpress SEQ\b", do) or re.search(r"\[SEQ\]", txt) and re.search(r"twice|same 16|boxes", txt, re.I)
                  or re.search(r"page is STEPS|^Page: STEPS", sub) or page == "STEPS"):
        f.append("SEQ")
    ongrid = track == "drums" and page == "STEPS"
    if ongrid:
        if re.search(r"\btap (KEY|box|\[|\d|[A-G]#?\d)", do) and not re.search(r"hold SEQ", do): f.append("GRID")
        elif re.search(r"\bplay \[(KEY )?\d+\]|\bpick\b|\bplay (one|its|every|a key|each)\b", txt, re.I): f.append("GRID")
        elif re.search(r"hold SEQ", do) or re.search(r"\[SEQ\]", txt): f.append("GRIDSEQ")
    if re.search(r"pick (a|the) (drum|kick|snare|hat|tom)|play its key|play \[KEY \d+\]|Each key is a different drum", txt, re.I) and "GRID" not in f:
        f.append("GRID")
    if ongrid and re.search(r"\b(QUANT|GATE|STEPS)\b|\[KNOB\d\]", txt): f.append("GRIDKN")
    if re.search(r"hard notes|soft|hard hit|\bhard\b", allw, re.I) and track == "drums": f.append("HITS")
    if re.search(r"\[SEL\]|\bSCALE\b|\bSNAP\b|\bROOT\b|\bKEYS\b|page is KEY", allw) or page == "KEY": f.append("SEL")
    if re.search(r"(hold|held)\s+\[HOME\]|\[HOME\]\s*\+|\bPROJECT\b|\bOPEN\b|\bNEW\b", allw): f.append("HOME")
    if re.search(r"\bSENDS\b|\bDELAY\b|\bECHO\b|\bFILT\b|\bPLUS\b|\bYOURS\b|(?<!\[)\bMASTER\b(?!\])", allw): f.append("NAMES")
    if step.kind.lower() == "strip" and page in SOUNDPAGES: f.append("LAYOUT")
    if re.search(r"page is (ENV|FILTER|SENDS|DELAY|INSERT|LFO|SOUNDS|ARP)|press (EDIT|FX|LFO)\b", allw): f.append("ORDER")
    if re.search(r"\bpattern\b|\bpage [2-8]\b|\bP[1-8]\b|\bpatterns\b", allw, re.I) and "SAVE" not in f: f.append("KEEP")
    if re.search(r"\bundo\b", allw, re.I) and re.search(r"\b8\b|eight", allw, re.I): f.append("UNDO")
    if re.search(r"\[FX\]", allw) and re.search(r"\[HOME\]", allw): f.append("LOCK")
    if re.search(r"\bP1\b|top right", allw): f.append("P1")
    return list(dict.fromkeys(f))

def main():
    out = ["# The steps to look at for SLOOP 3.4.2, lesson by lesson",
           "",
           "Made by `tutorial/upgrade/worksheet.py` from `tutorial/lessons/`. Each step below touches something SLOOP 3.4.2",
           "changed; its flags say what. A flag says what to check, not that the step is wrong: the runs and the review",
           "in `tutorial/UPGRADE-3.4.2.md` say which ones break. Today's words are as the lesson file has them (` / ` a",
           "new line, ` // ` a gap).",
           "",
           "## The flags",
           ""]
    for k, v in WHY.items(): out.append("- **%s**: %s." % (k, v))
    counts, total = {}, 0
    body = []
    for les in L.lessons():
        track, page, rows = None, None, []
        for s in les.steps:
            if s.kind.lower() == "setup":
                m = re.search(r"\btrack (\w+)", s.text); track = m.group(1) if m else track
                m = re.search(r"\bpage (\w+)", s.text); page = m.group(1) if m else page
                continue
            pg = s.sub("Page")
            if pg: page = pg.split()[0]
            fl = flags(s, track, page)
            if fl:
                rows.append((s, fl))
                for x in fl: counts[x] = counts.get(x, 0) + 1
            u = s.sub("Until") or ""
            m = re.search(r"page is (\w+)", u); page = m.group(1) if m else page
            m = re.search(r"track is (\w+)", u); track = m.group(1) if m else track
        if not rows and les.number not in STOPPED: continue
        total += 1
        body += ["", "## Lesson %d: %s" % (les.number, les.title), ""]
        if les.number in STOPPED: body += ["**Stopped on 3.4.2:** %s." % STOPPED[les.number], ""]
        for s, fl in rows:
            body.append("- **Step %d, %s** · %s" % (s.n, s.kind.lower(), " ".join("`%s`" % x for x in fl)))
            if s.text: body.append("  - Today: %s" % s.text)
            for k, v in s.subs:
                if k in ("Do", "Until", "Page", "Hint", "Robot", "Wait"): body.append("  - %s: %s" % (k, v))
    out += ["", "%d lessons have steps to look at. Steps flagged, by flag: %s." %
            (total, ", ".join("%s %d" % (k, counts[k]) for k in WHY if k in counts))]
    print("\n".join(out + body))

if __name__ == "__main__":
    main()
