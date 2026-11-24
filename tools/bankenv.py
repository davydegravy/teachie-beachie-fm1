#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# (3.4.2, issue #14: "the vibes ring much longer above F#4 than below") a set's zones made to die away as one of them
# does: each zone's level over time (30 ms windows, from its peak) brought down to the reference zone's where it is
# above it (never raised), the gain smoothed; the zone re-encoded (IMA ADPCM, as tools/mksamples.py), its length kept.
# The bank's seek points and CRC written again.   tools/bankenv.py IN OUT SET REF_ZONE ZONE [ZONE...]   (zones: in the set)
import struct, sys, zlib
import numpy as np
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
def decode(data, off, n):
    st = (0, 0); x = np.zeros(n)
    for k in range(n):
        b = data[off + (k >> 1)]; st = step(st, b >> 4 if k & 1 else b & 15); x[k] = st[0]
    return x
def encode(q):
    st = (0, 0); nib = []
    for s in q:
        pr, ix = st; d = int(s) - pr; c = 0
        if d < 0: c = 8; d = -d
        t = STEP[ix]
        if d >= t: c |= 4; d -= t
        if d >= t >> 1: c |= 2; d -= t >> 1
        if d >= t >> 2: c |= 1
        nib.append(c); st = step(st, c)
    if len(nib) & 1: nib.append(0)
    return bytes(nib[i] | nib[i + 1] << 4 for i in range(0, len(nib), 2))
def env(x, rate):
    W = max(8, int(rate * 0.03)); return np.sqrt(np.convolve(x * x, np.ones(W) / W, 'same')) + 1e-6
src, dst, SET, REF = sys.argv[1], sys.argv[2], sys.argv[3].encode(), int(sys.argv[4]); ZS = [int(a) for a in sys.argv[5:]]
b = bytearray(open(src, 'rb').read())
magic, ver, ns, nz, r0, doff, dlen, crc, nseek, r1 = struct.unpack_from('<IHHHHIIIII', b, 0)
sets = [struct.unpack_from('<8sHHB3x', b, 32 + 16 * i) for i in range(ns)]
z0 = next(s[1] for s in sets if s[0].rstrip(b'\0') == SET)
zo = 32 + 16 * ns
zones = [struct.unpack_from('<IIIIIhhBBBB', b, zo + 28 * i) for i in range(nz)]
data = bytearray(b[doff:doff + dlen])
def zx(i): z = zones[z0 + i]; return decode(data, z[0], z[1]), z[4] * 44100.0 / 65536.0
xr, rr = zx(REF); er = env(xr, rr); pr = int(np.argmax(er)); er /= er[pr]
for i in ZS:
    x, r = zx(i); e = env(x, r); p = int(np.argmax(e)); en = e / e[p]
    t = (np.arange(len(x)) - p) / r                     # (seconds from each one's peak)
    tgt = np.interp(t * rr + pr, np.arange(len(er)), er, left=1.0, right=er[-1])
    g = np.minimum(1.0, tgt / en); g[:p] = 1.0
    W = max(8, int(r * 0.02)); g = np.convolve(np.pad(g, (W, W), 'edge'), np.ones(2 * W + 1) / (2 * W + 1), 'valid')
    y = np.clip(np.round(x * g), -32768, 32767)
    z = zones[z0 + i]; enc = encode(y); assert len(enc) == (z[1] + 1) // 2
    data[z[0]:z[0] + len(enc)] = enc
    print(f'{SET.decode()} zone {i}: brought to zone {REF}\'s decay (at most {20 * np.log10(g.min() + 1e-9):.1f} dB)', file=sys.stderr)
seek = []
for z in zones:
    st = (0, 0)
    for k in range(z[1]):
        if k % 1024 == 0: seek.append((st[0] & 0xFFFF) | st[1] << 16)
        bb = data[z[0] + (k >> 1)]; st = step(st, bb >> 4 if k & 1 else bb & 15)
assert len(seek) == nseek
so = zo + 28 * nz + 4 * nz
for k, v in enumerate(seek): struct.pack_into('<I', b, so + 4 * k, v)
b[doff:doff + dlen] = data
struct.pack_into('<I', b, 20, zlib.crc32(bytes(data)) & 0xFFFFFFFF)
open(dst, 'wb').write(bytes(b))
