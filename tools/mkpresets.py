#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Writes app/preset_data.c: the factory sounds. SLOOP 2.8.5's 153 sounds as they are (docs/sloop/facts.json, read from
# SLOOP's sources by tools/sloopfacts.py: their values, not SLOOP's code), for each engine SLOOP 3 has; then the drum
# kits. The gains (all as loud) come from tools/levels.txt, made by build/level_tool (make levels). Also the map from
# SLOOP's sounds (its engine, its place in that engine's list) to ours, for the import of SLOOP's projects.
#   tools/mkpresets.py app/preset_data.c
import json, os, sys

HERE = os.path.dirname(os.path.abspath(__file__))
facts = json.load(open(os.path.join(HERE, '..', 'docs', 'sloop', 'facts.json'), encoding='utf-8'))
# SLOOP's engines (its order) as SLOOP 3's: None = not written yet (its sounds wait)
ENG = ['ENG_VA', 'ENG_FM', 'ENG_PD', 'ENG_LOFI', 'ENG_SAMPLE', 'ENG_VOICE', 'ENG_TRIO', 'ENG_WHEEL', 'ENG_GRAIN', 'ENG_FM6', 'ENG_PHYS', 'ENG_NOISE']
KIND = {'BASS': 'K_BASSE', 'KEYS': 'K_KEYS', 'ORGAN': 'K_ORGUE', 'PAD': 'K_PAD', 'LEAD': 'K_LEAD', 'PLUCK': 'K_PLUCK',
        'STAB': 'K_BRASS', 'FX': 'K_FX'}
KINDS = ['BASS', 'KEYS', 'ORGAN', 'PAD', 'LEAD', 'PLUCK', 'STAB', 'VOICE', 'FX', 'KIT']
# SLOOP's track parameters (P_*) as ours (engine.h SP_*, presets.h PX_*)
SP = {'P_ATK': 'SP_ATK', 'P_DEC': 'SP_DEC', 'P_SUS': 'SP_SUS', 'P_REL': 'SP_REL', 'P_ED_FLT': 'SP_EFLT', 'P_ED_PIT': 'SP_EPIT',
      'P_ED_SHP': 'SP_ESHP', 'P_ED_FX': 'SP_TRIM', 'P_LRATE': 'SP_LRATE', 'P_LWAVE': 'SP_LWAVE', 'P_LPHASE': 'SP_LPHASE',
      'P_LFADE': 'SP_LFADE', 'P_LD_PIT': 'SP_LPIT', 'P_LD_FLT': 'SP_LFLT', 'P_LD_SHP': 'SP_LSHP', 'P_LD_AMP': 'SP_LAMP',
      'P_LSYNC': 'SP_LSYNC', 'P_LTRIG': 'SP_LTRIG', 'P_LPOL': 'SP_LPOL', 'P_ESYNC': 'SP_ESYNC', 'P_VOICE': 'SP_VOICE',
      'P_GLIDE': 'SP_GLIDE', 'P_GLMODE': 'SP_GLMODE', 'P_PRIO': 'SP_PRIO', 'P_ALLOC': 'SP_ALLOC', 'P_DETUNE': 'SP_DETUNE',
      'P_FTYPE': 'SP_FTYPE', 'P_DIST': 'SP_DIST', 'P_ITYPE': 'SP_ITYPE', 'P_IA': 'SP_IA', 'P_IB': 'SP_IB', 'P_IC': 'SP_IC',
      'P_IMIX': 'SP_IMIX', 'P_SLCR': 'SP_SLCR', 'P_SLPAT': 'SP_SLPAT', 'P_SLRATE': 'SP_SLRATE', 'P_SLDEPTH': 'SP_SLDEPTH',
      'P_TRANS': 'PX_TRANS'}
kind_of = {(k[1], k[2]): k[0] for k in facts['kinds']}

gains = {}
lv = os.path.join(HERE, 'levels.txt')
if os.path.exists(lv):
    for line in open(lv, encoding='utf-8'):
        if '\t' in line: n, g = line.rstrip('\n').split('\t'); gains[n] = float(g)

rows, smap = [], [[] for _ in range(12)]
for e in facts['engines']:
    for k, p in enumerate(e['presets']):
        if ENG[e['index']] is None: smap[e['index']].append(0xFF); continue
        smap[e['index']].append(len(rows))
        rows.append((p['name'], ENG[e['index']], KIND[kind_of.get((e['index'], p['name']), 'FX')], p))
