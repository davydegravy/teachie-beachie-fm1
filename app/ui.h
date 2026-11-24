/* SPDX-License-Identifier: GPL-3.0-only
 * SLOOP 3's screens (ui.c). 4.0: SLOOP 2.8.5's screens, as its players know them (Terminus Bold, capitals, the four
 * 60 px columns, the 16 tiles, the dials), with the fifth track. Before 4.0 (kept for what the fields mean):
 *   - top: the tempo in big type, the transport (play / stop / record, bar.beat and the four beats), a status
 *     (the pattern, the one queued, the song, REC);
 *   - a track header on the pages: its colour, its number, the sound's name in big type;
 *   - the layers (a button held): a big title and 16 tiles in four rows of four, as the 16 white keys;
 *   - bottom: four dials, in the colours of KNOB 1 .. 4 (blue, green, yellow, orange), what each knob does here.
 * White is what you touch, red is recording, yellow a lock or what waits for the bar.
 * The firmware fills a ui_view; ui_draw draws it into the current tile (gfx.h). The PC draws the same screens. */
#pragma once
#include <stdint.h>

#define UI_NTRK 8
enum {
    UI_TRACKS,                 /* HOME: every track, its sound, its steps, its level */
    UI_LAYER,                  /* a layer held (or a tile page): title + 16 tiles */
    UI_OSC, UI_FILTER, UI_VOICE, UI_ENV, UI_ENVDEST, UI_FX,   /* the sound's pages, each with its picture */
    UI_KIT,                    /* the drum track's sound page: its 16 pads */
    UI_MIXER,                  /* GLO tapped: the levels as faders */
    UI_LFO,                    /* the LFO: its wave, its depths */
    UI_SOON,                   /* a page still to come */
    UI_PUNCH,                  /* FX held: the 27 punch-ins as the keyboard (16 white tiles, 11 black above them) */
    UI_MASTER,                 /* FX 2/2: the master bus (DJ filter, DUST, DUCK) and the track's filter */
    UI_PLAY,                   /* EDIT 4/4: how the track plays (POLY, MONO, LEGATO), its glide */
    UI_ARP,                    /* the arpeggiator: its pattern drawn */
    UI_DRUMS,                  /* SEQ tapped on the drums: 16 sounds x 16 steps */
    UI_GENERIC,                /* another engine's page: its four values as bars */
    UI_NOTES,                  /* SEQ tapped twice (a synth track): the pattern as a piano roll, a step edited */
    UI_SONG,                   /* SONG 2/2: the song's entries, each its pattern and its bars (as SLOOP's SONG screen) */
    UI_VIS,                    /* HOME tapped on TRACKS: the visualiser (vis.h), the messages over it */
    UI_SPLASH,                 /* the start: the SLOOP logo drawn in (vis.h) */
    UI_REC,                    /* (4.0) REC armed, its count-in, a free take: as SLOOP 2.8.5's REC screen */
    UI_MENU,                   /* (4.0) HOME held: SLOOP 2.8.5's menu (dpage: the section 0..4, knob i: its row i) */
    UI_SLICER                  /* (4.0.3) FX > SLICER: its pattern drawn, as SLOOP 2.8.5 */
};
/* a step cell (TRACKS) */
enum { UC_NOTE = 1, UC_LOCK = 2, UC_COND = 4, UC_RAT = 8, UC_OUT = 16, UC_PLAY = 32 };
/* a tile */
enum { T_EMPTY, T_ON, T_DIM, T_FRAME, T_WHITE, T_RED, T_NONE, T_GAUGE };   /* (3.3.8) GAUGE: filled from the left as far as lvl, in col */
enum { TF_PLAY = 1, TF_DOT = 2, TF_FILL = 4, TF_NOFILL = 8, TF_BAR = 16, TF_BLINK = 32, TF_LEVEL = 64, TF_WRAP = 128 };   /* LEVEL: a fader along its foot; WRAP: a name, on two lines if one is too short (SOUNDS) */

