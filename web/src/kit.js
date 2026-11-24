/* (4.0.3, Sam: "the editor as SLOOP 2.8.5 to make a drum kit with samples") A KIT FROM YOUR SAMPLES, the ways of
 * SLOOP 2.8.5's Drum kit page (its GUIDE, section 23), written again for this editor:
 *   1 your sounds: 16 pads (KICK .. COWBELL), a WAV dropped on its pad, or files sorted onto the pads by their names
 *     (kick, bd, snare, hh, open, crash...; a second kick to KICK 2), or a ZIP; each pad: PITCH (+-12 semitones, the
 *     sound played faster or slower, as a sampler), GAIN (0 .. -24 dB), LENGTH (cut, a 4 ms fade); MATCH LEVELS (as the
 *     ear hears: K-weighted, the loud ones down to the quietest); the time used; FIT THE SLOT
 *   2 where it goes: USR3+4 (two slots, ~15 s: the pads shared out, each whole in one) or one slot USR1-4 (~7 s), what
 *     it holds now and what a send replaces
 *   3 send, then the drum track on it; the kit as a ZIP of WAVs, and back
 * A pad is a zone at its lane's GM note (engine/drum.c usr_zone), 22.05 kHz IMA ADPCM, as SLOOP 2.8 writes them. */
