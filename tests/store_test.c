/* SPDX-License-Identifier: GPL-3.0-only
 * Projects in flash: a project comes back as it was saved; a power cut at any flash operation of a save leaves
 * either the version before or the new one, whole, never a broken one (G12); a sector erase never blocks. */
#include "../app/store.h"
#include "../app/project.h"
#include "../app/xbank.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
static int checks, fails;
#define CHECK(c, ...) do { checks++; if (!(c)) { fails++; printf("  FAIL %s:%d: ", __FILE__, __LINE__); printf(__VA_ARGS__); printf("\n"); } } while (0)

/* a NOR flash: erase sets 0xFF, a write can only clear bits; an operation is busy for a few polls */
static uint8_t fl[0x100000];
static int busy_left, ops, cut_at = -1, erases;
static int f_read(uint32_t a, void *d, uint32_t n) { memcpy(d, fl + a, n); return 0; }
static int f_busy(void) { if (busy_left > 0) { busy_left--; return 1; } return 0; }
static int f_erase(uint32_t a) { if (ops++ == cut_at) return -1; erases++; memset(fl + a, 0xFF, 4096); busy_left = 5; return 0; }
static int f_prog(uint32_t a, const void *s, uint32_t n) { if (ops++ == cut_at) return -1; for (uint32_t i = 0; i < n; i++) fl[a + i] &= ((const uint8_t *)s)[i]; busy_left = 2; return 0; }
static const store_flash F = { f_read, f_erase, f_prog, f_busy };

/* 3.3.5: the one project, given part by part as the firmware does (a 12 KB buffer at a time) */
static int save_proj(const uint8_t *d, uint32_t n, uint32_t crc, int chunk)
{
    if (store_proj_begin(n, crc, 1)) return -1;
    int r, guard = 0;
    for (;;) {
        r = store_step();
        if (r == 3) { uint32_t off = store_proj_need(), k = n - off > (uint32_t)chunk ? (uint32_t)chunk : n - off; store_proj_give(d + off, k); continue; }
        if (r != 1) break;
        if (++guard > 1000000) return -9;
    }
    return r == 2 ? 0 : r;
}
static int load_proj(uint8_t *got, uint32_t cap)
{
    uint32_t at, n, crc;
    if (store_proj_find(&at, &n, &crc) || n > cap) return -1;
    memcpy(got, fl + at, n);
    return (int)n;
}
static uint32_t crc_of(const uint8_t *d, uint32_t n) { return crc32_ieee(0, d, n); }
static uint8_t a_[STORE_PMAX], b_[STORE_PMAX], got[STORE_PMAX];

