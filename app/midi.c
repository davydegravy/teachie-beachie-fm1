/* SPDX-License-Identifier: GPL-3.0-only
 * MIDI apart from the wires: see midi.h. Written from the MIDI 1.0 specification and the USB-MIDI class
 * specification 1.0 (its code index numbers); the ways of playing as SLOOP 2.8's README describes them. */
#include "midi.h"
#include "../engine/engine.h"

const char *const midi_in_name[MIDI_IN_N] = { "OFF", "TRACKS", "SEL" };
const char *const midi_sync_name[MIDI_SYNC_N] = { "INT", "EXT" };
const char *const midi_out_name[MIDI_OUT_N] = { "OFF", "KEYS", "SEQ" };
const char *const midi_dmap_name[MIDI_DMAP_N] = { "GM", "PADS" };

void midi_cfg_default(midi_cfg *c)
{
    c->in = MIDI_IN_TRACKS; c->sync = MIDI_SYNC_INT; c->out = MIDI_OUT_KEYS; c->clk_out = 0; c->dmap = MIDI_DMAP_GM;
}
void midi_cfg_check(midi_cfg *c)
{
    if (c->in >= MIDI_IN_N) c->in = MIDI_IN_TRACKS;
    if (c->sync >= MIDI_SYNC_N) c->sync = MIDI_SYNC_INT;
    if (c->out >= MIDI_OUT_N) c->out = MIDI_OUT_KEYS;
    c->clk_out = c->clk_out ? 1 : 0;
    if (c->dmap >= MIDI_DMAP_N) c->dmap = MIDI_DMAP_GM;
}

/* ---------------------------------------------------------------- bytes */
int midi_msg_len(uint8_t st)
{
    if (st < 0x80) return 0;
    if (st < 0xC0 || (st >= 0xE0 && st < 0xF0)) return 3;
    if (st < 0xE0) return 2;
    if (st == 0xF1 || st == 0xF3) return 2;
    if (st == 0xF2) return 3;
    return 1;                                   /* F6, F7, the realtime ones (F0: a SysEx, of its own length) */
}
static void sx_keep(midi_parser *p, uint8_t b)
{
    if (p->sxn < MIDI_SX_MAX) p->sxb[p->sxn++] = b;
    if (p->sxe && p->sx_len < p->sxe_cap) p->sxe[p->sx_len] = b;
    p->sx_len++;
}
int midi_byte(midi_parser *p, uint8_t b, midi_msg *m)
{
    if (b >= 0xF8) { m->st = b; m->d1 = m->d2 = 0; return MIDI_MSG; }   /* realtime: anywhere, nothing disturbed */
    if (b == 0xF0) { p->sx = 1; p->sxn = 0; p->sx_len = 0; sx_keep(p, b); p->st = 0; p->sys = 0; p->got = 0; return MIDI_NONE; }
    if (b == 0xF7) {
        if (!p->sx) return MIDI_NONE;
        sx_keep(p, b); p->sx = 0;
        return MIDI_SYSEX;
    }
    if (b & 0x80) {                                                 /* a status: what was unfinished is dropped */
        p->sx = 0; p->got = 0;
        if (b < 0xF0) { p->st = b; p->sys = 0; p->need = (uint8_t)(midi_msg_len(b) - 1); return MIDI_NONE; }
        p->st = 0;                                                  /* (system common: no running status after) */
        int n = midi_msg_len(b);
        if (n == 1) { p->sys = 0; m->st = b; m->d1 = m->d2 = 0; return MIDI_MSG; }   /* F6 */
        p->sys = b; p->need = (uint8_t)(n - 1);
        return MIDI_NONE;
    }
    if (p->sx) { sx_keep(p, b); return MIDI_NONE; }
    uint8_t st = p->st ? p->st : p->sys;
    if (!st) return MIDI_NONE;                                      /* data with no status: nothing to do with it */
    if (p->need == 2 && !p->got) { p->d1 = b; p->got = 1; return MIDI_NONE; }
    m->st = st;
    if (p->need == 2) { m->d1 = p->d1; m->d2 = b; } else { m->d1 = b; m->d2 = 0; }
    p->got = 0; p->sys = 0;
    return MIDI_MSG;
}
int midi_usb_bytes(const uint8_t pk[4])
{
    static const uint8_t N[16] = { 0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1 };
    return N[pk[0] & 15u];
}
void midi_usb_packet(const uint8_t *msg, int n, uint8_t pk[4])
{
    uint8_t st = msg[0], cin;
    if (st < 0xF0) cin = (uint8_t)(st >> 4);
    else if (st >= 0xF8) cin = 0x0F;
    else cin = n == 3 ? 3 : n == 2 ? 2 : 5;
    pk[0] = cin;
    for (int i = 0; i < 3; i++) pk[1 + i] = i < n ? msg[i] : 0;
}
int midi_is_soft_key(const midi_parser *p)
{
    static const uint8_t KEY[6] = { 0xF0, 0x22, 0x24, 0x35, 0x7D, 0xF7 };
    if (p->sx_len != 6) return 0;
    for (int i = 0; i < 6; i++) if (p->sxb[i] != KEY[i]) return 0;
    return 1;
}

