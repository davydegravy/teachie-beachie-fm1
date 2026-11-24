/* SPDX-License-Identifier: GPL-3.0-only
 * Voices and their owners, stealing with tails, the voice layer of SLOOP's sound model (docs/sloop/SPEC-SON.md: the
 * envelope, the pitch envelope, the track's LFO, the voice modes, the glide), the track strips, the master and the
 * output. Written for this project.
 *   G1 float mix, one rounding at the output    G2 a stolen voice fades in a tail slot
 *   G3 every gain smoothed                      G4 stealing by audibility, protected tracks last
 *   G5 only a note's owner ends it */
#include "engine.h"

#ifndef __PI32V2__
float snd_last_l[GB_BLOCK], snd_last_r[GB_BLOCK];        /* (the PC tests read the last block's mix) */
#endif
void fx_init(gb_sound *s, int16_t *mem, int16_t *rev_low);
int drum_choke(int note), drum_choked(int note);
void fx_render(gb_sound *s, const float *dl, const float *dr, const float *rl, const float *rr, float *ol, float *or_);

/* ---------------------------------------------------------------- the sound's parameters (SLOOP's ranges) */
const gb_pd gb_sp[SP_N] = {
    [SP_E0] = { 0, 127, 0 }, [SP_E1] = { 0, 127, 0 }, [SP_E2] = { 0, 127, 0 }, [SP_E3] = { 0, 127, 0 },
    [SP_E4] = { 0, 127, 0 }, [SP_E5] = { 0, 127, 0 }, [SP_E6] = { 0, 127, 0 }, [SP_E7] = { 0, 127, 0 },
    [SP_ATK] = { 0, 127, 10 }, [SP_DEC] = { 0, 127, 70 }, [SP_SUS] = { 0, 127, 90 }, [SP_REL] = { 0, 127, 60 },
    [SP_EFLT] = { -64, 63, 0 }, [SP_EPIT] = { -64, 63, 0 }, [SP_ESHP] = { -64, 63, 0 }, [SP_TRIM] = { -64, 63, 0 },
    [SP_LRATE] = { 0, 127, 60 }, [SP_LWAVE] = { 0, 4, 0 }, [SP_LPHASE] = { 0, 127, 0 }, [SP_LFADE] = { 0, 127, 0 },
    [SP_LPIT] = { -64, 63, 0 }, [SP_LFLT] = { -64, 63, 0 }, [SP_LSHP] = { -64, 63, 0 }, [SP_LAMP] = { 0, 127, 0 },
    [SP_LSYNC] = { 0, 10, 0 }, [SP_LTRIG] = { 0, 1, 0 }, [SP_LPOL] = { 0, 1, 0 }, [SP_ESYNC] = { 0, 1, 0 },
    [SP_VOICE] = { 0, 3, 0 }, [SP_GLIDE] = { 0, 127, 0 }, [SP_GLMODE] = { 0, 1, 0 }, [SP_PRIO] = { 0, 2, 0 },
    [SP_ALLOC] = { 0, 1, 0 }, [SP_DETUNE] = { 0, 127, 40 }, [SP_FTYPE] = { 0, 2, 0 }, [SP_DIST] = { 0, 127, 0 },
    [SP_ITYPE] = { 0, 8, 0 }, [SP_IA] = { 0, 127, 64 }, [SP_IB] = { 0, 127, 96 }, [SP_IC] = { 0, 127, 96 },
    [SP_IMIX] = { 0, 127, 127 }, [SP_SLCR] = { 0, 2, 0 }, [SP_SLPAT] = { 1, 16, 1 }, [SP_SLRATE] = { 0, 5, 1 },
    [SP_SLDEPTH] = { 0, 127, 127 },
};
static const gb_pd *pd_of(const gb_patch *p, int k)
{
    return k < 8 && p->engine < gb_nengines ? &gb_engines[p->engine]->ed[k] : &gb_sp[k];
}
void gb_patch_set(gb_patch *p, int k, int v)
{
    if (k < 0 || k >= SP_N) return;
    const gb_pd *d = pd_of(p, k);
    v = v < d->min ? d->min : v > d->max ? d->max : v;
    p->p[k] = gb_nv(v, d->min, d->max);
}
GB_SMALL void gb_patch_default(gb_patch *p, int engine)
{
    p->engine = (uint8_t)(engine >= 0 && engine < gb_nengines ? engine : 0);
    const gb_engine *e = gb_engines[p->engine];
    p->poly = e->poly ? e->poly : 8; p->unison = 1; p->protect = 0; p->gain = 1.0f;
    for (int k = 0; k < GB_NPARAM; k++) p->p[k] = 0.0f;
    for (int k = 0; k < SP_N; k++) gb_patch_set(p, k, pd_of(p, k)->def);
}

/* ---------------------------------------------------------------- envelopes (gb_env: the engines' own) */
void env_set(gb_env *e, float att_ms, float dec_ms, float sus, float rel_ms)
{
    float att_n = gb_maxf(att_ms, 0.3f) * 0.001f * GB_SR;          /* 0.3 ms at least: no click at the start */
    e->att_inc = 1.0f / att_n;
    /* exponential segments: the level falls to 1/1000 (-60 dB) in the time given */
    e->dec_k = gb_exp2f(-9.9658f / (gb_maxf(dec_ms, 1.0f) * 0.001f * GB_SR));
    e->rel_k = gb_exp2f(-9.9658f / (gb_maxf(rel_ms, 1.0f) * 0.001f * GB_SR));
    e->sus = gb_clampf(sus, 0.0f, 1.0f);
}

/* ---------------------------------------------------------------- the tracks' parts, once a block (SPEC-SON) */
static const uint16_t ESYNC_U[25] = { 0, 4, 6, 8, 9, 12, 16, 18, 24, 32, 36, 48, 64, 72, 96, 128, 144, 192, 256, 288,
                                      384, 576, 768, 1152, 1536 };   /* ENV SYNC: note values in 1/96 of a beat */
