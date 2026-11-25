/* SPDX-License-Identifier: GPL-3.0-only
 * The visualiser (vis.c): SLOOP 2.8.5's styles (3.3.6: six of its twelve kept, Sam's choice), made again for SLOOP 3's five tracks and colours. On the
 * TRACKS screen HOME tapped opens it; SELECT the style; HOME (or any page) closes it. It reads what the sound
 * leaves for it (the mix before MASTER at 22 kHz, 8 bits, in a ring; each track's meter, its notes) and draws in
 * the main loop, a tile at a time like every screen: nothing of the audio's time. Its working memory is lent by
 * the firmware (the project's buffer, when no save needs it); lost, the pictures that remember start again.
 * Written for this project; portable (the PC draws the same screens). */
#pragma once
#include <stdint.h>

#define VIS_N 6
extern const uint8_t vis_from12[12];       /* (3.3.6) a style of the twelve (3.0 to 3.3.5): its place in the six */
#define VIS_RING 512                        /* the ring: VIS_RING left, then VIS_RING right */
extern const char *const vis_name[VIS_N];
typedef struct {
    const int8_t *ring; uint16_t w;         /* the mix (as if MASTER were full), the next index written */
    uint16_t col[5];                        /* the tracks as shown: synths 1-4, the drums */
    float lvl[5];                           /* their meters (peaks, 0..1 and over) */
    uint8_t hit[5], vel[5], note[5];        /* notes started (a counter), the last one's velocity and pitch */
    uint8_t kick;                           /* kicks (a counter) */
    uint16_t bpm10, bar; uint8_t beat, playing, rec;
    uint32_t beats256;                      /* where the music is, in beats x 256 (running on when stopped) */
    uint8_t style, name;                    /* the style; name > 0: its name shown (frames left) */
} vis_in;
#define VIS_MEM 11800u                      /* the working memory it needs */
void vis_frame(uint8_t *mem, const vis_in *in);   /* a new picture: worked out once (then drawn a tile at a time) */
void vis_draw(void);                              /* into the tile (gfx.h) */
void vis_splash(uint32_t ms, const char *ver, const uint16_t *col);   /* the start: the SLOOP logo drawn in, at ms (col: the 5 tracks') */
void vis_splash_draw(void);
/* Teachie Beachie's two colours: the paper of its cards and of the start screen, the ink of its words */
#define TP_PAPER RGB(0xFF, 0xCB, 0xA4)       /* #FFCBA4, a creamy peach */
#define TP_INK RGB(0x96, 0x00, 0x00)         /* #960000, a dark red */
extern const uint8_t *tp_logo;               /* the start screen's logo, in the lesson slot (fw/course.inc); 0: none, its name instead */
#define VIS_SPLASH_MS 2600u
