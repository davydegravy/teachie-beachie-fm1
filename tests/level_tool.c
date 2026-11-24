/* SPDX-License-Identifier: GPL-3.0-only
 * The factory sounds' loudness. level_tool: writes tools/levels.txt (each synth sound's gain, so that all are as
 * loud as the middle one); level_tool check: each within 1.5 dB (a test); level_tool report: each as heard, and on
 * the FM-1's speaker. Loudness (3.4.1): as heard (heard3: the sound's effects and sends, K-weighted, the loudest
 * 400 ms, three keys). */
#include "../app/presets.h"
#include <stdio.h>
#include "../engine/sample.h"
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <stdlib.h>
static int16_t fx[GB_FX_MEM];
static gb_sound S;
/* (3.4.0, Sam: "the kits barely heard, the kicks above all") K-weighted (ITU BS.1770: a shelf +4 dB over 1.7 kHz,
 * a high-pass at 38 Hz: the ear's weight) loudest 100 ms of the first second; on track 1, sound i (a kit: note n) */
typedef struct { double b0, b1, b2, a1, a2, x1, x2, y1, y2; } bq;
static double bq_run(bq *f, double x) { double y = f->b0 * x + f->b1 * f->x1 + f->b2 * f->x2 - f->a1 * f->y1 - f->a2 * f->y2; f->x2 = f->x1; f->x1 = x; f->y2 = f->y1; f->y1 = y; return y; }
static void kw_init(bq *k)
{
    double fs = GB_SR, A = pow(10.0, 3.99984 / 40.0), w = 2 * M_PI * 1681.974 / fs, al = sin(w) / (2 * 0.7071752), c = cos(w), sa = 2 * sqrt(A) * al;
    double a0 = (A + 1) - (A - 1) * c + sa;
    k[0] = (bq){ A * ((A + 1) + (A - 1) * c + sa) / a0, -2 * A * ((A - 1) + (A + 1) * c) / a0, A * ((A + 1) + (A - 1) * c - sa) / a0, 2 * ((A - 1) - (A + 1) * c) / a0, ((A + 1) - (A - 1) * c - sa) / a0, 0, 0, 0, 0 };
    w = 2 * M_PI * 38.13547 / fs; al = sin(w) / (2 * 0.5003270); c = cos(w); a0 = 1 + al;
    k[1] = (bq){ (1 + c) / 2 / a0, -(1 + c) / a0, (1 + c) / 2 / a0, -2 * c / a0, (1 - al) / a0, 0, 0, 0, 0 };
}
static double kloud(int i, int note, int with_gain)
{
    snd_init(&S, fx, 0); sm_jump(&S.master, 0.3f);
    S.trk[1].patch.engine = gb_presets[i].engine;
    preset_apply(&S, 1, i);
    if (!with_gain) S.trk[1].patch.gain = 1.0f;
    sm_jump(&S.trk[1].level, 1.0f); sm_jump(&S.trk[1].send_dly, 0.0f); sm_jump(&S.trk[1].send_rev, 0.0f); sm_jump(&S.trk[1].send_cho, 0.0f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, note, 0.8f);
    bq kl[2], kr[2]; kw_init(kl); kw_init(kr);
    int32_t o[GB_BLOCK * 2]; enum { NB = 689, WIN = 69 }; static double eb[NB]; double best = 0, run = 0;
    for (int b = 0; b < NB; b++) {
        if (b == 69 * 2 && gb_presets[i].engine != ENG_DRUM) snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 1, note);
        snd_render(&S, o); double e = 0;
        for (int n = 0; n < GB_BLOCK; n++) { double l = bq_run(&kl[1], bq_run(&kl[0], snd_last_l[n])), r = bq_run(&kr[1], bq_run(&kr[0], snd_last_r[n])); e += l * l + r * r; }
        eb[b] = e; run += e; if (b >= WIN) run -= eb[b - WIN];
        if (run > best) best = run;
    }
    return sqrt(best / (2.0 * WIN * GB_BLOCK)) / 0.3;
}
/* (3.4.1, Sam: "the instruments are not all as loud") a sound as it is heard: its gain, its effects and sends, the
 * K-weighting (ITU BS.1770) or the FM-1's speaker (a high-pass at 300 Hz, 2nd order: what a 28 mm speaker gives),
 * the loudest 400 ms of the first 2 s (EBU's momentary loudness), held 1 s; on three keys (C3 C4 C5 + its TRANS),
 * the power mean */
