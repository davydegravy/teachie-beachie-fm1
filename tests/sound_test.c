/* SPDX-License-Identifier: GPL-3.0-only
 * The guarantees of docs/ARCHITECTURE.md section 1, checked on the PC: each test fails if the guarantee breaks. */
#include "../engine/engine.h"
#include <stdio.h>
#include "../engine/sample.h"
#include "../engine/slice.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int fails, checks;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("  FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

static int16_t fxmem[GB_FX_MEM];
static gb_sound S;
static int32_t out[GB_BLOCK * 2];

static int16_t pmem[GB_PUNCH_MEM];
static gb_bus dbg_bus;
static void reset(void) { snd_init(&S, fxmem, pmem); }
static void va_patch(int t, float cut, float att, float rel)
{
    gb_patch *p = &S.trk[t].patch;
    gb_patch_default(p, ENG_VA); p->poly = 8;
    gb_patch_set(p, SP_E4, (int)(cut * 127.0f)); gb_patch_set(p, SP_E5, 13); gb_patch_set(p, SP_E2, 0);
    gb_patch_set(p, SP_ATK, (int)(att * 127.0f)); gb_patch_set(p, SP_DEC, 64); gb_patch_set(p, SP_SUS, 127); gb_patch_set(p, SP_REL, (int)(rel * 127.0f));
}
/* render n blocks; returns the largest sample-to-sample step of the float master (left), and the peak */
static float last_x;
static void run(int blocks, float *maxstep, float *peak)
{
    float ms = 0, pk = 0;
    for (int b = 0; b < blocks; b++) {
        snd_render(&S, out);
        for (int n = 0; n < GB_BLOCK; n++) {
            float x = snd_last_l[n];
            if (isnan(x)) { CHECK(0, "NaN in the output"); return; }
            float d = fabsf(x - last_x); if (d > ms) ms = d;
            if (fabsf(x) > pk) pk = fabsf(x);
            last_x = x;
        }
    }
    if (maxstep) *maxstep = ms;
    if (peak) *peak = pk;
}

/* G5: only a note's owner ends it; panic and owner_off leave nothing sounding */
static void owner_test(void)
{
    printf("G5 owners\n");
    reset(); va_patch(0, 0.6f, 0.0f, 0.1f);
    gb_owner key = GB_OWNER(OWN_KEY, 0), seq = GB_OWNER(OWN_SEQ, 0), midi = GB_OWNER(OWN_MIDI, 1);
    snd_note_on(&S, key, 0, 48, 1.0f);
    snd_note_off(&S, seq, 0, 48);                       /* the sequencer cannot end the key's note */
    run(2, 0, 0);
    CHECK(snd_voices(&S) == 1 && S.v[0].state == V_ON, "a key's note was ended by the sequencer");
    snd_note_off(&S, key, 0, 48);
    run(200, 0, 0);
    CHECK(snd_voices(&S) == 0, "the note did not end (%d voices)", snd_voices(&S));
    /* 10000 random events from four owners, then each owner lets go of everything */
    srand(1);
    gb_owner own[4] = { key, seq, midi, GB_OWNER(OWN_ARP, 0) };
    for (int i = 0; i < 10000; i++) {
        gb_owner o = own[rand() % 4]; int note = 36 + rand() % 24;
        if (rand() % 3) snd_note_on(&S, o, 0, note, 0.8f); else snd_note_off(&S, o, 0, note);
        if (i % 50 == 0) run(1, 0, 0);
    }
    for (int i = 0; i < 4; i++) snd_owner_off(&S, own[i]);
    run(400, 0, 0);
    CHECK(snd_voices(&S) == 0, "%d notes stuck after every owner let go", snd_voices(&S));
}

/* G2: a note played again on a sounding voice starts from silence, the old one fades: no click */
static void retrig_test(void)
{
    printf("G2 retrigger without a click\n");
    reset(); va_patch(0, 0.35f, 0.0f, 0.3f);              /* a dark, smooth tone */
    float steady, step, pk;
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 0, 45, 1.0f);
    run(60, 0, 0); run(60, &steady, &pk);
    float worst = 0;
    for (int i = 0; i < 20; i++) {
        snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 0, 45, 1.0f);  /* the same key again, the old note still sounding */
        run(8, &step, 0); if (step > worst) worst = step;
    }
    printf("  steady step %.4f, worst step at a retrigger %.4f (peak %.3f)\n", steady, worst, pk);
    CHECK(worst < steady * 1.6f + 1e-3f, "a retrigger made a step %.4f against %.4f", worst, steady);
    /* a stolen voice: 9 notes on a poly 8 track */
    reset(); va_patch(0, 0.35f, 0.0f, 0.5f); S.trk[0].patch.poly = 8;
    for (int i = 0; i < 8; i++) snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 0, 40 + i * 3, 0.7f);
    run(60, 0, 0); run(30, &steady, 0);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 0, 70, 0.7f);
    run(8, &step, 0);
    printf("  steal: steady %.4f, at the steal %.4f, tails cut %u\n", steady, step, S.tails_cut);
    CHECK(step < steady * 1.6f + 1e-3f, "a steal clicked: %.4f against %.4f", step, steady);
}

/* G3: a gain jump is smoothed */
static void zipper_test(void)
{
    printf("G3 smoothed gains\n");
    reset(); va_patch(0, 0.3f, 0.0f, 0.3f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 0, 52, 1.0f);
    float steady, step;
    run(60, 0, 0); run(30, &steady, 0);
    sm_set(&S.trk[0].level, 0.0f);                        /* a mute, at once */
    run(4, &step, 0);
    CHECK(step < steady * 1.3f + 1e-3f, "the mute clicked: %.4f against %.4f", step, steady);
    S.trk[0].mute = 0; sm_set(&S.trk[0].level, 1.0f); sm_set(&S.trk[0].pan, 0.0f);
    run(4, &step, 0);
    CHECK(step < steady * 1.6f + 1e-3f, "unmute and pan clicked: %.4f against %.4f", step, steady);
}

/* G4: a protected track (the kick) is never stolen, however dense the rest */
static void steal_test(void)
{
    printf("G4 the kick survives a dense passage\n");
    reset(); va_patch(1, 0.6f, 0.0f, 1.0f); S.trk[1].patch.poly = 32;
    gb_patch *d = &S.trk[0].patch; gb_patch_default(d, ENG_DRUM); d->poly = 8; d->protect = 1;
    int kicks_cut = 0;
    for (int beat = 0; beat < 16; beat++) {
        snd_note_on(&S, GB_OWNER(OWN_SEQ, 0), 0, 0, 1.0f);         /* the kick */
        int kslot = -1; for (int i = 0; i < GB_NVOICE; i++) if (S.v[i].state != V_FREE && S.v[i].track == 0 && S.v[i].note == 0) kslot = i;
        for (int k = 0; k < 6; k++) { snd_note_on(&S, GB_OWNER(OWN_SEQ, 1), 1, 40 + (beat * 7 + k * 5) % 40, 0.8f); run(2, 0, 0); }
        if (kslot < 0 || S.v[kslot].track != 0 || S.v[kslot].note != 0 || S.v[kslot].state == V_FREE) kicks_cut++;
        run(10, 0, 0);
    }
    printf("  steals %u, kicks cut %d\n", S.steals, kicks_cut);
    CHECK(kicks_cut == 0, "%d kicks cut", kicks_cut);
}

/* G14: the load guard keeps the voices within its budget, fades without a click, spares the protected track */
static void guard_test(void)
{
    printf("G14 the load guard\n");
    reset(); va_patch(1, 0.6f, 0.0f, 0.9f); S.trk[1].patch.poly = 32;
    gb_patch *d = &S.trk[0].patch; gb_patch_default(d, ENG_DRUM); d->poly = 8; d->protect = 1;
    S.voice_cap = 6;
    snd_note_on(&S, GB_OWNER(OWN_SEQ, 0), 0, 0, 1.0f);                /* the kick */
    for (int k = 0; k < 12; k++) { snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 48 + k, 0.8f); run(1, 0, 0); }
    CHECK(snd_voices(&S) <= 6, "%d voices over a budget of 6", snd_voices(&S));
    float steady, step;
    run(20, &steady, 0);
    int shed = 0; while (snd_voices(&S) > 2 && snd_shed(&S)) shed++;
    run(4, &step, 0);
    printf("  %d voices faded, step %.4f against %.4f\n", shed, step, steady);
    CHECK(step < steady * 1.6f + 1e-3f, "the guard clicked: %.4f against %.4f", step, steady);
    int kick = 0; for (int i = 0; i < GB_NVOICE; i++) if (S.v[i].state != V_FREE && S.v[i].track == 0) kick = 1;
    CHECK(kick, "the guard faded the protected kick before the others");
    S.voice_cap = 0;
}

/* G8: every note is higher than the one below, and in tune */
static void pitch_test(void)
{
    printf("G8 pitch 0..127\n");
    float prev = 0, worst = 0;
    for (int n = 0; n < 128; n++) {
        float f = gb_note_hz((float)n), ref = 440.0f * powf(2.0f, ((float)n - 69.0f) / 12.0f);
        float cents = 1200.0f * log2f(f / ref); if (fabsf(cents) > worst) worst = fabsf(cents);
        CHECK(f > prev, "note %d not above note %d", n, n - 1);
        prev = f;
    }
    printf("  worst error %.4f cents\n", worst);
    CHECK(worst < 0.05f, "pitch error %.3f cents", worst);
    float ws = 0;
    for (int i = 0; i < 1000; i++) { float t = (float)i / 1000.0f, e = fabsf(gb_sin_turn(t) - sinf(t * 6.2831853f)); if (e > ws) ws = e; }
    printf("  sine error %.2e\n", ws);
    CHECK(ws < 1e-5f, "sine error %.2e", ws);
}

