/* SPDX-License-Identifier: GPL-3.0-only
 * The visualiser (vis.h): six full-screen pictures of what SLOOP 3 plays (3.3.6: Sam's six of the twelve). vis_frame works a picture out once
 * (the wave caught on a rising edge, its spectrum, the meters, what moves); vis_draw draws it into each tile.
 * Written for this project. */
#include "vis.h"
#include "gfx.h"
#include "ui.h"
#include "../engine/engine.h"

const char *const vis_name[VIS_N] = { "OSCILLOSCOPE", "SLOOP", "WIRES", "VU METERS", "SPECTRUM", "LCD" };
enum { VS_SCOPE, VS_SLOOP, VS_WIRES, VS_VU, VS_SPEC, VS_LCD };
/* (3.3.6) a style kept by 3.0 to 3.3.5 (twelve): its place now, the oscilloscope for the six taken out */
const uint8_t vis_from12[12] = { VS_SCOPE, VS_SPEC, VS_SCOPE, VS_SCOPE, VS_VU, VS_SCOPE, VS_SCOPE, VS_LCD, VS_SCOPE, VS_SCOPE, VS_WIRES, VS_SLOOP };
#define NB 40                               /* the spectrum's bands: 40 Hz .. 10 kHz, by octave thirds or so */
#define MAGIC 0x56495333u
static const uint8_t EDGE[NB + 1] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 26,
                                      29, 34, 39, 44, 51, 58, 67, 77, 88, 101, 116, 134, 153, 176, 202, 232 };

typedef struct {
    uint32_t magic, t;
    uint8_t style, kick, hit[5], fl[5];
    int16_t re[512], im[512];
    int8_t l[512], r[512];                  /* the ring, oldest first */
    uint8_t band[NB];
    uint8_t sc[240];                        /* the scope's rows */
    int16_t cap[NB], capv[NB], vu[7], pk[7], pkt[7];
    int32_t scale;
    int16_t wa[5]; uint8_t wn[5], lv[5], mix;
} vis_mem;
_Static_assert(sizeof(vis_mem) <= VIS_MEM, "the visualiser's memory");
static vis_mem *M;
static vis_in I;

/* sin of a (1024 a turn), Q15: the engine's table (engine/common.c, filled at the start) */
#define isin(a) ((int)gb_sine[(a) & 1023])
#define icos(a) isin((a) + 256)
static int db8f(float a)                    /* an amplitude -> 0..255 over 54 dB */
{
    if (!(a > 0.002f)) return 0;
    union { float f; uint32_t u; } x = { a };
    int lg16 = (int)((x.u >> 23) & 255) * 16 - 127 * 16 + (int)((x.u >> 19) & 15);   /* log2 x 16 */
    int v = 255 + lg16 * 255 / (9 * 16);    /* (6 dB a log2: 54 dB is 9) */
    return v < 0 ? 0 : v > 255 ? 255 : v;
}
static int lg16u(uint32_t p)                /* log2 x 16 of a power */
{
    if (!p) return 0;
    int b = 31 - __builtin_clz(p);
    return b * 16 + (int)((b >= 4 ? p >> (b - 4) : p << (4 - b)) & 15);
}
/* colours */
#define BG (ui_pal.bg)
#define TXT (ui_pal.txt)
#define DIM (ui_pal.dim)
static uint16_t ramp(const uint16_t *st, int n, int v)   /* v 0..255 along n stops */
{
    int s = v * (n - 1), i = s >> 8; if (i >= n - 1) return st[n - 1];
    return gfx_mix(st[i], st[i + 1], (unsigned)((s & 255) >> 3));
}
static uint16_t hot(int v) { const uint16_t st[5] = { I.col[0], I.col[1], I.col[2], I.col[4], RGB(255, 70, 70) }; return ramp(st, 5, v); }
static int amp(void) { int lv = 0; for (int i = 0; i < 5; i++) if (M->lv[i] > lv) lv = M->lv[i]; return lv > M->mix ? lv : M->mix; }