static double g_try = -1.0;                             /* (a gain tried instead of the sound's own) */
static double heard(int i, int note, int spk)
{
    gb_blocks = 0;                                      /* (the grains' chance from the same start: each measure alike) */
    snd_init(&S, fx, 0); sm_jump(&S.master, 0.3f);
    S.trk[1].patch.engine = gb_presets[i].engine;
    preset_apply(&S, 1, i);
    if (g_try > 0.0) S.trk[1].patch.gain = (float)g_try;
    sm_jump(&S.trk[1].level, 1.0f);
    snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, note, 0.8f);
    bq kl[2], kr[2]; kw_init(kl); kw_init(kr);
    if (spk) {                                          /* (the speaker: the shelf kept, the high-pass at 300 Hz) */
        double fs = GB_SR, w = 2 * M_PI * 300.0 / fs, al = sin(w) / (2 * 0.7071), c = cos(w), a0 = 1 + al;
        kl[1] = kr[1] = (bq){ (1 + c) / 2 / a0, -(1 + c) / a0, (1 + c) / 2 / a0, -2 * c / a0, (1 - al) / a0, 0, 0, 0, 0 };
    }
    int32_t o[GB_BLOCK * 2]; enum { NB = 1378, WIN = 276 }; static double eb[NB]; double best = 0, run = 0;
    for (int b = 0; b < NB; b++) {
        if (b == 689) snd_note_off(&S, GB_OWNER(OWN_KEY, 0), 1, note);
        snd_render(&S, o); double e = 0;
        for (int n = 0; n < GB_BLOCK; n++) { double l = bq_run(&kl[1], bq_run(&kl[0], snd_last_l[n])), r = bq_run(&kr[1], bq_run(&kr[0], snd_last_r[n])); e += l * l + r * r; }
        eb[b] = e; run += e; if (b >= WIN) run -= eb[b - WIN];
        if (run > best) best = run;
    }
    return best / (2.0 * WIN * GB_BLOCK) / 0.09;        /* (a power) */
}
static double heard3(int i, int spk)
{
    /* (GRAIN: its grains fall at random, from a pool that lives on: four times, the mean) */
    int t = preset_trans(i), reps = gb_presets[i].engine == ENG_GRAIN ? 4 : 1; double p = 0;
    for (int r = 0; r < reps; r++)
        for (int k = 0; k < 3; k++) { int nt = 48 + 12 * k + t; p += heard(i, nt < 24 ? nt + 12 : nt > 100 ? nt - 12 : nt, spk); }
    return 10.0 * log10(p / (3.0 * reps) + 1e-20);
}
static int cmp(const void *a, const void *b) { double x = *(const double *)a, y = *(const double *)b; return x < y ? -1 : x > y; }
int main(int argc, char **argv)
{
    smp_bank_load("assets/samples.bin");
    int check = argc > 1 && !strcmp(argv[1], "check");
    if (argc > 1 && !strcmp(argv[1], "drumtrims")) {            /* (3.4.1) SLOOP's synthesised kits, each key against the
                                                                 * same key of the other kits: tools/drumtrims.txt */
        static double L[64][16]; int ki[64], nk = 0;
        for (int i = 0; i < gb_npresets; i++) {
            if (gb_presets[i].engine != ENG_DRUM || gb_presets[i].e[0] < GB_KIT_SAMPLED || gb_presets[i].e[0] >= GB_KIT_USR) continue;
            ki[nk] = i;
            for (int r = 0; r < 16; r++) { double l = kloud(i, r, 1); L[nk][r] = l > 1e-9 ? 20 * log10(l) : -999.0; }
            nk++;
        }
        double band = argc > 2 ? atof(argv[2]) : 1.5;
        /* (the kits as built carry the trims already: what is found now is added to them) */
        static char tn[1024][16]; static int tr[1024]; static double tv[1024]; int tk = 0;
        { FILE *g = fopen("tools/drumtrims.txt", "r"); char ln[128];
          while (g && fgets(ln, sizeof ln, g) && tk < 1024) if (ln[0] != '#' && sscanf(ln, "%15[^\t]\t%d\t%lf", tn[tk], &tr[tk], &tv[tk]) == 3) tk++;
          if (g) fclose(g); }
        FILE *f = fopen("tools/drumtrims.txt", "w");
        fprintf(f, "# (3.4.1) each synthesised kit's keys brought within %.1f dB of the same key in the other kits (the median), K-weighted:\n# kit, key (0-15), dB. Written by build/level_tool drumtrims; read by tools/mkdrumkits.py\n", band);
        int nt = 0;
        for (int r = 0; r < 16; r++) {
            static double v[64]; int n = 0; for (int k = 0; k < nk; k++) if (L[k][r] > -900) v[n++] = L[k][r];
            if (!n) continue;
            qsort(v, n, sizeof(double), cmp); double med = v[n / 2];
            for (int k = 0; k < nk; k++) {
                if (L[k][r] < -900) continue;
                double d = L[k][r] - med, t = d > band ? band - d : d < -band ? -band - d : 0.0;
                for (int j = 0; j < tk; j++) if (tr[j] == r && !strcmp(tn[j], gb_presets[ki[k]].name)) t += tv[j];
                if (fabs(t) >= 0.25) { fprintf(f, "%s\t%d\t%+.2f\n", gb_presets[ki[k]].name, r, t); nt++; }
            }
        }
        fclose(f); printf("tools/drumtrims.txt: %d keys trimmed (%d kits)\n", nt, nk);
        return 0;
    }
    if (argc > 1 && !strcmp(argv[1], "peaks")) {                /* (3.4.1) each sound's peak, full velocity, C2..C6, the track at 1 */
        for (int i = 0; i < gb_npresets; i++) {
            if (gb_presets[i].engine == ENG_DRUM) continue;
            double pk = 0;
            for (int nt = 36; nt <= 84; nt += 12) {
                gb_blocks = 0; snd_init(&S, fx, 0); sm_jump(&S.master, 1.0f);
                S.trk[1].patch.engine = gb_presets[i].engine; preset_apply(&S, 1, i); sm_jump(&S.trk[1].level, 1.0f);
                snd_note_on(&S, GB_OWNER(OWN_KEY, 0), 1, nt + preset_trans(i), 1.0f);
                int32_t o[GB_BLOCK * 2];
                for (int b = 0; b < 689; b++) { snd_render(&S, o); for (int n = 0; n < GB_BLOCK; n++) { double a = fabs(snd_last_l[n]) > fabs(snd_last_r[n]) ? fabs(snd_last_l[n]) : fabs(snd_last_r[n]); if (a > pk) pk = a; } }
            }
            printf("%-10s\t%.2f\n", gb_presets[i].name, pk);
        }
        return 0;
    }
    if (argc > 1 && !strcmp(argv[1], "report")) {               /* (3.4.1) each sound as heard, in dB from the median */
        static double hp[512], sp[512], srt[512]; int n = 0, idx[512];
        for (int i = 0; i < gb_npresets; i++) if (gb_presets[i].engine != ENG_DRUM) { idx[n] = i; hp[n] = heard3(i, 0); sp[n] = heard3(i, 1); n++; }
        memcpy(srt, hp, sizeof(double) * n); qsort(srt, n, sizeof(double), cmp); double med = srt[n / 2];
        memcpy(srt, sp, sizeof(double) * n); qsort(srt, n, sizeof(double), cmp); double meds = srt[n / 2];
        for (int j = 0; j < n; j++) printf("%-10s\t%s\t%+6.1f\t%+6.1f\n", gb_presets[idx[j]].name, gb_kind_name[gb_presets[idx[j]].kind], hp[j] - med, sp[j] - meds);
        memcpy(srt, hp, sizeof(double) * n); qsort(srt, n, sizeof(double), cmp);
        printf("# %d sounds, as heard (K-weighted): 5 %% %+.1f, 95 %% %+.1f dB, the extremes %+.1f .. %+.1f\n", n, srt[n / 20] - med, srt[n * 19 / 20] - med, srt[0] - med, srt[n - 1] - med);
        return 0;
    }
    if (argc > 1 && !strcmp(argv[1], "drums")) {                /* the kits against the synths, K-weighted, in dB */
        static double v[512]; int n = 0;
        for (int i = 0; i < gb_npresets; i++) if (gb_presets[i].engine != ENG_DRUM) v[n++] = kloud(i, 60 + preset_trans(i), 1);
        qsort(v, n, sizeof(double), cmp); double ref = v[n / 2];
        printf("the synth sounds (with their gains), K-weighted: median %.4f (0 dB); 10 %% %+.1f, 90 %% %+.1f dB\n", ref, 20 * log10(v[n / 10] / ref), 20 * log10(v[n * 9 / 10] / ref));
        static const int NT[6] = { 0, 2, 3, 6, 8, 12 }; static const char *const NN[6] = { "kick", "snare", "clap", "hat", "ohat", "tom" };
        for (int i = 0; i < gb_npresets; i++) if (gb_presets[i].engine == ENG_DRUM) {
            printf("  %-10s", gb_presets[i].name);
            for (int k = 0; k < 6; k++) { double l = kloud(i, NT[k], 1); printf(" %s %+5.1f", NN[k], l > 1e-9 ? 20 * log10(l / ref) : -99.0); }
            printf("\n");
        }
        return 0;
    }
    /* (3.4.1) the gains from the sounds as heard (heard3: their effects, three keys, K-weighted), so that all are as
     * loud as the middle one; a few passes, as a sound's drive does not follow its gain. USR SLICE plays your
     * sample: not measured here */
    static double gn[512], db[512], srt[512], be[512], bg[512]; int n = 0, idx[512];
    for (int i = 0; i < gb_npresets; i++) if (gb_presets[i].engine != ENG_DRUM && strcmp(gb_presets[i].name, "USR SLICE")) { idx[n] = i; gn[n] = preset_gain(i); n++; }
    double target = 0;
    for (int pass = 0; pass < (check ? 1 : 8); pass++) {
        for (int j = 0; j < n; j++) { g_try = gn[j]; db[j] = heard3(idx[j], 0); }
        g_try = -1.0;
        if (pass == 0) { memcpy(srt, db, sizeof(double) * n); qsort(srt, n, sizeof(double), cmp); target = srt[n / 2]; }
        if (check) break;
        double worst = 0;
        for (int j = 0; j < n; j++) {
            double e = db[j] - target; if (fabs(e) > worst) worst = fabs(e);
            if (pass == 0 || fabs(e) < be[j]) { be[j] = fabs(e); bg[j] = gn[j]; }   /* (the best gain so far: kept) */
            gn[j] *= pow(10.0, -e * (pass < 2 ? 1.0 : 0.6) / 20.0); gn[j] = gn[j] < 0.05 ? 0.05 : gn[j] > 20.0 ? 20.0 : gn[j];
        }
        printf("pass %d: the farthest %.1f dB from the middle\n", pass + 1, worst);
        if (worst < 0.3) break;
    }
    if (!check) for (int j = 0; j < n; j++) gn[j] = bg[j];
    if (!check) {
        FILE *f = fopen("tools/levels.txt", "w");
        for (int i = 0; i < gb_npresets; i++) {
            if (gb_presets[i].engine == ENG_DRUM) continue;
            double g = preset_gain(i);
            for (int j = 0; j < n; j++) if (idx[j] == i) g = gn[j];
            fprintf(f, "%s\t%.3f\n", gb_presets[i].name, g);
        }
        fclose(f); printf("tools/levels.txt written (%d sounds measured)\n", n);
        return 0;
    }
    int bad = 0; double lo = 99, hi = -99;
    for (int j = 0; j < n; j++) {
        double d = db[j] - target;
        if (d < lo) lo = d;
        if (d > hi) hi = d;
        if (fabs(d) > 1.5) { bad++; printf("  %s: %+.1f dB\n", gb_presets[idx[j]].name, d); }
    }
    printf("levels: %d synth sounds, as heard, within %+.1f .. %+.1f dB, %d off by more than 1.5 dB\n", n, lo, hi, bad);
    return bad != 0;
}
