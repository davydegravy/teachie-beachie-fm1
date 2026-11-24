/* SPDX-License-Identifier: GPL-3.0-only
 * A track's SLICER and INSERT (SLOOP 2.8.5's chain of a track, docs/sloop/SPEC-SON.md), written for this project after
 * SLOOP's behaviour: voices -> SLICER -> DIST -> INSERT -> the track's filter -> level, pan, sends.
 *
 * SLICER: a 16-step gate / stutter on the transport.
 *   SLCR OFF GATE STUT; PAT one of 16 patterns ('x' open / live, '.' closed / repeated); RATE the step (1/8 1/16 1/32
 *   8T 16T 32T); DEPTH how far a '.' closes (GATE), the repeat's level (STUT).
 *   The steps follow the transport (step 0 on its start, the track's swing plus the global one, as the sequencer
 *   plays them), running free at the tempo while stopped. GATE: the gain moves at most 1/128 a sample (2.9 ms), a
 *   closing ramp ending on the step's boundary, an opening one starting on it. STUT: an 'x' step plays live and is
 *   recorded (22.05 kHz, up to 186 ms); a '.' step plays the recording from its start, looped at the step's length
 *   (halved until it fits), every pass windowed (2.9 ms at both ends and at the step's end), cross-faded with the
 *   live sound by DEPTH as the gate moves. The recordings: IMA ADPCM, in three slots shared by the tracks.
 * INSERT: one effect, A B C and a dry / wet MIX (gliding 2.9 ms; a type change fades the old one out, clears, fades
 *   the new one in; MIX 0 or OFF: the track untouched).
 *   SOFT HARD FOLD FUZZ  a low cut (~27 Hz), DRIVE (1x .. 12.8x, squared), the shape (SOFT a soft clip, HARD a flat
 *                        clip, FOLD a triangle folder, FUZZ a hard top and a soft halved bottom), TONE (two one-pole
 *                        low-passes on the cutoff curve), LEVEL ((v - 112) / 2 dB), a DC blocker (~14 Hz)
 *   CRUSH                a held sample (RATE: 689 Hz .. 44.1 kHz), BITS (1..16), LPF (two one-pole low-passes)
 *   PHASR                four all-passes swept 150 Hz .. 4.8 kHz (around ~850 Hz by DEPTH) at RATE (the LFO curve),
 *                        FDBK 0 .. 0.7; the wet: (dry + swept) / 2
 *   FLANG CHOR           a 5.8 ms line: FLANG swept 0.25 .. 5 ms (a squared triangle), FDBK 0 .. 0.85; CHOR a sine
 *                        around 3.2 ms (+-2.3 ms at full DEPTH), FDBK 0 .. 0.4; the wet: (dry + delayed) / 2
 *   Stereo: each side its own filters and all-passes; the line (FLANG, CHOR) takes both sides' mean, in four slots
 *   shared by the tracks.
 * The state is the module's: one gb_sound in a program (tfx_reset: snd_init). */
#include "engine.h"
#include "sample.h"

enum { SL_OFF, SL_GATE, SL_STUT };
enum { IT_OFF, IT_SOFT, IT_HARD, IT_FOLD, IT_FUZZ, IT_CRUSH, IT_PHASER, IT_FLANGER, IT_CHORUS };
#define RAMP 128
#define SLOPE (1.0f / RAMP)
#define REC_N 4096u                       /* a recording: 22.05 kHz samples (186 ms) */
#define NREC 3
#define LINE 256u                         /* FLANG / CHOR: the line (5.8 ms) */
#define NLINE 4

/* bit k = step k: 1 'x', 0 '.' (SLOOP's 16 patterns) */
static const uint16_t SL_PAT[16] = { 0x5555, 0xB6DB, 0x5249, 0xB777, 0x6D6D, 0x0F0F, 0x1111, 0x54A5,
                                     0x5333, 0xA929, 0xAAFF, 0xD501, 0xAAAA, 0xAB6B, 0xBB5D, 0x7597 };
static const uint8_t SL_DEN[6] = { 2, 4, 8, 3, 6, 12 };        /* steps a beat */
static const uint8_t INS_HOLD[16] = { 64, 48, 40, 32, 24, 20, 16, 12, 10, 8, 6, 5, 4, 3, 2, 1 };   /* CRUSH RATE */

