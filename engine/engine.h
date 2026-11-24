/* SPDX-License-Identifier: GPL-3.0-only
 * The sound: voices, their owners, the engines' interface, the tracks and the mix. Float, block by block. */
#pragma once
#include "../core/gb.h"
#include "drum.h"

#ifndef GB_NVOICE
#define GB_NVOICE 32                /* voices that can sound (phase 0 measures what the FM-1 holds) */
#endif
#define GB_NTAIL 8                  /* extra slots where a stolen voice fades out (G2) */
#define GB_NTRACK 8
#define GB_VIS_RING 512
#define GB_KILL_FRAMES 96           /* a stolen or retriggered voice fades out over ~2.2 ms */

/* ---- who owns a sounding note (G5): only the owner ends it */
enum { OWN_NONE, OWN_KEY, OWN_SEQ, OWN_MIDI, OWN_ARP, OWN_HOLD, OWN_PREVIEW };
typedef uint16_t gb_owner;          /* kind << 8 | sub (the MIDI channel, the sequencer's track, ...) */
#define GB_OWNER(kind, sub) ((gb_owner)(((kind) << 8) | ((sub) & 0xFF)))

/* ---- a smoothed parameter (G3): moves to its target with a time constant, linearly inside each block */
typedef struct { float cur, tgt, k; } gb_smooth;
GB_INLINE void sm_init(gb_smooth *s, float v, float ms)
{
    s->cur = s->tgt = v;
    float blocks = ms * 0.001f * GB_SR / (float)GB_BLOCK;     /* the time constant in blocks */
    s->k = blocks <= 1.0f ? 1.0f : 1.0f / blocks;
}
GB_INLINE void sm_set(gb_smooth *s, float v) { s->tgt = v; }
GB_INLINE void sm_jump(gb_smooth *s, float v) { s->tgt = s->cur = v; }
/* this block's per-frame step; the value at frame i is cur + step * (i + 1); sm_end moves cur to the block's end */
GB_INLINE float sm_step(const gb_smooth *s)
{
    float d = s->tgt - s->cur, next = gb_absf(d) < 1e-7f ? s->tgt : s->cur + d * s->k;
    return (next - s->cur) * (1.0f / (float)GB_BLOCK);
}
GB_INLINE void sm_end(gb_smooth *s, float step) { s->cur += step * (float)GB_BLOCK; if (gb_absf(s->tgt - s->cur) < 1e-7f) s->cur = s->tgt; }

/* ---- an envelope: attack, decay, sustain, release; exact to the sample; KILL is the 2 ms fade of G2 */
enum { ENV_IDLE, ENV_ATT, ENV_DEC, ENV_SUS, ENV_REL };
typedef struct { uint8_t stage; float v, att_inc, dec_k, sus, rel_k; } gb_env;
void env_set(gb_env *e, float att_ms, float dec_ms, float sus, float rel_ms);
GB_INLINE void env_gate(gb_env *e, int on) { if (on) { e->stage = ENV_ATT; } else if (e->stage != ENV_IDLE) e->stage = ENV_REL; }
GB_INLINE float env_tick(gb_env *e)
{
    switch (e->stage) {
    case ENV_ATT: e->v += e->att_inc; if (e->v >= 1.0f) { e->v = 1.0f; e->stage = ENV_DEC; } break;
    case ENV_DEC: e->v = e->sus + (e->v - e->sus) * e->dec_k; if (e->v - e->sus < 1e-4f) { e->v = e->sus; e->stage = ENV_SUS; } break;
    case ENV_SUS: break;
    case ENV_REL: e->v *= e->rel_k; if (e->v < 1e-5f) { e->v = 0.0f; e->stage = ENV_IDLE; } break;
    default: break;
    }
    return e->v;
}

/* ---- the sound of a track: SLOOP's sound model (docs/sloop/SPEC-SON.md). Its values are SLOOP's track parameters
 * that make the sound, each kept normalised 0..1 over its range (gb_sp[], or the engine's own ed[] for E0..E7):
 * a value of SLOOP comes back exactly (gb_pv). Its presets, projects and user sounds load as they are. */