/* UNISON: four voices at random phases about as loud as one */
static float rms_of(int uni)
{
    reset(); va_patch(0, 0.5f, 0.0f, 0.3f);
    if (uni > 1) { gb_patch_set(&S.trk[0].patch, SP_VOICE, VM_UNISON); S.trk[0].patch.poly = (uint8_t)uni; }
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 0, 48, 1.0f);
    run(40, 0, 0);
    double acc = 0; int n = 0;
    for (int b = 0; b < 200; b++) { snd_render(&S, out); for (int i = 0; i < GB_BLOCK; i++) { acc += 0.5 * (snd_last_l[i] * snd_last_l[i] + snd_last_r[i] * snd_last_r[i]); n++; } }
    return sqrtf((float)(acc / n));
}
static void unison_test(void)
{
    printf("UNISON level\n");
    float r1 = rms_of(1), r4 = rms_of(4), db = 20.0f * log10f(r4 / r1);
    printf("  one voice %.4f, four %.4f: %+.1f dB\n", r1, r4, db);
    CHECK(fabsf(db) < 3.0f, "unison is %+.1f dB from one voice", db);
    int n = 0; for (int i = 0; i < GB_NVOICE; i++) n += S.v[i].state == V_ON;
    CHECK(n == 4, "UNISON 4: %d voices", n);
}

static void output_test(void)
{
    printf("G1 output\n");
    reset(); va_patch(0, 0.4f, 0.0f, 0.9f);
    sm_jump(&S.trk[0].level, 0.0003f);                    /* about -75 dBFS */
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 0, 60, 1.0f);
    double err = 0; int n = 0;
    for (int b = 0; b < 300; b++) {
        snd_render(&S, out);
        for (int i = 0; i < GB_BLOCK; i++) { double e = fabs((double)out[2 * i] - (double)snd_last_l[i] * 8388607.0); if (e > err) err = e; n++; }
    }
    CHECK(err <= 0.5001, "conversion error %.3f LSB", err);
    reset();
    for (int t = 0; t < GB_NTRACK; t++) { va_patch(t, 1.0f, 0.0f, 0.5f); S.trk[t].patch.p[14] = 1.0f; sm_jump(&S.trk[t].level, 1.0f); }
    sm_jump(&S.master, 1.0f);
    for (int t = 0; t < GB_NTRACK; t++) for (int k = 0; k < 4; k++) snd_note_on(&S, GB_OWNER(OWN_KEY, t), t, 30 + k * 7, 1.0f);
    int32_t mx = 0;
    for (int b = 0; b < 200; b++) { snd_render(&S, out); for (int i = 0; i < 2 * GB_BLOCK; i++) { int32_t a = out[i] < 0 ? -out[i] : out[i]; if (a > mx) mx = a; } }
    printf("  loudest mix: peak %d of 8388607\n", mx);
    CHECK(mx <= 8388607, "the output wrapped (%d)", mx);
    /* silence after it all: no NaN, no denormal hiss, comes back to zero */
    snd_all_off(&S); float pk; run(200, 0, 0); run(4000, 0, &pk);
    printf("  after 6 s of silence: peak %.2e\n", pk);
    CHECK(pk < 1e-4f, "the silence is not silent (%.2e)", pk);
}

/* two cores: the voices split in two parts, each into its own bus, then mixed, give the one-core block (to float
 * rounding: the sums are in another order) */
static gb_sound S2;
static int16_t fxmem2[GB_FX_MEM];
static gb_bus B0, B1;
static int32_t split_run(int sends)
{
    snd_init(&S, fxmem, 0); snd_init(&S2, fxmem2, 0);
    gb_sound *ss[2] = { &S, &S2 };
    for (int k = 0; k < 2; k++) {
        gb_sound *x = ss[k];
        for (int t = 0; t < 3; t++) {
            gb_patch *q = &x->trk[t].patch; q->engine = t == 0 ? ENG_DRUM : ENG_VA; q->poly = 8; q->unison = t == 2 ? 3 : 1;
            for (int i = 0; i < GB_NPARAM; i++) q->p[i] = 0.3f + 0.04f * (float)i;   /* (the LFO on too) */
            q->p[SP_ITYPE] = q->p[SP_SLCR] = 0.0f;   /* (the SLICER's and the INSERT's state is the program's: one sound) */
            if (sends) { sm_jump(&x->trk[t].send_rev, 0.3f); sm_jump(&x->trk[t].send_dly, 0.2f); }
        }
    }
    int32_t o1[GB_BLOCK * 2], o2[GB_BLOCK * 2];
    int32_t worst = 0; int moved = 0;
    for (int b = 0; b < 600; b++) {
        if (b % 25 == 0) for (int k = 0; k < 2; k++) {
            gb_owner o = GB_OWNER(OWN_SEQ, 0);
            snd_note_on(ss[k], o, 0, b / 25 % 16, 0.9f);
            snd_note_on(ss[k], o, 1, 36 + b / 25 % 12, 0.8f);
            snd_note_on(ss[k], o, 2, 60 + b / 50 % 7, 0.7f);
            if (b >= 100) { snd_note_off(ss[k], o, 1, 36 + (b / 25 - 4) % 12); snd_note_off(ss[k], o, 2, 60 + (b / 50 - 2) % 7); }
        }
        snd_render(&S, o1);
        snd_render_voices(&S2, &B0, 0, 2); snd_render_voices(&S2, &B1, 1, 2); snd_mix(&S2, &B0, &B1, o2);
        for (int i = 0; i < 2 * GB_BLOCK; i++) { int32_t d = o1[i] - o2[i]; d = d < 0 ? -d : d; if (d > worst) worst = d; if (o1[i]) moved = 1; }
    }
    CHECK(moved, "the test made no sound");
    return worst;
}
static void split_test(void)
{
    printf("two cores: the block in two halves\n");
    int32_t dry = split_run(0), wet = split_run(1);
    printf("  largest difference: %d (dry), %d (with the effects) of 8388607\n", dry, wet);
    CHECK(dry <= 4, "the two-core block differs by %d", dry);   /* (sums in another order: a few LSB of 24 bits, -126 dB) */
    /* with the effects, a sum in another order can round a 16-bit line value the other way, and the feedback
     * carries it on: still far below hearing (-66 dB at most) */
    CHECK(wet <= 4096, "the two-core block with effects differs by %d", wet);
}

/* the LFO: a tremolo at 4 Hz makes the level breathe 4 times a second; at depth 0 the sound is as without it */
static double block_rms(const int32_t *o) { double a = 0; for (int j = 0; j < GB_BLOCK * 2; j++) { double v = o[j] / 8388608.0; a += v * v; } return a; }
static int lfo_render(int rate, int wave, int pit, int amp, double *rms, int nblk)
{
    snd_init(&S, fxmem, 0); sm_jump(&S.master, 1.0f);
    gb_patch *p = &S.trk[1].patch; gb_patch_default(p, ENG_VA); p->poly = 2;
    gb_patch_set(p, SP_E4, 115); gb_patch_set(p, SP_SUS, 127); gb_patch_set(p, SP_ATK, 0); gb_patch_set(p, SP_E2, 0);
    gb_patch_set(p, SP_LRATE, rate); gb_patch_set(p, SP_LWAVE, wave); gb_patch_set(p, SP_LPIT, pit); gb_patch_set(p, SP_LAMP, amp);
    sm_jump(&S.trk[1].level, 1.0f); sm_jump(&S.trk[1].send_rev, 0.0f); sm_jump(&S.trk[1].send_dly, 0.0f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 81, 0.8f);   /* (high: a block holds periods whole enough)*/
    int32_t o[GB_BLOCK * 2];
    for (int b = 0; b < nblk; b++) { snd_render(&S, o); rms[b] = block_rms(o); }
    return 0;
}
static void lfo_test(void)
{
    printf("LFO: tremolo, vibrato, off at depth 0\n");
    enum { NB = 44100 / GB_BLOCK };                        /* a second */
    static double a[NB], b[NB];
    int r4 = 0; while (gb_lfo_hz((float)r4) < 4.0f) r4++;               /* RATE for 4 Hz */
    lfo_render(r4, 0, 0, 127, a, NB);
    /* (the level over 8 blocks, ~12 ms: a 64-frame block alone is shorter than the note's period) */
    for (int i = 0; i + 8 <= NB; i++) { double t = 0; for (int j = 0; j < 8; j++) t += a[i + j]; a[i] = t; }
    for (int i = NB - 7; i < NB; i++) a[i] = a[NB - 8];
    /* count the level's dips: local minima well under the mean, after the attack */
    double mean = 0; for (int i = 100; i < NB; i++) mean += a[i]; mean /= (NB - 100);
    int dips = 0, below = 0; for (int i = 100; i < NB; i++) { int lo = a[i] < 0.4 * mean; if (lo && !below) dips++; below = lo; }
    CHECK(dips >= 3 && dips <= 5, "%d dips in a second at 4 Hz (want ~4)", dips);
    lfo_render(r4, 0, 0, 0, b, NB);
    static double bw[NB];
    for (int i = 0; i + 8 <= NB; i++) { double t = 0; for (int j = 0; j < 8; j++) t += b[i + j]; bw[i] = t; }
    double mx = 0, mn = 1e9; for (int i = 100; i + 8 <= NB; i++) { if (bw[i] > mx) mx = bw[i]; if (bw[i] < mn) mn = bw[i]; }
    CHECK(mx < mn * 1.5, "depth 0: the level steady (%.3g .. %.3g)", mn, mx);
    /* vibrato: the same note sounds different with it, and returns (no drift): compare against no LFO */
    static double c[NB];
    lfo_render(r4, 0, 20, 0, c, NB);
    int differ = 0; for (int i = 0; i < NB; i++) if (c[i] != b[i]) differ = 1;
    CHECK(differ, "the vibrato changed nothing");
}


