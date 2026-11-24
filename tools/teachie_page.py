#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: SLOOP's installer page (tools/mkpage.py's output) under TEACHIE BEACHIE's name, in place. Run by fw/build.sh
# after mkpage.py, so web/installer.tmpl.html stays exactly SLOOP's and a merge of a new SLOOP never clashes there.
#   teachie_page.py PAGE.html
# What it does: the page in English only (SLOOP's French left out); Teachie Beachie where the page names the firmware it
# installs; SLOOP 3 credited in the title, the beta note and the footer; bug reports to Teachie Beachie's; the version shown as Teachie Beachie's, then
# SLOOP's; Teachie Beachie's logo above the title and as the tab's icon (the two pictures are made by
# tools/mkpagelogo.py); and its own look: the FM-1 cards' two colors (peach paper, dark red ink, app/vis.h) and the
# lessons' font (assets/fonts/page-font.woff, tools/mkpagefont.py), with no bar or menu at the top and a short
# introduction. The page's markup and script stay SLOOP's: only styles are added, so every step works as SLOOP wrote it.
# One thing is added, after SLOOP's own: the lessons (8, below), sent to the FM-1 once Teachie Beachie runs on it.
# A rule that no longer finds its text is reported (SLOOP reworded the page: look again); the build stops only if the
# credit to SLOOP or the name would be missing.
import base64, os, re, sys
import mkcourse                                          # (tools/mkcourse.py: the lessons' pack)

TP_VERSION = "0.1"
HOME = "https://github.com/davydegravy/teachie-beachie-fm1"

path = sys.argv[1]
t = open(path, encoding="utf-8").read()
m = re.search(r'const VERSION = "([^"]+)"', t)
sloop_ver = m.group(1) if m else "?"

# 1. the firmware's name: "SLOOP 3" alone (not "SLOOP 3.0", "SLOOP 3.3.4": those are SLOOP's own versions); the
#    installing code inlined from web/fwpkg.js is left as SLOOP wrote it (its comments describe SLOOP's loader)
pkg = open("web/fwpkg.js", encoding="utf-8").read()
held = pkg in t
if held: t = t.replace(pkg, "\0FWPKG\0")
t, renamed = re.subn(r"SLOOP 3(?!\.\d|\d)", "TEACHIE BEACHIE", t)
if held: t = t.replace("\0FWPKG\0", pkg)

# 1b. English only: SLOOP's page is in English and French, chosen by the browser's language. Teachie Beachie's is in
#     English: the French words are taken out (the script falls back to the English for every one) and the page no
#     longer looks at the browser's language
t, nfr = re.subn(r"\nfr: \{\n.*?\n\}\};", "\nfr: {}};", t, count=1, flags=re.S)
t, nlang = re.subn(r"setLang\(saved \|\| \(/\^fr/i\.test\(navigator\.language \|\| ''\) \? 'fr' : 'en'\)\);", "setLang('en');", t)

