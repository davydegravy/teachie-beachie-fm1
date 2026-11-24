/* SPDX-License-Identifier: GPL-3.0-only
 * DRUM: SLOOP's drum kits (docs/sloop/SPEC-MOTEURS.md, engine/drum.h), written for this project. A key plays one of
 * 16 sounds (the note modulo 16), in SLOOP's order of the drum track's white keys: KICK, KICK 2, SNARE, CLAP, HAT,
 * OPEN HAT, PEDAL, RIM, SNARE 2, LOW TOM, HI TOM, CRASH, RIDE, SHAKER, CONGA, COWBELL.
 *   E0 KIT    0..4: the General MIDI sampled kit as it is (ACOUSTIC) and treated: DEEP (lower, darker), TIGHT
 *             (shortened), BRIGHT (higher), DUST (lower, darker, 8 bits); 5..36: the 32 synthesised kits
 *   E1 TUNE   the whole kit, semitones          E2 DECAY  the whole kit, -4 .. +3 steps     E3 DRIVE  added
 * and each sound's own TUNE (-8 .. +7) and DECAY (-4 .. +3: as the kit has it at 0). A closed or pedal hat ends the
 * open one. The synthesised kits' tables are in the flash window: the kits in use are kept in RAM (3), so that a
 * save (the window off) never stops the drums; the sampled kit is silent then (it lives in the flash). */
#include "engine.h"
#include "sample.h"