/* ---------------------------------------------------------------- once a picture */
static void fft(void)                       /* 512 points, in place, scaled by 1/512 */
{
    int16_t *re = M->re, *im = M->im;
    for (int i = 1, j = 0; i < 512; i++) {
        int b = 256; for (; j & b; b >>= 1) j ^= b; j |= b;
        if (i < j) { int16_t t = re[i]; re[i] = re[j]; re[j] = t; t = im[i]; im[i] = im[j]; im[j] = t; }
    }
    for (int len = 2; len <= 512; len <<= 1) {
        int h = len >> 1, st = 1024 / len;
        for (int k = 0; k < h; k++) {
            int wr = icos(k * st), wi = -isin(k * st);
            for (int i = k; i < 512; i += len) {
                int j = i + h, tr = (re[j] * wr - im[j] * wi) >> 15, ti = (re[j] * wi + im[j] * wr) >> 15;
                re[j] = (int16_t)((re[i] - tr) >> 1); im[j] = (int16_t)((im[i] - ti) >> 1);
                re[i] = (int16_t)((re[i] + tr) >> 1); im[i] = (int16_t)((im[i] + ti) >> 1);
            }
        }
    }
}
static void reset(void)
{
    uint8_t *p = (uint8_t *)M; for (uint32_t i = 0; i < sizeof *M; i++) p[i] = 0;
    M->magic = MAGIC; M->style = I.style; M->scale = 40;
    for (int i = 0; i < 5; i++) M->hit[i] = I.hit[i];
    M->kick = I.kick;
}
void vis_frame(uint8_t *mem, const vis_in *in)
{
    M = (vis_mem *)mem; I = *in;
    if (M->magic != MAGIC || M->style != I.style) reset();
    vis_mem *m = M; m->t++;
    for (int i = 0; i < VIS_RING; i++) { int k = (I.w + i) & (VIS_RING - 1); m->l[i] = I.ring[k]; m->r[i] = I.ring[VIS_RING + k]; }
    int mx = 0; for (int i = 0; i < VIS_RING; i++) { int a = m->l[i] < 0 ? -m->l[i] : m->l[i], b = m->r[i] < 0 ? -m->r[i] : m->r[i]; if (a > mx) mx = a; if (b > mx) mx = b; }
    m->mix = (uint8_t)db8f((float)mx * (1.0f / 127.0f));
    for (int i = 0; i < 5; i++) {
        m->lv[i] = (uint8_t)db8f(I.lvl[i]);
        if (I.hit[i] != m->hit[i]) { m->hit[i] = I.hit[i]; m->fl[i] = 255; m->wa[i] = (int16_t)(400 + I.vel[i] * 5); m->wn[i] = (uint8_t)(1 + I.note[i] % 4); }
        else m->fl[i] = (uint8_t)(m->fl[i] * 7 / 8);
    }
    m->kick = I.kick;
    switch (I.style) {
    case VS_SCOPE: {                        /* the wave, still: caught on a rising edge */
        int st = 0;
        for (int i = 1; i < VIS_RING - 240; i++) if (m->l[i - 1] + m->r[i - 1] < 0 && m->l[i] + m->r[i] >= 0) { st = i; if (i > 40) break; }
        int want = mx < 3 ? 40 : mx * 2; m->scale += (want - m->scale) / 4; if (m->scale < 8) m->scale = 8;
        for (int x = 0; x < 240; x++) { int v = 120 - (m->l[st + x] + m->r[st + x]) * 92 / m->scale; m->sc[x] = (uint8_t)(v < 2 ? 2 : v > 237 ? 237 : v); }
        break; }
    case VS_SPEC: {                         /* the spectrum */
        for (int i = 0; i < 512; i++) { int w = (32767 - icos(i * 2)) >> 1; m->re[i] = (int16_t)(((m->l[i] + m->r[i]) * 128 * w) >> 15); m->im[i] = 0; }
        fft();
        for (int b = 0; b < NB; b++) {
            uint32_t p = 0; for (int k = EDGE[b]; k < EDGE[b + 1]; k++) { uint32_t q = (uint32_t)(m->re[k] * m->re[k] + m->im[k] * m->im[k]); if (q > p) p = q; }
            int v = (lg16u(p) - 212) * 255 / 200 + b * 3 / 2; m->band[b] = (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v);
            if (m->band[b] >= m->cap[b]) { m->cap[b] = m->band[b]; m->capv[b] = 0; } else { m->capv[b] += 2; m->cap[b] -= m->capv[b]; if (m->cap[b] < 0) m->cap[b] = 0; }
        }
        break; }
    case VS_VU: {                           /* the meters: each track, the mix's two sides */
        int l = 0, r = 0; for (int i = 0; i < VIS_RING; i++) { int a = m->l[i] < 0 ? -m->l[i] : m->l[i], b = m->r[i] < 0 ? -m->r[i] : m->r[i]; if (a > l) l = a; if (b > r) r = b; }
        int v[7] = { m->lv[0], m->lv[1], m->lv[2], m->lv[3], m->lv[4], db8f((float)l / 127.0f), db8f((float)r / 127.0f) };
        for (int i = 0; i < 7; i++) {
            m->vu[i] = (int16_t)(v[i] > m->vu[i] ? v[i] : m->vu[i] - 9 > v[i] ? m->vu[i] - 9 : v[i]);
            if (v[i] >= m->pk[i]) { m->pk[i] = (int16_t)v[i]; m->pkt[i] = 40; } else if (m->pkt[i]) m->pkt[i]--; else if (m->pk[i] > 4) m->pk[i] -= 4; else m->pk[i] = 0;
        }
        break; }
    case VS_WIRES:                          /* WIRES: each string dies away */
        for (int i = 0; i < 5; i++) m->wa[i] = (int16_t)(m->wa[i] * 29 / 32);
        break;
    default: break;
    }
}