/* MONO, LEGATO, GLIDE and PRIO: one note at a time; legato keeps its voice; the glide slides; let go, back */
static int on_voices(int t, int *note, uint32_t *age)
{
    int n = 0;
    for (int i = 0; i < GB_NVOICE; i++) if (S.v[i].state == V_ON && S.v[i].track == t) { n++; if (note) *note = S.v[i].note; if (age) *age = S.v[i].age; }
    return n;
}
static void mono_test(void)
{
    printf("voices: MONO, LEGATO, GLIDE, PRIO\n");
    gb_owner k = GB_OWNER(OWN_KEY, 0);
    int note; uint32_t a1, a2;
    reset(); va_patch(1, 0.8f, 0.0f, 0.2f); gb_patch_set(&S.trk[1].patch, SP_VOICE, VM_MONO);
    snd_note_on(&S, k, 1, 60, 0.8f); run(2, 0, 0);
    snd_note_on(&S, k, 1, 64, 0.8f); run(2, 0, 0);
    CHECK(on_voices(1, &note, &a1) == 1 && note == 64, "MONO: one voice, the new note (%d voices)", on_voices(1, 0, 0));
    snd_note_off(&S, k, 1, 64); run(1, 0, 0);
    CHECK(on_voices(1, &note, &a2) == 1 && note == 60 && a2 == a1, "MONO: let go, back to the held note, legato (SLOOP)");
    snd_note_off(&S, k, 1, 60); run(1, 0, 0);
    CHECK(on_voices(1, 0, 0) == 0, "MONO: all let go, released");
    reset(); va_patch(1, 0.8f, 0.0f, 0.2f); gb_patch_set(&S.trk[1].patch, SP_VOICE, VM_LEGATO);
    snd_note_on(&S, k, 1, 60, 0.8f); run(4, 0, 0); on_voices(1, 0, &a1);
    snd_note_on(&S, k, 1, 67, 0.8f); run(1, 0, 0);
    CHECK(on_voices(1, &note, &a2) == 1 && note == 67 && a2 == a1, "LEGATO: the same voice moved to the new note");
    float maxstep; run(1, &maxstep, 0);
    CHECK(maxstep < 0.2f, "LEGATO: no jump (%.3f)", maxstep);
    reset(); va_patch(1, 0.8f, 0.0f, 0.2f); gb_patch_set(&S.trk[1].patch, SP_VOICE, VM_LEGATO); gb_patch_set(&S.trk[1].patch, SP_GLIDE, 64);
    snd_note_on(&S, k, 1, 48, 0.8f); run(2, 0, 0);
    snd_note_on(&S, k, 1, 60, 0.8f); run(4, 0, 0);
    float pm = 0; for (int i = 0; i < GB_NVOICE; i++) if (S.v[i].state == V_ON) pm = S.v[i].pitch;
    CHECK(pm > 48.5f && pm < 59.5f, "GLIDE: on its way after 6 ms (%.2f)", pm);
    run(400, 0, 0);
    for (int i = 0; i < GB_NVOICE; i++) if (S.v[i].state == V_ON) pm = S.v[i].pitch;
    CHECK(fabsf(pm - 60.0f) < 0.01f, "GLIDE: there in the end (%.3f)", pm);
    reset(); va_patch(1, 0.8f, 0.0f, 0.2f); gb_patch_set(&S.trk[1].patch, SP_VOICE, VM_MONO); gb_patch_set(&S.trk[1].patch, SP_PRIO, PR_LOW);
    snd_note_on(&S, k, 1, 60, 0.8f); snd_note_on(&S, k, 1, 67, 0.8f); run(1, 0, 0);
    CHECK(on_voices(1, &note, 0) == 1 && note == 60, "PRIO LOW: the lowest sounds (%d)", note);
    snd_note_on(&S, k, 1, 55, 0.8f); run(1, 0, 0);
    CHECK(on_voices(1, &note, 0) == 1 && note == 55, "PRIO LOW: a lower one takes over (%d)", note);
    snd_owner_off(&S, k); run(1, 0, 0);
    CHECK(on_voices(1, 0, 0) == 0 && S.trk[1].mono_n == 0, "MONO: panic leaves nothing held");
}