# 2. the sentences that need SLOOP 3 named, or TEACHIE BEACHIE's own words
RULES = [
    ('content="Install TEACHIE BEACHIE, the free groovebox firmware for the M-VAVE FM-1,',
     'content="Install TEACHIE BEACHIE, free firmware that teaches the M-VAVE FM-1 in thirty lessons, made for kids and beginners, built on SLOOP 3,'),
    # what Teachie Beachie is, in a few words: made for kids, and why, in the author's own voice (no names, no ages;
    # the page's words for SLOOP described a groovebox only; &#8209;, a hyphen
    # that does not break the line, keeps M-VAVE and FM-1 whole on a phone; five short paragraphs, the later ones in spans that
    # the look, below, starts on lines of their own)
    ('TEACHIE BEACHIE turns the M-VAVE FM-1 into a complete groovebox: drums, bass, chords, melodies and effects, with a '
     'sequencer that loops it all. This page installs it for you, step by step, right from your browser.',
     'Teachie Beachie teaches kids, and any beginner, to make music on the M&#8209;VAVE FM&#8209;1. You don\'t need to read '
     'music or open a manual: every lesson shows you what to press, right on the FM&#8209;1\'s screen.'
     '<span class="tp-p">A lesson puts a short card on the screen, lights up the button or key to press, and waits while you try '
     'it. If you get stuck it offers a hint, and it can even do the step for you. Each lesson takes a few minutes and builds '
     'on the one before.</span>'
     '<span class="tp-p">There are thirty lessons in four parts: Your first beat, Notes and chords, Effects and sound, Make a '
     'song. By lesson 7 you have made a beat of your own. By lesson 30, a whole song.</span>'
     '<span class="tp-p">Teachie Beachie is free, and once it is installed no computer is needed. It is built on SLOOP 3, a '
     'complete groovebox, which stays when the lessons are done.</span>'
     '<span class="tp-p">I made Teachie Beachie for my kids, nieces and nephews, as part of a Christmas present. If you have kids '
     'who would like to learn how to use a groovebox and the FM&#8209;1, I\'d encourage you to give Teachie Beachie a try.</span>'),
    # the title: FM-1 kept whole on a phone (it broke as "FM-" and "1"), and "on your FM-1" kept together, as the
    # name is, so the title breaks only between its three parts; under it, a line saying what it is
    ('on your FM-1</h1>', '<span>on your FM&#8209;1</span></h1><p class="tp-tag">Thirty lessons that teach the FM&#8209;1, on the FM&#8209;1 itself.</p>'),
    # the beta, in a newcomer's words (SLOOP's compare SLOOP 3 with SLOOP 2.8, which a newcomer has never seen)
    ("It was rewritten from scratch: a new sound engine, a new sequencer, a new interface. It is being tested on real FM-1s "
     "right now, so you may meet bugs. Keep backups, and don't rely on it on stage yet. ",
     "SLOOP 3 is new and still being tested on real FM-1s, and so is Teachie Beachie, so you may meet bugs. This page backs "
     "up your FM-1 before it changes anything, so you can always go back. "),
    ('The FM-1 running TEACHIE BEACHIE (3D render)', 'The M-VAVE FM-1, with SLOOP 3 on its screen (3D render)'),
    ('The M-VAVE FM-1 running TEACHIE BEACHIE', 'The M-VAVE FM-1'),
    # after the install: the peach and the start card, not SLOOP's sailboat; and where the lessons come from
    ('The FM-1 restarts on TEACHIE BEACHIE and the sailboat logo draws itself in. Turn <b>MASTER</b> up gently, and have fun.',
     '<span class="tp-pend">The firmware is installed. Now send the thirty lessons: it takes a few seconds.</span>'
     '<span class="tp-ok">The thirty lessons are on your FM&#8209;1. Turn <b>MASTER</b> up gently, and have fun.</span>'),
    # its title: no "installed" until the lessons are on the FM-1 too
    ('data-t="h6">TEACHIE BEACHIE is installed</h2>', 'data-t="h6"><span class="tp-pend">One step left: the lessons</span>'
     '<span class="tp-ok">Teachie Beachie is ready</span></h2>'),
    # the step, before the lessons: no check mark, no tips (the look, below, and web/teachie_lessons.js)
    ('<section class="pane hide" id="p6"', '<section class="pane hide tp-pending" id="p6"'),
    # the sixth step in the list at the left: the lessons (SLOOP's: a first minute of play)
    ('data-t="r6">Play<small>Your first minute</small>', 'data-t="r6">Lessons<small>A few seconds</small>'),
    # the first start: TEACHIE BEACHIE leaves out SLOOP 3's opening of a SLOOP 2.8 project (TP_SLOOP28_PROJECT in fw/main.c)
    ("At its first start, TEACHIE BEACHIE opens what SLOOP 2.8 left: your sections become patterns, and your presets, "
     "favourites, samples, SYN kits, FM6 bank and settings come along. SLOOP 2.8's data is only read, never written.",
     "TEACHIE BEACHIE does not open the project SLOOP 2.8 left (SLOOP 3 does; TEACHIE BEACHIE leaves that out to make room "
     "for its lessons). Your presets, favorites, samples, SYN kits, FM6 bank and settings come along. SLOOP 2.8's data is "
     "only read, never written."),
    ('data-t="betaT">TEACHIE BEACHIE is a public beta', 'data-t="betaT">TEACHIE BEACHIE runs SLOOP 3, a public beta'),
    ('<p data-t="foot1"><b>TEACHIE BEACHIE</b> is free software (GPL-3.0) by the SLOOP project: '
     '<a href="https://github.com/isod89/sloop3-fm1">github.com/isod89/sloop3-fm1</a>.',
     '<p data-t="foot1"><b>Teachie Beachie</b> is free software (GPL-3.0), built on <b>SLOOP 3</b> by the '
     'SLOOP project (<a href="https://github.com/isod89/sloop3-fm1">github.com/isod89/sloop3-fm1</a>). '
     'The SLOOP project does not make or support TEACHIE BEACHIE.'),
]
missed = []
if not nfr: missed.append("SLOOP's French words")
if not nlang: missed.append("the language chosen from the browser's")
for old, new in RULES:
    if old in t: t = t.replace(old, new)
    else: missed.append(old[:70])

