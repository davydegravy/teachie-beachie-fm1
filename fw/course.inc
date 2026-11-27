/* SPDX-License-Identifier: GPL-3.0-only
 * TEACHIE BEACHIE: the course on the FM-1 itself, with no computer (part of fw/main.c: it reads SLOOP's own state).
 * The lessons are the ones a computer plays (tutorial/lessons/NN-name.md), packed by tools/mkcourse.py and kept in a
 * user sample slot behind a header SLOOP's own samples never have ("TPCO"). START on the start card, and no computer
 * answering, begins it. Every step is shown with the guide above (cards, strips, lights, the keys it takes), and is
 * done when SLOOP is in the state the lesson asks for, however the learner got there.
 * Its numbers (kinds, fields, instructions, conditions) are app/tp_course.h, written by the same tool. */
#include "../app/tp_course.h"
/* (CCOLD, fw/main.c: in the flash window, and as small as it can be made: it runs once a round, with time to spare) */
#define C_DRUM 3                                    /* the drum track */
enum { PH_OFF, PH_BEGIN, PH_LIST, PH_STEP, PH_PLAY, PH_ECHO, PH_DO, PH_ANSWER, PH_WAIT, PH_MSG, PH_STUCK, PH_END, PH_BYE, PH_TOC };   /* (PH_TOC: the card of the whole course, after START) */
static const uint8_t *tpc;                          /* the pack, read through the flash window; 0: none found */
static struct {
    uint8_t ph, ret, les, nstep, step, hint, at, turned, need, kplay, lastb, ev_r, gen, clip, go, said, turnedk, reshow, bar, held, wrong;   /* wrong: the drum picked that the step did not ask for, once said (its number + 1) */
    uint16_t pressed, twice, played;
    const uint8_t *st;                              /* the step under way (its length, then its kind) */
    uint32_t ms, last, t0, t_hint, t_end, t_off, t_lastb, t_out;   /* ms: our own clock (the firmware's wraps every 72 s) */
    int32_t latch[8];                               /* what each condition's number was when the step began ("changed") */
    int8_t clk[7];                                  /* each knob's clicks since the step began, right less left (the frames' fill) */
} C;

CCOLD static uint32_t c_u16(const uint8_t *p) { return (uint32_t)p[0] | (uint32_t)p[1] << 8; }
CCOLD static const uint8_t *c_lesson(int i) { return tpc + c_u16(tpc + 8 + 2 * i); }
CCOLD static const uint8_t *c_clip(int i) { return tpc + c_u16(tpc + 8 + 2 * tpc[6] + 2 * i); }
/* a field of the step under way: its bytes, *n how many; 0: it has none. nth: which, of several alike (the hints) */
CCOLD static const uint8_t *c_field(int tag, int nth, int *n)
{
    const uint8_t *p = C.st + 4, *e = C.st + 2 + c_u16(C.st);
    for (; p < e; p += 2 + p[1]) if (p[0] == tag && !nth--) { *n = p[1]; return p + 2; }
    *n = 0; return 0;
}
CCOLD static int c_same(const char *a, const uint8_t *b) { while (*a && *a == (char)*b) a++, b++; return !*a && !*b; }
CCOLD static int c_in(const char *hay, const uint8_t *nd)       /* nd somewhere in hay */
{
    for (; *hay; hay++) { int i = 0; while (nd[i] && hay[i] == (char)nd[i]) i++; if (!nd[i]) return 1; }
    return !*nd;
}
CCOLD static uint32_t c_u32(const uint8_t *p) { return c_u16(p) | c_u16(p + 2) << 16; }
CCOLD static int course_find(void)                   /* the pack, in whichever slot holds it; the start's logo, at the slot's end */
{
    tpc = 0; tp_logo = 0;
    for (int i = SMP_NUSR - 1; i >= 0 && flash_ok; i--) {
        const uint8_t *h = (const uint8_t *)(uintptr_t)(FLASH_WIN + SLOT_AT[i] - 0x4000u);
        if (h[0] == 'T' && h[1] == 'P' && h[2] == 'C' && h[3] == 'O' && h[512] == 'T' && h[513] == 'P' && h[514] == 'C' && h[515] == '4' && h[518]) {   /* (TPC4: SLOOP 3.4's lessons; TPC3 was 3.3.7's) */
            tpc = h + 512;
            {                                            /* the logo: its place and length at 24 and 28 (tools/mkcourse.py slot_image) */
                uint32_t at = c_u32(h + 24), n = c_u32(h + 28); const uint8_t *g = tpc + at;
                if (n > 6 && n <= SMP_USR_SIZE - 512u && at <= SMP_USR_SIZE - 512u - n && g[0] == 'T' && g[1] == 'P' && g[2] == 'L' && g[3] == '1' && g[4] && g[4] <= GFX_W && g[5] && g[5] <= GFX_H) tp_logo = g;
            }
            for (int k = 0; k < 2; k++) {                /* its two fonts, after the lessons and the clips: 6 bytes, the glyphs, the bits */
                const uint8_t *b = tpc + c_u16(tpc + 8 + 2 * (tpc[6] + tpc[7] + k));
                tp_font[k] = (gfx_font){ (const gfx_glyph *)(const void *)(b + 6), b + 6 + 8 * b[0], b[0], b[1], b[2], b[3], b[4], b[5] };
            }
            tp_fonts = 1;
            return 1;
        }
    }
    tp_fonts = 0;
    return 0;
}
/* ---- the lesson remembered after the FM-1 is switched off (step 8 of the move to SLOOP 3.4.2; NOTES, item 118).
 * Where: the blank bytes of the pack's slot, between the pack's end (its length, at 4) and the logo (its place, in the
 * slot's header at 24). When a lesson begins, its number is written into the next blank byte; the first START after
 * the pack is found reads the last one written. Blank flash takes a byte without an erase, and the learner's song is
 * not touched. The rules:
 *   one new byte for each record, and no byte written twice (the original FM-1's flash is known for not taking a
 *   second write into a byte that holds something); a lesson begun again writes nothing; the bytes used up, nothing
 *   more is written and the last record stays;
 *   a record is a byte with exactly four bits set, the lesson's number counted among those (70 of them). A write cut
 *   by a power-off leaves more bits set, or the byte blank: such a byte is no record, and the one before it stands.
 * The write itself is started by the loop in fw/main.c, from RAM (tp_rec_at, tp_rec_b). */
