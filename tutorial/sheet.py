#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Peachie: the screens a run on the simulated FM-1 saved, side by side in one picture.
    python3 tutorial/sheet.py build/tw/lessons/02-sixteen-boxes [out.png]"""
import os, sys
from PIL import Image

def sheet(folder, out=None, cols=6, names=None):
    files = sorted(f for f in os.listdir(folder) if f.endswith(".ppm") and (names is None or f[:-4] in names))
    if not files: return None
    ims = [Image.open(os.path.join(folder, f)).convert("RGB") for f in files]
    w, h = ims[0].size; cols = min(cols, len(ims)); rows = (len(ims) + cols - 1) // cols; gap = 10
    page = Image.new("RGB", (cols * (w + gap) + gap, rows * (h + gap) + gap), (40, 40, 40))
    for i, im in enumerate(ims): page.paste(im, (gap + (i % cols) * (w + gap), gap + (i // cols) * (h + gap)))
    out = out or os.path.join(folder, "screens.png"); page.save(out); return out

if __name__ == "__main__": print(sheet(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None))