/* ---------------------------------------------------------------- channels, drums */
/* the tracks as the player sees them (fw/main.c POS): synths 1-4 = tracks 0 1 2 4, the drums (track 3) 5th; a swap,
 * so it maps both ways */
static const int8_t MIDI_SHOWN[5] = { 0, 1, 2, 4, 3 };
int midi_track_of(const midi_cfg *c, int ch, int sel, int ntrk)
{
    if (c->in == MIDI_IN_OFF || ch < 0 || ch > 15) return -1;
    if (c->in == MIDI_IN_SEL) return sel;
    if (ch == 9) return MIDI_DRUM_TRACK;
    if (ch < ntrk) return ch < 5 ? MIDI_SHOWN[ch] : ch;
    return sel;
}
/* the synths shown 1-4 on channels 1-4, the drums on 10 (and on 5, their place on the screen) */
int midi_ch_of_track(int t) { return t == MIDI_DRUM_TRACK ? 9 : t < 5 ? MIDI_SHOWN[t] : t; }

/* the 16 sounds (fw/main.c PAD): KICK KICK2 SNARE CLAP HAT OPEN PEDAL RIM SNR2 TOM-L TOM-H CRASH RIDE SHAKE CONGA BELL */
static const uint8_t GM_OUT[16] = { 36, 35, 38, 39, 42, 46, 44, 37, 40, 41, 48, 49, 51, 70, 63, 56 };
static const int8_t GM_IN[48] = {                                   /* notes 35 .. 82 */
    1, 0, 7, 2, 3, 8, 9, 4, 9, 6, 9, 5, 10, 10, 11, 10,                     /* 35 .. 50 */
    12, 11, 12, 13, 11, 15, 11, -1, 12, 14, 14, 14, 14, 14, 14, 14,         /* 51 .. 66 */
    15, 15, 13, 13, -1, -1, -1, -1, 7, 7, 7, -1, -1, 15, 15, 13 };          /* 67 .. 82 */
int midi_drum_in(int note, int map)
{
    if (map == MIDI_DMAP_PADS) return note >= 36 && note < 52 ? note - 36 : -1;
    return note >= 35 && note <= 82 ? GM_IN[note - 35] : -1;
}
int midi_drum_out(int pad, int map)
{
    pad &= 15;
    return map == MIDI_DMAP_PADS ? 36 + pad : GM_OUT[pad];
}

/* ---------------------------------------------------------------- controllers (SLOOP 2.8's numbers, the GM ones) */
midi_cc_dest midi_cc(int cc)
{
    midi_cc_dest d = { CC_NONE, 0 };
    switch (cc) {
    case 1:  d.what = CC_PARAM; d.param = SP_LPIT; break;           /* the mod wheel: the LFO to the pitch */
    case 5:  d.what = CC_PARAM; d.param = SP_GLIDE; break;
    case 7:  d.what = CC_LEVEL; break;
    case 10: d.what = CC_PAN; break;
    case 11: d.what = CC_LEVEL; break;                              /* (3.4.2, issue #21) EXPRESSION: the level too */
    case 64: d.what = CC_SUSTAIN; break;
    case 71: d.what = CC_RES; break;                                /* the engine's resonance (RES or Q) */
    case 72: d.what = CC_PARAM; d.param = SP_REL; break;
    case 73: d.what = CC_PARAM; d.param = SP_ATK; break;
    case 74: d.what = CC_BRIGHT; break;                             /* the engine's cutoff, else the track's filter */
    case 75: d.what = CC_PARAM; d.param = SP_DEC; break;
    case 76: d.what = CC_PARAM; d.param = SP_LRATE; break;          /* the LFO's rate */
    case 91: d.what = CC_SEND_REV; break;
    case 93: d.what = CC_SEND_CHO; break;
    case 94: d.what = CC_SEND_DLY; break;
    case 120: case 123: d.what = CC_ALL_OFF; break;
    case 121: d.what = CC_RESET; break;
    default: break;
    }
    return d;
}