/* ---------------------------------------------------------------- the six pictures */
static void seg7(int x, int y, int w, int h, int d, uint16_t on, uint16_t off)   /* a digit (d < 0: blank, 10: '-') */
{
    static const uint8_t S[11] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x40 };
    int m = d >= 0 && d <= 10 ? S[d] : 0, t = w / 5 > 2 ? w / 5 : 2, hh = h / 2;
    const int R[7][4] = { { t, 0, w - 2 * t, t }, { w - t, t, t, hh - t }, { w - t, hh, t, hh - t }, { t, h - t, w - 2 * t, t },
                          { 0, hh, t, hh - t }, { 0, t, t, hh - t }, { t, hh - t / 2, w - 2 * t, t } };
    for (int s = 0; s < 7; s++) gfx_rfill(x + R[s][0], y + R[s][1], R[s][2], R[s][3], t / 2, (m >> s) & 1 ? on : off);
}
static void scope(void)
{
    for (int y = 30; y < 240; y += 30) gfx_fill(0, y, 240, 1, ui_pal.panel);
    gfx_fill(0, 120, 240, 1, ui_pal.line);
    uint16_t c = hot(amp()), g = gfx_mix(BG, c, 9);
    for (int x = 1; x < 240; x++) { gfx_line((x - 1) * 16, M->sc[x - 1] * 16 + 24, x * 16, M->sc[x] * 16 + 24, g); gfx_line((x - 1) * 16, M->sc[x - 1] * 16 - 24, x * 16, M->sc[x] * 16 - 24, g); }
    for (int x = 1; x < 240; x++) { gfx_line((x - 1) * 16, M->sc[x - 1] * 16, x * 16, M->sc[x] * 16, c); gfx_line((x - 1) * 16, M->sc[x - 1] * 16 + 10, x * 16, M->sc[x] * 16 + 10, c); }
}
static void spectrum(void)
{
    for (int b = 0; b < NB; b++) {
        int x = b * 6, h = M->band[b] * 200 / 255, ct = M->cap[b] * 200 / 255;
        if (!gfx_visible(x, 30, 6, 210)) continue;
        for (int y = 0; y < h; y += 4) gfx_fill(x, 236 - y - 3, 5, 3, hot(y * 255 / 200));
        gfx_fill(x, 236 - ct - 2, 5, 2, TXT);
    }
}
static void vu(void)
{
    static const char *const N[7] = { "1", "2", "3", "4", "D", "L", "R" };
    for (int i = 0; i < 7; i++) {
        int x = 6 + i * 29 + (i >= 5 ? 6 : 0), w = 22; uint16_t tc = i < 5 ? I.col[i] : TXT;
        if (!gfx_visible(x, 30, w, 210)) continue;
        int lit = M->vu[i] * 36 / 255, pk = M->pk[i] * 36 / 255;
        for (int s = 0; s < 36; s++) {
            uint16_t c = s >= 33 ? RGB(255, 60, 60) : s >= 27 ? RGB(255, 210, 60) : tc;
            gfx_rfill(x, 212 - s * 5, w, 4, 1, s < lit ? c : s == pk && pk ? TXT : gfx_mix(BG, c, 4));
        }
        gfx_text_c(&font_s, x + w / 2, 222, N[i], tc);
    }
    static const char *const D[4] = { "0", "-6", "-12", "-24" }; static const uint8_t S[4] = { 36, 32, 28, 20 };
    for (int i = 0; i < 4; i++) gfx_text_r(&font_s, 238, 212 - S[i] * 5 + 2, D[i], DIM);
}
static void lcd(void)
{
    uint16_t on = gfx_mix(I.col[1], TXT, 8), off = gfx_mix(BG, on, 3);
    gfx_rfill(6, 30, 228, 204, 8, gfx_mix(BG, on, 2));
    int v = I.bpm10; int d[4] = { v / 1000 % 10, v / 100 % 10, v / 10 % 10, v % 10 };
    for (int i = 0; i < 4; i++) seg7(18 + i * 52 + (i == 3 ? 6 : 0), 44, 40, 74, i == 0 && !d[0] ? -1 : d[i], on, off);
    gfx_rfill(172, 112, 6, 6, 2, on);
    gfx_text(&font_s, 18, 124, "BPM", on);
    int bar = I.bar + 1; int bd[3] = { bar / 100 % 10, bar / 10 % 10, bar % 10 };
    for (int i = 0; i < 3; i++) seg7(120 + i * 22, 126, 16, 28, (i < 2 && bar < (i ? 10 : 100)) ? -1 : bd[i], on, off);
    seg7(192, 126, 16, 28, (I.beat & 3) + 1, on, off); gfx_rfill(186, 150, 4, 4, 1, on);
    for (int t = 0; t < 5; t++) { int y = 166 + t * 13, lit = M->lv[t] * 24 / 255;
        gfx_rfill(16, y, 8, 8, 2, I.col[t]);
        for (int s = 0; s < 24; s++) gfx_fill(30 + s * 8, y, 6, 8, s < lit ? (s >= 21 ? RGB(255, 80, 60) : on) : off); }
}
static void wires(void)
{
    int ph = (int)(M->t * 90);
    for (int i = 0; i < 5; i++) {
        int y0 = 52 + i * 40, a = M->wa[i], n = M->wn[i] ? M->wn[i] : 1, sw = a * icos(ph * (n + 1) / 2) >> 15;
        gfx_rfill(4, y0 - 4, 8, 8, 2, DIM); gfx_rfill(228, y0 - 4, 8, 8, 2, DIM);
        int px = 12 * 16, py = y0 * 16;
        for (int s = 1; s <= 32; s++) { int x = 12 * 16 + s * 216 * 16 / 32, y = y0 * 16 + (sw * isin(s * 512 * n / 32) >> 15) * 30 / 1000 * 16;
            gfx_line(px, py, x, y, I.col[i]); gfx_line(px, py + 16, x, y + 16, gfx_mix(BG, I.col[i], 12)); px = x; py = y; }
    }
}
/* ---------------------------------------------------------------- the SLOOP logo (Sam's: assets/logo of SLOOP), drawn
 * in its style (flat shapes, white strokes, a dial round them) as a truer sloop (3.1.6, Sam's choice): a mainsail of
 * five curved bands for SLOOP 3's five tracks in their colours, a curved jib, a boom, a hull with a pointed bow; the word "sloop" from the long logo's strokes. Anti-aliased by the
 * distance to each shape's edge (no square root: the distance squared, straightened near the edge). */
