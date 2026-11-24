/* SPDX-License-Identifier: GPL-3.0-only
 * SLOOP 2.8's projects into SLOOP 3's (sloopimport.h). Written for this project.
 * The facts used: an object's two copies (4 KB sectors; projects at 0x97000 + (obj - 1) x 8 KB, the working
 * project at 0x9F000 and 0xFE000), each a 32-byte header ("FELU", its object, its copy, a sequence, the payload's
 * length and zlib CRC, the header's CRC) and the payload at +256; a project of format 6 ("FUN6", 3816 bytes, an
 * FNV-1a hash last): 32 globals (BPM, swing ..), then 4 tracks of 934 bytes (53 parameters in 16 bits, 8 engine
 * values, 10 bytes, the engine and its preset, 64 steps of 10 bytes, a byte a step: its nudge and fill condition,
 * 24 locks); format 5 ("FUN5", 3840 bytes): tracks of 940 bytes (61 parameters, the engine and its preset, the
 * steps, a nudge a step, 24 locks, the fill conditions 2 bits a step); a synth step: 4 notes, how many, NOTE / TIE / REST, flags, velocity, 2-bit levels and ratchets; a
 * drum step: 16 lanes on / off, their levels and ratchets. The drum lanes are SLOOP 3's 16 pads, in order. */
#include "sloopimport.h"
#include "../core/gb.h"          /* memcmp */
#include "presets.h"
#include "store.h"
#include "arp.h"
#include "../engine/drum.h"

static uint32_t rd32(const uint8_t *b) { return b[0] | b[1] << 8 | b[2] << 16 | (uint32_t)b[3] << 24; }
static int16_t rd16(const uint8_t *b) { return (int16_t)(b[0] | b[1] << 8); }
static const int8_t SL_SP[49] = {
    [1] = SP_ATK, [2] = SP_DEC, [3] = SP_SUS, [4] = SP_REL, [5] = SP_EFLT, [6] = SP_EPIT, [7] = SP_ESHP, [8] = SP_TRIM,
    [9] = SP_LRATE, [10] = SP_LWAVE, [11] = SP_LPHASE, [12] = SP_LFADE, [13] = SP_LPIT, [14] = SP_LFLT, [15] = SP_LSHP,
    [16] = SP_LAMP, [33] = SP_DIST, [37] = SP_VOICE, [38] = SP_GLIDE, [41] = SP_GLMODE, [42] = SP_PRIO, [43] = SP_ALLOC,
    [44] = SP_DETUNE, [45] = SP_SLCR, [46] = SP_SLPAT, [47] = SP_SLRATE, [48] = SP_SLDEPTH };
static const uint8_t SL_26[10] = { SP_ITYPE, SP_IA, SP_IB, SP_IC, SP_IMIX, SP_FTYPE, SP_ESYNC, SP_LSYNC, SP_LTRIG, SP_LPOL };
/* SLOOP's objects: 0 its settings (0xFC000, 0xFD000), 1..4 its sections, 5 and 6 its user presets' banks (0xDC000..),
 * 7 its working project (0x9F000, 0xFE000), 8 its FM6 bank (0xE5000, 0xE6000) */
