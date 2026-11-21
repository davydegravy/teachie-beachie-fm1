#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: the start screen's logo for app/vis.c. The picture (the logo on a plain background) is cut to the
# sticker, made flat (a few colours, no gradients) and shrunk, then written row by row, as small as it goes:
#   a row of runs: a byte each, high nibble n, low nibble the colour; n < 15: n + 1 pixels; n = 15: the next byte pixels.
#   a row like the one above: 0xFF, then a nibble for each run but the first (high nibble first): how far its start
#     moved, -8..7, as 0..15. A run may shrink to nothing, and come back. Most rows of a flat picture are these.
# Colour 0 is the background round the sticker: not drawn, so the start screen's own colour shows there.
# Every 16th row is a row of runs and its place is kept, so a tile of the screen reads from the one above it.
#   mklogo.py PICTURE OUT.h [WIDTH]     (needs Pillow)
import sys
from PIL import Image, ImageDraw, ImageFilter

src, out = sys.argv[1], sys.argv[2]
W = int(sys.argv[3]) if len(sys.argv) > 3 else 168
im = Image.open(src).convert("RGB")
px = im.load(); w0, h0 = im.size
BG = px[4, 4]                                                 # the picture's background, from its corner
PAL = [BG, (255, 255, 255), (0, 0, 0), (248, 114, 61), (157, 201, 62), (192, 27, 31), (252, 204, 120), (4, 116, 136)]
# the background, white, the outline, the peach, the leaf, the stem, the sun and the sand, the sea
far = lambda a, b: sum((x - y) ** 2 for x, y in zip(a, b))
ink = [(x, y) for x in range(0, w0, 3) for y in range(0, h0, 3) if far(px[x, y], BG) > 3000]
box = (min(x for x, _ in ink), min(y for _, y in ink), max(x for x, _ in ink) + 3, max(y for _, y in ink) + 3)
im = im.crop(box)
H = round(W * im.size[1] / im.size[0])
pal = Image.new("P", (1, 1)); pal.putpalette(sum(PAL, ()) + PAL[2] * (256 - len(PAL)))
big = im.resize((W * 4, H * 4), Image.LANCZOS).filter(ImageFilter.MedianFilter(5))
q = big.quantize(palette=pal, dither=Image.Dither.NONE).resize((W, H), Image.NEAREST).filter(ImageFilter.ModeFilter(3))
q = q.point(lambda i: i if i < len(PAL) else 2)               # (the palette's padding is the outline's colour)
# only what touches the picture's edge is background: a speck of its colour inside the sticker (the soft edge of a
# letter) takes the nearest of the sticker's own colours instead
frame = Image.new("P", (W + 2, H + 2), 0); frame.paste(q, (1, 1)); ImageDraw.floodfill(frame, (0, 0), 255)
outside = frame.crop((1, 1, W + 1, H + 1)); small = big.resize((W, H), Image.BOX); idx = list(q.getdata())
for i, (o, c) in enumerate(zip(outside.getdata(), small.getdata())):
    if o == 255: idx[i] = 0
    elif idx[i] == 0: idx[i] = min(range(1, len(PAL)), key=lambda k: far(PAL[k], c))
assert max(idx) < len(PAL)
# a lone pixel of a colour none of its four neighbours has (what is left of a soft edge) takes its neighbours' colour
for y in range(H):
    for x in range(W):
        near = [idx[yy * W + xx] for xx, yy in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)) if 0 <= xx < W and 0 <= yy < H]
        if idx[y * W + x] not in near: idx[y * W + x] = max(set(near), key=near.count)
q.putdata(idx)

def runs(row):
    out, x = [], 0
    while x < len(row):
        n = 1
        while x + n < len(row) and row[x + n] == row[x]: n += 1
        out.append((row[x], n)); x += n
    return out
