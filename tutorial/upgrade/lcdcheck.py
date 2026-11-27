#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""(SLOOP 3.4.2 trial, step 7) The screen's start-up: what lcd_init() sends to the panel.

On newer FM-1s SLOOP's colours are faded (sloop-fm1 issue 269): M-VAVE's own firmware sets the panel's voltages, VCOM
and gamma at the start with one table, the same on every FM-1, and SLOOP left them at the controller's reset values.
Teachie Beachie sends M-VAVE's table in lcd_init() (hal/fm1/board.c), between "normal mode" (13) and "display on" (29),
behind a switch of its own (-DTP_LCD_RESET builds without it).

The simulated FM-1 has a stand-in screen (host/hal.c), so this file of SLOOP's is not in it. Here hal/fm1/board.c is
compiled for the computer as it stands, with the chip's registers replaced by a recorder (a stand-in fm1.h), and the
bytes it sends over SPI are read back, each with its command/data line:

  with the table   SLOOP's own commands, then the eleven registers with the values of the report, then the screen
                   cleared and "display on"
  without          -DTP_LCD_RESET: SLOOP's own commands only, exactly as before

Run from the trial's top folder:    python3 tutorial/upgrade/lcdcheck.py      (needs a C compiler: cc)
"""
import os, shutil, subprocess, sys
HERE = os.path.dirname(os.path.abspath(__file__)); TOP = os.path.join(HERE, "..", "..")
OUT = os.path.join(TOP, "build", "tw", "lcdcheck")
# M-VAVE's table, as the report gives it (https://github.com/isod89/sloop-fm1/issues/269): written here apart from board.c
TABLE = [(0xB2, [0x0C, 0x0C, 0x0C, 0x00, 0x33]), (0xB7, [0x56]), (0xBB, [0x18]), (0xC0, [0x2C]), (0xC2, [0x01]), (0xC3, [0x1F]), (0xC4, [0x20]), (0xC6, [0x0F]),
         (0xD0, [0xA6, 0xA1]),
         (0xE0, [0xD0, 0x0D, 0x14, 0x0B, 0x0B, 0x07, 0x3A, 0x44, 0x50, 0x08, 0x13, 0x13, 0x2D, 0x32]),
         (0xE1, [0xD0, 0x0D, 0x14, 0x0B, 0x0B, 0x07, 0x3A, 0x44, 0x50, 0x08, 0x13, 0x13, 0x2D, 0x32])]
SLOOP = [(0x01, []), (0x11, []), (0x3A, [0x55]), (0x36, [0x00]), (0x21, []), (0x13, [])]     # reset, sleep out, 16 bits a pixel, MADCTL, inversion on, normal mode
CLEAR = [(0x2A, [0, 0, 0, 239]), (0x2B, [0, 0, 0, 239]), (0x2C, ["240 rows"])]              # the whole screen black
ON = [(0x29, [])]
FM1_H = r'''#pragma once
#include <stdint.h>
/* the FM-1's registers, replaced by a recorder (tutorial/upgrade/lcdcheck.py) */
uint32_t *fk_t0(void), *fk_con(void), *fk_buf(void), *fk_cnt(void);
void fk_pin(int p, int n, int level);
extern uint32_t fk_reg[8];
#define T0_CON fk_reg[0]
#define T0_PRD fk_reg[1]
#define T0_CNT (*fk_t0())
#define SPI1_CON (*fk_con())
#define SPI1_BAUD fk_reg[2]
#define SPI1_BUF (*fk_buf())
#define SPI1_ADR fk_reg[3]
#define SPI1_CNT (*fk_cnt())
#define IOMAP_CON1 fk_reg[4]
#define PA 0
#define PC 2
#define LCD_CS PC, 7
#define LCD_DC PC, 8
#define LCD_CLK PC, 9
#define LCD_MOSI PC, 10
#define LCD_BL PA, 2
#define KEY_DATA PA, 4
#define KEY_CLK PA, 3
#define KEY_LATCH PA, 1
#define KEY_ROW_PLAY PA, 6
static inline void pin_out(int p, int n, int level) { fk_pin(p, n, level); }
static inline void pin_set(int p, int n, int level) { fk_pin(p, n, level); }
static inline void pin_in_pullup(int p, int n) { (void)p; (void)n; }
static inline int pin_read(int p, int n) { (void)p; (void)n; return 1; }
'''
MAIN_C = r'''#include <stdio.h>
#include <stdint.h>
#include "board.h"
/* every byte written to the SPI buffer, with the command/data line as it stood ("C xx" a command, "D xx" a value);
 * a block sent by DMA as its length. A write lands after the register's address is taken, so it is printed at the next
 * thing that happens. The clock runs on at every look, so the waits end */
uint32_t fk_reg[8]; static uint32_t t0, con, slot, cnt; static int dc, wrote, sent;
static void flush(void) { if (wrote) printf("%c %02X\n", dc ? 'D' : 'C', (unsigned)(slot & 255u)); if (sent) printf("DMA %u\n", (unsigned)cnt); wrote = sent = 0; }
uint32_t *fk_t0(void) { t0 += 1u << 20; return &t0; }
uint32_t *fk_con(void) { con |= 1u << 15; return &con; }
uint32_t *fk_buf(void) { flush(); wrote = 1; return &slot; }
uint32_t *fk_cnt(void) { flush(); sent = 1; return &cnt; }
void fk_pin(int p, int n, int level) { flush(); if (p == 2 && n == 8) dc = level; }
int main(void) { time_init(60); lcd_init(); flush(); return 0; }
'''
fails = []
def check(ok, what):
    print("  %s %s" % ("ok  " if ok else "FAIL", what), flush=True)
    if not ok: fails.append(what)
def show(seq): return " ".join("%02X%s" % (c, "(" + " ".join("%02X" % v if isinstance(v, int) else v for v in d) + ")" if d else "") for c, d in seq)
def sent(flags):
    """lcd_init()'s commands, in order: (the command, its values)"""
    exe = os.path.join(OUT, "lcd" + ("-reset" if flags else ""))
    r = subprocess.run(["cc", "-std=c11", "-O1", "-I", OUT] + flags + [os.path.join(OUT, "board.c"), os.path.join(OUT, "main.c"), "-o", exe], capture_output=True, text=True)
    if r.returncode: print(r.stderr); raise SystemExit("lcdcheck: hal/fm1/board.c did not compile for the computer")
    seq = []
    for line in subprocess.run([exe], capture_output=True, text=True).stdout.split("\n"):
        w = line.split()
        if not w: continue
        if w[0] == "C": seq.append((int(w[1], 16), []))
        elif w[0] == "D": seq[-1][1].append(int(w[1], 16))
        elif w[0] == "DMA":
            if seq[-1][1] and isinstance(seq[-1][1][-1], str): seq[-1][1][-1] = "%d rows" % (int(seq[-1][1][-1].split()[0]) + 1)
            else: seq[-1][1].append("1 rows")
    return seq

