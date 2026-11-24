#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# (3.4.2) the sample bank's unlooped zones without their last 60 dB: each one ends where it last rises above its peak
# - 60 dB (what is cut is under the noise of the FM-1's output), the bytes after it gone; the loops, the seek points
# (every 1024 samples) and the CRC as tools/mksamples.py writes them. ~3 KB: the firmware's room before the bank.
#   tools/banktrim.py IN OUT [DB]
import struct, sys, zlib
STEP = [7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166,
        1282, 1411, 1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845,
        8630, 9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767]
IDX = [-1, -1, -1, -1, 2, 4, 6, 8]
def step(st, c):
    pr, ix = st; s = STEP[ix]; vd = s >> 3
    if c & 4: vd += s
    if c & 2: vd += s >> 1
    if c & 1: vd += s >> 2
    pr = max(-32768, min(32767, pr - vd if c & 8 else pr + vd)); return pr, max(0, min(88, ix + IDX[c & 7]))
def walk(data, off, n):                          # each sample, and the decoder's state before it
    st = (0, 0)
    for k in range(n):
        b = data[off + (k >> 1)]; yield k, st; st = step(st, b >> 4 if k & 1 else b & 15)
src, dst = sys.argv[1], sys.argv[2]; DB = float(sys.argv[3]) if len(sys.argv) > 3 else 60.0
b = open(src, 'rb').read()
magic, ver, ns, nz, r0, doff, dlen, crc, nseek, r1 = struct.unpack_from('<IHHHHIIIII', b, 0)
assert magic == 0x42533353 and ver == 4
sets = b[32:32 + 16 * ns]
zo = 32 + 16 * ns
zones = [list(struct.unpack_from('<IIIIIhhBBBB', b, zo + 28 * i)) for i in range(nz)]
data = b[doff:doff + dlen]
new = bytearray(); moved = {}; cut = 0
for z in zones:
    off, n, ls, le = z[0], z[1], z[2], z[3]
    if off in moved: z[0], z[1], z[2], z[3] = moved[off]; continue
    keep = n
    if DB >= 0 and (ls == le == n - 1 or ls >= n - 1):   # unlooped: the tail under -DB goes (DB < 0: kept, a check)
        pk, last = 0, 0
        vals = [st for k, st in walk(data, off, n)]
        amp = [abs(step(vals[k], data[off + (k >> 1)] >> 4 if k & 1 else data[off + (k >> 1)] & 15)[0]) for k in range(n)]
        pk = max(amp) or 1
        th = pk * 10 ** (-DB / 20)
        last = max((k for k in range(n) if amp[k] > th), default=n - 1) + 1
        keep = max(2, min(n, (last + 1) & ~1))
    nb = (keep + 1) // 2
    no = len(new); new += data[off:off + nb]
    if len(new) & 1: new += b'\0'
    nls, nle = (keep - 1, keep - 1) if keep < n else (ls, le)
    moved[off] = (no, keep, nls, nle); z[0], z[1], z[2], z[3] = no, keep, nls, nle
    cut += n - keep
seek_first, seek = [], []
for z in zones:
    seek_first.append(len(seek))
    for k, st in walk(new, z[0], z[1]):
        if k % 1024 == 0: seek.append((st[0] & 0xFFFF) | st[1] << 16)
head = (32 + 16 * ns + 28 * nz + 4 * nz + 4 * len(seek) + 255) & ~255   # (the data on 256 bytes, as tools/mksamples.py)
body = sets + b''.join(struct.pack('<IIIIIhhBBBB', *z) for z in zones) + b''.join(struct.pack('<I', x) for x in seek_first) + \
       b''.join(struct.pack('<I', x) for x in seek)
hdr = struct.pack('<IHHHHIIIII', magic, ver, ns, nz, r0, head, len(new), zlib.crc32(bytes(new)) & 0xFFFFFFFF, len(seek), r1)
out = hdr + body + b"\xff" * (head - 32 - len(body)) + bytes(new)
open(dst, 'wb').write(out)
print(f'bank: {len(b)} -> {len(out)} bytes ({cut} samples under -{DB:g} dB cut)', file=sys.stderr)