words({
  tab_mkkit: 'Kit from samples', mkTitle: 'A drum kit from your samples (as SLOOP 2.8.5)',
  mkHelp: '1. Drop a WAV on a pad, or CHOOSE FILES (or a ZIP): the files go onto the pads by their names (kick, bd, snare, hh, open, crash...; a second kick to KICK 2). Click a wave to hear it. PITCH plays it faster or slower, GAIN turns it down, LENGTH cuts it (a short fade). MATCH LEVELS sets every GAIN so the kit is heard even, then adjust by ear.\n2. Where it goes: USR3+4 (about 15 s over two slots: the editor shares the pads out) or one slot (about 7 s). Too long? Shorten the long sounds or press FIT THE SLOT.\n3. Name it, SEND, then THE DRUM TRACK ON IT (on the FM-1: SOUNDS > YOUR KITS). The closed hat cuts the open one. SAVE AS ZIP keeps the 16 WAVs.',
  mkStep1: '1 · your sounds', mkStep2: '2 · where it goes', mkStep3: '3 · send it', mkAdd: 'Choose files…', mkClear: 'Clear all', mkZip: '⇩ Save as ZIP', mkOpenZip: '⇧ Open ZIP',
  mkMatch: 'Match levels', mkFit: 'Fit the slot', mkEmpty: 'drop a WAV\nor click', mkPitch: 'pitch', mkGain: 'gain', mkLen: 'length', mkName: 'name',
  mkSend: 'Send to {x}', mkUse: 'The drum track on {x}', mkUsed: '{s} s of {t} s', mkOver: 'too long', mkBig: 'two slots, about 15 s', mkOne: 'one slot, about 7 s',
  mkReplaces: 'A send replaces: {x}.', mkSure: 'Write the kit in {x} on the FM-1? What is there now is replaced.', mkSent: 'Kit sent: {x}.', mkUsing: 'The drum track plays {x}.',
  mkPlaced: 'On the pads: ', mkFull: 'every pad is taken', mkMatched: 'Levels matched: adjust by ear.', mkNone: 'No sound on the pads.', mkOld: 'This FM-1 cannot play USR3+4: install 4.0.3.'
}, {
  tab_mkkit: 'Kit d\'échantillons', mkTitle: 'Un kit de batterie avec tes échantillons (comme SLOOP 2.8.5)',
  mkHelp: '1. Dépose un WAV sur un pad, ou CHOISIR DES FICHIERS (ou un ZIP) : les fichiers vont sur les pads d\'après leur nom (kick, bd, snare, hh, open, crash... ; un 2e kick sur KICK 2). Clique une forme d\'onde pour l\'entendre. PITCH le joue plus vite ou plus lentement, GAIN le baisse, LENGTH le coupe (avec un court fondu). ÉGALISER LES NIVEAUX règle chaque GAIN pour que le kit s\'entende égal, puis ajuste à l\'oreille.\n2. Où il va : USR3+4 (environ 15 s sur deux cases : l\'éditeur répartit les pads) ou une case (environ 7 s). Trop long ? Raccourcis les sons longs ou appuie sur AJUSTER À LA CASE.\n3. Nomme-le, ENVOIE, puis LA PISTE DE BATTERIE DESSUS (sur le FM-1 : SOUNDS > YOUR KITS). Le charley fermé coupe l\'ouvert. ENREGISTRER EN ZIP garde les 16 WAV.',
  mkStep1: '1 · tes sons', mkStep2: '2 · où il va', mkStep3: '3 · l\'envoyer', mkAdd: 'Choisir des fichiers…', mkClear: 'Tout effacer', mkZip: '⇩ Enregistrer en ZIP', mkOpenZip: '⇧ Ouvrir un ZIP',
  mkMatch: 'Égaliser les niveaux', mkFit: 'Ajuster à la case', mkEmpty: 'dépose un WAV\nou clique', mkPitch: 'pitch', mkGain: 'gain', mkLen: 'durée', mkName: 'nom',
  mkSend: 'Envoyer dans {x}', mkUse: 'La batterie sur {x}', mkUsed: '{s} s sur {t} s', mkOver: 'trop long', mkBig: 'deux cases, environ 15 s', mkOne: 'une case, environ 7 s',
  mkReplaces: 'Un envoi remplace : {x}.', mkSure: 'Écrire le kit dans {x} sur le FM-1 ? Ce qui y est maintenant est remplacé.', mkSent: 'Kit envoyé : {x}.', mkUsing: 'La piste de batterie joue {x}.',
  mkPlaced: 'Sur les pads : ', mkFull: 'tous les pads sont pris', mkMatched: 'Niveaux égalisés : ajuste à l\'oreille.', mkNone: 'Aucun son sur les pads.', mkOld: 'Ce FM-1 ne joue pas USR3+4 : installe la 4.0.3.'
});
const KN = ['KICK', 'KICK 2', 'SNARE', 'CLAP', 'HAT', 'OPEN HAT', 'PEDAL', 'RIM', 'SNARE 2', 'LOW TOM', 'HI TOM', 'CRASH', 'RIDE', 'SHAKER', 'CONGA', 'COWBELL'];
const KKEY = ['F3', 'G3', 'A3', 'B3', 'C4', 'D4', 'E4', 'F4', 'G4', 'A4', 'B4', 'C5', 'D5', 'E5', 'F5', 'G5'];
const KR = 22050, KFADE = Math.round(KR * 0.004), KROOM = 0x14000 - 512 - 64, KPAIR = 4, KGMIN = -24;
const KD = { name: 'KIT', dest: KPAIR, pads: Array(16).fill(null), slots: null, ctx: null, src: null, ui: null };
const kSlots = (d = KD.dest) => (d === KPAIR ? [2, 3] : [d]);
const kDest = (d = KD.dest) => (d === KPAIR ? 'USR3+4' : 'USR' + (d + 1));
const kRate = p => Math.pow(2, (p.tune || 0) / 12);
/* the samples a pad keeps after its pitch and its cut (len: samples after the pitch, 0 all) */
const kLen = p => { const full = p.tune ? Math.floor((p.x.length - 1) / kRate(p)) : p.x.length; return Math.max(1, Math.min(full, p.len > 0 ? Math.round(p.len) : full)); };
const kBytes = pads => pads.reduce((a, p) => a + (p ? (kLen(p) + 1) >> 1 : 0), 0);
/* a pad as it goes: pitched (played faster or slower), cut and faded out (4 ms), its gain */
async function kRender(p, gain = p.gain) {
  let x = p.x;
  if (p.tune) {
    const n = Math.max(1, Math.floor((x.length - 1) / kRate(p))), oc = new OfflineAudioContext(1, n, KR), b = oc.createBuffer(1, x.length, KR); b.copyToChannel(x, 0);
    const s = oc.createBufferSource(); s.buffer = b; s.playbackRate.value = kRate(p); s.connect(oc.destination); s.start(); x = (await oc.startRendering()).getChannelData(0);
  }
  const n = Math.min(x.length, kLen(p)), g = Math.pow(10, clamp(gain || 0, KGMIN, 0) / 20), o = new Float32Array(n), fd = n < x.length ? Math.min(KFADE, n) : 0;
  for (let i = 0; i < n; i++) o[i] = x[i] * g * (fd && i >= n - fd ? (n - i) / fd : 1);
  return o;
}
/* a sound's loudness as the ear hears it (dB): K-weighted, its loudest 100 ms */
function kLoud(y0) {
  const bq = (G, Q, fc, shelf) => { const A = Math.pow(10, G / 40), w = 2 * Math.PI * fc / KR, c = Math.cos(w), al = Math.sin(w) / (2 * Q), r = 2 * Math.sqrt(A) * al;
    const k = shelf ? [A * ((A + 1) + (A - 1) * c + r), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - r), (A + 1) - (A - 1) * c + r, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - r]
      : [(1 + c) / 2, -(1 + c), (1 + c) / 2, 1 + al, -2 * c, 1 - al];
    return k.map(v => v / k[3]); };
  const y = Float64Array.from(y0);
  for (const [b0, b1, b2, , a1, a2] of [bq(4, 0.7072, 1682, true), bq(0, 0.5003, 38.1, false)]) {
    let x1 = 0, x2 = 0, y1 = 0, y2 = 0; for (let i = 0; i < y.length; i++) { const x = y[i], v = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2; x2 = x1; x1 = x; y2 = y1; y1 = v; y[i] = v; }
  }
  const W = Math.round(KR * 0.1), hop = W >> 2; let best = 1e-12;
  for (let a = 0; a < Math.max(1, y.length - W + hop); a += hop) { let e = 0; for (let i = a; i < Math.min(y.length, a + W); i++) e += y[i] * y[i]; best = Math.max(best, e / W); }
  return 10 * Math.log10(best);
}
/* the longest pads shortened to fit (room: bytes); a pair: no pad longer than a slot, and the pads must share out */
function kFit(pads, room = KROOM, bins = 1) {
  if (bins > 1) {
    let ch = false;
    for (const p of pads) if (p && ((kLen(p) + 1) >> 1) > room) { p.len = room * 2 - 1; ch = true; }
    for (let r = room * 2; !kSplit(pads, room) && r > 64; r = Math.floor(r * 0.98)) { kFit(pads, r); ch = true; }
    return ch;
  }
  const use = pads.map((p, l) => (p ? { l, n: kLen(p) } : null)).filter(Boolean), sum = M => use.reduce((a, u) => a + ((Math.min(u.n, M) + 1) >> 1), 0);
  if (sum(Infinity) <= room) return false;
  let lo = 1, hi = Math.max(...use.map(u => u.n));
  while (lo < hi) { const M = (lo + hi + 1) >> 1; if (sum(M) <= room) lo = M; else hi = M - 1; }
  for (const u of use) if (u.n > lo) pads[u.l].len = lo;
  return true;
}
/* USR3+4: the pads over two slots, each whole in one (the biggest first, into the emptier that takes it); null: no fit */
function kSplit(pads, room = KROOM) {
  const it = pads.map((p, l) => (p ? { l, b: (kLen(p) + 1) >> 1 } : null)).filter(Boolean).sort((a, b) => b.b - a.b || a.l - b.l), bins = [[], []], used = [0, 0];
  for (const i of it) { const k = (used[0] <= used[1] ? [0, 1] : [1, 0]).find(j => used[j] + i.b <= room); if (k === undefined) return null; bins[k].push(i.l); used[k] += i.b; }
  return bins.map(b => b.sort((a, c) => a - c));
}
/* a file's name -> its pad (-1: none) */
const KWORDS = [[5, /open|\boh\b|ophat|o ?hat/], [6, /pedal|\bph\b|foot/], [4, /clos|\bch\b|\bhh\b|hi ?hat|hihat|\bhat/], [7, /\brim|\brs\b|side ?stick|stick/],
  [3, /clap|\bcp\b|\bclp\b|snap/], [1, /kick ?2|\bbd ?2|sub ?kick/], [0, /kick|\bbd\b|bass ?drum|\bkik\b|\bkck\b/], [8, /snare ?2|\bsd ?2|snr ?2/], [2, /snare|\bsd\b|\bsnr\b/],
  [10, /hi(gh)? ?tom|\bht\b|tom ?h|mid ?tom|\bmt\b|tom ?[23]\b/], [9, /lo(w)? ?tom|\blt\b|floor|tom ?l|tom ?1\b|\btom/], [11, /crash|\bcr\b|\bcy\b|cymbal|splash|china/],
  [12, /ride|\brd\b/], [13, /shak|\bsh\b|maraca|cabasa|tamb|\bshk\b/], [14, /conga|bongo|\bcg\b|\bperc|timbal|djembe/], [15, /cow ?bell|\bcb\b|bell|block|clave|agogo/]];