if __name__ == "__main__":
    shutil.rmtree(OUT, ignore_errors=True); os.makedirs(OUT)
    for name in ("board.c", "board.h"): shutil.copy(os.path.join(TOP, "hal", "fm1", name), OUT)   # SLOOP's file as it stands
    open(os.path.join(OUT, "fm1.h"), "w").write(FM1_H); open(os.path.join(OUT, "main.c"), "w").write(MAIN_C)
    print("with the table (as built)", flush=True)
    a = sent([]); n = len(SLOOP)
    check(a[:n] == SLOOP, "SLOOP's own commands first: %s" % show(a[:n]))
    check(a[n:n + len(TABLE)] == TABLE, "then M-VAVE's eleven registers, with the report's values: %s" % show(a[n:n + len(TABLE)]))
    check(a[n + len(TABLE):] == CLEAR + ON, "then the screen cleared, and display on last: %s" % show(a[n + len(TABLE):]))
    check(sum(len(d) for _, d in TABLE) == 42, "42 values in all")
    print("without it (-DTP_LCD_RESET)", flush=True)
    b = sent(["-DTP_LCD_RESET"])
    check(b == SLOOP + CLEAR + ON, "SLOOP's commands only, as before: %s" % show(b))
    check(not any(c >= 0xB0 for c, _ in b), "none of the table's registers")
    print("lcdcheck: %s" % ("OK" if not fails else "%d FAILED" % len(fails)))
    sys.exit(1 if fails else 0)
