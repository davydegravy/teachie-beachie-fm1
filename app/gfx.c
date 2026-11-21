/* SPDX-License-Identifier: GPL-3.0-only
 * Drawing into one tile (gfx.h). Written for this project. */
#include "gfx.h"

static uint16_t *T;
static int TX, TY, TW, TH;

void gfx_target(uint16_t *px, int x, int y, int w, int h) { T = px; TX = x; TY = y; TW = w; TH = h; }
void gfx_tile(int *x, int *y, int *w, int *h) { *x = TX; *y = TY; *w = TW; *h = TH; }
int gfx_visible(int x, int y, int w, int h) { return x < TX + TW && x + w > TX && y < TY + TH && y + h > TY; }

uint16_t gfx_mix(uint16_t bg, uint16_t fg, unsigned a)
{
    uint32_t b = (bg | (uint32_t)bg << 16) & 0x07E0F81Fu, f = (fg | (uint32_t)fg << 16) & 0x07E0F81Fu;
    uint32_t r = ((((f - b) * a) >> 5) + b) & 0x07E0F81Fu;
    return (uint16_t)(r | r >> 16);
}
static inline void blend(int x, int y, uint16_t c, unsigned a)      /* a 0..32, (x, y) on the screen */
{
    if (x < TX || x >= TX + TW || y < TY || y >= TY + TH || !a) return;
    uint16_t *p = &T[(y - TY) * TW + (x - TX)];
    *p = a >= 32 ? c : gfx_mix(*p, c, a);
}
void gfx_fill(int x, int y, int w, int h, uint16_t c)
{
    int x0 = x > TX ? x : TX, y0 = y > TY ? y : TY;
    int x1 = x + w < TX + TW ? x + w : TX + TW, y1 = y + h < TY + TH ? y + h : TY + TH;
    for (int yy = y0; yy < y1; yy++) {
        uint16_t *p = &T[(yy - TY) * TW + (x0 - TX)];
        for (int xx = x0; xx < x1; xx++) *p++ = c;
    }
}
void gfx_frame(int x, int y, int w, int h, int t, uint16_t c)
{
    gfx_fill(x, y, w, t, c); gfx_fill(x, y + h - t, w, t, c);
    gfx_fill(x, y + t, t, h - 2 * t, c); gfx_fill(x + w - t, y + t, t, h - 2 * t, c);
}
/* coverage (0..32) of the pixel (px, py) by a disc of radius r centred at (cx, cy), all in 1/4 px: 4 x 4 samples */
static unsigned disc_cov(int px4, int py4, int cx4, int cy4, int r4)
{
    unsigned n = 0;
    for (int j = 0; j < 4; j++) for (int i = 0; i < 4; i++) {
        int dx = px4 + i * 2 + 1 - cx4 * 2, dy = py4 + j * 2 + 1 - cy4 * 2;     /* in 1/8 px */
        if (dx * dx + dy * dy <= 4 * r4 * r4) n++;
    }
    return n * 2;
}
/* rounded corners: a quarter disc's coverage, worked out once for each radius (1..8) and mirrored */
#define RMAX 8
static uint8_t COV[RMAX + 1][RMAX][RMAX], cov_done[RMAX + 1];
static const uint8_t *cov(int r)                         /* [y][x] of the top left corner, 0..32 */
{
    if (r < 1) r = 1;
    if (r > RMAX) r = RMAX;
    if (!cov_done[r]) {
        for (int y = 0; y < r; y++) for (int x = 0; x < r; x++) COV[r][y][x] = (uint8_t)disc_cov(x * 8, y * 8, r * 4, r * 4, r * 4);
        cov_done[r] = 1;
    }
    return &COV[r][0][0];
}
/* the corners of the box: coverage by radius r, less that of radius r2 (0: none) */
static void corners(int x, int y, int w, int h, int r, int r2, uint16_t c)
{
    const uint8_t *a = cov(r), *b = r2 > 0 ? cov(r2) : 0;
    int d = r - r2;                                      /* the inner disc's corner is d px in */
    for (int k = 0; k < 4; k++) {
        int ox = k & 1 ? x + w - r : x, oy = k & 2 ? y + h - r : y;
        if (!gfx_visible(ox, oy, r, r)) continue;
        for (int j = 0; j < r; j++) for (int i = 0; i < r; i++) {
            int ci = k & 1 ? r - 1 - i : i, cj = k & 2 ? r - 1 - j : j;   /* mirrored to the top left */
            int v = a[cj * RMAX + ci];
            if (b && ci >= d && cj >= d) v -= b[(cj - d) * RMAX + (ci - d)];
            if (v > 0) blend(ox + i, oy + j, c, (unsigned)v);
        }
    }
}
void gfx_rfill(int x, int y, int w, int h, int r, uint16_t c)
{
    if (!gfx_visible(x, y, w, h)) return;
    if (r > RMAX) r = RMAX;
    if (r < 1) { gfx_fill(x, y, w, h, c); return; }
    gfx_fill(x + r, y, w - 2 * r, h, c);
    gfx_fill(x, y + r, r, h - 2 * r, c); gfx_fill(x + w - r, y + r, r, h - 2 * r, c);
    corners(x, y, w, h, r, 0, c);
}
void gfx_rframe(int x, int y, int w, int h, int r, int t, uint16_t c)
{
    if (!gfx_visible(x, y, w, h)) return;
    if (r > RMAX) r = RMAX;
    if (r <= t) { gfx_frame(x, y, w, h, t, c); return; }
    gfx_fill(x + r, y, w - 2 * r, t, c); gfx_fill(x + r, y + h - t, w - 2 * r, t, c);
    gfx_fill(x, y + r, t, h - 2 * r, c); gfx_fill(x + w - t, y + r, t, h - 2 * r, c);
    corners(x, y, w, h, r, r - t, c);
}
void gfx_disc(int cx, int cy, int r, uint16_t c)
{
    if (!gfx_visible(cx - r, cy - r, 2 * r, 2 * r)) return;
    int y0 = cy - r > TY ? cy - r : TY, y1 = cy + r < TY + TH ? cy + r : TY + TH, x0 = cx - r > TX ? cx - r : TX, x1 = cx + r < TX + TW ? cx + r : TX + TW;
    for (int yy = y0; yy < y1; yy++) for (int xx = x0; xx < x1; xx++)
        blend(xx, yy, c, disc_cov(xx * 8, yy * 8, cx * 4, cy * 4, r * 4));
}
void gfx_fill_a(int x, int y, uint16_t c, unsigned a32) { blend(x, y, c, a32); }
/* a line, anti-aliased (Wu's way: each step two pixels shared by where the line passes); its ends in 1/16 px */
void gfx_line(int x0, int y0, int x1, int y1, uint16_t c)
{
    int lx = (x0 < x1 ? x0 : x1) >> 4, hx = (x0 > x1 ? x0 : x1) >> 4, ly = (y0 < y1 ? y0 : y1) >> 4, hy = (y0 > y1 ? y0 : y1) >> 4;
    if (!gfx_visible(lx - 1, ly - 1, hx - lx + 3, hy - ly + 3)) return;
    int dx = x1 - x0, dy = y1 - y0, steep = (dy < 0 ? -dy : dy) > (dx < 0 ? -dx : dx), t;
    if (steep) { t = x0; x0 = y0; y0 = t; t = x1; x1 = y1; y1 = t; }
    if (x0 > x1) { t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    dx = x1 - x0; dy = y1 - y0;
    int32_t g = dx ? (int32_t)((dy * 65536) / dx) : 0;                /* px a px, Q16 */
    int xs = x0 >> 4, xe = x1 >> 4;
    int32_t y = (y0 << 12) + ((xs * 16 + 8 - x0) * g >> 4) - 32768;   /* Q16, a pixel's centre up */
    for (int x = xs; x <= xe; x++, y += g) {
        int yi = y >> 16; unsigned f = (unsigned)(y >> 11) & 31;
        if (steep) { blend(yi, x, c, 32 - f); blend(yi + 1, x, c, f); } else { blend(x, yi, c, 32 - f); blend(x, yi + 1, c, f); }
    }
}
void gfx_play(int x, int y, int s, uint16_t c)
{
    if (!gfx_visible(x, y, s, s)) return;
    /* the triangle (0,0) (s,s/2) (0,s): a point is inside when its distance from the middle line is under
     * half the height left at its x; 4 x 4 samples a pixel */
    for (int yy = 0; yy < s; yy++) for (int xx = 0; xx < s; xx++) {
        unsigned n = 0;
        for (int j = 0; j < 4; j++) for (int i = 0; i < 4; i++) {
            int fx = xx * 8 + i * 2 + 1, fy = yy * 8 + j * 2 + 1, S8 = s * 8;   /* 1/8 px */
            int dy = 2 * fy - S8; if (dy < 0) dy = -dy;
            if (dy <= S8 - fx) n++;
        }
        blend(x + xx, y + yy, c, n * 2);
    }
}

/* a glyph's pixel i: bpp bits, one after the other across the bytes (MSB first) */
static unsigned px_of(const uint8_t *b, unsigned i, unsigned bpp, unsigned mask)
{
    unsigned q = i * bpp, w = (unsigned)b[q >> 3] << 8 | b[(q >> 3) + 1];
    return (w >> (16u - bpp - (q & 7u))) & mask;
}
/* ---------------------------------------------------------------- text */
static uint8_t FN;                                       /* how many glyphs the font being drawn has (Teachie Beachie's own have 95: all of ASCII, small letters too) */
static int glyph_of(const char **ps)
{
    const uint8_t *s = (const uint8_t *)*ps;
    int c = *s++;
    if (c == 0xC3 && *s) {                               /* the accented capitals (and their small letters) */
        int d = *s++ & ~0x20;
        c = d == 0x89 ? 64 : d == 0x88 ? 65 : d == 0x80 ? 66 : d == 0x87 ? 67 : d == 0x8A ? 68 : 3;
        *ps = (const char *)s; return c;
    }
    if (c == 0xE2 && s[0] == 0x86 && s[1] == 0x92) { *ps = (const char *)(s + 2); return 69; }   /* → */
    if (c == 0xC2 && s[0] == 0xB7) { *ps = (const char *)(s + 1); return 70; }                  /* · */
    if (c == 0xE2 && s[0] && s[1]) { *ps = (const char *)(s + 2); return '-' - 32; }           /* − – —: a dash */
    if (c >= 0x80) { while ((*s & 0xC0) == 0x80) s++; *ps = (const char *)s; return '?' - 32; }
    *ps = (const char *)s;
    if (FN > 77 && c >= 32 && c < 127) return c - 32;    /* (Teachie Beachie: a font with all of ASCII) */
    switch (c) {                                         /* the few small letters the font has (chord names) */
    case 'm': return 71; case 'b': return 72; case 'd': return 73; case 'i': return 74; case 's': return 75; case 'u': return 76;
    default: break;
    }
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c < 32 || c > 95) c = '?';
    return c - 32;
}
static int SP;                                           /* extra space between letters (fitting) */
static int CX = 100;                                     /* the letters' width, in % (fitting: condensed) */
static int cx(int v) { return CX == 100 ? v : (v * CX + 50) / 100; }
int gfx_text_w(const gfx_font *f, const char *s)
{
    int w = 0, gap = 0;
    FN = f->n;
    while (*s) { int k = glyph_of(&s); w += gap + cx(f->g[k].adv); gap = k == 0 && SP < 0 ? 0 : SP; }   /* (a space keeps its width: words stay apart when squeezed) */
    return w;
}
void gfx_spacing(int sp) { SP = sp; }
void gfx_condense(int pct) { CX = pct < 50 ? 50 : pct > 100 ? 100 : pct; }
int gfx_text(const gfx_font *f, int x, int y, const char *s, uint16_t c)
{
    int top = y - f->cap_top;                            /* the em box's top */
    if (!(top - 8 < TY + TH && top + f->height + 8 > TY)) return x + gfx_text_w(f, s);   /* (accents rise above) */
    const unsigned bpp = f->bpp, mask = (1u << bpp) - 1u, scale = (512u + mask / 2u) / mask;   /* to 0..32 (x 16) */
    FN = f->n;
    while (*s) {
        int k = glyph_of(&s);
        const gfx_glyph *g = &f->g[k];
        int gx = x + cx(g->x), gy = top + g->y, gw = cx(g->w);
        if (g->w && gfx_visible(gx, gy, gw, g->h)) {
            const uint8_t *b = f->bits + g->off;
            if (CX == 100) {
                unsigned i = 0;
                for (int yy = 0; yy < g->h; yy++) for (int xx = 0; xx < g->w; xx++, i++) {
                    unsigned v = px_of(b, i, bpp, mask);
                    if (v) blend(gx + xx, gy + yy, c, (v * scale + 8) >> 4);
                }
            } else for (int yy = 0; yy < g->h; yy++) for (int dx = 0; dx < gw; dx++) {   /* (condensed: two columns read, averaged) */
                int s0 = dx * 100 / CX, s1 = ((dx + 1) * 100 - 1) / CX; if (s1 >= g->w) s1 = g->w - 1;
                unsigned i0 = (unsigned)(yy * g->w + s0), i1 = (unsigned)(yy * g->w + s1);
                unsigned v0 = px_of(b, i0, bpp, mask), v1 = px_of(b, i1, bpp, mask);
                unsigned v = v0 > v1 ? v0 : v1;
                if (v) blend(gx + dx, gy + yy, c, (v * scale + 8) >> 4);
            }
        }
        x += cx(g->adv) + (k == 0 && SP < 0 ? 0 : SP);
    }
    return x - SP;
}
void gfx_text_c(const gfx_font *f, int cx, int y, const char *s, uint16_t c) { gfx_text(f, cx - gfx_text_w(f, s) / 2, y, s, c); }
void gfx_text_r(const gfx_font *f, int rx, int y, const char *s, uint16_t c) { gfx_text(f, rx - gfx_text_w(f, s), y, s, c); }

uint32_t gfx_finish(uint16_t *px, int n)
{
    uint32_t h = 0x811C9DC5u, *w = (uint32_t *)px;
    for (int i = 0; i < n / 2; i++) {
        uint32_t v = w[i];
        v = ((v & 0x00FF00FFu) << 8) | ((v >> 8) & 0x00FF00FFu);     /* each pixel's high byte first */
        w[i] = v;
        h = (h ^ v) * 0x01000193u;
    }
    return h;
}
