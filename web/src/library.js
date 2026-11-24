/* Library: the FM-1's sounds and kits listed, your 32 sounds (renamed, moved, erased, their kind, saved to a file
 * and back) and your 16 favourites (first in PRESETS). Drum kit: the kits on the drum track, each pad's TUNE and
 * DECAY (ED_LIVE op 11), heard. */
words({
  libTitle: 'Your sounds', libHelp: 'Your 32 places (EDIT 5 on the FM-1). Rename, choose the kind, move (↑), erase, put on the track chosen (▶). Nothing changes on the FM-1 until SEND.\nTO A FILE / FROM A FILE: all 32 (with your favourites and your theme), as SLOOP 3 keeps them.',
  read: 'READ', send: 'SEND TO THE FM-1', toFile: 'TO A FILE', fromFile: 'FROM A FILE', erase: 'erase', onTrack: '▶ track', empty: '(empty)',
  sent: 'Your sounds sent: the FM-1 saves them.', notUs: 'Not a file of your sounds (.s3u).', sureUs: 'Replace your 32 sounds, your favourites and your theme on the FM-1 by these?',
  favs: 'Your favourites', favHelp: 'The 16 sounds PRESETS shows first (EDIT 6 on the FM-1). Add one from the lists; ↑ moves it, × takes it out. Sent with your sounds (SEND).', addFav: '+ favourite', favFull: '16 favourites already.',
  lists: 'The FM-1\'s lists', listsHelp: 'Its factory sounds (by kind, with their engine) and its drum kits. ▶: on the track chosen. ★: into your favourites.',
  yourKits: 'Your kits:', kitsTitle: 'Drum kit', kitsHelp: 'Click a kit: the drum track plays it. YOUR KITS: a sample slot as a kit (a break sent to the drums: its slices on the pads), by its sample\'s name, and your SYN kits; on the FM-1, SOUNDS > YOUR KITS. The pads: TUNE (−8..+7) and DECAY (−4..+3) of each sound, heard at once; ▶ plays it. They are kept with the project (and the pattern) as on the FM-1.',
  tune: 'TUNE', dec: 'DECAY', resetPads: 'all to 0'
}, {
  libTitle: 'Tes sons', libHelp: 'Tes 32 places (EDIT 5 sur le FM-1). Renommer, choisir la catégorie, déplacer (↑), effacer, mettre sur la piste choisie (▶). Rien ne change sur le FM-1 avant ENVOYER.\nVERS UN FICHIER / D\'UN FICHIER : les 32 (avec tes favoris et ton thème), comme SLOOP 3 les garde.',
  read: 'LIRE', send: 'ENVOYER AU FM-1', toFile: 'VERS UN FICHIER', fromFile: 'D\'UN FICHIER', erase: 'effacer', onTrack: '▶ piste', empty: '(vide)',
  sent: 'Tes sons envoyés : le FM-1 les enregistre.', notUs: 'Pas un fichier de tes sons (.s3u).', sureUs: 'Remplacer tes 32 sons, tes favoris et ton thème sur le FM-1 par ceux-ci ?',
  favs: 'Tes favoris', favHelp: 'Les 16 sons que PRESETS montre d\'abord (EDIT 6 sur le FM-1). Ajoutes-en depuis les listes ; ↑ le déplace, × le retire. Envoyés avec tes sons (ENVOYER).', addFav: '+ favori', favFull: 'Déjà 16 favoris.',
  lists: 'Les listes du FM-1', listsHelp: 'Ses sons d\'usine (par catégorie, avec leur moteur) et ses kits de batterie. ▶ : sur la piste choisie. ★ : dans tes favoris.',
  yourKits: 'Tes kits :', kitsTitle: 'Kit batterie', kitsHelp: 'Clic sur un kit : la piste de batterie le joue. TES KITS : une case d\'échantillons comme kit (un break envoyé à la batterie : ses tranches sur les pads), au nom de son échantillon, et tes kits SYN ; sur le FM-1, SOUNDS > YOUR KITS. Les pads : TUNE (−8..+7) et DECAY (−4..+3) de chaque son, entendus tout de suite ; ▶ le joue. Ils sont gardés avec le projet (et le pattern) comme sur le FM-1.',
  tune: 'TUNE', dec: 'DECAY', resetPads: 'tout à 0'
});
const US_N = 32, US_REC = 80, US_LEN = 8 + US_N * US_REC, US_FV = US_LEN + 2 + 32, US_FILE = US_FV + 40;
const LIB = { f: null, recs: [], fav: [] };
function usParse(f) {
  if (!f || f.length < US_LEN || f[0] !== 0x53 || f[1] !== 0x33 || f[2] !== 0x55 || f[3] !== 0x53) return false;
  const full = new Uint8Array(US_FILE); full.set(f.slice(0, Math.min(f.length, US_FILE)));
  if (f.length < US_FV || f[US_LEN] !== 0x46 || f[US_LEN + 1] !== 0x56) { full[US_LEN] = 0x46; full[US_LEN + 1] = 0x56; for (let i = 0; i < 16; i++) { full[US_LEN + 2 + 2 * i] = 0xFF; full[US_LEN + 3 + 2 * i] = 0xFF; } }
  LIB.f = full; LIB.recs = []; for (let k = 0; k < US_N; k++) LIB.recs.push(full.slice(8 + k * US_REC, 8 + (k + 1) * US_REC));
  LIB.fav = []; for (let i = 0; i < 16; i++) { const v = u16(full, US_LEN + 2 + 2 * i); if (v !== 0xFFFF) LIB.fav.push(v); }
  return true;
}
function usBytes() {
  const f = LIB.f.slice(); f.set([0x53, 0x33, 0x55, 0x53, 1, US_N, 0, 0]);
  LIB.recs.forEach((r, k) => f.set(r, 8 + k * US_REC));
  f[US_LEN] = 0x46; f[US_LEN + 1] = 0x56; for (let i = 0; i < 16; i++) { const v = i < LIB.fav.length ? LIB.fav[i] : 0xFFFF; f[US_LEN + 2 + 2 * i] = v & 255; f[US_LEN + 3 + 2 * i] = v >> 8; }
  return f;
}
const recName = r => r[0] === 0xA5 ? str(r, 4, 12) : '';
function favName(v) { if (v >= 2000) return dxName(v - 2000); if (v >= 1000) { const r = LIB.recs[v - 1000]; return (r && recName(r)) || 'USER ' + (v - 999); } const p = NAMES.presets[v]; return p ? p.name : '#' + v; }
async function libRead() { try { if (!usParse(await fileRead(1, 0))) throw new Error(L('notUs')); drawLibrary(); } catch (e) { fail(e); } }
function drawLibrary() {
  const P = $('tab_library'); P.innerHTML = '';
  const cards = el('div', 'cards'); cards.style.gridTemplateColumns = 'minmax(0,3fr) minmax(280px,2fr)'; P.appendChild(cards);
  const left = el('div'), right = el('div'); left.style.cssText = right.style.cssText = 'display:flex;flex-direction:column;gap:12px;min-width:0'; cards.append(left, right);
  const c = card(left, L('libTitle'), '', L('libHelp'));
  const r = el('div', 'tools');
  r.append(button(L('read'), libRead), button(L('send'), async () => { if (!LIB.f) return; try { await fileWrite(1, 0, usBytes()); log(L('sent'), 'ok'); await retryBusy(() => readNames()); } catch (e) { fail(e); } }, 'pri'),
    button(L('toFile'), () => { if (LIB.f) save(usBytes(), 'sloop3-your-sounds.s3u'); }),
    button(L('fromFile'), () => pick(d => { if (!usParse(d)) { log(L('notUs'), 'bad'); return; } drawLibrary(); }, '.s3u')));
  c.appendChild(r);
  if (LIB.f) {
    const tb = el('table', 'tb');
    LIB.recs.forEach((rec, k) => {
      const tr = el('tr'), used = rec[0] === 0xA5; tr.appendChild(el('td', 'dim', String(k + 1)));
      const nm = el('input', 'txt'); nm.maxLength = 12; nm.value = recName(rec); nm.placeholder = L('empty'); nm.disabled = !used; nm.style.width = '130px';
      nm.oninput = () => { const s = nm.value.toUpperCase().slice(0, 12); for (let i = 0; i < 12; i++) rec[4 + i] = i < s.length ? s.charCodeAt(i) & 127 : 0; };
      const kd = select(NAMES.kinds.filter(x => x.name !== 'KIT').map(x => [x.i, x.name]), rec[2], v => rec[2] = +v); kd.disabled = !used;
      const tds = [nm, kd, el('span', 'dim', used ? ENGINES[rec[1]] || '?' : ''),
        button('↑', () => { if (k) { [LIB.recs[k - 1], LIB.recs[k]] = [LIB.recs[k], LIB.recs[k - 1]]; LIB.fav = LIB.fav.map(v => v === 1000 + k ? 999 + k : v === 999 + k ? 1000 + k : v); drawLibrary(); } }, 'sm'),
        button(L('erase'), () => { rec.fill(0); LIB.fav = LIB.fav.filter(v => v !== 1000 + k); drawLibrary(); }, 'sm red'),
        button(L('onTrack'), () => act(10, curTrk, (1000 + k) & 255, (1000 + k) >> 8), 'sm'),
        button('★', () => addFav(1000 + k), 'sm')];
      if (!used) kd.style.visibility = 'hidden';
      for (const x of tds) { const td = el('td'); td.appendChild(x); tr.appendChild(td); }
      if (!used) for (const b of tr.querySelectorAll('button')) b.disabled = true;
      tb.appendChild(tr);
    });
    c.appendChild(tb);
  }
  /* the favourites */
  const fc = card(right, L('favs'), '', L('favHelp'));
  const fl = el('div', 'list'); fc.appendChild(fl);
  LIB.fav.forEach((v, i) => { const d = el('div'); const n = el('span', '', (i + 1) + '. ' + favName(v)); n.style.flex = '1';
    d.append(n, button('↑', () => { if (i) { [LIB.fav[i - 1], LIB.fav[i]] = [LIB.fav[i], LIB.fav[i - 1]]; drawLibrary(); } }, 'sm'), button('×', () => { LIB.fav.splice(i, 1); drawLibrary(); }, 'sm')); fl.appendChild(d); });
  if (!LIB.f) fl.appendChild(el('div', 'dim', L('read') + '…'));
  /* the FM-1's lists */
  const lc = card(right, L('lists'), '', L('listsHelp'));
  const q = el('input', 'txt'); q.placeholder = L('search'); q.style.width = '100%'; const ll = el('div', 'list'); ll.style.marginTop = '6px'; lc.append(q, ll);
  const draw = () => {
    ll.innerHTML = ''; const f = q.value.trim().toUpperCase(); let last = -1;
    for (const p of NAMES.presets.slice().sort((a, b) => a.kind - b.kind || a.i - b.i)) {
      const kn = (NAMES.kinds[p.kind] || {}).name || ''; if (f && !p.name.includes(f) && !kn.includes(f)) continue;
      if (p.kind !== last) { last = p.kind; ll.appendChild(el('div', 'hd', kn)); }
      const d = el('div'); const n = el('span', '', p.name); n.style.flex = '1';
      d.append(n, el('span', 'cat', ENGINES[p.eng] || ''), button('▶', () => act(10, kn === 'KIT' ? DRUM_T : curTrk === DRUM_T ? 0 : curTrk, p.i & 255, p.i >> 8), 'sm'), button('★', () => addFav(p.i), 'sm')); ll.appendChild(d);
    }
  };
  q.oninput = draw; draw();
}
function addFav(v) { if (!LIB.f) { log(L('read') + '…', 'bad'); return; } if (LIB.fav.length >= 16) { log(L('favFull'), 'bad'); return; } if (!LIB.fav.includes(v)) LIB.fav.push(v); drawLibrary(); }
/* the drum kit */
const KIT = { knobs: [] }, UK_BASE = 3000, KIT_USR = 37, KIT_SYN = 42;   /* (fw/main.c UK_BASE; engine/drum.h GB_KIT_USR, GB_KIT_SYN) */
function drawKits() {
  const P = $('tab_kits'); P.innerHTML = '';
  const cards = el('div', 'cards'); P.appendChild(cards);
  const c = card(cards, L('kitsTitle'), '', L('kitsHelp'), 'wide');
  const kl = el('div', 'row'); c.appendChild(kl); KIT.btns = [];
  for (const p of NAMES.presets.filter(p => (NAMES.kinds[p.kind] || {}).name === 'KIT')) { const b = button(p.name, () => act(10, DRUM_T, p.i & 255, p.i >> 8), 'sm'); b.dataset.i = p.i; kl.appendChild(b); KIT.btns.push(b); }
  /* (3.3.6) your kits: USR1-4 (a sample slot as a kit, by its sample's name) and SYN1-4, as on the FM-1 (SOUNDS, YOUR KITS) */
  const ul = el('div', 'row'); ul.style.marginTop = '8px'; ul.append(el('span', 'dim', L('yourKits'))); c.appendChild(ul);
  const ukBtn = (kit, name, ok) => { const v = UK_BASE + kit, b = button(name, () => act(10, DRUM_T, v & 255, v >> 8), 'sm'); b.dataset.i = v; b.disabled = !ok; ul.appendChild(b); KIT.btns.push(b); };
  (async () => { let sl = null; try { const b = await ask(ED.SAMPLE, [0]); sl = []; for (let i = 0; i < b[1]; i++) { const o = 3 + 11 * i; sl.push({ nz: b[o], name: str(b, o + 1, 8) }); } } catch (e) {}
    for (let k = 0; k < 4; k++) { const s = sl && sl[k]; ukBtn(KIT_USR + k, s && s.nz ? s.name + ' (USR' + (k + 1) + ')' : 'USR' + (k + 1) + ' · ' + L('empty'), !!(s && s.nz)); }
    { const a3 = sl && sl[2] && sl[2].nz, a4 = sl && sl[3] && sl[3].nz; ukBtn(KIT_USR + 4, 'USR3+4' + (a3 || a4 ? '' : ' · ' + L('empty')), !!(a3 || a4)); }   /* (4.0.3: one kit over two slots) */
    for (let k = 0; k < 4; k++) ukBtn(KIT_SYN + k, 'SYN' + (k + 1), true);
    if (LV) KIT.upd(LV); })();
  const g = el('div', 'drumpads'); g.style.marginTop = '12px'; c.appendChild(g); KIT.knobs = [];
  PADS.forEach((n, p) => {
    const w = el('div', 'pgroup'); w.style.textAlign = 'center';
    const pl = button('▶ ' + n, () => { noteOut(DRUM_T, p, 110, 1); setTimeout(() => noteOut(DRUM_T, p, 0, 0), 200); }, 'sm'); pl.style.width = '100%';
    const send = () => soon('pad' + p, () => act(11, p, KIT.knobs[p][0].get() & 255, KIT.knobs[p][1].get() & 255).then(() => { noteOut(DRUM_T, p, 110, 1); setTimeout(() => noteOut(DRUM_T, p, 0, 0), 150); }));
    const t = knob({ label: L('tune'), min: -8, max: 7, value: 0, def: 0, bip: true, color: TCOL[DRUM_T], id: 'kit.' + n + '.tune', onChange: send });
    const d = knob({ label: L('dec'), min: -4, max: 3, value: 0, def: 0, bip: true, color: TCOL[DRUM_T], id: 'kit.' + n + '.dec', onChange: send });
    KIT.knobs.push([t, d]); const ks = el('div', 'knobs'); ks.style.justifyContent = 'center'; ks.append(t, d); w.append(pl, ks); g.appendChild(w);
  });
  c.appendChild(button(L('resetPads'), async () => { for (let p = 0; p < 16; p++) await live(11, p, 0, 0); }, 'sm'));
  if (LV) KIT.upd(LV);
}
KIT.upd = st => {
  if (!$('tab_kits').classList.contains('on') || !KIT.knobs.length) return;
  st.pads.forEach((x, p) => { KIT.knobs[p][0].set(x.tune); KIT.knobs[p][1].set(x.dec); });
  const cur = st.t[DRUM_T].preset & 0x7FFF; for (const b of KIT.btns) b.classList.toggle('on', +b.dataset.i === cur);
};
liveHooks.push(st => KIT.upd(st));