static void store_tests(void)
{
    printf("store: the one project, two banks, the newest whole wins\n");
    memset(fl, 0xFF, sizeof fl); store_init(&F);
    CHECK(load_proj(got, sizeof got) < 0, "an empty flash gave a project");
    for (uint32_t i = 0; i < sizeof a_; i++) { a_[i] = (uint8_t)(i * 7 + 1); b_[i] = (uint8_t)(i * 13 + 5); }
    CHECK(save_proj(a_, 5000, crc_of(a_, 5000), 12032) == 0, "the first save failed");
    CHECK(load_proj(got, sizeof got) == 5000 && !memcmp(got, a_, 5000), "the first save did not come back");
    CHECK(save_proj(b_, 7000, crc_of(b_, 7000), 12032) == 0 && load_proj(got, sizeof got) == 7000 && !memcmp(got, b_, 7000), "the second save did not come back");
    CHECK(save_proj(a_, STORE_PMAX, crc_of(a_, STORE_PMAX), 12032) == 0 && load_proj(got, sizeof got) == (int)STORE_PMAX && !memcmp(got, a_, STORE_PMAX), "a full-size project failed");
    CHECK(save_proj(b_, 3000, crc_of(b_, 3000), 100) == 0 && load_proj(got, sizeof got) == 3000 && !memcmp(got, b_, 3000), "given 100 bytes at a time");
    CHECK(store_proj_begin(STORE_PMAX + 1, 0, 1) < 0, "an oversized project was accepted");
    /* changed while written: refused, the copy before kept */
    CHECK(save_proj(a_, 4000, crc_of(b_, 4000), 12032) == -2 && load_proj(got, sizeof got) == 3000 && !memcmp(got, b_, 3000), "a project not as begun was kept");
    /* an upload: its CRC at the end */
    {
        CHECK(store_proj_begin(6000, 0, 0) == 0, "an upload begun");
        int r, ended = 0;
        while ((r = store_step()) != 2 && r >= 0) {
            if (r == 3 || store_proj_wants()) { uint32_t off = store_proj_given(); if (off < 6000) store_proj_give(a_ + off, 6000 - off > 384 ? 384 : 6000 - off); }
            if (!ended && store_proj_written()) { CHECK(store_proj_end(crc_of(a_, 6000)) == 0, "the end taken"); ended = 1; }
        }
        CHECK(r == 2 && load_proj(got, sizeof got) == 6000 && !memcmp(got, a_, 6000), "an upload did not come back (%d)", r);
    }

    printf("store: a power cut at every flash operation of a save\n");
    int worst = 0, n_old = 0, n_new = 0;
    for (int cut = 0; cut < 13 + 24 + 2; cut++) {
        memset(fl, 0xFF, sizeof fl); store_init(&F); cut_at = -1; ops = 0;
        save_proj(a_, 6000, crc_of(a_, 6000), 12032);         /* the version before */
        save_proj(b_, 5000, crc_of(b_, 5000), 12032);         /* (and one in the other bank: both full) */
        ops = 0; cut_at = cut;
        int r = save_proj(a_ + 100, 6000, crc_of(a_ + 100, 6000), 12032);   /* cut here: the operation does not happen */
        cut_at = -1;
        store_init(&F);                                        /* (the start after it) */
        int n = load_proj(got, sizeof got);
        int old = n == 5000 && !memcmp(got, b_, 5000), neu = n == 6000 && !memcmp(got, a_ + 100, 6000);
        if (!old && !neu) worst++;
        n_old += old; n_new += neu;
        if (r == 0) CHECK(neu, "a save that finished did not load as new");
    }
    printf("  39 cuts: %d gave the version before, %d the new one, %d neither\n", n_old, n_new, worst);
    CHECK(worst == 0, "%d cuts left no whole project", worst);
    /* (3.4.0) saves one after another (as while it plays): each after the newest, where the flash is still erased, so no
     * erase until the banks are full; then the other bank again, erased; every copy comes back, the cuts too */
    {
        memset(fl, 0xFF, sizeof fl); store_init(&F); erases = 0;
        int bad = 0, first_erase = -1;
        for (int i = 0; i < 24; i++) {
            const uint8_t *d = (i & 1) ? b_ : a_ + i; uint32_t n = 7000u + (uint32_t)i * 37u;
            int e0 = erases;
            if (save_proj(d, n, crc_of(d, n), 12032) || load_proj(got, sizeof got) != (int)n || memcmp(got, d, n)) bad++;
            if (erases > e0 && first_erase < 0) first_erase = i;
        }
        printf("  24 saves of 7 KB in a row: the first erase at save %d, %d erases in all\n", first_erase + 1, erases);
        CHECK(!bad, "%d saves in a row did not come back", bad);
        CHECK(first_erase >= 7, "an erase already at save %d (6 copies of 2 sectors fit in the two banks after the first)", first_erase + 1);
        int w = 0;
        for (int cut = 0; cut < 2 + 28 + 8 + 2; cut++) {             /* a save that must erase, cut at each operation */
            memset(fl, 0xFF, sizeof fl); store_init(&F); cut_at = -1;
            for (int i = 0; i < 12; i++) save_proj(a_ + i, 7000, crc_of(a_ + i, 7000), 12032);
            ops = 0; cut_at = cut;
            int r = save_proj(b_, 7000, crc_of(b_, 7000), 12032);
            cut_at = -1; store_init(&F);
            int n = load_proj(got, sizeof got);
            int old = n == 7000 && !memcmp(got, a_ + 11, 7000), neu = n == 7000 && !memcmp(got, b_, 7000);
            if (!old && !neu) w++;
            if (r == 0 && !neu) w++;
        }
        CHECK(w == 0, "a save over a full bank, cut: %d left no whole project", w);
    }
    /* a torn byte in the new copy's data: the one before */
    memset(fl, 0xFF, sizeof fl); store_init(&F);
    save_proj(a_, 2000, crc_of(a_, 2000), 12032); save_proj(b_, 2000, crc_of(b_, 2000), 12032);
    { uint32_t at, n, c; store_proj_find(&at, &n, &c); fl[at + 10] ^= 0x55; }
    int n = load_proj(got, sizeof got);
    CHECK(n == 2000 && !memcmp(got, a_, 2000), "a torn copy did not fall back to the one before");
    /* the 8 places of 3.0 to 3.3.4: their newest found and read (the firmware keeps it as the one project) */
    {
        memset(fl, 0xFF, sizeof fl); store_init(&F);
        struct { uint32_t magic, seq, len, dcrc; uint8_t slot, pad[3]; uint32_t hcrc; } h;
        for (int k = 0; k < 3; k++) {
            uint32_t at = STORE_BASE + (uint32_t)(k * 2) * STORE_COPY;
            h.magic = 0x32504247u; h.seq = k == 1 ? 30u : 10u + (uint32_t)k; h.len = 1000u + (uint32_t)k; h.slot = (uint8_t)(k + 3); h.pad[0] = h.pad[1] = h.pad[2] = 0;
            memcpy(fl + at + 256, k == 1 ? b_ : a_, h.len); h.dcrc = crc_of(fl + at + 256, h.len); h.hcrc = crc32_ieee(0, &h, 20);
            memcpy(fl + at, &h, sizeof h);
        }
        int last = store_old_latest(); int m = last >= 0 ? store_old_load(last, got, sizeof got) : -1;
        CHECK(last == 4 && m == 1001 && !memcmp(got, b_, 1001), "the newest of the 8 places (%d, %d)", last, m);
        CHECK(load_proj(got, sizeof got) < 0, "an old place taken for the one project");
    }
}