static const uint16_t LSYNC_P[11] = { 0, 384, 192, 96, 48, 24, 12, 8, 6, 4, 3 };   /* LFO SYNC: a cycle in 1/24 beat */
static float seg_frames(const gb_sound *s, const float *q, int k)  /* a segment's time in frames (ATK DEC REL) */
{
    int v = gb_pv(q, k);
    if (gb_pv(q, SP_ESYNC)) {
        float beat = s->beat > 1.0f ? s->beat : 22050.0f;
        return beat * (float)ESYNC_U[v * 25 >> 7] * (1.0f / 96.0f);
    }
    return gb_time_ms((float)v) * (0.001f * GB_SR);
}
/* 99 % of the way in n frames: the factor a frame */
static float seg_k(float n) { return n < 1.0f ? 0.0f : gb_exp2f(-6.6365f / n); }   /* e^(-4.6 / n) */
GB_SMALL static void tracks_tick(gb_sound *s)
{
    for (int t = 0; t < GB_NTRACK; t++) {
        gb_track *k = &s->trk[t];
        const float *q = k->patch.p;
        float na = seg_frames(s, q, SP_ATK);
        k->att_inc = na < 1.0f ? 1.0f : 1.0f / na;
        k->dec_k = seg_k(seg_frames(s, q, SP_DEC));
        k->rel_k = seg_k(seg_frames(s, q, SP_REL));
        k->sus = (float)gb_pv(q, SP_SUS) * (1.0f / 128.0f);
        /* the LFO: a block's step of its phase, S&H at each new cycle, its fade, its polarity */
        int sy = gb_pv(q, SP_LSYNC);
        float turns = sy && s->beat > 1.0f ? (float)GB_BLOCK / (s->beat * (float)LSYNC_P[sy] * (1.0f / 24.0f))
                                           : gb_lfo_hz((float)gb_pv(q, SP_LRATE)) * (float)GB_BLOCK * GB_ISR;
        uint32_t old = k->lfo_ph;
        k->lfo_ph += (uint32_t)(turns * 4294967296.0f);
        if (k->lfo_ph < old) { uint32_t r = k->lfo_ph * 1664525u + 1013904223u + (uint32_t)t; r ^= r >> 15; k->lfo_rnd = (float)(int32_t)(r * 0x2C1B3C6Du) * (1.0f / 2147483648.0f); }
        float ph = (float)k->lfo_ph * (1.0f / 4294967296.0f), l;
        switch (gb_pv(q, SP_LWAVE)) {
        case 1: l = ph < 0.5f ? 4.0f * ph - 1.0f : 3.0f - 4.0f * ph; break;
        case 2: l = 2.0f * ph - 1.0f; break;
        case 3: l = ph < 0.5f ? 1.0f : -1.0f; break;
        case 4: l = k->lfo_rnd; break;
        default: l = gb_sin_turn(ph); break;
        }
        if (gb_pv(q, SP_LPOL)) l = 0.5f * (l + 1.0f);
        if (k->lfo_fade < 1.0f) {
            int f = gb_pv(q, SP_LFADE);
            k->lfo_fade = f ? gb_minf(1.0f, k->lfo_fade + (float)GB_BLOCK / (gb_time_ms((float)f) * 0.001f * GB_SR)) : 1.0f;
        }
        k->lfo = l * k->lfo_fade;
        const gb_engine *e = gb_engines[k->patch.engine];
        if (e->block) e->block(&k->patch, t, s->beat);
    }
}

/* ---------------------------------------------------------------- init */
GB_SMALL void snd_init(gb_sound *s, int16_t *fx_mem, int16_t *punch_mem)
{
    memset(s, 0, sizeof *s);
    tfx_reset(); texture_reset();
    drum_syn_default();
    for (int t = 0; t < GB_NTRACK; t++) {
        gb_track *k = &s->trk[t];
        sm_init(&k->level, 0.7f, 20.0f);
        sm_init(&k->pan, 0.5f, 20.0f);
        sm_init(&k->send_dly, 0.0f, 20.0f);
        sm_init(&k->send_rev, 0.0f, 20.0f);
        sm_init(&k->send_cho, 0.0f, 20.0f);
        sm_init(&k->filt, 0.0f, 40.0f);
        k->mono_cur = -1; k->lfo_fade = 1.0f; k->last_pitch = -1.0f;
        gb_patch_default(&k->patch, ENG_VA);
    }
    sm_init(&s->master, 0.8f, 30.0f);
    s->lim_gain = 1.0f; s->mix_gain = 1.0f;
    master_init(s, punch_mem);
    fx_init(s, fx_mem, punch_mem ? punch_mem + GB_HIST + 2 * GB_MODL + GB_CHO : 0);   /* (after: master_init clears its memory) */
    gb_tables_init();
    tracks_tick(s);
}

/* ---------------------------------------------------------------- voices */
static int track_voices(const gb_sound *s, int t)
{
    int n = 0;
    for (int i = 0; i < GB_NVOICE; i++) if (s->v[i].state != V_FREE && s->v[i].track == t) n++;
    return n;
}
int snd_voices(const gb_sound *s)
{
    int n = 0;
    for (int i = 0; i < GB_NVOICE; i++) n += s->v[i].state != V_FREE;
    return n;
}
/* move the voice in slot i into a tail slot, fading over GB_KILL_FRAMES; slot i is then free (G2) */
static void to_tail(gb_sound *s, int i)
{
    gb_voice *v = &s->v[i];
    if (v->state == V_FREE) return;
    int best = -1, left = 1 << 30;
    for (int j = GB_NVOICE; j < GB_NVOICE + GB_NTAIL; j++) {
        if (s->v[j].state == V_FREE) { best = j; break; }
        if (s->v[j].kill < left) { left = s->v[j].kill; best = j; }      /* the tail nearest its end */
    }
    if (s->v[best].state != V_FREE) s->tails_cut++;                       /* all tails busy: the nearest ends now */
    s->v[best] = *v;
    if (s->v[best].state != V_KILL) { s->v[best].state = V_KILL; s->v[best].kill = GB_KILL_FRAMES; }
    v->state = V_FREE;
}
/* the slot to give a new voice of track t: a free one, else the least audible (G4) */
GB_SMALL static int pick_slot(gb_sound *s, int t, int note)
{
    int used = 0, free_slot = -1;
    for (int i = 0; i < GB_NVOICE; i++) { if (s->v[i].state == V_FREE) { if (free_slot < 0) free_slot = i; } else used++; }
    if (free_slot >= 0 && (s->voice_cap <= 0 || used < s->voice_cap)) return free_slot;
    int best = 0; float best_score = 1e30f;
    for (int i = 0; i < GB_NVOICE; i++) {
        const gb_voice *v = &s->v[i];
        if (v->state == V_FREE) continue;
        float score = v->level - 1e-9f * (float)(s->next_age - v->age);  /* quieter first, then older */
        if (v->state == V_REL) score *= 0.25f;                       /* a released voice goes first */
        if (v->track == t && v->note == note) score *= 0.01f;        /* the same sound again: choke it (a hat) */
        if (s->trk[v->track].patch.protect) score += 100.0f;         /* the kick, the bass: last */
        if (score < best_score) { best_score = score; best = i; }
    }
    s->steals++;
    to_tail(s, best);
    return best;
}
static uint32_t mix32(uint32_t h) { h ^= h >> 16; h *= 0x7FEB352Du; h ^= h >> 15; h *= 0x846CA68Bu; h ^= h >> 16; return h; }