/* the master bus: at zero, nothing changes (to the bit); each does what it says; the punch-ins come and go */
static void play_some(void)                    /* a bright saw chord on track 1 and a beat on track 3 */
{
    va_patch(1, 1.0f, 0.0f, 0.3f); sm_jump(&S.trk[1].level, 0.6f);
    S.trk[3].patch.engine = ENG_DRUM; sm_jump(&S.trk[3].level, 0.7f);
    gb_owner k = GB_OWNER(OWN_KEY, 0);
    snd_note_on(&S, k, 1, 48, 0.8f); snd_note_on(&S, k, 1, 55, 0.8f); snd_note_on(&S, k, 1, 64, 0.8f);
}
static double hf_energy(int blocks)            /* the sum of squared differences: the highs */
{
    double e = 0; float prev = 0;
    for (int b = 0; b < blocks; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) { float d = snd_last_l[n] - prev; e += d * d; prev = snd_last_l[n]; } }
    return e;
}
static double blk_rms(int blocks)
{
    double e = 0;
    for (int b = 0; b < blocks; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) e += snd_last_l[n] * snd_last_l[n]; }
    return sqrt(e / (blocks * GB_BLOCK));
}
static void master_test(void)
{
    printf("master: DUST, DUCK, the filters, the throws\n");
    static int32_t ref[200][GB_BLOCK * 2];
    reset(); sm_jump(&S.master, 1.0f); play_some();
    for (int b = 0; b < 200; b++) snd_render(&S, ref[b]);
    reset(); sm_jump(&S.master, 1.0f); play_some(); S.dust = 0.0f; S.duck = 0.0f; sm_jump(&S.mfilt, 0.0f);
    int same = 1; for (int b = 0; b < 200; b++) { snd_render(&S, out); if (memcmp(out, ref[b], sizeof out)) same = 0; }
    CHECK(same, "everything at zero: not the same sound");
    reset(); sm_jump(&S.master, 1.0f); play_some(); run(20, 0, 0); double h0 = hf_energy(200);
    reset(); sm_jump(&S.master, 1.0f); play_some(); sm_jump(&S.mfilt, -0.7f); run(20, 0, 0); double h1 = hf_energy(200);
    reset(); sm_jump(&S.master, 1.0f); play_some(); sm_jump(&S.trk[1].filt, -0.7f); run(20, 0, 0); double h2 = hf_energy(200);
    reset(); sm_jump(&S.master, 1.0f); play_some(); sm_jump(&S.mfilt, 0.7f); run(20, 0, 0); double r3 = blk_rms(200);
    reset(); sm_jump(&S.master, 1.0f); play_some(); run(20, 0, 0); double r0 = blk_rms(200);
    printf("  highs: %.3g open, %.3g master LP, %.3g track LP; level %.3g open, %.3g master HP\n", h0, h1, h2, r0, r3);
    CHECK(h1 < h0 * 0.1 && h2 < h0 * 0.1, "the low-pass did not take the highs");
    CHECK(r3 < r0 * 0.5, "the high-pass did not take the body");
    /* the filter swept from end to end: no click */
    reset(); sm_jump(&S.master, 1.0f); play_some(); run(10, 0, 0);
    float worst = 0, ms;
    for (int i = 0; i <= 40; i++) { sm_set(&S.mfilt, -1.0f + 0.05f * (float)i); run(4, &ms, 0); if (ms > worst) worst = ms; }
    CHECK(worst < 0.25f, "the filter swept: a step of %.3f", worst);
    reset(); sm_jump(&S.master, 1.0f); play_some(); S.dust = 1.0f; S.playing = 1; run(20, 0, 0);
    float pk; run(200, &ms, &pk);
    CHECK(pk > 0.01f && pk < 1.0f, "DUST: the sound goes on (peak %.3f)", pk);
    /* DUCK: the synths dip after a kick, and come back */
    reset(); sm_jump(&S.master, 1.0f); va_patch(1, 0.5f, 0.0f, 0.3f); sm_jump(&S.trk[1].level, 0.6f); S.trk[3].patch.engine = ENG_DRUM;
    sm_jump(&S.trk[3].level, 0.0f); S.duck = 1.0f; S.beat = 22050.0f;
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 57, 0.8f); run(40, 0, 0);
    double before = blk_rms(20);
    snd_note_on(&S, GB_OWNER(OWN_SEQ, 3), 3, 0, 1.0f);
    double during = blk_rms(20); run(200, 0, 0); double after = blk_rms(20);
    printf("  DUCK: %.3g before, %.3g after the kick, %.3g later\n", before, during, after);
    CHECK(during < before * 0.5 && after > before * 0.8, "DUCK did not pump");
    /* the chorus: a send, so the dry centre gains a wide wet image (left and right differ) */
    reset(); sm_jump(&S.master, 1.0f); va_patch(1, 0.6f, 0.0f, 0.3f); sm_jump(&S.trk[1].level, 0.6f); sm_jump(&S.trk[1].send_cho, 1.0f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 57, 0.8f); run(60, 0, 0);
    double side = 0; for (int i = 0; i < 100; i++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) side += fabsf(snd_last_l[n] - snd_last_r[n]); }
    CHECK(side > 1.0, "the chorus made no stereo (%.3g)", side);
    /* the echo throw: no send, and still an echo while held */
    reset(); sm_jump(&S.master, 1.0f); va_patch(1, 0.6f, 0.0f, 0.02f); sm_jump(&S.trk[1].level, 0.6f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 57, 0.8f); run(20, 0, 0); snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 1, 57); run(100, 0, 0);
    double dry = blk_rms(300);
    reset(); sm_jump(&S.master, 1.0f); va_patch(1, 0.6f, 0.0f, 0.02f); sm_jump(&S.trk[1].level, 0.6f); sm_set(&S.throw_dly, 1.0f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 57, 0.8f); run(20, 0, 0); snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 1, 57); run(100, 0, 0);
    double thrown = blk_rms(300);
    CHECK(thrown > dry * 5 + 1e-4, "the echo throw made no echo (%.3g vs %.3g)", thrown, dry);
}
static void punch_test(void)
{
    printf("punch-ins: a loop repeats, all 27 come and go cleanly\n");
    /* LOOP 1/16 stopped: after its capture, the output repeats every 1/16 */
    reset(); sm_jump(&S.master, 1.0f); S.beat = 22050.0f; S.pk_fade = 0;
    va_patch(1, 0.7f, 0.0f, 0.02f); sm_jump(&S.trk[1].level, 0.6f); sm_jump(&S.trk[1].send_rev, 0.0f);
    gb_owner k = GB_OWNER(OWN_KEY, 0);
    static float o[400 * GB_BLOCK];
    int note = 40;
    snd_punch(&S, PK_LOOP16);
    for (int b = 0; b < 400; b++) {
        if (b % 7 == 0) { snd_note_off(&S, k, 1, note); note = 40 + (b * 5) % 24; snd_note_on(&S, k, 1, note, 0.8f); }
        snd_render(&S, out);
        for (int n = 0; n < GB_BLOCK; n++) o[b * GB_BLOCK + n] = snd_last_l[n];
    }
    uint32_t L = S.pk.len;
    double diff = 0, en = 0;
    for (int i = 14000; i < 14000 + 4000; i++) { if ((i % L) < 40 || (i % L) > L - 40) continue; diff += fabs(o[i] - o[i + L]); en += fabs(o[i]); }
    CHECK(L == 5512 && S.pk.stage == 2 && diff < en * 0.01, "LOOP 1/16: the 1/16 again (len %u, %.3g of %.3g)", L, diff, en);
    snd_punch(&S, -1); run(10, 0, 0);
    CHECK(S.pk.fx < 0 && S.pk.lend == 0, "LOOP let go: the delay's memory back");
    /* every effect: held 0.3 s, let go: no NaN, no click, back to rest */
    float worst = 0; int bad = -1, idle = 0;
    static float ref[200 * GB_BLOCK];
    for (int fx = -1; fx < PK_N; fx++) {
        reset(); sm_jump(&S.master, 1.0f); S.beat = 11025.0f;     /* (240 BPM: the echo heard within the window) */ S.grid = 0.0f; play_some(); sm_jump(&S.trk[1].send_rev, 0.3f); sm_jump(&S.trk[1].send_dly, 0.3f);
        snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 3, 0, 1.0f);
        run(60, 0, 0);
        float ms, pk, m = 0; double d = 0;
        snd_punch(&S, fx);
        for (int b = 0; b < 200; b++) {
            S.grid += GB_BLOCK; run(1, &ms, &pk); if (ms > m) m = ms;
            for (int n = 0; n < GB_BLOCK; n++) { if (fx < 0) ref[b * GB_BLOCK + n] = snd_last_l[n]; else d += fabs(snd_last_l[n] - ref[b * GB_BLOCK + n]); }
        }
        if (fx < 0) continue;
        if (d < 1.0) { idle++; printf("  effect %d changed nothing (%.3g)\n", fx, d); }
        snd_punch(&S, -1);
        for (int b = 0; b < 40; b++) { S.grid += GB_BLOCK; run(1, &ms, &pk); if (ms > m) m = ms; }
        if (S.pk.fx >= 0 || S.freeze || S.hist_stop || S.pk.lend) { CHECK(0, "punch-in %d did not come back to rest", fx); }
        if (m > worst) { worst = m; bad = fx; }
    }
    printf("  the largest step: %.3f (effect %d)\n", worst, bad);
    CHECK(!idle, "%d punch-ins changed nothing", idle);
    CHECK(worst < 0.6f, "a punch-in clicked: a step of %.3f (effect %d)", worst, bad);
}

/* every engine: notes low to high, every choice of its first parameters: no NaN, bounded, silent once released;
 * and what a voice costs here against the VA (the FM-1 is ~ the same ratio) */
#include <time.h>
static void engines_test(void)
{
    printf("engines: every one sounds, stays bounded, ends\n");
    for (int e = 0; e < ENG_N; e++) {
        if (e == ENG_DRUM) continue;
        float worst = 0; int bad = 0, stuck = 0;
        for (int trial = 0; trial < 12; trial++) {
            reset(); sm_jump(&S.master, 1.0f);
            gb_patch *p = &S.trk[1].patch; gb_patch_default(p, e);
            uint32_t r = 77u + (uint32_t)trial * 911u;
            for (int i = 0; i < SP_N; i++) { r ^= r << 13; r ^= r >> 17; r ^= r << 5; p->p[i] = (float)(r >> 8) * (1.0f / 16777216.0f); }
            gb_patch_set(p, SP_ATK, 10); gb_patch_set(p, SP_DEC, 64); gb_patch_set(p, SP_SUS, 90); gb_patch_set(p, SP_REL, 40);
            gb_patch_set(p, SP_VOICE, trial % 4); gb_patch_set(p, SP_LSYNC, 0); gb_patch_set(p, SP_ESYNC, 0); gb_patch_set(p, SP_GLIDE, 20);
            sm_jump(&S.trk[1].level, 1.0f); sm_jump(&S.trk[1].send_rev, 0.0f); sm_jump(&S.trk[1].send_dly, 0.0f);
            int note = 24 + trial * 8;
            snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, note, 1.0f);
            for (int b = 0; b < 200; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) { float x = snd_last_l[n]; if (x != x) bad = 1; if (fabsf(x) > worst) worst = fabsf(x); } }
            snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 1, note);
            for (int b = 0; b < 3000; b++) snd_render(&S, out);
            if (snd_voices(&S)) stuck++;
        }
        CHECK(!bad && !stuck, "%s: NaN %d, voices left %d", gb_engines[e]->name, bad, stuck);
        /* the cost: 16 voices, 2000 blocks */
        reset(); gb_patch *p = &S.trk[1].patch; gb_patch_default(p, e); p->poly = 16;
        gb_patch_set(p, SP_SUS, 127);
        for (int k = 0; k < 16; k++) snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 40 + k * 2, 0.8f);
        clock_t t0 = clock(); for (int b = 0; b < 2000; b++) snd_render_voices(&S, &dbg_bus, 0, 1); double us = (double)(clock() - t0) / CLOCKS_PER_SEC * 1e6 / (2000.0 * 16.0);
        printf("  %-8s peak %.2f, a voice %.2f us a block\n", gb_engines[e]->name, worst, us);
    }
}


/* GRAIN: every set sounds (forwards, backwards, dense and long), bounded; silent while the flash window is off; a
 * third and a fourth track of GRAIN (two pools) stay silent without harm */
