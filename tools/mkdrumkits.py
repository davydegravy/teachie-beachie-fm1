#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Writes engine/drumkits.c: SLOOP 2.8.5's 32 synthesised drum kits (docs/sloop/facts.json synth_kits: each of their
# 16 sounds in musical units, Hz, ms, dB, as SLOOP's sources write them, with SLOOP's measured level trims) in our
# own table (engine/drum.h gb_dsnd), in the order of our 16 drum keys. The facts used here besides the kits:
#  - our keys play SLOOP's 16 lanes as its drum track does (KICK, KICK 2, SNARE, CLAP, HAT, OPEN HAT, PEDAL, RIM,
#    SNARE 2, LOW TOM, HI TOM, CRASH, RIDE, SHAKER, CONGA, COWBELL): each a General MIDI note, and that note a sound
#    of the synthesised kit (the pedal hat: the closed hat a semitone down);
#  - its times are kept as SLOOP keeps them (5 ms .. 4 s on 0..127, to -60 dB), its cutoffs on the cutoff curve
#    (30 Hz .. 16 kHz on 0..127), its levels in quarter dB;
#  - SLOOP 2.7's "kicks on a small speaker": the sine and triangle kicks get at least a drive of 36, a fall of 18
#    semitones over 34 ms, and 0.6 of their click.
#   tools/mkdrumkits.py engine/drumkits.c
import json, math, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
facts = json.load(open(os.path.join(HERE, '..', 'docs', 'sloop', 'facts.json'), encoding='utf-8'))
LANES = facts['drum_lanes']            # SLOOP's synthesised kit: its 16 sounds, in its order
# our 16 keys: SLOOP's synthesised sound and a semitone offset
OURS = [('KICK', 0), ('KICK2', 0), ('SNARE', 0), ('CLAP', 0), ('CHH', 0), ('OHH', 0), ('CHH', -1), ('RIM', 0),
        ('SNARE2', 0), ('TOMLO', 0), ('TOMHI', 0), ('CRASH', 0), ('RIDE', 0), ('SHAKER', 0), ('CONGA', 0), ('COWBELL', 0)]
WAVE = {None: 0, 'sine': 1, 'tri': 2, 'square': 3, 'fm': 4, 'bell': 5}
SRC = {None: 0, 'white': 1, 'metal': 2, 'cym': 3, 'chip': 4, 'clap': 0x11}
FMODE = {None: 0, 'lp': 1, 'bp': 2, 'hp': 3}
STEPS_PER_OCT = 127 / math.log2(16000 / 30)

def tidx(ms): ms = max(5.0, min(4000.0, ms)); return int(round(127 * math.log(ms / 5.0) / math.log(800)))
def cidx(hz): hz = max(30.0, min(16000.0, hz)); return int(round(127 * math.log(hz / 30.0) / math.log(16000 / 30)))
def n16(hz): return int(round((69 + 12 * math.log2(hz / 440.0)) * 16))

def punch(lane, s):
    s = dict(s)
    if lane in ('KICK', 'KICK2') and s['wave'] in ('sine', 'tri'):
        s['drive'] = max(s['drive'], 36); s['bend'] = max(s['bend'], 18); s['bt'] = max(s['bt'], 34)
        s['click'] = int(s['click'] * 0.6)
    return s

def enc(s, semi):
    flt, fcut = 0, 127
    if s['filt']:
        mode, hz, res = s['filt'][:3]
        flt = FMODE[mode] | (4 if 'all' in s['filt'][3:] else 0) | (int(round(res * 31)) << 3)
        fcut = cidx(hz)
    q = int(round(4 * (s['level'] + s.get('trim', 0.0))))
    v = [WAVE[s['wave']], SRC[s['src']], n16(s['hz']) + 16 * semi, int(s['bend']), tidx(s['bt']), min(255, int(round(s['hold'] / 2))),
         tidx(s['decay']), int(s['tlev']) if s['wave'] else 0, int(round(s['t2'] * 32)) if s['t2'] else 0, int(s['t2lev']),
         int(s['click']), int(s['nlev']) if s['src'] else 0, min(255, int(round(s['nhold'] / 2))), tidx(s['ndec']), flt, fcut,
         int(round(s['fenv'] * STEPS_PER_OCT)), 0 if s['hpf'] is None else cidx(s['hpf']),
         0 if not s['chip'] else max(0, min(127, int(round(69 + 12 * math.log2(s['chip'] / 440.0))))), int(s['drive']),
         max(-128, min(127, q))]
    assert 0 <= v[2] < 65536 and -128 <= v[16] <= 127, v
    return v

out = ['/* SPDX-License-Identifier: GPL-3.0-only',
       ' * SLOOP 2.8.5\'s synthesised drum kits, in our keys\' order (engine/drum.h), written by tools/mkdrumkits.py from',
       ' * docs/sloop/facts.json (edit the script, not this file). Each different sound once (gb_dsnds; 3.1.6: the kits share',
       ' * many), each kit its 16 by number. Read only while the flash window is on (engine/drum.c). */',
       '#include "drum.h"', '']
# (3.4.1, Sam: "the instruments are not all as loud") tools/drumtrims.txt: a kit's key brought near the same key of
# the other kits (build/level_tool drumtrims: K-weighted); a trim added to SLOOP's own
TRIM = {}
tp = os.path.join(HERE, 'drumtrims.txt')
if os.path.exists(tp):
    for line in open(tp, encoding='utf-8'):
        if line.startswith('#') or '\t' not in line: continue
        kn, r, db = line.rstrip('\n').split('\t'); TRIM[(kn, int(r))] = float(db)
uniq, idx, kits = [], {}, []
for k in facts['synth_kits']:
    lanes = dict(zip(LANES, k['lanes'])); si = []
    for r, (name, semi) in enumerate(OURS):
        s = dict(punch(name, lanes[name])); s['trim'] = s.get('trim', 0.0) + TRIM.get((k['name'], r), 0.0)
        v = tuple(enc(s, semi))
        if v not in idx: idx[v] = len(uniq); uniq.append(v)
        si.append(idx[v])
    kits.append((k, si))
out.append('const gb_dsnd gb_dsnds[%d] = {' % len(uniq))
for v in uniq: out.append('    { %s },' % ', '.join(str(x) for x in v))
out.append('};')
out.append('const gb_dkit gb_dkits[GB_NDKITS] = {')
for k, si in kits: out.append('    { "%s", "%s", 0x%02X, { %s } },' % (k['name'], k['style'], k['crush'], ', '.join(str(i) for i in si)))
out.append('};')
names = facts['drum_kits'][:5] + [k['name'] for k in facts['synth_kits']] + facts['drum_kits'][5:]
out.append('const char *const gb_dkit_names[GB_NKITS] = { %s };' % ', '.join('"%s"' % n for n in names))
assert len(facts['synth_kits']) == 32 and len(names) == 46, len(names)
open(sys.argv[1], 'w', encoding='utf-8').write('\n'.join(out) + '\n')
print('drum kits: %d synthesised' % len(facts['synth_kits']), file=sys.stderr)