/* the glide from `from` (< 0: none) to the voice's target (SPEC-SON): RATE an octave in T(GLIDE), TIME the whole way
 * in T(GLIDE); a slide without GLIDE: TIME, ~50 ms */
static void glide_to(const gb_track *k, gb_voice *v, float from, int glide)
{
    const float *q = k->patch.p;
    int g = gb_pv(q, SP_GLIDE);
    if (!glide || from < 0.0f || (!g && !k->slide)) { v->pitch = v->pitch_tgt; v->gstep = 0.0f; return; }
    v->pitch = from;
    float blocks = gb_time_ms((float)(g ? g : 54)) * (0.001f * GB_SR / (float)GB_BLOCK), d = gb_absf(v->pitch_tgt - from);
    v->gstep = (g && !gb_pv(q, SP_GLMODE) ? 12.0f : d) / gb_maxf(blocks, 1.0f);
    if (v->gstep <= 0.0f) v->pitch = v->pitch_tgt;
}
/* a note on voice slot i: fresh (from silence) or a voice of the track played again (its level, phases kept) */
GB_SMALL static void voice_go(gb_sound *s, int i, int fresh, gb_owner o, int t, int note, float vel, int uni, float fine)
{
    gb_track *k = &s->trk[t];
    const gb_patch *p = &k->patch;
    gb_voice *v = &s->v[i];
    if (fresh) {
        memset(v, 0, sizeof *v);
        v->seed = mix32((s->next_age + 1u) * 0x9E3779B9u ^ (uint32_t)note * 0x85EBCA6Bu) | 1u;
        v->pitch = -1.0f;
    }
    v->state = V_ON; v->track = (uint8_t)t; v->note = (uint8_t)note; v->uni = (uint8_t)uni; v->eng = p->engine;
    v->owner = o; v->age = s->next_age++;
    v->vel = vel; v->fine = fine; v->penv = 1.0f; v->eg = EG_ATT;
    v->pitch_tgt = (float)note;
    float gn = (p->gain > 0.0f ? p->gain : 1.0f) * gb_db(0.5f * (float)gb_pv(p->p, SP_TRIM));
    v->pan_l = v->pan_r = 0.70710678f * gn;                            /* (a part is mono, as SLOOP's) */
    if (fresh) v->level = 1.0f;
    v->others = 0;
    for (int j = 0; j < GB_NVOICE; j++) if (j != i && s->v[j].state == V_ON && s->v[j].track == t) v->others = 1;
    gb_engines[p->engine]->start(v, p, fresh);
}
/* the track's voice for unison index u that can be played again (not a tail), -1 none */
static int voice_of(const gb_sound *s, int t, int u)
{
    for (int i = 0; i < GB_NVOICE; i++) if ((s->v[i].state == V_ON || s->v[i].state == V_REL) && s->v[i].track == t && s->v[i].uni == u) return i;
    return -1;
}
static void peak_count(gb_sound *s) { int n = snd_voices(s); if ((uint32_t)n > s->voices_peak) s->voices_peak = (uint32_t)n; }

/* POLY (and the drums): a voice a note. The same note of the same owner again: its voice played again (a sound
 * with its own envelopes, or the drums: from silence, the old one in a tail) */
static void poly_on(gb_sound *s, gb_owner o, int t, int note, float vel)
{
    gb_track *k = &s->trk[t];
    const gb_patch *p = &k->patch;
    const gb_engine *e = gb_engines[p->engine];
    if (p->engine == ENG_DRUM) {                                       /* a choke group: the closed hat ends the open one */
        for (int i = 0; drum_choke(note) && i < GB_NVOICE; i++)
            if (s->v[i].state != V_FREE && s->v[i].track == t && drum_choked(s->v[i].note)) to_tail(s, i);
        if (drum_is_kick(note)) s->duck_pos = 0.0f;                    /* DUCK: the synth tracks pump from here */
    }
    int again = -1;
    for (int i = 0; i < GB_NVOICE; i++) {
        gb_voice *v = &s->v[i];
        if ((v->state == V_ON || v->state == V_REL) && v->track == t && v->note == note && v->owner == o && v->eng == p->engine) {
            if (again < 0 && !e->ownenv) again = i; else to_tail(s, i);
        }
    }
    if (again < 0) {                                                   /* the track's own limit: its oldest go first */
        int limit = gb_clampi(p->poly, 1, GB_NVOICE);
        while (track_voices(s, t) + 1 > limit) {
            int old = -1; uint32_t oldest = 0;
            for (int i = 0; i < GB_NVOICE; i++) {
                const gb_voice *v = &s->v[i];
                if (v->state == V_FREE || v->track != t) continue;
                uint32_t a = s->next_age - v->age;
                if (v->state == V_REL) a += 0x40000000u;                   /* released ones first */
                if (old < 0 || a > oldest) { oldest = a; old = i; }
            }
            if (old < 0) break;
            to_tail(s, old);
        }
    }
    int i = again >= 0 ? again : pick_slot(s, t, note);
    float from = again >= 0 ? s->v[i].pitch : k->last_pitch;
    voice_go(s, i, again < 0, o, t, note, vel, 0, 0.0f);
    glide_to(k, &s->v[i], from, gb_pv(p->p, SP_GLIDE) != 0 && p->engine != ENG_DRUM);
    k->last_pitch = (float)note;
    peak_count(s);
}