#include "../engine/sample.h"
static void grain_test(void)
{
    printf("grain: the sets, reversed, dense; the window off; four tracks\n");
    /* the bank's seek index: the decoder's state every 256 samples, as decoding from the zone's start gives it */
    int seekbad = 0, nz = 0;
    for (int set = 0; set < smp_nsets(); set++) {
        const smp_set *st = smp_set_at(set);
        for (int zi = st->z0; zi < st->z0 + st->nz; zi++, nz++) {
            const smp_zone *z = smp_zone_at(zi); const uint8_t *d = smp_data() + z->off;
            int pr = 0, ix = 0;
            for (uint32_t k = 0; k < z->n; k++) {
                if (!(k & ((1u << SMP_SEEK_SHIFT) - 1u))) { uint32_t e = smp_seek(zi, k >> SMP_SEEK_SHIFT); if ((int16_t)(e & 0xFFFF) != pr || (int)(e >> 16) != ix) seekbad++; }
                uint32_t b = d[k >> 1], c = (k & 1u) ? b >> 4 : b & 15u; int stp = smp_ima_step[ix], vd = stp >> 3;
                vd += (c & 4u ? stp : 0) + (c & 2u ? stp >> 1 : 0) + (c & 1u ? stp >> 2 : 0);
                pr += (c & 8u) ? -vd : vd; pr = pr < -32768 ? -32768 : pr > 32767 ? 32767 : pr;
                ix += smp_ima_idx[c & 7u]; ix = ix < 0 ? 0 : ix > 88 ? 88 : ix;
            }
        }
    }
    CHECK(nz > 40 && !seekbad, "the seek index: %d zones, %d wrong points", nz, seekbad);
    float lo = 1e9f, hi = 0.0f; int nan = 0;
    for (int set = 0; set < 8; set++)
        for (int mode = 0; mode < 3; mode++) {
            reset(); sm_jump(&S.master, 1.0f); sm_jump(&S.trk[1].level, 1.0f);
            gb_patch *p = &S.trk[1].patch; gb_patch_default(p, ENG_GRAIN);
            gb_patch_set(p, SP_E0, set); gb_patch_set(p, SP_SUS, 127);
            if (mode == 1) gb_patch_set(p, SP_E6, 127);                               /* RAND: half backwards, detuned */
            if (mode == 2) { gb_patch_set(p, SP_E2, 127); gb_patch_set(p, SP_E3, 127); gb_patch_set(p, SP_E4, 24); }
            int note = set == 7 ? 38 : 60;
            snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, note, 1.0f);
            double a = 0; float pk = 0;
            for (int b = 0; b < 400; b++) {
                snd_render(&S, out);
                for (int n = 0; n < GB_BLOCK; n++) { float x = snd_last_l[n]; if (x != x) nan = 1; if (b >= 100) a += (double)x * x; if (fabsf(x) > pk) pk = fabsf(x); }
            }
            float r = (float)sqrt(a / (300.0 * GB_BLOCK));
            if (r < lo) lo = r;
            if (pk > hi) hi = pk;
        }
    printf("  rms at least %.4f, peak %.3f\n", lo, hi);
    CHECK(!nan && lo > 0.002f && hi < 4.0f, "grain: NaN %d, the quietest %.4f, the peak %.3f", nan, lo, hi);
    /* the window off: nothing read, silence; back on: it sounds again */
    reset(); gb_patch_default(&S.trk[1].patch, ENG_GRAIN); gb_patch_set(&S.trk[1].patch, SP_SUS, 127);
    sm_jump(&S.trk[1].send_rev, 0.0f); sm_jump(&S.trk[1].send_dly, 0.0f); sm_jump(&S.trk[1].send_cho, 0.0f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 60, 1.0f);
    for (int b = 0; b < 100; b++) snd_render(&S, out);
    gb_flash_ok = 0;
    for (int b = 0; b < 20; b++) snd_render(&S, out);
    double off = 0; for (int b = 0; b < 20; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) off += fabs(snd_last_l[n]); }
    gb_flash_ok = 1;
    double on = 0; for (int b = 0; b < 200; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) on += fabs(snd_last_l[n]); }
    CHECK(off < 1e-3 && on > 1.0, "grain: the window off %.4g, back on %.4g", off, on);
    /* four tracks of GRAIN at once */
    reset(); nan = 0;
    for (int t = 0; t < 4; t++) { gb_patch_default(&S.trk[t].patch, ENG_GRAIN); gb_patch_set(&S.trk[t].patch, SP_SUS, 127); snd_note_on(&S, GB_OWNER(OWN_KEY, 0), t, 48 + 5 * t, 1.0f); }
    for (int b = 0; b < 300; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) if (snd_last_l[n] != snd_last_l[n]) nan = 1; }
    CHECK(!nan, "grain: four tracks");
}

/* the kits: every sound of every kit sounds, bounded, and ends; the kits about as loud as each other (SLOOP measured
 * its trims); the closed hat ends the open one; a kit at hand during a save */
static void kits_test(void)
{
    printf("kits: %d, their 16 sounds\n", GB_NKITS);
    float worst = 0.0f, lo = 1e9f, hi = -1e9f; int nan = 0, silent = 0, stuck = 0, klo = 0, khi = 0;
    for (int kit = 0; kit < GB_NKITS; kit++) {
        if (kit >= GB_KIT_USR && kit < GB_KIT_SYN) continue;   /* (the user slots' kits: empty here) */
        double e = 0;                                   /* (the loudness: the sounds' peaks, as SLOOP levels its kits) */
        for (int l = 0; l < 16; l++) {
            reset(); sm_jump(&S.master, 1.0f); sm_jump(&S.trk[3].level, 1.0f);
            sm_jump(&S.trk[3].send_rev, 0.0f); sm_jump(&S.trk[3].send_dly, 0.0f); sm_jump(&S.trk[3].send_cho, 0.0f);
            gb_patch *p = &S.trk[3].patch; gb_patch_default(p, ENG_DRUM); gb_patch_set(p, SP_E0, kit);
            snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 3, l, 1.0f);
            snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 3, l);
            double a = 0; float pk = 0.0f;
            for (int b = 0; b < 4200; b++) {
                snd_render(&S, out);
                for (int n = 0; n < GB_BLOCK; n++) { float x = snd_last_l[n]; if (x != x) nan = 1; if (fabsf(x) > pk) pk = fabsf(x); if (b < 350) a += (double)x * x; }
                if (b > 200 && !snd_voices(&S)) break;
            }
            if (snd_voices(&S)) { stuck++; printf("  kit %d sound %d still sounds after 6 s\n", kit, l); }
            if (a < 1e-6) { silent++; printf("  kit %d sound %d silent\n", kit, l); }
            if (pk > worst) worst = pk;
            e += 20.0 * log10(pk + 1e-9) / 16.0;
        }
        float db = (float)e;
        if (getenv("KITS")) printf("  %-8s %.1f dB\n", gb_dkit_names[kit], db);
        if (db < lo) { lo = db; klo = kit; }
        if (db > hi) { hi = db; khi = kit; }
    }
    printf("  peak %.2f; the kits' loudness %.1f dB (%s) .. %.1f dB (%s)\n", worst, lo, gb_dkit_names[klo], hi, gb_dkit_names[khi]);
    CHECK(!nan && !silent && !stuck && worst < 1.5f, "kits: NaN %d, silent %d, stuck %d, peak %.2f", nan, silent, stuck, worst);
    CHECK(hi - lo < 6.0f, "kits: %.1f dB between the quietest and the loudest", hi - lo);
    /* the choke */
    reset(); gb_patch_default(&S.trk[3].patch, ENG_DRUM);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 3, 5, 1.0f);
    for (int b = 0; b < 10; b++) snd_render(&S, out);
    int before = snd_voices(&S);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 3, 4, 1.0f);
    for (int b = 0; b < 40; b++) snd_render(&S, out);
    int open = 0; for (int i = 0; i < GB_NVOICE; i++) if (S.v[i].state != V_FREE && S.v[i].note == 5) open = 1;
    CHECK(before == 1 && !open, "the closed hat did not end the open one");
    /* a save: the kit at hand plays on */
    reset(); gb_patch_default(&S.trk[3].patch, ENG_DRUM); gb_patch_set(&S.trk[3].patch, SP_E0, 9);
    for (int b = 0; b < 4; b++) snd_render(&S, out);
    gb_flash_ok = 0;
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 3, 2, 1.0f);
    double a = 0; for (int b = 0; b < 100; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) a += fabs(snd_last_l[n]); }
    gb_flash_ok = 1;
    CHECK(a > 1.0, "a synthesised kit fell silent during a save (%.3g)", a);
}

/* the INSERT: every type changes the sound, bounded, and OFF / MIX 0 is the dry track bit for bit; the SLICER: GATE
 * closes the '.' steps on the transport's grid, STUT repeats the live step in them */
