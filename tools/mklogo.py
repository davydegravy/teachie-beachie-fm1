#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: the start screen's logo, kept in the lesson slot (since 2026-11-25; before, app/tp_logo.h in the
# firmware). tools/mkcourse.py puts it at the end of the slot, fw/course.inc finds it, app/vis.c draws it.
#
# The picture (the logo on a plain background) is cut to the sticker and made flat at 8 times the size: each pixel
# one of the sticker's own colours (PAL, picked by hand: a new picture needs its own), the background round it the
# start screen's peach. Then it is shrunk to W x H, each pixel the mean of its 8 x 8: flat colours with smooth edges.
#
# Written in full colour (RGB565), row by row, every number little-endian:
#   "TPL1"  u8 W  u8 H  u16 every 16th row's place (from the start of this file), (H + 15) / 16 of them  then the rows
#   a row: runs until W pixels:  b < 0x80: b + 1 pixels of the colour that follows (u16)
#                                b >= 0x80: (b & 0x7F) + 1 pixels, each its own colour (u16 each)
# Every 16th row begins at its place, so a tile of the screen (16 rows high) reads from the one at or above it.
#   mklogo.py [PICTURE] [OUT.bin] [HEIGHT]     (needs Pillow and numpy; HEIGHT 204, the most over WITH SLOOP 3)
import os, struct, sys
import numpy as np
from PIL import Image, ImageFilter

HERE = os.path.dirname(os.path.abspath(__file__))
src = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "..", "assets", "logo", "teachie-beachie.webp")
out = sys.argv[2] if len(sys.argv) > 2 else os.path.join(HERE, "..", "assets", "logo", "teachie-beachie-fm1.bin")
H = int(sys.argv[3]) if len(sys.argv) > 3 else 204
UP = 8                                                        # the flat picture's size, times the screen's
PAPER = (0xFF, 0xCB, 0xA4)                                    # TP_PAPER (app/vis.h): the start screen's peach
PAL = [None, (255, 255, 255), (0, 0, 0), (254, 117, 47), (254, 214, 111), (2, 97, 123)]
# the background (from the picture's corner), white, the outline, the sky, the sun and the sand, the sea
# (the round logo, 2026-11-27; the peach before it had a leaf and a stem too)

im = Image.open(src).convert("RGB")
px = im.load(); w0, h0 = im.size
PAL[0] = px[4, 4]
far = lambda a, b: sum((x - y) ** 2 for x, y in zip(a, b))
ink = [(x, y) for x in range(0, w0, 3) for y in range(0, h0, 3) if far(px[x, y], PAL[0]) > 3000]
im = im.crop((min(x for x, _ in ink), min(y for _, y in ink), max(x for x, _ in ink) + 3, max(y for _, y in ink) + 3))
W = round(H * im.size[0] / im.size[1])
assert W <= 240 and H <= 240, "larger than the screen"

# flat, at UP times the size: the picture's noise smoothed out, then each pixel its nearest colour of PAL
pal = Image.new("P", (1, 1)); pal.putpalette(sum(PAL, ()) + PAL[2] * (256 - len(PAL)))
big = im.resize((W * UP, H * UP), Image.LANCZOS).filter(ImageFilter.MedianFilter(2 * UP + 1))
idx = np.array(big.quantize(palette=pal, dither=Image.Dither.NONE)); idx[idx >= len(PAL)] = 2
# only the background that touches the picture's edge is background: a speck of its colour inside the sticker (the
# soft edge of a letter) takes the nearest of the sticker's own colours instead
try:
    from scipy import ndimage
    lab, _ = ndimage.label(idx == 0)
    edge = set(np.unique(np.concatenate([lab[0], lab[-1], lab[:, 0], lab[:, -1]]))) - {0}
    outside = np.isin(lab, list(edge))