enum {
    SP_E0, SP_E1, SP_E2, SP_E3, SP_E4, SP_E5, SP_E6, SP_E7,       /* the engine's eight (gb_engine.ed) */
    SP_ATK, SP_DEC, SP_SUS, SP_REL,                               /* 0..127: T(v) ms, SUS v/128 */
    SP_EFLT, SP_EPIT, SP_ESHP, SP_TRIM,                           /* ENV > FILTER, PITCH, SHAPE (-64..63); TRIM 1/2 dB */
    SP_LRATE, SP_LWAVE, SP_LPHASE, SP_LFADE, SP_LPIT, SP_LFLT, SP_LSHP, SP_LAMP,   /* the track's LFO */
    SP_LSYNC, SP_LTRIG, SP_LPOL, SP_ESYNC,
    SP_VOICE, SP_GLIDE, SP_GLMODE, SP_PRIO, SP_ALLOC, SP_DETUNE,  /* POLY MONO LEGATO UNISON; GLIDE RATE / TIME */
    SP_FTYPE, SP_DIST,                                            /* ANALOG's filter LP BP HP; the track's DIST */
    SP_ITYPE, SP_IA, SP_IB, SP_IC, SP_IMIX,                       /* the track's INSERT */
    SP_SLCR, SP_SLPAT, SP_SLRATE, SP_SLDEPTH,                     /* the track's SLICER */
    SP_N
};
#define GB_NPARAM 48                /* SP_N and room */
_Static_assert(SP_N <= GB_NPARAM, "the sound's parameters fit");
typedef struct { int16_t min, max, def; } gb_pd;   /* a parameter's range and default, in SLOOP's units */
extern const gb_pd gb_sp[SP_N];                    /* (sound.c; E0..E7 there: 0..127, the engine's ed[] are used) */
enum { VM_POLY, VM_MONO, VM_LEGATO, VM_UNISON };
enum { PR_LAST, PR_LOW, PR_HIGH };
/* a value in its units, and back */
GB_INLINE int gb_iv(float x, int lo, int hi) { int v = lo + (int)(x * (float)(hi - lo) + 0.5f); return v < lo ? lo : v > hi ? hi : v; }
GB_INLINE float gb_nv(int v, int lo, int hi) { return hi > lo ? (float)(v - lo) / (float)(hi - lo) : 0.0f; }
#define gb_pv(q, k) gb_iv((q)[k], gb_sp[k].min, gb_sp[k].max)
GB_INLINE int gb_vmode(const float *p) { return gb_pv(p, SP_VOICE); }
GB_INLINE int gb_prio(const float *p) { return gb_pv(p, SP_PRIO); }
/* the curves (SPEC-SON): a value 0..127 to its unit */
GB_INLINE float gb_time_ms(float v) { return gb_exp2f(v * (13.287712f / 127.0f)); }        /* 10000^(v/127) */
GB_INLINE float gb_lfo_hz(float v) { return 0.05f * gb_exp2f(v * (9.6438562f / 127.0f)); } /* 0.05 x 800^(v/127) */
GB_INLINE float gb_cut_hz(float v) { return 30.0f * gb_exp2f(v * (9.0588936f / 127.0f)); } /* 30 x 533^(v/127) */
typedef struct {
    uint8_t engine;                 /* index in gb_engines[] */
    uint8_t poly;                   /* the most voices this track takes (POLY) */
    uint8_t unison;                 /* (kept for the projects' bytes; UNISON is a voice mode) */
    uint8_t protect;                /* stolen last (a kick, a bass) (G4) */
    float gain;                     /* the sound's own level (its preset's: all presets as loud) */
    int8_t dtune[16], ddec[16];     /* DRUM: each sound's tune (semitones, -8 .. +7) and decay (-4 .. +3 steps): GB_DDEC,
                                     * and (3.4.2, issue #25) its level in the bits above (gb_dlev, dB) */
    uint16_t dmute;                 /* DRUM: the sounds the sequencer does not play (bit = sound) */
    float p[GB_NPARAM];             /* the sound's parameters, 0..1 (SP_*) */
} gb_patch;
/* (3.4.2, issue #25) a drum sound's decay and level, in its ddec byte: the decay in the low 3 bits (-4 .. +3, as
 * before: a project from before reads the same), the level (dB, GB_DLEV_MIN .. GB_DLEV_MAX) above, 0 in an old byte */