static float tfx_play(int ity, int mix, int slcr, int pat, float *step_rms, float *ref)
{
    reset(); sm_jump(&S.master, 1.0f); va_patch(1, 0.6f, 0.0f, 0.3f); sm_jump(&S.trk[1].level, 1.0f);
    sm_jump(&S.trk[1].send_rev, 0.0f); sm_jump(&S.trk[1].send_dly, 0.0f); sm_jump(&S.trk[1].send_cho, 0.0f);
    gb_patch *p = &S.trk[1].patch;
    gb_patch_set(p, SP_SUS, 127); gb_patch_set(p, SP_ITYPE, ity); gb_patch_set(p, SP_IMIX, mix);
    gb_patch_set(p, SP_SLCR, slcr); gb_patch_set(p, SP_SLPAT, pat); gb_patch_set(p, SP_SLRATE, 1);
    S.beat = 22050.0f; S.grid = 0.0f; S.bar = 0;                /* 120 BPM: a 1/16 step = 5512.5 frames */
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 48, 1.0f);
    float worst = 0.0f, d = 0.0f; double e[16] = { 0 };
    for (int b = 0; b < 1378; b++) {                           /* a bar */
        snd_render(&S, out);
        for (int n = 0; n < GB_BLOCK; n++) {
            float x = snd_last_l[n]; int f = b * GB_BLOCK + n;
            if (x != x || fabsf(x) > worst) worst = x != x ? 99.0f : fabsf(x);
            if (ref) { if (step_rms == 0) ref[f] = x; else d = gb_maxf(d, fabsf(x - ref[f])); }
            e[(int)((float)f / 5512.5f) & 15] += (double)x * x;
        }
        S.grid += GB_BLOCK; if (S.grid >= 4.0f * S.beat) { S.grid -= 4.0f * S.beat; S.bar++; }
    }
    if (step_rms) for (int k = 0; k < 16; k++) step_rms[k] = (float)sqrt(e[k] / 5512.5);
    return ref && step_rms ? d : worst;
}
static float tfx_ref[1378 * GB_BLOCK];
static void tfx_test(void)
{
    printf("insert and slicer\n");
    float st[16];
    tfx_play(0, 127, 0, 1, 0, tfx_ref);                         /* the dry track */
    float off = tfx_play(0, 127, 0, 1, st, tfx_ref), mix0 = tfx_play(1, 0, 0, 1, st, tfx_ref);
    CHECK(off == 0.0f && mix0 == 0.0f, "INSERT OFF / MIX 0 changed the track (%g %g)", off, mix0);
    int idle = 0; float worst = 0.0f;
    for (int ty = 1; ty <= 8; ty++) {
        float dd = tfx_play(ty, 127, 0, 1, st, tfx_ref), pk = tfx_play(ty, 127, 0, 1, 0, 0);
        if (dd < 0.01f) { idle++; printf("  insert %d changed nothing (%g)\n", ty, dd); }
        if (pk > worst) worst = pk;
    }
    CHECK(!idle && worst < 3.0f, "INSERT: %d types idle, peak %.2f", idle, worst);
    /* GATE, pattern 7 (x...x...x...x...) */
    tfx_play(0, 127, 1, 7, st, 0);
    float open = (st[0] + st[4] + st[8] + st[12]) * 0.25f, shut = (st[2] + st[6] + st[10] + st[14]) * 0.25f;
    printf("  GATE: open steps %.3f, closed %.4f\n", open, shut);
    CHECK(open > 0.05f && shut < open * 0.02f, "GATE: open %.3f closed %.4f", open, shut);
    /* STUT, pattern 6 (xxxx....xxxx....): the '.' steps hear the repeat, as loud as the live sound about */
    extern uint32_t tfx_decodes; tfx_decodes = 0;
    tfx_play(0, 127, 2, 6, st, 0);
    float live = (st[1] + st[2] + st[3]) / 3.0f, rep = (st[5] + st[6] + st[7]) / 3.0f;
    printf("  STUT: live %.3f, repeated %.3f; %u decodes in the bar (%u frames)\n", live, rep, tfx_decodes, 1378u * GB_BLOCK);
    CHECK(tfx_decodes < 1378u * GB_BLOCK, "STUT: the repeat decoded %u samples for a bar of %u frames (it must be about half)", tfx_decodes, 1378u * GB_BLOCK);
    CHECK(rep > live * 0.5f && rep < live * 1.5f, "STUT: live %.3f repeated %.3f", live, rep);
}

/* FM6's bank: a packed voice (the classic 128 bytes) read back as the patch it was packed from, and played */
#include "../engine/fm6.h"
static void fm6_bank_test(void)
{
    printf("fm6 bank: a packed voice\n");
    const fm6_patch *f = &fm6_factory[3]; uint8_t m[128]; memset(m, 0, sizeof m);
    for (int k = 0; k < 6; k++) {
        uint8_t *b = m + 17 * (5 - k); const fm6_op *o = &f->op[k];
        for (int j = 0; j < 4; j++) { b[j] = o->r[j]; b[4 + j] = o->l[j]; }
        b[8] = o->bp; b[9] = o->ld; b[10] = o->rd; b[11] = (uint8_t)(o->lc | o->rc << 2); b[12] = (uint8_t)(o->rs | o->det << 3);
        b[13] = (uint8_t)(o->ams | o->kvs << 2); b[14] = o->ol; b[15] = (uint8_t)(o->mode | o->fc << 1); b[16] = o->ff;
    }
    for (int j = 0; j < 4; j++) { m[102 + j] = f->pr[j]; m[106 + j] = f->pl[j]; }
    m[110] = (uint8_t)(f->alg - 1); m[111] = (uint8_t)(f->fb | f->oks << 3); m[112] = f->lfs; m[113] = f->lfd; m[114] = f->lpmd; m[115] = f->lamd;
    m[116] = (uint8_t)(f->lks | f->lfw << 1 | f->lpms << 4); m[117] = f->trnsp;
    fm6_user_set(1, 8 + 4, m);
    fm6_patch g = fm6_user[1]; g.name = f->name;
    CHECK(!memcmp(&g, f, sizeof g) && fm6_user_id[1] == 12, "FM6: the packed voice differs from its patch");
    reset(); gb_patch *p = &S.trk[1].patch; gb_patch_default(p, ENG_FM6); gb_patch_set(p, SP_E7, 12);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, 60, 1.0f);
    double a = 0; for (int b = 0; b < 200; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) a += fabs(snd_last_l[n]); }
    CHECK(a > 1.0, "FM6: the bank voice is silent");
}

/* the user slots (SLOOP's USR1..4): a slot made from the bank's piano and kit zones; SAMPLE, GRAIN and a user kit
 * play it */
static uint8_t uslot[SMP_USR_SIZE];
static double play_rms(int eng, int e0, int t, int note, int blocks)
{
    reset(); sm_jump(&S.master, 1.0f); sm_jump(&S.trk[t].level, 1.0f);
    gb_patch *p = &S.trk[t].patch; gb_patch_default(p, eng); gb_patch_set(p, SP_E0, e0); gb_patch_set(p, SP_SUS, 127);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), t, note, 1.0f);
    double a = 0; for (int b = 0; b < blocks; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) a += (double)snd_last_l[n] * snd_last_l[n]; }
    return sqrt(a / (blocks * (double)GB_BLOCK));
}
/* 3.3.6: a note's pitch, from the output's autocorrelation (the strongest lag past the first dip) */
static double pitch_of(int eng, int e0, int note)
{
    static float o[72 * GB_BLOCK];       /* (Boatswain: 72, not 64: the lags below read up to block 16 + 32 + 1199 samples) */
    reset(); sm_jump(&S.master, 1.0f); sm_jump(&S.trk[1].level, 1.0f);
    gb_patch *p = &S.trk[1].patch; gb_patch_default(p, eng); gb_patch_set(p, SP_E0, e0); gb_patch_set(p, SP_SUS, 127);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, note, 1.0f);
    for (int b = 0; b < 72; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) o[b * GB_BLOCK + n] = snd_last_l[n]; }
    const float *x = o + 16 * GB_BLOCK; int N = 32 * GB_BLOCK, best = 0; double bv = -1e9, r0 = 0;
    for (int i = 0; i < N; i++) r0 += (double)x[i] * x[i];
    static double R[1200]; int dipped = 0, d0 = 0;
    for (int l = 20; l < 1200; l++) { double r = 0; for (int i = 0; i < N; i++) r += (double)x[i] * x[i + l]; R[l] = r; if (!dipped && r < 0.2 * r0) { dipped = 1; d0 = l; } if (dipped && r > bv) bv = r; }
    for (int l = d0 + 1; dipped && l < 1199; l++) if (R[l] > 0.85 * bv && R[l] >= R[l - 1] && R[l] >= R[l + 1]) { best = l; break; }   /* (the first strong peak: no octave below) */
    return best ? 44100.0 / best : 0;
}
static void usr_test(void)
{
    {   /* SAMPLE and GRAIN read a zone's rate alike: the same note, the same pitch (GRAIN was 1.49 x too high) */
        double fs = pitch_of(ENG_SAMPLE, 0, 57), fg = pitch_of(ENG_GRAIN, 0, 57);
        printf("  A3: SAMPLE %.1f Hz, GRAIN %.1f Hz\n", fs, fg);
        CHECK(fs > 0 && fg > 0 && fabs(fg / fs - 1.0) < 0.03 && fabs(fs / 220.0 - 1.0) < 0.03, "GRAIN's pitch is not SAMPLE's (%.1f, %.1f Hz)", fg, fs);
    }
    printf("user slots\n");
    memset(uslot, 0xFF, sizeof uslot);
    const smp_set *pn = smp_set_at(0); int kit = -1;
    for (int i = 0; i < smp_nsets(); i++) if (smp_set_at(i)->kind == SK_KIT) kit = i;
    int zs[2] = { pn->z0, -1 };
    for (int i = 0; i < smp_set_at(kit)->nz; i++) { const smp_zone *z = smp_zone_at(smp_set_at(kit)->z0 + i); if (36 >= z->lo && 36 <= z->hi) zs[1] = smp_set_at(kit)->z0 + i; }
    uint32_t off = 0;
    memcpy(uslot, "FSMP", 4); uslot[4] = 1; uslot[5] = 0; uslot[6] = 2; memcpy(uslot + 8, "MYSET\0\0\0", 8);
    for (int i = 0; i < 2; i++) {
        smp_zone z = *smp_zone_at(zs[i]); uint32_t nb = (z.n + 1u) / 2u;
        if (nb > 30000u) { z.n = 60000u; nb = 30000u; if (z.le >= z.n) z.le = z.ls = z.n - 1u; }
        memcpy(uslot + 512 + off, smp_data() + z.off, nb); z.off = off; off += nb;
        if (i == 0) { z.lo = 48; z.hi = 100; } else { z.lo = 36; z.hi = 36; }
        memcpy(uslot + 32 + 28 * i, &z, 28);
    }
    uslot[16] = (uint8_t)off; uslot[17] = (uint8_t)(off >> 8); uslot[18] = (uint8_t)(off >> 16); uslot[19] = 0;
    smp_user_scan(1, uslot);
    char nm[9];
    CHECK(smp_user_nz(1) == 2 && !strcmp(smp_user_name(1, nm), "MYSET"), "the user slot was not taken (%d)", smp_user_nz(1));
    double a = play_rms(ENG_SAMPLE, smp_nsets() + 1, 1, 60, 200);
    double g = play_rms(ENG_GRAIN, smp_nsets() + 1, 1, 60, 600);
    double k = play_rms(ENG_DRUM, GB_KIT_USR + 1, 3, 0, 200);   /* (the kick key: GM 36) */
    double k0 = play_rms(ENG_DRUM, GB_KIT_USR + 1, 3, 2, 200);  /* (the snare: none in the slot) */
    printf("  SAMPLE %.4f, GRAIN %.4f, kit: kick %.4f, snare %.6f\n", a, g, k, k0);
    CHECK(a > 0.01 && g > 0.005 && k > 0.01 && k0 < 1e-6, "the user slot: SAMPLE %.4f GRAIN %.4f kick %.4f snare %.6f", a, g, k, k0);
    smp_user_scan(1, 0);
}