#define LW RGB(242, 242, 242)
typedef struct { float ox, oy, s; } xf;
static int clip(float x0, float y0, float x1, float y1, int *bx, int *by, int *ex, int *ey)
{
    int tx, ty, tw, th; gfx_tile(&tx, &ty, &tw, &th);       /* (only the tile's part: the work a tile asks, not the shape's) */
    if (!(x0 > -1e4f && x1 < 1e4f && y0 > -1e4f && y1 < 1e4f)) return 0;
    *bx = (int)x0 - 1; *by = (int)y0 - 1; *ex = (int)x1 + 2; *ey = (int)y1 + 2;
    if (*bx < tx) *bx = tx;
    if (*by < ty) *by = ty;
    if (*ex > tx + tw) *ex = tx + tw;
    if (*ey > ty + th) *ey = ty + th;
    return *bx < *ex && *by < *ey;
}
static void cov_px(int x, int y, float c, uint16_t col) { if (c > 0.0f) gfx_fill_a(x, y, col, c >= 1.0f ? 32u : (unsigned)(c * 32.0f)); }
/* a stroke along a segment, w wide, round ends */
static void capsule(float ax, float ay, float bx, float by, float w, uint16_t col)
{
    float R = w * 0.5f; int x0, y0, x1, y1;
    if (!clip((ax < bx ? ax : bx) - R, (ay < by ? ay : by) - R, (ax > bx ? ax : bx) + R, (ay > by ? ay : by) + R, &x0, &y0, &x1, &y1)) return;
    float dx = bx - ax, dy = by - ay, l2 = dx * dx + dy * dy, il = l2 > 0.0f ? 1.0f / l2 : 0.0f;
    for (int y = y0; y < y1; y++) for (int x = x0; x < x1; x++) {
        float px = (float)x + 0.5f - ax, py = (float)y + 0.5f - ay, t = (px * dx + py * dy) * il;
        t = t < 0.0f ? 0.0f : t > 1.0f ? 1.0f : t;
        float ex = px - t * dx, ey = py - t * dy, d2 = ex * ex + ey * ey;
        if (d2 < (R + 1.0f) * (R + 1.0f)) cov_px(x, y, R + 0.5f - (d2 + R * R) / (2.0f * R), col);
    }
}
/* a stroke along a circle from angle a (1024 a turn, clockwise on the screen from 3 o'clock) for len, round ends */
static void arc(float cx, float cy, float r, float w, int a, int len, uint16_t col)
{
    if (len <= 0) return;
    float R = w * 0.5f, ro = r + R, ri = r - R; int x0, y0, x1, y1;
    float ax = (float)icos(a) * (1.0f / 32767.0f), ay = (float)isin(a) * (1.0f / 32767.0f), bx = (float)icos(a + len) * (1.0f / 32767.0f), by = (float)isin(a + len) * (1.0f / 32767.0f);
    if (clip(cx - ro, cy - ro, cx + ro, cy + ro, &x0, &y0, &x1, &y1))
        for (int y = y0; y < y1; y++) for (int x = x0; x < x1; x++) {
            float px = (float)x + 0.5f - cx, py = (float)y + 0.5f - cy, d2 = px * px + py * py;
            if (d2 > (ro + 1.0f) * (ro + 1.0f) || d2 < (ri - 1.0f) * (ri - 1.0f)) continue;
            float c1 = ax * py - ay * px, c2 = px * by - py * bx;          /* (inside the sweep: after a, before a + len) */
            int in = len <= 512 ? (c1 >= 0.0f && c2 >= 0.0f) : !(c1 < 0.0f && c2 < 0.0f);
            if (len >= 1024) in = 1;
            if (!in) continue;
            float co = ro + 0.5f - (d2 + ro * ro) / (2.0f * ro), ci = (d2 + ri * ri) / (2.0f * ri) - ri + 0.5f;
            cov_px(x, y, co < ci ? co : ci, col);
        }
    if (len < 1024) { capsule(cx + r * ax, cy + r * ay, cx + r * ax, cy + r * ay, w, col); capsule(cx + r * bx, cy + r * by, cx + r * bx, cy + r * by, w, col); }
}
/* the sloop (3.1.6, Sam's choice of four): in the logo's style (flat shapes, white strokes, the dial round them), a
 * mast; aft of it the mainsail in five bands (the tracks, top to bottom: 1 2 3 4 and the drums), its leech curved, on
 * its boom; forward a curved jib; the hull, its bow pointed, its keel curved. Curves: quadratic, in the icon's units */