#define GB_DDEC(b) ((int)(int8_t)((uint8_t)((uint8_t)(b) << 5)) >> 5)
#define GB_DLEV_MIN (-12)
#define GB_DLEV_MAX 6
static inline int gb_dlev(int8_t b) { int x = ((uint8_t)b >> 3) ^ (GB_DDEC(b) < 0 ? 31 : 0); return x & 16 ? x - 32 : x; }
static inline int8_t gb_dpack(int dec, int lev)
{
    dec = dec < -4 ? -4 : dec > 3 ? 3 : dec; lev = lev < GB_DLEV_MIN ? GB_DLEV_MIN : lev > GB_DLEV_MAX ? GB_DLEV_MAX : lev;
    return (int8_t)(uint8_t)((dec & 7) | (((lev ^ (dec < 0 ? -1 : 0)) & 31) << 3));
}
GB_INLINE int gb_ev(const gb_patch *p, int k);     /* an engine value E0..E7 in its units (after gb_engines) */
void gb_patch_default(gb_patch *p, int engine);    /* every value its default (the engine's for E0..E7) */
void gb_patch_set(gb_patch *p, int k, int v);      /* a value in its units */

/* ---- a voice */
enum { V_FREE, V_ON, V_REL, V_KILL };
enum { EG_IDLE, EG_ATT, EG_DEC, EG_REL };
typedef struct gb_voice {
    uint8_t state, track, note, uni;    /* uni: index inside a unison stack */
    uint8_t eng;                        /* the engine that started it (its state is that engine's, whatever comes) */
    uint8_t eg;                         /* the amplitude envelope's stage (EG_*) */
    uint8_t others;                     /* at its start: another voice of the track was held (WHEEL's single trigger) */
    uint8_t flat;                       /* the engine's own volume this block (LOFI STEP): the ADSR only gates it,
                                         * held at full, its release ending the voice at 1 % */
    gb_owner owner;
    uint32_t age;                       /* the order of starts: older = smaller */
    float vel, pan_l, pan_r, level;     /* level: the last block's loudness estimate (for stealing) */
    float env;                          /* the amplitude envelope, 0..1 */
    float penv;                         /* the pitch envelope (ENV > PIT): 1 at the note's start, falling (~40 ms) */
    float pitch, pitch_tgt, gstep;      /* semitones; the glide: gstep a block (0: there at once) */
    float fine;                         /* UNISON: this voice's detune, semitones */
    int16_t kill;                       /* frames left in the KILL fade */
    uint32_t seed;
    union {                             /* the engine's own state */
        struct { float ph[2], ic1, ic2; uint32_t nz; } va;
        struct {                        /* DRUM, a synthesised kit's sound (drum.h) */
            uint8_t smp, crush, bursts, fmode;
            gb_dsnd d;
            int16_t holdn;
            uint32_t ph, ph2, ph3, inc, mph[6], minc[6], t, hold, nhold, rnd, lfsr, cacc, cinc;
            float pe, kpe, amp, ka, nz, kn, ck, base, cut, f1, f2, fk, a1, a2, a3, hp1, hp2, ahp, cprev, held, gain;
        } drum;
        struct {                        /* DRUM, a sampled kit's sound */
            uint8_t smp, kit, idx, ended;
            uint16_t zone; int16_t pred;
            uint32_t pos;
            float frac, s0, s1, step, lp, env, denv, dkf, gain;
        } dsmp;
        struct { uint32_t ph[4]; float fb[2], menv, mdk; } fm;
        struct { uint32_t ph, lfsr, step, t; float hold, sweep, lp, vib, vamp; uint8_t arp; } lofi;
        struct { uint32_t ph[3]; float dcw, edec; uint8_t w; } pd;
        struct { uint32_t ph[9], nz; float gp[9], perc, clk, hp; uint8_t gate; } wheel;
        struct { float ic1, ic2, b0, b1, b2, br, drift, dtg, dc, held; uint32_t ph, reg, rng; int16_t dcnt; uint8_t hold; } noise;
        struct { uint32_t ph; float y[8], lp, talk, rv; } voice;
        struct { uint32_t ph[3], nst; float ic1, ic2, nz, prev, drift; } trio;
        struct { uint32_t tok; uint8_t slot, trig; } phys;
        struct { uint32_t pos; float frac, s0, s1, lp; int16_t pred; uint16_t zone; uint8_t idx, ended, primed; } smp;
        struct { uint32_t tag; float cd, lp; uint16_t zone; } grain;
        struct {                        /* SLICE (slice.c): a slice of the material, forwards or backwards */
            uint32_t pos, a, b, tgt, ws;    /* the decoder's position; the slice [a, b); where the seek goes; the window */
            float frac, s0, s1, lp, dec, ramp;
            int16_t pred, apred;            /* the decoder; its state at a (LOOP) */
            uint8_t idx, aidx, src, phase, rev, mode;
            uint16_t zone;
            uint32_t ck[20];                /* backwards: the decoder's states at a + k (b - a) / 20 (predictor | index << 16;
                                             * 0xFFFFFFFF: not yet) */
            int16_t rb[40];                 /* backwards: the window, decoded forwards, read from its top */
        } slc;
        uint8_t raw[232];
    } s;
} gb_voice;