/* SLICE: BREAK's hits found; each slice sounds and ends at its length; ONE ignores the release, GATE does not; LOOP
 * repeats; a slice backwards is that slice forwards reversed (sample for sample, the decoder states found along it);
 * a user slot slices too */
static float slc_buf[96000];
static int slice_play(int e[8], int note, int hold_blocks, int blocks, float *keep)
{
    reset(); sm_jump(&S.master, 1.0f); sm_jump(&S.trk[1].level, 1.0f);
    gb_patch *p = &S.trk[1].patch; gb_patch_default(p, ENG_SLICE);
    for (int k = 0; k < 8; k++) gb_patch_set(p, SP_E0 + k, e[k]);
    gb_patch_set(p, SP_ATK, 0); gb_patch_set(p, SP_SUS, 127); gb_patch_set(p, SP_REL, 0);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, note, 1.0f);
    int last = -1;
    for (int b = 0; b < blocks; b++) {
        if (b == hold_blocks) snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 1, note);
        snd_render(&S, out);
        for (int n = 0; n < GB_BLOCK; n++) { float x = snd_last_l[n]; if (keep && b * GB_BLOCK + n < 96000) keep[b * GB_BLOCK + n] = x; if (fabsf(x) > 1e-4f) last = b; }
    }
    return last;                                               /* the last block that sounded */
}
/* 3.3.6: a break cut in the editor: a user slot of several zones in order, each encoded from its own start (as the
 * editor writes them; SLOOP 2.8 and the kit play them as pads). SLICE plays zone j as slice j, whatever DIV. */