/* MONO / LEGATO / UNISON: one note (UNISON: on n voices, detuned across DETUNE). SLOOP's rules (SPEC-SON) */
static int uni_voices(const gb_patch *p) { return gb_vmode(p->p) == VM_UNISON ? gb_clampi(p->poly, 2, 8) : 1; }
GB_SMALL static void mono_play(gb_sound *s, gb_owner o, int t, int note, float vel, int retrig, int glide)
{
    gb_track *k = &s->trk[t];
    const gb_patch *p = &k->patch;
    int nv = uni_voices(p);
    for (int i = 0; i < GB_NVOICE; i++)                                /* voices of another mode, or past the count */
        if (s->v[i].state != V_FREE && s->v[i].state != V_KILL && s->v[i].track == t && (s->v[i].uni >= nv || s->v[i].eng != p->engine)) to_tail(s, i);
    float lvl = nv >= 8 ? vel * 0.36f : nv > 1 ? vel * 0.5f : vel;
    float det = (float)gb_pv(p->p, SP_DETUNE) * (56.0f / 889.0f / 4096.0f) * 17.312340f;   /* (ln 2 / 12: a ratio -> semitones) */
    for (int u = 0; u < nv; u++) {
        int kk = 2 * u - (nv - 1);
        if (nv > 1 && nv < 8) kk = kk * 7 / (nv - 1);                   /* fewer voices: the outer ones as wide */
        float fine = nv > 1 ? (float)kk * det : 0.0f;
        int i = voice_of(s, t, u);
        if (i >= 0 && retrig && gb_engines[p->engine]->ownenv == 2) { to_tail(s, i); i = -1; }   /* (a sample: from its start) */
        if (retrig || i < 0 || s->v[i].state != V_ON) {
            int fresh = i < 0;
            if (fresh) i = pick_slot(s, t, note);
            float from = fresh ? k->last_pitch : s->v[i].pitch;
            voice_go(s, i, fresh, o, t, note, lvl, u, fine);
            glide_to(k, &s->v[i], from, glide);
            const gb_engine *e = gb_engines[p->engine];
            if (fresh && u && nv > 1 && e->spread) e->spread(&s->v[i], mix32(s->v[i].seed * 0x2545F491u));
        } else {                                                       /* legato: the new pitch, the same envelope */
            gb_voice *v = &s->v[i];
            float from = v->pitch;
            v->note = (uint8_t)note; v->owner = o; v->pitch_tgt = (float)note;
            if (vel > v->vel && nv == 1) v->vel = vel;
            glide_to(k, v, from, glide);
        }
    }
    k->mono_cur = (int16_t)note;
    k->last_pitch = (float)note;
    peak_count(s);
}
static int mono_pick(const gb_track *k)
{
    if (!k->mono_n) return -1;
    int pr = gb_prio(k->patch.p), best = k->mono_n - 1;
    for (int i = 0; i < k->mono_n && pr != PR_LAST; i++)
        if ((pr == PR_LOW && k->mono_note[i] < k->mono_note[best]) || (pr == PR_HIGH && k->mono_note[i] > k->mono_note[best])) best = i;
    return best;
}
static void mono_drop(gb_track *k, int i)
{
    for (int j = i; j + 1 < k->mono_n; j++) { k->mono_note[j] = k->mono_note[j + 1]; k->mono_own[j] = k->mono_own[j + 1]; k->mono_vel[j] = k->mono_vel[j + 1]; }
    k->mono_n--;
}
static void release_voice(gb_sound *s, gb_voice *v)
{
    v->state = V_REL; v->eg = EG_REL;
    const gb_engine *e = gb_engines[v->eng];
    if (e->release && s->trk[v->track].patch.engine == v->eng) e->release(v, &s->trk[v->track].patch);   /* (another engine's patch: not its own to read) */
}
static int track_sounding(const gb_sound *s, int t)
{
    for (int i = 0; i < GB_NVOICE; i++) if (s->v[i].state == V_ON && s->v[i].track == t) return 1;
    return 0;
}
static void mono_off(gb_sound *s, gb_owner o, int t, int note)
{
    gb_track *k = &s->trk[t];
    int i = 0;
    while (i < k->mono_n && !(k->mono_note[i] == note && k->mono_own[i] == o)) i++;
    if (i == k->mono_n) return;
    mono_drop(k, i);
    if (note != k->mono_cur) return;
    int b = mono_pick(k);
    if (b >= 0) {                                                      /* back to a held note, legato */
        mono_play(s, k->mono_own[b], t, k->mono_note[b], k->mono_vel[b], 0, gb_pv(k->patch.p, SP_GLIDE) != 0);
        return;
    }
    for (int j = 0; j < GB_NVOICE; j++) if (s->v[j].state == V_ON && s->v[j].track == t) release_voice(s, &s->v[j]);
    k->mono_cur = -1;
}

GB_SMALL void snd_note_on(gb_sound *s, gb_owner o, int t, int note, float vel)
{
    if (t < 0 || t >= GB_NTRACK || note < 0 || note > 127) return;
    gb_track *k = &s->trk[t];
    const gb_patch *p = &k->patch;
    if (p->engine >= gb_nengines) return;
    vel = gb_clampf(vel, 0.0f, 1.0f);
    s->vis_hit[t]++; s->vis_vel[t] = (uint8_t)(vel * 127.0f); s->vis_note[t] = (uint8_t)note;
    if (p->engine == ENG_DRUM && drum_is_kick(note)) s->vis_kick++;
    if (!track_sounding(s, t)) {                                       /* a fresh phrase: the LFO's TRIG and FADE */
        if (!gb_pv(p->p, SP_LTRIG)) k->lfo_ph = (uint32_t)gb_pv(p->p, SP_LPHASE) << 25;
        k->lfo_fade = gb_pv(p->p, SP_LFADE) ? 0.0f : 1.0f;
    }
    int mode = gb_vmode(p->p);
    if (p->engine == ENG_DRUM || mode == VM_POLY) { k->mono_n = 0; k->mono_cur = -1; poly_on(s, o, t, note, vel); return; }
    int sounding = k->mono_cur >= 0 && track_sounding(s, t);
    for (int i = 0; i < k->mono_n; i++) if (k->mono_note[i] == note && k->mono_own[i] == o) { mono_drop(k, i); break; }
    if (k->mono_n == GB_MONO_HELD) mono_drop(k, 0);                    /* (the oldest forgotten) */
    k->mono_note[k->mono_n] = (uint8_t)note; k->mono_own[k->mono_n] = o; k->mono_vel[k->mono_n] = vel; k->mono_n++;
    int b = mono_pick(k);
    if (sounding && k->mono_note[b] == k->mono_cur && b != k->mono_n - 1) return;   /* LOW / HIGH: this key does not win */
    int gl = gb_pv(p->p, SP_GLIDE) != 0 || k->slide;
    if (mode == VM_MONO) mono_play(s, k->mono_own[b], t, k->mono_note[b], k->mono_vel[b], 1, gl);
    else mono_play(s, k->mono_own[b], t, k->mono_note[b], k->mono_vel[b], !sounding, sounding && gl);
}
void snd_slide(gb_sound *s, int t, int on) { if (t >= 0 && t < GB_NTRACK) s->trk[t].slide = (uint8_t)(on != 0); }