static uint16_t tp_rec_n;                           /* the blank bytes used so far (written, or asked for) */
static uint8_t tp_rec_last, tp_rec_read;            /* the lesson last recorded + 1 (0: none); the records read since the pack was found */
CCOLD static int c_rec_byte(int les)                 /* a lesson (from 0) as a record: the byte with four bits set that many on from the first */
{
    for (int b = 0; ; b++) { int n = 0; for (int x = b; x; x &= x - 1) n++; if (n == 4 && !les--) return b; }
}
CCOLD static const uint8_t *c_rec_area(uint32_t *len)   /* the blank bytes: where they begin, how many (none without a logo: an odd slot) */
{
    uint32_t a = c_u16(tpc + 4), e = c_u32(tpc - 512 + 24);
    *len = tp_logo && e > a ? e - a : 0;
    return tpc + a;
}
CCOLD static void c_rec_read(void)                   /* the first START: how many bytes are used, and the last lesson among them */
{
    uint32_t len, k = 0; const uint8_t *p = c_rec_area(&len);
    while (k < len && p[k] != 0xFF) k++;
    tp_rec_n = (uint16_t)k; tp_rec_last = 0; tp_rec_read = 1;
    while (k-- && !tp_rec_last) for (int l = 0; l < tpc[6]; l++) if (c_rec_byte(l) == p[k]) tp_rec_last = (uint8_t)(l + 1);   /* (from the last byte back: the first that is a lesson's) */
}
CCOLD static void c_rec_ask(void)                    /* this lesson begins: remembered, unless it is the one remembered already */
{
    uint32_t len; const uint8_t *p = c_rec_area(&len);
    if (!tp_rec_read || tp_rec_last == C.les + 1) return;
    if (!tp_rec_at) {                                /* (one still waiting for the flash: its byte takes this lesson instead) */
        if (tp_rec_n >= len) return;
        tp_rec_at = (uint32_t)((uintptr_t)(p + tp_rec_n) - FLASH_WIN) + 0x4000u; tp_rec_n++;
    }
    tp_rec_b = (uint8_t)c_rec_byte(C.les); tp_rec_last = (uint8_t)(C.les + 1);
}
/* a slot is about to be erased or rewritten (the editor's sample messages): if it is the one the pack is in, the
 * course stops and the pack, its fonts and the logo are forgotten, for they are read from that slot: a card drawn in
 * the pack's font while its bytes were being replaced read past the flash (2026-11-25). The start's card, in SLOOP's
 * own font, until course_found */
CCOLD static void course_stop(void); COLD static void guide_home(void);
CCOLD static void course_forget(int k)
{
    if (!tpc || tpc != (const uint8_t *)(uintptr_t)(FLASH_WIN + SLOT_AT[k] - 0x4000u) + 512) return;
    int was = C.ph != PH_OFF || G.home;
    course_stop(); tpc = 0; tp_logo = 0; tp_fonts = 0;
    tp_rec_at = 0; tp_rec_read = 0; C.les = 0;       /* (the lesson remembered: gone with the slot; lessons sent again start at lesson 1) */
    if (was) guide_home(); else { guide_wrap(); G.gen++; }
}
CCOLD static int course_found(void)                  /* a slot written: the pack looked for again; the start's card in its font */
{
    int ok = course_find();
    if (ok && G.home) guide_home();
    return ok;
}
/* ---- what the learner sees */
CCOLD static void c_leds(void) { for (int i = 0; i < 41; i++) G.led[i] = 0; G.only = 0; }
CCOLD static int c_strip(int kind) { return kind == CK_STRIP ? 2 : kind == CK_STRIP_TOP ? 1 : kind == CK_STRIP_CORNER ? 3 : 0; }   /* a strip's place (0: a card) */
CCOLD static void c_show(int kind, const uint8_t *t, int n, const char *l, const char *r)
{
    G.card = 1; G.big = kind == CK_BIG || kind == CK_REPLY; G.strip = (uint8_t)c_strip(kind);
    if (n > (int)sizeof G.text - 1) n = (int)sizeof G.text - 1;
    for (int i = 0; i < n; i++) G.text[i] = (char)t[i];
    G.text[n] = 0;
    int i = 0; for (; l && l[i] && i < (int)sizeof G.left - 1; i++) G.left[i] = l[i];
    G.left[i] = 0;
    for (i = 0; r && r[i] && i < (int)sizeof G.right - 1; i++) G.right[i] = r[i];
    G.right[i] = 0;
    guide_wrap(); G.gen++;
}
CCOLD static void c_say(int kind, const char *s, const char *l, const char *r) { int n = 0; while (s[n]) n++; c_show(kind, (const uint8_t *)s, n, l, r); }
CCOLD static void c_words(void)                      /* the step's own words again, with its answers */
{
    int n, ln, rn; const uint8_t *t = c_field(CT_TEXT, 0, &n), *l = c_field(CT_LEFT, 0, &ln), *r = c_field(CT_RIGHT, 0, &rn);
    if (t) c_show(C.st[2], t, n, (const char *)l, (const char *)r);
}
CCOLD static void c_key(int k, int down)            /* a key played from here, as the editor's LIVE page does */
{
    uint32_t m = 1u << k;
    if (down) { if (!(ek_held & m)) ek_pr |= m; ek_held |= m; } else if (ek_held & m) { ek_held &= ~m; ek_rel |= m; }
}
CCOLD static void c_goto(int pg)                     /* a page of SLOOP, as its button would open it */
{
    if (pg == PG_NOTES) { ne_step = 0; ne_sub = 0; }
    {   int b = group_of(pg); const uint8_t *l; int n = grp_of(b, &l);   /* (SLOOP 3.4: each button remembers its page among its own, grp_last) */
        for (int i = 0; i < n; i++) if (l[i] == pg) grp_last[b] = (uint8_t)i;
    }
    vis_on = 0; S.vis_ring = 0; sticky = pg;
}
CCOLD static void c_pick(int pad) { last_notes[C_DRUM] = pad; last_chord[C_DRUM][0] = (uint8_t)pad; last_chord_n[C_DRUM] = 1; }
/* ---- the FM-1 made ready */
CCOLD static int c_preset(const uint8_t *name, int drum)
{
    for (int i = 0; i < gb_npresets; i++) if ((gb_presets[i].kind == K_KIT) == drum && c_same(gb_presets[i].name, name)) return i;
    return -1;
}
/* a clip set's notes into pattern pat of every track (pat < 0: the patterns playing, and its sounds, levels and tempo
 * too); only_empty: the tracks with nothing on them alone. Answers the tracks whose playing pattern it wrote */