static int16_t brk_pcm[40000];
static int ima_enc(const int16_t *x, int n, uint8_t *o)             /* (web/src/files.js's ima: from 0, 0) */
{
    int pred = 0, idx = 0;
    for (int i = 0; i < n; i++) {
        int step = smp_ima_step[idx], d = x[i] - pred, code = 0, vd = step >> 3;
        if (d < 0) { code = 8; d = -d; }
        if (d >= step) { code |= 4; d -= step; vd += step; }
        if (d >= step >> 1) { code |= 2; d -= step >> 1; vd += step >> 1; }
        if (d >= step >> 2) { code |= 1; vd += step >> 2; }
        pred += code & 8 ? -vd : vd; pred = pred < -32768 ? -32768 : pred > 32767 ? 32767 : pred;
        idx += smp_ima_idx[code & 7]; idx = idx < 0 ? 0 : idx > 88 ? 88 : idx;
        if (i & 1) o[i >> 1] |= (uint8_t)(code << 4); else o[i >> 1] = (uint8_t)code;
    }
    return (n + 1) / 2;
}
static double corr(const float *a, const int16_t *b, int n)
{
    double sa = 0, sb = 0, sab = 0, saa = 0, sbb = 0;
    for (int i = 0; i < n; i++) { sa += a[i]; sb += b[i]; sab += (double)a[i] * b[i]; saa += (double)a[i] * a[i]; sbb += (double)b[i] * b[i]; }
    double c = sab - sa * sb / n, va = saa - sa * sa / n, vb = sbb - sb * sb / n;
    return va > 0 && vb > 0 ? c / sqrt(va * vb) : 0;
}
static int slice_play(int e[8], int note, int hold_blocks, int blocks, float *keep);
static void break_slots_test(void)
{
    /* the BREAK decoded (its material), cut at 8 points, each slice encoded from 0, 0 into USR1 at 44.1 kHz */
    int bz = smp_break_zone(); const smp_zone *b0 = smp_zone_at(bz); const uint8_t *d = smp_data() + b0->off;
    int n = b0->n < 32000u ? (int)b0->n : 32000, pred = 0, idx = 0;
    for (int i = 0; i < n; i++) {
        uint32_t c = (i & 1) ? d[i >> 1] >> 4 : d[i >> 1] & 15u; int step = smp_ima_step[idx], vd = step >> 3;
        if (c & 4u) vd += step;
        if (c & 2u) vd += step >> 1;
        if (c & 1u) vd += step >> 2;
        pred += (c & 8u) ? -vd : vd; pred = pred < -32768 ? -32768 : pred > 32767 ? 32767 : pred;
        idx += smp_ima_idx[c & 7u]; idx = idx < 0 ? 0 : idx > 88 ? 88 : idx; brk_pcm[i] = (int16_t)pred;
    }
    static const int cut[] = { 0, 2900, 6100, 9000, 13200, 16000, 19800, 24000, 28000 };
    static const uint8_t GMK[16] = { 36, 35, 38, 39, 42, 46, 44, 37, 40, 43, 48, 49, 51, 70, 63, 56 };
    const int nz = 8;
    memset(uslot, 0xFF, sizeof uslot);
    memcpy(uslot, "FSMP", 4); uslot[4] = 1; uslot[5] = 0; uslot[6] = (uint8_t)nz; memcpy(uslot + 8, "AMEN\0\0\0\0", 8);
    uint32_t off = 0;
    for (int j = 0; j < nz; j++) {
        int m = cut[j + 1] - cut[j];
        smp_zone z; memset(&z, 0, sizeof z);
        z.off = off; z.n = (uint32_t)m; z.ls = z.le = (uint32_t)m - 1u; z.rate = 65536u; z.root16 = (int16_t)(GMK[j] * 16); z.lo = z.hi = GMK[j];   /* (the pads' GM notes, as the editor writes them) */
        off += (uint32_t)ima_enc(brk_pcm + cut[j], m, uslot + 512 + off); if (off & 1u) uslot[512 + off++] = 0;
        memcpy(uslot + 32 + 28 * j, &z, 28);
    }
    uslot[16] = (uint8_t)off; uslot[17] = (uint8_t)(off >> 8); uslot[18] = (uint8_t)(off >> 16); uslot[19] = 0;
    smp_user_scan(0, uslot);
    const slc_tab *t = &slc_tabs[1];
    printf("  a break from the editor: %d slices (brk %d), %u samples\n", t->non, t->brk, t->len);
    CHECK(t->brk == nz && t->non == nz && t->len >= (uint32_t)cut[nz], "a slot of zones: its slices (brk %d, non %d)", t->brk, t->non);
    /* each slice is its zone: heard as the break's part there, ONE to its end; whatever DIV */
    static float o[96000]; int bad = 0, wrong = 0, len_bad = 0;
    for (int dv = 0; dv <= 4; dv += 2) for (int j = 0; j < nz; j++) {
        int e[8] = { 1, dv, 0, 0, 0, 0, 127, 127 }, m = cut[j + 1] - cut[j];
        int last = slice_play(e, 60 + j, 2, 120, o);
        double c = corr(o + 128, brk_pcm + cut[j] + 127, m - 400), c2 = j + 1 < nz ? corr(o + 128, brk_pcm + cut[j + 1] + 127, m - 400) : 0;
        if (c < 0.98) bad++;
        if (c2 > c) wrong++;
        if (abs((last + 1) * GB_BLOCK - m) > 2 * GB_BLOCK) len_bad++;
        if (dv == 0 && j == 3) printf("  slice 3: correlation %.4f with its part, %.4f with the next; ends at block %d (%d samples)\n", c, c2, last, m);
    }
    CHECK(!bad && !wrong, "a break's slices not their parts (%d unlike, %d the wrong one)", bad, wrong);
    CHECK(!len_bad, "a break's slices: %d not their length", len_bad);
    /* START shifts them; backwards is the slice reversed */
    int es[8] = { 1, 4, 2, 0, 0, 0, 127, 127 }, m3 = cut[3] - cut[2]; slice_play(es, 60, 2, 60, o);
    CHECK(corr(o + 128, brk_pcm + cut[2] + 127, m3 - 400) > 0.98, "START 2: the first key not slice 2");
    int er[8] = { 1, 4, 0, 0, 0, 1, 127, 127 }, m5 = cut[6] - cut[5]; slice_play(er, 65, 2, 60, o);
    static int16_t rv[20000]; for (int i = 0; i < m5; i++) rv[i] = brk_pcm[cut[6] - 1 - i];
    double cr = corr(o + 128, rv + 127, m5 - 400);
    printf("  slice 5 backwards: correlation %.4f with its part reversed\n", cr);
    CHECK(cr > 0.95, "REV: a break's slice not reversed (%.3f)", cr);
    /* the same slot as the drum track's kit (USR1): pad k plays slice k */
    int dbad = 0; double dmin = 1;
    for (int j = 0; j < nz; j++) {
        reset(); sm_jump(&S.master, 1.0f); sm_jump(&S.trk[3].level, 1.0f);
        gb_patch *p = &S.trk[3].patch; gb_patch_default(p, ENG_DRUM); gb_patch_set(p, SP_E0, GB_KIT_USR);
        snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 3, j, 1.0f);
        for (int b = 0; b < 120; b++) { if (b == 2) snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 3, j); snd_render(&S, out); for (int q = 0; q < GB_BLOCK; q++) o[b * GB_BLOCK + q] = snd_last_l[q]; }
        int m = cut[j + 1] - cut[j]; double c = -1; int lag = 0;
        for (int l = 0; l < 1500; l++) { double cc = corr(o + l, brk_pcm + cut[j], m - 1600); if (cc > c) { c = cc; lag = l; } }
        if (j == 0) { double e = 0; for (int q = 0; q < 120 * GB_BLOCK; q++) e += fabs(o[q]); printf("  (pad 0: energy %.2f, best lag %d, correlation %.4f)\n", e, lag, c); }
        if (c < dmin) dmin = c;
        if (c < 0.9) dbad++;
    }
    printf("  the drums, USR1 as the kit: pad k is slice k (lowest correlation %.4f)\n", dmin);
    CHECK(!dbad, "the drums' USR kit: %d pads not their slice", dbad);
    /* a single zone: as before (cut by DIV, its hits) */
    uslot[6] = 1; smp_user_scan(0, uslot);
    CHECK(slc_tabs[1].brk == 0 && slc_tabs[1].len == (uint32_t)(cut[1] - cut[0]), "one zone taken as slices (brk %d)", slc_tabs[1].brk);
    smp_user_scan(0, 0);
    CHECK(slc_tabs[1].len == 0 && slc_tabs[1].brk == 0, "USR1 dropped (a break)");
}
static void slice_test(void)
{
    printf("slice\n");
    const slc_tab *br = &slc_tabs[0];
    printf("  BREAK: %u samples, %d hits\n", br->len, br->non);
    CHECK(br->len > 20000 && br->non >= 6 && br->non <= 16, "BREAK: len %u, hits %d (its kicks and snares at least)", br->len, br->non);
    /* DIV 16 at 16 kHz, 120 BPM: a slice is 125 ms = 86 blocks; ONE plays it whole even when released at once */
    int e[8] = { 0, 2, 0, 0, 0, 0, 127, 127 };
    int one = slice_play(e, 60, 2, 200, 0);
    e[4] = 1; int gate = slice_play(e, 60, 10, 200, 0);
    e[4] = 2; int loop = slice_play(e, 60, 180, 220, 0);
    printf("  ONE ends at block %d, GATE (released at 10) %d, LOOP (held 180) %d\n", one, gate, loop);
    CHECK(one >= 80 && one <= 92, "ONE: the slice's length (block %d)", one);
    CHECK(gate >= 9 && gate < 20, "GATE: ends after its release (block %d)", gate);
    CHECK(loop >= 178, "LOOP: repeats while held (block %d)", loop);
    /* every slice of every division sounds */
    int silent = 0;
    for (int dv = 0; dv <= 4; dv++) for (int j = 0; j < 8; j++) { int f[8] = { 0, dv, 0, 0, 0, 0, 127, 127 }; double a = 0;
        slice_play(f, 60 + j, 4, 30, slc_buf); for (int i = 256; i < 30 * GB_BLOCK; i++) a += (double)slc_buf[i] * slc_buf[i]; if (a < 1e-6) silent++; }
    CHECK(!silent, "%d slices silent", silent);
    /* backwards = forwards reversed: slice 5 of 16 (TONE open, no DECAY), the whole slice at the material's rate x 2.756 */
    static float fw[96000], bw[96000];
    int f1[8] = { 0, 2, 0, 0, 0, 0, 127, 127 }, f2[8] = { 0, 2, 0, 0, 0, 1, 127, 127 };
    int lf = slice_play(f1, 65, 2, 120, fw), lb = slice_play(f2, 65, 2, 120, bw);
    float pf = 0, pb = 0; for (int i = 0; i < 120 * GB_BLOCK; i++) { pf = fmaxf(pf, fabsf(fw[i])); pb = fmaxf(pb, fabsf(bw[i])); }
    /* the hit is at the forward slice's start: backwards, its loudest part comes at the end */
    int af = 0, ab = 0; for (int i = 0; i < 120 * GB_BLOCK; i++) { if (fabsf(fw[i]) == pf) af = i; if (fabsf(bw[i]) == pb) ab = i; }
    printf("  forwards: peak %.3f at %d, ends %d; backwards: peak %.3f at %d, ends %d\n", pf, af, lf, pb, ab, lb);
    CHECK(pf > 0.03f && pb > 0.03f && fabsf(pf - pb) < 0.3f * pf && af < ab && abs(lf - lb) <= 3, "REV: not the slice reversed");   /* (3.3.1: the BREAK played with the right kit, slice 5 a hat: 0.05 -> 0.03) */
    /* AUTO: a slice per hit, each starting loud */
    int quiet = 0;
    for (int j = 0; j < br->non; j++) { int f[8] = { 0, 4, 0, 0, 0, 0, 127, 127 }; slice_play(f, 60 + j, 2, 20, slc_buf);
        float pk = 0; for (int i = 0; i < 12 * GB_BLOCK; i++) pk = fmaxf(pk, fabsf(slc_buf[i])); if (pk < 0.03f) quiet++; }
    CHECK(!quiet, "AUTO: %d slices do not start on a hit", quiet);
    /* a user slot: the bank's piano zone, sliced */
    memset(uslot, 0xFF, sizeof uslot);
    smp_zone z = *smp_zone_at(smp_set_at(0)->z0); uint32_t nb = (z.n + 1u) / 2u; if (nb > 60000u) { nb = 60000u; z.n = 120000u; z.ls = z.le = z.n - 1u; }
    memcpy(uslot, "FSMP", 4); uslot[4] = 1; uslot[5] = 0; uslot[6] = 1; memcpy(uslot + 8, "PNO\0\0\0\0\0", 8);
    memcpy(uslot + 512, smp_data() + z.off, nb); z.off = 0; z.lo = 0; z.hi = 127; memcpy(uslot + 32, &z, 28);
    uslot[16] = (uint8_t)nb; uslot[17] = (uint8_t)(nb >> 8); uslot[18] = (uint8_t)(nb >> 16); uslot[19] = 0;
    smp_user_scan(0, uslot);
    CHECK(slc_tabs[1].len == z.n, "USR1 not scanned (%u)", slc_tabs[1].len);
    int us[8] = { 1, 3, 0, 0, 0, 0, 127, 127 }, ur[8] = { 1, 3, 0, 0, 0, 1, 127, 127 }; double ua = 0, ub = 0;
    slice_play(us, 60 + 20, 2, 60, slc_buf); for (int i = 0; i < 60 * GB_BLOCK; i++) ua += fabs(slc_buf[i]);
    slice_play(ur, 60 + 20, 2, 60, slc_buf); for (int i = 0; i < 60 * GB_BLOCK; i++) ub += fabs(slc_buf[i]);
    printf("  USR1: %u samples, %d hits; slice 20 of 32: %.2f, backwards %.2f\n", slc_tabs[1].len, slc_tabs[1].non, ua, ub);
    CHECK(ua > 1 && ub > 1, "USR1's slices silent");
    smp_user_scan(0, 0);
    CHECK(slc_tabs[1].len == 0, "USR1 dropped");
    break_slots_test();
}

/* TEXTURE: each bed about -18 dBFS rms at LEVEL 127 (-30 at 64), none at OFF; WHEN PLAY: silent while stopped */
static void texture_test(void)
{
    printf("texture\n");
    float lo = 0.0f, hi = -99.0f; int bad = 0;
    for (int src = 0; src <= 6; src++) for (int lv = 64; lv <= 127; lv += 63) {
        reset(); sm_jump(&S.master, 1.0f); S.tx[0] = (uint8_t)src; S.tx[1] = (uint8_t)lv; S.tx[5] = 1;
        double a = 0; for (int b = 0; b < 700; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) { float x = snd_last_l[n]; if (x != x) bad = 1; if (b >= 100) a += (double)x * x; } }
        float db = 10.0f * (float)log10(a / (600.0 * GB_BLOCK) + 1e-30);
        if (!src) { if (a > 1e-12) bad = 1; continue; }
        printf("  src %d level %d: %.1f dB\n", src, lv, db);
        if (lv == 127) { if (db < lo || lo == 0.0f) lo = db; if (db > hi) hi = db; }
        if (lv == 127 && (db < -26.0f || db > -12.0f)) bad = 1;
        if (lv == 64 && (db < -38.0f || db > -24.0f)) bad = 1;
    }
    reset(); S.tx[0] = 3; S.tx[1] = 127; S.tx[5] = 0;                  /* WHEN PLAY, stopped */
    double a = 0; for (int b = 0; b < 200; b++) { snd_render(&S, out); for (int n = 0; n < GB_BLOCK; n++) a += fabs(snd_last_l[n]); }
    CHECK(!bad && a < 1e-9, "TEXTURE: levels %.1f .. %.1f dB, stopped %.3g", lo, hi, a);
}

int main(void)
{
    smp_bank_load("assets/samples.bin");
    owner_test(); retrig_test(); zipper_test(); steal_test(); guard_test(); pitch_test(); unison_test(); output_test(); split_test(); lfo_test(); mono_test(); master_test(); punch_test(); grain_test(); kits_test(); tfx_test(); fm6_bank_test(); usr_test(); slice_test(); texture_test(); engines_test();
    printf("%d checks, %d failed\n", checks, fails);
    return fails != 0;
}
