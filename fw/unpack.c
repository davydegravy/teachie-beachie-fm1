/* SPDX-License-Identifier: GPL-3.0-only
 * 3.2: the firmware's RAM image, packed in the flash (tools/lz4pack.py: one LZ4 block), unpacked into RAM at the
 * start. Called by fw/xip.S from the flash window, on m-bootloader's stack, interrupts off. Bytes only: nothing here
 * reads or writes a word off its boundary, calls anything, or keeps data outside its registers and the stack.
 * The packed image: [the unpacked length, u32][two sums of the unpacked image's words, u32 each][the block]. The
 * sums: a, multiplied, the word added, its high bits folded down; b, a mixed in and multiplied, at each word: only
 * multiplies, adds, shifts and exclusive ors, which the firmware has always used. (Plain sums of the words
 * let a damaged image through: a byte's top bit wrong in a literal that a match copies an even number of words
 * further is the same error twice, and cancels: tests/unpack_test.c found one in 4000.)
 * Returns 0 when the image in RAM has the length and the sum it should; anything else: fw/xip.S goes back to
 * m-bootloader (its menu) instead of running a damaged image.
 * Written for this project; the format is the LZ4 block format's. */
#include <stdint.h>
#ifndef RAM_AT
#define RAM_AT 0x01C30000u          /* (tests/unpack_test.c gives its own) */
#endif

#ifdef __MACH__                     /* (tests/unpack_test.c on a Mac: Mach-O names its sections segment,section) */
#define STUBC __attribute__((used))
#else
#define STUBC __attribute__((section(".stubc"), used))
#endif
STUBC int xip_unpack(const uint8_t *s, const uint8_t *end)
{
    uint32_t len = (uint32_t)s[0] | (uint32_t)s[1] << 8 | (uint32_t)s[2] << 16 | (uint32_t)s[3] << 24;
    uint32_t want = (uint32_t)s[4] | (uint32_t)s[5] << 8 | (uint32_t)s[6] << 16 | (uint32_t)s[7] << 24;
    uint32_t want2 = (uint32_t)s[8] | (uint32_t)s[9] << 8 | (uint32_t)s[10] << 16 | (uint32_t)s[11] << 24;
    uint8_t *const d0 = (uint8_t *)(uintptr_t)RAM_AT, *d = d0, *const dend = d0 + len;
    s += 12;
    for (;;) {
        uint32_t t = *s++, n = t >> 4;
        if (n == 15u) { uint32_t b; do { b = *s++; n += b; } while (b == 255u); }
        if (n > (uint32_t)(dend - d)) return 1;
        if (s > end || n > (uint32_t)(end - s)) return 5;      /* (3.3: never past the image's end: a cut image refused) */
        while (n) { *d++ = *s++; n--; }
        if (s >= end || d >= dend) break;
        if (end - s < 2) return 6;                               /* (its offset: two bytes) */
        uint32_t off = (uint32_t)s[0] | (uint32_t)s[1] << 8; s += 2;
        n = t & 15u;
        if (n == 15u) { uint32_t b; do { b = *s++; n += b; } while (b == 255u); }
        n += 4u;
        if (!off || off > (uint32_t)(d - d0) || n > (uint32_t)(dend - d)) return 2;
        const uint8_t *m = d - off;
        while (n) { *d++ = *m++; n--; }
    }
    if (d != dend) return 3;
    uint32_t a = 0, b = 0;
    for (const uint32_t *w = (const uint32_t *)(uintptr_t)RAM_AT, *we = w + len / 4u; w < we; w++) { a = a * 0x01000193u + *w; a ^= a >> 15; b = (b ^ a) * 0x9E3779B1u; }
    return a == want && b == want2 ? 0 : 4;
}