GB_SMALL void snd_note_off(gb_sound *s, gb_owner o, int t, int note)
{
    if (t < 0 || t >= GB_NTRACK) return;
    gb_track *k = &s->trk[t];
    if (k->mono_n || k->mono_cur >= 0) mono_off(s, o, t, note);
    for (int i = 0; i < GB_NVOICE; i++) {
        gb_voice *v = &s->v[i];
        if (v->state == V_ON && v->owner == o && v->track == t && v->note == note && !(k->mono_cur == note && k->mono_n)) release_voice(s, v);
    }
}
GB_SMALL void snd_owner_off(gb_sound *s, gb_owner o)
{
    for (int t = 0; t < GB_NTRACK; t++) {                              /* MONO / LEGATO: its held notes forgotten */
        gb_track *k = &s->trk[t];
        for (int i = k->mono_n - 1; i >= 0; i--) if (k->mono_own[i] == o) mono_drop(k, i);
        if (k->mono_cur >= 0 && !k->mono_n) k->mono_cur = -1;
    }
    for (int i = 0; i < GB_NVOICE; i++) {
        gb_voice *v = &s->v[i];
        if (v->state == V_ON && v->owner == o) release_voice(s, v);
    }
}
int snd_shed(gb_sound *s)
{
    int best = -1; float best_score = 1e30f;
    for (int i = 0; i < GB_NVOICE; i++) {
        const gb_voice *v = &s->v[i];
        if (v->state == V_FREE) continue;
        float score = v->level - 1e-9f * (float)(s->next_age - v->age);
        if (v->state == V_REL) score *= 0.25f;
        if (s->trk[v->track].patch.protect) score += 100.0f;
        if (score < best_score) { best_score = score; best = i; }
    }
    if (best < 0) return 0;
    to_tail(s, best);
    s->shed++;
    return 1;
}

void snd_kill(gb_sound *s)                                /* every voice silent at once (a sample slot rewritten) */
{
    for (int i = 0; i < GB_NVOICE; i++) s->v[i].state = V_FREE;
    for (int t = 0; t < GB_NTRACK; t++) { s->trk[t].mono_n = 0; s->trk[t].mono_cur = -1; }
    grain_forget();
}
void snd_all_off(gb_sound *s)
{
    for (int i = 0; i < GB_NVOICE; i++) if (s->v[i].state != V_FREE) to_tail(s, i);
    for (int t = 0; t < GB_NTRACK; t++) { s->trk[t].mono_n = 0; s->trk[t].mono_cur = -1; }
}

/* ---------------------------------------------------------------- the block
 * In two halves, so that two cores can share it: one renders a part of the voices (index % nparts == part)
 * (snd_render_voices) into a bus of track buffers, each core its own bus and its own voices; snd_mix (one core) adds the other bus's
 * tracks in, then the strips, the effects, the master and the output, and makes the tracks' parts for the next
 * block. No voice is touched by two cores, and the notes change only between blocks. snd_render is the one-core block. */
static gb_bus bus0;
uint32_t gb_blocks;

/* the amplitude envelope over a block, a value a frame into eb (runs of one stage, no test a frame) */
GB_HOT static void env_run(gb_voice *v, const gb_track *k, float *eb)
{
    int n = 0;
    float e = v->env;
    while (n < GB_BLOCK) {
        switch (v->eg) {
        case EG_ATT: {
            float inc = k->att_inc;
            int run = (int)((1.0f - e) / inc) + 1, end = n + run < GB_BLOCK ? n + run : GB_BLOCK;
            for (; n < end; n++) { e += inc; eb[n] = e < 1.0f ? e : 1.0f; }
            if (e >= 1.0f) { e = 1.0f; v->eg = EG_DEC; }
            break; }
        case EG_DEC: {
            float kk = k->dec_k, su = k->sus;
            GB_UNROLL4
            for (; n < GB_BLOCK; n++) { e = su + (e - su) * kk; eb[n] = e; }
            /* (3.1.8: AT the sustain once within -120 dB of it. With SUS at 0, a pluck held went on falling into the
             * denormals for as long as the key stayed down: tests/denorm_test.c) */
            { float d = e - su; if (d < 1e-6f && d > -1e-6f) e = su; }
            break; }
        case EG_REL: {
            float kk = k->rel_k;
            GB_UNROLL4
            for (; n < GB_BLOCK; n++) { e *= kk; eb[n] = e; }
            if (e < (1.0f / 4096.0f)) { e = 0.0f; v->eg = EG_IDLE; }
            break; }
        default: for (; n < GB_BLOCK; n++) eb[n] = 0.0f; e = 0.0f; break;
        }
    }
    v->env = e;
}

GB_HOT void snd_render_voices(gb_sound *s, gb_bus *b, int part, int nparts)
{
    float vb[GB_BLOCK], eb[GB_BLOCK];
    uint32_t live = 0;                                                 /* the tracks a voice wrote into this block */
    for (int t = 0; t < GB_NTRACK; t++) b->pk[t] = 0.0f;
    const float pfall = 0.9653f;                                       /* the pitch envelope a block: e^(-64 / (40.3 ms x 44.1)) */
    for (int i = part; i < GB_NVOICE + GB_NTAIL; i += nparts) {
        gb_voice *v = &s->v[i];
        if (v->state == V_FREE) continue;
        const gb_track *k = &s->trk[v->track];
        const gb_patch *p = &k->patch;
        const gb_engine *e = gb_engines[v->eng];
        gb_patch vp;
        if (p->engine != v->eng) {           /* the track's sound changed engine under this voice (3.1.2): read with its
                                              * own engine's ranges (another's would index past its tables), and out
                                              * in a short fade */
            vp = *p; vp.engine = v->eng; p = &vp;
            if (v->state != V_KILL) { v->state = V_KILL; v->kill = GB_KILL_FRAMES; }
        }
        const float *q = p->p;
        if (v->pitch != v->pitch_tgt) {                                 /* the glide, a block at a time */
            float d = v->pitch_tgt - v->pitch;
            v->pitch = gb_absf(d) <= v->gstep || v->gstep <= 0.0f ? v->pitch_tgt : v->pitch + (d > 0.0f ? v->gstep : -v->gstep);
        }
        gb_mod m;
        float penv = v->penv, lfo = k->lfo;
        v->penv = gb_flush(penv * pfall);
        m.env0 = v->env;
        int own = e->ownenv == 1;
        if (!own) env_run(v, k, eb);
        m.env1 = own ? 0.0f : v->env;
        if (!own && v->eg == EG_DEC && v->env < 1e-6f && m.env0 < 1e-6f && k->sus <= 0.0f) {   /* held at a silent sustain */
            v->level = 0.0f;
            if (v->state == V_KILL) { v->kill = (int16_t)(v->kill > GB_BLOCK ? v->kill - GB_BLOCK : 0); if (!v->kill) v->state = V_FREE; }
            continue;
        }
        m.lfo = lfo;
        m.pitch = v->pitch + v->fine + k->bend + 0.1875f * (lfo * (float)gb_pv(q, SP_LPIT) + penv * (float)gb_pv(q, SP_EPIT));
        m.dt = gb_minf(gb_note_hz(m.pitch) * GB_ISR, 0.45f);
        float ef = (float)gb_pv(q, SP_EFLT) + (v->vel > (110.0f / 127.0f) ? 24.0f : 0.0f), lf = lfo * (float)gb_pv(q, SP_LFLT);
        m.cut0 = lf + (own ? 0.0f : m.env0) * ef;
        m.cut = lf + m.env1 * ef;
        m.shape = 64.0f + lfo * (float)gb_pv(q, SP_LSHP) + m.env1 * (float)gb_pv(q, SP_ESHP);
        int alive = e->render(v, p, &m, vb);                         /* (render writes every frame of vb) */
        if (!own) {                                                    /* the amplitude: envelope x velocity x LFO > AMP */
            float g = v->vel;
            int la = gb_pv(q, SP_LAMP);
            if (la) g *= 1.0f - (gb_pv(q, SP_LPOL) ? lfo : 0.5f * (lfo + 1.0f)) * (float)la * (1.0f / 127.0f);
            if (v->flat) {                                             /* the engine's volume: the ADSR gates */
                GB_UNROLL4
                for (int n = 0; n < GB_BLOCK; n++) vb[n] *= g;
                if (v->eg == EG_REL) alive = alive && v->env > 0.01f;
                else { v->env = 1.0f; v->eg = EG_DEC; }
            } else {
                GB_UNROLL4
                for (int n = 0; n < GB_BLOCK; n++) vb[n] *= eb[n] * g;
                alive = v->eg != EG_IDLE;
            }
        }
        if (v->state == V_KILL) {                                       /* the tail's fade (G2) */
            float g = (float)v->kill * (1.0f / GB_KILL_FRAMES), dg = 1.0f / GB_KILL_FRAMES;
            GB_UNROLL4
            for (int n = 0; n < GB_BLOCK; n++) { vb[n] *= g; g -= dg; g = g > 0.0f ? g : 0.0f; }
            v->kill = (int16_t)(v->kill > GB_BLOCK ? v->kill - GB_BLOCK : 0);
            if (v->kill == 0) alive = 0;
        }
        float *l = b->l[v->track], *r = b->r[v->track], pl = v->pan_l, pr = v->pan_r, pk = 0.0f;
        if (!(live & (1u << v->track))) {                               /* the first voice of its track writes */
            live |= 1u << v->track;
            GB_UNROLL4
            for (int n = 0; n < GB_BLOCK; n++) { float x = vb[n]; l[n] = x * pl; r[n] = x * pr; float a = gb_absf(x); pk = a > pk ? a : pk; }
        } else {
            GB_UNROLL4
            for (int n = 0; n < GB_BLOCK; n++) { float x = vb[n]; l[n] += x * pl; r[n] += x * pr; float a = gb_absf(x); pk = a > pk ? a : pk; }
        }
        v->level = pk; b->pk[v->track] += pk;
        if (!alive) v->state = V_FREE;
    }
    b->live = live;
}