#define MX 106.0f                                       /* the mast */
static float qat(float a, float b, float c, float t) { return (1.0f - t) * (1.0f - t) * a + 2.0f * (1.0f - t) * t * b + t * t * c; }
static float qt(float a, float b, float c, float y, int hi)   /* t where the curve's coordinate is y (the root asked) */
{
    float A = a - 2.0f * b + c, B = 2.0f * (b - a), C = a - y;
    if (A > -1e-3f && A < 1e-3f) return B != 0.0f ? -C / B : 0.0f;
    float D = B * B - 4.0f * A * C; if (D < 0.0f) return -1.0f;
    float r = gb_sqrtf(D), t0 = (-B - r) / (2.0f * A), t1 = (-B + r) / (2.0f * A);
    if (t0 > t1) { float x = t0; t0 = t1; t1 = x; }
    return hi ? t1 : (t0 >= 0.0f ? t0 : t1);
}
/* a part's row at y (icon units): its left and right; 0: none. 0..4 the bands, 5 the jib, 6 the hull */
static int edges(int id, float y, float reach, float *l, float *r)
{
    if (id < 5) { float t = qt(34.0f, 84.0f, 156.0f, y, 0); if (t < 0.0f) return 0; *l = MX + 6.0f; *r = *l + (qat(MX + 6.0f, 170.0f, 188.0f, t) - *l) * reach; return 1; }
    if (id == 5) { float t = qt(42.0f, 104.0f, 160.0f, y, 0); if (t < 0.0f || t > 1.0f) t = 1.0f; *l = qat(MX - 6.0f, 76.0f, 52.0f, t); *r = MX - 6.0f; *l = *r - (*r - *l) * reach; return 1; }
    float t0 = qt(174.0f, 206.0f, 184.0f, y, 0), t1 = qt(174.0f, 206.0f, 184.0f, y, 1);   /* the keel: (44,174) (112,206) (182,184) */
    if (t0 < 0.0f || t0 > 1.0f) return 0;
    float c = 119.0f, w = reach;
    *l = c + (qat(44.0f, 112.0f, 182.0f, t0) - c) * w;
    float xr = t1 >= 0.0f && t1 <= 1.0f ? qat(44.0f, 112.0f, 182.0f, t1) : 194.0f, tr = 194.0f - (y - 174.0f);   /* (the transom, raked) */
    *r = c + ((xr < tr ? tr : xr) - c) * w; if (y < 184.0f && tr < *r) *r = c + (tr - c) * w;
    return *r > *l;
}
static void part(const xf *T, int id, float y0, float y1, float reach, float shear, float dy, uint16_t col)
{
    float Y0 = T->oy + T->s * (y0 + dy), Y1 = T->oy + T->s * (y1 + dy);
    int bx, by, ex, ey;
    if (Y1 <= Y0 || !clip(0.0f, Y0, (float)GFX_W, Y1, &bx, &by, &ex, &ey)) return;
    if (by < (int)Y0) by = (int)Y0;
    for (int y = by; y < ey && y < (int)Y1 + 1; y++) {
        float cv = (float)y + 1.0f - Y0; if (cv > 1.0f) cv = 1.0f;
        float cb = Y1 - (float)y; if (cb < cv) cv = cb; if (cv <= 0.0f) continue;
        float iy = ((float)y + 0.5f - T->oy) / T->s - dy, l, r;
        iy = iy < y0 ? y0 : iy > y1 ? y1 : iy;
        if (!edges(id, iy, reach, &l, &r)) continue;
        float sh = shear * (150.0f - iy) * T->s, xl = T->ox + T->s * l + sh, xr = T->ox + T->s * r + sh;
        int x0 = xl > (float)bx ? (int)xl : bx, x1 = xr < (float)(ex - 1) ? (int)xr : ex - 1;
        for (int x = x0; x <= x1; x++) {
            float c = cv, a = (float)x + 1.0f - xl; if (a < c) c = a; a = xr - (float)x; if (a < c) c = a;
            cov_px(x, y, c, col);
        }
    }
}
/* the icon: the dial lit to `lit` (the rest `dim`), drawn to `dial`; the mast to `mast`, each band to its reach, the
 * jib to `jib`, the hull to `hull` (0..1: the start's animation); the boat leans by shear and rises by dy */
