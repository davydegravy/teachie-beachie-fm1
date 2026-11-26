#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Teachie Beachie: the lessons' own fonts, with small letters, for the lesson pack.
#   python3 tools/mklessonfont.py "assets/fonts/google-sans/GoogleSans[GRAD,opsz,wght].ttf" assets/fonts/lessons-font.bin
# Google Sans SemiBold (SIL Open Font License 1.1, (c) The Google Sans Project Authors; assets/fonts/google-sans/OFL.txt),
# rendered into anti-aliased bitmaps exactly as tools/mkfont.py does for SLOOP's own fonts: a small size (cards and
# strips, 3 bits a pixel) and a large one (the big cards, 2 bits a pixel), each with every ASCII letter from the space
# to ~, so that a letter's place is its code less 32 (app/gfx.c). Run by hand when the typeface changes (it needs
# Pillow); the result is kept in the repository, and tools/mkcourse.py puts it in the pack without Pillow.
#
#
# Pictures (2026-11-25, the move to SLOOP 3.4.2): a few characters the lessons' words never use hold a small picture
# in place of their letter, drawn here at each size, as high as a capital, in the letters' own ink. A lesson file
# names one by a word (tutorial/tp/lesson.py, words()), and its width is measured as any letter's is.
#   @   a black key (SLOOP 3.4.2's drum grid picks a drum with one): [BLACK 2] in a lesson file is drawn as this
#       picture and 2 in a frame, the second black key from the left
#
# The file: "TPF1", then for each font a u16 (where its block starts), then the blocks. A block: n, asc, height, cap,
# cap_top, bpp (one byte each), n glyphs of 8 bytes (w, h, x, y, adv, 0, u16 off: app/gfx.h's gfx_glyph), the bits.
import struct, sys
from PIL import Image, ImageDraw, ImageFont

FONT, OUT = sys.argv[1], sys.argv[2]
WEIGHT = int(sys.argv[3]) if len(sys.argv) > 3 else 600   # SemiBold (a third word on the command line: another weight, 400 to 700)
CHARS = [chr(c) for c in range(32, 127)]
SIZES = [("small", 18, 3), ("large", 34, 2)]           # the same letter heights as SLOOP's font_s and font_l (13 and 25)

SS = 8                                                 # a picture is drawn at 8 times its size, then shrunk: smooth edges
def black_key(cap):
    """the black key's picture, as high as a capital: two white keys in outline and the black key between them, filled
    (chosen 2026-11-25 over the key alone, a bar: "its easier to see that they are keys"). (its image, how far the
    pen moves on)"""
    w = round(cap * 0.85) | 1; t = max(1, round(cap / 13)); im = Image.new("L", (w * SS, cap * SS), 0); d = ImageDraw.Draw(im)
    bw = round(cap * 0.38) | 1; bh = round(cap * 0.62); x0 = (w - bw) // 2
    d.rounded_rectangle((0, 0, w * SS - 1, cap * SS - 1), radius=round(cap * SS * 0.14), outline=255, width=t * SS)
    d.rectangle(((w // 2 - t // 2) * SS, bh * SS, (w // 2 - t // 2 + t) * SS - 1, cap * SS - 1), fill=255)          # the line between the two white keys
    d.rounded_rectangle((x0 * SS, 0, (x0 + bw) * SS - 1, bh * SS - 1), radius=round(cap * SS * 0.07), fill=255)      # the black key
    d.rectangle((x0 * SS, 0, (x0 + bw) * SS - 1, t * SS), fill=255)                                                  # (square at the top edge)
    return im.resize((w, cap), Image.BOX), w + max(2, round(cap * 0.16))
ICONS = {"@": black_key}

def build(px, bpp):
    f = ImageFont.truetype(FONT, px); f.set_variation_by_axes([18, WEIGHT, 0])   # (the axes: optical size, weight, grade)
    asc, desc = f.getmetrics(); glyphs, bits = [], bytearray()
    hi = Image.new("L", (px * 3, px * 3), 0); ImageDraw.Draw(hi).text((px, px), "H", font=f, fill=255); hb = hi.getbbox()   # a capital: its top, its height
    for ch in CHARS:
        if ch in ICONS:                                # a picture in place of the letter, on the capitals' line
            pic, adv = ICONS[ch](hb[3] - hb[1]); im = Image.new("L", (px * 3, px * 3), 0); im.paste(pic, (px, hb[1]))
        else:
            adv = round(f.getlength(ch))
            im = Image.new("L", (px * 3, px * 3), 0); ImageDraw.Draw(im).text((px, px), ch, font=f, fill=255)
        bb = im.getbbox()
        if not bb: glyphs.append((0, 0, 0, 0, adv, len(bits))); continue
        x0, y0, x1, y1 = bb; off = len(bits); acc = n = 0
        for y in range(y0, y1):
            for x in range(x0, x1):                    # (MSB first, a glyph from a byte's start)
                acc = acc << bpp | (im.getpixel((x, y)) * ((1 << bpp) - 1) + 127) // 255; n += bpp
                while n >= 8: n -= 8; bits.append(acc >> n & 255); acc &= (1 << n) - 1
        if n: bits.append(acc << (8 - n) & 255)
        glyphs.append((x1 - x0, y1 - y0, x0 - px, y0 - px, adv, off))   # offsets from the pen (top of the em box)
    bits += b"\0\0"                                    # (gfx.c reads two bytes at a time)
    if len(bits) & 1: bits.append(0)
    if len(bits) > 0xFFFF: sys.exit("the bits are more than a glyph's offset can say")
    H = glyphs[ord("H") - 32]
    head = bytes([len(glyphs), asc, asc + desc, H[1], H[3], bpp])
    return head + b"".join(struct.pack("<BBbbBxH", *g) for g in glyphs) + bytes(bits), (asc, desc, H[1], glyphs)

blocks = []
for name, px, bpp in SIZES:
    b, (asc, desc, cap, glyphs) = build(px, bpp); blocks.append(b)
    print("%s: %d px, capitals %d high, %d bytes; widths: space %d, n %d, M %d" % (name, px, cap, len(b), glyphs[0][4], glyphs[ord("n") - 32][4], glyphs[ord("M") - 32][4]))
at = 4 + 2 * len(blocks); offs = []
for b in blocks: offs.append(at); at += len(b)
open(OUT, "wb").write(b"TPF1" + b"".join(struct.pack("<H", o) for o in offs) + b"".join(blocks))
print("%s: %d bytes" % (OUT, at))