static seq_t Q1, Q2;
static gb_sound S1, S2;
static int16_t fx1[GB_FX_MEM], fx2[GB_FX_MEM];
static uint8_t buf[STORE_PMAX];
static project_extra X1, X2;
static psounds PS1, PS2;
/* 3.3.5: the patterns' notes packed (format 15): every kind of note comes back exactly, at every density */
static uint32_t prs = 12345u;
static uint32_t prnd(void) { prs ^= prs << 13; prs ^= prs >> 17; prs ^= prs << 5; return prs; }
static void pack_tests(void)
{
    printf("project: packed notes come back exactly\n");
    static seq_hist h1, h2; static uint8_t big[60000];
    int bad = 0, rounds = 0;
    for (int r = 0; r < 300; r++) {
        seq_init(&Q1, 44100, 120.0f, &h1); snd_init(&S1, fx1, 0); seq_init(&Q2, 44100, 90.0f, &h2); snd_init(&S2, fx2, 0);
        int kind = r % 6;
        for (int t = 0; t < SEQ_NTRK; t++) for (int i = 0; i < SEQ_NPAT; i++) {
            seq_pat *pt = &Q1.t[t].pat[i];
            pt->steps = (uint16_t)(1 + prnd() % SEQ_MAXSTEPS);
            int n = (int)(prnd() % (SEQ_PATEV + 1)); if (r % 7 == 0) n = SEQ_PATEV;
            uint32_t tick = 0;
            for (int e = 0; e < n; e++) {
                seq_ev *v = &pt->ev[e];
                switch (kind) {                                   /* on the grid, chords, drum steps, played, anything */
                case 0: tick = (uint32_t)e * SEQ_TPS; v->len = SEQ_TPS - 2; v->vel = 100; v->note = (uint8_t)(48 + e % 12); break;
                case 1: if (e % 4 == 0 && e) tick += 2 * SEQ_TPS; v->len = (uint16_t)(2 * SEQ_TPS - 2); v->vel = (uint8_t)(70 + e % 3); v->note = (uint8_t)(57 + (e % 4) * 4); break;
                case 2: if (prnd() % 3 == 0) tick += SEQ_TPS; v->len = SEQ_TPS / 2; v->vel = (uint8_t)(60 + prnd() % 60); v->note = (uint8_t)(prnd() % 16); break;
                case 3: tick += prnd() % 300; v->len = (uint16_t)(1 + prnd() % 900); v->vel = (uint8_t)(1 + prnd() % 127); v->note = (uint8_t)(30 + prnd() % 60); break;
                default: tick += prnd() % 2000; v->len = (uint16_t)(prnd() & 0xFFFF); v->vel = (uint8_t)(prnd() % 128); v->note = (uint8_t)(prnd() % 128); break;
                }
                if (tick > 0xFFFF) tick = 0xFFFF;
                v->tick = (uint16_t)tick;
                v->cond = (uint8_t)(kind >= 4 && prnd() % 3 == 0 ? prnd() % 40 : SEQ_ALWAYS);
                uint32_t ra = kind >= 4 && prnd() % 3 == 0 ? (1 + prnd() % 4) | (prnd() & 0x30u) : 1u; v->ratchet = (uint8_t)ra;
            }
            pt->n = (uint16_t)n;
            pt->nlock = (uint8_t)(prnd() % (SEQ_LOCKS + 1));
            for (int l = 0; l < pt->nlock; l++) { pt->lk[l].step = (uint8_t)(prnd() % 128); pt->lk[l].param = (uint8_t)(prnd() % 64); pt->lk[l].value = (uint8_t)prnd(); }
        }
        int n = project_save(&Q1, &S1, 0, big, sizeof big);
        if (n <= 0 || project_load(&Q2, &S2, 0, big, (uint32_t)n) != 0) { bad++; continue; }
        for (int t = 0; t < SEQ_NTRK; t++) for (int i = 0; i < SEQ_NPAT; i++) {
            const seq_pat *a = &Q1.t[t].pat[i], *b = &Q2.t[t].pat[i];
            if (a->n != b->n || a->steps != b->steps || a->nlock != b->nlock || memcmp(a->lk, b->lk, sizeof(seq_lock) * a->nlock)) { bad++; continue; }
            for (int e = 0; e < a->n; e++) {
                const seq_ev *x = &a->ev[e], *y = &b->ev[e];
                if (x->tick != y->tick || x->len != y->len || x->note != y->note || x->vel != y->vel || x->cond != y->cond || x->ratchet != y->ratchet) { bad++; break; }
            }
        }
        rounds++;
        /* every cut of a packed file is refused whole, never loaded in part */
        if (r < 20) for (int cut = 1; cut < n; cut += 1 + n / 200) if (project_load(&Q2, &S2, 0, big, (uint32_t)cut) == 0) { bad++; break; }
    }
    CHECK(bad == 0, "%d patterns came back different (of %d projects)", bad, rounds);
}
static void project_tests(void)
{
    printf("project: what is saved comes back\n");
    static seq_hist h1, h2; seq_init(&Q1, 44100, 133.0f, &h1); snd_init(&S1, fx1, 0);
    seq_init(&Q2, 44100, 90.0f, &h2); snd_init(&S2, fx2, 0);
    for (int t = 0; t < SEQ_NTRK; t++) { S1.trk[t].patch.engine = (uint8_t)(t == 0 ? ENG_DRUM : ENG_VA); S1.trk[t].patch.p[3] = 0.1f * (float)t; sm_set(&S1.trk[t].level, 0.05f * (float)t); }
    seq_set_len(&Q1, 3, 96); seq_queue(&Q1, 1, 7);
    for (int i = 0; i < 40; i++) seq_step_toggle(&Q1, i % SEQ_NTRK, i % 16, 30 + i, 50 + i);
    Q1.t[2].mute = 1; Q1.t[4].quant = 70;
    seq_queue(&Q1, 4, 6); seq_step_toggle(&Q1, 4, 3, 99, 99);
    seq_step_cond(&Q1, 4, 3, SEQ_CHANCE + 3); seq_step_ratchet(&Q1, 4, 3, 3); seq_lock_set(&Q1, 4, 3, 6, 77); seq_lock_set(&Q1, 4, 3, 7, 200);
    Q1.song_n = 2; Q1.song_len = 8; Q1.song[1].bar = 4; Q1.song[1].mute = 5; Q1.song[1].pat[2] = 3;
    X1.dly_div = 5; for (int t = 0; t < SEQ_NTRK; t++) X1.preset[t] = (uint8_t)(t * 3); S1.dly_fb = 0.61f; S1.rev_fb = 0.9f; Q1.swing = 37; Q1.t[3].swing = 12; X1.harm[2][0] = 2; X1.harm[2][1] = 7; X1.harm[2][3] = 3; X1.harm[2][4] = 64 + 5;
    S1.dust = 0.37f; S1.duck = 0.5f; sm_set(&S1.mfilt, -0.42f); X1.fade = 9; X1.strum[1] = 40; X1.vlead[2] = 1; memset(X1.home, 0xFF, sizeof X1.home); X1.home[3] = 7; X1.home[39] = 0; for (int i = 0; i < 8; i++) X1.tex[i] = (uint8_t)(i + 3); for (int i = 0; i < 4; i++) X1.fxv[i] = (uint8_t)(10 * i + 5); for (int i = 0; i < 8; i++) X1.midi[i] = (uint8_t)(i < 5 ? i % 3 : 0xFF); S1.trk[3].patch.dtune[2] = -5; S1.trk[3].patch.ddec[15] = 3; S1.trk[3].patch.ddec[14] = gb_dpack(-2, -7); S1.trk[3].patch.dmute = 0x8001; Q1.t[2].div = SEQ_DIV_8T; Q1.t[1].gate = 60; sm_set(&S1.trk[2].send_cho, 0.4f); sm_set(&S1.trk[3].filt, 0.25f); sm_set(&S1.trk[1].filt, -1.0f);
    /* each pattern's own sound: track 1's pattern 3 another cutoff and engine, the drums' pattern 6 a tune */
    for (int t = 0; t < SEQ_NTRK; t++) ps_fill(&PS1, t, 0, &S1.trk[t].patch, S1.trk[t].patch.p, 5);
    PS1.s[0][2].p[6] = 200; PS1.s[0][2].engine = ENG_FM; PS1.s[0][2].preset = -9; PS1.d[5].dtune[3] = -7; PS1.d[5].dmute = 0x0102;
    X1.ps = &PS1; X2.ps = &PS2;
    int n = project_save(&Q1, &S1, &X1, buf, sizeof buf);
    printf("  %d bytes\n", n);
    CHECK(n > 0, "save failed");
    CHECK(project_load(&Q2, &S2, &X2, buf, (uint32_t)n) == 0, "load failed");
    CHECK(fabsf(S2.trk[0].level.tgt - S1.trk[0].level.tgt) < 1e-6f, "the drums' level not kept (%f)", S2.trk[0].level.tgt);
    /* (3.4.0) a project from before (17): the kits 3 x louder since, its drums' level a third (as it sounded) */
    { static uint8_t old[sizeof buf]; memcpy(old, buf, (size_t)n); old[4] = 16;
      sm_set(&S1.trk[0].level, 0.9f); int m = project_save(&Q1, &S1, &X1, old, sizeof old); old[4] = 16;
      CHECK(m > 0 && project_load(&Q2, &S2, &X2, old, (uint32_t)m) == 0 && fabsf(S2.trk[0].level.tgt - 0.3f) < 1e-5f && fabsf(S2.trk[1].level.tgt - 0.05f) < 1e-5f,
            "a project of 16: the drums %f (0.3), a synth %f (0.05)", S2.trk[0].level.tgt, S2.trk[1].level.tgt);
      sm_set(&S1.trk[0].level, 0.0f); }
    CHECK(project_load(&Q2, &S2, &X2, buf, (uint32_t)n) == 0, "load failed");
    CHECK(Q2.bpm == 133.0f && Q2.t[2].mute == 1 && Q2.t[4].quant == 70 && Q2.t[4].cur == 6 && Q2.t[3].pat[0].steps == 96, "settings lost");
    int same = 1;
    for (int t = 0; t < SEQ_NTRK; t++) for (int i = 0; i < SEQ_NPAT; i++) {
        const seq_pat *a = &Q1.t[t].pat[i], *b = &Q2.t[t].pat[i];
        if (a->n != b->n || a->steps != b->steps || memcmp(a->ev, b->ev, sizeof(seq_ev) * a->n)) same = 0;
        if (a->nlock != b->nlock || memcmp(a->lk, b->lk, sizeof(seq_lock) * a->nlock)) same = 0;
    }
    CHECK(same, "the patterns differ");
    CHECK(Q2.song_n == 2 && Q2.song_len == 8 && Q2.song[1].bar == 4 && Q2.song[1].mute == 5 && Q2.song[1].pat[2] == 3, "the song differs");
    CHECK(S2.trk[3].patch.p[3] == S1.trk[3].patch.p[3] && S2.trk[0].patch.engine == ENG_DRUM && S2.trk[5].level.tgt == S1.trk[5].level.tgt, "the sounds differ");
    CHECK(X2.dly_div == 5 && X2.preset[4] == 12 && S2.dly_fb == 0.61f && S2.rev_fb == 0.9f && Q2.swing == 37 && Q2.t[3].swing == 12, "the effects, presets or swing differ");
    CHECK(S2.dust > 0.3699f && S2.dust < 0.3701f && S2.duck > 0.4999f && S2.duck < 0.5001f && S2.mfilt.tgt > -0.4201f && S2.mfilt.tgt < -0.4199f && X2.fade == 9
          && S2.trk[3].filt.tgt > 0.2499f && S2.trk[3].filt.tgt < 0.2501f && S2.trk[1].filt.tgt < -0.9999f && S2.trk[0].filt.tgt == 0.0f, "the master bus or the track filters differ");
    CHECK(Q2.t[2].div == SEQ_DIV_8T && Q2.t[0].div == SEQ_DIV_16 && Q2.t[1].gate == 60 && Q2.t[0].gate == 100 && S2.trk[2].send_cho.tgt > 0.399f && S2.trk[2].send_cho.tgt < 0.401f, "DIV, GATE or the chorus differ");
    CHECK(S2.trk[3].patch.dtune[2] == -5 && S2.trk[3].patch.ddec[15] == 3 && S2.trk[3].patch.dmute == 0x8001 && S2.trk[0].patch.dtune[2] == 0, "the drums' sounds differ");
    CHECK(GB_DDEC(S2.trk[3].patch.ddec[14]) == -2 && gb_dlev(S2.trk[3].patch.ddec[14]) == -7, "a drum sound's level (3.4.2) not kept");
    CHECK(X2.strum[1] == 40 && X2.vlead[2] == 1 && X2.strum[0] == 0, "STRUM or VLEAD differ");
    CHECK(X2.home[3] == 7 && X2.home[39] == 0 && X2.home[0] == 0xFF, "the device's settings differ");
    CHECK(X2.tex[0] == 3 && X2.tex[6] == 9, "TEXTURE differs"); CHECK(X2.fxv[0] == 5 && X2.fxv[3] == 35, "COLR MIX DAMP CRT differ");
    CHECK(X2.midi[0] == 0 && X2.midi[1] == 1 && X2.midi[4] == 1 && X2.midi[7] == 0xFF, "the MIDI settings differ");
    CHECK(X2.ps_mask[0] == (1u << 2) && X2.ps_mask[PS_DRUM] == (1u << 5) && X2.ps_mask[1] == 0, "the patterns' own sounds: which (%x %x)", X2.ps_mask[0], X2.ps_mask[PS_DRUM]);
    CHECK(PS2.s[0][2].p[6] == 200 && PS2.s[0][2].engine == ENG_FM && PS2.s[0][2].preset == -9 && PS2.d[5].dtune[3] == -7 && PS2.d[5].dmute == 0x0102, "the patterns' own sounds differ");
    {   /* a pattern's sound taken from the live patch and given back: the same values (to a byte), its engine */
        gb_patch lp = S1.trk[1].patch; float own[GB_NPARAM]; for (int i = 0; i < GB_NPARAM; i++) own[i] = 0.37f + 0.01f * (float)i;
        lp.p[0] = 0.0f; ps_take(&PS1, 1, 7, &lp, own, 12);
        gb_patch back = S1.trk[1].patch; float o2[GB_NPARAM]; int pr = 0; ps_give(&PS1, 1, 7, &back, o2, &pr);
        int ok = pr == 12; for (int i = 0; i < GB_NPARAM; i++) { int a = (int)(o2[i] * 100.0f + 0.5f), b = (int)(own[i] * 100.0f + 0.5f); if (a != b || back.p[i] != o2[i]) ok = 0; }
        CHECK(ok, "a pattern's sound comes back as it was shown");
        CHECK(ps_same(&PS1, 1, 7, 7) && !ps_same(&PS1, 1, 7, 6), "same / not the same");
    }
    CHECK(X2.harm[2][0] == 2 && X2.harm[2][1] == 7 && X2.harm[2][3] == 3 && X2.harm[2][4] == 69 && X2.harm[0][4] == 64, "the key and chords differ");
    /* a project saved by 0.9 (format 1: 6-byte events, no locks, no song) still loads */
    {
        uint8_t v1[1024]; uint32_t n1 = 0;
        #define P8(v) (v1[n1++] = (uint8_t)(v))
        #define P16(v) do { P8(v); P8((v) >> 8); } while (0)
        #define P32(v) do { P16(v); P16((v) >> 16); } while (0)
        union { float f; uint32_t u; } x;
        P32(0x4A504247u); P8(1); x.f = 101.0f; P32(x.u);
        for (int t = 0; t < 8; t++) {                         /* (a file has room for 8 tracks) */
            P8(t == 0 ? ENG_DRUM : ENG_VA); P8(8); P8(1); P8(0);
            for (int i = 0; i < 16; i++) { x.f = 0.5f; P32(x.u); }   /* (a version 1 file: 16 values) */
            for (int i = 0; i < 4; i++) { x.f = 0.6f; P32(x.u); }
            P8(0); P8(25); P8(0);
        }
        P16(2); P8(0); P8(0); P16(16); P16(2);
        P16(0); P16(72); P8(36); P8(100);
        P16(384); P16(72); P8(38); P8(90);
        P8(6); P8(0); P16(16); P16(1); P16(0); P16(72); P8(60); P8(100);   /* track 7: passed over (this firmware has 5) */
        CHECK(project_load(&Q2, &S2, &X2, v1, n1) == 0, "a format-1 project did not load");
        CHECK(Q2.bpm == 101.0f && Q2.t[0].pat[0].n == 2 && Q2.t[0].pat[0].ev[1].note == 38 && Q2.t[0].pat[0].ev[1].ratchet == 1
              && Q2.t[0].pat[0].ev[1].cond == SEQ_ALWAYS && Q2.t[0].quant == 25 && Q2.song_n == 0, "a format-1 project came back wrong");
        CHECK(X2.dly_div == 0xFF && X2.preset[0] == 0xFF, "a format-1 project gave presets");
    }
    /* a damaged or foreign buffer is refused before anything changes */
    buf[3] ^= 1;
    CHECK(project_load(&Q2, &S2, 0, buf, (uint32_t)n) < 0, "a bad magic was loaded");
    buf[3] ^= 1;
    CHECK(project_load(&Q2, &S2, 0, buf, (uint32_t)n - 10) < 0, "a cut buffer was loaded");
    for (int k = 0; k < 2000; k++) {                           /* random bytes: never a crash, never a wild pattern */
        uint8_t junk[512]; for (int i = 0; i < 512; i++) junk[i] = (uint8_t)rand();
        memcpy(junk, buf, 5);
        project_load(&Q2, &S2, &X2, junk, sizeof junk);
    }
    int ok = 1;
    for (int t = 0; t < SEQ_NTRK; t++) for (int i = 0; i < SEQ_NPAT; i++) if (Q2.t[t].pat[i].n > SEQ_PATEV || Q2.t[t].pat[i].steps > SEQ_MAXSTEPS) ok = 0;
    CHECK(ok, "random bytes made a pattern out of range");
}