typedef struct {
    float pos;                            /* the free clock (stopped): steps, 0..32 */
    float gc, w;                          /* the gate's closure, the repeat's share */
    uint32_t rp, loop, rec;               /* the repeat's read position and loop (44.1 kHz), recorded (22.05 kHz) */
    float half; uint32_t half_n;          /* recording: the first of a pair */
    int16_t pred, rpred; uint8_t idx, ridx, rec_on, bit, step, slot;   /* the encoder's and the reader's states */
    float r0, r1; uint32_t rk;            /* the reader: the decoded samples k - 1 and k, k */
} slicer;
typedef struct {
    float w; uint8_t type, cnt, slot; uint16_t wp;
    uint32_t ph;
    float hp[2], lp1[2], lp2[2], dc[2], hold[2], px[2][4], py[2][4], fb[2];
} insert;
static slicer SLC[GB_NTRACK];
static insert INS[GB_NTRACK];
static uint8_t rec_buf[NREC][REC_N / 2] GB_BOOTRAM;       /* (written before read: a recording, then its repeat) */
static int8_t rec_trk[NREC];              /* the track + 1 using it */
static uint32_t rec_last[NREC];
static int16_t line_buf[NLINE][LINE] GB_BOOTRAM;          /* (cleared when a track takes it) */
static int8_t line_trk[NLINE];
static uint32_t line_last[NLINE];

/* a slot of a pool for track t (one not used in the last two blocks), or -1 */
static int pool_slot(int8_t *who, uint32_t *last, int n, int t)
{
    int best = -1; uint32_t age = 1;
    for (int i = 0; i < n; i++) if (who[i] == t + 1) { last[i] = gb_blocks; return i; }
    for (int i = 0; i < n; i++) { uint32_t a = who[i] ? gb_blocks - last[i] : 0xFFFFFFFFu; if (a > age) { age = a; best = i; } }
    if (best >= 0) { who[best] = (int8_t)(t + 1); last[best] = gb_blocks; }
    return best;
}

/* ---------------------------------------------------------------- the ADPCM of the recordings */
GB_INLINE int ima_enc(int x, int16_t *pred, uint8_t *idx)
{
    int step = smp_ima_step[*idx], diff = x - *pred, code = 0, vd = step >> 3;
    if (diff < 0) { code = 8; diff = -diff; }
    if (diff >= step) { code |= 4; diff -= step; vd += step; }
    step >>= 1; if (diff >= step) { code |= 2; diff -= step; vd += step; }
    step >>= 1; if (diff >= step) { code |= 1; vd += step; }
    int p = *pred + ((code & 8) ? -vd : vd); *pred = (int16_t)(p < -32768 ? -32768 : p > 32767 ? 32767 : p);
    int i = *idx + smp_ima_idx[code & 7]; *idx = (uint8_t)(i < 0 ? 0 : i > 88 ? 88 : i);
    return code;
}
GB_INLINE int ima_dec(int code, int16_t *pred, uint8_t *idx)
{
    int step = smp_ima_step[*idx], vd = step >> 3;
    if (code & 4) vd += step;
    if (code & 2) vd += step >> 1;
    if (code & 1) vd += step >> 2;
    int p = *pred + ((code & 8) ? -vd : vd); *pred = (int16_t)(p < -32768 ? -32768 : p > 32767 ? 32767 : p);
    int i = *idx + smp_ima_idx[code & 7]; *idx = (uint8_t)(i < 0 ? 0 : i > 88 ? 88 : i);
    return *pred;
}
#ifndef __PI32V2__
uint32_t tfx_decodes;                     /* (the PC's tests: the repeat's decoding, counted) */
#endif
/* the recording's sample k (22.05 kHz), read in order from the start (the reader starts over at 0). The repeat
 * asks for k, then k + 1 (between two samples), then k + 1 again: the last two are kept, so that only a pass's
 * start decodes from 0 (3.1.4: asking for k + 1 then k + 1 again started over at 0 each time, ~2000 decodes a
 * sample: STUT's repeat took 15 times a core, Sam's PEAK at 1500 %) */