CCOLD static uint32_t c_clip_load(int id, int pat, int only_empty)
{
    const uint8_t *p = c_clip(id), *snd[NTRK]; uint32_t wrote = 0;
    if (pat < 0 && !only_empty) seq_set_bpm(&Q, (float)c_u16(p) * 0.1f);
    p += 2;
    for (int t = 0; t < NTRK; t++) { snd[t] = p; while (*p) p++; p += 3; }
    for (int t = 0; t < NTRK; t++) {
        int steps = (int)c_u16(p), n = (int)c_u16(p + 2), pt = pat < 0 ? Q.t[t].cur : pat; p += 4;
        if (!only_empty || !Q.t[t].pat[pt].n) {
            if (pat < 0) {
                const uint8_t *e = snd[t]; while (*e) e++;
                int np = c_preset(snd[t], e[1]);
                if (np >= 0) apply_preset(t, np);
                sm_set(&S.trk[t].level, (float)e[2] * (1.0f / 127.0f));
                ps_keep(t); for (int i = 0; i < SEQ_NPAT; i++) ps_copy(&PS, t, PS.cur[t], i);   /* (its 8 patterns: the same sound) */
            }
            seq_pat_put2(&Q, t, pt, 0, steps, p, n, 8);
            if (pt == Q.t[t].cur) wrote |= 1u << t;
        }
        p += 8 * n;
    }
    return wrote;
}
CCOLD static void c_add(int t, int note, int vel, int cond, uint32_t mask)   /* a note in each of these boxes that lacks it */
{
    seq_pat *p = &Q.t[t].pat[Q.t[t].cur];
    for (int b = 0; b < 16; b++) if ((mask >> b) & 1u && !seq_step_has(&Q, t, b, note)) {
        uint32_t tk = (uint32_t)b * SEQ_TPS, ln = is_drum(t) ? SEQ_TPS : 2 * SEQ_TPS;
        uint8_t x[8] = { (uint8_t)tk, (uint8_t)(tk >> 8), (uint8_t)ln, (uint8_t)(ln >> 8), (uint8_t)note, (uint8_t)vel, (uint8_t)cond, 1 };
        seq_pat_put2(&Q, t, Q.t[t].cur, p->n ? p->n : 0, p->steps, x, 1, 8);
    }
}
/* (SLOOP 3.4: each track's pattern keeps a saved copy, put back when the FM-1 leaves it (keep[], fw/main.c). What a
 * setup writes into a pattern that is playing, notes or a sound, is made that track's saved copy at once, or the
 * lesson's song would empty at the first change of pattern. A track it did not write keeps the learner's own, saved
 * or not; a pattern that is not playing is its own saved copy already; a new project saves its own (new_project)) */
