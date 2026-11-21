#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: SLOOP's installer page (tools/mkpage.py's output) under TEACHIE BEACHIE's name, in place. Run by fw/build.sh
# after mkpage.py, so web/installer.tmpl.html stays exactly SLOOP's and a merge of a new SLOOP never clashes there.
#   teachie_page.py PAGE.html
# What it does: TEACHIE BEACHIE where the page names the firmware it installs; SLOOP 3 credited in the title, the beta note
# and the footer (English and French); bug reports and the page's own address to TEACHIE BEACHIE's; the version shown as
# TEACHIE BEACHIE's, then SLOOP's; TEACHIE BEACHIE's logo by the name, as the tab's icon and above the title (the two
# pictures are made by tools/mkpagelogo.py). A rule that no longer finds its text is reported (SLOOP reworded the page:
# look again); the build stops only if the credit to SLOOP or the name would be missing.
import base64, os, re, sys

TP_VERSION = "0.1"
HOME = "https://github.com/davydegravy/teachie-beachie-fm1"
PAGE = "https://davydegravy.github.io/teachie-beachie-fm1/"

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

# 2. the sentences that need SLOOP 3 named, or TEACHIE BEACHIE's own words
RULES = [
    ('content="Install TEACHIE BEACHIE, the free groovebox firmware for the M-VAVE FM-1,',
     'content="Install TEACHIE BEACHIE, free firmware that teaches the M-VAVE FM-1 in thirty lessons, built on SLOOP 3,'),
    # what TEACHIE BEACHIE is, in its author's own voice: made for his children, the lessons, on top of SLOOP 3 (the
    # page's words for SLOOP described a groovebox only)
    ('TEACHIE BEACHIE turns the M-VAVE FM-1 into a complete groovebox: drums, bass, chords, melodies and effects, with a '
     'sequencer that loops it all. This page installs it for you, step by step, right from your browser.',
     'I built TEACHIE BEACHIE for my children, so they can learn what a sequencer and a groovebox are and make their own '
     'songs on an FM-1, their Christmas gift. It teaches the M-VAVE FM-1 on the FM-1 itself: thirty short lessons, from '
     'a first beat to a whole song. It is built on SLOOP 3, a complete groovebox, which stays when the lessons are '
     'done. This page installs it for you, step by step, right from your browser.'),
    ("TEACHIE BEACHIE fait du M-VAVE FM-1 une groovebox complète : batterie, basse, accords, mélodies et effets, avec un "
     "séquenceur qui boucle le tout. Cette page l\\'installe pour toi, étape par étape, depuis ton navigateur.",
     "J\\'ai créé TEACHIE BEACHIE pour mes enfants, pour qu\\'ils apprennent ce que sont un séquenceur et une groovebox et "
     "qu\\'ils fassent leurs propres morceaux sur un FM-1, leur cadeau de Noël. Il apprend le M-VAVE FM-1 sur le FM-1 "
     "lui-même : trente courtes leçons, d\\'un premier rythme à un morceau entier. Il repose sur SLOOP 3, une groovebox "
     "complète, qui reste une fois les leçons finies. Cette page l\\'installe pour toi, étape par étape, depuis ton "
     "navigateur."),
    ('The FM-1 running TEACHIE BEACHIE (3D render)', 'The M-VAVE FM-1, with SLOOP 3 on its screen (3D render)'),
    ('Le FM-1 avec TEACHIE BEACHIE (rendu 3D)', "Le M-VAVE FM-1, avec SLOOP 3 à l\\'écran (rendu 3D)"),
    ('The M-VAVE FM-1 running TEACHIE BEACHIE', 'The M-VAVE FM-1'),
    ('Le M-VAVE FM-1 avec TEACHIE BEACHIE', 'Le M-VAVE FM-1'),
    # after the install: the peach and the start card, not SLOOP's sailboat; and where the lessons come from
    ('The FM-1 restarts on TEACHIE BEACHIE and the sailboat logo draws itself in. Turn <b>MASTER</b> up gently, and have fun.',
     'The FM-1 restarts on TEACHIE BEACHIE: the peach logo, then a card that says READY FOR A LESSON. <kbd>OCT+</kbd> starts '
     'a lesson, <kbd>OCT-</kbd> goes straight to SLOOP 3. Turn <b>MASTER</b> up gently, and have fun. The thirty lessons are a '
     'small pack of their own, which this page does not install yet: <a href="' + HOME + '">the project\'s page</a> says how '
     'to put it on the FM-1.'),
    ("Le FM-1 redémarre sur TEACHIE BEACHIE et le voilier se dessine. Monte <b>MASTER</b> doucement, et amuse-toi bien.",
     "Le FM-1 redémarre sur TEACHIE BEACHIE : la pêche, puis une carte qui dit READY FOR A LESSON. <kbd>OCT+</kbd> lance une "
     "leçon, <kbd>OCT-</kbd> va droit à SLOOP 3. Monte <b>MASTER</b> doucement, et amuse-toi bien. Les trente leçons sont un "
     "petit paquet à part, que cette page n\\'installe pas encore : <a href=\"" + HOME + "\">la page du projet</a> explique "
     "comment le mettre sur le FM-1."),
    # the first start: TEACHIE BEACHIE leaves out SLOOP 3's opening of a SLOOP 2.8 project (TP_SLOOP28_PROJECT in fw/main.c)
    ("At its first start, TEACHIE BEACHIE opens what SLOOP 2.8 left: your sections become patterns, and your presets, "
     "favourites, samples, SYN kits, FM6 bank and settings come along. SLOOP 2.8's data is only read, never written.",
     "TEACHIE BEACHIE does not open the project SLOOP 2.8 left (SLOOP 3 does; TEACHIE BEACHIE leaves that out to make room "
     "for its lessons). Your presets, favorites, samples, SYN kits, FM6 bank and settings come along. SLOOP 2.8's data is "
     "only read, never written."),
    ("Au premier démarrage, TEACHIE BEACHIE ouvre ce que SLOOP 2.8 a laissé : tes sections deviennent des patterns, et tes "
     "presets, favoris, samples, kits SYN, banque FM6 et réglages suivent. Les données de SLOOP 2.8 sont seulement lues, "
     "jamais écrites.",
     "TEACHIE BEACHIE n\\'ouvre pas le projet que SLOOP 2.8 a laissé (SLOOP 3 le fait ; TEACHIE BEACHIE s\\'en passe pour "
     "faire de la place à ses leçons). Tes presets, favoris, samples, kits SYN, banque FM6 et réglages suivent. Les données "
     "de SLOOP 2.8 sont seulement lues, jamais écrites."),
    ('data-t="betaT">TEACHIE BEACHIE is a public beta', 'data-t="betaT">TEACHIE BEACHIE runs SLOOP 3, a public beta'),
    ("betaT: 'TEACHIE BEACHIE est une bêta publique'", "betaT: 'TEACHIE BEACHIE fait tourner SLOOP 3, une bêta publique'"),
    ('<p data-t="foot1"><b>TEACHIE BEACHIE</b> is free software (GPL-3.0) by the SLOOP project: '
     '<a href="https://github.com/isod89/sloop3-fm1">github.com/isod89/sloop3-fm1</a>.',
     '<p data-t="foot1"><b>TEACHIE BEACHIE</b> is free software (GPL-3.0) by Davy DeGravy, built on <b>SLOOP 3</b> by the '
     'SLOOP project (<a href="https://github.com/isod89/sloop3-fm1">github.com/isod89/sloop3-fm1</a>). '
     'The SLOOP project does not make or support TEACHIE BEACHIE.'),
    ("foot1: '<b>TEACHIE BEACHIE</b> est un logiciel libre (GPL-3.0) du projet SLOOP : "
     "<a href=\"https://github.com/isod89/sloop3-fm1\">github.com/isod89/sloop3-fm1</a>.",
     "foot1: '<b>TEACHIE BEACHIE</b> est un logiciel libre (GPL-3.0) de Davy DeGravy, construit sur <b>SLOOP 3</b> du "
     "projet SLOOP (<a href=\"https://github.com/isod89/sloop3-fm1\">github.com/isod89/sloop3-fm1</a>). "
     "Le projet SLOOP ne fait ni ne prend en charge TEACHIE BEACHIE."),
]
missed = []
for old, new in RULES:
    if old in t: t = t.replace(old, new)
    else: missed.append(old[:70])