/* ---- what the voice layer gives an engine for a block (sound.c, SPEC-SON): the pitch with its modulations, the
 * amplitude envelope at the block's two ends (a modulation source), the cutoff's and SHAPE's offsets in steps of
 * their 0..127 curves */
typedef struct {
    float pitch;                        /* semitones (MIDI note), glide, LFO, ENV > PIT, bend, unison detune */
    float dt;                           /* its frequency / the sample rate */
    float env0, env1;                   /* the amplitude envelope (0..1) at the block's start and end */
    float cut0, cut;                    /* LFO > FLT + ENV > FLT (+ the accent's), steps of the cutoff curve: at the
                                         * block's start and end */
    float shape;                        /* 64 + LFO > SHP + ENV > SHP, steps 0..127 */
    float lfo;                          /* the track's LFO, -1..1 (or 0..1, POL UNI) with its fade */
} gb_mod;

/* ---- an engine: start (fresh: from silence; else a voice that sounds, played again: phases and filters kept),
 * render one block of the raw sound into out (the voice layer applies the amplitude), release (optional) */
typedef struct {
    const char *name;
    const gb_pd *ed;                    /* E0..E7: range and default, in SLOOP's units */
    uint8_t poly;                       /* the voices it allows a track (POLY, UNISON); 0 = 8 */
    uint8_t ownenv;                     /* 1: its own envelopes (render's return ends the voice; no ADSR); 2: the ADSR, but a
                                         * note played again starts from silence (a sample: the old one in a tail) */
    void (*start)(gb_voice *v, const gb_patch *p, int fresh);
    int (*render)(gb_voice *v, const gb_patch *p, const gb_mod *m, float *out);
    void (*release)(gb_voice *v, const gb_patch *p);
    void (*spread)(gb_voice *v, uint32_t r);    /* UNISON: start phases moved by r (a turn = 2^32) */
    void (*block)(const gb_patch *p, int t, float beat);   /* once a block for each track it plays (one core) */
} gb_engine;
extern const gb_engine *const gb_engines[];
extern const int gb_nengines;
extern uint32_t gb_blocks;                          /* blocks rendered (sound.c): the engines' own slots age by it */
enum { ENG_VA, ENG_DRUM, ENG_FM, ENG_LOFI, ENG_PD, ENG_WHEEL, ENG_NOISE, ENG_VOICE, ENG_TRIO, ENG_PHYS, ENG_FM6, ENG_SAMPLE, ENG_GRAIN, ENG_SLICE, ENG_N };
#define ENG_ANALOG ENG_VA
#define ENG_DIGITAL ENG_FM
#define ENG_PHASE ENG_PD
GB_INLINE int gb_ev(const gb_patch *p, int k) { const gb_pd *d = &gb_engines[p->engine]->ed[k]; return gb_iv(p->p[k], d->min, d->max); }
extern int16_t gb_sine[1025];                       /* one turn, +-32767, and its first value again */
void gb_tables_init(void);
GB_INLINE float gb_sin_u32(uint32_t ph)             /* a sine from a 32-bit phase: the table, interpolated */
{
    uint32_t i = ph >> 22; float f = (float)((ph >> 6) & 0xFFFFu) * (1.0f / 65536.0f);
    float a = (float)gb_sine[i]; return (a + ((float)gb_sine[i + 1] - a) * f) * (1.0f / 32767.0f);
}
GB_INLINE uint32_t gb_inc_u32(float hz) { float x = hz * GB_ISR; x = x < 0.0f ? 0.0f : x > 0.49f ? 0.49f : x; return (uint32_t)(x * 4294967296.0f); }
GB_INLINE int gb_choice(float p, int n) { int i = (int)(p * (float)n * 0.9999f); return i < 0 ? 0 : i >= n ? n - 1 : i; }
/* the trapezoidal state-variable filter (Simper): coefficients from a cutoff in Hz and the damping k (2 = flat) */
typedef struct { float a1, a2, a3, k; } gb_svf;
void gb_svf_set(gb_svf *f, float hz, float k);
/* tan(pi x) for 0 <= x < 0.5, from the quarter-wave sine */
GB_INLINE float gb_tan_pi(float x) { float c = gb_sin_turn(0.5f * x + 0.25f); return gb_sin_turn(0.5f * x) / (c > 1e-4f ? c : 1e-4f); }
/* polyBLEP: the residual that removes a step's aliasing; t the phase (0..1), dt the increment */
GB_INLINE float gb_blep(float t, float dt)
{
    if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
    if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
    return 0.0f;
}
GB_INLINE float gb_softclip(float x) { x = x > 1.5f ? 1.5f : x < -1.5f ? -1.5f : x; return x - x * x * x * (4.0f / 27.0f); }   /* (flat at +-1 past 1.5) */