static uint32_t sector_of(int obj, int copy)
{
    uint32_t c = (uint32_t)copy * 4096u;
    return obj == 0 ? 0xFC000u + c : obj == 7 ? (copy ? 0xFE000u : 0x9F000u) : obj == 8 ? 0xE5000u + c :
           obj >= 5 ? 0xDC000u + (uint32_t)(obj - 5) * 8192u + c : 0x97000u + (uint32_t)(obj - 1) * 8192u + c;
}
uint32_t sloop_obj_at;
int sloop_obj(sloop_rd rd, int obj, uint8_t *buf, uint32_t cap)
{
    uint8_t h[2][32]; int ok[2];
    for (int c = 0; c < 2; c++)
        ok[c] = !rd(sector_of(obj, c), h[c], 32) && rd32(h[c]) == 0x554C4546u && (h[c][4] | h[c][5] << 8) == obj && (h[c][6] | h[c][7] << 8) == c &&
                rd32(h[c] + 12) <= 3840u && rd32(h[c] + 12) <= cap && rd32(h[c] + 28) == crc32_ieee(0, h[c], 28);
    for (int k = 0; k < 2; k++) {                             /* the newer valid copy first */
        int c = ok[1] && (!ok[0] || rd32(h[1] + 8) > rd32(h[0] + 8)) ? (k ? 0 : 1) : (k ? 1 : 0);
        uint32_t n = ok[c] ? rd32(h[c] + 12) : 0u;
        if (ok[c] && n && !rd(sector_of(obj, c) + 256u, buf, n) && crc32_ieee(0, buf, n) == rd32(h[c] + 16)) { sloop_obj_at = sector_of(obj, c) + 256u; return (int)n; }
    }
    return 0;
}
int sloop_settings(sloop_rd rd, uint8_t *buf, sloop_set *o)
{
    int n = sloop_obj(rd, 0, buf, 3840u);
    uint32_t m = n >= 52 ? rd32(buf) : 0u;
    if (m != 0x50455233u && m != 0x50455232u) return 0;      /* "PER3" (with the song order), "PER2" */
    uint32_t lo = m == 0x50455233u ? 84u : 48u;
    uint32_t w = (uint32_t)n >= lo + 4u ? rd32(buf + lo) : 0u;
    o->lowcut = (uint8_t)(rd32(buf + 8) == 1u);
    o->lights = (uint8_t)((w & 15u) < 4u ? w & 15u : 0u); o->keys = (uint8_t)(((w >> 4) & 15u) < 4u ? (w >> 4) & 15u : 0u);
    o->notes = (uint8_t)((w >> 8) & 1u); o->usb_full = (uint8_t)((w >> 11) & 1u); o->core2 = (uint8_t)!((w >> 26) & 1u);
    o->accel = (uint8_t)!((w >> 27) & 1u); o->usb_rate = (uint8_t)(((w >> 28) & 3u) % 3u);
    o->palette = (uint8_t)(rd32(buf + 4) % 5u); o->theme_on = (uint8_t)((w >> 30) & 1u);   /* MENU > COLOR: its 5 palettes, or its theme */
    /* the calibration ("PAN5"): its buttons by their labels (FX SEL ENV LFO EDIT GLO HOME SAVE ARP SEQ PLAY REC OCT-
     * OCT+), its encoders by their roles (SELECT ALGORITHM PRESETS KNOB 1..4), as ours */
    static const uint8_t ID[14] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 0, 1 }, EID[7] = { 0, 1, 6, 2, 3, 4, 5 };
    const uint8_t *pn = buf + 16;
    o->cal = rd32(pn) == 0x50414E35u;
    for (int i = 0; o->cal && i < 14; i++) o->btn[ID[i]] = pn[4 + i];
    for (int i = 0; o->cal && i < 7; i++) { o->enc[EID[i]] = pn[18 + i]; o->dir[EID[i]] = (int8_t)pn[25 + i]; }
    return 1;
}

/* SYN1..SYN4 (SLOOP 2.5): in its settings' payload after them, "DSU1", version 1, 4 kits of a name (8), the crush,
 * the factory kit it began from, 2 spare, 16 sounds of 22 bytes in its synthesised kits' lane order (wave, source,
 * pitch, its 1/16, bend, its time, hold, decay, tone level, 2nd partial's ratio and level, click, noise level, hold
 * and decay, filter, cutoff, envelope, high-pass, chip clock, drive, level + 128 in quarter dB) */
int sloop_synkits(const uint8_t *pl, int n, gb_dsnd out[4][16], uint8_t crush[4])
{
    static const uint8_t LANE[16] = { 0, 14, 1, 2, 3, 4, 3, 11, 15, 5, 6, 7, 8, 9, 10, 12 };   /* our keys: its lanes */
    uint32_t m = n >= 52 ? rd32(pl) : 0u, at = m == 0x50455233u ? 88u : 52u;
    if ((m != 0x50455233u && m != 0x50455232u) || (uint32_t)n < at + 1464u) return 0;
    const uint8_t *b = pl + at;
    if (rd32(b) != 0x31555344u || (b[4] | b[5] << 8) != 1 || (b[6] | b[7] << 8) != 4) return 0;
    for (int k = 0; k < 4; k++) {
        const uint8_t *kt = b + 8 + k * 364;
        crush[k] = kt[8];
        for (int l = 0; l < 16; l++) {
            const uint8_t *d = kt + 12 + LANE[l] * 22;
            gb_dsnd *o = &out[k][l];
            o->wave = d[0] <= 5 ? d[0] : 1;
            o->src = (d[1] & 15u) <= 4 && !(d[1] & ~0x1Fu) ? d[1] : 1;
            o->note16 = (uint16_t)((d[2] & 127) * 16 + (d[3] & 15) - (l == 6 ? 16 : 0));   /* (the pedal: a semitone down) */
            o->bend = d[4] > 96 ? 96 : d[4]; o->bt = d[5] & 127; o->hold = d[6]; o->decay = d[7] & 127;
            o->tlev = d[8] > 127 ? 127 : d[8]; o->t2 = d[9]; o->t2lev = d[10] > 127 ? 127 : d[10];
            o->click = d[11] > 127 ? 127 : d[11]; o->nlev = d[12] > 127 ? 127 : d[12]; o->nhold = d[13]; o->ndec = d[14] & 127;
            o->flt = d[15]; o->fcut = d[16] & 127; o->fenv = (int8_t)d[17]; o->hpf = d[18] & 127; o->chip = d[19] & 127;
            o->drive = d[20] > 127 ? 127 : d[20]; o->qdb = (int8_t)(d[21] - 128);
        }
    }
    return 1;
}