# 3. where to report a bug (Teachie Beachie's, not SLOOP's); the page's own address, which SLOOP's names when a browser
#    cannot install, left out (it would show the author's account name: "open this page in Chrome or Edge" says enough)
t = t.replace("https://github.com/isod89/sloop3-fm1/issues/new/choose", HOME + "/issues")
t, naddr = re.subn(r" ?: <code>https://isod89\.github\.io/sloop3-fm1/</code>", ".", t)
if naddr != 1: missed.append("the page's own address, when a browser cannot install (%d of 1)" % naddr)

# 4. the version shown: TEACHIE BEACHIE's, then SLOOP's (the badge at the top and the summary before installing)
shown = "v%s · SLOOP %s" % (TP_VERSION, sloop_ver)
t, nver = re.subn(r'(<span class="badge ver">|<dd>TEACHIE BEACHIE )v' + re.escape(sloop_ver) + r'(?=[ <])', lambda g: g.group(1) + shown, t)

# 5. the logo: the small mark by the name and in the browser's tab (SLOOP's sailboat there), and the logo above the title
logos = 0
def b64(name): return base64.b64encode(open(os.path.join("assets", "logo", name), "rb").read()).decode()
m = re.search(r'<a class="brand" href="#top"><img src="data:image/png;base64,([A-Za-z0-9+/=]+)"', t)
if m and os.path.exists("assets/logo/teachie-beachie-mark.png"): logos += t.count(m.group(1)); t = t.replace(m.group(1), b64("teachie-beachie-mark.png"))
else: missed.append("the mark by the page's name")
hero = '<span class="badge beta" data-t="heroBadge">'
if hero in t and os.path.exists("assets/logo/teachie-beachie-page.png"):
    t = t.replace(hero, '<img class="tp-logo" src="data:image/png;base64,%s" alt="Teachie Beachie" width="220" height="220">' % b64("teachie-beachie-page.png") + hero, 1); logos += 1
else: missed.append("the title's badge, where the logo goes")