/* ---------------------------------------------------------------- the clock in */
void mclk_init(midi_clock *c, uint32_t tpu)
{
    uint8_t *z = (uint8_t *)c; for (uint32_t i = 0; i < sizeof *c; i++) z[i] = 0;
    c->tpu = tpu ? tpu : 1;
}
void mclk_start(midi_clock *c) { c->have = 0; c->pulses = 0; c->done = 0; }
static void mclk_tempo(midi_clock *c, uint32_t dt)
{
    float k = 60.0e6f * (float)c->tpu, one = k / (float)dt;
    if (c->nb && (one - c->bpm >= 5.0f || c->bpm - one >= 5.0f)) c->nb = 0;   /* a new tempo: from now */
    c->bdt[c->bi] = dt; c->bi = (uint8_t)((c->bi + 1u) % MIDI_CLK_NB);
    if (c->nb < MIDI_CLK_NB) c->nb++;
    float sum = 0.0f;
    for (int i = 0; i < c->nb; i++) sum += (float)c->bdt[(c->bi + MIDI_CLK_NB - 1u - (uint32_t)i) % MIDI_CLK_NB];
    float mean = k * (float)c->nb / sum, d = mean - c->bpm;
    if (c->nb < MIDI_CLK_NB || d >= 0.8f || d <= -0.8f) c->bpm = (float)(int)(mean * 10.0f + 0.5f) * 0.1f;
}
void mclk_pulse(midi_clock *c, uint32_t now, int src)
{
    uint32_t gap = now - c->last;
    int fresh = !c->alive || gap >= 500000u * c->tpu;
    if (fresh) { c->n24 = 0; c->nb = 0; c->beat_t = now; c->iv = 0; }
    else {
        if (gap < 200000u * c->tpu) c->iv = c->iv ? (c->iv * 3u + gap + 2u) / 4u : gap;   /* (a gap is no tempo) */
        if (++c->n24 == 24u) {
            uint32_t dt = now - c->beat_t;
            c->n24 = 0; c->beat_t = now;
            if (dt >= 250000u * c->tpu && dt <= 1500000u * c->tpu) mclk_tempo(c, dt);   /* 40 .. 240 BPM */
        }
    }
    c->alive = 1; c->last = now; c->src = (uint8_t)src;
    if (c->have) c->pulses++;
    c->have = 1;                                                    /* the first pulse after START: the downbeat */
}
int mclk_alive(const midi_clock *c, uint32_t now)       /* (now may be a little before the last pulse: alive) */
{
    return c->alive && (int32_t)(now - c->last) < (int32_t)(500000u * c->tpu);
}
uint64_t mclk_advance(midi_clock *c, uint32_t now, uint64_t cap)
{
    if (!c->have) return 0;
    const uint64_t P = 1u << 16;
    uint64_t off = 0;
    if (c->iv) {                                                    /* between two pulses: as the last interval says */
        uint32_t el = (int32_t)(now - c->last) > 0 ? now - c->last : 0u;   /* (now before it: none yet) */
        if (el > c->iv) el = c->iv;
        off = (uint64_t)el * P / c->iv;
        if (off >= P) off = P - 1u;                                 /* never the next pulse before it comes */
    }
    uint64_t tgt = (uint64_t)c->pulses * P + off, adv = tgt > c->done ? tgt - c->done : 0;
    if (adv > cap) adv = cap;                                       /* behind: caught up, never in a jump */
    c->done += adv;
    return adv;
}