def as_runs(rs): return b"".join(bytes([(n - 1) << 4 | c]) if n <= 15 else bytes([0xF0 | c, n]) for c, n in rs)
def moved(cols, at, rs):
    """the row as the row above's runs (their colours, their starts) with the starts moved -8..7: the new starts, or None.
    Each old run takes the row's next run, if of its colour, or nothing; of the ways that work, the one that moves least."""
    ends, x = [], 0
    for _, n in rs: x += n; ends.append(x)
    best = {0: (0, [])}                                       # the row's runs taken so far: (moves, the starts)
    for i, c in enumerate(cols):
        nxt = {}
        for j, (cost, xs) in best.items():
            start = ends[j - 1] if j else 0; d = start - at[i]
            if i and not -8 <= d <= 7: continue
            v = (cost + abs(d), xs + [start])
            for jj in (j, j + 1) if j < len(rs) and rs[j][0] == c else (j,):
                if jj not in nxt or v[0] < nxt[jj][0]: nxt[jj] = v
        best = nxt
    return best[len(rs)][1] if len(rs) in best else None

data, rows16, cols, at, most, kinds = bytearray(), [], None, None, 0, [0, 0]
for y in range(H):
    rs = runs(idx[y * W:(y + 1) * W]); xs = None
    if y % 16 == 0: rows16.append(len(data))
    else: xs = moved(cols, at, rs)
    if xs is not None and 1 + len(cols) // 2 < len(as_runs(rs)):
        nib = [a - b + 8 for a, b in zip(xs[1:], at[1:])]; nib += [0] * (len(nib) & 1)
        data += bytes([0xFF]) + bytes(nib[i] << 4 | nib[i + 1] for i in range(0, len(nib), 2)); at = xs; kinds[1] += 1
    else:
        data += as_runs(rs); cols = [c for c, _ in rs]; at = [sum(n for _, n in rs[:i]) for i in range(len(rs))]; kinds[0] += 1
    most = max(most, len(cols))

# read back the way app/vis.c does it: the same picture, or nothing is written
back, p, k = [], 0, 0
for y in range(H):
    if data[p] == 0xFF:
        for i in range(1, k):
            if i & 1: p += 1
            ax[i] += (data[p] >> 4 if i & 1 else data[p] & 15) - 8
        p += 1
    else:
        assert y % 16 or p == rows16[y // 16]
        x, k, ax, ac = 0, 0, [], []
        while x < W:
            b = data[p]; p += 1; n = (b >> 4) + 1
            if n > 15: n = data[p]; p += 1
            ax.append(x); ac.append(b & 15); k += 1; x += n
        ax.append(W)
    for i in range(k): back += [ac[i]] * (ax[i + 1] - ax[i])
assert p == len(data) and back == idx and W < 256 and len(data) < 65536

rgb565 = lambda r, g, b: (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3
with open(out, "w") as f:
    f.write("/* SPDX-License-Identifier: GPL-3.0-only\n * Teachie Beachie's logo, written by tools/mklogo.py from assets/logo/teachie-beachie.jpg: do not edit.\n * (colour 0 is the background round it: not drawn) */\n")
    f.write("#define TP_LOGO_W %d\n#define TP_LOGO_H %d\n#define TP_LOGO_RUNS %d       /* the most runs in a row */\n" % (W, H, most))
    f.write("static const uint16_t tp_logo_pal[%d] = { %s };\n" % (len(PAL), ", ".join("0x%04X" % rgb565(*c) for c in PAL)))
    f.write("static const uint16_t tp_logo_row16[%d] = { %s };\n" % (len(rows16), ", ".join(map(str, rows16))))
    f.write("static const uint8_t tp_logo_rle[%d] = {\n" % len(data))
    for i in range(0, len(data), 24): f.write("    " + ", ".join("%d" % b for b in data[i:i + 24]) + ",\n")
    f.write("};\n")
pv = q.copy(); pv.putpalette(sum(PAL, ())); pv.convert("RGB").resize((W * 2, H * 2), Image.NEAREST).save(out.rsplit(".", 1)[0] + "-preview.png")
print("%s: %dx%d, %d colours, %d bytes (%d rows of runs, %d rows moved; as runs alone: %d), %d row marks" % (out, W, H, len(PAL), len(data), kinds[0], kinds[1], sum(len(as_runs(runs(idx[y * W:(y + 1) * W]))) for y in range(H)), len(rows16)), file=sys.stderr)
