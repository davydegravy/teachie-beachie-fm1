/* Sequencer: a pattern of a track drawn (a piano roll; the drums a grid of 16 sounds), its notes edited (moved,
 * lengthened, velocity, condition, ratchet, accent, slide, nudge), its locks as a lane, its track's DIV GATE SWING
 * QUANT and length, tools (copy, paste, shift, transpose, double, quantize, humanize, undo), MIDI files in and out.
 * Song: the song's lines. ED_PATTERN (wide), ED_NOTES2, ED_LOCKS, ED_SONG, ED_LIVE op 16. */
words({
  seqHelp: 'Click an empty place: a note (drums: a hit). Drag a note: move it; its right edge: its length. Shift-click: add to the selection; drag on empty ground with Shift: a box. Right-click or double-click a note: delete. Delete: the selection out.\nThe lane under the notes: velocity (drag the bars). Under it: the locks of the value chosen (drag: set; right-click: remove).\nEverything is sent to the FM-1 as you go; ↶ undoes here (on the FM-1, EDIT + OCT−).',
  playing: 'playing', follow: 'follow', steps: 'steps', grid: 'grid', len: 'length', vel: 'velocity', lockOf: 'locks of', none: '-',
  note: 'Note', cond: 'condition', ratchet: 'ratchet', accent: 'ACCENT', slide: 'SLIDE', nudge: 'nudge', sel: ' selected', noSel: 'nothing selected',
  clear: 'CLEAR', copyP: 'COPY', pasteP: 'PASTE', left: '◀ step', right: 'step ▶', tUp: '+1', tDn: '−1', oUp: '+12', oDn: '−12', double: 'DOUBLE', quant: 'QUANTIZE', human: 'HUMANIZE', all: 'ALL', undo: '↶', redo: '↷',
  trkSet: 'Track', trkHelp: 'DIV: how long a step is, for this track. GATE: the notes\' length (100 %: as written). SWING: this track\'s own. QUANT: how much what you play is put on the steps.',
  midiIn: 'MIDI file in', midiOut: 'MIDI file out', midiHelp: 'IN: a .mid file\'s notes into this pattern (the first track with notes; the drums from channel 10 by their GM notes). OUT: the patterns of every track as a .mid file (the drums on channel 10).',
  midiPat: 'patterns ', midiBad: 'Not a MIDI file (or no notes in it).', midiGot: ' notes read into ', tooMany: ' (128 at most: the rest left out)',
  song: 'Song', songHelp: 'Each line: from that bar, these patterns, these tracks muted. LENGTH: the song\'s bars (then it loops). READ takes the FM-1\'s song; SEND writes it there (kept with the project once saved).\nSONG plays it (or the loop again); SONG REC records the patterns you play into it, from the next bar.',
  songRead: 'READ', songSend: 'SEND', songLen: 'LENGTH (bars)', addLine: '+ line', bar: 'bar', songSent: 'Song sent: ', lines: ' lines'
}, {
  seqHelp: 'Clic sur une place vide : une note (batterie : un coup). Glisse une note : la déplacer ; son bord droit : sa longueur. Maj-clic : ajouter à la sélection ; glisser sur le vide avec Maj : un cadre. Clic droit ou double-clic sur une note : la supprimer. Suppr : la sélection enlevée.\nLa bande sous les notes : la vélocité (glisse les barres). En dessous : les verrous de la valeur choisie (glisser : poser ; clic droit : enlever).\nTout est envoyé au FM-1 au fur et à mesure ; ↶ annule ici (sur le FM-1, EDIT + OCT−).',
  playing: 'qui joue', follow: 'suivre', steps: 'pas', grid: 'grille', len: 'longueur', vel: 'vélocité', lockOf: 'verrous de', none: '-',
  note: 'Note', cond: 'condition', ratchet: 'ratchet', accent: 'ACCENT', slide: 'SLIDE', nudge: 'décalage', sel: ' sélectionnée(s)', noSel: 'rien de sélectionné',
  clear: 'VIDER', copyP: 'COPIER', pasteP: 'COLLER', left: '◀ pas', right: 'pas ▶', tUp: '+1', tDn: '−1', oUp: '+12', oDn: '−12', double: 'DOUBLER', quant: 'QUANTIFIER', human: 'HUMANISER', all: 'TOUT', undo: '↶', redo: '↷',
  trkSet: 'Piste', trkHelp: 'DIV : la durée d\'un pas, pour cette piste. GATE : la longueur des notes (100 % : telle qu\'écrite). SWING : celui de la piste. QUANT : combien ce que tu joues est posé sur les pas.',
  midiIn: 'Fichier MIDI entrant', midiOut: 'Fichier MIDI sortant', midiHelp: 'ENTRANT : les notes d\'un fichier .mid dans ce pattern (la première piste qui a des notes ; la batterie depuis le canal 10 par ses notes GM). SORTANT : les patterns de toutes les pistes en fichier .mid (la batterie sur le canal 10).',
  midiPat: 'patterns ', midiBad: 'Pas un fichier MIDI (ou pas de notes dedans).', midiGot: ' notes lues dans ', tooMany: ' (128 au plus : le reste laissé)',
  song: 'Song', songHelp: 'Chaque ligne : à partir de cette mesure, ces patterns, ces pistes muettes. LONGUEUR : les mesures de la song (puis elle boucle). LIRE prend la song du FM-1 ; ENVOYER l\'y écrit (gardée avec le projet une fois sauvé).\nSONG la joue (ou de nouveau la boucle) ; SONG REC y enregistre les patterns que tu joues, à partir de la mesure suivante.',
  songRead: 'LIRE', songSend: 'ENVOYER', songLen: 'LONGUEUR (mesures)', addLine: '+ ligne', bar: 'mesure', songSent: 'Song envoyée : ', lines: ' lignes'
});
const R = { t: 0, pt: 255, follow: true, P: null, lo: 48, rows: 30, sel: new Set(), grid: 96, len: 96, vel: 100, lockP: 48, drag: null, hist: [], fut: [], clip: null, d: null };
const RH = () => R.t === DRUM_T ? 22 : 15, VH = 64, KH = 64, X0 = 58;
const drumT = () => R.t === DRUM_T;
async function rollLoad() {
  if (!info) return;
  try {
    R.P = await patRead(R.t, R.pt); R.sel.clear();
    if (drumT()) { R.lo = 0; R.rows = 16; }
    else { R.rows = 30; if (R.P.notes.length) { const mn = Math.min(...R.P.notes.map(n => n.note)), mx = Math.max(...R.P.notes.map(n => n.note)); R.lo = clamp(Math.round((mn + mx) / 2) - 15, 0, 127 - R.rows + 1); } }
    try { R.d = await getDesc(LV ? LV.t[R.t].eng : (await getSound(R.t)).e); } catch (e) { R.d = null; }
    drawSeqTools(); drawRoll();
  } catch (e) { fail(e); }
}
function snapshot() { R.hist.push(JSON.stringify({ n: R.P.notes, l: R.P.locks, s: R.P.steps })); if (R.hist.length > 40) R.hist.shift(); R.fut = []; }
function restore(s) { const o = JSON.parse(s); R.P.notes = o.n; R.P.locks = o.l; R.P.steps = o.s; R.sel.clear(); drawRoll(); rollSend(true); }
let rollT = 0;
function rollSend(locks) {
  clearTimeout(rollT);
  rollT = setTimeout(async () => { try { await patWrite(R.t, R.P.pt, R.P.steps, R.P.notes); if (locks && P2()) await locksWrite(R.t, R.P.pt, R.P.locks); } catch (e) { fail(e); } }, 200);
}
function drawSeq() {
  const P = $('tab_seq'); P.innerHTML = '';
  const top = el('div', 'row'); top.style.marginBottom = '8px';
  for (const t of SHOWN) { const b = button(trkName(t), () => { R.t = t; R.hist = []; R.fut = []; drawSeq(); rollLoad(); }, t === R.t ? 'on' : ''); b.style.borderColor = TCOL[t]; top.appendChild(b); }
  top.appendChild(el('span', 'grow'));
  const ps = select([[255, L('playing')], ...[...Array(8)].map((_, i) => [i, L('pat') + (i + 1)])], R.pt, v => { R.pt = +v; rollLoad(); });
  const fl = el('label', 'dim'); const fc = el('input'); fc.type = 'checkbox'; fc.checked = R.follow; fc.onchange = () => R.follow = fc.checked; fl.append(fc, ' ' + L('follow'));
  top.append(ps, fl, button('↻', rollLoad, 'sm'));
  P.appendChild(top);
  const c = card(P, L('tab_seq'), '', L('seqHelp'), 'wide');
  R.toolsEl = el('div'); c.appendChild(R.toolsEl);
  const wrap = el('div'); wrap.style.position = 'relative'; const cv = el('canvas', 'roll'); wrap.appendChild(cv); c.appendChild(wrap); R.cv = cv;
  R.insp = el('div', 'tools'); R.inspKey = null; c.appendChild(R.insp);
  rollEvents(cv);
  const cards = el('div', 'cards'); cards.style.marginTop = '12px'; P.appendChild(cards);
  const tc = card(cards, L('trkSet'), '', L('trkHelp')); R.trkEl = el('div', 'knobs'); tc.appendChild(R.trkEl);
  const mc = card(cards, L('midiIn') + ' / ' + L('midiOut'), '', L('midiHelp'));
  const mp = select([[255, L('playing')], ...[...Array(8)].map((_, i) => [i, L('pat') + (i + 1)])], 255);
  const mr = el('div', 'row'); mr.append(button(L('midiIn'), () => pick(midiImport, '.mid,.midi,audio/midi')), button(L('midiOut'), () => midiFile(+mp.value)), el('span', 'dim', L('midiPat')), mp); mc.appendChild(mr);
  drawSeqTools(); drawRoll(); drawTrackSet();
}
function drawSeqTools() {
  const T = R.toolsEl; if (!T) return; T.innerHTML = '';
  const row = el('div', 'tools');
  const st = el('input', 'num'); st.type = 'number'; st.min = 1; st.max = 128; st.value = R.P ? R.P.steps : 16;
  st.onchange = () => { if (!R.P) return; snapshot(); R.P.steps = clamp(+st.value || 16, 1, 128); R.P.notes = R.P.notes.filter(n => n.at < R.P.steps * 96); R.P.locks = R.P.locks.filter(l => l.step < R.P.steps); drawRoll(); rollSend(true); };
  const gr = select([[24, '1/64'], [48, '1/32'], [96, '1/16'], [192, '1/8'], [384, '1/4'], [1, 'free']], R.grid, v => R.grid = +v);
  const ln = select([[24, '1/64'], [48, '1/32'], [96, '1/16'], [192, '1/8'], [384, '1/4'], [768, '1/2'], [1536, '1']], R.len, v => R.len = +v);
  const vl = el('input', 'num'); vl.type = 'number'; vl.min = 1; vl.max = 127; vl.value = R.vel; vl.onchange = () => R.vel = clamp(+vl.value || 100, 1, 127);
  const lockItems = [...LK_MIX.map((n, i) => [48 + i, n])];
  if (R.d) R.d.p.forEach((p, k) => { if (p.name && p.max > p.min && k < 45) lockItems.push([k, p.name]); });
  const lk = select(lockItems, R.lockP, v => { R.lockP = +v; drawRoll(); });
  row.append(el('span', 'dim', L('steps')), st, el('span', 'dim', L('grid')), gr, el('span', 'dim', L('len')), ln, el('span', 'dim', L('vel')), vl, el('span', 'dim', L('lockOf')), lk);
  if (!drumT()) row.append(button('▲ oct', () => { R.lo = Math.min(127 - R.rows + 1, R.lo + 12); drawRoll(); }, 'sm'), button('▼ oct', () => { R.lo = Math.max(0, R.lo - 12); drawRoll(); }, 'sm'));
  const row2 = el('div', 'tools');
  const ed = (f, lk2) => () => { if (!R.P) return; snapshot(); f(); drawRoll(); rollSend(lk2); };
  const S = () => R.sel.size ? [...R.sel] : R.P.notes;
  row2.append(
    button(L('undo'), () => { if (R.hist.length) { R.fut.push(JSON.stringify({ n: R.P.notes, l: R.P.locks, s: R.P.steps })); restore(R.hist.pop()); } }, 'sm'),
    button(L('redo'), () => { if (R.fut.length) { R.hist.push(JSON.stringify({ n: R.P.notes, l: R.P.locks, s: R.P.steps })); restore(R.fut.pop()); } }, 'sm'),
    button(L('all'), () => { R.sel = new Set(R.P.notes); drawRoll(); }, 'sm'),
    button(L('clear'), ed(() => { if (R.sel.size) R.P.notes = R.P.notes.filter(n => !R.sel.has(n)); else { R.P.notes = []; R.P.locks = []; } R.sel.clear(); }, true), 'sm red'),
    button(L('copyP'), () => { if (R.P) R.clip = JSON.parse(JSON.stringify({ n: S(), l: R.sel.size ? [] : R.P.locks, s: R.P.steps })); }, 'sm'),
    button(L('pasteP'), ed(() => { if (!R.clip) return; if (R.sel.size || !R.clip.l.length) { R.P.notes.push(...JSON.parse(JSON.stringify(R.clip.n))); } else { R.P.notes = JSON.parse(JSON.stringify(R.clip.n)); R.P.locks = JSON.parse(JSON.stringify(R.clip.l)); R.P.steps = R.clip.s; } R.sel.clear(); }, true), 'sm'),
    button(L('left'), ed(() => { const T2 = R.P.steps * 96; for (const n of S()) n.at = (n.at - 96 + T2) % T2; if (!R.sel.size) for (const l of R.P.locks) l.step = (l.step - 1 + R.P.steps) % R.P.steps; }, true), 'sm'),
    button(L('right'), ed(() => { const T2 = R.P.steps * 96; for (const n of S()) n.at = (n.at + 96) % T2; if (!R.sel.size) for (const l of R.P.locks) l.step = (l.step + 1) % R.P.steps; }, true), 'sm'),
    ...(drumT() ? [] : [button(L('tDn'), ed(() => { for (const n of S()) n.note = clamp(n.note - 1, 0, 127); }), 'sm'), button(L('tUp'), ed(() => { for (const n of S()) n.note = clamp(n.note + 1, 0, 127); }), 'sm'),
      button(L('oDn'), ed(() => { for (const n of S()) n.note = clamp(n.note - 12, 0, 127); }), 'sm'), button(L('oUp'), ed(() => { for (const n of S()) n.note = clamp(n.note + 12, 0, 127); }), 'sm')]),
    button(L('double'), ed(() => { const s0 = R.P.steps; if (s0 * 2 > 128) return; R.P.steps = s0 * 2; R.P.notes.push(...R.P.notes.map(n => ({ ...n, at: n.at + s0 * 96 }))); R.P.locks.push(...R.P.locks.map(l => ({ ...l, step: l.step + s0 }))); R.P.notes = R.P.notes.slice(0, 128); R.P.locks = R.P.locks.slice(0, 32); }, true), 'sm'),
    button(L('quant'), ed(() => { const g = Math.max(24, R.grid); for (const n of S()) n.at = Math.min(R.P.steps * 96 - g, Math.round(n.at / g) * g); }), 'sm'),
    button(L('human'), ed(() => { for (const n of S()) { n.vel = clamp(n.vel + Math.round((Math.random() - 0.5) * 24), 1, 127); n.at = clamp(n.at + Math.round((Math.random() - 0.5) * 8), 0, R.P.steps * 96 - 1); } }), 'sm'));
  T.append(row, row2);
}
/* the canvas: the notes, the velocities, the locks */
function rollGeom() {
  const cv = R.cv, W = cv.clientWidth || 860, rh = RH(), H1 = R.rows * rh, H = H1 + VH + KH + 8;
  return { W, H, H1, rh, sw: (W - X0 - 4) / (R.P ? R.P.steps : 16), vy: H1 + 4, ky: H1 + VH + 8 };
}
function drawRoll() {
  const cv = R.cv; if (!cv || !R.P) return;
  const G = rollGeom(), dpr = (typeof devicePixelRatio !== 'undefined' && devicePixelRatio) || 1;
  if (cv.width !== Math.round(G.W * dpr) || cv.height !== Math.round(G.H * dpr)) { cv.width = Math.round(G.W * dpr); cv.height = Math.round(G.H * dpr); cv.style.height = G.H + 'px'; }
  const g = cv.getContext('2d'); g.setTransform(dpr, 0, 0, dpr, 0, 0);
  const col = getComputedStyle(document.body).getPropertyValue(TCOL[R.t].slice(4, -1)).trim() || '#ffd23c';
  g.fillStyle = '#09080e'; g.fillRect(0, 0, G.W, G.H);
  for (let r = 0; r < R.rows; r++) {
    const n = R.lo + R.rows - 1 - r, y = r * G.rh, k = n % 12, black = !drumT() && [1, 3, 6, 8, 10].includes(k);
    g.fillStyle = black ? '#100e17' : '#1a1724'; g.fillRect(X0, y, G.W - X0 - 4, G.rh - 1);
    g.fillStyle = '#a49cb6'; g.font = '11px system-ui';
    const lab = drumT() ? PADS[n] : (k === 0 ? 'C' + (Math.floor(n / 12) - 1) : ['', '', 'D', '', 'E', 'F', '', 'G', '', 'A', '', 'B'][k]);
    if (lab) g.fillText(lab, 4, y + G.rh * 0.72);
  }
  const steps = R.P.steps;
  for (let s = 0; s <= steps; s++) { g.fillStyle = s % 16 === 0 ? '#5a5070' : s % 4 === 0 ? '#3a3450' : '#211d2c'; g.fillRect(X0 + s * G.sw, 0, 1, G.H1); }
  /* the playhead */
  if (LV && LV.t[R.t].cur === R.P.pt && LV.t[R.t].step !== 255) { g.fillStyle = '#ffffff22'; g.fillRect(X0 + LV.t[R.t].step * G.sw, 0, G.sw, G.H1); }
  for (const n of R.P.notes) {
    const r = R.lo + R.rows - 1 - n.note; if (r < 0 || r >= R.rows) continue;
    const x = X0 + n.at / 96 * G.sw, w = drumT() ? Math.max(4, G.sw * 0.8) : Math.max(3, n.len / 96 * G.sw - 1), sel = R.sel.has(n);
    g.globalAlpha = 0.4 + 0.6 * n.vel / 127; g.fillStyle = sel ? '#ffffff' : col; g.fillRect(x + 1, r * G.rh + 1, w - 1, G.rh - 3); g.globalAlpha = 1;
    const rat = (n.rat || 1) & 7; if (rat > 1) { g.fillStyle = '#000a'; for (let i = 1; i < rat; i++) g.fillRect(x + 1 + (w - 1) * i / rat, r * G.rh + 2, 1, G.rh - 5); }
    if (n.rat & SEQ_ACCENT) { g.fillStyle = '#ff6b6b'; g.fillRect(x + 1, r * G.rh + 1, 3, G.rh - 3); }
    if (n.rat & SEQ_SLIDE) { g.strokeStyle = '#5ef08a'; g.beginPath(); g.moveTo(x + w - 6, r * G.rh + G.rh - 3); g.lineTo(x + w + 4, r * G.rh + 1); g.stroke(); }
    if (n.cond) { g.fillStyle = '#ffb03c'; g.fillRect(x + 1, r * G.rh + G.rh - 4, w - 1, 2); }
  }
  if (R.box) { g.strokeStyle = '#ffd23c'; g.strokeRect(Math.min(R.box.x0, R.box.x1), Math.min(R.box.y0, R.box.y1), Math.abs(R.box.x1 - R.box.x0), Math.abs(R.box.y1 - R.box.y0)); }
  /* velocities */
  g.fillStyle = '#0d0c13'; g.fillRect(X0, G.vy, G.W - X0 - 4, VH); g.fillStyle = '#a49cb6'; g.fillText('VEL', 4, G.vy + 14);
  for (const n of R.P.notes) { const h = n.vel / 127 * (VH - 4), x = X0 + n.at / 96 * G.sw; g.fillStyle = R.sel.has(n) ? '#fff' : col; g.fillRect(x + 1, G.vy + VH - h, Math.max(2, Math.min(6, G.sw * 0.3)), h); }
  /* the locks of the value chosen */
  g.fillStyle = '#0d0c13'; g.fillRect(X0, G.ky, G.W - X0 - 4, KH);
  const lname = R.lockP >= 48 ? LK_MIX[R.lockP - 48] : R.d && R.d.p[R.lockP] ? R.d.p[R.lockP].name : '#' + R.lockP;
  g.fillStyle = '#a49cb6'; g.fillText(lname.slice(0, 8), 4, G.ky + 14);
  for (let s = 0; s < steps; s++) { g.fillStyle = '#1a1724'; g.fillRect(X0 + s * G.sw + 1, G.ky, G.sw - 2, KH); }
  for (const l of R.P.locks) if (l.param === R.lockP) { const h = l.value / 255 * (KH - 4); g.fillStyle = '#ffb03c'; g.fillRect(X0 + l.step * G.sw + 2, G.ky + KH - h, G.sw - 4, h); }
  const others = new Set(R.P.locks.filter(l => l.param !== R.lockP).map(l => l.step)); g.fillStyle = '#ffb03c66';
  for (const s of others) g.fillRect(X0 + s * G.sw + G.sw / 2 - 2, G.ky + 2, 4, 4);
  drawInspector();
}
function hitNote(x, y, G) {
  for (let i = R.P.notes.length - 1; i >= 0; i--) { const n = R.P.notes[i], r = R.lo + R.rows - 1 - n.note; if (r < 0 || r >= R.rows) continue;
    const nx = X0 + n.at / 96 * G.sw, w = drumT() ? Math.max(4, G.sw * 0.8) : Math.max(3, n.len / 96 * G.sw - 1);
    if (x >= nx && x <= nx + w + 1 && y >= r * G.rh && y < (r + 1) * G.rh) return { n, edge: !drumT() && x > nx + w - 6 }; }
  return null;
}
function rollEvents(cv) {
  const pos = ev => { const rc = cv.getBoundingClientRect(); return [ev.clientX - rc.left, ev.clientY - rc.top]; };
  const snap = t => R.grid > 1 ? Math.floor(t / R.grid) * R.grid : Math.round(t);
  cv.addEventListener('contextmenu', ev => ev.preventDefault());
  cv.addEventListener('pointerdown', ev => {
    if (!R.P || !info) return; const [x, y] = pos(ev), G = rollGeom(); if (x < X0) return; cv.setPointerCapture(ev.pointerId);
    const T2 = R.P.steps * 96, tick = clamp((x - X0) / G.sw * 96, 0, T2 - 1);
    if (y < G.H1) {
      const h = hitNote(x, y, G);
      if (ev.button === 2) { if (h) { snapshot(); R.P.notes.splice(R.P.notes.indexOf(h.n), 1); R.sel.delete(h.n); drawRoll(); rollSend(); } return; }
      if (h) {
        if (ev.shiftKey) { R.sel.has(h.n) ? R.sel.delete(h.n) : R.sel.add(h.n); } else if (!R.sel.has(h.n)) { R.sel.clear(); R.sel.add(h.n); }
        snapshot(); R.drag = { kind: h.edge ? 'len' : 'move', x0: x, y0: y, orig: [...R.sel].map(n => [n, n.at, n.note, n.len]), moved: false };
        noteOut(R.t, h.n.note, h.n.vel, 1); setTimeout(() => noteOut(R.t, h.n.note, 0, 0), 150);
      } else if (ev.shiftKey) { R.box = { x0: x, y0: y, x1: x, y1: y }; R.drag = { kind: 'box' }; }
      else {
        const note = R.lo + R.rows - 1 - Math.floor(y / G.rh); if (note < 0 || note > 127) return;
        snapshot(); const n = { at: snap(tick), len: drumT() ? 72 : R.len, note, vel: R.vel, cond: 0, rat: 1 };
        if (R.P.notes.length >= 128) return; R.P.notes.push(n); R.sel.clear(); R.sel.add(n);
        R.drag = { kind: drumT() ? 'none' : 'len', x0: x, y0: y, orig: [[n, n.at, n.note, n.len]], moved: false, fresh: true };
        noteOut(R.t, note, R.vel, 1); setTimeout(() => noteOut(R.t, note, 0, 0), 150); drawRoll(); rollSend();
      }
      drawRoll(); return;
    }
    if (y >= G.vy && y < G.vy + VH) { snapshot(); R.drag = { kind: 'vel' }; velAt(x, y, G); return; }
    if (y >= G.ky) { const s = Math.floor((x - X0) / G.sw); if (ev.button === 2) { snapshot(); R.P.locks = R.P.locks.filter(l => !(l.step === s && l.param === R.lockP)); drawRoll(); rollSend(true); return; } snapshot(); R.drag = { kind: 'lock' }; lockAt(x, y, G); }
  });
  cv.addEventListener('pointermove', ev => {
    const D = R.drag; if (!D) return; const [x, y] = pos(ev), G = rollGeom(), T2 = R.P.steps * 96;
    if (D.kind === 'move') { const dt = (x - D.x0) / G.sw * 96, dn = -Math.round((y - D.y0) / G.rh); for (const [n, at, note] of D.orig) { n.at = clamp(R.grid > 1 ? Math.round((at + dt) / R.grid) * R.grid : Math.round(at + dt), 0, T2 - 1); n.note = drumT() ? clamp(note + dn, 0, 15) : clamp(note + dn, 0, 127); } D.moved = true; drawRoll(); }
    else if (D.kind === 'len') { const dt = (x - D.x0) / G.sw * 96; for (const [n, , , len] of D.orig) n.len = clamp(R.grid > 1 ? Math.max(R.grid, Math.round((len + dt) / R.grid) * R.grid) : Math.round(len + dt), 12, T2); D.moved = true; drawRoll(); }
    else if (D.kind === 'box') { R.box.x1 = x; R.box.y1 = y; drawRoll(); }
    else if (D.kind === 'vel') velAt(x, y, G);
    else if (D.kind === 'lock') lockAt(x, y, G);
  });
  const end = () => {
    const D = R.drag; R.drag = null; if (!D) return;
    if (D.kind === 'box') { const G = rollGeom(), [a, b] = [Math.min(R.box.x0, R.box.x1), Math.max(R.box.x0, R.box.x1)], [c, d] = [Math.min(R.box.y0, R.box.y1), Math.max(R.box.y0, R.box.y1)];
      for (const n of R.P.notes) { const nx = X0 + n.at / 96 * G.sw, ny = (R.lo + R.rows - 1 - n.note) * G.rh + G.rh / 2; if (nx >= a && nx <= b && ny >= c && ny <= d) R.sel.add(n); } R.box = null; drawRoll(); return; }
    if (D.kind === 'move' && !D.moved) { R.hist.pop(); return; }
    if (D.kind === 'move' || D.kind === 'len' || D.kind === 'vel') { R.P.notes.sort((a, b) => a.at - b.at); rollSend(); }
    if (D.kind === 'lock') rollSend(true);
  };
  cv.addEventListener('pointerup', end); cv.addEventListener('pointercancel', end);
  cv.addEventListener('dblclick', ev => { const [x, y] = pos(ev), G = rollGeom(); if (y >= G.H1) return; const h = hitNote(x, y, G); if (h) { R.P.notes.splice(R.P.notes.indexOf(h.n), 1); R.sel.delete(h.n); drawRoll(); rollSend(); } });
  cv.addEventListener('wheel', ev => { if (drumT() || ev.offsetY > rollGeom().H1) return; ev.preventDefault(); R.lo = clamp(R.lo + (ev.deltaY < 0 ? 2 : -2), 0, 127 - R.rows + 1); drawRoll(); }, { passive: false });
}
function velAt(x, y, G) {
  const v = clamp(Math.round((G.vy + VH - y) / (VH - 4) * 127), 1, 127), tick = (x - X0) / G.sw * 96;
  const near = R.P.notes.filter(n => Math.abs(n.at - tick) < 48 && (!R.sel.size || R.sel.has(n)));
  for (const n of near) n.vel = v; if (near.length) drawRoll();
}
function lockAt(x, y, G) {
  const s = Math.floor((x - X0) / G.sw); if (s < 0 || s >= R.P.steps) return;
  const v = clamp(Math.round((G.ky + KH - y) / (KH - 4) * 255), 0, 255);
  let l = R.P.locks.find(l => l.step === s && l.param === R.lockP);
  if (!l) { if (R.P.locks.length >= 32) return; l = { step: s, param: R.lockP, value: v }; R.P.locks.push(l); }
  l.value = v; drawRoll();
}
/* the notes selected: their values */
function drawInspector() {
  const I = R.insp; if (!I) return;
  const S = [...R.sel];
  /* (3.4.2, issue #31: "the lists of condition and ratchet don't open") the roll is drawn again at each news of the
   * FM-1 while it plays: the inspector only when what it shows changed, so that an open list is not taken away */
  const key = lang + '|' + S.map(x => [x.at, x.len, x.vel, x.cond, x.rat, x.note].join(',')).join(';');
  if (R.inspKey === key && R.inspSel && R.inspSel.length === S.length && S.every((x, i) => x === R.inspSel[i])) return;
  R.inspKey = key; R.inspSel = S; I.innerHTML = '';
  if (!S.length) { I.appendChild(el('span', 'dim', L('noSel'))); return; }
  const n = S[0], all = f => { snapshot(); for (const x of S) f(x); drawRoll(); rollSend(); };
  I.appendChild(el('b', '', S.length + L('sel')));
  const num = (lab, v, mn, mx, f) => { const i = el('input', 'num'); i.type = 'number'; i.min = mn; i.max = mx; i.value = v; i.onchange = () => all(x => f(x, clamp(+i.value, mn, mx))); I.append(el('span', 'dim', lab), i); };
  num(L('vel'), n.vel, 1, 127, (x, v) => x.vel = v);
  if (!drumT()) num(L('len'), n.len, 12, R.P.steps * 96, (x, v) => x.len = v);
  num(L('nudge'), n.at % 96 - (n.at % 96 > 48 ? 96 : 0), -47, 48, (x, v) => { const s = Math.round((x.at - (x.at % 96 > 48 ? x.at % 96 - 96 : x.at % 96)) / 96) * 96; x.at = clamp(s + v, 0, R.P.steps * 96 - 1); });
  const cs = select(CONDS.map(c => [c, condName(c)]), n.cond || 0, v => all(x => x.cond = +v)); I.append(el('span', 'dim', L('cond')), cs);
  const rs = select([1, 2, 3, 4].map(k => [k, '×' + k]), (n.rat || 1) & 7, v => all(x => x.rat = (x.rat & 0x70) | +v)); I.append(el('span', 'dim', L('ratchet')), rs);
  const flag = (lab, bit) => { const b = button(lab, () => { const on = !(n.rat & bit); all(x => x.rat = on ? (x.rat || 1) | bit : (x.rat || 1) & ~bit); }, 'sm' + (n.rat & bit ? ' on' : '')); I.appendChild(b); };
  flag(L('accent'), SEQ_ACCENT); flag(L('slide'), SEQ_SLIDE);
}
/* the track's DIV GATE SWING QUANT (op 16) */
function drawTrackSet() {
  const E = R.trkEl; if (!E) return; E.innerHTML = ''; const tk = LV && LV.t[R.t]; if (!tk) return;
  const s = (w, v) => soon('ts' + R.t + w, () => act(16, R.t, w, v));
  R.trkKnobs = [knob({ label: 'DIV', min: 0, max: 9, value: tk.div, fmt: v => DIVS[v], onChange: v => s(0, v) }), knob({ label: 'GATE', min: 10, max: 200, value: tk.gate, def: 100, fmt: v => v + '%', onChange: v => s(1, v) }),
    knob({ label: 'SWING', min: 0, max: 100, value: tk.swing, onChange: v => s(2, v) }), knob({ label: 'QUANT', min: 0, max: 100, value: tk.quant, onChange: v => s(3, v) })];
  E.append(...R.trkKnobs);
}
liveHooks.push(st => {
  if (!$('tab_seq').classList.contains('on') || !R.P) return;
  const tk = st.t[R.t];
  if (R.pt === 255 && R.follow && tk.cur !== R.P.pt && !R.drag) { soon('rollre', rollLoad, 100); return; }
  if (R.trkKnobs) { R.trkKnobs[0].set(tk.div); R.trkKnobs[1].set(tk.gate); R.trkKnobs[2].set(tk.swing); R.trkKnobs[3].set(tk.quant); } else drawTrackSet();
  if (!R.drag) drawRoll();
});
if (typeof window !== 'undefined') window.addEventListener('keydown', ev => {
  if (!$('tab_seq').classList.contains('on') || /INPUT|SELECT/.test(ev.target.tagName) || !R.P) return;
  if ((ev.key === 'Delete' || ev.key === 'Backspace') && R.sel.size) { snapshot(); R.P.notes = R.P.notes.filter(n => !R.sel.has(n)); R.sel.clear(); drawRoll(); rollSend(); ev.preventDefault(); }
  if ((ev.ctrlKey || ev.metaKey) && ev.key === 'z') { ev.preventDefault(); if (R.hist.length) { R.fut.push(JSON.stringify({ n: R.P.notes, l: R.P.locks, s: R.P.steps })); restore(R.hist.pop()); } }
  if ((ev.ctrlKey || ev.metaKey) && ev.key === 'a') { ev.preventDefault(); R.sel = new Set(R.P.notes); drawRoll(); }
});
/* ---------------------------------------------------------------- MIDI files */
function vlq(v) { const b = [v & 127]; while ((v >>= 7)) b.unshift((v & 127) | 128); return b; }
function chunk(id, data) { return [...id].map(c => c.charCodeAt(0)).concat([data.length >>> 24, data.length >> 16 & 255, data.length >> 8 & 255, data.length & 255], data); }
async function midiFile(pat) {
  try {
    const tracks = []; let bpm = 120;
    for (const t of SHOWN) {
      const P = await patRead(t, pat); bpm = P.bpm || bpm;
      const ch = t === DRUM_T ? 9 : SHOWN.indexOf(t), ev = [], end = P.steps * 96;
      for (const n of P.notes) { const k = t === DRUM_T ? GM_PAD[n.note & 15] : n.note, off = Math.min(end, n.at + Math.max(1, t === DRUM_T ? 48 : n.len)); ev.push([n.at, 1, [0x90 | ch, k, clamp(n.vel, 1, 127)]], [off, 0, [0x80 | ch, k, 0]]); }
      ev.sort((a, b) => a[0] - b[0] || a[1] - b[1]);
      const nm = t === DRUM_T ? 'DRUMS' : 'TRACK ' + (SHOWN.indexOf(t) + 1), d = [0, 0xFF, 0x03, ...vlq(nm.length), ...[...nm].map(c => c.charCodeAt(0))];
      let at = 0; for (const [tk, , m] of ev) { d.push(...vlq(tk - at), ...m); at = tk; }
      d.push(...vlq(Math.max(0, end - at)), 0xFF, 0x2F, 0); tracks.push(d);
    }
    const us = Math.round(60000000 / bpm), tempo = [0, 0xFF, 0x51, 3, us >> 16 & 255, us >> 8 & 255, us & 255, 0, 0xFF, 0x58, 4, 4, 2, 24, 8, 0, 0xFF, 0x2F, 0];
    const head = chunk('MThd', [0, 1, 0, tracks.length + 1, 384 >> 8, 384 & 255]);
    const file = new Uint8Array(head.concat(chunk('MTrk', tempo), ...tracks.map(d => chunk('MTrk', d))));
    const name = 'sloop3-' + (pat === 255 ? 'now' : 'pattern-' + (pat + 1)) + '.mid'; save(file, name); log(name, 'ok');
  } catch (e) { fail(e); }
}
/* a .mid file read: each track's notes (tick, len, note, vel, channel), its division */
function midiParse(d) {
  const rd = (i, n) => { let v = 0; for (let k = 0; k < n; k++) v = v * 256 + d[i + k]; return v; };
  if (String.fromCharCode(...d.slice(0, 4)) !== 'MThd') return null;
  const ntr = rd(10, 2), div = rd(12, 2); if (div & 0x8000) return null;
  let i = 8 + rd(4, 4); const trs = [];
  for (let t = 0; t < ntr && i + 8 <= d.length; t++) {
    const len = rd(i + 4, 4), end = i + 8 + len; let p = i + 8, tick = 0, run = 0; const on = {}, notes = [];
    if (String.fromCharCode(...d.slice(i, i + 4)) !== 'MTrk') break;
    while (p < end) {
      let v = 0, b; do { b = d[p++]; v = v * 128 + (b & 127); } while (b & 128 && p < end); tick += v;
      let st = d[p]; if (st & 128) p++; else st = run;
      if (st === 0xFF) { p++; let l = 0; do { b = d[p++]; l = l * 128 + (b & 127); } while (b & 128); p += l; continue; }
      if (st === 0xF0 || st === 0xF7) { let l = 0; do { b = d[p++]; l = l * 128 + (b & 127); } while (b & 128); p += l; continue; }
      run = st; const ty = st & 0xF0, ch = st & 15, a = d[p++], c = ty === 0xC0 || ty === 0xD0 ? 0 : d[p++];
      if (ty === 0x90 && c) on[ch * 128 + a] = { at: tick, note: a, vel: c, ch };
      else if (ty === 0x80 || (ty === 0x90 && !c)) { const o = on[ch * 128 + a]; if (o) { notes.push({ ...o, len: tick - o.at }); delete on[ch * 128 + a]; } }
    }
    trs.push(notes); i = end;
  }
  return { div, trs };
}
async function midiImport(bytes, fname) {
  const m = midiParse(bytes); const tr = m && m.trs.find(n => n.length);
  if (!tr) { log(L('midiBad'), 'bad'); return; }
  if (!R.P) await rollLoad(); if (!R.P) return;
  const k = 96 * 4 / m.div, notes = [];
  /* (3.4.0, Sam: "the notes all SOFT") a file played softly (many give every note 64 or 80): the loudest note brought
   * up to NORM (100), the others with it, so the FM-1 plays it as loud as its keys; a louder file stays as it is */
  const vmax = Math.max(...tr.map(n => n.vel), 1), vk = vmax < 100 ? 100 / vmax : 1;
  for (const n of tr) {
    let note = n.note;
    if (drumT()) { let p = GM_PAD.indexOf(n.note); if (p < 0) p = [36, 35, 38, 39, 42, 46, 44, 37, 40, 43, 48, 49, 51, 70, 63, 56].indexOf(n.note); if (p < 0) continue; note = p; }
    notes.push({ at: Math.round(n.at * k), len: Math.max(12, Math.round(n.len * k)), note, vel: clamp(Math.round(n.vel * vk), 1, 127), cond: 0, rat: 1 });
  }
  const last = Math.max(...notes.map(n => n.at + 1), 1), steps = clamp(Math.ceil(last / 96 / 16) * 16, 16, 128);
  snapshot(); R.P.steps = steps; R.P.notes = notes.filter(n => n.at < steps * 96).slice(0, 128);
  log(fname + ': ' + R.P.notes.length + L('midiGot') + trkName(R.t) + ' ' + L('pat') + (R.P.pt + 1) + (notes.length > 128 ? L('tooMany') : ''), 'ok');
  drawSeqTools(); drawRoll(); rollSend();
}
/* ---------------------------------------------------------------- the song */
const SONG = { n: 0, len: 0, play: 0, rec: 0, e: [] };
async function songRead() {
  try { let first = 0; SONG.e = [];
    do { const b = await ask(ED.SONG, [first]); SONG.n = b[1]; SONG.len = u16(b, 2); SONG.play = b[4]; SONG.rec = b[5]; const cnt = b[6];
      for (let i = 0; i < cnt; i++) { const o = 7 + 8 * i; SONG.e.push({ bar: u16(b, o), mute: b[o + 2], pat: [b[o + 3], b[o + 4], b[o + 5], b[o + 6], b[o + 7]] }); }
      first += cnt; if (!cnt) break; } while (first < SONG.n);
    drawSongTable();
  } catch (e) { fail(e); }
}
async function songSend() {
  try { SONG.e.sort((a, b) => a.bar - b.bar); const n = SONG.e.length; let first = 0;
    do { const part = SONG.e.slice(first, first + 60), raw = [first, n, SONG.len & 255, SONG.len >> 8];
      for (const e of part) raw.push(e.bar & 255, e.bar >> 8, e.mute, ...e.pat); await ask(ED.SONG, raw); first += 60; } while (first < n);
    log(L('songSent') + n + L('lines'), 'ok'); songRead();
  } catch (e) { fail(e); }
}
function drawSong() {
  const P = $('tab_song'); P.innerHTML = '';
  const c = card(P, L('song'), '', L('songHelp'), 'wide');
  const r = el('div', 'tools'); SONG.lenIn = el('input', 'num'); SONG.lenIn.type = 'number'; SONG.lenIn.min = 0; SONG.lenIn.max = 9999; SONG.lenIn.onchange = () => SONG.len = clamp(+SONG.lenIn.value | 0, 0, 65535);
  SONG.bPlay = button(L('songMode'), () => layerKey(BTN.SAVE, WHITE_KEY[12]).then(songRead).catch(fail)); SONG.bRec = button(L('songRec'), () => layerKey(BTN.SAVE, WHITE_KEY[13]).then(songRead).catch(fail), 'red');
  r.append(button(L('songRead'), songRead), button(L('songSend'), songSend, 'pri'), el('span', 'dim', L('songLen')), SONG.lenIn,
    button(L('addLine'), () => { const l = SONG.e[SONG.e.length - 1]; SONG.e.push(l ? { bar: l.bar + 4, mute: l.mute, pat: l.pat.slice() } : { bar: 0, mute: 0, pat: [0, 0, 0, 0, 0] }); SONG.len = Math.max(SONG.len, SONG.e[SONG.e.length - 1].bar + 4); drawSongTable(); }), SONG.bPlay, SONG.bRec);
  c.appendChild(r); SONG.tb = el('table', 'tb'); c.appendChild(SONG.tb); drawSongTable();
}
function drawSongTable() {
  const tb = SONG.tb; if (!tb) return; tb.innerHTML = ''; if (SONG.lenIn) SONG.lenIn.value = SONG.len;
  if (SONG.bPlay) { SONG.bPlay.classList.toggle('on', !!SONG.play); SONG.bRec.classList.toggle('on', !!SONG.rec); }
  const hr = el('tr'); hr.appendChild(el('th', '', '#')); hr.appendChild(el('th', '', L('bar')));
  for (const t of SHOWN) { const th = el('th', '', trkName(t)); th.style.color = TCOL[t]; hr.appendChild(th); } hr.appendChild(el('th')); tb.appendChild(hr);
  SONG.e.forEach((e, i) => {
    const tr = el('tr'); tr.appendChild(el('td', 'dim', String(i + 1)));
    const bi = el('input', 'cell'); bi.type = 'number'; bi.min = 0; bi.value = e.bar + 1; bi.onchange = () => e.bar = clamp((+bi.value | 0) - 1, 0, 65535); const td0 = el('td'); td0.appendChild(bi); tr.appendChild(td0);
    for (const t of SHOWN) { const td = el('td'); const s = select([...Array(8)].map((_, p) => [p, String(p + 1)]), e.pat[t], v => e.pat[t] = +v); s.style.width = '52px';
      const m = button('M', () => { e.mute ^= 1 << t; m.classList.toggle('on', !!(e.mute >> t & 1)); }, 'sm red' + (e.mute >> t & 1 ? ' on' : '')); td.append(s, m); tr.appendChild(td); }
    const td = el('td'); td.append(button('↑', () => { if (i) { [SONG.e[i - 1], SONG.e[i]] = [SONG.e[i], SONG.e[i - 1]]; drawSongTable(); } }, 'sm'), button('×', () => { SONG.e.splice(i, 1); drawSongTable(); }, 'sm')); tr.appendChild(td);
    tb.appendChild(tr);
  });
}
liveHooks.push(st => { if (SONG.bPlay && $('tab_song').classList.contains('on')) { SONG.bPlay.classList.toggle('on', st.song); SONG.bRec.classList.toggle('on', st.songRec); } });
