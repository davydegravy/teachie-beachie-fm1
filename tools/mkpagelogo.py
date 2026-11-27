#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: the logo for the installer page, cut out of its paper so it sits on the page's own peach.
#   python3 tools/mkpagelogo.py assets/logo/teachie-beachie.webp assets/logo
# writes teachie-beachie-page.png (the logo at the top of the page, 220 px there: sharp on a fine screen) and teachie-beachie-mark.png (the small mark by the
# name, and the browser tab's icon). Run by hand when the logo changes (it needs PIL); tools/teachie_page.py only reads
# the two files, so the build needs nothing new.
import os, sys
from PIL import Image, ImageDraw, ImageFilter

src, out = sys.argv[1], sys.argv[2]
im = Image.open(src).convert("RGB"); w, h = im.size
key = (255, 0, 255)                                   # the paper, flooded from the four corners
for xy in ((0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1)): ImageDraw.floodfill(im, xy, key, thresh=60)
mask = Image.new("L", (w, h), 255); px, mp = im.load(), mask.load()
for y in range(h):
    for x in range(w):
        if px[x, y] == key: mp[x, y] = 0
mask = mask.filter(ImageFilter.MinFilter(7))          # (the paper's fringe along the black outline goes with it)
im = Image.open(src).convert("RGBA"); im.putalpha(mask)
im = im.crop(mask.getbbox())
side = max(im.size); sq = Image.new("RGBA", (side, side), (0, 0, 0, 0)); sq.paste(im, ((side - im.size[0]) // 2, (side - im.size[1]) // 2))
for name, size in (("teachie-beachie-page.png", 440), ("teachie-beachie-mark.png", 96)):
    small = sq.resize((size, size), Image.LANCZOS)
    small.save(os.path.join(out, name), optimize=True)
    print(name, small.size, os.path.getsize(os.path.join(out, name)), "bytes")