static float rec_at(slicer *s, const uint8_t *b, uint32_t k)
{
    if (s->rk && k + 1u == s->rk) return s->r1;
    if (s->rk >= 2u && k + 2u == s->rk) return s->r0;
    if (k < s->rk || !s->rk) { s->rpred = 0; s->ridx = 0; s->rk = 0; s->r0 = s->r1 = 0.0f; }
    while (s->rk <= k) {
#ifndef __PI32V2__
        tfx_decodes++;
#endif
        uint32_t j = s->rk < s->rec ? s->rk : s->rec - 1u;
        int c = (b[j >> 1] >> ((j & 1u) * 4u)) & 15;
        s->r0 = s->r1; s->r1 = (float)ima_dec(c, &s->rpred, &s->ridx) * (2.0f / 32768.0f);
        s->rk++;
    }
    return s->rk - 1u == k ? s->r1 : s->r0;
}

/* ---------------------------------------------------------------- SLICER */
static void sl_enter(slicer *s, const gb_patch *p, int t, int idx, float base)
{
    int mode = gb_pv(p->p, SP_SLCR);
    s->step = (uint8_t)idx;
    s->bit = (uint8_t)((SL_PAT[(gb_pv(p->p, SP_SLPAT) - 1) & 15] >> idx) & 1u);
    s->rp = 0; s->loop = 0; s->rec_on = 0;
    if (mode != SL_STUT) { s->rec = 0; return; }
    if (s->bit) {                                         /* a live step: recorded */
        int k = pool_slot(rec_trk, rec_last, NREC, t);
        s->rec = 0; s->half_n = 0; s->pred = 0; s->idx = 0;
        if (k >= 0) { s->slot = (uint8_t)k; s->rec_on = 1; } else s->slot = 0xFF;
    } else if (s->rec >= 2u * RAMP && s->slot < NREC && rec_trk[s->slot] == t + 1) {   /* a repeat of the last one */
        uint32_t l = (uint32_t)base;
        while (l > 2u * REC_N) l >>= 1;
        s->loop = l < 2u * s->rec ? l : 2u * s->rec;
        s->rk = 0;
    }
}
/* the SLICER on a track's l, r (0: silent, the clock only); 1 while it changes the sound */
static int slicer_run(gb_sound *snd, int t, float *l, float *r)
{
    slicer *s = &SLC[t];
    const gb_patch *p = &snd->trk[t].patch;
    int mode = gb_pv(p->p, SP_SLCR), den = SL_DEN[gb_pv(p->p, SP_SLRATE) % 6];
    float beat = snd->beat > 1.0f ? snd->beat : GB_SR * 0.5f, fps = beat / (float)den;   /* frames a step */
    float sw = (float)snd->trk[t].swing * (1.0f / 200.0f);   /* the long step: 1 + sw */
    /* where the block starts: steps into the 32-step pair grid (the transport's, else the free clock) */
    float pos;
    if (snd->grid >= 0.0f) {
        uint32_t whole = (snd->bar * 4u * (uint32_t)den) & 31u;
        pos = (float)whole + snd->grid / fps;
        while (pos >= 32.0f) pos -= 32.0f;
    } else pos = s->pos;
    float d = 1.0f / fps;
    s->pos = pos + (float)GB_BLOCK * d; while (s->pos >= 32.0f) s->pos -= 32.0f;
    if (!l) { s->gc = s->w = 0.0f; return 0; }
    if (mode == SL_OFF && s->gc <= 0.0f && s->w <= 0.0f) { s->rec = 0; return 0; }
    float depth = (float)gb_pv(p->p, SP_SLDEPTH) * (1.0f / 127.0f);
    uint16_t pat = SL_PAT[(gb_pv(p->p, SP_SLPAT) - 1) & 15];
    uint8_t *rb = s->slot < NREC ? rec_buf[s->slot] : 0;
    for (int n = 0; n < GB_BLOCK; n++, pos += d) {
        if (pos >= 32.0f) pos -= 32.0f;
        int pair = (int)(pos * 0.5f); float rr = pos - 2.0f * (float)pair;
        int odd = rr >= 1.0f + sw, idx = (2 * pair + odd) & 15;
        float edge = odd ? 2.0f : 1.0f + sw;                       /* the step's end, in the pair */
        uint32_t left = (uint32_t)((edge - rr) * fps) + 1u;        /* frames to it */
        if (idx != s->step) {
            sl_enter(s, p, t, idx, fps);
            rb = s->slot < NREC ? rec_buf[s->slot] : 0;
        }
        int nbit = (pat >> ((idx + 1) & 15)) & 1;
        float tc = mode == SL_GATE && !s->bit ? depth : 0.0f, tn = mode == SL_GATE && !nbit ? depth : 0.0f;
        float tw = mode == SL_STUT && s->loop ? depth : 0.0f, tg = tc;
        if (left <= RAMP && tn > tg) tg = tn;                      /* close by the boundary */
        s->gc += gb_clampf(tg - s->gc, -SLOPE, SLOPE);
        s->w += gb_clampf(tw - s->w, -SLOPE, SLOPE);
        float g = 1.0f - s->gc, yl = l[n] * g, yr = r[n] * g;
        if (s->rec_on && rb) {                                     /* 2:1, the pair's mean */
            float x = 0.5f * (l[n] + r[n]);
            if (s->half_n++ & 1u) {
                int v = (int)((s->half + x) * 0.5f * 16384.0f); v = v < -32768 ? -32768 : v > 32767 ? 32767 : v;
                int c = ima_enc(v, &s->pred, &s->idx);
                uint8_t *q = &rb[s->rec >> 1];
                *q = (s->rec & 1u) ? (uint8_t)((*q & 15u) | (unsigned)c << 4) : (uint8_t)c;
                if (++s->rec >= REC_N) s->rec_on = 0;
            } else s->half = x;
        }
        if (s->loop && rb) {                                       /* the repeat, windowed */
            uint32_t rp = s->rp, e = s->loop - 1u - rp;
            float v = rec_at(s, rb, rp >> 1);
            if (rp & 1u) v = 0.5f * (v + rec_at(s, rb, ((rp >> 1) + 1u) < s->rec ? (rp >> 1) + 1u : rp >> 1));
            if (rp < e) e = rp;
            if (left - 1u < e) e = left - 1u;
            if (e < RAMP) v *= (float)e * (1.0f / RAMP);
            if (++s->rp >= s->loop) { s->rp = 0; }
            yl += (v - yl) * s->w; yr += (v - yr) * s->w;
        } else if (s->w > 0.0f) { yl -= yl * s->w; yr -= yr * s->w; }
        l[n] = yl; r[n] = yr;
    }
    return 1;
}