# 6. the look: the FM-1 cards' paper and ink (app/vis.h: TP_PAPER #FFCBA4, TP_INK #960000), lighter and darker papers
#    made of those two, and the lessons' font, inside the page. Added after SLOOP's styles, so it wins; SLOOP's markup
#    and script are untouched (the bar at the top, with its menu and language buttons, is hidden, not removed: the
#    script still finds them; the language follows the browser). At the top: the logo and the title in the middle,
#    a line saying what it is, then a few words from the left, as wide as the title; SLOOP's beta badge, its three facts and the 3D picture hidden.
font = base64.b64encode(open("assets/fonts/page-font.woff", "rb").read()).decode()
LOOK = """
/* Teachie Beachie's look (tools/teachie_page.py): the FM-1 cards' peach paper and dark red ink. Its font is the
   lessons' own, Google Sans (c) 2025 The Google Sans Project Authors, SIL Open Font License 1.1 (openfontlicense.org),
   letters of English and French only (tools/mkpagefont.py) */
@font-face{font-family:"TB Sans";src:url(data:font/woff;base64,__FONT__) format("woff");font-weight:400 700;font-display:swap}
:root{--paper:#FFCBA4;--bg:#FFCBA4;--bg2:#FFD4B3;--panel:#FFE0C8;--panel2:#F8BD99;--line:#E89E80;--line2:#D57A62;
--ink:#960000;--ink2:#960000;--dim:#A61E19;--acc:#960000;--acc2:#960000;--ok:#960000;--warn:#960000;--bad:#960000;--blue:#960000;
--sh:0 10px 30px rgba(150,0,0,.08)}
html{scroll-padding-top:20px}
body{background:var(--bg);color:var(--ink);font-family:"TB Sans",system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
::selection{background:var(--ink);color:var(--paper)}
a{color:var(--ink);text-decoration-thickness:1.5px}a:hover{color:#780000}
:focus-visible{outline-color:var(--ink)}
.top{display:none}
code{background:var(--panel2);color:var(--ink)}
kbd{background:transparent;color:var(--ink);border:1.5px solid var(--ink);border-radius:4px;font-family:"TB Sans",system-ui,sans-serif;font-weight:600;padding:0 5px}
.dim{color:var(--dim)}
.badge.beta{background:var(--ink);color:var(--paper)}.badge.ver{background:var(--panel);color:var(--ink);border-color:var(--line2)}
/* the top: the logo, the title, a few words, the checks */
.hero{display:block;width:auto;max-width:none;margin:0;padding:44px 0 10px;text-align:left}
.hero .badge,.hero .facts,.hero .shot{display:none}
/* (as wide as the sections under it, on the six columns of the screens below. The logo, small, on a row of its own
   above everything. Then two columns: the title, a line under it and the words over the first four; six screens of the
   lessons over the last two, two across, so each is exactly as large as the six under them and stands over them.
   On a small screen: the logo and the title in the middle, the words under them, and the six screens beside them left
   out: six more follow just below) */
.hero>div{display:grid;grid-template-columns:repeat(6,minmax(0,1fr));column-gap:12px;align-items:start}
.hero .tp-logo{grid-column:1 / -1;grid-row:1;display:block;width:124px;height:124px;margin:0 0 14px -6px}
.hero h1{grid-column:1 / span 4;grid-row:2;padding-right:44px;font-size:38px;font-weight:700;margin:0;letter-spacing:-.02em;text-wrap:balance}
.hero h1 span{background:none;-webkit-background-clip:border-box;background-clip:border-box;color:var(--ink);white-space:nowrap}
.hero .tp-tag{grid-column:1 / span 4;grid-row:3;padding-right:44px;font-size:22px;font-weight:600;line-height:1.3;margin:8px 0 0}
.hero .lead{grid-column:1 / span 4;grid-row:4;padding-right:44px;font-size:19px;color:var(--ink);max-width:none;margin:16px 0 0}
.hero .tp-side{grid-column:5 / span 2;grid-row:2 / span 3;margin-top:6px}
.hero .lead .tp-p{display:block;margin-top:.7em}
/* (the browser's checks, in step 1 now) */
.pane .checks{justify-content:flex-start;margin:14px 0 4px}
/* what a lesson looks like: six screens in a row (three, then two, on smaller screens) */
.tp-show{margin:26px 0 0}
.tp-shots{display:grid;grid-template-columns:repeat(6,1fr);gap:12px}
.tp-shots.tp-side{grid-template-columns:1fr 1fr;row-gap:14px}
.tp-shots figure{margin:0}
/* (each in a fat dark gray frame, the FM-1's body, with the screen's black edge inside it: a peach card on this peach
   page was lost) */
.tp-shots img{display:block;width:100%;height:auto;border:13px solid #47474c;background:#0b0b0b;padding:3px;border-radius:18px;box-shadow:0 6px 16px rgba(90,0,0,.2)}
.tp-shots figcaption{font-size:13px;color:var(--dim);margin-top:6px;text-align:center}
/* the lessons with an AI assistant: a few words, then four boxes */
.tp-ai{margin:34px 0 0}
.tp-ai h2{font-size:26px;font-weight:700;margin:0 0 4px}
.tp-ai-sub{font-size:17px;margin:0 0 14px;max-width:46em}
.tp-three{display:grid;grid-template-columns:repeat(4,1fr);gap:14px}
.tp-box{background:var(--panel);border:1px solid var(--line);border-radius:16px;padding:18px 20px}
.tp-box h3{font-size:19px;margin:0 0 6px}
.tp-box p{margin:0;font-size:15.5px}
.tp-ai-more{font-size:15.5px;margin:12px 0 0}
@media(max-width:1000px){.tp-three{grid-template-columns:1fr 1fr}}
@media(max-width:560px){.tp-three{grid-template-columns:1fr}}
@media(max-width:900px){.tp-shots{grid-template-columns:repeat(3,1fr)}}
@media(max-width:480px){.tp-shots{grid-template-columns:repeat(2,1fr)}.hero .tp-tag{font-size:19px}}
details.tp-sloop{margin:16px 0}details.tp-sloop .note{margin:10px 0}
.chk{background:var(--panel);border-color:var(--line2);color:var(--ink)}
.chk i{background:var(--ink);color:var(--paper)}
.chk.no{border-color:var(--ink);border-style:dashed;color:var(--ink)}.chk.no i{background:transparent;color:var(--ink);box-shadow:inset 0 0 0 1.5px var(--ink)}
.blocker{text-align:left;border:2px solid var(--ink);background:var(--panel)}.blocker b{color:var(--ink)}
@media(max-width:900px){.hero{padding-top:26px;text-align:center}.hero>div{display:block}.hero h1{font-size:34px;margin-top:6px}.hero .tp-logo{width:160px;height:160px;margin:0 auto 4px}.hero h1,.hero .tp-tag,.hero .lead{padding-right:0}.hero .lead{text-align:left}.hero .tp-side{display:none}}
/* the steps */
.wizard{margin-top:30px}
.rail{background:var(--panel);border-color:var(--line);top:20px}
.rail button{color:var(--dim)}.rail button:not(:disabled):hover{background:var(--bg2)}
.rail .n{border-color:var(--line2);color:var(--dim)}.rail .t small{color:var(--dim)}
.rail li.now button{background:var(--bg);color:var(--ink)}.rail li.now .n{border-color:var(--ink);background:var(--ink);color:var(--paper)}
.rail li.done button{color:var(--ink)}.rail li.done .n{border-color:var(--ink);background:transparent;color:var(--ink)}
.rail .line{background:var(--line2)}
.pane{background:var(--panel);border-color:var(--line)}
.pane .eyebrow{color:var(--dim)}.pane .sub{color:var(--ink)}
.foot{border-top-color:var(--line)}
.btn{background:transparent;border:1.5px solid var(--ink);color:var(--ink)}
.btn:hover:not(:disabled){filter:none;background:var(--bg2);color:var(--ink)}
.btn.primary{background:var(--ink);border-color:var(--ink);color:var(--paper);box-shadow:none}
.btn.primary:hover:not(:disabled){background:#780000;color:var(--paper)}
.btn.ghost{border-color:transparent;color:var(--ink)}.btn.ghost:hover:not(:disabled){background:var(--bg2)}
.note,.note.bad{background:var(--bg2);border-color:var(--line);border-left-color:var(--line2)}
.note.warn,.note.safe,.note.info,.note.bad{border-left-color:var(--ink)}
.note>b:first-child{color:var(--ink)}
.steps>li::before{background:var(--bg2);border-color:var(--line2);color:var(--ink)}
.check{background:var(--bg2);border-color:var(--line)}.check:hover{border-color:var(--line2)}.check input{accent-color:var(--ink)}
.choice{background:var(--bg2);border-color:var(--line2)}.choice:hover{border-color:var(--ink)}
.choice[aria-pressed=true]{border-color:var(--ink);background:var(--panel2);box-shadow:inset 0 0 0 1px var(--ink)}
.choice b,.choice span{color:var(--ink)}.choice .ico{background:var(--panel)}
.media img{border-color:var(--line)}
.way{background:var(--bg2);border-color:var(--line)}.way.rec{border-color:var(--ink)}.way .tag,.way.rec .tag{color:var(--ink)}
.links a{border-color:var(--line2);color:var(--ink)}.links a:hover{border-color:var(--ink)}
details.more{background:var(--bg2);border-color:var(--line)}details.more>summary::before{color:var(--ink)}
.device{border-color:var(--ink);background:var(--bg2)}.device .dot{background:var(--ink);box-shadow:0 0 0 5px rgba(150,0,0,.15)}
.device b,.device .meta{color:var(--ink)}
.wait{border-color:var(--line2);color:var(--ink)}.spin{border-color:var(--line2);border-top-color:var(--ink)}
.status,.status.ok,.status.bad{color:var(--ink)}
.prog .row,.prog .row b{color:var(--ink)}
.bar{background:var(--bg2);border-color:var(--line)}.bar>div{background:var(--ink)}
.phase{border-color:var(--line2);color:var(--dim)}.phase.now{border-color:var(--ink);background:var(--ink);color:var(--paper)}
.phase.done{border-color:var(--ink);color:var(--ink)}
.summary{background:var(--bg2);border-color:var(--line)}.summary dt{color:var(--dim)}.summary dd{color:var(--ink)}
.big-ok{background:var(--ink);box-shadow:0 0 0 10px rgba(150,0,0,.12)}.big-ok svg{color:var(--paper)}
.card{background:var(--bg2);border-color:var(--line)}.card b,.card p{color:var(--ink)}
#log{background:#FFF1E6;border-color:var(--line);color:var(--ink)}#log .ok,#log .bad{color:var(--ink);font-weight:700}
/* the lessons, in the last step; alone (a FM-1 that runs Teachie Beachie already), without the words about the install */
.tp-les{border-top:1px solid var(--line);margin-top:22px;padding-top:4px}
.tp-les .btn.big{margin:4px 0}
#p6.tp-only .big-ok,#p6.tp-only #h6,#p6.tp-only>.sub{display:none}
#p6.tp-only .tp-les{border-top:0;margin-top:0;padding-top:0}
#p6.tp-only #tpLesT{font-size:28px;font-weight:800;margin:6px 0 4px}
/* step 6: until the lessons are on the FM-1 (tp-pending), no check mark, its title and words say what is left, and
   SLOOP's tips and links wait; after, the check mark and "ready". After an install the step's title says it all, so
   the lessons' own title and first words show only when the step is opened by itself (tp-only); once they are sent,
   what to do to send them goes, and what was done stays */
#p6 .tp-ok,#p6.tp-pending .tp-pend{display:inline}#p6.tp-pending .tp-ok,#p6:not(.tp-pending) .tp-pend{display:none}
#p6.tp-pending .big-ok,#p6.tp-pending .cards,#p6.tp-pending>.buttons,#p6.tp-pending>.note.info{display:none!important}
#p6:not(.tp-only) #tpLesT,#p6:not(.tp-only) .tp-les-intro,#p6:not(.tp-only) .tp-les .eyebrow{display:none}
#p6:not(.tp-pending) .tp-les .steps,#p6:not(.tp-pending) .tp-les .buttons{display:none}
#p6:not(.tp-only) .tp-les{border-top:0;margin-top:0;padding-top:0}
/* help, the footer, the message that pops up */
.faq details{background:var(--panel);border-color:var(--line)}.faq details>div{color:var(--ink)}
footer{border-top-color:var(--line2);color:var(--dim)}footer a{color:var(--ink)}
.toast{background:var(--ink);border-color:var(--ink);color:var(--paper)}
""".replace("__FONT__", font)
looks = 0
if "</head>" in t: t = t.replace("</head>", '<style id="tp-look">' + LOOK + "</style></head>", 1); looks += 1
else: missed.append("the end of the page's head, where its look goes")
t, n = re.subn(r'<meta name="theme-color" content="#[0-9a-fA-F]+">', '<meta name="theme-color" content="#FFCBA4">', t)
if not n: missed.append("the browser's color for the page")