/* ---- a track's strip in the mix */
#define GB_MONO_HELD 8
typedef struct {
    gb_patch patch;
    gb_smooth level, pan, send_dly, send_rev, send_cho;
    gb_smooth filt;                         /* its one-knob filter: -1 low-pass .. 0 off .. 1 high-pass */
    float filt_st[4];
    uint8_t mute;
    float bend;                             /* the pitch bend, semitones (MIDI: +-2), on every voice of the track */
    /* MONO / LEGATO: the notes held, oldest first, and the one sounding (-1 none) */
    uint8_t mono_n; uint8_t mono_note[GB_MONO_HELD]; gb_owner mono_own[GB_MONO_HELD]; float mono_vel[GB_MONO_HELD];
    int16_t mono_cur;
    /* the sound's track-wide parts (sound.c, SPEC-SON), made once a block before the voices */
    uint32_t lfo_ph;                        /* the LFO: its phase (a turn = 2^32), value (with its fade), S&H, fade */
    float lfo, lfo_rnd, lfo_fade;
    float att_inc, dec_k, rel_k, sus;       /* the envelope's steps this block (a sample's) */
    float last_pitch;                       /* POLY glide: from the last note played */
    uint8_t slide;                          /* the sequencer's slide into the next note (a glide without GLIDE) */
    float dist_st[4];                       /* DIST: its low cut, both sides */
    uint8_t swing;                          /* the sequencer's swing on this track, its own + the global (0..100: the SLICER) */
    uint8_t fx_tail;                        /* blocks the INSERT runs on after the last voice (a flanger's feedback) */
    float meter;                            /* its peak after its level, falling (the web editor's meters) */
} gb_track;

/* ---- the punch-in effects (master.c): one at a time, on the whole mix, while its key is held */
enum { PK_LOOP4, PK_LOOP8, PK_LOOP16, PK_LOOP32, PK_STUTTER, PK_REVERSE, PK_TAPESTOP, PK_HALF,
       PK_LPSWEEP, PK_HPSWEEP, PK_PHONE, PK_CRUSH, PK_ALIAS, PK_GATE, PK_ECHO, PK_WOBBLE,       /* the white keys */
       PK_KILLLOW, PK_KILLMID, PK_KILLHIGH, PK_REWIND, PK_OCTUP, PK_FLANGER, PK_AUTOPAN, PK_FREEZE,
       PK_LOOP8T, PK_MONO, PK_FADEOUT, PK_N };                                                   /* the black keys */
typedef struct {
    int8_t fx, want;                        /* the effect running, the effect held (-1 none) */
    uint8_t stage, lend;                    /* lend: the delay's memory borrowed (1 asked, 2 ours) */
    float mix;                              /* how much of it is heard, 0..1 (the fade) */
    uint32_t t, wait, len, w, k;            /* frames since it began; before its capture; the loop; written; chunk */
    float r, prev_pitch;                    /* a read position */
    float st[16];                           /* filters */
    float g, hold[2], ph;
    float saved_time;                       /* (ECHO: the delay's time before it) */
} gb_punch;

