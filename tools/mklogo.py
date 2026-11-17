#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Peachie: the start screen's logo for app/vis.c. The picture is cut to the sticker, made flat (a few colours,
# no gradients) and shrunk, then written as runs of one colour, row by row (no run crosses a row's end):
#   a byte: high nibble n, low nibble the colour; n < 15: n + 1 pixels; n = 15: 16 + the next byte pixels.
# Every 8th row's start is kept too, so a tile of the screen only reads its own rows.
#   mklogo.py PICTURE OUT.h [WIDTH]     (needs Pillow)
import sys
from PIL import Image, ImageFilter

src, out = sys.argv[1], sys.argv[2]
W = int(sys.argv[3]) if len(sys.argv) > 3 else 168
PAL = [(255, 255, 255), (255, 247, 228), (13, 17, 38), (247, 138, 34), (232, 104, 28), (243, 72, 62),
       (139, 200, 30), (40, 120, 20), (196, 96, 20)]          # white, cream, ink, orange, deep orange, red, leaf, vein, stem

im = Image.open(src).convert("RGB")
px = im.load(); w0, h0 = im.size
ink = [(x, y) for x in range(0, w0, 3) for y in range(0, h0, 3) if min(px[x, y]) < 235]
box = (min(x for x, _ in ink), min(y for _, y in ink), max(x for x, _ in ink) + 3, max(y for _, y in ink) + 3)
im = im.crop(box)
H = round(W * im.size[1] / im.size[0])
pal = Image.new("P", (1, 1)); pal.putpalette(sum(PAL, ()) + (0, 0, 0) * (256 - len(PAL)))
big = im.resize((W * 4, H * 4), Image.LANCZOS).filter(ImageFilter.MedianFilter(5))
q = big.quantize(palette=pal, dither=Image.Dither.NONE).resize((W, H), Image.NEAREST).filter(ImageFilter.ModeFilter(3))
# (the palette is padded to 256 with black: a pixel nearest that padding takes the nearest real colour instead)
spare = q.getpalette()
near = lambda c: min(range(len(PAL)), key=lambda k: sum((a - b) ** 2 for a, b in zip(PAL[k], c)))
fix = {i: near(tuple(spare[3 * i:3 * i + 3])) for i in set(q.getdata()) if i >= len(PAL)}
idx = [fix.get(i, i) for i in q.getdata()]
assert max(idx) < len(PAL)

data, rows8 = bytearray(), []
for y in range(H):
    if y % 8 == 0: rows8.append(len(data))
    row = idx[y * W:(y + 1) * W]; x = 0
    while x < W:
        c = row[x]; n = 1
        while x + n < W and row[x + n] == c and n < 16 + 255: n += 1
        if n <= 15: data.append((n - 1) << 4 | c)
        else: data += bytes([0xF0 | c, n - 16])
        x += n
rgb565 = lambda r, g, b: (r & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3
with open(out, "w") as f:
    f.write("/* SPDX-License-Identifier: GPL-3.0-only\n * Teachie Peachie's logo, written by tools/mklogo.py from assets/logo/teachie-peachie.jpg: do not edit. */\n")
    f.write("#define TP_LOGO_W %d\n#define TP_LOGO_H %d\n" % (W, H))
    f.write("static const uint16_t tp_logo_pal[%d] = { %s };\n" % (len(PAL), ", ".join("0x%04X" % rgb565(*c) for c in PAL)))
    f.write("static const uint16_t tp_logo_row8[%d] = { %s };\n" % (len(rows8), ", ".join(map(str, rows8))))
    f.write("static const uint8_t tp_logo_rle[%d] = {\n" % len(data))
    for i in range(0, len(data), 24): f.write("    " + ", ".join("%d" % b for b in data[i:i + 24]) + ",\n")
    f.write("};\n")
q.convert("RGB").resize((W * 2, H * 2), Image.NEAREST).save(out.rsplit(".", 1)[0] + "-preview.png")
print("%s: %dx%d, %d colours, %d bytes of runs, %d row marks" % (out, W, H, len(PAL), len(data), len(rows8)), file=sys.stderr)