GB_HOT void snd_render(gb_sound *s, int32_t *out)
{
    snd_render_voices(s, &bus0, 0, 1);
    snd_mix(s, &bus0, 0, out);
}

/* DIST (SPEC-SON): a low cut, a gain of 1x .. 8x (exponential), an asymmetric soft clip */
GB_HOT static void dist_block(float *l, float *r, int v, float *st)
{
    float g = gb_exp2f((float)v * (3.0f / 127.0f)), mk = 1.0f / (1.0f + (g - 1.0f) * 0.35f), bias = 0.12f;
    float c0 = gb_softclip(bias), a = 0.0028f;                         /* (~20 Hz) */
    float hl = st[0], hr = st[1];
    for (int n = 0; n < GB_BLOCK; n++) {
        hl += (l[n] - hl) * a; hr += (r[n] - hr) * a;
        l[n] = (gb_softclip((l[n] - hl) * g + bias) - c0) * mk;
        r[n] = (gb_softclip((r[n] - hr) * g + bias) - c0) * mk;
    }
    st[0] = gb_flush(hl); st[1] = gb_flush(hr);
}
static float duck_gain(const gb_sound *s, float pos)
{
    if (s->duck <= 0.0f) return 1.0f;
    float len = gb_maxf(s->beat * 0.5f, 2000.0f);                      /* an 1/8 note */
    if (pos >= len) return 1.0f;
    float x = 1.0f - pos / len, att = pos < 88.0f ? pos * (1.0f / 88.0f) : 1.0f;   /* (2 ms down: no click) */
    return 1.0f - 0.85f * s->duck * x * x * att;
}
/* SPEAKER (HOME > AUDIO, with the low cut, 3.1.3): a drum track's lows, which the FM-1's small speaker cannot play
 * (an 808 kick: 50 Hz), carried up as their harmonics: the low band (two poles at 140 Hz) rectified and softly
 * limited (its 2nd, 4th... harmonics), its own lows taken out (two poles at 120 Hz), added. The kick is heard again
 * on the speaker, ~10 dB more there (Sam, 11 nov.: "on n'entend quasiment pas le drum") */
static float spk_st[GB_NTRACK][4];
static void drum_speaker(float *l, float *r, float *st)
{
    float l1 = st[0], l2 = st[1], h1 = st[2], h2 = st[3];
    for (int n = 0; n < GB_BLOCK; n++) {
        float m = 0.5f * (l[n] + r[n]);
        l1 += 0.0197f * (m - l1); l2 += 0.0197f * (l1 - l2);
        float x = 2.0f * gb_absf(l2), hh = 0.5f * x / (1.0f + x);
        h1 += 0.0170f * (hh - h1); h2 += 0.0170f * (h1 - h2);
        float e = 5.0f * (hh - h2);
        l[n] += e; r[n] += e;
    }
    st[0] = gb_flush(l1); st[1] = gb_flush(l2); st[2] = gb_flush(h1); st[3] = gb_flush(h2);
}

