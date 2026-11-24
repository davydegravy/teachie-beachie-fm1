#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# The firmware's RAM image packed for the flash (3.2): one LZ4 block (the LZ4 block format: token, literals, a 16-bit
# offset, the match's length), found with hash chains and a lazy step; fw/unpack.c unpacks it into RAM at the start.
# The file: [the unpacked length, u32][two sums of its words, u32 each: fw/unpack.c][the block].
# Written for this project.   lz4pack.py IN OUT   (prints the sizes; unpacks what it made and compares)
import sys

MINM, WINDOW, CHAIN = 4, 65535, 96

def pack(src):
    n = len(src); out = bytearray(); head = {}; prev = [0] * n
    def find(i):
        best_len, best_off = 0, 0
        if i + MINM > n - 5: return 0, 0
        key = src[i:i + 4]; j = head.get(key, -1); tries = CHAIN
        limit = n - 5                                   # (the last 5 bytes are literals: the format's rule)
        while j >= 0 and i - j <= WINDOW and tries:
            if src[j + best_len:j + best_len + 1] == src[i + best_len:i + best_len + 1]:
                l = 0
                while i + l < limit and src[j + l] == src[i + l]: l += 1
                if l > best_len: best_len, best_off = l, i - j
            j = prev[j] - 1; tries -= 1
        return (best_len, best_off) if best_len >= MINM else (0, 0)
    def insert(i):
        if i + 4 <= n:
            key = src[i:i + 4]; prev[i] = head.get(key, -1) + 1; head[key] = i
    def emit(lit, mlen, off):
        ll = len(lit); ml = mlen - MINM if mlen else 0
        out.append((min(ll, 15) << 4) | (min(ml, 15) if mlen else 0))
        if ll >= 15:
            r = ll - 15
            while r >= 255: out.append(255); r -= 255
            out.append(r)
        out.extend(lit)
        if mlen:
            out.append(off & 255); out.append(off >> 8)
            if ml >= 15:
                r = ml - 15
                while r >= 255: out.append(255); r -= 255
                out.append(r)
    i = 0; anchor = 0
    while i < n:
        l, o = find(i)
        if l:
            insert(i)
            l2, o2 = find(i + 1)                        # lazy: a longer match one byte on wins
            if l2 > l + 1: i += 1; continue
            emit(src[anchor:i], l, o)
            for k in range(i + 1, i + l): insert(k)
            i += l; anchor = i
        else:
            insert(i); i += 1
    emit(src[anchor:], 0, 0)
    return bytes(out)

# (4.0.3) the optimal parse: the cheapest cover of the image by literals and matches (a match: its token, its offset,
# its length's bytes; a literal: its byte), the matches of every length each place can take, found on the hash chains;
# the smaller of it and the lazy one is kept. The same LZ4 block: fw/unpack.c unchanged. (~700 bytes less, 4.0.3)
def pack_opt(src):
    n = len(src); limit = n - 5; mstart_max = n - 12; CH = 128
    head = {}; prev = [0] * n
    INF = 1 << 60
    cost = [INF] * (n + 1); frm = [0] * (n + 1); cost[0] = 0   # frm: >0 match len (with off in fo), 0 literal
    fo = [0] * (n + 1)
    def mc(l): return 3 if l <= 18 else 4 + (l - 19) // 255
    for i in range(n):
        c = cost[i]
        if cost[i + 1] > c + 1: cost[i + 1] = c + 1; frm[i + 1] = 0
        if i + 4 <= n:
            key = src[i:i + 4]
            j = head.get(key, -1)
            if i <= mstart_max:
                best = MINM - 1; tries = CH
                while j >= 0 and i - j <= WINDOW and tries:
                    if src[j + best:j + best + 1] == src[i + best:i + best + 1] and i + best < limit:
                        l = 0
                        while i + l + 32 <= limit and src[j + l:j + l + 32] == src[i + l:i + l + 32]: l += 32
                        while i + l < limit and src[j + l] == src[i + l]: l += 1
                        if l > best:
                            off = i - j
                            for L in range(max(best + 1, MINM), min(l, 18) + 1):
                                v = c + 3
                                if v < cost[i + L]: cost[i + L] = v; frm[i + L] = L; fo[i + L] = off
                            for L in (l, l - 1, l - 2):
                                if L > 18 and L > best:
                                    v = c + mc(L)
                                    if v < cost[i + L]: cost[i + L] = v; frm[i + L] = L; fo[i + L] = off
                            best = l
                            if i + l >= limit: break
                    j = prev[j] - 1; tries -= 1
            prev[i] = head.get(key, -1) + 1; head[key] = i
    # backtrack
    seq = []; p = n
    while p > 0:
        L = frm[p]
        if L: seq.append(('m', p - L, L, fo[p])); p -= L
        else: seq.append(('l', p - 1)); p -= 1
    seq.reverse()
    out = bytearray(); anchor = 0
    def emit(lit, mlen, off):
        ll = len(lit); ml = mlen - MINM if mlen else 0
        out.append((min(ll, 15) << 4) | (min(ml, 15) if mlen else 0))
        if ll >= 15:
            r = ll - 15
            while r >= 255: out.append(255); r -= 255
            out.append(r)
        out.extend(lit)
        if mlen:
            out.append(off & 255); out.append(off >> 8)
            if ml >= 15:
                r = ml - 15
                while r >= 255: out.append(255); r -= 255
                out.append(r)
    for s in seq:
        if s[0] == 'm':
            _, at, L, off = s
            emit(src[anchor:at], L, off); anchor = at + L
    emit(src[anchor:], 0, 0)
    return bytes(out)

def unpack(b, size):
    out = bytearray(); s = 0
    while True:
        t = b[s]; s += 1; n = t >> 4
        if n == 15:
            while True:
                x = b[s]; s += 1; n += x
                if x != 255: break
        out += b[s:s + n]; s += n
        if s >= len(b): break
        off = b[s] | b[s + 1] << 8; s += 2
        n = t & 15
        if n == 15:
            while True:
                x = b[s]; s += 1; n += x
                if x != 255: break
        n += 4
        for _ in range(n): out.append(out[-off])
    assert len(out) == size
    return bytes(out)

if __name__ == "__main__":
    src = open(sys.argv[1], "rb").read()
    src += bytes(-len(src) % 4)
    p = pack(src); q = pack_opt(src)
    if len(q) < len(p): p = q
    assert unpack(p, len(src)) == src, "the packed image does not unpack to itself"
    import struct
    words = struct.unpack("<%dI" % (len(src) // 4), src)
    a = b = 0
    for w in words: a = (a * 0x01000193 + w) & 0xFFFFFFFF; a ^= a >> 15; b = ((b ^ a) * 0x9E3779B1) & 0xFFFFFFFF
    head = struct.pack("<III", len(src), a, b)      # (fw/unpack.c: the length; its two sums of the words)
    open(sys.argv[2], "wb").write(head + p)
    print(f"{sys.argv[2]}: {len(src)} bytes packed into {len(p)} ({100 * len(p) // len(src)} %)")
