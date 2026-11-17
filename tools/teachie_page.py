#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Peachie: SLOOP's installer page (tools/mkpage.py's output) under TEACHIE PEACHIE's name, in place. Run by fw/build.sh
# after mkpage.py, so web/installer.tmpl.html stays exactly SLOOP's and a merge of a new SLOOP never clashes there.
#   teachie_page.py PAGE.html
# What it does: TEACHIE PEACHIE where the page names the firmware it installs; SLOOP 3 credited in the title, the beta note
# and the footer (English and French); bug reports and the page's own address to TEACHIE PEACHIE's; the version shown as
# TEACHIE PEACHIE's, then SLOOP's. A rule that no longer finds its text is reported (SLOOP reworded the page: look again);
# the build stops only if the credit to SLOOP or the name would be missing.
import re, sys

TP_VERSION = "0.1"
HOME = "https://github.com/davydegravy/teachie-peachie-fm1"
PAGE = "https://davydegravy.github.io/teachie-peachie-fm1/"

path = sys.argv[1]
t = open(path, encoding="utf-8").read()
m = re.search(r'const VERSION = "([^"]+)"', t)
sloop_ver = m.group(1) if m else "?"

# 1. the firmware's name: "SLOOP 3" alone (not "SLOOP 3.0", "SLOOP 3.3.4": those are SLOOP's own versions); the
#    installing code inlined from web/fwpkg.js is left as SLOOP wrote it (its comments describe SLOOP's loader)
pkg = open("web/fwpkg.js", encoding="utf-8").read()
held = pkg in t
if held: t = t.replace(pkg, "\0FWPKG\0")
t, renamed = re.subn(r"SLOOP 3(?!\.\d|\d)", "TEACHIE PEACHIE", t)
if held: t = t.replace("\0FWPKG\0", pkg)

# 2. the sentences that need SLOOP 3 named, or TEACHIE PEACHIE's own words
RULES = [
    ('content="Install TEACHIE PEACHIE, the free groovebox firmware for the M-VAVE FM-1,',
     'content="Install TEACHIE PEACHIE, a free firmware for the M-VAVE FM-1 built on SLOOP 3,'),
    ('data-t="betaT">TEACHIE PEACHIE is a public beta', 'data-t="betaT">TEACHIE PEACHIE runs SLOOP 3, a public beta'),
    ("betaT: 'TEACHIE PEACHIE est une bêta publique'", "betaT: 'TEACHIE PEACHIE fait tourner SLOOP 3, une bêta publique'"),
    ('<p data-t="foot1"><b>TEACHIE PEACHIE</b> is free software (GPL-3.0) by the SLOOP project: '
     '<a href="https://github.com/isod89/sloop3-fm1">github.com/isod89/sloop3-fm1</a>.',
     '<p data-t="foot1"><b>TEACHIE PEACHIE</b> is free software (GPL-3.0) by Davy DeGravy, built on <b>SLOOP 3</b> by the '
     'SLOOP project (<a href="https://github.com/isod89/sloop3-fm1">github.com/isod89/sloop3-fm1</a>). '
     'The SLOOP project does not make or support TEACHIE PEACHIE.'),
    ("foot1: '<b>TEACHIE PEACHIE</b> est un logiciel libre (GPL-3.0) du projet SLOOP : "
     "<a href=\"https://github.com/isod89/sloop3-fm1\">github.com/isod89/sloop3-fm1</a>.",
     "foot1: '<b>TEACHIE PEACHIE</b> est un logiciel libre (GPL-3.0) de Davy DeGravy, construit sur <b>SLOOP 3</b> du "
     "projet SLOOP (<a href=\"https://github.com/isod89/sloop3-fm1\">github.com/isod89/sloop3-fm1</a>). "
     "Le projet SLOOP ne fait ni ne prend en charge TEACHIE PEACHIE."),
]
missed = []
for old, new in RULES:
    if old in t: t = t.replace(old, new)
    else: missed.append(old[:70])

# 3. where to report a bug, and the page's own address (TEACHIE PEACHIE's, not SLOOP's)
t = t.replace("https://github.com/isod89/sloop3-fm1/issues/new/choose", HOME + "/issues")
t = t.replace("<code>https://isod89.github.io/sloop3-fm1/</code>", "<code>" + PAGE + "</code>")

# 4. the version shown: TEACHIE PEACHIE's, then SLOOP's (the badge at the top and the summary before installing)
shown = "v%s · SLOOP %s" % (TP_VERSION, sloop_ver)
t, nver = re.subn(r'(<span class="badge ver">|<dd>TEACHIE PEACHIE )v' + re.escape(sloop_ver) + r'(?=[ <])', lambda g: g.group(1) + shown, t)

open(path, "w", encoding="utf-8").write(t)
for s in missed: print("teachie_page: not found (SLOOP reworded it?):", s, file=sys.stderr)
credited = "built on <b>SLOOP 3</b>" in t and "construit sur <b>SLOOP 3</b>" in t
print("%s: TEACHIE PEACHIE %s on SLOOP %s, %d names, %d versions, %s" % (path, TP_VERSION, sloop_ver, renamed, nver,
      "SLOOP credited" if credited else "SLOOP NOT CREDITED"))
if not credited or "<title>Install TEACHIE PEACHIE" not in t: sys.exit("teachie_page: the page lacks its name or the credit to SLOOP")
