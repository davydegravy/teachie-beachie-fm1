# An issue for SLOOP 3: faded colors on newer FM-1s

A draft, for the course's author to post from their own account on SLOOP 3's tracker
(<https://github.com/isod89/sloop3-fm1/issues>). Nothing has been posted. SLOOP's author does not merge pull
requests; he fixes what issues report, in his own release. So the change is given in the issue itself.

**Before posting:** fill in the two results under "Tested" (or say plainly which one is still missing). The newer
screen can be looked at when one of the ordered FM-1s arrives, late December 2026 (`tutorial/upgrade/decisions.md`,
decision 8).

---

**Title:** Faded colors on newer FM-1s: the panel's start-up settings (as sloop-fm1 #269)

**Text:**

On newer FM-1s the colors are faded on SLOOP 3, as reported for SLOOP 2.8 in
[sloop-fm1#269](https://github.com/isod89/sloop-fm1/issues/269): greys turn white, and on the drum grid GHOST, SOFT
and NORM look the same. On M-VAVE's own firmware (V15, V16) the same unit looks right.

**Why.** M-VAVE's firmware sets the panel's voltages, VCOM and gamma at start-up, with one table, the same in V15 and
V16 and sent to every FM-1. SLOOP 3's `lcd_init()` (`hal/fm1/board.c`, the same from 3.3.7 to 3.4.2) sends reset,
sleep out, the pixel format, MADCTL, inversion on, normal mode and display on, and leaves those settings at the
ST7789V's reset values. The original panels look right that way; the newer ones do not.

**The table,** as #269 gives it (COLMOD `55`, MADCTL `00` and INVON are already SLOOP's):

| Register | | Values |
|---|---|---|
| `B2` | PORCTRL | `0C 0C 0C 00 33` |
| `B7` | GCTRL | `56` |
| `BB` | VCOMS | `18` |
| `C0` | LCMCTRL | `2C` |
| `C2` | VDVVRHEN | `01` |
| `C3` | VRHS | `1F` |
| `C4` | VDVS | `20` |
| `C6` | FRCTRL2 | `0F` |
| `D0` | PWCTRL1 | `A6 A1` |
| `E0` | PVGAMCTRL | `D0 0D 14 0B 0B 07 3A 44 50 08 13 13 2D 32` |
| `E1` | NVGAMCTRL | `D0 0D 14 0B 0B 07 3A 44 50 08 13 13 2D 32` |

**Why it should be safe on every FM-1:** it is what M-VAVE's own firmware sends to all of them, old and new. It is
sent once at start-up, before display on. It touches neither the flash nor a project.

**The change,** in `hal/fm1/board.c`, as it is in Teachie Beachie's fork of SLOOP 3.4.2 (a table and one loop,
between normal mode `13` and the clearing of the screen):

```c
/* The panel's voltages, VCOM and gamma, as M-VAVE's own firmware (V15, V16) sets them at the start on every FM-1.
 * Each entry: the register, how many values, the values. */
static const uint8_t LCD_SETUP[] = {
    0xB2, 5, 0x0C, 0x0C, 0x0C, 0x00, 0x33,                                                           /* PORCTRL */
    0xB7, 1, 0x56,                                                                                   /* GCTRL */
    0xBB, 1, 0x18,                                                                                   /* VCOMS */
    0xC0, 1, 0x2C,                                                                                   /* LCMCTRL */
    0xC2, 1, 0x01,                                                                                   /* VDVVRHEN */
    0xC3, 1, 0x1F,                                                                                   /* VRHS */
    0xC4, 1, 0x20,                                                                                   /* VDVS */
    0xC6, 1, 0x0F,                                                                                   /* FRCTRL2 */
    0xD0, 2, 0xA6, 0xA1,                                                                             /* PWCTRL1 */
    0xE0, 14, 0xD0, 0x0D, 0x14, 0x0B, 0x0B, 0x07, 0x3A, 0x44, 0x50, 0x08, 0x13, 0x13, 0x2D, 0x32,   /* PVGAMCTRL */
    0xE1, 14, 0xD0, 0x0D, 0x14, 0x0B, 0x0B, 0x07, 0x3A, 0x44, 0x50, 0x08, 0x13, 0x13, 0x2D, 0x32,   /* NVGAMCTRL */
};
```

and in `lcd_init()`, after `lcd_cmd(0x21); lcd_cmd(0x13);`:

```c
    for (const uint8_t *p = LCD_SETUP; p < LCD_SETUP + sizeof LCD_SETUP; p += 2 + p[1]) { lcd_cmd(p[0]); lcd_data(p + 2, p[1]); }
```

**What it costs** in SLOOP 3.4.2's build: 88 bytes of the flash before the sample bank and 128 bytes of RAM (the file
runs from RAM).

**Tested:**

- The commands `lcd_init()` sends were recorded on a computer (the file compiled with the chip's registers replaced
  by a recorder): SLOOP's own commands, then the eleven registers with the values above, then the screen cleared
  and display on.
- On an FM-1 with the original panel: _to fill in: nothing changes._
- On an FM-1 with a newer panel: _to fill in: the colors look right._

**Credit:** the report and the table are from sloop-fm1#269; its author tested the same table on their newer unit, on
SLOOP 2.8.5 ([Tragiciy/sloop-fm1@73f00ee](https://github.com/Tragiciy/sloop-fm1/commit/73f00ee270fde8c08bd924e16cb20628abe0ce95)).
