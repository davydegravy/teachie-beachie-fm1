# Teachie Peachie

Teachie Peachie is firmware for the M-VAVE FM-1, built on **SLOOP 3** by the SLOOP project
([github.com/isod89/sloop3-fm1](https://github.com/isod89/sloop3-fm1)). It is free software under the
GPL-3.0, the same licence as SLOOP 3 (see `LICENSE`). The SLOOP project does not make or support Teachie Peachie.

The base is SLOOP 3 **v3.3.7-beta**, the tag `v3.3.7-beta` in this repository. SLOOP's own README stays as it
was, in `README.md`.

## Building on a Mac (Apple chip)

You need three things, all kept inside this folder and none installed on the system:
- **The compiler for the FM-1's chip:** the pi32v2 LLVM toolchain by Julius Vitkauskas (MIT), release
  `pi32v2-23.1.5-2`, file `pi32v2-llvm-23.1.5-2-macos-arm64.tar.xz` (35 MB). It is checked against the
  release's `SHA256SUMS` and unpacked into `toolchain/`.
- **Python 3** with NumPy and Pillow.
- **Node.js.**

The commands, each from this folder:

```sh
EDITOR_E2E=0 INSTALLER_E2E=0 sh fw/build.sh toolchain/bin
make test
make twin
```

- **The first command builds the firmware and installer page.** It writes `build/fw/fw.area` and
  `build/fw/installer-fm1.html`, then checks the page against a simulated m-bootloader. The two `=0`
  settings skip the browser tests, which need Playwright and Chromium.
- **`make test` runs SLOOP's PC tests.**
- **`make twin` runs the whole firmware on a simulated FM-1, through every scenario.** It takes about ten
  minutes.

The unchanged v3.3.7 built this way is byte-for-byte the firmware inside SLOOP's own published installer page
(`index.html`).

## What is changed from SLOOP 3.3.7

**Making the PC side build on a Mac.** None of these change the FM-1 firmware itself.
- **Section names:** a section name goes through `GB_SECT` in `core/gb.h`, which leaves it out on a Mac. Mach-O
  only takes `segment,section` names. The twin's scrambled-RAM section uses Mach-O's own name, and so does
  `fw/unpack.c`.
- **Sanitizers:** macOS 26 with Apple clang 17's AddressSanitizer hangs before `main`. On a Mac the twin, the
  fuzzer and the unpack test use the undefined-behaviour checks only (`SAN` in the `Makefile` and
  `fw/build.sh`).
- **Missing Linux commands:** `host/twin.sh` falls back to `perl`'s alarm when there is no `timeout`. In
  `fw/build.sh`, `stat -c` becomes `wc -c`. `host/twin.c` includes `unistd.h` for `_exit`.
- **A test bug:** in `tests/sound_test.c`, `pitch_of` read past the end of its buffer, so it rendered 72 blocks
  instead of 64. On Linux the stray memory happened to be harmless; on a Mac it failed the GRAIN pitch check.

**The name:**
- **Start screen:** the sailboat logo is drawn in as before, then **TEACHIE PEACHIE** is written in under it, on two lines, with
  "WITH SLOOP 3" below that (`app/vis.c`). The twin's start scenario saves a picture of it,
  `boot-name.ppm` (`host/twin.c`).
- **Installer page:** `web/installer.tmpl.html` stays exactly SLOOP's, so a merge never clashes there. During the
  build, `tools/teachie_page.py` rewrites the finished page (`fw/build.sh` runs it after `mkpage.py`):
  - Teachie Peachie wherever the page names the firmware it installs, with SLOOP 3 credited in the title, the beta
    note and the footer.
  - Bug reports point to Teachie Peachie, not to SLOOP's tracker.
  - It shows version `0.1 · SLOOP 3.3.7`, with Teachie Peachie's version set at the top of the script.
  - If SLOOP rewords the page, the script names each sentence it no longer finds. It stops the build if the
    name or the credit to SLOOP would be missing.
  - `tests/install_sim.js` expects the new "restarts on Teachie Peachie" message.

**Not changed yet:**
- **The USB MIDI name, "FM-1 SLOOP 3".** The installer and SLOOP's web editor recognise the FM-1 by that name.
- **The firmware's own version, `VERSION` "3.3.7" in `fw/main.c`.** The editor reads it.