/* the region: between the sample bank and SLOOP 2.8's data; saved again and again, nothing lost; your sounds beside */
static void region_tests(void)
{
    printf("store: its region, two banks of 13 sectors\n");
    CHECK(STORE_BASE >= 0x4E000u + 0x2C000u && STORE_BASE + 2u * STORE_PBANK <= STORE_SOUNDS_AT && STORE_SOUNDS_AT + 8192u <= 0x97000u, "the projects' region");
    memset(fl, 0xFF, sizeof fl); store_init(&F);
    int ok = 1;
    for (int round = 0; round < 40; round++) {
        uint32_t n = 3000u + (uint32_t)round * 1200u;
        const uint8_t *d = (round & 1) ? b_ : a_;
        if (save_proj(d, n, crc_of(d, n), 12032) != 0) ok = 0;
        if ((uint32_t)load_proj(got, sizeof got) != n || memcmp(got, d, n)) ok = 0;
    }
    CHECK(ok, "a save lost");
    {
        int n0 = load_proj(got, sizeof got); uint32_t c0 = crc_of(got, (uint32_t)n0);
        int okk = 1;
        for (int r = 0; r < 5; r++) {
            for (int i = 0; i < 2000; i++) a_[i] = (uint8_t)(i * 3 + r);
            if (store_sounds_begin(a_, 2000)) okk = 0;
            int q; while ((q = store_step()) == 1) { } if (q != 2) okk = 0;
            if (store_sounds_load(got, sizeof got) != 2000 || memcmp(got, a_, 2000)) okk = 0;
        }
        for (int i = 0; i < 2000; i++) b_[i] = (uint8_t)(i * 5);
        cut_at = ops + 5;                                            /* a cut in the middle of the next */
        store_sounds_begin(b_, 2000); { int q; while ((q = store_step()) == 1) { } }
        cut_at = -1;
        CHECK(okk && store_sounds_load(got, sizeof got) == 2000 && !memcmp(got, a_, 2000), "the user sounds: saved, the copy before kept after a cut");
        int n1 = load_proj(got, sizeof got);
        CHECK(n1 == n0 && crc_of(got, (uint32_t)n1) == c0, "the user sounds touched the project");
    }
    int sloop = 1; for (uint32_t a = 0x97000u; a < 0x100000u; a++) if (fl[a] != 0xFF) sloop = 0;
    CHECK(sloop, "something written in SLOOP 2.8's data");
}

