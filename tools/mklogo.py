#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Peachie: the start screen's logo for app/vis.c. The picture (the logo on a plain background) is cut to the
# sticker, made flat (a few colours, no gradients) and shrunk, then written as runs of one colour, row by row (no run
# crosses a row's end):
#   a byte: high nibble n, low nibble the colour; n < 15: n + 1 pixels; n = 15: 16 + the next byte pixels.
# Colour 0 is the background round the sticker: not drawn, so the start screen's own colour shows there.
# Every 8th row's start is kept too, so a tile of the screen only reads its own rows.
#   mklogo.py PICTURE OUT.h [WIDTH]     (needs Pillow)
import sys
from PIL import Image, ImageDraw, ImageFilter

src, out = sys.argv[1], sys.argv[2]
W = int(sys.argv[3]) if len(sys.argv) > 3 else 168
im = Image.open(src).convert("RGB")
px = im.load(); w0, h0 = im.size
BG = px[4, 4]                                                 # the picture's background, from its corner
PAL = [BG, (255, 255, 255), (0, 0, 0), (251, 146, 80), (251, 92, 86), (161, 219, 49), (64, 157, 4), (181, 1, 2)]
# the background, white, the outline, the peach's orange side, its red side, the leaf, the leaf's shade, the stem
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
q.putdata(idx)

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
    f.write("/* SPDX-License-Identifier: GPL-3.0-only\n * Teachie Peachie's logo, written by tools/mklogo.py from assets/logo/teachie-peachie.jpg: do not edit.\n * (colour 0 is the background round it: not drawn) */\n")
    f.write("#define TP_LOGO_W %d\n#define TP_LOGO_H %d\n" % (W, H))
    f.write("static const uint16_t tp_logo_pal[%d] = { %s };\n" % (len(PAL), ", ".join("0x%04X" % rgb565(*c) for c in PAL)))
    f.write("static const uint16_t tp_logo_row8[%d] = { %s };\n" % (len(rows8), ", ".join(map(str, rows8))))
    f.write("static const uint8_t tp_logo_rle[%d] = {\n" % len(data))
    for i in range(0, len(data), 24): f.write("    " + ", ".join("%d" % b for b in data[i:i + 24]) + ",\n")
    f.write("};\n")
pv = q.copy(); pv.putpalette(sum(PAL, ())); pv.convert("RGB").resize((W * 2, H * 2), Image.NEAREST).save(out.rsplit(".", 1)[0] + "-preview.png")
print("%s: %dx%d, %d colours, %d bytes of runs, %d row marks" % (out, W, H, len(PAL), len(data), len(rows8)), file=sys.stderr)