typedef struct { const char *label; char value[10]; int8_t bar; uint8_t hot, dial; } ui_knob;   /* bar 0..100, -1 none */
typedef struct { char text[11]; uint8_t style, flags; uint16_t col; uint8_t lvl; } ui_tile;   /* lvl: TF_LEVEL, 0..100 */
/* a note of the NOTES page: where (quarter steps in the 16 shown), how long, its pitch, NR_* */
typedef struct { uint8_t x, len, pitch, flags; } ui_note;
enum { NR_SEL = 1, NR_ACC = 2, NR_SLIDE = 4, NR_SOFT = 8, NR_GHOST = 16 };
#define UI_NOTES_MAX 64
/* a row of SONG 2/2: the entry's number, its pattern (the track's), its bars, SR_*; at: the bar playing in it */
typedef struct { uint8_t num, pat, flags, at; uint16_t bars; } ui_songrow;
enum { SR_SEL = 1, SR_MIX = 2, SR_EMPTY = 4, SR_PLAY = 8, SR_MUTE = 16 };
typedef struct {
    uint8_t page, ntrk, trk, blink;
    /* top bar */
    uint16_t bpm, bar; uint8_t beat, playing, recording, armed;
    char status[16]; uint16_t status_col;
    /* each track */
    uint16_t col[UI_NTRK];
    const char *sound[UI_NTRK];                    /* its sound's name */
    const char *kind[UI_NTRK];                     /* its engine, in words */
    uint8_t mute[UI_NTRK], solo[UI_NTRK], rec[UI_NTRK], level[UI_NTRK], edited[UI_NTRK];
    uint8_t cells[UI_NTRK][16];
    /* a layer: its title, a line beside it, its tiles */
    char title[12], sub[40]; uint16_t title_col;
    ui_tile tile[16];
    uint8_t cols;                                  /* tiles a row: 0 or 4 (as the keys), 5 (MIX: a track a column) */
    ui_tile btile[11];                             /* UI_PUNCH: the black keys, low to high */
    /* the sound's pages */
    char page_name[32];                            /* "ENV 1/2", "MIDI 5/5 · USB OK · GOT 12" */
    uint8_t engine;                                /* 0 VA, 1 DRUM */
    uint8_t prm[48];                               /* the sound's own values (engine.h SP_*), 0..100 over each range */
    uint8_t dly_beats8, dly_fb, rev_size, send_dly, send_rev;
    int8_t pad_hit;                                /* the drum pad last played (UI_KIT), -1 none */
    int8_t mfilt, tfilt;                           /* the DJ filter, the track's: -100 LP .. 0 off .. 100 HP */
    uint8_t dust, duck;
    uint8_t arp_n; uint8_t arp_seq[16];            /* UI_ARP: the notes it would play, 0..100 high (255: a rest) */
    uint16_t drum_row[16]; uint8_t drum_sel; int8_t drum_at, drum_cur; uint32_t drum_lvl, drum_rat; uint8_t drum_sheet;   /* UI_DRUMS: each sound's steps (bit i), its sound, the playhead,
                                                   (3.3.8) the step picked (-1 none), the sound picked's levels and ratchets (2 bits a step),
                                                   the sheet open (page_name its title) */
    const char *const *pad_names;
    uint16_t pad_mute;                             /* the drum sounds muted (bit = pad) */
    ui_note nr[UI_NOTES_MAX]; uint8_t nr_n, nr_lo, nr_hi, nr_cur, nr_play, nr_cols, nr_bar;   /* UI_NOTES: the notes (x, len
                                                   in 64ths of the steps shown), the pitches shown, the step edited (0 .. nr_cols
                                                   - 1), the playhead (64ths; 255 none), the steps shown (0: 16), the steps in a
                                                   bar (the track's DIV, 3.3.8; 0: 16) */
    ui_songrow sg[5]; uint8_t sg_n;                /* UI_SONG: the rows shown */
    /* the dials */
    ui_knob knob[4];
    /* a message over the page */
    const char *toast; uint8_t toast_kind; uint8_t saved;     /* (3.3.8) saved: 1 the big SAVED, 2 SAVE FAILED */
    const char *info;                              /* 3.3: what just changed: in the page's own title line, nothing covered */
    uint8_t saving, save_pct;
    uint8_t batt, batt_usb;                        /* (3.3.6) the battery: 0..100 % (255: not read yet); USB: 1..8, the light's step */
    /* (4.0) as SLOOP 2.8.5 */
    uint8_t rec_kind;                              /* UI_REC: 0 armed, free take; 1 armed, a note starts; 2 armed, PLAY counts in;
                                                      3 counting in (count_n 4..1); 4 a free take (take_*) */
    uint8_t count_n, take_secs, take_bars; uint16_t take_bpm;
    int8_t pat_badge; uint8_t pat_next;            /* TRACKS / the drums: the pattern playing (0..7, -1 none), the one waiting (255 none) */
    uint8_t hot_k, oct; int8_t octave;             /* the knob just turned (0..3, 255 none): its value white; the octave (0: none shown) */
    char head_r[12];                               /* a layer's readout, top right (FX: its fade) */
    uint8_t locked;                                /* the layer locked open (HOME): LOCK */
    uint8_t dpage;                                 /* the drums' screen: 0 GRID, 1 KIT, 2 SOUND */
    uint16_t slc_pat; uint8_t slc_mode, slc_depth, slc_at;   /* (4.0.3) UI_SLICER: the pattern (bit i), OFF GATE STUT, 0..100, the step playing */
} ui_view;

void ui_draw(const ui_view *v);
/* the screen's colours (HOME > SCREEN > COLOR): NIGHT, SLOOP's GREEN AMBER CYAN RED MONO, or your theme */
typedef struct { uint16_t bg, panel, panel2, line, txt, dim; } ui_palette;
extern ui_palette ui_pal;
#define UI_NTHEMES 6
extern const char *const ui_theme_name[UI_NTHEMES];
void ui_theme(int i);                                      /* a built-in one */
void ui_theme_own(const uint16_t c[5], uint16_t bg);       /* yours: five steps faintest .. strongest, the background */
extern const uint16_t ui_knob_col[4];