# the drum kits, SLOOP's: its General MIDI sampled kit and four treatments, then its 32 synthesised kits (E0: the kit)
KITS = facts['drum_kits'][:5] + [k['name'] for k in facts['synth_kits']]

def c(v): return str(int(v))
out = ['/* SPDX-License-Identifier: GPL-3.0-only',
       ' * The factory sounds (presets.h), written by tools/mkpresets.py (edit the script, not this file): SLOOP 2.8.5\'s',
       ' * sounds, their values as SLOOP has them (docs/sloop/facts.json), then the drum kits. The gain makes them all as',
       ' * loud (tools/levels.txt, from build/level_tool). */',
       '#include "presets.h"', '',
       'const char *const gb_kind_name[K_N] = { ' + ', '.join('"%s"' % k for k in KINDS) + ' };',
       'const gb_preset gb_presets[] = {']
for name, eng, kind, p in rows:
    fx = p['fx'] or [0, 0, 0, 0]
    arp = [a + 1 for a in p['arp']] if p['arp'] else [0, 0, 0, 0]
    xs = []
    lead = kind == 'K_LEAD'   # (4.0.10, Sam: the leads POLY, to play chords; the basses stay MONO)
    for k, v in p['x']:
        if k in SP and not (lead and k == 'P_VOICE'): xs += [SP[k], c(v)]
    xs = (xs + ['-1'] * 8)[:8]
    g = gains.get(name, 1.0)
    out.append('    { %-14s %s, %s, { %s }, { %s }, %d, %d, { %s }, { %s }, { %s }, %d },' % (
        '"%s",' % name, eng, kind, ', '.join(map(c, p['e'])), ', '.join(map(c, p['env'])), p['fenv'], 0 if lead else p['mono'],
        ', '.join(map(c, fx)), ', '.join(map(c, arp)), ', '.join(xs), round(g * 1000)))
for k, name in enumerate(KITS):
    out.append('    { %-14s ENG_DRUM, K_KIT, { %d, 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0 }, 0, 0, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, '
               '{ -1, -1, -1, -1, -1, -1, -1, -1 }, 1000 },' % ('"%s",' % name, k))
# SLICE's sounds (not in SLOOP 2.8.5's release: ours, after everything else, so no sound's number moves)
SLICES = [('BREAK 16', [0, 2, 0, 0, 0, 0, 127, 127], [0, 127, 127, 30], [0, 0, 0, 12]),
          ('CHOP 8', [0, 1, 0, 0, 1, 0, 90, 110], [0, 127, 127, 12], [10, 0, 20, 10]),
          ('REVERSE', [0, 2, 0, -2, 0, 1, 127, 100], [0, 127, 127, 30], [0, 0, 30, 30]),
          ('USR SLICE', [1, 4, 0, 0, 0, 0, 127, 127], [0, 127, 127, 30], [0, 0, 0, 12])]
for name, e, env, fx in SLICES:
    out.append('    { %-14s ENG_SLICE, K_FX, { %s }, { %s }, 0, 0, { %s }, { 0, 0, 0, 0 }, { -1, -1, -1, -1, -1, -1, -1, -1 }, %d },' % (
        '"%s",' % name, ', '.join(map(c, e)), ', '.join(map(c, env)), ', '.join(map(c, fx)), round(gains.get(name, 1.0) * 1000)))
out.append('};')
out.append('const int gb_npresets = (int)(sizeof gb_presets / sizeof gb_presets[0]);')
out.append('/* SLOOP\'s sounds, engine by engine in its order: ours (0xFF: not here) */')
out.append('const uint8_t gb_sloop_n[12] = { %s };' % ', '.join(str(len(m)) for m in smap))
out.append('const uint8_t gb_sloop_eng[12] = { %s };' % ', '.join(e if e else '0xFF' for e in ENG))
out.append('const uint8_t gb_sloop_map[12][32] = {')
for m in smap:
    out.append('    { %s },' % ', '.join(str(v) for v in m) if m else '    { 0xFF },')
out.append('};')
open(sys.argv[1], 'w', encoding='utf-8').write('\n'.join(out) + '\n')
print(len(rows), 'sounds +', len(KITS), 'kits', file=sys.stderr)