/* the settings' third part (SLOOP 2.6, after the SYN kits): MIDI LEARN's 16 entries (cc | track << 7 | code << 9, code a
 * parameter's id + 1, or 120 + k for its engine's k-th), then its MIDI settings (channels, BEND, FX CH, DR MAP and CLK
 * OUT, FILTER RESO, REMOTE). The map in ours (a value's code: usersnd.h ... fw/main.c ML_*) */
int sloop_midi(const uint8_t *pl, int n, uint16_t ml[16], sloop_mset *o)
{
    uint32_t m = n >= 52 ? rd32(pl) : 0u, at = (m == 0x50455233u ? 88u : 52u) + 1464u;
    if ((m != 0x50455233u && m != 0x50455232u) || (uint32_t)n < at + 32u) return 0;
    const uint8_t *b = pl + at;
    int got = 0;
    for (int i = 0; i < 16; i++) {
        uint32_t e = (uint32_t)b[2 * i] | (uint32_t)b[2 * i + 1] << 8, cc = e & 127u, t = (e >> 7) & 3u, code = e >> 9, c = 0;
        ml[i] = 0;
        if (!code || !cc) continue;
        if (code >= 120u) c = code - 120u < 8u ? SP_E0 + (code - 120u) + 1u : 0u;
        else {
            uint32_t id = code - 1u;
            c = id == 0 ? 49u : id == 39 ? 50u : id == 35 ? 51u : id == 36 ? 52u : id == 34 ? 53u :
                id < 49 && SL_SP[id] && id != 8 ? (uint32_t)SL_SP[id] + 1u : id >= 53 && id <= 62 ? (uint32_t)SL_26[id - 53] + 1u : 0u;
        }
        if (c) { ml[i] = (uint16_t)(cc | t << 7 | c << 10); got++; }
    }
    if ((uint32_t)n >= at + 40u) {
        o->dprev = (uint8_t)((b[35] >> 5) & 1u); o->bend = (uint8_t)((b[36] & 15u) >= 1 && (b[36] & 15u) <= 12 ? b[36] & 15u : 2u);
        o->fxch = (uint8_t)(b[37] <= 16 ? b[37] : 0u); o->dmap = (uint8_t)(b[38] & 1u); o->clk = (uint8_t)((b[38] >> 1) & 1u);
        o->reso = (uint8_t)((b[38] >> 2) & 15u); if (o->reso > 10) o->reso = 10;
        o->rmch = (uint8_t)(b[39] <= 16 ? b[39] : 0u); o->have = 1;
    } else o->have = 0;
    return got;
}

int sloop_read(sloop_rd rd, int obj, uint8_t *buf)
{
    uint8_t h[2][32]; int ok[2];
    for (int c = 0; c < 2; c++) {
        ok[c] = !rd(sector_of(obj, c), h[c], 32) && rd32(h[c]) == 0x554C4546u && (h[c][4] | h[c][5] << 8) == obj && (h[c][6] | h[c][7] << 8) == c &&
                (rd32(h[c] + 12) == 3816u || rd32(h[c] + 12) == 3840u) && rd32(h[c] + 28) == crc32_ieee(0, h[c], 28);
    }
    for (int k = 0; k < 2; k++) {                             /* the newer valid copy first */
        int c = ok[1] && (!ok[0] || rd32(h[1] + 8) > rd32(h[0] + 8)) ? (k ? 0 : 1) : (k ? 1 : 0);
        uint32_t n = ok[c] ? rd32(h[c] + 12) : 0u;
        if (!ok[c] || rd(sector_of(obj, c) + 256u, buf, n) || crc32_ieee(0, buf, n) != rd32(h[c] + 16)) continue;
        uint32_t s = 0x811C9DC5u; for (uint32_t i = 0; i < n - 4u; i++) s = (s ^ buf[i]) * 16777619u;
        if (rd32(buf) == (n == 3816u ? 0x46554E36u : 0x46554E35u) && rd32(buf + 4) == n && rd32(buf + n - 4u) == s) return 1;
    }
    return 0;
}