static const gb_pd ED[8] = { { 0, GB_NKITS - 1, GB_KIT_SAMPLED }, { -12, 12, 0 }, { -4, 3, 0 }, { 0, 127, 0 }, { 0, 0, 0 },
                             { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
/* each key's General MIDI note (the sampled kit's zones) */
static const uint8_t GM[16] = { 36, 35, 38, 39, 42, 46, 44, 37, 40, 43, 48, 49, 51, 70, 63, 56 };
/* a DECAY step: the times' index moved (x 0.25 .. x 3 the time) and the holds scaled; a sampled sound's shorter
 * DECAYs (-4 .. -1): a fade to -60 dB in 60, 120, 250, 500 ms (times' indices) */
static const int8_t DK_IDX[8] = { -26, -17, -10, -4, 0, 6, 13, 21 };
static const float DK_HOLD[8] = { 0.25f, 0.398f, 0.602f, 0.801f, 1.0f, 1.398f, 2.0f, 3.0f };
static const uint8_t DK_SMP[4] = { 47, 60, 74, 87 };
/* the metal noise: six squares at the classic cymbal's ratios, in 1/16 semitones above the first */
static const int16_t METAL[6] = { 0, 109, 163, 259, 268, 377 };

/* ---- SYN1..SYN4: in RAM (from SLOOP's settings at power-on, fw/main.c) */
gb_dsnd gb_dsyn[4][16];
uint8_t gb_dsyn_crush[4];
GB_SMALL void drum_syn_default(void)
{
    static const uint8_t DEF[4] = { 0, 1, 5, 12 };            /* (window on: the factory kits are read) */
    for (int k = 0; k < 4; k++) { for (int j = 0; j < 16; j++) gb_dsyn[k][j] = gb_dsnds[gb_dkits[DEF[k]].s[j]]; gb_dsyn_crush[k] = gb_dkits[DEF[k]].crush; }
}
/* ---- the synthesised kits in use, in RAM */
#define NKC 3
static gb_dsnd KC[NKC][16];
static uint8_t kc_kit[NKC], kc_crush[NKC];       /* the kit + 1 (0: none) */
static uint32_t kc_last[NKC];
GB_SMALL static int kit_cached(int kit)                   /* its slot (-1: not here, and the window off) */
{
    int best = 0; uint32_t age = 0;
    for (int i = 0; i < NKC; i++) if (kc_kit[i] == kit + 1) { kc_last[i] = gb_blocks; return i; }
    if (!gb_flash_ok && kit < GB_KIT_SYN) return -1;
    for (int i = 0; i < NKC; i++) { uint32_t a = kc_kit[i] ? gb_blocks - kc_last[i] : 0xFFFFFFFFu; if (a > age) { age = a; best = i; } }
    if (kit >= GB_KIT_SYN) { for (int j = 0; j < 16; j++) KC[best][j] = gb_dsyn[kit - GB_KIT_SYN][j]; kc_crush[best] = gb_dsyn_crush[kit - GB_KIT_SYN]; }
    else { const gb_dkit *k = &gb_dkits[kit - GB_KIT_SAMPLED]; for (int j = 0; j < 16; j++) KC[best][j] = gb_dsnds[k->s[j]]; kc_crush[best] = k->crush; }
    kc_kit[best] = (uint8_t)(kit + 1); kc_last[best] = gb_blocks;
    return best;
}
void drum_syn_forget(int k)                     /* SYN k changed (the main loop): its cached copy goes, the next hit takes it again */
{
    for (int i = 0; i < NKC; i++) if (kc_kit[i] == GB_KIT_SYN + k + 1) kc_kit[i] = 0;
}
static void drum_block(const gb_patch *p, int t, float beat)    /* (keeps the track's kit at hand before a save) */
{
    (void)t; (void)beat;
    int kit = gb_ev(p, 0);
    if (kit >= GB_KIT_SAMPLED && (kit < GB_KIT_USR || kit >= GB_KIT_SYN)) kit_cached(kit);
}

GB_INLINE float tk(int idx) { idx = idx < 0 ? 0 : idx > 127 ? 127 : idx; return 5.0f * gb_exp2f((float)idx * (9.6438562f / 127.0f)); }  /* ms */
GB_INLINE float kdec(int idx) { return gb_exp2f(-9.9657843f / (tk(idx) * 0.001f * GB_SR)); }   /* -60 dB in that time */
GB_INLINE uint32_t pinc(float note) { return (uint32_t)(gb_note_hz(gb_clampf(note, 0.0f, 127.9375f)) * GB_ISR * 4294967296.0f); }
static float onepole(int idx) { float g = gb_tan_pi(gb_minf(gb_cut_hz((float)idx) * GB_ISR, 0.49f)); return g / (1.0f + g); }
static void svf_coef(gb_voice *v)
{
    float c = gb_clampf(v->s.drum.cut + v->s.drum.pe * (float)v->s.drum.d.fenv, 0.0f, 127.0f);
    float g = gb_tan_pi(gb_minf(gb_cut_hz(c) * GB_ISR, 0.49f)), k = v->s.drum.fk;
    v->s.drum.a1 = 1.0f / (1.0f + g * (g + k)); v->s.drum.a2 = g * v->s.drum.a1; v->s.drum.a3 = g * v->s.drum.a2;
}

static int perc_set(void)                         /* the bank's General MIDI kit */
{
    for (int i = 0; i < smp_nsets(); i++) if (smp_set_at(i)->kind == SK_KIT) return i;
    return -1;
}
static int usr_zone(int k, int gm)                /* a user slot's zone for a key's GM note, exactly (-1: none) */
{
    int zi = -1;
    for (int i = 0; i < smp_user_nz(k); i++) {
        uint32_t id = SMP_USR | (uint32_t)k << 5 | (uint32_t)i; const smp_zone *z = smp_zone_of(id);
        if (gm >= z->lo && gm <= z->hi) zi = (int)id;
    }
    return zi;
}
/* (3.4.0, Sam: "the drum kits barely heard, above all the kicks, even with the drums higher than the other tracks")
 * the kits 3 times (+9.5 dB) as loud as before (0.5), so that the drum track at the synths' level (45) is heard over
 * them; a new project's drums at 45 like the others (were 85), a project from before scaled down to sound as it did
 * (app/project.c); the kicks (lanes 1-2) +3 dB more */
#define GB_DRUM_GAIN 1.5f
#define GB_KICK_GAIN 1.41f
GB_SMALL static void drum_start(gb_voice *v, const gb_patch *p, int fresh)
{
    (void)fresh;
    int lane = v->note & 15, kit = gb_ev(p, 0), tune = gb_ev(p, 1) + p->dtune[lane];
    int ds = gb_ev(p, 2) + GB_DDEC(p->ddec[lane]); ds = ds < -4 ? -4 : ds > 3 ? 3 : ds;
    float vel = v->vel, lg = vel * (lane < 2 ? GB_DRUM_GAIN * GB_KICK_GAIN : GB_DRUM_GAIN);   /* (3.4.0) the hit's gain */
    { int dl = gb_dlev(p->ddec[lane]); if (dl) lg *= gb_db((float)dl); }   /* (3.4.2, #25) the sound's own level */
    if (kit < GB_KIT_SAMPLED || (kit >= GB_KIT_USR && kit < GB_KIT_SYN)) {   /* the GM sampled kit, a user slot's kit */
        memset(&v->s.dsmp, 0, sizeof v->s.dsmp);
        v->s.dsmp.smp = 1; v->s.dsmp.ended = 1; v->s.dsmp.kit = (uint8_t)(kit < GB_KIT_SAMPLED ? kit : 0);
        int zi = -1, gm = GM[lane];
        if (!gb_flash_ok || !smp_bank_ok()) return;
        if (kit < GB_KIT_SAMPLED) { int set = perc_set(); if (set >= 0) zi = smp_find(set, gm); }
        else if (kit < GB_KIT_USR + 4) zi = usr_zone(kit - GB_KIT_USR, gm);
        else { zi = usr_zone(2, gm); if (zi < 0) zi = usr_zone(3, gm); }   /* USR3+4: the sound in USR3, else in USR4 */
        if (zi < 0) return;
        const smp_zone *z = smp_zone_of((uint32_t)zi);
        int shift = kit == 1 ? (gm <= 36 ? -5 : -2) : kit == 3 ? 2 : kit == 4 ? -1 : 0;
        float semi = gb_clampf((float)(gm + shift + tune) - (float)z->root16 * (1.0f / 16.0f), -96.0f, 127.0f);
        v->s.dsmp.zone = (uint16_t)zi; v->s.dsmp.ended = 0;
        v->s.dsmp.step = gb_minf(gb_exp2f(semi * (1.0f / 12.0f)) * (float)z->rate * (1.0f / 65536.0f), 8.0f);   /* (3.3.6: the zone's rate is 65536 x its rate / 44.1 kHz, as SAMPLE reads it; was / 44100: 1.49 x too fast) */
        v->s.dsmp.env = 1.0f; v->s.dsmp.denv = 1.0f;
        v->s.dsmp.dkf = ds < 0 ? kdec(DK_SMP[ds + 4]) : 1.0f;
        v->s.dsmp.gain = lg * 2.43f;   /* (3.4.0: as loud as the synthesised kits, K-weighted (tests/level_tool.c drums); was 1.35, their peaks) */
        return;
    }
    int slot = kit_cached(kit);
    memset(&v->s.drum, 0, sizeof v->s.drum);
    if (slot < 0) { v->s.drum.smp = 2; return; }                  /* (a kit not at hand during a save: silent) */
    const gb_dsnd *d = &KC[slot][lane];
    v->s.drum.d = *d; v->s.drum.crush = kc_crush[slot];
    int dk = DK_IDX[ds + 4];
    float hs = DK_HOLD[ds + 4];
    v->s.drum.base = (float)d->note16 * (1.0f / 16.0f) + (float)tune;
    v->s.drum.pe = d->bend || d->fenv ? 1.0f : 0.0f;
    v->s.drum.kpe = kdec(d->bt);
    v->s.drum.amp = d->wave ? (float)d->tlev * (1.0f / 127.0f) : 0.0f;
    v->s.drum.ka = kdec(d->decay + dk);
    v->s.drum.hold = (uint32_t)((float)d->hold * 0.002f * GB_SR * hs);
    int clap = (d->src & DN_CLAP) != 0;
    v->s.drum.nz = (d->src & 15) ? (float)d->nlev * (1.0f / 127.0f) : 0.0f;
    v->s.drum.kn = clap ? kdec(12) : kdec(d->ndec + dk);         /* the clap: short bursts first */
    v->s.drum.nhold = clap ? 0u : (uint32_t)((float)d->nhold * 0.002f * GB_SR * hs);
    v->s.drum.bursts = (uint8_t)(clap ? 1 : 4);
    v->s.drum.ck = (float)d->click * (1.0f / 127.0f);
    v->s.drum.fmode = d->flt & 7u;
    v->s.drum.fk = 2.0f - (float)(d->flt >> 3) * (245.0f / 4096.0f);
    v->s.drum.cut = (float)d->fcut + (vel * 127.0f - 110.0f) * (40.0f / 256.0f);   /* softer is darker */
    int hpf = d->hpf;
    if (!d->wave && tune) {                                       /* no tone: TUNE moves its brightness */
        float co = (float)tune * 1.1689453f;
        v->s.drum.cut += co;
        if (hpf) { hpf += (int)co; hpf = hpf < 1 ? 1 : hpf > 127 ? 127 : hpf; }
    }
    v->s.drum.ahp = d->hpf ? onepole(hpf) : 0.0f;
    if (v->s.drum.fmode & 3u) svf_coef(v);
    v->s.drum.rnd = 0x9E3779B9u ^ ((uint32_t)v->note * 2654435761u) ^ v->seed;
    v->s.drum.lfsr = 0x4001u;
    if (d->chip) {
        float c = gb_note_hz((float)d->chip) * 8.0f * GB_ISR * 4294967296.0f;
        v->s.drum.cinc = c >= 4294967040.0f ? 0xFFFFFFFFu : (uint32_t)c;
    }
    for (int i = 0; i < 6; i++) v->s.drum.minc[i] = pinc(v->s.drum.base + (float)METAL[i] * (1.0f / 16.0f));
    v->s.drum.inc = pinc(v->s.drum.base + v->s.drum.pe * (float)d->bend);
    v->s.drum.gain = lg * gb_db((float)d->qdb * 0.25f);
    v->s.drum.holdn = 0; v->s.drum.held = 0.0f;
    (void)dk;
}
/* the choke: a closed hat (or the pedal) ends the open hat on the same track */
int drum_is_kick(int note) { return (note & 15) < 2; }
int drum_choke(int note) { int l = note & 15; return l == 4 || l == 6; }    /* the closed hat, the pedal */
int drum_choked(int note) { return (note & 15) == 5; }                      /* the open hat */

static void drum_release(gb_voice *v, const gb_patch *p) { (void)v; (void)p; }   /* a drum rings to its end */

GB_INLINE float wsin(uint32_t ph) { return gb_sin_turn((float)ph * (1.0f / 4294967296.0f)); }
GB_INLINE float white(uint32_t *r) { uint32_t x = *r; x ^= x << 13; x ^= x >> 17; x ^= x << 5; *r = x; return (float)(int32_t)x * (0.5f / 2147483648.0f); }

#define SUB 16                                   /* the pitch and the filter's sweep: every 16 samples */
GB_HOT static int synth_render(gb_voice *v, const gb_patch *p, float *out)
{
    const gb_dsnd *d = &v->s.drum.d;
    int wave = d->wave, src = d->src & 15, clap = (d->src & DN_CLAP) != 0, fmode = v->s.drum.fmode & 3, fall = v->s.drum.fmode & 4;
    float drive = d->drive || gb_ev(p, 3) ? (16.0f + (float)(d->drive + gb_ev(p, 3))) * (1.0f / 16.0f) : 0.0f;
    int crush = v->s.drum.crush, bits = crush & 15, hold = (crush >> 4) + 1;
    float q = (float)(1 << bits) * (1.0f / 32768.0f), iq = 1.0f / q;
    float t2r = (float)d->t2 * (1.0f / 32.0f), t2l = (float)d->t2lev * (1.0f / 127.0f), nlev = (float)d->nlev * (1.0f / 127.0f);
    float pe = v->s.drum.pe, kpe = v->s.drum.kpe, amp = v->s.drum.amp, ka = v->s.drum.ka, nz = v->s.drum.nz, kn = v->s.drum.kn;
    float ck = v->s.drum.ck, f1 = v->s.drum.f1, f2 = v->s.drum.f2, hp1 = v->s.drum.hp1, hp2 = v->s.drum.hp2, ahp = v->s.drum.ahp;
    float cprev = v->s.drum.cprev, held = v->s.drum.held, gain = v->s.drum.gain, fk = v->s.drum.fk;
    uint32_t ph = v->s.drum.ph, ph2 = v->s.drum.ph2, ph3 = v->s.drum.ph3, t = v->s.drum.t, hl = v->s.drum.hold, nh = v->s.drum.nhold;
    uint32_t rnd = v->s.drum.rnd, lfsr = v->s.drum.lfsr, cacc = v->s.drum.cacc, cinc = v->s.drum.cinc;
    int holdn = v->s.drum.holdn, bursts = v->s.drum.bursts;
    float kp16 = kpe; for (int i = 0; i < 4; i++) kp16 *= kp16;                 /* (kpe^16: a sub-block's fall) */
    for (int i0 = 0; i0 < GB_BLOCK; i0 += SUB) {
        uint32_t inc = v->s.drum.inc, inc1 = pinc(v->s.drum.base + pe * kp16 * (float)d->bend);
        int32_t di = ((int32_t)(inc1 - inc)) / SUB;
        if (fmode && d->fenv) { v->s.drum.pe = pe; svf_coef(v); }
        float a1 = v->s.drum.a1, a2 = v->s.drum.a2, a3 = v->s.drum.a3;
        for (int i = i0; i < i0 + SUB; i++) {
            float tone = 0.0f, n = 0.0f, x;
            if (wave) {
                switch (wave) {
                case DW_SINE: tone = wsin(ph); break;
                case DW_TRI: { float u = (float)ph * (1.0f / 4294967296.0f); tone = u < 0.5f ? 4.0f * u - 1.0f : 3.0f - 4.0f * u; } break;
                case DW_SQUARE: tone = (int32_t)ph < 0 ? -0.732f : 0.732f; break;
                case DW_FM:                                     /* ratio 1.40625: metallic, the index follows the tone */
                    tone = gb_sin_turn((float)ph * (1.0f / 4294967296.0f) + 0.5f * wsin(ph2) * amp);
                    ph2 += inc + (inc >> 2) + (inc >> 3) + (inc >> 5);
                    break;
                default:                                        /* BELL: two squares, 1 : 1.484 */
                    tone = ((int32_t)ph < 0 ? -0.366f : 0.366f) + ((int32_t)ph2 < 0 ? -0.366f : 0.366f);
                    ph2 += inc + (inc >> 1) - (inc >> 6);
                    break;
                }
                ph += inc;
                tone *= amp;
                if (d->t2) { ph3 += (uint32_t)((float)inc * t2r); tone += wsin(ph3) * amp * amp * t2l; }
            }
            if (src) {
                if (src == DN_WHITE) n = white(&rnd);
                else if (src == DN_CHIP) {
                    uint32_t c0 = cacc; cacc += cinc;
                    if (cacc < c0 || cinc == 0xFFFFFFFFu) { uint32_t b = (lfsr ^ (lfsr >> 1)) & 1u; lfsr = (lfsr >> 1) | (b << 14); }
                    n = (lfsr & 1u) ? 0.61f : -0.61f;
                } else {                                        /* METAL / CYM */
                    uint32_t c = 0;
                    for (int j = 0; j < 6; j++) { v->s.drum.mph[j] += v->s.drum.minc[j]; c += v->s.drum.mph[j] >> 31; }
                    n = (float)c * (9600.0f / 32768.0f) - (28800.0f / 32768.0f);
                    if (src == DN_CYM) n += 0.5f * white(&rnd);
                }
                if (wave == DW_BELL) { n += tone; tone = 0.0f; }
                if (ahp > 0.0f) { hp1 += (n - hp1) * ahp; n -= hp1; hp2 += (n - hp2) * ahp; n -= hp2; }
                n *= nz;
                if (clap) n *= 2.0f;
            }
            if (ck > 1e-5f) { float w = white(&rnd); n += (w - cprev) * ck; cprev = w; ck *= 0.96982f; }
            x = fall ? tone + n : n;
            if (fmode) {
                float v3 = x - f2, v1 = a1 * f1 + a2 * v3, v2 = f2 + a2 * f1 + a3 * v3;
                f1 = gb_clampf(2.0f * v1 - f1, -4.5f, 4.5f); f2 = gb_clampf(2.0f * v2 - f2, -4.5f, 4.5f);
                x = fmode == 1 ? v2 : fmode == 2 ? v1 : x - fk * v1 - v2;
            }
            if (!fall) x += tone;
            if (drive > 0.0f) x = gb_softclip(x * drive);
            if (crush) { if (--holdn <= 0) { holdn = hold; held = (float)(int32_t)(x * iq + (x < 0.0f ? -0.999f : 0.0f)) * q; } x = held; }
            out[i] = x * gain;
            if (hl) hl--; else amp *= ka;
            if (bursts < 4 && t >= 420u * (uint32_t)bursts) {   /* the clap: bursts at 0, 9.5, 19 ms, the 4th is the tail */
                nz = nlev;
                if (++bursts == 4) {
                    int ds = gb_ev(p, 2) + GB_DDEC(p->ddec[v->note & 15]); ds = ds < -4 ? -4 : ds > 3 ? 3 : ds;
                    kn = kdec(d->ndec + DK_IDX[ds + 4]); nh = (uint32_t)((float)d->nhold * 0.002f * GB_SR);
                }
            }
            if (nh) nh--; else nz *= kn;
            t++;
            inc += (uint32_t)di;
        }
        pe *= kp16;
        v->s.drum.inc = inc1;
    }
    v->s.drum.pe = gb_flush(pe); v->s.drum.amp = gb_flush(amp); v->s.drum.nz = gb_flush(nz); v->s.drum.ck = gb_flush(ck);
    v->s.drum.f1 = gb_flush(f1); v->s.drum.f2 = gb_flush(f2); v->s.drum.hp1 = gb_flush(hp1); v->s.drum.hp2 = gb_flush(hp2);
    v->s.drum.cprev = cprev; v->s.drum.held = held; v->s.drum.kn = kn;
    v->s.drum.ph = ph; v->s.drum.ph2 = ph2; v->s.drum.ph3 = ph3; v->s.drum.t = t; v->s.drum.hold = hl; v->s.drum.nhold = nh;
    v->s.drum.rnd = rnd; v->s.drum.lfsr = lfsr; v->s.drum.cacc = cacc; v->s.drum.holdn = (int16_t)holdn; v->s.drum.bursts = (uint8_t)bursts;
    if (bursts < 4) return 1;
    return hl || amp > 2.5e-4f || nz > 2.5e-4f || ck > 2.5e-4f;
}

/* the sampled kit: the zone's ADPCM at its pitch, and the kit's treatment */
GB_HOT static int smp_kit_render(gb_voice *v, float *out)
{
    for (int i = 0; i < GB_BLOCK; i++) out[i] = 0.0f;
    if (v->s.dsmp.ended) return 0;
    if (!gb_flash_ok) return 1;                                  /* (a save: silent, waiting) */
    const smp_zone *z = smp_zone_of(v->s.dsmp.zone);
    const uint8_t *d = smp_zone_data(v->s.dsmp.zone) + z->off;
    int kit = v->s.dsmp.kit, pr = v->s.dsmp.pred, ix = v->s.dsmp.idx, end = 0;
    uint32_t pos = v->s.dsmp.pos;
    float fr = v->s.dsmp.frac, s0 = v->s.dsmp.s0, s1 = v->s.dsmp.s1, st = v->s.dsmp.step, lp = v->s.dsmp.lp, env = v->s.dsmp.env;
    float denv = v->s.dsmp.denv, dkf = v->s.dsmp.dkf, g = v->s.dsmp.gain;
    for (int i = 0; i < GB_BLOCK; i++) {
        if (end) break;
        fr += st;
        while (fr >= 1.0f) {
            fr -= 1.0f; s0 = s1;
            if (pos >= z->n) { end = 1; break; }
            uint32_t b = d[pos >> 1], c = (pos & 1u) ? b >> 4 : b & 15u;
            int step = smp_ima_step[ix], vd = step >> 3;
            if (c & 4u) vd += step;
            if (c & 2u) vd += step >> 1;
            if (c & 1u) vd += step >> 2;
            pr += (c & 8u) ? -vd : vd; pr = pr < -32768 ? -32768 : pr > 32767 ? 32767 : pr;
            ix += smp_ima_idx[c & 7u]; ix = ix < 0 ? 0 : ix > 88 ? 88 : ix;
            s1 = (float)pr * (1.0f / 32768.0f); pos++;
        }
        if (end) break;
        float s = (s0 + (s1 - s0) * fr) * g;
        if (kit == 1) { lp += (s - lp) * 0.25f; s = lp; }                             /* DEEP: darker */
        else if (kit == 4) { lp += (s - lp) * 0.5f; s = (float)(int32_t)(lp * 128.0f + (lp < 0.0f ? -0.999f : 0.0f)) * (1.0f / 128.0f); }   /* DUST */
        else if (kit == 2) { s *= env; env -= env * (1.0f / 2048.0f) + (1.0f / 32768.0f); if (env <= 0.0f) end = 1; }   /* TIGHT */
        if (dkf < 1.0f) { s *= denv; denv *= dkf; if (denv < 2.5e-4f) end = 1; }
        out[i] = s;
    }
    v->s.dsmp.pos = pos; v->s.dsmp.pred = (int16_t)pr; v->s.dsmp.idx = (uint8_t)ix; v->s.dsmp.frac = fr; v->s.dsmp.s0 = s0; v->s.dsmp.s1 = s1;
    v->s.dsmp.lp = gb_flush(lp); v->s.dsmp.env = env; v->s.dsmp.denv = denv; v->s.dsmp.ended = (uint8_t)end;
    return !end;
}

GB_HOT static int drum_render(gb_voice *v, const gb_patch *p, const gb_mod *m, float *out)
{
    (void)m;
    if (v->s.drum.smp == 1) return smp_kit_render(v, out);
    if (v->s.drum.smp == 2) { for (int i = 0; i < GB_BLOCK; i++) out[i] = 0.0f; return 0; }
    return synth_render(v, p, out);
}
const gb_engine gb_eng_drum = { "DRUM", ED, 8, 1, drum_start, drum_render, drum_release, 0, drum_block };