# 3. where to report a bug, and the page's own address (TEACHIE BEACHIE's, not SLOOP's)
t = t.replace("https://github.com/isod89/sloop3-fm1/issues/new/choose", HOME + "/issues")
t = t.replace("<code>https://isod89.github.io/sloop3-fm1/</code>", "<code>" + PAGE + "</code>")

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
    t = t.replace(hero, '<img src="data:image/png;base64,%s" alt="Teachie Beachie" style="display:block;width:160px;height:160px;margin:0 0 12px -8px">' % b64("teachie-beachie-page.png") + hero, 1); logos += 1
else: missed.append("the title's badge, where the logo goes")

open(path, "w", encoding="utf-8").write(t)
for s in missed: print("teachie_page: not found (SLOOP reworded it?):", s, file=sys.stderr)
credited = "built on <b>SLOOP 3</b>" in t and "construit sur <b>SLOOP 3</b>" in t
print("%s: TEACHIE BEACHIE %s on SLOOP %s, %d names, %d versions, %d logos, %s" % (path, TP_VERSION, sloop_ver, renamed, nver, logos,
      "SLOOP credited" if credited else "SLOOP NOT CREDITED"))
if not credited or "<title>Install TEACHIE BEACHIE" not in t: sys.exit("teachie_page: the page lacks its name or the credit to SLOOP")