/* ---- the whole sound */
typedef struct gb_sound {
    gb_voice v[GB_NVOICE + GB_NTAIL];
    gb_track trk[GB_NTRACK];
    uint32_t next_age;
    gb_smooth master;
    /* the effects (fx.c): lines in 16 bits (OpenPocket's lesson: RAM is the scarce resource) */
    int16_t *dly; uint32_t dly_len, dly_pos;           /* stereo interleaved, dly_len frames */
    gb_smooth dly_time;                                /* in frames */
    float dly_fb, dly_damp, dly_lp[2];
    int16_t *rev_line[8]; uint32_t rev_n[8], rev_pos[8];
    float rev_lp[8], rev_fb, rev_damp;
    /* the limiter */
    float lim_gain, lim_env;
    float mix_gain;                 /* (3.4.2, issue #16) the mix before the limiter: 1 (snd_init), the FM-1's 2 (fw/main.c) */
    uint8_t lowcut;                                    /* HOME > AUDIO SPEAKER: the speaker's low cut (12 dB/oct ~110 Hz), the drums' lows carried up;
                                                        * 2 (4.0.2): the drums' only, the firmware cuts the DAC's out itself (the USB audio stays whole) */
    float lc[4];
    float flt_q;                                       /* HOME > AUDIO > FILTER RESO: the tracks' filters' Q */
    uint8_t usb_full;                                  /* HOME > AUDIO > USB FULL: usb_l / usb_r the mix before MASTER */
    uint8_t tx[7];                                     /* TEXTURE (texture.c): SRC LEVEL TONE (int8) MOVE RATE WHEN DUCK */
    float usb_l[GB_BLOCK], usb_r[GB_BLOCK];
    /* the visualiser (app/vis.h): the mix before MASTER at half the rate, 8 bits, into vis_ring (GB_VIS_RING left, then right;
     * 0: none); the notes started on each track (a counter), the last one's velocity and pitch; the kicks */
    int8_t *vis_ring; uint16_t vis_w; uint8_t vis_kick, vis_hit[GB_NTRACK], vis_vel[GB_NTRACK], vis_note[GB_NTRACK];
    /* the master bus (master.c): DUST, DUCK (the synth tracks pumped by the kick), the DJ filter, the punch-ins */
    float dust, duck;                                  /* 0..1 */
    gb_smooth mfilt;                                   /* -1 low-pass .. 0 off .. 1 high-pass */
    float mfilt_st[4];
    float duck_pos;                                    /* frames since the last kick */
    float beat;                                        /* frames a beat (the firmware: from the tempo) */
    float grid;                                        /* the transport's position at this block, frames; < 0 stopped */
    uint32_t bar;                                      /* .. its bar (the SLICER's steps) */
    uint8_t playing, freeze, pk_fade;                  /* pk_fade: the punch-ins' fade, 0 (an anti-click) .. 15 */
    gb_smooth throw_dly, throw_rev;                    /* the throws: every send towards full while > 0 */
    float du_hold[2], du_ph, du_lp[2], du_click;
    uint32_t du_seed;
    gb_punch pk;
    int16_t *hist; uint32_t hist_pos; uint8_t hist_odd, hist_stop;   /* the mix's past, mono, half rate (REWIND) */
    int16_t *modl; uint32_t mod_pos;                   /* a short stereo line (FLANGER, WOBBLE, OCT UP) */
    int16_t *cho; uint32_t cho_pos; float cho_ph;      /* the chorus (a send): its line, its LFO */
    float cho_depth;                                   /* 0..1 */
    float cho_rate, dly_mix;                           /* the chorus's rate (Hz); the echoes' level (1: as they come) */
    /* counters (shown in SYSTEM) */
    uint32_t steals, tails_cut, voices_peak;
    /* the load guard: at most voice_cap voices sound (a new note beyond steals the least audible); 0 = all */
    int voice_cap;
    uint32_t shed;                                     /* voices the guard faded out */
    uint32_t panics; uint16_t panic_quiet;             /* 3.3.3: snd_panic's count; blocks before its lines are cleared again */
} gb_sound;

