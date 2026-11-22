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
# A rule that no longer finds its text is reported (SLOOP reworded the page: look again); the build stops only if the
# credit to SLOOP or the name would be missing.
import base64, os, re, sys

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
     'content="Install TEACHIE BEACHIE, free firmware that teaches the M-VAVE FM-1 in thirty lessons, built on SLOOP 3,'),
    # what Teachie Beachie is, in a few words (the page's words for SLOOP described a groovebox only; &#8209;, a hyphen
    # that does not break the line, keeps M-VAVE and FM-1 whole on a phone)
    ('TEACHIE BEACHIE turns the M-VAVE FM-1 into a complete groovebox: drums, bass, chords, melodies and effects, with a '
     'sequencer that loops it all. This page installs it for you, step by step, right from your browser.',
     'Teachie Beachie is a tutorial for beginners and people new to the M&#8209;VAVE FM&#8209;1 and SLOOP. Its simple, '
     'intuitive lessons, on the FM-1 itself, guide you step by step from a first beat to a whole song of your own. '
     'Teachie Beachie can light up the FM-1\'s buttons and keys, notices when you are stuck and offers a hint, and can '
     'even complete steps when you ask it to. It\'s the best way to learn the basics of sequencers and grooveboxes as '
     'well as specific workflows in SLOOP.'),
    ('The FM-1 running TEACHIE BEACHIE (3D render)', 'The M-VAVE FM-1, with SLOOP 3 on its screen (3D render)'),
    ('The M-VAVE FM-1 running TEACHIE BEACHIE', 'The M-VAVE FM-1'),
    # after the install: the peach and the start card, not SLOOP's sailboat; and where the lessons come from
    ('The FM-1 restarts on TEACHIE BEACHIE and the sailboat logo draws itself in. Turn <b>MASTER</b> up gently, and have fun.',
     'The FM-1 restarts on Teachie Beachie: the peach logo, then a card that asks "Ready for a lesson?" <kbd>OCT+</kbd> starts '
     'a lesson, <kbd>OCT-</kbd> goes straight to SLOOP 3. Turn <b>MASTER</b> up gently, and have fun. The thirty lessons are a '
     'small pack of their own, which this page does not install yet: <a href="' + HOME + '">the project\'s page</a> says how '
     'to put it on the FM-1.'),
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
#    script still finds them; the language follows the browser). At the top: the logo, the title, a few words, the
#    browser's checks; SLOOP's beta badge, its three facts and the 3D picture hidden.
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
.hero{display:block;text-align:center;max-width:760px;margin:0 auto;padding:44px 0 10px}
.hero .badge,.hero .facts,.hero .shot{display:none}
.hero .tp-logo{display:block;width:220px;height:220px;margin:0 auto 4px}
.hero h1{font-size:44px;font-weight:700;margin:6px 0 0;letter-spacing:-.02em}
.hero h1 span{background:none;-webkit-background-clip:border-box;background-clip:border-box;color:var(--ink)}
.hero .lead{font-size:19px;color:var(--ink);max-width:34em;margin:14px auto 0}
.checks{justify-content:center}
.chk{background:var(--panel);border-color:var(--line2);color:var(--ink)}
.chk i{background:var(--ink);color:var(--paper)}
.chk.no{border-color:var(--ink);border-style:dashed;color:var(--ink)}.chk.no i{background:transparent;color:var(--ink);box-shadow:inset 0 0 0 1.5px var(--ink)}
.blocker{text-align:left;border:2px solid var(--ink);background:var(--panel)}.blocker b{color:var(--ink)}
@media(max-width:900px){.hero{padding-top:26px}.hero h1{font-size:34px}.hero .tp-logo{width:160px;height:160px}}
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

open(path, "w", encoding="utf-8").write(t)
for s in missed: print("teachie_page: not found (SLOOP reworded it?):", s, file=sys.stderr)
credited = "built on <b>SLOOP 3</b>" in t
print("%s: Teachie Beachie %s on SLOOP %s, %d names, %d versions, %d logos, its look %s, %s" % (path, TP_VERSION, sloop_ver, renamed, nver,
      logos, "added" if looks else "MISSING", "SLOOP credited" if credited else "SLOOP NOT CREDITED"))
if not credited or "<title>Install Teachie Beachie" not in t: sys.exit("teachie_page: the page lacks its name or the credit to SLOOP")