/* ---------------------------------------------------------------- INSERT */
GB_INLINE float one_pole_k(int idx) { float g = gb_tan_pi(gb_minf(gb_cut_hz((float)idx) * GB_ISR, 0.49f)); return g / (1.0f + g); }
GB_INLINE float tpt(float x, float *s, float k) { float v = (x - *s) * k, y = v + *s; *s = y + v; return y; }
/* the drives' shape, on our scale (a voice's full scale: 0.5) */
GB_INLINE float shape(int ty, float v)
{
    switch (ty) {
    case IT_SOFT: return 0.5f * gb_softclip(v * 2.0f);
    case IT_HARD: return gb_clampf(v, -0.3662f, 0.3662f);
    case IT_FOLD: {                                                 /* the wave reflected at +-0.25 (a triangle of period 1) */
        float z = v + 0.25f; z -= (float)(int)z; z += z < 0.0f ? 1.0f : 0.0f;
        float u = z - 0.5f;
        return 1.5f * (0.25f - (u < 0.0f ? -u : u));
    }
    case IT_FUZZ: return v > 0.0f ? (v > 0.3052f ? 0.3052f : v) : 0.25f * gb_softclip(v * 2.0f);
    default: return v;
    }
}
static int insert_run(gb_sound *snd, int t, float *l, float *r)
{
    insert *s = &INS[t];
    const float *q = snd->trk[t].patch.p;
    int ty = gb_pv(q, SP_ITYPE);
    if (s->type != ty && s->w <= 0.0f) s->type = (uint8_t)ty;
    int mx = gb_pv(q, SP_IMIX);
    float tw = s->type == ty && ty ? (mx >= 127 ? 1.0f : (float)mx * (1.0f / 127.0f)) : 0.0f;
    if (s->w <= 0.0f) {
        if (tw <= 0.0f) return 0;                                   /* dry: untouched */
        memset(s, 0, sizeof *s); s->type = (uint8_t)ty; s->slot = 0xFF;
    }
    int a = gb_pv(q, SP_IA), b = gb_pv(q, SP_IB), c = gb_pv(q, SP_IC);
    float w = s->w;
    if (s->type <= IT_FUZZ) {
        float g = 1.0f + (float)(a * a) * (3.0f / 4096.0f), k = one_pole_k(b), lv = c ? gb_db((float)(c - 112) * 0.5f) : 0.0f;
        for (int ch = 0; ch < 2; ch++) {
            float *x = ch ? r : l, hp = s->hp[ch], l1 = s->lp1[ch], l2 = s->lp2[ch], dc = s->dc[ch], ww = w;
            for (int n = 0; n < GB_BLOCK; n++) {
                ww += gb_clampf(tw - ww, -SLOPE, SLOPE);
                hp += (x[n] - hp) * (1.0f / 256.0f);
                float y = shape(s->type, gb_clampf(x[n] - hp, -3.5f, 3.5f) * g);
                y = tpt(tpt(y, &l1, k), &l2, k) * lv;
                dc += (y - dc) * (1.0f / 512.0f);
                x[n] += (y - dc - x[n]) * ww;
            }
            s->hp[ch] = gb_flush(hp); s->lp1[ch] = gb_flush(l1); s->lp2[ch] = gb_flush(l2); s->dc[ch] = gb_flush(dc);
            if (ch) w = ww;
        }
    } else if (s->type == IT_CRUSH) {
        float stp = (float)(1 << (15 - (a >> 3))) * (0.5f / 32768.0f), is = 1.0f / stp, k = one_pole_k(c);
        int hold = INS_HOLD[b >> 3], cnt = s->cnt;
        float hl = s->hold[0], hr = s->hold[1], l1 = s->lp1[0], l2 = s->lp2[0], r1 = s->lp1[1], r2 = s->lp2[1];
        for (int n = 0; n < GB_BLOCK; n++) {
            w += gb_clampf(tw - w, -SLOPE, SLOPE);
            if (!cnt) {
                float u = gb_clampf(l[n], -4.0f, 4.0f) * is, v = gb_clampf(r[n], -4.0f, 4.0f) * is;
                hl = (float)(int)(u + (u < 0.0f ? -0.5f : 0.5f)) * stp; hr = (float)(int)(v + (v < 0.0f ? -0.5f : 0.5f)) * stp;
                cnt = hold;
            }
            cnt--;
            float yl = tpt(tpt(hl, &l1, k), &l2, k), yr = tpt(tpt(hr, &r1, k), &r2, k);
            l[n] += (yl - l[n]) * w; r[n] += (yr - r[n]) * w;
        }
        s->cnt = (uint8_t)cnt; s->hold[0] = hl; s->hold[1] = hr;
        s->lp1[0] = gb_flush(l1); s->lp2[0] = gb_flush(l2); s->lp1[1] = gb_flush(r1); s->lp2[1] = gb_flush(r2);
    } else if (s->type == IT_PHASER) {                              /* the coefficient a block at a time (its middle) */
        uint32_t inc = (uint32_t)(gb_lfo_hz((float)a) * GB_ISR * 4294967296.0f);
        float u = (float)(s->ph + (GB_BLOCK / 2) * inc) * (1.0f / 4294967296.0f), tri = u < 0.5f ? 2.0f * u : 2.0f - 2.0f * u;
        float qq = 0.5f + (tri - 0.5f) * (float)b * (1.0f / 127.0f);
        float tn = gb_tan_pi(150.0f * gb_exp2f(5.0f * qq) * GB_ISR), co = (tn - 1.0f) / (tn + 1.0f), fbk = (float)c * (180.0f / 32768.0f);
        for (int ch = 0; ch < 2; ch++) {
            float *x = ch ? r : l, *px = s->px[ch], *py = s->py[ch], fb = s->fb[ch], dc = s->dc[ch], ww = w;
            for (int n = 0; n < GB_BLOCK; n++) {
                ww += gb_clampf(tw - ww, -SLOPE, SLOPE);
                float in = gb_clampf(x[n], -2.0f, 2.0f) + fb * fbk, y = in;
                for (int k = 0; k < 4; k++) { float o = px[k] + co * (y - py[k]); px[k] = y; py[k] = gb_clampf(o, -8.0f, 8.0f); y = py[k]; }
                fb = gb_clampf(y, -4.0f, 4.0f);
                dc += (y - dc) * (1.0f / 512.0f);
                x[n] += (0.5f * x[n] + 0.5f * (y - dc) - x[n]) * ww;
            }
            s->fb[ch] = gb_flush(fb); s->dc[ch] = gb_flush(dc);
            for (int k = 0; k < 4; k++) { px[k] = gb_flush(px[k]); py[k] = gb_flush(py[k]); }
            if (ch) w = ww;
        }
        s->ph += GB_BLOCK * inc;
    } else {                                                        /* FLANG, CHOR: a line of both sides' mean */
        if (s->slot >= NLINE || line_trk[s->slot] != t + 1) {
            int k = pool_slot(line_trk, line_last, NLINE, t);
            if (k < 0) { for (int n = 0; n < GB_BLOCK; n++) w += gb_clampf(-w, -SLOPE, SLOPE); s->w = w; return w > 0.0f; }
            s->slot = (uint8_t)k; memset(line_buf[k], 0, sizeof line_buf[k]);
        } else line_last[s->slot] = gb_blocks;
        int16_t *ln = line_buf[s->slot];
        int chorus = s->type == IT_CHORUS;
        uint32_t inc = (uint32_t)(gb_lfo_hz((float)a) * GB_ISR * 4294967296.0f), ph = s->ph, wp = s->wp;
        float dep = (float)b * (1.0f / 127.0f), fbk = (float)c * ((chorus ? 103.0f : 219.0f) / 32768.0f), dl = s->dc[0], dr = s->dc[1];
        for (int n = 0; n < GB_BLOCK; n++, ph += inc) {
            w += gb_clampf(tw - w, -SLOPE, SLOPE);
            float u = (float)ph * (1.0f / 4294967296.0f), dly;
            if (chorus) dly = 140.0f + gb_sin_turn(u) * dep * 99.2f;
            else { float tr = u < 0.5f ? 2.0f * u : 2.0f - 2.0f * u; dly = 11.0f + 209.0f * tr * tr * dep; }
            int di = (int)dly; float fr = dly - (float)di;
            uint32_t r0 = (wp - (uint32_t)di) & (LINE - 1u), r1 = (r0 - 1u) & (LINE - 1u);
            float v = ((float)ln[r0] + ((float)ln[r1] - (float)ln[r0]) * fr) * (2.0f / 32768.0f);
            float x = 0.5f * (l[n] + r[n]) + 2.0f * gb_softclip(0.5f * v * fbk);
            int iv = (int)(x * 16384.0f); ln[wp] = (int16_t)(iv < -32767 ? -32767 : iv > 32767 ? 32767 : iv);
            wp = (wp + 1u) & (LINE - 1u);
            dl += (v - dl) * (1.0f / 512.0f);
            float y = v - dl;
            l[n] += (0.5f * (l[n] + y) - l[n]) * w; r[n] += (0.5f * (r[n] + y) - r[n]) * w;
        }
        s->ph = ph; s->wp = (uint16_t)wp; s->dc[0] = gb_flush(dl); (void)dr;
    }
    s->w = w;
    return w > 0.0f;
}