CCOLD static void c_setup(const uint8_t *e, int n)
{
    uint32_t kept = 0;                              /* the tracks this setup wrote */
    for (const uint8_t *end = e + n; e < end; e += 1 + e[0]) {
        const uint8_t *a = e + 2; int t = a[0] < NTRK ? a[0] : 0;
        switch (e[1]) {
        case CS_NEW: case CS_SONG:
            if (Q.playing) transport(0);
            new_project(0); recording_armed = 0; oct = 0; select_track(0); page = 0; c_goto(PG_TRACKS);
            if (e[1] == CS_SONG) { kept |= c_clip_load(a[0], -1, 0); C.clip = a[0]; }
            break;
        case CS_PATTERN: kept |= c_clip_load(a[1], a[0], 0); break;
        case CS_EMPTY: seq_pat_put2(&Q, t, Q.t[t].cur, 0, Q.t[t].pat[Q.t[t].cur].steps, a, 0, 8); kept |= 1u << t; break;
        case CS_STEPS: Q.t[t].pat[Q.t[t].cur].steps = a[1]; kept |= 1u << t; break;
        case CS_ADD: c_add(t, a[1], a[2], a[3], c_u16(a + 4)); kept |= 1u << t; break;
        case CS_TRACK: select_track(t); break;
        case CS_TEMPO: seq_set_bpm(&Q, (float)c_u16(a) * 0.1f); break;
        case CS_SWING: Q.swing = a[0]; break;
        case CS_PAGE: c_goto(a[0]); break;
        case CS_PLAY: transport(1); break;
        case CS_STOP: transport(0); break;
        case CS_MUTE: for (int i = 0; i < NTRK; i++) if ((a[0] >> i) & 1u) seq_mute(&Q, i, 1); break;
        case CS_SOLO: for (int i = 0; i < NTRK; i++) if ((a[0] >> i) & 1u) solo[i] = 1; break;
        case CS_UNMUTE: for (int i = 0; i < NTRK; i++) { seq_mute(&Q, i, 0); solo[i] = 0; } break;
        case CS_SOUND: { int np = c_preset(a + 1, is_drum(t)); if (np >= 0) apply_preset(t, np); } kept |= 1u << t; break;
        case CS_LEVEL: sm_set(&S.trk[t].level, (float)a[1] * (1.0f / 127.0f)); break;
        case CS_PICK: c_pick(a[0]); break;
        case CS_KEY: for (int i = 0; i < NTRK; i++) if (!is_drum(i)) { harm[i].root = a[0]; harm[i].scale = a[1]; } break;
        case CS_FILLEMPTY: kept |= c_clip_load(C.clip, -1, 1); break;
        case CS_LINES:
            for (int i = 0; i < a[0] && i < SEQ_SONG; i++) { Q.song[i].bar = (uint16_t)(i * a[1]); Q.song[i].mute = 0; for (int k = 0; k < SEQ_NTRK; k++) Q.song[i].pat[k] = a[2 + i]; }
            Q.song_n = a[0]; Q.song_len = (uint16_t)(a[0] * a[1]); break;
        case CS_SONGMODE: seq_song_play(&Q, a[0]); break;
        case CS_MFILT: sm_set(&S.mfilt, (float)(int8_t)a[0] * 0.01f); break;
        case CS_SEND: sm_set(a[1] == 0 ? &S.trk[t].send_cho : a[1] == 1 ? &S.trk[t].send_dly : &S.trk[t].send_rev, (float)a[2] * (1.0f / 127.0f)); break;
        case CS_RECOFF: if (Q.recording) seq_record(&Q, 0, trk); recording_armed = 0; break;
        default: break;
        }
    }
    for (int t = 0; t < NTRK; t++) if ((kept >> t) & 1u) commit_trk(t);
}
/* ---- how the lesson knows */
CCOLD static uint32_t c_miss(const uint8_t *c)       /* HAS: the boxes that still lack the note */
{
    uint32_t m = c_u16(c + 3), out = 0;
    for (int b = 0; b < 16; b++) if ((m >> b) & 1u && !seq_step_has(&Q, c[1], b, c[2] == 255 ? -1 : c[2])) out |= 1u << b;
    return out;
}
CCOLD static int32_t c_num(const uint8_t *c)
{
    int t = c[2] < NTRK ? c[2] : 0, v = 0; const seq_pat *p = &Q.t[t].pat[Q.t[t].cur]; const gb_track *k = &S.trk[t];
    switch (c[1]) {
    case CN_TEMPO: return (int32_t)(Q.bpm + 0.5f);
    case CN_SWING: return Q.swing;
    case CN_MFILT: return (int32_t)(S.mfilt.tgt * 100.0f + (S.mfilt.tgt >= 0.0f ? 0.5f : -0.5f));
    case CN_DUST: return (int32_t)(S.dust * 100.0f + 0.5f);
    case CN_DUCK: return (int32_t)(S.duck * 100.0f + 0.5f);
    case CN_TRK:
        switch (c[3]) {
        case CP_LEVEL: return level_of(t);
        case CP_PAN: return (int32_t)(k->pan.tgt * 100.0f + 0.5f) - 50;
        case CP_CHORUS: return (int32_t)(k->send_cho.tgt * 100.0f + 0.5f);
        case CP_DELAY: return (int32_t)(k->send_dly.tgt * 100.0f + 0.5f);
        case CP_REVERB: return (int32_t)(k->send_rev.tgt * 100.0f + 0.5f);
        case CP_FILTER: return (int32_t)(k->filt.tgt * 100.0f + (k->filt.tgt >= 0.0f ? 0.5f : -0.5f));
        case CP_STEPS: return p->steps;
        case CP_SWING: return Q.t[t].swing;
        case CP_QUANT: return Q.t[t].quant;
        default: return Q.t[t].gate;
        }
    case CN_VALUE: { const gb_pd *d = ed_pd(k->patch.engine, c[3]); return gb_iv(base[t][c[3] < GB_NPARAM ? c[3] : 0], d->min, d->max); }
    case CN_NOTES: return p->n;
    case CN_LONGEST: for (int i = 0; i < p->n; i++) if (p->ev[i].len > v) v = p->ev[i].len; return v * 10 / SEQ_TPS;
    case CN_SOFTEST: v = 127; for (int i = 0; i < p->n; i++) if (p->ev[i].vel < v) v = p->ev[i].vel; return v;
    case CN_LOUDEST: for (int i = 0; i < p->n; i++) if (p->ev[i].vel > v) v = p->ev[i].vel; return v;
    case CN_FILLS: for (int i = 0; i < p->n; i++) v += p->ev[i].cond == SEQ_FILL; return v;
    case CN_HARD: for (int i = 0; i < p->n; i++) v += p->ev[i].vel > 115; return v;
    case CN_SOFT: for (int i = 0; i < p->n; i++) v += p->ev[i].vel < 65; return v;
    case CN_PATTERNS: { uint32_t m = 0; for (int i = 0; i < NTRK; i++) for (int j = 0; j < SEQ_NPAT; j++) if (Q.t[i].pat[j].n) m |= 1u << j; for (; m; m >>= 1) v += (int)(m & 1u); return v; }
    case CN_TRACKS: for (int i = 0; i < NTRK; i++) { int any = 0; for (int j = 0; j < SEQ_NPAT; j++) any |= Q.t[i].pat[j].n != 0; v += any; } return v;
    case CN_LINES: return Q.song_n;
    default: return C.played;
    }
}
CCOLD static int c_true(const uint8_t *c, int i)     /* condition number i of the step (c: its number, then its values) */
{
    int t = c[1] < NTRK ? c[1] : 0;
    switch (c[0]) {
    case CC_PLAYING: return Q.playing;
    case CC_STOPPED: return !Q.playing;
    case CC_RECORDING: return Q.recording;
    case CC_NOTREC: return !Q.recording && !recording_armed;
    case CC_ARMED: return recording_armed;
    case CC_SONGPLAY: return Q.song_play;
    case CC_SONGREC: return Q.song_rec;
    case CC_FILLON: return Q.fill;
    case CC_PUNCHON: return punch_key >= 0 || mfx_key >= 0;
    case CC_NOMUTE: for (int k = 0; k < NTRK; k++) if (Q.t[k].mute) return 0; return 1;
    case CC_NOSOLO: return !solo_any();
    case CC_TRACKIS: return trk == c[1];
    case CC_PAGEIS: return sticky == c[1];
    case CC_MUTED: return (Q.t[t].mute != 0) == c[2];
    case CC_SOLOED: return (solo[t] != 0) == c[2];
    case CC_HAS: return !c_miss(c);
    case CC_HASNO: { uint32_t m = c_u16(c + 3); for (int b = 0; b < 16; b++) if ((m >> b) & 1u && seq_step_has(&Q, t, b, c[2] == 255 ? -1 : c[2])) return 0; return 1; }
    case CC_EMPTY: return !Q.t[t].pat[Q.t[t].cur].n;
    case CC_NUM: {
        int32_t v = c_num(c), k = (int16_t)c_u16(c + 5);
        switch (c[4]) { case CM_ABOVE: return v > k; case CM_BELOW: return v < k; case CM_IS: return v == k; case CM_IS_NOT: return v != k;
                        case CM_AT_LEAST: return v >= k; case CM_AT_MOST: return v <= k; default: return i < 8 && v != C.latch[i]; }
    }
    case CC_SOUNDCHG: return i < 8 && preset_of[t] != C.latch[i];
    case CC_SOUNDIS: { int po = preset_origin(preset_of[t]); return (po >= 0 && c_same(snd_name(po), c + 3)) != c[2]; }
    case CC_INSERT: return (gb_pv(S.trk[t].patch.p, SP_ITYPE) == c[2]) != c[3];
    case CC_ONPAT: return Q.t[t].cur == c[2] && (Q.t[t].next == SEQ_NONE || Q.t[t].next == c[2]);
    case CC_PATHAS: return (Q.t[t].pat[c[2] < SEQ_NPAT ? c[2] : 0].n != 0) == c[3];
    case CC_SAYS: case CC_SAYSONLY: return (C.said >> i) & 1u;
    case CC_PRESSED: return (C.pressed >> c[1]) & 1u;
    case CC_TWICE: return (C.twice >> c[1]) & 1u;
    case CC_TURNED: return (C.turnedk >> c[1]) & 1u;
    case CC_LINEPAT: return c[1] < Q.song_n && Q.song[c[1]].pat[0] == c[2];
    case CC_LINEBARS: return c[1] < Q.song_n && seq_song_len_of(&Q, c[1]) == c[2];
    case CC_PICKED: return (last_notes[C_DRUM] & 15) == c[1];   /* (the drum that boxes write: the grid's black keys and SOUND, a drum played on TRACKS) */
    default: return 1;
    }
}
/* the step's conditions: 1 all true (0 none to weigh: *any 0). miss: the boxes a HAS still lacks, the drums' in bits 0..15
 * and another track's in bits 16..31; need: the drum it asks for */