# 7. the name as the lessons and the FM-1's start card write it now (sentence case): Teachie Beachie
cased = t.count("TEACHIE BEACHIE"); t = t.replace("TEACHIE BEACHIE", "Teachie Beachie")

# 8. the lessons: the pack (tools/mkcourse.py, in a sample slot's shape) and what sends it (web/teachie_lessons.js) at
#    the end of the page's script; its buttons in the last step, after the install; and in the first step a way straight
#    there, for a FM-1 that runs Teachie Beachie already (newer lessons, without installing again). The pack and the
#    firmware in one page are of one build.
try: pack, sizes, clips = mkcourse.build()
except mkcourse.Cant as e: sys.exit("teachie_page: the lessons are not packed: %s" % e)
head, data = mkcourse.slot_image(pack)
js = open("web/teachie_lessons.js", encoding="utf-8").read()
js = js.replace("__TP_PACK__", base64.b64encode(data).decode()).replace("__TP_HEAD__", base64.b64encode(head).decode()).replace("__TP_N__", str(len(sizes)))
LESSONS = """<div class="tp-les" id="tpLes">
<div class="eyebrow">Step 6 of 6</div>
<h3 id="tpLesT">The lessons</h3>
<p class="tp-les-intro">The thirty lessons are a small pack of their own. Send them to the FM-1 now: it takes a few seconds.</p>
<ol class="steps">
<li>The FM-1 is on and plugged in. Its screen shows Teachie Beachie's card or SLOOP 3, not the menu with <b>MIDI MODE</b>.</li>
<li>Click <b>Send the lessons</b>. If the browser asks to control your MIDI devices, click <b>Allow</b>.</li>
</ol>
<div class="buttons"><button class="btn primary big" id="tpSend">Send the lessons</button></div>
<div class="wait hide" id="tpWait"><div class="spin"></div><div><b>Looking for your FM-1&hellip;</b><div class="small dim">It is found as soon as it has restarted. Leave it plugged in.</div></div></div>
<div class="prog hide" id="tpProg"><div class="row"><b id="tpPct">0 %</b><span>Sending the lessons</span></div><div class="bar"><div id="tpFill"></div></div></div>
<div class="hide" id="tpPick"><p><b>All four sample slots of this FM-1 hold samples.</b> The lessons need one of them. Choose the slot to replace:</p><div class="choices" id="tpSlots" role="group" aria-label="Sample slots"></div></div>
<div class="device hide" id="tpDone"><div class="dot"></div><div><b>The lessons are on your FM-1</b><div class="meta" id="tpMeta"></div></div></div>
<div class="note bad hide" id="tpErr"></div>
</div>
"""
ONLY = """<div class="note info"><b>Teachie Beachie already on your FM-1?</b><span>To get this page's lessons without installing again, <a href="#p6" id="tpOnly">send the lessons only</a>.</span></div>
"""
lessons = 0
t, n = re.subn(r'(<p class="sub" data-t="s6">.*?</p>\n)(?=<div class="cards">)', lambda g: g.group(1) + LESSONS, t, count=1, flags=re.S); lessons += n
if not n: missed.append("the last step's cards, where the lessons' buttons go")
a = '<label class="check"><input type="checkbox" id="ok1">'
if a in t: t = t.replace(a, ONLY + a, 1); lessons += 1
else: missed.append("the first step's agreement, where the way straight to the lessons goes")
a = "</script></body></html>"
if t.rstrip().endswith(a): t = t.rstrip()[:-len(a)] + js + a + "\n"; lessons += 1
else: missed.append("the end of the page's script, where the lessons go")
if "<script" in LESSONS + ONLY + js or "</script" in LESSONS + ONLY + js: sys.exit("teachie_page: the lessons' part would open or close a script (tests/install_sim.js reads the page's one script)")