except ImportError:                                           # (without scipy: grown from the edge, slower)
    outside = np.zeros(idx.shape, bool); bg = idx == 0
    outside[0], outside[-1], outside[:, 0], outside[:, -1] = bg[0], bg[-1], bg[:, 0], bg[:, -1]
    while True:
        g = outside.copy(); g[1:] |= outside[:-1]; g[:-1] |= outside[1:]; g[:, 1:] |= outside[:, :-1]; g[:, :-1] |= outside[:, 1:]; g &= bg
        if (g == outside).all(): break
        outside = g
inner = (idx == 0) & ~outside
if inner.any():
    rgb = np.array(big, dtype=np.int32)[inner]
    d = np.stack([((rgb - np.array(PAL[k])) ** 2).sum(1) for k in range(1, len(PAL))]); idx[inner] = 1 + d.argmin(0)
cols = np.array([PAPER] + PAL[1:], dtype=np.float64)
flat = cols[idx]                                              # (H*UP, W*UP, 3)
# shrunk: each pixel the mean of its UP x UP, then RGB565
small = flat.reshape(H, UP, W, UP, 3).mean(axis=(1, 3))
r, g, b = (np.clip(np.rint(small[..., i] * m / 255.0), 0, m).astype(np.uint16) for i, m in ((0, 31), (1, 63), (2, 31)))
c565 = (r << 11 | g << 5 | b).astype(np.uint16)

def row_bytes(row):
    """a row's runs: a colour repeated as one run (2 or more pixels), else the pixels one by one, together"""
    out, x, lit = bytearray(), 0, []
    def flush():
        while lit:
            n = min(len(lit), 128); out.append(0x80 | (n - 1)); out.extend(struct.pack("<%dH" % n, *lit[:n])); del lit[:n]
    while x < W:
        n = 1
        while x + n < W and row[x + n] == row[x]: n += 1
        if n >= 2:
            flush()
            m = n
            while m: k = min(m, 128); out.append(k - 1); out.extend(struct.pack("<H", int(row[x]))); m -= k
        else: lit.append(int(row[x]))
        x += n
    flush()
    return bytes(out)

nrow16 = (H + 15) // 16
head = 6 + 2 * nrow16
rows, places, at = [], [], head
for y in range(H):
    rb = row_bytes(c565[y])
    if y % 16 == 0: places.append(at)
    rows.append(rb); at += len(rb)
data = b"TPL1" + bytes([W, H]) + struct.pack("<%dH" % nrow16, *places) + b"".join(rows)
assert len(data) < 65536

# read back the way app/vis.c does it: the same picture, or nothing is written
def draw(d):
    w, h = d[4], d[5]; back = np.zeros((h, w), np.uint16)
    for y0 in range(0, h, 16):
        p = struct.unpack_from("<H", d, 6 + 2 * (y0 // 16))[0]
        for y in range(y0, min(y0 + 16, h)):
            x = 0
            while x < w:
                b = d[p]; p += 1; n = (b & 0x7F) + 1
                if b < 0x80: back[y, x:x + n] = struct.unpack_from("<H", d, p)[0]; p += 2
                else: back[y, x:x + n] = struct.unpack_from("<%dH" % n, d, p); p += 2 * n
                x += n
            assert x == w, "a row runs past the picture"
    return back
assert (draw(data) == c565).all()

open(out, "wb").write(data)
pv = np.stack([(c565 >> 11) * 255 // 31, (c565 >> 5 & 63) * 255 // 63, (c565 & 31) * 255 // 31], -1).astype(np.uint8)
os.makedirs(os.path.join(HERE, "..", "build"), exist_ok=True)
Image.fromarray(pv).resize((W * 2, H * 2), Image.NEAREST).save(os.path.join(HERE, "..", "build", "logo-preview.png"))
print("%s: %dx%d, %d colours, %d bytes (build/logo-preview.png)" % (out, W, H, len(np.unique(c565)), len(data)), file=sys.stderr)