/* a track of a SLOOP project; parameters by SLOOP's ids */
enum { SL_LEVEL = 0, SL_TRANS = 28, SL_SLEN = 29, SL_SDIV = 30, SL_SSWING = 31, SL_SGATE = 32, SL_CHOR = 34, SL_DLY = 35, SL_REV = 36, SL_PAN = 39, SL_MUTE = 40 };
static int v5(const uint8_t *p) { return rd32(p) == 0x46554E35u; }
static const uint8_t *trk(const uint8_t *p, int t) { return p + 76 + t * (v5(p) ? 940 : 934); }
/* where a track's parts are: format 6 / format 5 */
static int eng_off(const uint8_t *p) { return v5(p) ? 122 : 132; }
static int step_off(const uint8_t *p) { return v5(p) ? 124 : 134; }
static int par(const uint8_t *tk, int id) { return rd16(tk + 2 * id); }
static int clampi(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

static void sort_pat(seq_pat *p) { for (int a = 1; a < p->n; a++) { seq_ev e = p->ev[a]; int b = a; while (b > 0 && p->ev[b - 1].tick > e.tick) { p->ev[b] = p->ev[b - 1]; b--; } p->ev[b] = e; } }
static int vel_of(int lv, int v) { return lv == 1 ? 40 : lv == 2 ? 70 : lv == 3 ? 127 : (v > 0 ? clampi(v, 1, 127) : 100); }
/* a section's track t as pattern pat */
static void pattern_of(const uint8_t *pr, int t, seq_pat *p)
{
    const uint8_t *tk = trk(pr, t), *st = tk + step_off(pr), *mf = tk + (v5(pr) ? 764 : 774), *fl5 = tk + 924;
    int old = v5(pr);
    int len = clampi(par(tk, SL_SLEN), 1, 64);
    p->n = 0; p->nlock = 0; p->steps = (uint16_t)len;
    for (int s = 0; s < len; s++) {
        const uint8_t *x = st + 10 * s;
        int micro = old ? (int8_t)mf[s] : (mf[s] & 63) >= 32 ? (mf[s] & 63) - 64 : (mf[s] & 63);
        int fc = old ? (fl5[s >> 2] >> (2 * (s & 3))) & 3 : mf[s] >> 6, cond = fc == 1 ? SEQ_FILL : fc == 2 ? SEQ_NOFILL : SEQ_ALWAYS;
        micro = micro < -32 ? -32 : micro > 31 ? 31 : micro;
        int32_t tick = s * SEQ_TPS + micro * SEQ_TPS / 64; if (tick < 0) tick += len * SEQ_TPS;
        if (t == 3) {                                        /* the drums: 16 lanes */
            for (int l = 0; l < 16 && p->n < SEQ_PATEV; l++) {
                if (!((x[l >> 3] >> (l & 7)) & 1)) continue;
                int lv = (x[2 + l / 4] >> (2 * (l % 4))) & 3, ra = ((x[6 + l / 4] >> (2 * (l % 4))) & 3) + 1;
                seq_ev e = { (uint16_t)tick, SEQ_TPS * 3 / 4, (uint8_t)l, (uint8_t)vel_of(lv, 100), (uint8_t)cond, (uint8_t)ra };
                p->ev[p->n++] = e;
            }
            continue;
        }
        if (x[5] != 0 || x[4] == 0) continue;                /* a NOTE step with notes (a TIE carries one on) */
        int j = s + 1; while (j < len && st[10 * j + 5] == 1) j++;
        for (int k = 0; k < x[4] && k < 4 && p->n < SEQ_PATEV; k++) {
            int lv = (x[8] >> (2 * k)) & 3, ra = ((x[9] >> (2 * k)) & 3) + 1;
            int fl = ((x[6] & 1) ? SEQ_ACCENT : 0) | ((x[6] & 2) ? SEQ_SLIDE : 0);   /* (SLOOP's accent, slide) */
            seq_ev e = { (uint16_t)tick, (uint16_t)((j - s) * SEQ_TPS), (uint8_t)(x[k] & 127), (uint8_t)vel_of(lv, x[7]), (uint8_t)cond, (uint8_t)(ra | fl) };
            p->ev[p->n++] = e;
        }
    }
    sort_pat(p);
}
/* the sound, exactly: SLOOP's values of the parameters that make it (SLOOP 3's model is SLOOP's, engine.h SP_*):
 * by SLOOP's ids 1..16 (the envelope, ENV >, the LFO), 33 DIST, 37 VOICE, 38 GLIDE, 41..48 (GLMODE PRIO ALLOC DETUNE,
 * the SLICER); its 8 engine values; format 6: the 10 bytes of 2.6 (INSERT, FTYPE, ESYNC, LFO SYNC TRIG POL) */
static void sound_from(const uint8_t *src, const uint8_t *tk, gb_patch *p)
{
    for (int id = 1; id < 49; id++) if (SL_SP[id] && id != 8) gb_patch_set(p, SL_SP[id], par(tk, id));   /* (TRIM: SLOOP's own levels) */
    for (int k = 0; k < 8; k++) gb_patch_set(p, SP_E0 + k, v5(src) ? par(tk, 53 + k) : rd16(tk + 106 + 2 * k));
    if (!v5(src)) for (int k = 0; k < 10; k++) gb_patch_set(p, SL_26[k], tk[122 + k]);
}
/* a user preset of SLOOP's (a 192-byte record: used 0xA5, version 1, its engine, how many values it keeps, a name of
 * 12, its values in 16 bits by SLOOP's ids (or, kept by an older SLOOP with fewer: the first ones from LEVEL on, the
 * last 8 its engine's), a 16-step pattern): its engine, its sound, its sends, its arpeggiator, its transpose */
int sloop_upreset(const uint8_t *r, gb_patch *p, char name[13], uint8_t fx[4], uint8_t arp[4], int *trans)
{
    int e = r[2], np = r[3];
    if (r[0] != 0xA5 || r[1] != 1 || e >= 12 || np < 8 || np > 72 || !r[4] || gb_sloop_eng[e] == 0xFF) return 0;
    int16_t v[71];
    for (int i = 0; i < 71; i++) v[i] = INT16_MIN;
    if (np == 71) for (int i = 0; i < 71; i++) v[i] = rd16(r + 16 + 2 * i);
    else {
        for (int i = 0; i < np - 8 && i < 63; i++) v[i] = rd16(r + 16 + 2 * i);
        for (int k = 0; k < 8; k++) v[63 + k] = rd16(r + 16 + 2 * (np - 8 + k));
    }
    gb_patch_default(p, gb_sloop_eng[e]);
    for (int id = 1; id < 49; id++) if (SL_SP[id] && id != 8 && v[id] != INT16_MIN) gb_patch_set(p, SL_SP[id], v[id]);
    for (int k = 0; k < 10; k++) if (v[53 + k] != INT16_MIN) gb_patch_set(p, SL_26[k], v[53 + k]);
    for (int k = 0; k < 8; k++) if (v[63 + k] != INT16_MIN) gb_patch_set(p, SP_E0 + k, v[63 + k]);
    int i = 0;
    for (; i < 12 && r[4 + i] >= 32 && r[4 + i] < 127; i++) name[i] = (char)r[4 + i];
    name[i] = 0;
    fx[0] = (uint8_t)clampi(v[33] == INT16_MIN ? 0 : v[33], 0, 127); fx[1] = (uint8_t)clampi(v[34] == INT16_MIN ? 0 : v[34], 0, 127);
    fx[2] = (uint8_t)clampi(v[35] == INT16_MIN ? 0 : v[35], 0, 127); fx[3] = (uint8_t)clampi(v[36] == INT16_MIN ? 0 : v[36], 0, 127);
    int am = v[17] == INT16_MIN ? 0 : v[17];
    for (int k = 0; k < 4; k++) arp[k] = am > 0 && v[17 + k] != INT16_MIN ? (uint8_t)clampi(v[17 + k] + 1, 0, 255) : 0;
    *trans = v[28] == INT16_MIN ? 0 : clampi(v[28], -24, 24);
    return 1;
}
/* a sound SLOOP 3 has no engine for yet: the nearest of the same kind */
static int sound_of(int e, int pi)
{
    int kind = e >= 0 && e < 12 && pi < sloop_nkind[e] ? sloop_kind[e][pi] : K_PAD;
    for (int pass = 0; pass < 2; pass++)
        for (int i = 0; i < gb_npresets; i++) {
            const gb_preset *x = &gb_presets[i];
            if (x->engine == ENG_DRUM) continue;
            if (pass == 1 || x->kind == kind) return i;
        }
    return 0;
}
/* SLOOP 2.7's favourites: the side record of its settings' copy (64 bytes into the copy's sector: a theme of 40, "FAV1",
 * 16 entries, a CRC of them), each entry a place in its PRESETS list (sloop_bank) or 0x8000 | a user preset's slot */
int sloop_favs(sloop_rd rd, uint8_t *scratch, uint16_t out[16])
{
    int n = 0;
    for (int i = 0; i < 16; i++) out[i] = 0xFFFF;
    if (sloop_obj(rd, 0, scratch, 3840u) <= 0) return 0;
    uint8_t r[80];
    if (rd(sloop_obj_at - 256u + 64u, r, 80) || rd32(r + 40) != 0x31564146u || rd32(r + 76) != crc32_ieee(0, r + 40, 36)) return 0;
    for (int i = 0; i < 16; i++) {
        uint32_t f = (uint32_t)(r[44 + 2 * i] | r[45 + 2 * i] << 8), our = 0xFFFF;
        if (f == 0xFFFFu) continue;
        if (f & 0x8000u) { if ((f & 0xFFu) < 32u) our = 1000u + (f & 0xFFu); }
        else if (f < sloop_nbank) { uint32_t b = sloop_bank[f]; int pi = (b & 0xFFu) == 0xFFu ? -1 : preset_of_sloop((int)(b >> 8), (int)(b & 0xFFu)); if (pi >= 0) our = (uint32_t)pi; }
        if (our != 0xFFFF) out[n++] = (uint16_t)our;
    }
    return n;
}
int sloop_theme(sloop_rd rd, uint8_t *scratch, uint8_t out[40])
{
    if (sloop_obj(rd, 0, scratch, 3840u) <= 0) return 0;
    uint8_t r[40];
    if (rd(sloop_obj_at - 256u + 64u, r, 40) || rd32(r) != 0x324D4854u || rd32(r + 36) != crc32_ieee(0, r, 36)) return 0;
    for (int i = 0; i < 40; i++) out[i] = r[i];
    return 1;
}
/* SLOOP's arpeggiator in ours: its MODE (its order) and RATE (1/4 1/8 1/16 1/32 8T 16T) */
const uint8_t sloop_amode[15] = { AM_OFF, AM_UP, AM_DN, AM_UPDN, AM_RND, AM_ORD, AM_REPEAT, AM_DNUP, AM_UP8, AM_CONV,
                                  AM_DIVG, AM_PINKY, AM_THUMB, AM_WALK, AM_CHORD };
const uint8_t sloop_arate[6] = { 0, 1, 3, 5, 2, 4 };
/* where a lock's parameter is in ours (SP_*), -1: none of the sound (a mix value: not kept) */
static int lock_sp(int id)
{
    if (id > 0 && id < 49 && SL_SP[id] && id != 8) return SL_SP[id];
    if (id >= 53 && id <= 62) return SL_26[id - 53];
    if (id >= 63 && id <= 70) return SP_E0 + (id - 63);
    return -1;
}
/* the locks of track t of a section, onto its pattern: each SLOOP value made ours (0..255 over our range, on the
 * section's own engine: an engine's values have its ranges) */
static void locks_of(const uint8_t *pr, int t, seq_pat *p)
{
    const uint8_t *tk = trk(pr, t), *lk = tk + (v5(pr) ? 828 : 838);
    int e = tk[eng_off(pr)], have = t != 3 && e < 12 && gb_sloop_eng[e] != 0xFF;
    gb_patch tmp; gb_patch_default(&tmp, have ? gb_sloop_eng[e] : ENG_VA);
    for (int i = 0; i < 24 && p->nlock < SEQ_LOCKS; i++) {
        const uint8_t *l = lk + 4 * i;
        int step = l[0], id = l[1], val = rd16(l + 2), sp = -1, v = 0;
        if (step >= p->steps || step == 0xFF) continue;
        /* the mix (the drum track's too): LEVEL, PAN, the sends, the track's filter (5.. its P_TFLT, 50) */
        int mx = id == 0 ? SEQ_LK_LEVEL : id == 39 ? SEQ_LK_PAN : id == 34 ? SEQ_LK_CHO : id == 35 ? SEQ_LK_DLY : id == 36 ? SEQ_LK_REV : id == 50 ? SEQ_LK_FILT : -1;
        if (mx >= 0) { sp = mx; v = mx == SEQ_LK_PAN || mx == SEQ_LK_FILT ? (clampi(val, -64, 63) + 64) * 255 / 127 : clampi(val, 0, 127) * 255 / 127; }
        else {
            if (t == 3) continue;                        /* (the drum track's own: the kit's, not kept) */
            sp = lock_sp(id);
            if (sp < 0 || (sp >= SP_E0 && sp < SP_E0 + 8 && !have)) continue;
            float keep = tmp.p[sp];
            gb_patch_set(&tmp, sp, val);
            v = (int)(tmp.p[sp] * 255.0f + 0.5f);
            tmp.p[sp] = keep;
        }
        int j = 0; while (j < p->nlock && !(p->lk[j].step == step && p->lk[j].param == sp)) j++;
        if (j == p->nlock) p->nlock++;
        p->lk[j].step = (uint8_t)step; p->lk[j].param = (uint8_t)sp; p->lk[j].value = (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v);
    }
}
/* the drum track's per-sound TUNE and DECAY (SLOOP 2.7: 7 bits a sound in its free engine values E1..E7, checked
 * by a mark in g[G_SAVE]); none: as the kit has them */
static void drum_tw(const uint8_t *pr, int8_t tune[16], int8_t dec[16])
{
    const uint8_t *e = trk(pr, 3) + 106;                 /* (format 6: E0..E7 after the 53 parameters) */
    uint32_t sum = 0x5Au;
    for (uint32_t i = 1; i < 8u; i++) { uint32_t w = (uint16_t)rd16(e + 2 * i); sum += (w & 0xFFu) * (i * 2u + 1u) + (w >> 8) * (i * 2u + 2u); }
    int ok = !v5(pr) && (uint16_t)rd16(pr + 8 + 2 * 19) == (0x5400u | (sum & 0xFFu));
    for (uint32_t l = 0; l < 16u; l++) {
        uint32_t v = 0;
        for (uint32_t b = 0; ok && b < 7u; b++) if (((uint16_t)rd16(e + 2 * (1u + (l * 7u + b) / 16u)) >> ((l * 7u + b) % 16u)) & 1u) v |= 1u << b;
        tune[l] = (int8_t)((int)((v & 15u) ^ 8u) - 8); dec[l] = (int8_t)((int)(((v >> 4) & 7u) ^ 4u) - 4);
    }
}
static int same_name(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return !*a && !*b; }
int sloop_sound(const uint8_t *src, int t, gb_sound *s)
{
    const uint8_t *tk = trk(src, t);
    gb_track *k = &s->trk[t];
    if (t == 3) {                                        /* the drums: the kit, each sound's TUNE and DECAY */
        int kit = clampi(v5(src) ? par(tk, 53) : rd16(tk + 106), 0, GB_NKITS - 1), pi = -1;
        for (int i = 0; i < gb_npresets && pi < 0; i++) if (gb_presets[i].engine == ENG_DRUM && same_name(gb_presets[i].name, gb_dkit_names[kit])) pi = i;
        if (pi >= 0) preset_apply(s, t, pi);
        gb_patch_set(&k->patch, SP_E0, kit);
        drum_tw(src, k->patch.dtune, k->patch.ddec);
        return pi;
    }
    int e = tk[eng_off(src)], k0 = tk[eng_off(src) + 1], pi = preset_of_sloop(e, k0);
    if (e < 12 && gb_sloop_eng[e] != 0xFF) {
        if (pi >= 0) preset_apply(s, t, pi); else gb_patch_default(&k->patch, gb_sloop_eng[e]);
        sound_from(src, tk, &k->patch);
    } else if (e == 12) {                                /* SLICE (SLOOP's builds that have it): ours, its values as they are */
        pi = -1;
        for (int i = 0; i < gb_npresets && pi < 0; i++) if (gb_presets[i].engine == ENG_SLICE && (int)k0 == i - (gb_npresets - 4)) pi = i;
        if (pi >= 0) preset_apply(s, t, pi); else gb_patch_default(&k->patch, ENG_SLICE);
        sound_from(src, tk, &k->patch);
    } else pi = sound_of(e, k0), preset_apply(s, t, pi);   /* (an engine not here: the nearest of its kind) */
    return pi;
}
/* SLOOP's song (its settings, bytes 48..83: how many entries, loop, then each its section and bars) as ours: each
 * entry every track on the section's pattern, the section's mutes; the sections not there left out */
int sloop_song(const uint8_t *pl, int n, const uint8_t *const sections[4], seq_t *q)
{
    if (n < 84 || rd32(pl) != 0x50455233u) return 0;
    const uint8_t *a = pl + 48;
    int cnt = a[0] > 16 ? 16 : a[0];
    q->song_n = 0; q->song_len = 0; q->song_play = 0;
    for (int i = 0; i < cnt; i++) {
        int sc = a[4 + 2 * i], bars = a[5 + 2 * i];
        if (sc > 3 || !sections[sc] || bars < 1 || bars > 64) continue;
        seq_song_ev e; e.bar = q->song_len; e.mute = 0;
        for (int t = 0; t < SEQ_NTRK; t++) e.pat[t] = (uint8_t)sc;
        for (int t = 0; t < 4; t++) if (par(trk(sections[sc], t), SL_MUTE)) e.mute |= (uint8_t)(1u << t);
        if (q->song_n && !memcmp(q->song[q->song_n - 1].pat, e.pat, SEQ_NTRK) && q->song[q->song_n - 1].mute == e.mute) { q->song_len = (uint16_t)(q->song_len + bars); continue; }
        q->song[q->song_n++] = e; q->song_len = (uint16_t)(q->song_len + bars);
    }
    if (!q->song_n) q->song_len = 0;
    return q->song_n;
}
int sloop_import(const uint8_t *const sections[4], const uint8_t *work, seq_t *q, gb_sound *s, project_extra *x, int *preset_of)
{
    const uint8_t *src = work;
    for (int i = 0; i < 4 && !src; i++) src = sections[i];
    if (!src) return 0;
    seq_set_bpm(q, (float)clampi(rd16(src + 8), 40, 240));
    seq_set_swing(q, -1, clampi(rd16(src + 10), 0, 100));
    const uint8_t *g = src + 8;                          /* its globals: the effects, the master, the drums' mix */
    s->dly_fb = (float)clampi(rd16(g + 2 * 5), 0, 120) * (0.95f / 120.0f);
    s->rev_fb = 0.3f + (float)clampi(rd16(g + 2 * 8), 0, 127) * (0.67f / 127.0f);
    s->cho_depth = (float)clampi(rd16(g + 2 * 11), 0, 127) * (1.0f / 127.0f);
    s->dust = (float)clampi(rd16(g + 2 * 27), 0, 127) * (1.0f / 127.0f);
    s->duck = (float)clampi(rd16(g + 2 * 28), 0, 127) * (1.0f / 127.0f);
    sm_set(&s->mfilt, (float)clampi(rd16(g + 2 * 29), -64, 63) * (1.0f / 64.0f));
    if (x) {
        static const uint8_t DLY[9] = { 7, 4, 1, 0, 2, 0, 6, 3, 8 };   /* (its 1/4 1/8 1/16 1/32 8T 16T 1/8D 1/16D 1/4D: ours) */
        x->dly_div = DLY[clampi(rd16(g + 2 * 4), 0, 8)];
        x->fxv[0] = (uint8_t)clampi(rd16(g + 2 * 6), 0, 127); x->fxv[1] = (uint8_t)clampi(rd16(g + 2 * 7), 0, 127);   /* COLR MIX */
        x->fxv[2] = (uint8_t)clampi(rd16(g + 2 * 9), 0, 127); x->fxv[3] = (uint8_t)clampi(rd16(g + 2 * 10), 0, 127);  /* DAMP CRT */
        int has = (src[74] & 0x80u) != 0;                /* TEXTURE (2.6), in its spare bytes */
        x->tex[0] = (uint8_t)(has ? clampi(src[74] & 15, 0, 10) : 0); x->tex[5] = (uint8_t)(has ? (src[74] >> 4) & 1 : 0);
        x->tex[6] = (uint8_t)(has ? (src[74] >> 5) & 1 : 0); x->tex[4] = (uint8_t)(has ? clampi(src[75], 0, 4) : 2);
        int dc = rd16(g + 2 * 24);
        x->tex[1] = (uint8_t)(has ? dc & 127 : 64); x->tex[2] = (uint8_t)(int8_t)(has ? ((dc >> 7) & 127) - 64 : 0);
        x->tex[3] = (uint8_t)(has ? clampi(rd16(g + 2 * 15), 0, 127) : 40); x->tex[7] = 0;
    }
    for (int t = 0; t < 4; t++) {
        const uint8_t *tk = trk(src, t);
        gb_track *k = &s->trk[t];
        int pi = sloop_sound(src, t, s);                 /* the sound: SLOOP's own, value for value */
        if (preset_of) preset_of[t] = pi;
        if (t < 3) {
            sm_set(&k->level, (float)clampi(par(tk, SL_LEVEL), 0, 127) * (1.0f / 127.0f));
            sm_set(&k->pan, 0.5f + (float)clampi(par(tk, SL_PAN), -64, 63) * (1.0f / 128.0f));
            sm_set(&k->send_cho, (float)clampi(par(tk, SL_CHOR), 0, 127) * (1.0f / 127.0f));
            sm_set(&k->send_dly, (float)clampi(par(tk, SL_DLY), 0, 127) * (1.0f / 127.0f));
            sm_set(&k->send_rev, (float)clampi(par(tk, SL_REV), 0, 127) * (1.0f / 127.0f));
        } else {                                         /* the drums: GLO > DRUMS (LVL, REV, DLY) */
            sm_set(&k->level, (float)clampi(rd16(g + 2 * 25), 0, 127) * (1.0f / 381.0f));   /* (3.4.0: our kits 3 x louder: as SLOOP 2.8 played them) */
            sm_set(&k->send_rev, (float)clampi(rd16(g + 2 * 26), 0, 127) * (1.0f / 127.0f));
            sm_set(&k->send_dly, (float)clampi(src[73], 0, 127) * (1.0f / 127.0f));
        }
        seq_track *qt = &q->t[t];
        qt->mute = (uint8_t)(par(tk, SL_MUTE) != 0);
        qt->swing = (uint8_t)clampi(par(tk, SL_SSWING), 0, 100);
        static const uint8_t DIVS[9] = { SEQ_DIV_4, SEQ_DIV_8, SEQ_DIV_16, SEQ_DIV_32, SEQ_DIV_8T, SEQ_DIV_16T, SEQ_DIV_2, SEQ_DIV_BAR, SEQ_DIV_2BAR };
        qt->div = DIVS[clampi(par(tk, SL_SDIV), 0, 8)];
        qt->gate = (uint8_t)clampi(par(tk, SL_SGATE) * 100 / 64, 10, 200);
        if (x && t < 3) {
            x->harm[t][4] = (uint8_t)(64 + clampi(par(tk, SL_TRANS), -24, 24));
            int am = clampi(par(tk, 17), 0, 14);         /* the arpeggiator: MODE RATE OCT GATE SWG PROB HOLD ORD */
            uint8_t *ar = x->arp[t];
            ar[0] = sloop_amode[am]; ar[1] = sloop_arate[clampi(par(tk, 18), 0, 5)]; ar[2] = (uint8_t)clampi(par(tk, 19), 1, 4);
            ar[3] = (uint8_t)clampi(par(tk, 20) * 100 / 127, 5, 100); ar[4] = (uint8_t)clampi(par(tk, 21), 0, 100);
            ar[5] = (uint8_t)clampi(par(tk, 22) * 100 / 127, 0, 100);
            ar[6] = (uint8_t)(par(tk, 23) != 0 && (am || (src[74] & 0x40u)));   /* (before 2.7, HOLD without the arp did nothing) */
            ar[7] = (uint8_t)(par(tk, 24) != 0);
        }
        /* the sections A..D as patterns 1..4 (their locks: on the sound of that section); none: the working project */
        int any = 0;
        for (int i = 0; i < 4; i++) if (sections[i]) { pattern_of(sections[i], t, &qt->pat[i]); locks_of(sections[i], t, &qt->pat[i]); any = 1; }
        if (!any) { pattern_of(work, t, &qt->pat[0]); locks_of(work, t, &qt->pat[0]); }
        qt->cur = 0;
    }
    if (q->hist) q->hist->n_undo = q->hist->n_redo = 0;
    return 1;
}