# 9. the page for a newcomer, a parent perhaps, who has never seen SLOOP:
#    - what a lesson looks like: six screens beside the introduction (two across, three down, each as large as those
#      under them) and six more (assets/shots/, from tutorial/device.py's run on the simulated FM-1) in a
#      row between the introduction and the steps (the course's four parts are in the introduction); under them, the
#      lessons with an AI assistant, whichever it is: what it can do today with the project's files (tutorial/AI.md);
#    - the browser's three checks (and what it says when a browser cannot install) moved from the top into step 1,
#      so the top ends on the author's words;
#    - step 1's two notes for people who use SLOOP already (the tried and tested version, updating from 3.0 to 3.3.4)
#      folded into one closed section;
#    - the steps counted to 6, the lessons being the sixth (SLOOP's page counts 5 and a last screen).
# (twelve screens in the lessons' order: the card of the whole course first, then strips and tags of a lesson's words over
#  SLOOP's own page, lesson 16's cheat sheet (a lesson's last card), lesson 20's card about the filter, and the last card,
#  Lesson 30 Done. The first
#  six beside the introduction, the next six under it)
SIDE = [("course.png", "The course", "The card of the whole course: 30 lessons in 4 parts"),
        ("lesson-01.png", "Lesson 1: play", "Lesson 1: SLOOP's five tracks, with a strip of words across the bottom: SLOOP! Press PLAY."),
        ("lesson-03.png", "Lesson 3: drums", "Lesson 3: the drum grid, with the words: Hold SEQ. Hats on 3, 7, 11, 15: between the kicks."),
        ("lesson-08.png", "Lesson 8: a bass line", "Lesson 8: the bass line's sixteen boxes, some holding the note A1, with the words: Hold SEQ. Put it on 16."),
        ("lesson-12.png", "Lesson 12: chords", "Lesson 12: sixteen chord names on the keys, with the words: One key, one chord. Play the blinking keys."),
        ("lesson-13.png", "Lesson 13: piano roll", "Lesson 13: the piano roll, with the words across the top: KNOB1 to step 7.")]