GB_HOT void snd_mix(gb_sound *s, gb_bus *b, const gb_bus *other, int32_t *out)
{
    uint32_t live = b->live;
    if (other) {                                                       /* the other core's tracks into ours */
        for (int t = 0; t < GB_NTRACK; t++) {
            if (!(other->live & (1u << t))) continue;
            float *l = b->l[t], *r = b->r[t];
            const float *ol = other->l[t], *or_ = other->r[t];
            if (live & (1u << t)) {
                GB_UNROLL4
                for (int n = 0; n < GB_BLOCK; n++) { l[n] += ol[n]; r[n] += or_[n]; }
            } else {
                GB_UNROLL4
                for (int n = 0; n < GB_BLOCK; n++) { l[n] = ol[n]; r[n] = or_[n]; }
            }
        }
        live |= other->live;
    }
    float (*trk_l)[GB_BLOCK] = b->l, (*trk_r)[GB_BLOCK] = b->r;
    /* the strips: level, pan, mute, sends; all smoothed (G3). The pan's sine at the block's two ends only, the
     * gains between them linear; a track with no voice this block is skipped (its smoothing still moves) */
    float ml[GB_BLOCK], mr[GB_BLOCK], dl[GB_BLOCK], dr[GB_BLOCK], rl[GB_BLOCK], rr[GB_BLOCK], cl[GB_BLOCK];
    GB_UNROLL4
    for (int n = 0; n < GB_BLOCK; n++) { ml[n] = mr[n] = dl[n] = dr[n] = rl[n] = rr[n] = cl[n] = 0.0f; }
    int cho_any = 0;
    /* DUCK: the synth tracks' gain at the block's two ends (the kick's 1/8: down at once, back as a square) */
    float duck0 = duck_gain(s, s->duck_pos), duck1 = duck_gain(s, s->duck_pos + (float)GB_BLOCK);
    /* the throws: every send towards full while held */
    float std_ = sm_step(&s->throw_dly), str_ = sm_step(&s->throw_rev);
    float thd0 = s->throw_dly.cur, thd1 = thd0 + std_ * GB_BLOCK, thr0 = s->throw_rev.cur, thr1 = thr0 + str_ * GB_BLOCK;
    for (int t = 0; t < GB_NTRACK; t++) {
        gb_track *k = &s->trk[t];
        if (k->mute) sm_set(&k->level, 0.0f);
        float sl = sm_step(&k->level), sp = sm_step(&k->pan), sd = sm_step(&k->send_dly), sr = sm_step(&k->send_rev), sf = sm_step(&k->filt), sc = sm_step(&k->send_cho);
        int run = (live >> t) & 1u;
        /* 3.1.8: voices that sound, but below anything heard (-180 dB: a sample ended and held, its filter settled): the
         * track gets silence, not their trace. A trace so small, kept up for as long as the key, took the track's
         * filter and its distortion into the denormals at every sample (tests/denorm_test.c, the twin's monkey). */
        if (run && b->pk[t] + (other ? other->pk[t] : 0.0f) < GB_TINY) for (int n = 0; n < GB_BLOCK; n++) trk_l[t][n] = trk_r[t][n] = 0.0f;
        if (run) k->fx_tail = gb_pv(k->patch.p, SP_ITYPE) ? 64 : 0;
        else if (k->fx_tail || tfx_busy(s, t)) {                       /* no voice: the SLICER's repeat, the INSERT's tail */
            if (k->fx_tail) k->fx_tail--;
            for (int n = 0; n < GB_BLOCK; n++) trk_l[t][n] = trk_r[t][n] = 0.0f;
            run = 1;
        } else tfx_slicer(s, t, 0, 0);                                 /* (the clock only) */
        if (run) {
            int dv = gb_pv(k->patch.p, SP_DIST);
            tfx_slicer(s, t, trk_l[t], trk_r[t]);
            if (dv && k->patch.engine != ENG_DRUM) dist_block(trk_l[t], trk_r[t], dv, k->dist_st);
            tfx_insert(s, t, trk_l[t], trk_r[t]);
            if (gb_absf(k->filt.cur + sf * GB_BLOCK) > 1e-4f) gb_svf_dj(trk_l[t], trk_r[t], k->filt.cur + sf * GB_BLOCK, k->filt_st, s->flt_q);
            if (s->lowcut && k->patch.engine == ENG_DRUM) drum_speaker(trk_l[t], trk_r[t], spk_st[t]);
            float lv0 = k->level.cur, lv1 = lv0 + sl * GB_BLOCK, pn0 = k->pan.cur, pn1 = pn0 + sp * GB_BLOCK;
            if (k->patch.engine != ENG_DRUM) { lv0 *= duck0; lv1 *= duck1; }
            /* the sends (a throw takes them towards full; the track's own values stay as they are) */
            float ds = k->send_dly.cur, ds1 = ds + sd * GB_BLOCK, rs = k->send_rev.cur, rs1 = rs + sr * GB_BLOCK;
            if (thd0 > 0.0f || thd1 > 0.0f) { ds += (1.0f - ds) * thd0; ds1 += (1.0f - ds1) * thd1; }
            if (thr0 > 0.0f || thr1 > 0.0f) { rs += (1.0f - rs) * thr0; rs1 += (1.0f - rs1) * thr1; }
            float dsd = (ds1 - ds) * (1.0f / GB_BLOCK), dsr = (rs1 - rs) * (1.0f / GB_BLOCK);
            float gl = lv0 * gb_sin_turn(0.25f + pn0 * 0.25f), gr = lv0 * gb_sin_turn(pn0 * 0.25f);
            float dgl = (lv1 * gb_sin_turn(0.25f + pn1 * 0.25f) - gl) * (1.0f / GB_BLOCK);
            float dgr = (lv1 * gb_sin_turn(pn1 * 0.25f) - gr) * (1.0f / GB_BLOCK);
            float *tl = trk_l[t], *tr = trk_r[t];
            /* in three short loops rather than one long one: few pointers each, nothing spilled (16 registers) */
            GB_UNROLL4
            for (int n = 0; n < GB_BLOCK; n++) { gl += dgl; gr += dgr; tl[n] *= gl; tr[n] *= gr; ml[n] += tl[n]; mr[n] += tr[n]; }
            { float pk = gb_maxf(gb_absf(tl[0]), gb_absf(tr[0])); for (int n = 8; n < GB_BLOCK; n += 8) pk = gb_maxf(pk, gb_maxf(gb_absf(tl[n]), gb_absf(tr[n]))); if (pk > k->meter) k->meter = pk; }
            if (ds > 0.0f || dsd != 0.0f) {
                GB_UNROLL4
                for (int n = 0; n < GB_BLOCK; n++) { ds += dsd; dl[n] += tl[n] * ds; dr[n] += tr[n] * ds; }
            }
            if (rs > 0.0f || dsr != 0.0f) {
                GB_UNROLL4
                for (int n = 0; n < GB_BLOCK; n++) { rs += dsr; rl[n] += tl[n] * rs; rr[n] += tr[n] * rs; }
            }
            float cs = k->send_cho.cur;
            if (cs > 0.0f || sc != 0.0f) {
                cho_any = 1;
                GB_UNROLL4
                for (int n = 0; n < GB_BLOCK; n++) { cs += sc; cl[n] += (tl[n] + tr[n]) * 0.5f * cs; }
            }
        } else { k->filt_st[0] = k->filt_st[1] = k->filt_st[2] = k->filt_st[3] = 0.0f; }
        k->meter = gb_flush(k->meter * 0.97f);
        sm_end(&k->level, sl); sm_end(&k->pan, sp); sm_end(&k->send_dly, sd); sm_end(&k->send_rev, sr); sm_end(&k->filt, sf); sm_end(&k->send_cho, sc);
    }
    sm_end(&s->throw_dly, std_); sm_end(&s->throw_rev, str_);
    float dry_l[GB_BLOCK], dry_r[GB_BLOCK];
    texture_mix(s, ml, mr, duck0, duck1);                              /* (the bed, dry) */
    master_pre(s, ml, mr, dl, dr, rl, rr, dry_l, dry_r);
    fx_render(s, dl, dr, rl, rr, ml, mr);
    if (cho_any || s->cho_ph != 0.0f) chorus_block(s, cl, ml, mr);   /* (once started it runs on: its tail) */
    master_post(s, ml, mr, dry_l, dry_r);
    if (s->vis_ring) {                                                 /* (the visualiser: two frames averaged, 8 bits) */
        int8_t *v = s->vis_ring; unsigned w = s->vis_w;
        for (int n = 0; n < GB_BLOCK; n += 2) {
            float l = gb_clampf((ml[n] + ml[n + 1]) * 63.5f, -127.0f, 127.0f), r = gb_clampf((mr[n] + mr[n + 1]) * 63.5f, -127.0f, 127.0f);
            v[w] = (int8_t)l; v[GB_VIS_RING + w] = (int8_t)r; w = (w + 1u) & (GB_VIS_RING - 1u);
        }
        s->vis_w = (uint16_t)w;
    }
    s->duck_pos = s->duck_pos < 1e8f ? s->duck_pos + (float)GB_BLOCK : s->duck_pos;
    tracks_tick(s);                                                    /* the tracks' parts for the next block */
    gb_blocks++;
    /* the master and the limiter: the whole block is known before it is played, so the gain is lowered before
     * the peak arrives (a block of look-ahead, no added latency) */
    if (s->usb_full)                                                   /* (USB AUDIO FULL: the mix, MASTER all the way up) */
        for (int n = 0; n < GB_BLOCK; n++) { s->usb_l[n] = gb_clampf(ml[n], -1.0f, 1.0f); s->usb_r[n] = gb_clampf(mr[n], -1.0f, 1.0f); }
    /* (3.4.2, issue #16: "3.4 is 6 to 9 dB quieter than 2.8.5", measured) the mix 6 dB up before the limiter: as loud
     * as before 3.4.0, its peaks still held at the ceiling (the DAC 6 dB under it since 4.0.9: fw/main.c), as 2.8.5's
     * limiter holds its own; a quiet mix is louder, a loud one is limited, never clipped */
    float sm = sm_step(&s->master), smg = sm * s->mix_gain, mg = s->master.cur * s->mix_gain, pk = 0.0f, e = 0.0f;
    GB_UNROLL4
    for (int n = 0; n < GB_BLOCK; n++) { mg += smg; ml[n] *= mg; mr[n] *= mg; float x = gb_absf(ml[n]), y = gb_absf(mr[n]), a = gb_maxf(x, y); e += x + y; if (a > pk) pk = a; }   /* (e: a NaN kept, as max would not) */
    sm_end(&s->master, sm);
    if (s->panic_quiet) s->panic_quiet--;
    if (!(e < 1e9f)) {          /* 3.3.2: a value gone wrong (NaN, infinite; no float trap: no restart) would stay in the
                                 * effects' lines and the filters for good: this block silent, the effects cleared */
        for (int n = 0; n < GB_BLOCK; n++) ml[n] = mr[n] = 0.0f;
        pk = 0.0f; snd_panic(s);
    }
    if (s->lowcut == 1) {                                              /* the small speaker: two one-pole high-passes */
        float a = s->lc[0], b2 = s->lc[1], c = s->lc[2], d = s->lc[3];
        for (int n = 0; n < GB_BLOCK; n++) {
            a += (ml[n] - a) * (1.0f / 64.0f); float x = ml[n] - a; b2 += (x - b2) * (1.0f / 64.0f); ml[n] = x - b2;
            c += (mr[n] - c) * (1.0f / 64.0f); x = mr[n] - c; d += (x - d) * (1.0f / 64.0f); mr[n] = x - d;
        }
        s->lc[0] = gb_flush(a); s->lc[1] = gb_flush(b2); s->lc[2] = gb_flush(c); s->lc[3] = gb_flush(d);
    }
    const float ceil_ = 0.891f;                                        /* -1 dBFS */
    float want = pk > ceil_ ? ceil_ / pk : 1.0f;
    float g0 = s->lim_gain, g1 = want < g0 ? want : g0 + (want - g0) * 0.02f;   /* down at once, up slowly (~70 ms) */
    float dg = (g1 - g0) * (1.0f / GB_BLOCK);
    /* the soft knee is needed only when something can still pass the ceiling (the first samples of a sudden
     * peak, as the gain comes down): known for the whole block, so the usual block has no test per sample */
    int knee = pk * (g0 > g1 ? g0 : g1) > ceil_;
    if (!knee) {
        GB_UNROLL2
        for (int n = 0; n < GB_BLOCK; n++) {
            float g = g0 + dg * (float)(n + 1);
            float l = ml[n] * g, r = mr[n] * g;
#ifndef __PI32V2__
            snd_last_l[n] = l; snd_last_r[n] = r;
#endif
            /* the one conversion (G1): to 24 bits, rounded to nearest */
            float fl = l * 8388607.0f, fr = r * 8388607.0f;
            out[2 * n] = (int32_t)(fl + (fl >= 0.0f ? 0.5f : -0.5f));
            out[2 * n + 1] = (int32_t)(fr + (fr >= 0.0f ? 0.5f : -0.5f));
        }
    } else {
        for (int n = 0; n < GB_BLOCK; n++) {
            float g = g0 + dg * (float)(n + 1);
            float l = ml[n] * g, r = mr[n] * g;
            if (gb_absf(l) > ceil_) l = (l > 0 ? 1.0f : -1.0f) * (ceil_ + (1.0f - ceil_) * gb_sat((gb_absf(l) - ceil_) / (1.0f - ceil_)));
            if (gb_absf(r) > ceil_) r = (r > 0 ? 1.0f : -1.0f) * (ceil_ + (1.0f - ceil_) * gb_sat((gb_absf(r) - ceil_) / (1.0f - ceil_)));
#ifndef __PI32V2__
            snd_last_l[n] = l; snd_last_r[n] = r;
#endif
            float fl = l * 8388607.0f, fr = r * 8388607.0f;
            out[2 * n] = (int32_t)(fl + (fl >= 0.0f ? 0.5f : -0.5f));
            out[2 * n + 1] = (int32_t)(fr + (fr >= 0.0f ? 0.5f : -0.5f));
        }
    }
    s->lim_gain = g1;
}