#define GB_DLY_FRAMES 32768          /* 0.74 s of stereo delay: 128 KB */
#define GB_REV_SAMPLES 13600         /* the reverb's eight lines (fx.c REV_N, added up): 27 KB */
#define GB_REV_LOW 7243              /* of them, lines 1..5: in the master bus's memory when there is one */
#define GB_FX_MEM (GB_DLY_FRAMES * 2 + GB_REV_SAMPLES)     /* int16 values the caller gives snd_init (with a master
                                                               bus's memory, GB_FX_MEM - GB_REV_LOW are used) */
#define GB_HIST 8192                 /* the past, for REWIND: 0.37 s, mono, at half the rate (16 KB) */
#define GB_MODL 1024                 /* the short line: 23 ms stereo (4 KB) */
#define GB_CHO 2048                  /* the chorus's line: 46 ms, mono in (4 KB) */
#define GB_PUNCH_MEM (GB_HIST + 2 * GB_MODL + GB_CHO + GB_REV_LOW)   /* int16 values for the master bus (snd_init) */
void snd_init(gb_sound *s, int16_t *fx_mem, int16_t *punch_mem);
/* the punch-ins: fx < 0 lets go of the one held */
void snd_punch(gb_sound *s, int fx);
/* events, from the sequencer, the keys, MIDI: the owner rules (G5) */
void snd_note_on(gb_sound *s, gb_owner o, int track, int note, float vel);
void snd_note_off(gb_sound *s, gb_owner o, int track, int note);
void snd_slide(gb_sound *s, int track, int on);      /* the sequencer's slide: the next note glides (no GLIDE: ~50 ms) */
void snd_owner_off(gb_sound *s, gb_owner o);         /* every note of an owner: panic, a lost MIDI message */
void snd_all_off(gb_sound *s);
void snd_panic(gb_sound *s);            /* a value gone wrong: voices stopped, every effect's line and filter cleared (3.3.2) */
void snd_kill(gb_sound *s);                        /* every voice silent at once, the grains dropped */
void grain_forget(void);
/* the load guard (never a crackle, G14): fade out the least audible voice (released ones first, protected tracks
 * last); 1 if one was faded. With voice_cap, new notes stay within the budget the guard found. */
int snd_shed(gb_sound *s);
int snd_voices(const gb_sound *s);                    /* voices sounding (not tails) */
/* one block: interleaved int32 stereo out, 24-bit values, rounded to nearest (G1) */
void snd_render(gb_sound *s, int32_t *out);
/* the block in two halves, for two cores (sound.c): each core renders its part of the voices into its own bus,
 * then one core mixes both and makes the output */
typedef struct { float l[GB_NTRACK][GB_BLOCK], r[GB_NTRACK][GB_BLOCK]; uint32_t live; float pk[GB_NTRACK]; } gb_bus;   /* pk: the voices' peaks, added up (3.1.8) */
/* a track's SLICER and INSERT (trackfx.c) */
int tfx_slicer(gb_sound *s, int t, float *l, float *r), tfx_insert(gb_sound *s, int t, float *l, float *r);
int tfx_busy(const gb_sound *s, int t);
uint32_t tfx_slicer_view(int t, int pat);
void tfx_reset(void);
void texture_mix(gb_sound *s, float *ml, float *mr, float duck0, float duck1);
void texture_reset(void);
void snd_render_voices(gb_sound *s, gb_bus *b, int part, int nparts);
void snd_mix(gb_sound *s, gb_bus *b, const gb_bus *other, int32_t *out);
void master_init(gb_sound *s, int16_t *mem);
void master_pre(gb_sound *s, const float *ml, const float *mr, float *dl, float *dr, float *rl, float *rr, float *dry_l, float *dry_r);
void master_post(gb_sound *s, float *ml, float *mr, const float *dry_l, const float *dry_r);
void chorus_block(gb_sound *s, const float *in, float *ml, float *mr);
void gb_svf_dj(float *l, float *r, float x, float *st, float q);   /* a one-knob filter, in place: -1 LP .. 0 off .. 1 HP */
int drum_is_kick(int note);
/* the float master of the last block, before the conversion (tests read it) */
#ifndef __PI32V2__
extern float snd_last_l[GB_BLOCK], snd_last_r[GB_BLOCK];
#endif