SHOTS = [("lesson-16.png", "Lesson 16: cheat sheet", "Lesson 16's last card, a cheat sheet: A page is also called a pattern. Go to a page: SAVE + a key. Copy this page: SAVE + hold a key."),
         ("lesson-20.png", "Lesson 20: a filter", "Lesson 20, a card: A filter is a DJ's favorite trick. Close it: the song sinks underwater. Open it: it bursts back to life."),
         ("lesson-21.png", "Lesson 21: a sound", "Lesson 21: a sound's envelope drawn as a curve, with the words: KNOB1, ATK: up."),
         ("lesson-22.png", "Lesson 22: wobble", "Lesson 22: a slow wave drawn across the screen, with the words: KNOB1, RATE: try it."),
         ("lesson-25.png", "Lesson 25: a song", "Lesson 25: the song page with its eight pages, with the words: The chain plays: 1, 2, 3, round and round."),
         ("lesson-30.png", "Lesson 30: done", "The last card: Lesson 30, Done")]
def shot(name): return base64.b64encode(open(os.path.join("assets", "shots", name), "rb").read()).decode()
def figures(shots): return "".join(
    '<figure><img src="data:image/png;base64,%s" alt="%s" width="240" height="240"><figcaption>%s</figcaption></figure>\n' % (shot(n), alt, cap)
    for n, cap, alt in shots)