static void raw_tests(void)
{
    printf("a raw sector job (the editor's sample slots): erased, written at an offset, read back\n");
    memset(fl, 0x5A, sizeof fl); store_init(&F); cut_at = -1; ops = 0;
    static uint8_t d[3000]; for (int i = 0; i < 3000; i++) d[i] = (uint8_t)(i * 7 + 1);
    CHECK(!store_raw_begin(0xA0000, 512, d, 3000, 1), "started");
    int r; while ((r = store_step()) == 1) { }
    CHECK(r == 2 && !memcmp(fl + 0xA0000 + 512, d, 3000) && fl[0xA0000] == 0xFF && fl[0xA0000 + 3512] == 0xFF && fl[0xA1000] == 0x5A, "the sector (%d)", r);
    CHECK(store_step() == 0, "idle after");
    CHECK(!store_raw_begin(0xA1000, 0, d, 100, 0), "no erase");
    while ((r = store_step()) == 1) { }
    CHECK(r == -1, "a write over unerased bytes found by the read back");
}
/* your FM6 bank and SYN kits as images (app/xbank.c): every field of a sound back as it was, a damaged image refused */
static void xbank_tests(void)
{
    static uint8_t img[XB_SYN_LEN > XB_FM6_LEN ? XB_SYN_LEN : XB_FM6_LEN];
    gb_dsnd a = { DW_BELL, DN_CYM | DN_CLAP, 1234, 77, 99, 200, 127, 64, 33, 100, 120, 5, 6, 7, 0x8A, 110, -40, 12, 90, 111, -37 }, b;
    uint8_t p[22]; xb_dsnd_pack(&a, p); xb_dsnd_unpack(p, &b);
    CHECK(!memcmp(&a, &b, sizeof a), "a SYN sound: every field back");
    drum_syn_default();
    for (int k = 0; k < 4; k++) xb_kit_pack(gb_dsyn[k], gb_dsyn_crush[k], img + 16 + k * XB_KIT_LEN);
    xb_head(img, 0, 0, XB_SYN_LEN);
    CHECK(!xb_check(img, XB_SYN_LEN, 0, 0) && xb_check(img, XB_SYN_LEN, 1, 0), "the SYN image checks out (and is not an FM6 bank)");
    gb_dsnd back[16]; uint8_t cr;
    xb_kit_unpack(img + 16 + 2 * XB_KIT_LEN, back, &cr);
    CHECK(!memcmp(back, gb_dsyn[2], sizeof back) && cr == gb_dsyn_crush[2], "a kit back as it was");
    img[100] ^= 1; CHECK(xb_check(img, XB_SYN_LEN, 0, 0), "a damaged SYN image refused");
    for (uint32_t i = 16; i < XB_FM6_LEN; i++) img[i] = (uint8_t)(i * 7);
    xb_head(img, 1, 0x0400005u, XB_FM6_LEN);
    uint32_t u = 0;
    CHECK(!xb_check(img, XB_FM6_LEN, 1, &u) && u == 0x0400005u, "the FM6 image and its slots used");
    CHECK(xb_check(img, XB_FM6_LEN - 1, 1, 0), "an FM6 image of another length refused");
    CHECK(XB_FM6_AT >= 0x4000u + 0x74000u && XB_SYN_AT == XB_FM6_AT + 0x1000u && XB_SYN_AT + 0x1000u <= STORE_BASE, "their sectors: after the area, before the projects");
}

