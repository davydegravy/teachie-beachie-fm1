/* SPDX-License-Identifier: GPL-3.0-only
 * The editor's protocol (edproto.c): SysEx between SLOOP 3 and its web editor (web/editor.html), written for this
 * project. A frame: F0 7D 53 33 (7D: non-commercial; "S3"), the command, its bytes packed 8 to 7 (LSB first), F7.
 * Every request gets an answer with the same command (0 with one byte: refused, its reason).
 *   1 INFO        -> protocol (1), tracks, values a sound, project, projects used (u16), your sounds used (u32), version
 *   2 SOUND t     -> t, engine, preset (i16: -1 none, k, -k-2 edited), name (16), each value in its units (i16 x 48)
 *   3 SET t k v   -> the same three: the track's own value k (in its units) set
 *   4 DESC e      -> e, its name (12), then each value: min (i16), max (i16), name (8)
 *   5 READ kind slot off(u32) n(u16 <= 384) -> kind slot off total(u32) bytes: kind 0 a project slot, 1 your sounds,
 *                 2 the project as it is now
 *   6 WBEGIN kind slot total(u32) / 7 WDATA off(u32) bytes / 8 WEND crc32 -> a file written: kind 0 into a project
 *                 slot (the current one: opened again), 1 your sounds
 *   9 LOAD slot   -> slot: that project opened
 *  10 PATTERN t pat first(u16) -> t pat (pattern 255: the one playing) steps(u16) bpm(u16) total(u16) first(u16) count,
 *                 then each note: tick(u16, 96 a step) len(u16) note vel (at most 80 an answer)
 *  11 THEME [40 bytes] -> the screen's colour (HOME > SCREEN > COLOR) and the 40 bytes of your theme (SLOOP 2.7's
 *                 layout: "THM2", name 8, 5 steps, 4 tracks, background, accent, 0, CRC-32 of the 36 before);
 *                 with the 40 bytes: they become your theme, kept with your sounds, and chosen
 *  12 NOTES t pat first(u16) steps(u16) n, each note as PATTERN's -> t pat total: the pattern rewritten (first 0: it is
 *                 emptied first and takes that length; then the notes are added; its locks stay); one undo
 *  13 SAMPLE ...  (see fw/main.c ed_request: a user slot of SLOOP's, "FSMP")
 *  14 BANK sub ... your FM6 bank (which 0) and SYN kits (which 1), as images (app/xbank.h):
 *                 0 -> fm6's source (0 none, 1 SLOOP 2.8's, 2 yours), its slots used (u32), the SYN kits' source (0 the
 *                 defaults, 1 SLOOP 2.8's, 2 yours) · 1 which off(u16) n(u16 <= 384) -> which off total(u16) bytes: the
 *                 image as it plays now · 2 which total(u16) / 3 which off(u16) bytes / 4 which: an image written (its
 *                 sector, then it plays) · 5 kit [a kit's 356 bytes]: SYN kit heard at once, not kept · 6 slot [128 bytes]:
 *                 an FM6 voice heard at once on the tracks that play that slot, not kept
 * Values cross as their units (the same numbers the screen shows). */
#pragma once
#include <stdint.h>
#define ED_H0 0x7D
#define ED_H1 0x53
#define ED_H2 0x33
enum { ED_NONE, ED_INFO, ED_SOUND, ED_SET, ED_DESC, ED_READ, ED_WBEGIN, ED_WDATA, ED_WEND, ED_LOAD, ED_PATTERN, ED_THEME, ED_NOTES, ED_SAMPLE, ED_BANK,
       ED_LIVE, ED_NOTES2, ED_LOCKS, ED_SONG, ED_HOME, ED_NAMES,
       ED_GUIDE };   /* (Teachie Peachie: the guide, fw/main.c) */   /* (protocol 2, 3.1.5: the editor's LIVE, SEQUENCER, SONG, SETTINGS, LIBRARY) */
enum { ED_ERR_BAD = 1, ED_ERR_BUSY, ED_ERR_NONE, ED_ERR_CRC, ED_ERR_FILE, ED_ERR_FLASH };
#define ED_PROTO 2
/* a received SysEx (F0 .. F7, n bytes): its command and its bytes unpacked into out (cap); -1 not ours */
int ed_parse(const uint8_t *sx, uint32_t n, uint8_t *out, uint32_t cap, uint32_t *len);
/* a frame of cmd and its bytes, into out (cap): its length, 0 if it does not fit */
uint32_t ed_frame(int cmd, const uint8_t *raw, uint32_t n, uint8_t *out, uint32_t cap);
/* a SysEx's bytes as USB-MIDI packets: the packet starting at byte i of sx (n bytes); the bytes it took */
int ed_usb_packet(const uint8_t *sx, uint32_t n, uint32_t i, uint8_t pk[4]);