SHOW = '<section class="tp-show" aria-label="What the lessons look like">\n<div class="tp-shots">\n' + figures(SHOTS) + """</div>
</section>
<section class="tp-ai" aria-labelledby="tpAiT">
<h2 id="tpAiT">Take it further with an AI assistant</h2>
<p class="tp-ai-sub">The lessons are plain text files, written to be read by people and by AI. Open this project on a computer, in an AI assistant that works with files, and you have a tutor for the learner, a lesson writer for you, and a second pair of hands on the FM&#8209;1. Teachie Beachie was built and tested this way.</p>
<div class="tp-three">
<div class="tp-box"><h3>Ask about any step</h3>
<p>The assistant knows every lesson: each step, its hints, and what usually goes wrong. Ask it what a step wants, or have it explain an idea another way.</p></div>
<div class="tp-box"><h3>Control the FM&#8209;1</h3>
<p>With the FM&#8209;1 plugged in, the assistant can work it: load a lesson's song, press play, change the tempo, write notes, light up a button. It can show a move, or set up a practice round.</p></div>
<div class="tp-box"><h3>Change a lesson</h3>
<p>Tell it a card is confusing. It rewrites the card, checks that the words fit the FM&#8209;1's screen, tests the lesson on a simulated FM&#8209;1, and sends it to yours.</p></div>
<div class="tp-box"><h3>Write your own</h3>
<p>A new lesson for a kid who wants more drums, or an easier one. The assistant writes it, tests it the same way, and adds it to the FM&#8209;1.</p></div>
</div>
<p class="tp-ai-more"><a href="__HOME__/blob/main/tutorial/AI.md">How to set it up, and what to ask</a>. Any assistant that can read files and run commands will do.</p>
</section>

""".replace("__HOME__", HOME)
new = 0
# the chip: tried on one FM-1 so far, which has the original flash chip (0x856014); FM-1s sold since fall 2026 have
# another (SLOOP's own note says so in step 2, which only those setting up m-bootloader see). Said in step 1, after
# the beta note, where everyone reads it
CHIP = ('<div class="note warn"><b>Tested on the original flash chip only</b><span>Teachie Beachie has been tested on '
        'my FM&#8209;1, which has the original flash chip. FM&#8209;1s sold since fall 2026 can have a newer chip, the '
        '<b>UCUN UC25HQ80</b>, and neither Teachie Beachie nor SLOOP 3 has been tried on one yet. If yours is one of '
        'them, you would be among the first.</span></div>\n')
m = re.search(r'<div class="note warn"><b data-t="betaT">.*?</div>\n', t, flags=re.S)
if m: t = t[:m.end()] + CHIP + t[m.end():]; new += 1
else: missed.append("step 1's beta note, after which the chip's note goes")
a = '<ul class="facts">'                                 # (in the top's own block, after the introduction: the second column)
if a in t: t = t.replace(a, '<div class="tp-shots tp-side" aria-label="More of the lessons">\n' + figures(SIDE) + "</div>\n" + a, 1); new += 1
else: missed.append("the top's facts, before which the six screens beside the introduction go")
a = '<div class="wizard" id="wizard">'
if a in t: t = t.replace(a, SHOW + a, 1); new += 1
else: missed.append("the steps, before which the lessons' screens go")
a = '<div class="checks" id="checks"></div>\n<div class="blocker hide" id="blocker"></div>\n'
m = re.search(r'<p class="sub" data-t="s1">.*?</p>\n', t, flags=re.S)
if a in t and m: t = t.replace(a, "", 1); m = re.search(r'<p class="sub" data-t="s1">.*?</p>\n', t, flags=re.S); t = t[:m.end()] + a + t[m.end():]; new += 1
else: missed.append("the browser's checks, or step 1's first words, where they go")
notes = [re.search(r'<div class="note[^"]*"><b data-t="%s">.*?</div>\n' % k, t, flags=re.S) for k in ("stableT", "upT")]
a = ONLY
if all(notes) and a in t:
    for g in notes: t = t.replace(g.group(0), "", 1)
    t = t.replace(a, '<details class="more tp-sloop"><summary>Already use SLOOP?</summary><div>\n' + "".join(g.group(0) for g in notes) + "</div></details>\n" + a, 1); new += 1
else: missed.append("step 1's notes for SLOOP's users")
t, n = re.subn(r'(data-t="e[1-5]">Step [1-5]) of 5<', r"\1 of 6<", t)
if n == 5: new += 1
else: missed.append("the steps' count (%d of 5)" % n)
if new != 6: print("teachie_page: the page for a newcomer is not whole (%d of 6)" % new, file=sys.stderr)

open(path, "w", encoding="utf-8").write(t)
for s in missed: print("teachie_page: not found (SLOOP reworded it?):", s, file=sys.stderr)
credited = "built on <b>SLOOP 3</b>" in t
print("%s: Teachie Beachie %s on SLOOP %s, %d names, %d versions, %d logos, its look %s, %s, %s" % (path, TP_VERSION, sloop_ver, renamed, nver,
      logos, "added" if looks else "MISSING", "SLOOP credited" if credited else "SLOOP NOT CREDITED",
      "%d lessons inside (%d bytes)" % (len(sizes), len(data)) if lessons == 3 else "ITS LESSONS MISSING"))
if not credited or "<title>Install Teachie Beachie" not in t: sys.exit("teachie_page: the page lacks its name or the credit to SLOOP")
if lessons != 3: sys.exit("teachie_page: the page lacks its lessons (%d of 3 places found)" % lessons)