/* a track's SLICER (before DIST) and INSERT (after it), and whether they still sound with no voice */
int tfx_slicer(gb_sound *s, int t, float *l, float *r) { return slicer_run(s, t, l, r); }
int tfx_insert(gb_sound *s, int t, float *l, float *r) { return insert_run(s, t, l, r); }
int tfx_busy(const gb_sound *s, int t)
{
    const slicer *c = &SLC[t];
    return c->w > 0.0f || (gb_pv(s->trk[t].patch.p, SP_SLCR) == SL_STUT && c->loop) || INS[t].w > 0.0f;
}
/* (4.0.3) the SLICER page's picture (as SLOOP 2.8.5's): the pattern's 16 steps, the step playing (bits 16..19) */
uint32_t tfx_slicer_view(int t, int pat) { return SL_PAT[(pat - 1) & 15] | (uint32_t)(SLC[t].step & 15u) << 16; }
void tfx_reset(void)
{
    memset(SLC, 0, sizeof SLC); memset(INS, 0, sizeof INS);
    for (int t = 0; t < GB_NTRACK; t++) { SLC[t].slot = 0xFF; SLC[t].step = 0xFF; INS[t].slot = 0xFF; }
    memset(rec_trk, 0, sizeof rec_trk); memset(line_trk, 0, sizeof line_trk);
}