__attribute__((unused)) static void logo(const xf *T, float dial, float lit, uint16_t dim, float mast, const float *reach, const uint16_t *bc, float jib, float hull, float shear, float dy)
{
    float s = T->s, cx = T->ox + 120.0f * s, cy = T->oy + 120.0f * s;
    int len = (int)(768.0f * dial), on = (int)(768.0f * dial * lit);
    arc(cx, cy, 100.0f * s, 12.0f * s, 384, on, LW); arc(cx, cy, 100.0f * s, 12.0f * s, 384 + on, len - on, dim);
    xf B = { T->ox + s * 120.0f * 0.13f, T->oy + s * 112.0f * 0.13f, s * 0.87f };   /* (the boat a little smaller in the dial: Sam) */
    T = &B; s = B.s;
#define SX(x, y) (T->ox + ((x) + shear * (150.0f - (y))) * s)
#define SY(y) (T->oy + ((y) + dy) * s)
    if (hull > 0.0f) part(T, 6, 174.0f, 194.0f, 0.4f + 0.6f * hull, shear, dy, LW);
    if (jib > 0.01f) part(T, 5, 42.0f, 162.0f, jib, shear, dy, RGB(200, 202, 210));
    for (int k = 0; k < 5; k++) if (reach[k] > 0.01f) { float y0 = 34.0f + 25.2f * (float)k; part(T, k, y0, y0 + 21.2f, reach[k], shear, dy, bc[k]); }
    if (mast > 0.0f) { float yt = 172.0f - (172.0f - 28.0f) * mast; capsule(SX(MX, yt), SY(yt), SX(MX, 172.0f), SY(172.0f), 7.0f * s, LW); }
    if (mast > 0.99f) capsule(SX(MX, 162.0f), SY(162.0f), SX(190.0f, 162.0f), SY(162.0f), 5.0f * s, LW);   /* the boom */
#undef SX
#undef SY
}
/* "sloop" (the long logo's strokes, its units); reveal 0..1 letter by letter */
__attribute__((unused)) static void word(float ox, float oy, float s, float reveal, uint16_t col)   /* (Teachie Peachie: its name instead, below) */
{
#define P(v) (ox + (v) * s)
#define Q(v) (oy + (v) * s)
    float w = 12.0f * s, r = reveal * 5.0f;
    if (r > 0.0f) { int k = r >= 1.0f ? 245 * 1024 / 360 : (int)(r * 245.0f * 1024.0f / 360.0f);
        arc(P(290.88f), Q(115.0f), 15.0f * s, w, 256, k, col); arc(P(290.88f), Q(145.0f), 15.0f * s, w, 768, k, col); }
    if (r > 1.0f) capsule(P(337.76f), Q(160.0f), P(337.76f), Q(160.0f - 92.0f * (r - 1.0f > 1.0f ? 1.0f : r - 1.0f)), w, col);
    for (int i = 0; i < 3; i++) { float q = r - 2.0f - (float)i; if (q > 0.0f) arc(P(399.76f + 90.0f * (float)i), Q(130.0f), 30.0f * s, w, 768, (int)(1024.0f * (q > 1.0f ? 1.0f : q)), col); }
    if (r > 4.0f) capsule(P(549.76f), Q(130.0f), P(549.76f), Q(130.0f + 66.0f * (r - 4.0f > 1.0f ? 1.0f : r - 4.0f)), w, col);
#undef P
#undef Q
}
static void sloop(void)
{
    float t = (float)M->t, a = (float)amp() * (1.0f / 255.0f), sh = (float)isin((int)(t * 9.0f)) * (1.0f / 32767.0f) * (0.03f + 0.05f * a);
    float dy = (float)isin((int)(t * 13.0f)) * (1.0f / 32767.0f) * 3.0f, reach[5]; uint16_t bc[5];
    for (int k = 0; k < 5; k++) { reach[k] = 0.35f + 0.65f * (float)M->lv[k] * (1.0f / 255.0f); bc[k] = M->fl[k] > 60 ? gfx_mix(I.col[k], LW, (unsigned)(M->fl[k] >> 4)) : I.col[k]; }
    xf T = { 0.0f, 0.0f, 1.0f };
    for (int x = 40; x < 200; x += 4) { int y = 196 + (isin(x * 7 + (int)(t * 30.0f)) * (2 + (int)(a * 4.0f)) >> 15); gfx_fill(x, y, 4, 3, gfx_mix(BG, I.col[0], 14)); }
    logo(&T, 1.0f, (float)M->mix * (1.0f / 255.0f), gfx_mix(BG, LW, 6), 1.0f, reach, bc, 1.0f, 1.0f, sh, dy);
}
/* the start: the logo drawn in, the word "sloop" written, the version (vis_splash_draw for ms since the start) */
__attribute__((unused)) static uint32_t SPL_MS; __attribute__((unused)) static const char *SPL_VER; static uint16_t SPL_COL[5];   /* (Teachie Peachie: SPL_VER no longer shown) */
void vis_splash(uint32_t ms, const char *ver, const uint16_t *col) { SPL_MS = ms; SPL_VER = ver; for (int k = 0; k < 5; k++) SPL_COL[k] = col[k]; }
__attribute__((unused)) static float ease(float x) { x = x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x); }
__attribute__((unused)) static float back(float x) { x = x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; float c = 1.7f; return 1.0f + (c + 1.0f) * (x - 1.0f) * (x - 1.0f) * (x - 1.0f) + c * (x - 1.0f) * (x - 1.0f); }
#include "tp_logo.h"
/* Teachie Peachie: its logo from the runs of tp_logo.h (tools/mklogo.py), only the rows of the tile being drawn */
static void tp_logo_draw(int x0, int y0)
{
    int tx, ty, tw, th; gfx_tile(&tx, &ty, &tw, &th); (void)tx; (void)tw;
    int r0 = ty - y0, r1 = ty + th - y0;
    if (r1 <= 0 || r0 >= TP_LOGO_H) return;
    if (r0 < 0) r0 = 0;
    if (r1 > TP_LOGO_H) r1 = TP_LOGO_H;
    const uint8_t *p = tp_logo_rle + tp_logo_row8[r0 >> 3];
    for (int r = r0 & ~7; r < r1; r++)
        for (int x = 0; x < TP_LOGO_W; ) {
            unsigned b = *p++, n = b >> 4;
            n = n < 15u ? n + 1u : 16u + *p++;
            if (r >= r0 && (b & 15u)) gfx_fill(x0 + x, y0 + r, (int)n, 1, tp_logo_pal[b & 15u]);   /* (0 is the background round it: the page already) */
            x += (int)n;
        }
}
void vis_splash_draw(void)
{
    /* Teachie Peachie: its logo on the cards' peach, SLOOP 3 credited under it in their red (SLOOP's sailboat: logo(), no
     * longer drawn here) */
    gfx_fill(0, 0, GFX_W, GFX_H, TP_PAPER);
    tp_logo_draw((GFX_W - TP_LOGO_W) / 2, 6);
    gfx_text_c(&font_s, 120, 212, "WITH SLOOP 3", TP_INK);
}
void vis_draw(void)
{
    gfx_fill(0, 0, GFX_W, GFX_H, BG);
    if (!M || M->magic != MAGIC) return;
    switch (I.style) {
    case VS_SCOPE: scope(); break; case VS_SPEC: spectrum(); break; case VS_VU: vu(); break;
    case VS_LCD: lcd(); break; case VS_WIRES: wires(); break; default: sloop(); break;
    }
    if (I.name && gfx_visible(20, 2, 200, 26)) {
        gfx_rfill(20, 2, 200, 24, 8, ui_pal.panel2);
        gfx_text_c(&font_s, 120, 8, vis_name[I.style < VIS_N ? I.style : 0], TXT);
    }
}
