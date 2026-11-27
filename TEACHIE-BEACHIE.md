# Teachie Beachie

Teachie Beachie is firmware for the M-VAVE FM-1, built on **SLOOP 3** by the SLOOP project
([github.com/isod89/sloop3-fm1](https://github.com/isod89/sloop3-fm1)). It is free software under the
GPL-3.0, the same license as SLOOP 3 (see `LICENSE`). The SLOOP project does not make or support Teachie Beachie.

The base is SLOOP 3 **v3.4.2-beta**, the tag `v3.4.2-beta` in this repository. `README.md` is Teachie Beachie's own;
SLOOP's guide to the instrument is `docs/GUIDE.md`. This file is for someone who builds it or merges a newer SLOOP:
what is Teachie Beachie's, and what was changed in SLOOP's own files. How the move from 3.3.7 to 3.4.2 was made,
step by step, is `tutorial/UPGRADE-3.4.2.md`.

## Building on a Mac (Apple chip)

You need three things, all kept inside this folder and none installed on the system:
- **The compiler for the FM-1's chip:** the pi32v2 LLVM toolchain by Julius Vitkauskas (MIT), release
  `pi32v2-23.1.5-2`, file `pi32v2-llvm-23.1.5-2-macos-arm64.tar.xz` (35 MB). It is checked against the
  release's `SHA256SUMS` and unpacked into `toolchain/`.
- **Python 3** with NumPy and Pillow.
- **Node.js.**

The commands, each from this folder:

```sh
make test
make build/twin
dash host/twin.sh 60 "1 2"
EDITOR_E2E=0 INSTALLER_E2E=0 sh fw/build.sh toolchain/bin
```

- **`make test` runs SLOOP's PC tests.**
- **`make build/twin` builds the simulated FM-1,** the whole firmware on the computer. After a change to the
  firmware, build it before `fw/build.sh`: the build's lessons test runs the simulated FM-1 as it is.
- **`dash host/twin.sh 60 "1 2"` runs SLOOP's scenarios on it,** in about twenty seconds. On a Mac use `dash`, not
  `sh`: a Mac's `sh` is bash 3.2, which carries one scenario's settings into the next and fails two of them falsely.
- **The last command builds the firmware and the installer page.** It writes `build/fw/fw.area` and
  `build/fw/installer-fm1.html`, prints the firmware's five rooms (`tools/tp_room.py`), checks the page against a
  simulated m-bootloader and its lessons step against a simulated FM-1, and copies the page to `docs/index.html`,
  from where GitHub Pages serves it. The two `=0` settings skip the browser tests, which need Playwright and
  Chromium.

The lessons have checks of their own, on the simulated FM-1:

```sh
python3 tutorial/check.py              # every lesson read, its words measured against the screen
python3 tutorial/check.py --run        # all thirty played with the computer leading (it also makes their pictures)
python3 tutorial/device.py             # all thirty played with the FM-1 leading itself; --clumsy, --sticky: other robots
python3 tutorial/upgrade/keepcheck.py      # a lesson's song kept over a change of pattern
python3 tutorial/upgrade/drumcheck.py      # the drum picked, and the lesson's words for the wrong one
python3 tutorial/upgrade/rollcheck.py      # the piano roll: which key blinks
python3 tutorial/upgrade/framecheck.py     # the framed names answer the controls
python3 tutorial/upgrade/lcdcheck.py       # what the screen's start-up sends
python3 tutorial/upgrade/remembercheck.py  # the lesson remembered after power-off
python3 tutorial/upgrade/soundscheck.py    # a blank FM-1 gets its "your sounds" file
```

SLOOP 3.3.7, unchanged and built this way, was byte-for-byte the firmware inside SLOOP's own published installer
page. That was not checked again for 3.4.2.

## What is changed in SLOOP's files

SLOOP's own files stay as they are wherever that is possible, so that a newer SLOOP merges cleanly. Each change below
is marked in the code with "Teachie Beachie".

**Making the PC side build on a Mac.** None of these change the FM-1 firmware itself.
- **Section names:** a section name goes through `GB_SECT` in `core/gb.h`, which leaves it out on a Mac. Mach-O
  only takes `segment,section` names. The twin's scrambled-RAM section uses Mach-O's own name, and so does
  `fw/unpack.c`.
- **Sanitizers:** macOS 26 with Apple clang 17's AddressSanitizer hangs before `main`. On a Mac the twin, the
  fuzzer and the unpack test use the undefined-behavior checks only (`SAN` in the `Makefile` and
  `fw/build.sh`).
- **The check of the flash window's code** (SLOOP 3.4.1: no code of the window may run while the window is off)
  needs `objcopy` and ELF sections. On a Mac the simulated FM-1 is built in one go without it (`Makefile`,
  `app/store.c`, `host/hal.c`); on Linux SLOOP's way is kept. So a change that writes to the flash should be run
  once on Linux.
- **Missing Linux commands:** `host/twin.sh` falls back to `perl`'s alarm when there is no `timeout`. In
  `fw/build.sh`, `stat -c` becomes `wc -c`. `host/twin.c` includes `unistd.h` for `_exit`.

**Room made for the lessons' code** (`fw/main.c`, `app/ui.c`, `host/twin.c`). SLOOP's code is left in place behind a
switch of our own, which no build defines:
- **`TP_VISUALIZER`:** SLOOP's visualizer. HOME on the main screen does nothing.
- **`TP_SLOOP28_PROJECT`, `TP_SLOOP28_DATA`:** what SLOOP 3 brings over from SLOOP 2.8 at its first start: the
  project, the user sounds, favourites and colour theme, the SYN kits, the MIDI settings, the place of the FM6 bank.
  Its samples, menu settings and calibration still come along, and an empty "your sounds" file is still written at a
  first start, as SLOOP does: its web editor keeps a sound in that file.
- **`TP_SPLASH_MOVES`:** SLOOP's start draws itself in; Teachie Beachie's is a still picture, drawn when it changes.

The firmware's five rooms after all of it are in `tutorial/UPGRADE-3.4.2.md`, "The room": 2,672 bytes free before
the sample bank, where SLOOP's own 3.4.2 has 1,912.

**The name:**
- **Start screen:** Teachie Beachie's own logo, a round sticker with the name over a sunset on the sea, on the cards'
  peach color, with "WITH SLOOP 3" under it (`app/vis.c`; SLOOP's sailboat is no longer drawn there). The picture is
  `assets/logo/teachie-beachie.webp`; `tools/mklogo.py` makes it flat with smooth edges, in full color, 198 x 204,
  and writes `assets/logo/teachie-beachie-fm1.bin` (17 KB). It is not in the firmware: it sits at the end of the
  lessons' sample slot, and the firmware draws it from there. With no lessons on the FM-1 the start shows the name
  instead, in SLOOP's capitals. The page's two pictures of it are made by `tools/mkpagelogo.py`. A new picture needs
  its own colors at the top of `tools/mklogo.py`.
- **Installer page:** `web/installer.tmpl.html` stays exactly SLOOP's, so a merge never clashes there. During the
  build, `tools/teachie_page.py` rewrites the finished page (`fw/build.sh` runs it after `mkpage.py`):
  - Teachie Beachie wherever the page names the firmware it installs (in sentence case, as on the FM-1), with SLOOP 3
    credited in the title, the beta note and the footer.
  - Its own look, added after SLOOP's styles (SLOOP's markup and script untouched): the FM-1 cards' two colors,
    peach paper `#FFCBA4` and dark red ink `#960000` (`app/vis.h`), and the lessons' font, Google Sans, inside the
    page (`assets/fonts/page-font.woff`, made by `tools/mkpagefont.py`; the page still loads nothing from the web).
    No bar or menu at the top: SLOOP's is hidden, not removed, since its script still uses it.
  - English only: SLOOP's French words are taken out, and the page no longer follows the browser's language.
    `tests/installer_e2e.js` goes through it in English only.
  - At the top the logo, the title, an introduction and twelve screens of the lessons (`assets/shots/`), then the
    lessons with an AI assistant. SLOOP's beta badge, its three facts and its 3D picture are hidden; the browser's
    checks are in step 1.
  - Six steps, the sixth being the lessons: the page carries the thirty lessons and sends them to the FM-1 after
    the install, or alone (`web/teachie_lessons.js`).
  - Nothing about SLOOP 2.8: its three notes and two half-sentences are taken out.
  - No author's name on the page. The GPL needs the license named and the source reachable, not a name; the
    copyright notices are in the source files. The page's own web address, which shows the account name, is left
    out of the message for a browser that cannot install.
  - Bug reports point to Teachie Beachie, not to SLOOP's tracker.
  - It shows version `0.2 · SLOOP 3.4.2`, with Teachie Beachie's version set at the top of the script.
  - If SLOOP rewords the page, the script names each sentence it no longer finds, and any mention of SLOOP 2.8 left
    in the page's words. It stops the build if the name or the credit to SLOOP would be missing.
  - `tests/install_sim.js` expects the new "restarts on Teachie Beachie" message; `tests/lessons_sim.js` is ours.

**The lessons** (Teachie Beachie's own files, and its own part of `fw/main.c`):
- **The lessons' player** is `fw/course.inc`, included by `fw/main.c`, with its numbers in `app/tp_course.h`.
  `tools/mkcourse.py` packs `tutorial/lessons/*.md` for it, with their song, their two fonts and the course's card.
  The pack's mark is `TPC4`; 3.3.7's was `TPC3`, so neither firmware takes the other's lessons.
- **The guide** is a marked part of `fw/main.c`: the cards and strips over SLOOP's pages, the lights, the keys a
  lesson takes for itself, and a message of the editor's protocol (`ED_GUIDE`) by which a computer can lead a
  lesson. It also matches each framed name on a card to its button, knob or key, and fills the frame while that is
  held or as far as a knob's turn is done.
- **The lesson remembered:** when a lesson begins, the player asks for one byte to be written into the blank part
  of the lessons' slot; three marked lines in the main loop of `fw/main.c` start that write from RAM when the flash
  is free.
- **In SLOOP's test tool,** `host/twin.c`: the simulated FM-1 takes a learner's hands on its command line (keys,
  buttons, knobs, the lights read back, the sound kept as a file), for `tutorial/tp/fm1.py`.

**The screen's start-up** (`hal/fm1/board.c`, SLOOP's file): M-VAVE's own table of the panel's voltages and gamma is
sent in `lcd_init()`, for the newer FM-1s whose colors are faded without it (sloop-fm1 issue 269). `-DTP_LCD_RESET`
builds without it. It is offered to SLOOP as an issue (`tutorial/upgrade/sloop-issue-screen-colors.md`); when SLOOP
has its own, a merge takes SLOOP's.

**The lessons' font:**
- The lessons are drawn in a font of their own, with small letters: **Google Sans SemiBold** ((c) The Google Sans
  Project Authors, SIL Open Font License 1.1; the license and Google's note on its trademarks are in
  `assets/fonts/google-sans/`). `tools/mklessonfont.py` renders it once into `assets/fonts/lessons-font.bin` (two
  sizes, 14 KB), with one picture among its letters, a black key, and `tools/mkcourse.py` puts that into the lesson
  pack. The typeface's own file is not in this repository: it is `GoogleSans[GRAD,opsz,wght].ttf` from
  `github.com/google/fonts`, folder `ofl/googlesans`.
- SLOOP's own screens keep SLOOP's fonts (Inter Bold, capitals only). With no lesson pack on the FM-1, the
  cards use those too, in capitals.
- In `app/gfx.c`, a font with more than SLOOP's 77 glyphs is read as all of ASCII (a letter's glyph is its code
  less 32). Nothing else in SLOOP's drawing changes.

**Not changed:**
- **The USB MIDI name, "FM-1 SLOOP 3".** The installer and SLOOP's web editor recognize the FM-1 by that name.
- **The firmware's own version, `VERSION` "3.4.2" in `fw/main.c`.** The editor reads it.