const KALT = { 0: 1, 1: 0, 2: 8, 8: 2, 3: 8, 4: 6, 6: 4, 5: 4, 7: 3, 9: 10, 10: 9, 11: 12, 12: 11, 13: 14, 14: 13, 15: 14 };
function kLaneOf(name) { const n = String(name).toLowerCase().replace(/\.[a-z0-9]{2,4}$/, '').replace(/[_\-.()[\]]+/g, ' '); for (const [l, re] of KWORDS) if (re.test(n)) return l; return -1; }
function kPlace(pads, name, want = -1) {
  if (want >= 0) return want;
  const l = kLaneOf(name);
  if (l >= 0 && !pads[l]) return l;
  if (l >= 0 && !pads[KALT[l]]) return KALT[l];
  return pads.findIndex(p => !p);
}
/* ZIP: stored files out; stored or deflated in */
function zipOut(files) {
  const enc = new TextEncoder(), loc = [], cen = []; let off = 0;
  for (const f of files) {
    const nm = enc.encode(f.name), c = crc32(f.data), h = new Uint8Array(30 + nm.length), dv = new DataView(h.buffer);
    dv.setUint32(0, 0x04034B50, true); dv.setUint16(4, 20, true); dv.setUint32(14, c, true); dv.setUint32(18, f.data.length, true); dv.setUint32(22, f.data.length, true); dv.setUint16(26, nm.length, true); h.set(nm, 30);
    const e = new Uint8Array(46 + nm.length), de = new DataView(e.buffer);
    de.setUint32(0, 0x02014B50, true); de.setUint16(4, 20, true); de.setUint16(6, 20, true); de.setUint32(16, c, true); de.setUint32(20, f.data.length, true); de.setUint32(24, f.data.length, true);
    de.setUint16(28, nm.length, true); de.setUint32(42, off, true); e.set(nm, 46);
    loc.push(h, f.data); cen.push(e); off += h.length + f.data.length;
  }
  const cl = cen.reduce((a, e) => a + e.length, 0), end = new Uint8Array(22), dz = new DataView(end.buffer);
  dz.setUint32(0, 0x06054B50, true); dz.setUint16(8, files.length, true); dz.setUint16(10, files.length, true); dz.setUint32(12, cl, true); dz.setUint32(16, off, true);
  const all = [...loc, ...cen, end], o = new Uint8Array(all.reduce((a, b) => a + b.length, 0)); let p = 0; for (const b of all) { o.set(b, p); p += b.length; } return o;
}
async function zipIn(b) {
  const v = new DataView(b.buffer, b.byteOffset, b.byteLength); let e = b.length - 22;
  while (e >= 0 && v.getUint32(e, true) !== 0x06054B50) e--;
  if (e < 0) throw new Error('not a ZIP');
  const n = v.getUint16(e + 10, true), out = []; let c = v.getUint32(e + 16, true);
  for (let k = 0; k < n; k++) {
    if (v.getUint32(c, true) !== 0x02014B50) throw new Error('bad ZIP');
    const m = v.getUint16(c + 10, true), size = v.getUint32(c + 20, true), nl = v.getUint16(c + 28, true), xl = v.getUint16(c + 30, true), cl = v.getUint16(c + 32, true), off = v.getUint32(c + 42, true);
    const name = new TextDecoder().decode(b.subarray(c + 46, c + 46 + nl)), lo = off + 30 + v.getUint16(off + 26, true) + v.getUint16(off + 28, true), raw = b.subarray(lo, lo + size);
    if (!name.endsWith('/')) {
      if (m === 0) out.push({ name, data: raw.slice() });
      else if (m === 8 && typeof DecompressionStream !== 'undefined') out.push({ name, data: new Uint8Array(await new Response(new Blob([raw]).stream().pipeThrough(new DecompressionStream('deflate-raw'))).arrayBuffer()) });
    }
    c += 46 + nl + xl + cl;
  }
  return out;
}
function kWav(x) {
  const n = x.length, f = new Uint8Array(44 + 2 * n), dv = new DataView(f.buffer), s = (o, t) => { for (let i = 0; i < t.length; i++) f[o + i] = t.charCodeAt(i); };
  s(0, 'RIFF'); dv.setUint32(4, 36 + 2 * n, true); s(8, 'WAVEfmt '); dv.setUint32(16, 16, true); dv.setUint16(20, 1, true); dv.setUint16(22, 1, true);
  dv.setUint32(24, KR, true); dv.setUint32(28, KR * 2, true); dv.setUint16(32, 2, true); dv.setUint16(34, 16, true); s(36, 'data'); dv.setUint32(40, 2 * n, true);
  for (let i = 0; i < n; i++) dv.setInt16(44 + 2 * i, clamp(Math.round(x[i] * 32767), -32768, 32767), true);
  return f;
}
/* ---------------------------------------------------------------- the files onto the pads */
async function kAdd(list, lane = -1) {
  const done = []; let full = false;
  for (const f of list) {
    if (/\.zip$/i.test(f.name)) { await kOpenZip(new Uint8Array(await f.arrayBuffer()), f.name); continue; }
    try {
      const x = await decodeTo(f, KR, true), l = kPlace(KD.pads, f.name, lane);
      if (l < 0) { full = true; break; }
      KD.pads[l] = { x, fname: f.name, tune: 0, gain: 0, len: 0 }; done.push(KN[l] + ' ← ' + f.name); lane = -1;
    } catch (e) { log(L('noAudio') + f.name, 'bad'); }
  }
  if (done.length) log(L('mkPlaced') + done.join(', ') + (full ? ' · ' + L('mkFull') : ''), 'ok'); else if (full) log(L('mkFull'), 'bad');
  drawMkKit();
}
async function kOpenZip(b, name) {
  try {
    const fs = (await zipIn(b)).filter(x => /\.(wav|aif|aiff|mp3|flac|ogg)$/i.test(x.name));
    KD.pads = Array(16).fill(null);
    await kAdd(fs.map(x => new File([x.data], x.name.split('/').pop())));
    const base = String(name || '').replace(/\.zip$/i, '').replace(/-drumkit$/i, '').toUpperCase().replace(/[^A-Z0-9 ]/g, '').slice(0, 8); if (base) KD.name = base;
    drawMkKit();
  } catch (e) { fail(e); }
}
async function kPlay(l) {
  const p = KD.pads[l]; if (!p) return;
  if (!KD.ctx) KD.ctx = new AudioContext();
  if (KD.src) try { KD.src.stop(); } catch (e) {}
  const x = await kRender(p), b = KD.ctx.createBuffer(1, Math.max(1, x.length), KR); b.copyToChannel(x, 0);
  const s = KD.ctx.createBufferSource(); s.buffer = b; s.connect(KD.ctx.destination); s.start(); KD.src = s;
}
async function kMatch() {
  const Ls = await Promise.all(KD.pads.map(async p => (p ? kLoud(await kRender(p, 0)) : null))), q = Ls.filter(v => v !== null);
  if (q.length < 2) return;
  const tg = Math.max(Math.min(...q), Math.max(...q) + KGMIN);
  KD.pads.forEach((p, l) => { if (p) p.gain = clamp(Math.round(tg - Ls[l]), KGMIN, 0); });
  log(L('mkMatched'), 'ok'); drawMkKit();
}
const kOver = () => (KD.dest === KPAIR ? !kSplit(KD.pads) : kBytes(KD.pads) > KROOM);
/* ---------------------------------------------------------------- to the FM-1 */
async function kReadSlots() { try { const b = await ask(ED.SAMPLE, [0]); KD.slots = []; for (let i = 0; i < b[1]; i++) { const o = 3 + 11 * i; KD.slots.push({ nz: b[o], name: str(b, o + 1, 8) }); } } catch (e) { KD.slots = null; } }
async function kWriteSlot(k, lanes) {
  const zones = [], blob = []; let used = 0;
  for (const l of lanes) {
    const x = await kRender(KD.pads[l]), n = x.length, { data, st } = ima(x, n - 1), off = used;
    blob.push(data); used += data.length; if (used & 1) { blob.push(new Uint8Array(1)); used++; }
    zones.push({ off, n, ls: n - 1, le: n - 1, rate: 32768, root16: GM_IN[l] * 16, pred: st[0], idx: st[1], lo: GM_IN[l], hi: GM_IN[l] });
  }
  const data = new Uint8Array(used); let o = 0; for (const b of blob) { data.set(b, o); o += b.length; }
  const h = new Uint8Array(480), dv = new DataView(h.buffer);
  h.set([0x46, 0x53, 0x4D, 0x50]); dv.setUint16(4, 1, true); h[6] = zones.length;
  const nm = (KD.name || 'KIT').toUpperCase().slice(0, 8); for (let i = 0; i < nm.length; i++) h[8 + i] = nm.charCodeAt(i) & 127;
  dv.setUint32(16, data.length, true); dv.setUint32(20, crc32(data), true);
  zones.forEach((z, i) => { const b = 32 + 28 * i; dv.setUint32(b, z.off, true); dv.setUint32(b + 4, z.n, true); dv.setUint32(b + 8, z.ls, true); dv.setUint32(b + 12, z.le, true);
    dv.setUint32(b + 16, z.rate, true); dv.setInt16(b + 20, z.root16, true); dv.setInt16(b + 22, z.pred, true); h[b + 24] = z.idx; h[b + 25] = z.lo; h[b + 26] = z.hi; h[b + 27] = 0; });
  await retryBusy(() => ask(ED.SAMPLE, [1, k]), 80);
  for (let off = 0; off < data.length; off += 384) { await ask(ED.SAMPLE, [2, k, ...w32(off), ...data.slice(off, off + 384)]); if (KD.ui) KD.ui.prog.textContent = 'USR' + (k + 1) + ' ' + Math.floor(off * 100 / data.length) + ' %'; }
  await ask(ED.SAMPLE, [3, k, ...h]);
}
async function kSend() {
  if (!KD.pads.some(Boolean)) { log(L('mkNone'), 'bad'); return; }
  const ks = kSlots(), parts = ks.length > 1 ? kSplit(KD.pads) : [KD.pads.map((p, l) => (p ? l : -1)).filter(l => l >= 0)];
  if (!parts || kOver()) { log(L('mkOver'), 'bad'); return; }
  if (!confirm(L('mkSure').replace('{x}', ks.map(k => 'USR' + (k + 1)).join(' + ')))) return;
  liveBusy++;
  try {
    for (let j = 0; j < ks.length; j++) {
      if (parts[j].length) await kWriteSlot(ks[j], parts[j]);
      else await retryBusy(() => ask(ED.SAMPLE, [4, ks[j]]), 80);          /* (a slot of the pair without a pad: emptied, nothing old plays) */
    }
    log(L('mkSent').replace('{x}', kDest()), 'ok');
  } catch (e) { fail(e); } finally { liveBusy--; if (KD.ui) KD.ui.prog.textContent = ''; }
  await kReadSlots(); drawMkKit();
}
async function kUse() {
  const v = UK_BASE + KIT_USR + KD.dest;
  try { await retryBusy(() => live(10, DRUM_T, v & 255, v >> 8), 80); await sleep(150);
    const st = await live(0); if (KD.dest === KPAIR && st && st.t && (st.t[DRUM_T].preset & 0x7FFF) !== v) { log(L('mkOld'), 'bad'); return; }
    log(L('mkUsing').replace('{x}', kDest()), 'ok'); } catch (e) { fail(e); }
}
async function kZip() {
  const files = [];
  for (let l = 0; l < 16; l++) if (KD.pads[l]) files.push({ name: (KD.name || 'KIT') + '/' + String(l + 1).padStart(2, '0') + ' ' + KN[l] + '.wav', data: kWav(await kRender(KD.pads[l])) });
  if (files.length) save(zipOut(files), (KD.name || 'kit').toLowerCase().replace(/\s+/g, '-') + '-drumkit.zip', 'application/zip');
}
/* ---------------------------------------------------------------- the page */
function kWave(cv, p) {
  const w = cv.width = Math.max(60, cv.clientWidth || 150), h = cv.height = 40, g = cv.getContext('2d'), x = p.x, n = x.length, keep = kLen(p) * kRate(p);
  g.clearRect(0, 0, w, h);
  for (let i = 0; i < w; i++) { const a = Math.floor(i / w * n), b = Math.max(a + 1, Math.floor((i + 1) / w * n)); let pk = 0; for (let j = a; j < b && j < n; j += 2) pk = Math.max(pk, Math.abs(x[j]));
    const y = Math.max(1, Math.round(pk * (h / 2 - 2))); g.fillStyle = a < keep ? TCOL[DRUM_T] : '#3a3450'; g.fillRect(i, h / 2 - y, 1, 2 * y); }
}
function drawMkKit() {
  const P = $('tab_mkkit'); if (!P) return; P.innerHTML = '';
  const c = card(P, L('mkTitle'), '', L('mkHelp'), 'wide'), u = KD.ui = {};
  const fi = el('input'); fi.type = 'file'; fi.multiple = true; fi.accept = '.wav,.zip,audio/*'; fi.style.display = 'none';
  fi.onchange = () => { const w = u.want ?? -1; u.want = -1; kAdd([...fi.files], w); };
  const pickFor = l => { u.want = l; fi.value = ''; fi.click(); };
  /* 1: the sounds */
  c.appendChild(el('h3', '', L('mkStep1')));
  const nb = KD.dest === KPAIR ? 2 : 1, bytes = kBytes(KD.pads), secs = KD.pads.reduce((a, p) => a + (p ? kLen(p) : 0), 0) / KR, over = kOver();
  const r1 = el('div', 'tools'), meter = el('progress'); meter.max = KROOM * nb; meter.value = Math.min(bytes, KROOM * nb); meter.style.width = '160px';
  const mt = el('span', over ? 'bad' : 'dim', (over ? L('mkOver') + ' · ' : '') + L('mkUsed').replace('{s}', secs.toFixed(1)).replace('{t}', (KROOM * nb * 2 / KR).toFixed(1)));
  const oz = button(L('mkOpenZip'), () => pick((b, n) => kOpenZip(b, n), '.zip'), 'sm');
  const zb = button(L('mkZip'), kZip, 'sm'); zb.disabled = !KD.pads.some(Boolean);
  r1.append(button(L('mkAdd'), () => pickFor(-1), 'pri'), fi, oz, zb, button(L('mkClear'), () => { KD.pads = Array(16).fill(null); drawMkKit(); }, 'red sm'), meter, mt);
  c.appendChild(r1);
  const g = el('div', ''); g.style.cssText = 'display:grid;grid-template-columns:repeat(auto-fill,minmax(170px,1fr));gap:6px';
  for (let l = 0; l < 16; l++) {
    const p = KD.pads[l], b = el('div', 'pgroup'); b.style.cssText = 'border-top:3px solid ' + (p ? TCOL[DRUM_T] : 'var(--line)') + ';min-height:118px';
    const hd = el('div', 'row'); hd.append(el('b', '', KN[l]), el('span', 'dim', KKEY[l])); hd.lastChild.style.marginLeft = 'auto'; b.appendChild(hd);
    if (!p) { const e = el('div', 'dim', L('mkEmpty')); e.style.cssText = 'white-space:pre-line;text-align:center;padding:22px 0;cursor:pointer;font-size:12px'; e.onclick = () => pickFor(l); b.appendChild(e); }
    else {
      const cv = el('canvas'); cv.style.cssText = 'width:100%;height:40px;cursor:pointer;display:block;margin:3px 0'; cv.title = '▶'; cv.onclick = () => kPlay(l); b.appendChild(cv); setTimeout(() => kWave(cv, p), 0);
      const fn = el('div', 'dim', p.fname); fn.style.cssText = 'font-size:11px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis'; fn.title = p.fname; b.appendChild(fn);
      const full = kLen({ ...p, len: 0 }), maxMs = Math.max(10, Math.ceil(full / KR * 1000 / 5) * 5);
      const sl = (lab, mn, mx, st, val, fmt, set) => { const r = el('div', 'row'), i = el('input'); i.type = 'range'; i.min = mn; i.max = mx; i.step = st; i.value = val; i.style.cssText = 'flex:1;min-width:60px';
        const v = el('span', 'dim', fmt(val)); v.style.cssText = 'font-size:11px;width:44px;text-align:right'; i.oninput = () => { set(+i.value); v.textContent = fmt(+i.value); kWave(cv, p); kInfo(); }; i.onchange = () => kPlay(l);
        r.append(el('span', 'dim', lab), i, v); r.firstChild.style.cssText = 'font-size:11px;width:40px'; return r; };
      b.append(sl(L('mkPitch'), -12, 12, 1, p.tune, v => (v > 0 ? '+' : '') + v + ' st', v => { p.tune = v; }),
        sl(L('mkGain'), KGMIN, 0, 1, p.gain, v => v + ' dB', v => { p.gain = v; }),
        sl(L('mkLen'), 10, maxMs, 5, p.len ? Math.round(p.len / KR * 1000) : maxMs, v => (v >= maxMs ? full / KR : v / 1000).toFixed(2) + ' s', v => { p.len = v >= maxMs ? 0 : Math.round(v / 1000 * KR); }));
      const ac = el('div', 'row'); ac.append(button('▶', () => kPlay(l), 'sm'), button('×', () => { KD.pads[l] = null; drawMkKit(); }, 'sm red')); b.appendChild(ac);
    }
    b.addEventListener('dragover', e => { e.preventDefault(); b.style.outline = '2px solid var(--acc)'; });
    b.addEventListener('dragleave', () => { b.style.outline = ''; });
    b.addEventListener('drop', e => { e.preventDefault(); e.stopPropagation(); b.style.outline = ''; if (e.dataTransfer.files.length) kAdd([...e.dataTransfer.files], l); });
    g.appendChild(b);
  }
  c.appendChild(g);
  const r2 = el('div', 'tools'), mb = button(L('mkMatch'), kMatch, 'sm'), fb = button(L('mkFit'), () => { if (kFit(KD.pads, KROOM, nb)) drawMkKit(); }, 'sm');
  mb.disabled = KD.pads.filter(Boolean).length < 2; fb.disabled = !over; r2.append(mb, fb); c.appendChild(r2);
  /* 2: where it goes */
  c.appendChild(el('h3', '', L('mkStep2')));
  const ds = el('div', 'slots');
  for (const d of [KPAIR, 0, 1, 2, 3]) {
    const s = el('div', 'slot' + (d === KD.dest ? ' on' : '')); s.style.cursor = 'pointer'; s.onclick = () => { KD.dest = d; drawMkKit(); };
    const now = kSlots(d).map(k => KD.slots && KD.slots[k]).filter(x => x && x.nz).map(x => x.name), un = [...new Set(now)];
    s.append(el('b', '', kDest(d) + ' '), el('span', 'dim', d === KPAIR ? L('mkBig') : L('mkOne')), el('br'), el('span', un.length ? 'warn' : 'dim', un.length ? un.join(' + ') : (KD.slots ? L('emptySlot') : '')));
    ds.appendChild(s);
  }
  c.appendChild(ds);
  const rep = kSlots().map(k => KD.slots && KD.slots[k]).filter(x => x && x.nz).map(x => x.name);
  if (rep.length) c.appendChild(el('div', 'warn', L('mkReplaces').replace('{x}', [...new Set(rep)].join(' + '))));
  const r3 = el('div', 'tools'), nm = el('input', 'txt'); nm.maxLength = 8; nm.value = KD.name; nm.style.width = '110px';
  nm.oninput = () => { const v = nm.value.toUpperCase().replace(/[^A-Z0-9 .+-]/g, '').slice(0, 8); if (v !== nm.value) nm.value = v; KD.name = v; };
  r3.append(el('span', 'dim', L('mkName')), nm); c.appendChild(r3);
  /* 3: send it */
  c.appendChild(el('h3', '', L('mkStep3')));
  const r4 = el('div', 'tools'), sb = button(L('mkSend').replace('{x}', kDest()), kSend, 'pri'), ub = button(L('mkUse').replace('{x}', kDest()), kUse);
  sb.disabled = !KD.pads.some(Boolean) || over; ub.disabled = !kSlots().some(k => KD.slots && KD.slots[k] && KD.slots[k].nz);
  u.prog = el('span', 'dim'); r4.append(sb, ub, u.prog); c.appendChild(r4);
  c.addEventListener('dragover', e => e.preventDefault());
  c.addEventListener('drop', e => { e.preventDefault(); if (e.dataTransfer.files.length) kAdd([...e.dataTransfer.files]); });
  u.meter = meter; u.mt = mt; u.fb = fb; u.sb = sb; u.nb = nb;
  if (!KD.slots && info) kReadSlots().then(() => { if (KD.slots) drawMkKit(); });
}
/* the meter while a slider moves (the page drawn again only when it is let go) */
function kInfo() {
  const u = KD.ui; if (!u || !u.meter) return;
  const bytes = kBytes(KD.pads), secs = KD.pads.reduce((a, p) => a + (p ? kLen(p) : 0), 0) / KR, over = kOver();
  u.meter.value = Math.min(bytes, u.meter.max); u.mt.className = over ? 'bad' : 'dim';
  u.mt.textContent = (over ? L('mkOver') + ' · ' : '') + L('mkUsed').replace('{s}', secs.toFixed(1)).replace('{t}', (KROOM * u.nb * 2 / KR).toFixed(1));
  u.fb.disabled = !over; u.sb.disabled = !KD.pads.some(Boolean) || over;
}