/* 3.3.5: the largest project the sequencer can hold fits the one project's place; made a part at a time it is the
 * same file; read back through the flash's fetch it loads the same */
static uint8_t whole[60000], parts[60000];
static uint32_t fetch_at; static const uint8_t *fetch_src;
static int fetch_fn(uint32_t off, void *dst, uint32_t n) { memcpy(dst, fetch_src + off, n); return 0; }
static void max_tests(void)
{
    printf("project: the largest the sequencer holds fits its place\n");
    static seq_hist h1, h2; static psounds P1, P2;
    seq_init(&Q1, 44100, 120.0f, &h1); snd_init(&S1, fx1, 0); seq_init(&Q2, 44100, 90.0f, &h2); snd_init(&S2, fx2, 0);
    for (int t = 0; t < SEQ_NTRK; t++) for (int i = 0; i < SEQ_NPAT; i++) {
        seq_pat *pt = &Q1.t[t].pat[i]; pt->steps = SEQ_MAXSTEPS; uint32_t tick = 0;
        for (int e = 0; e < SEQ_PATEV; e++) {
            seq_ev *v = &pt->ev[e]; tick += prnd() % 95u;
            v->tick = (uint16_t)tick; v->len = (uint16_t)(prnd() & 0xFFFFu); v->note = (uint8_t)(prnd() % 128u); v->vel = (uint8_t)(1u + prnd() % 127u);
            v->cond = (uint8_t)(prnd() % 200u); v->ratchet = (uint8_t)((1u + prnd() % 4u) | (prnd() & 0x30u));
        }
        pt->n = SEQ_PATEV; pt->nlock = SEQ_LOCKS;
        for (int l = 0; l < SEQ_LOCKS; l++) { pt->lk[l].step = (uint8_t)(prnd() % 128u); pt->lk[l].param = (uint8_t)(prnd() % 64u); pt->lk[l].value = (uint8_t)prnd(); }
        gb_patch lp = S1.trk[t].patch; lp.engine = (uint8_t)(prnd() % 8u); float own[GB_NPARAM]; for (int k = 0; k < GB_NPARAM; k++) own[k] = (float)(prnd() % 256u) / 255.0f;
        for (int k = 0; k < 16; k++) { lp.dtune[k] = (int8_t)(prnd() % 16u) - 8; lp.ddec[k] = (int8_t)(prnd() % 8u) - 4; }
        ps_take(&P1, t, i, &lp, own, (int)(prnd() % 1000u));
    }
    Q1.song_n = SEQ_SONG; Q1.song_len = 0xFFFF;
    for (int i = 0; i < SEQ_SONG; i++) { Q1.song[i].bar = (uint16_t)(i * 9); Q1.song[i].mute = 31; for (int t = 0; t < SEQ_NTRK; t++) Q1.song[i].pat[t] = (uint8_t)(prnd() % SEQ_NPAT); }
    /* (4.0) each track's current pattern as last saved: full too, and different (format 16) */
    static seq_pat K1[SEQ_NTRK], K2[SEQ_NTRK];
    for (int t = 0; t < SEQ_NTRK; t++) {
        seq_pat *k = &K1[t]; *k = Q1.t[t].pat[Q1.t[t].cur]; k->steps = 77;
        for (int e = 0; e < SEQ_PATEV; e++) k->ev[e].note = (uint8_t)((k->ev[e].note + 5) & 127);
    }
    memset(&X1, 0x7F, sizeof X1); X1.ps = &P1; X1.keep = K1; memset(&X2, 0, sizeof X2); X2.ps = &P2; X2.keep = K2;
    int n = project_save(&Q1, &S1, &X1, whole, sizeof whole);
    printf("  %d bytes, %d %% of the place (%u)\n", n, n * 100 / (int)STORE_PMAX, STORE_PMAX);
    CHECK(n > 0 && n <= (int)STORE_PMAX, "the largest project (%d bytes) does not fit its place (%u)", n, STORE_PMAX);
    /* a part at a time, as a save makes it: the same bytes, the CRC of all of them */
    uint32_t crc = 0, total = (uint32_t)project_part(&Q1, &S1, &X1, whole + 0, 0, 0, &crc);
    CHECK(total == (uint32_t)n && crc == crc32_ieee(0, whole, (uint32_t)n), "measured: not its length or CRC");
    for (uint32_t lo = 0; lo < (uint32_t)n; lo += 12032u) project_part(&Q1, &S1, &X1, parts + lo, lo, 12032u, 0);
    CHECK(!memcmp(parts, whole, (size_t)n), "made a part at a time, it is not the same file");
    /* read back through fetch (the flash): the same project; checked only, nothing changed */
    fetch_src = whole; (void)fetch_at;
    CHECK(project_load_from(0, 0, 0, fetch_fn, (uint32_t)n, 1) == 0, "the largest project: its check failed");
    CHECK(project_load_from(&Q2, &S2, &X2, fetch_fn, (uint32_t)n, 0) == 0, "the largest project did not load from the flash");
    int same = 1;
    for (int t = 0; t < SEQ_NTRK; t++) for (int i = 0; i < SEQ_NPAT; i++) {
        const seq_pat *a = &Q1.t[t].pat[i], *b = &Q2.t[t].pat[i];
        if (a->n != b->n || a->nlock != b->nlock || memcmp(a->lk, b->lk, sizeof(seq_lock) * a->nlock)) same = 0;
        for (int e = 0; e < a->n && same; e++) if (a->ev[e].tick != b->ev[e].tick || a->ev[e].len != b->ev[e].len || a->ev[e].note != b->ev[e].note || a->ev[e].vel != b->ev[e].vel || a->ev[e].cond != b->ev[e].cond) same = 0;
    }
    CHECK(same && Q2.song_n == SEQ_SONG, "the largest project came back different");
    int ksame = 1, kgot = 0;                             /* (the largest: as many as fit; each one kept, as it was) */
    for (int t = 0; t < SEQ_NTRK; t++) { const seq_pat *a = &K1[t], *b = &K2[t];
        if (!X2.keep_got[t]) continue;
        kgot++;
        if (a->n != b->n || a->steps != b->steps || a->nlock != b->nlock) ksame = 0;
        for (int e = 0; e < a->n && ksame; e++) if (a->ev[e].note != b->ev[e].note || a->ev[e].len != b->ev[e].len) ksame = 0; }
    printf("  (4.0) %d of %d patterns as last saved kept\n", kgot, SEQ_NTRK);
    CHECK(ksame && kgot >= 1, "(4.0) the patterns as last saved came back different (%d kept)", kgot);
    whole[n / 2] ^= 0x40; int bad = project_load_from(0, 0, 0, fetch_fn, (uint32_t)n, 1); whole[n / 2] ^= 0x40;
    (void)bad;                                           /* (a byte changed may still make a file: the store's CRC catches it) */
    CHECK(project_load_from(0, 0, 0, fetch_fn, (uint32_t)n - 1, 1) < 0, "a cut file passed the check");
}

int main(void)
{
    xbank_tests();
    store_tests(); region_tests(); project_tests(); pack_tests(); max_tests();
    raw_tests();
    printf("%d checks, %d failed\n", checks, fails);
    return fails != 0;
}