CCOLD static int c_until(int *any, uint32_t *miss, int *need)
{
    int n, i = 0, ok = 1; const uint8_t *e = c_field(CT_UNTIL, 0, &n);
    if (miss) *miss = 0;
    if (need) *need = 255;
    *any = e != 0;
    for (const uint8_t *end = e ? e + n : 0; e && e < end; e += 1 + e[0], i++) {
        /* (SLOOP 3.4) the white keys are boxes on the drum grid only. On another track the step's Do blinks its own keys:
         * on NOTES the key played is written at the step shown, so there it blinks while STEP is on a step that still
         * lacks the note (PH_DO, below) */
        if (e[1] == CC_HAS) { if (miss) *miss |= c_miss(e + 1) << (e[2] == C_DRUM ? 0 : 16); if (need && e[2] == C_DRUM && e[3] < 16) *need = e[3]; }
        if (!c_true(e + 1, i)) ok = 0;
    }
    return ok;
}
CCOLD static void c_fix(void)                        /* SHOW ME: what can be done for the learner is done */
{
    int n; const uint8_t *e = c_field(CT_UNTIL, 0, &n);
    for (const uint8_t *end = e ? e + n : 0; e && e < end; e += 1 + e[0]) {
        const uint8_t *c = e + 1; int t = c[1] < NTRK ? c[1] : 0;
        switch (c[0]) {
        case CC_HAS: if (c[2] != 255) c_add(t, c[2], 100, 0, c_u16(c + 3)); else if (!is_drum(t)) c_add(t, last_notes[t], 100, 0, c_miss(c)); break;   /* (any note asked for, as on NOTES: the track's last one, in the steps with none) */
        case CC_PLAYING: transport(1); break;
        case CC_STOPPED: transport(0); break;
        case CC_TRACKIS: select_track(c[1]); break;
        case CC_PAGEIS: c_goto(c[1]); break;
        case CC_MUTED: seq_mute(&Q, t, c[2]); break;
        case CC_SOLOED: solo[t] = c[2]; break;
        case CC_NOMUTE: for (int k = 0; k < NTRK; k++) seq_mute(&Q, k, 0); break;
        case CC_NOSOLO: for (int k = 0; k < NTRK; k++) solo[k] = 0; break;
        default: break;
        }
    }
}
/* ---- a step from its start to its end */
CCOLD static void c_lesson_card(int list)           /* a lesson's first card, or the list of lessons with this one shown */
{
    const uint8_t *l = c_lesson(C.les); char b[72], *p;
    c_leds(); G.cap_keys = 0; G.cap_btn = 1u << BTN_OCT_UP | 1u << BTN_OCT_DOWN; G.cap_enc = list ? 1u << ENC_SELECT : 0;
    G.led[BTN_OCT_UP] = G.led[BTN_OCT_DOWN] = 5; G.only = 1;
    p = put_s(b, list ? "^< Turn [SELECT] >\n_Lesson " : "_Lesson "); p = put_u(p, l[0]); p = put_s(p, "\n");   /* (the heading has to fit one small line, 212 px: a part that wraps is drawn large) */
    for (const uint8_t *s = l + 2; *s && p < b + sizeof b - 1; s++) *p++ = (char)*s;
    *p = 0; c_say(CK_BIG, b, list ? "Home" : "Lessons", "Begin");
    C.ph = list ? PH_LIST : PH_BEGIN;
}
CCOLD static void course_stop(void) { if (C.kplay) c_key(C.kplay - 1, 0); C.ph = PH_OFF; C.kplay = 0; }
CCOLD static void course_start(void)                 /* START, and no computer answered: the lesson the learner was at */
{
    if (!course_find()) return;
    if (!tp_rec_read) { c_rec_read(); if (tp_rec_last) C.les = (uint8_t)(tp_rec_last - 1); }   /* (the first START since the pack was found: the lesson remembered) */
    if (C.les >= tpc[6]) C.les = 0;
    C.ms = 0; C.last = time_ticks(); C.ev_r = G.ev_w; C.gen = (uint8_t)msg_gen; G.home = 0; G.ask = 0;
    c_lesson_card(0);                                /* (its lights and buttons: the course's card has the same two answers) */
    const uint8_t *t = tpc + c_u16(tpc + 8 + 2 * (tpc[6] + tpc[7] + 2));   /* the card of the whole course: after the fonts in the pack, its length first */
    c_show(CK_CARD, t + 1, t[0], "Lessons", "Begin"); C.ph = PH_TOC;
}
CCOLD static void c_after(int from);
CCOLD static void c_to_step(void);
CCOLD static int c_held(void) { return Q.fill || punch_key >= 0 || mfx_key >= 0; }   /* a fill or an effect is on */
CCOLD static void c_step_begin(void)
{
    int n, any, need; uint32_t miss; int kind = C.st[2];
    C.hint = C.at = C.turned = C.said = C.turnedk = C.held = 0; C.pressed = C.twice = C.played = 0; C.lastb = 255; C.go = 255; C.t_off = 0;
    C.t0 = C.ms; C.t_hint = C.ms + 25000u; G.cap_keys = G.cap_btn = 0; G.cap_enc = 0; c_leds();
    for (int e = 0; e < 7; e++) C.clk[e] = 0;
    if (kind == CK_SETUP) { const uint8_t *e = c_field(CT_SETUP, 0, &n); if (e) c_setup(e, n); c_after(PH_WAIT); return; }
    if (c_field(CT_SILENT, 0, &n)) G.cap_keys = 0x7FFFFFFu;
    if (kind >= CK_CARD && kind <= CK_REPLY) { c_words(); if (!c_strip(kind)) G.only = 1; }
    {   /* what each condition's number is now ("changed" compares with it) */
        int i = 0; const uint8_t *e = c_field(CT_UNTIL, 0, &n);
        for (const uint8_t *end = e ? e + n : 0; e && e < end && i < 8; e += 1 + e[0], i++) C.latch[i] = e[1] == CC_NUM ? c_num(e + 1) : e[1] == CC_SOUNDCHG ? preset_of[e[2] < NTRK ? e[2] : 0] : 0;
    }
    int ok = c_until(&any, &miss, &need); C.need = (uint8_t)need; C.wrong = 0;
    if (any && ok && !c_field(CT_LEFT, 0, &n) && !c_field(CT_RIGHT, 0, &n)) {   /* it is so already: the step is over... */
        if (!c_held()) { c_after(PH_DO); return; }
        C.held = 1;                                  /* ...unless a fill or an effect is still held from the step before: let go first */
    }
    if (need != 255) { if (trk != C_DRUM) select_track(C_DRUM); c_pick(need); }   /* a drum's boxes: that drum the one picked */
    c_after(PH_STEP);
}
/* the step's next part, after the one named: a phrase played, played back, the move or the answer, the wait */
CCOLD static void c_after(int from)
{
    int n; int kind = C.st[2];
    if (from == PH_STEP && c_field(CT_PLAY, 0, &n)) { C.t0 = C.ms; C.ph = PH_PLAY; return; }
    if (from <= PH_PLAY && c_field(CT_ECHO, 0, &n)) { C.at = 0; C.ph = PH_ECHO; return; }
    if (from <= PH_ECHO) {
        int ln, rn; const uint8_t *l = c_field(CT_LEFT, 0, &ln), *r = c_field(CT_RIGHT, 0, &rn);
        if (l || r) { G.cap_btn = (l ? 1u << BTN_OCT_DOWN : 0) | (r ? 1u << BTN_OCT_UP : 0); C.ph = PH_ANSWER; return; }
        if (c_field(CT_DO, 0, &n) || c_field(CT_UNTIL, 0, &n)) { C.at = 0; C.ph = PH_DO; return; }
    }
    if (from <= PH_ANSWER) {
        const uint8_t *w = c_field(CT_WAIT, 0, &n);
        if (w || kind == CK_REPLY) {
            uint32_t v = w ? c_u16(w + 1) : 12, beat = (uint32_t)(60000.0f / (Q.bpm < 40.0f ? 40.0f : Q.bpm));
            C.bar = 0; C.t_end = C.ms + (!w ? 1200u : w[0] == CW_SECOND ? v * 100u : w[0] == CW_BAR ? v * 4u * beat / 10u : w[0] == CW_BEAT ? v * beat / 10u : 180000u);
            if (w && w[0] == CW_UNTIL) C.bar = (uint8_t)(v + 1);
            c_leds(); G.only = !c_strip(kind); C.ph = PH_WAIT; return;
        }
    }
    /* the step is over: the next one, or the lesson's end */
    if (C.go != 255) C.step = C.go; else C.step++;
    c_to_step();
}
CCOLD static void c_to_step(void)                    /* step number C.step under way, or past the last one: the lesson's end */
{
    G.cap_keys = G.cap_btn = 0; G.cap_enc = 0; c_leds();
    if (C.step >= C.nstep) {
        if (Q.playing) transport(0);
        char b[24], *p = put_s(b, "_Lesson "); p = put_u(p, c_lesson(C.les)[0]); *put_s(p, "\nDone") = 0;   /* (the number small, as on the list) */
        G.cap_btn = 1u << BTN_OCT_UP | 1u << BTN_OCT_DOWN; G.led[BTN_OCT_UP] = G.led[BTN_OCT_DOWN] = 5; G.only = 1;
        c_say(CK_BIG, b, "Replay", "Next"); C.ph = PH_END; return;
    }
    const uint8_t *s = c_lesson(C.les) + 2; while (*s) s++;
    s++; for (int i = 0; i < C.step; i++) s += 2 + c_u16(s);
    C.st = s; C.ph = PH_STEP;
}
CCOLD static void c_lesson_begin(void) { c_rec_ask(); C.nstep = c_lesson(C.les)[1]; C.step = 0; c_to_step(); }
CCOLD static void c_msg(const char *s, int kind, uint32_t ms, int ret, int reshow) { c_leds(); G.only = !c_strip(kind); c_say(kind, s, 0, 0); C.t_end = C.ms + ms; C.ret = (uint8_t)ret; C.reshow = (uint8_t)reshow; C.ph = PH_MSG; }
CCOLD static void c_answered(int right)             /* an answer given: what it does */
{
    int n, any, msg = 0; const uint8_t *e = c_field(right ? CT_RIGHT : CT_LEFT, 0, &n), *end = e + n;
    if (!c_until(&any, 0, 0)) {                      /* (too early: what is still needed, then the question again) */
        int hn; const uint8_t *h = c_field(CT_HINT, 0, &hn);
        c_leds(); G.only = !c_strip(C.st[2]);
        if (h) c_show(C.st[2], h, hn, 0, 0); else c_say(C.st[2], "Not yet.", 0, 0);
        C.t_end = C.ms + 6000u; C.ret = PH_ANSWER; C.reshow = 1; C.ph = PH_MSG; return;
    }
    while (*e) e++;
    for (e++; e < end; e += 1 + e[0]) {
        if (e[1] == CE_GOTO) C.go = e[2]; else if (e[1] == CE_RIGHT) msg = 1; else if (e[1] == CE_WRONG) msg = 2; else if (e[1] == CE_SETUP) c_setup(e + 2, e[0] - 1);
    }
    G.cap_btn = 0;
    if (msg) {
        int tn; const uint8_t *t = c_field(msg == 1 ? CT_RIGHTTXT : CT_WRONGTXT, 0, &tn);
        c_leds(); G.only = 1;
        if (t) c_show(tn < 24 ? CK_BIG : CK_CARD, t, tn, 0, 0); else c_say(CK_BIG, msg == 1 ? "Yes." : "Not quite.", 0, 0);
        C.t_end = C.ms + (msg == 1 ? 1400u : 3000u); C.ret = PH_BYE; C.reshow = 0; C.ph = PH_MSG; return;
    }
    c_after(PH_ANSWER);
}
CCOLD static void c_event(int kind, int id)          /* a press or a turn, as the guide reports it */
{
    int n;
    if (kind == 1) C.played++;
    if (kind == 3) { C.pressed |= (uint16_t)(1u << id); if (id == C.lastb && C.ms - C.t_lastb < 700u) C.twice |= (uint16_t)(1u << id); C.lastb = (uint8_t)id; C.t_lastb = C.ms; }
    if (kind == 1) C.lastb = 255;
    if (kind >= 5) C.turnedk |= (uint8_t)(1u << id);
    if (kind >= 5 && id < 7) C.clk[id] = (int8_t)(C.clk[id] + (kind == 5 ? 1 : -1));
    switch (C.ph) {
    case PH_BEGIN: case PH_LIST: case PH_END: case PH_STUCK: case PH_ANSWER: case PH_TOC:
        if (C.ph == PH_LIST && kind >= 5 && id == ENC_SELECT) { int k = C.les + (kind == 5 ? 1 : -1); if (k >= 0 && k < tpc[6]) { C.les = (uint8_t)k; c_lesson_card(1); } }
        if (kind != 3 || (id != BTN_OCT_UP && id != BTN_OCT_DOWN) || !((G.cap_btn >> id) & 1u)) break;
        if (C.ph == PH_ANSWER) { c_answered(id == BTN_OCT_UP); break; }
        if (C.ph == PH_TOC) { c_lesson_card(id != BTN_OCT_UP); break; }   /* BEGIN: the lesson's first card; LESSONS: the list */
        if (C.ph == PH_STUCK) {
            G.cap_btn = 0; c_leds(); c_words(); C.t_hint = C.ms + 25000u; C.hint = 0;
            if (id == BTN_OCT_UP) { c_fix(); c_after(PH_DO); } else C.ph = PH_DO;
            break;
        }
        if (id == BTN_OCT_UP) {
            if (C.ph == PH_END) {
                if (C.les + 1 >= tpc[6]) { c_msg("That was\nthe last\nlesson.", CK_BIG, 2500u, PH_OFF, 0); break; }
                C.les++; c_lesson_card(0);
            } else c_lesson_begin();
        } else if (C.ph == PH_BEGIN) c_lesson_card(1);
        else if (C.ph == PH_END) c_lesson_begin();
        else { course_stop(); guide_home(); }
        break;
    case PH_ECHO: { const uint8_t *k = c_field(CT_ECHO, 0, &n); if (kind == 1 && k && id == k[C.at] && ++C.at >= n) { c_leds(); c_after(PH_ECHO); } } break;
    case PH_DO: {
        const uint8_t *d = c_field(CT_DO, 0, &n), *op; int un;
        if (!d || c_field(CT_UNTIL, 0, &un) || 3 * C.at >= n) break;      /* (with an Until, the state decides) */
        op = d + 3 * C.at;
        int hit = ((op[0] == CD_PRESS || op[0] == CD_HOLD) && kind == 3 && id == op[1]) || ((op[0] == CD_TAP || op[0] == CD_LONG || op[0] == CD_KHOLD) && kind == 1 && id == op[1]) ||
                  (op[0] == CD_ANY && kind == 1) || (op[0] == CD_TURN && kind == ((int8_t)op[2] > 0 ? 5 : 6) && id == op[1]) || (op[0] == CD_LETGO && kind == 4);
        if (hit && op[0] == CD_TURN) { int want = (int8_t)op[2]; if (++C.turned >= (want < 0 ? -want : want)) { C.at++; C.turned = 0; } }
        else if (hit) C.at++;
    } break;
    default: break;
    }
}
/* (the frames answer the controls) each knob's framed name: how far the turn this step asks for is done, 0..255 of its
 * frame, and from which side it fills (a turn to the left: from the right). Empty when the step begins, full exactly
 * when the turn is done:
 *   a turn of a set number of clicks (the Do's): each click its share; one click: nothing to show;
 *   a turn to a value the knob sets (the tempo, the swing, a filter, a level or a send, a sound's value: CN_TEMPO to
 *   CN_VALUE): as the value goes from where it stood when the step began (C.latch) to where the step waits for it */
