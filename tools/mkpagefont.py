#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: the installer page's font, the lessons' own (Google Sans, (c) The Google Sans Project Authors, SIL
# Open Font License 1.1, assets/fonts/google-sans/), made once into assets/fonts/page-font.woff, which
# tools/teachie_page.py puts inside the page (the page sends nothing anywhere: no font from the web).
#   mkpagefont.py
# Weights 400 to 700 kept (a variable font), the other two axes fixed at their defaults; the letters of English and
# French and the page's punctuation only. The font's copyright and license stay in it (its name table, whole).
# Needs fontTools, and the typeface's own file (not in this repository, see assets/fonts/.gitignore):
# GoogleSans[GRAD,opsz,wght].ttf from github.com/google/fonts, folder ofl/googlesans.
import os
from fontTools.ttLib import TTFont
from fontTools.varLib import instancer
from fontTools import subset

SRC = "assets/fonts/google-sans/GoogleSans[GRAD,opsz,wght].ttf"
OUT = "assets/fonts/page-font.woff"
UNI = (list(range(0x20, 0x7F)) + list(range(0xA0, 0x100)) + [0x152, 0x153, 0x178, 0x2BC, 0x2C6, 0x2DC]
       + list(range(0x2010, 0x2028)) + list(range(0x2030, 0x203B)) + [0x2044, 0x20AC, 0x2122] + list(range(0x2190, 0x2194)))

f = TTFont(SRC, lazy=False)
opt = subset.Options(); opt.name_IDs = ["*"]; opt.name_languages = ["*"]; opt.layout_features = ["*"]; opt.notdef_outline = True
s = subset.Subsetter(opt); s.populate(unicodes=UNI); s.subset(f)          # (the letters first, then the axes: the other
f = instancer.instantiateVariableFont(f, {"GRAD": 0, "opsz": 18, "wght": (400, 700)})   # way round, fontTools trips)
f.flavor = "woff"; f.save(OUT)
print(OUT, os.path.getsize(OUT), "bytes")