CCOLD static void c_fills(void)
{
    uint8_t f[7], dir = 0; int8_t want[7]; int n = 0, un, knobs = 0, one = 0, i = 0;
    const uint8_t *d = C.ph == PH_DO ? c_field(CT_DO, 0, &n) : 0, *u = c_field(CT_UNTIL, 0, &un);
    for (int e = 0; e < 7; e++) f[e] = 0, want[e] = 0;
    for (int k = 0; d && 3 * k < n; k++) {
        const uint8_t *op = d + 3 * k; int w = (int8_t)op[2];
        if (op[0] != CD_TURN || op[1] >= 7) continue;
        if (!want[op[1]]) { knobs++; one = op[1]; if (w < 0) dir |= (uint8_t)(1u << one); }
        want[op[1]] = (int8_t)(want[op[1]] + (w < 0 ? -w : w));
    }
    for (int e = 0; e < 7; e++) if (want[e] > 1) { int c = (dir >> e) & 1u ? -C.clk[e] : C.clk[e]; f[e] = (uint8_t)(c <= 0 ? 0 : c >= want[e] ? 255 : c * 255 / want[e]); }
    if (knobs == 1)
        for (const uint8_t *end = u ? u + un : 0; u && u < end && i < 8; u += 1 + u[0], i++) {
            if (u[1] != CC_NUM || u[2] > CN_VALUE || u[5] == CM_IS_NOT || u[5] == CM_CHANGED) continue;
            int32_t v0 = C.latch[i], k = (int16_t)c_u16(u + 6) + (u[5] == CM_ABOVE) - (u[5] == CM_BELOW), q = 255;
            if (!c_true(u + 1, i)) { q = k == v0 ? 0 : (c_num(u + 1) - v0) * 255 / (k - v0); q = q < 0 ? 0 : q > 255 ? 255 : q; }
            f[one] = (uint8_t)q; dir = (uint8_t)(k < v0 ? dir | 1u << one : dir & ~(1u << one));
            if (q < 255) break;                       /* (several: the first not yet reached says how far) */
        }
    for (int e = 0; e < 7; e++) if (G.kf[e] != f[e]) { G.kf[e] = f[e]; G.gen++; }
    if (G.kdir != dir) { G.kdir = dir; G.gen++; }
}
CCOLD static void course_tick(void)                  /* each round of the interface */
{
    if (C.ph == PH_OFF) {                            /* SLOOP by itself: both OCT buttons held two seconds bring back the start's card */
        if (!G.card && (panel_buttons() & 3u) == 3u) {
            if (!C.t_out) C.t_out = time_ticks() | 1u;
            else if ((uint32_t)(time_ticks() - C.t_out) > 2000000u * lsb) { C.t_out = 0; guide_home(); }
        } else C.t_out = 0;
        return;
    }
    {   uint32_t now = time_ticks(), d = (uint32_t)(now - C.last) / (1000u * lsb); C.ms += d; C.last += d * 1000u * lsb; }
    if ((uint8_t)(G.ev_w - C.ev_r) > 32) C.ev_r = (uint8_t)(G.ev_w - 32);
    while (C.ev_r != G.ev_w && C.ph != PH_OFF) { int kind = G.ev[C.ev_r & 31][0], id = G.ev[C.ev_r & 31][1]; C.ev_r++; c_event(kind, id); }
    if (C.ph != PH_OFF && C.st) c_fills();
    if (C.ph >= PH_STEP && C.ph <= PH_STUCK && C.st && (msg_live || info_live) && C.gen != (uint8_t)msg_gen) {   /* what SLOOP just said */
        int n, i = 0; const uint8_t *e = c_field(CT_UNTIL, 0, &n); const char *m = msg_live ? msg : info;
        for (const uint8_t *end = e ? e + n : 0; e && e < end; e += 1 + e[0], i++)
            if ((e[1] == CC_SAYS && c_in(m, e + 2)) || (e[1] == CC_SAYSONLY && c_same(m, e + 2))) C.said |= (uint8_t)(1u << i);
    }
    C.gen = (uint8_t)msg_gen;
    if (C.ph >= PH_STEP && C.ph <= PH_STUCK && (panel_buttons() & 3u) == 3u) {   /* both OCT buttons held: out of the lesson, to its first card */
        if (!C.t_out) C.t_out = C.ms | 1u;
        else if (C.ms - C.t_out > 1500u) { C.t_out = 0; if (C.kplay) c_key(C.kplay - 1, 0); C.kplay = 0; if (Q.playing) transport(0); G.cap_keys = 0; c_lesson_card(0); return; }
    } else C.t_out = 0;
    int n, kind = C.st ? C.st[2] : CK_CARD;
    switch (C.ph) {
    case PH_STEP: c_step_begin(); break;
    case PH_PLAY: {                                  /* the FM-1 plays a phrase, each key lit as it sounds */
        const uint8_t *k = c_field(CT_PLAY, 0, &n); uint32_t beat = k[0] ? (uint32_t)k[0] * 10u : 10u, e = C.ms - C.t0, i = e / beat;
        int on = i < (uint32_t)(n - 1) && e % beat < beat * 7u / 10u ? k[1 + i] + 1 : 0;
        if (on != C.kplay) { if (C.kplay) c_key(C.kplay - 1, 0); c_leds(); G.only = 1; if (on) { c_key(on - 1, 1); G.led[14 + on - 1] = 4; } C.kplay = (uint8_t)on; }
        if (i >= (uint32_t)(n - 1)) { c_leds(); c_after(PH_PLAY); }
    } break;
    case PH_ECHO: { const uint8_t *k = c_field(CT_ECHO, 0, &n); c_leds(); G.only = 1; if (k && C.at < n) G.led[14 + k[C.at]] = 5; } break;
    case PH_WAIT:
        if (C.ms >= C.t_end || (C.bar && (!Q.playing || (uint32_t)(Q.pos / ((uint64_t)SEQ_BAR << 16)) + 1u >= C.bar))) c_after(PH_WAIT);
        break;
    case PH_MSG:
        if (C.ms < C.t_end) break;
        if (C.ret == PH_OFF) { course_stop(); guide_home(); break; }
        if (C.ret == PH_BYE) { c_after(PH_ANSWER); break; }
        if (C.reshow) c_words();
        if (C.ret == PH_ANSWER) { int ln, rn; G.cap_btn = (c_field(CT_LEFT, 0, &ln) ? 1u << BTN_OCT_DOWN : 0) | (c_field(CT_RIGHT, 0, &rn) ? 1u << BTN_OCT_UP : 0); }
        C.t_hint = C.ms + 25000u; C.ph = C.ret;
        break;
    case PH_ANSWER: case PH_DO: {
        int any, need, dn, fn; uint32_t miss; const uint8_t *d = c_field(CT_DO, 0, &dn), *f = c_field(CT_FOLLOW, 0, &fn);
        int ok = c_until(&any, &miss, &need), strip = c_strip(kind);
        /* (SLOOP 3.4) NOTES, and a step still lacks its note: the key waits until [KNOB1] has brought STEP to such a step */
        int wait = miss >> 16 && sticky == PG_NOTES && !is_drum(trk) && !(ne_step < 16 && (miss >> (16 + ne_step)) & 1u);
        miss &= 0xFFFFu;
        c_leds(); G.only = !strip;
        if (C.ph == PH_ANSWER) { for (int b = 0; b < 2; b++) if ((G.cap_btn >> b) & 1u) G.led[b] = 5; }
        else {
            if (C.held) C.held = (uint8_t)c_held();
            else if (any ? ok : 3 * C.at >= dn) { c_after(PH_DO); break; }
            /* the lights: what to hold, what is still to press */
            for (int i = 0; 3 * i < dn; i++) {
                const uint8_t *op = d + 3 * i; int key = op[0] == CD_TAP || op[0] == CD_LONG || op[0] == CD_KHOLD || (op[0] == CD_ANY && op[1] != 255);
                if (any) { if (op[0] == CD_PRESS || op[0] == CD_HOLD) G.led[op[1]] = 5; else if (key && C.need == 255 && !miss && !wait) G.led[14 + op[1]] = 5; }
                else if (i < C.at) { if (op[0] == CD_HOLD) G.led[op[1]] = 5; else if (op[0] == CD_KHOLD) G.led[14 + op[1]] = 5; }
                else if (i == C.at) { if (op[0] == CD_PRESS || op[0] == CD_HOLD) G.led[op[1]] = 5; else if (key) G.led[14 + op[1]] = 5; }
            }
            for (int b = 0; b < 16; b++) if ((miss >> b) & 1u) G.led[14 + white_key[b]] = 5;
            if (C.need != 255 && (last_notes[C_DRUM] & 15) != C.need) {   /* (SLOOP 3.4) another drum picked than the boxes ask for: */
                int key = 14 + drum_black_key(C.need), w = (last_notes[C_DRUM] & 15) + 1;
                for (int k = 14; k < 41; k++) G.led[k] = 0;                /* its black key blinks, SLOOP's way to pick it on the grid, */
                G.led[key] = 5;                                            /* not the boxes, until it is picked; said once for each wrong drum */
                if (C.wrong != w) { C.wrong = (uint8_t)w; c_msg("Another drum is picked.\nTap the blinking key.", kind, 2400u, PH_DO, 1); G.led[key] = 5; break; }
            } else C.wrong = 0;
            while (!any && 3 * C.at < dn && d[3 * C.at] == CD_HOLD && (held_btn >> d[3 * C.at + 1]) & 1u) C.at++;
        }
        if (f && fn) G.led[14 + f[Q.playing ? (uint32_t)(Q.pos / ((uint64_t)SEQ_BAR << 16)) % (uint32_t)fn : 0]] = 5;
        { int ln; const uint8_t *lt = c_field(CT_LIGHTS, 0, &ln); for (int i = 0; lt && i < ln; i++) if (!G.led[14 + lt[i]]) G.led[14 + lt[i]] = 2; }   /* Lights: keys lit faintly, part of the picture (not blinking: that means press) */
        {   /* wandered to another page: brought back */
            int pn; const uint8_t *pg = c_field(CT_PAGE, 0, &pn);
            if (pg && sticky != pg[0] && !(held_btn & LAYER_BITS)) { if (!C.t_off) C.t_off = C.ms | 1u; else if (C.ms - C.t_off > 2500u) { c_goto(pg[0]); C.t_off = 0; } }
            else C.t_off = 0;
        }
        if (C.ph == PH_DO && C.ms >= C.t_hint) {     /* it takes long: a hint; after the last one, the offer to do it */
            int hn; const uint8_t *h = c_field(CT_HINT, C.hint, &hn);
            if (h) { c_show(kind, h, hn, 0, 0); C.hint++; C.t_hint = C.ms + 25000u; }
            else if (d && C.ms >= C.t_hint + 20000u) {
                c_leds(); G.cap_btn = 1u << BTN_OCT_UP | 1u << BTN_OCT_DOWN; G.led[BTN_OCT_UP] = G.led[BTN_OCT_DOWN] = 5; G.only = 1;
                c_say(CK_CARD, "Stuck?\nI can do this step for you.", "No", "Show me"); C.ph = PH_STUCK;
            }
        }
    } break;
    default: break;
    }
}
CCOLD static void course_state(uint8_t *o) { o[0] = C.ph; o[1] = C.ph >= PH_STEP && tpc ? c_lesson(C.les)[0] : 0; o[2] = C.step; o[3] = tpc ? tpc[6] : 0; }
