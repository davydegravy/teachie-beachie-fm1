/* SPDX-License-Identifier: GPL-3.0-only
 * The web editor (web/editor.html) in Chromium, talking to the PC twin (build/twin serve: SLOOP 3's firmware on a
 * simulated FM-1) through a Web MIDI stand-in: every page opened, its main actions done, the FM-1's state checked
 * after them, no error in the page. Screenshots in OUT.   node tests/editor_e2e.js web/editor.html OUT */
const { chromium } = require('playwright');
const { spawn } = require('child_process');
const path = require('path'), fs = require('fs');
const page_ = path.resolve(process.argv[2] || 'web/editor.html'), OUT = path.resolve(process.argv[3] || 'build/e2e');
fs.mkdirSync(OUT, { recursive: true });
let fails = 0; const check = (c, w) => { console.log((c ? 'ok   ' : 'FAIL ') + w); if (!c) fails++; };
(async () => {
  const tw = spawn('./build/twin', ['serve', OUT + '/twin', '-', '1'], { env: { ...process.env, ASAN_OPTIONS: 'detect_leaks=0:handle_segv=0:allow_user_segv_handler=1' } });
  fs.mkdirSync(OUT + '/twin', { recursive: true });
  let buf = '', ready, readyP = new Promise(r => ready = r), deliver = null, said = [];
  tw.stdout.on('data', d => { buf += d; let i; while ((i = buf.indexOf('\n')) >= 0) { const l = buf.slice(0, i); buf = buf.slice(i + 1);
    if (l === 'READY') ready(); else if (l.startsWith('M ')) { if (deliver) deliver(l.slice(2).split(' ').map(h => parseInt(h, 16))); } else if (/^\[ *[0-9.]+\]/.test(l)) said.push(l.replace(/^\[ *[0-9.]+\] */, '')); } });
  tw.stderr.on('data', d => process.stderr.write(d));
  await readyP;
  const br = await chromium.launch(); const pg = await br.newPage({ viewport: { width: 1400, height: 950 }, deviceScaleFactor: 1 });
  const errors = []; pg.on('pageerror', e => errors.push(String(e))); pg.on('console', m => { if (m.type() === 'error') errors.push(m.text()); });
  await pg.exposeFunction('twinSend', a => tw.stdin.write('M ' + a.map(x => x.toString(16)).join(' ') + '\n'));
  /* the twin's packets, joined into messages for the page */
  let sx = null;
  deliver = bytes => { for (const b of bytes) { if (b === 0xF0) sx = []; if (sx) { sx.push(b); if (b === 0xF7) { const m = sx; sx = null; pg.evaluate(m => window.__twinIn(m), m).catch(() => {}); } } } };
  await pg.addInitScript(() => {
    const inp = { name: 'Felucca', onmidimessage: null }, out = { name: 'Felucca', send: d => window.twinSend(Array.from(d)) };   /* (issue #2: the name a Mac kept from SLOOP 2.8) */
    window.__twinIn = m => { if (inp.onmidimessage) inp.onmidimessage({ data: Uint8Array.from(m) }); };
    navigator.requestMIDIAccess = async () => ({ inputs: new Map([['i', inp]]), outputs: new Map([['o', out]]), onstatechange: null });
    try { localStorage.clear(); } catch (e) {}
  });
  await pg.goto('file://' + page_);
  const shot = n => pg.screenshot({ path: OUT + '/' + n + '.png' });
  const tab = async t => { await pg.click('#tabs button[data-t="' + t + '"]'); await pg.waitForTimeout(700); };
  const LVs = () => pg.evaluate(() => LV && { playing: LV.playing, bpm: LV.bpm, trk: LV.trk, cur: LV.t.map(t => t.cur), level: LV.t.map(t => t.level), mute: LV.t.map(t => t.mute), punch: LV.punch, pads: LV.pads });
  await pg.click('#con'); await pg.waitForTimeout(1500);
  check(await pg.evaluate(() => !!info && info.proto === 2), 'connected, protocol 2');
  check(await pg.evaluate(() => NAMES.presets.length > 50 && NAMES.kits.length > 0 && NAMES.user.length === 32 && NAMES.kinds.length >= 9), 'the lists read (NAMES)');
  /* LIVE */
  await tab('live'); await shot('live-0');
  await pg.click('#tab_live .btn.big.grn'); await pg.waitForTimeout(900);
  let s = await LVs(); check(s && s.playing, 'LIVE: PLAY plays');
  await pg.evaluate(() => act(13, 1333 & 255, 1333 >> 8, 255)); await pg.waitForTimeout(400); s = await LVs(); check(Math.abs(s.bpm - 133.3) < 0.05, 'LIVE: the tempo set (' + s.bpm + ')');
  const pad = await pg.$$('#tab_live .pgrid .pad'); await pad[2].click(); await pg.waitForTimeout(300);   /* track 1, pattern 3 queued */
  s = await LVs(); check(s.cur[0] === 2 || (await pg.evaluate(() => LV.t[0].next)) === 2, 'LIVE: a pattern queued');
  await pg.evaluate(() => act(6, 0, 0)); await pg.waitForTimeout(200);
  const fx = await pg.$$('#tab_live .fxpads .pad'); await fx[4].scrollIntoViewIfNeeded(); const b = await fx[4].boundingBox(); await pg.mouse.move(b.x + 10, b.y + 10); await pg.mouse.down(); await pg.waitForTimeout(400);
  s = await LVs(); check(s.punch !== 255, 'LIVE: a punch-in held (STUT)'); await pg.mouse.up(); await pg.waitForTimeout(400); s = await LVs(); check(s.punch === 255, 'LIVE: the punch-in let go');
  /* a strip's fader dragged */
  const fd = await pg.$('#tab_live .strip .fader'); await fd.scrollIntoViewIfNeeded(); const fb = await fd.boundingBox(); await pg.mouse.move(fb.x + 17, fb.y + 40); await pg.mouse.down(); await pg.mouse.move(fb.x + 17, fb.y + 120, { steps: 6 }); await pg.mouse.up(); await pg.waitForTimeout(500);
  s = await LVs(); check(s.level[0] < 70, 'LIVE: track 1\'s fader down (' + s.level[0] + ')');
  await pg.click('#tab_live .strip .btn.red'); await pg.waitForTimeout(400); s = await LVs(); check(s.mute[0] === 1, 'LIVE: M mutes'); await pg.click('#tab_live .strip .btn.red'); await pg.waitForTimeout(400);
  /* scenes: stored, changed, recalled */
  await pg.click('#tab_live .scene >> nth=0', { modifiers: ['Shift'] }); await pg.evaluate(() => act(4, 1, 0, 20)); await pg.waitForTimeout(300);
  await pg.click('#tab_live .scene >> nth=0'); await pg.waitForTimeout(800); s = await LVs(); check(s.level[1] !== 20, 'LIVE: a scene recalled (' + s.level[1] + ')');
  /* keys: the computer's keyboard on the FM-1's keys, the track chosen */
  await pg.evaluate(() => act(9, 0)); await pg.keyboard.down('KeyQ'); await pg.waitForTimeout(200); await pg.keyboard.up('KeyQ'); await pg.waitForTimeout(200);
  await pg.keyboard.press('ArrowRight'); await pg.waitForTimeout(400); s = await LVs(); check(s.trk === 1, 'LIVE: → chooses the next track (' + s.trk + ')');
  await pg.evaluate(() => { learnMode = true; learnSel = 'master.djf'; ctlIn({ data: [0xB0, 21, 127] }); learnMode = false; learnSel = null; ctlIn({ data: [0xB0, 21, 0] }); }); await pg.waitForTimeout(600);
  check(await pg.evaluate(() => LV.mfilt === 0 && learnMap['cc0:21'] === 'master.djf'), 'LIVE: MIDI learn (a CC on the DJ filter)');
  await pg.evaluate(() => ctlIn({ data: [0xB0, 21, 64] })); await pg.waitForTimeout(300);
  await shot('live-1');
  /* SOUND */
  await tab('sound'); await pg.waitForTimeout(800); await shot('sound');
  check(await pg.evaluate(() => SND.s && SND.knobs.filter(Boolean).length > 10), 'SOUND: the values as knobs');
  const before = await pg.evaluate(() => SND.s.v.slice());
  await pg.evaluate(() => randomize()); await pg.waitForTimeout(1200);
  const after = await pg.evaluate(async () => (await getSound(SND.t)).v);
  check(after.some((v, i) => v !== before[i]), 'SOUND: RANDOM heard on the FM-1');
  const nItems = await pg.$$eval('#tab_sound .list div:not(.hd)', d => d.length); check(nItems > 20, 'SOUND: the browser lists sounds (' + nItems + ')');
  await pg.click('#tab_sound .list div:not(.hd) >> nth=3'); await pg.waitForTimeout(900);
  /* keep among yours, then the lists know it */
  await pg.evaluate(() => keepSound(5, 'E2E SOUND', 3)); await pg.waitForTimeout(3500);
  check(await pg.evaluate(() => NAMES.user[5] && NAMES.user[5].used && NAMES.user[5].name === 'E2E SOUND'), 'SOUND: kept among your sounds (place 6)');
  /* SEQUENCER */
  await pg.evaluate(() => act(9, 0)); await tab('seq'); await pg.waitForTimeout(800);
  check(await pg.evaluate(() => !!R.P), 'SEQ: the pattern read');
  await pg.evaluate(async () => { R.pt = 5; await rollLoad(); });
  const cv = await pg.$('#tab_seq canvas.roll'); const cb = await cv.boundingBox();
  await pg.mouse.click(cb.x + 58 + 30, cb.y + 100); await pg.waitForTimeout(200); await pg.mouse.click(cb.x + 58 + 200, cb.y + 60); await pg.waitForTimeout(800);
  let P = await pg.evaluate(async () => (await patRead(R.t, 5)).notes.length); check(P === 2, 'SEQ: two notes drawn, on the FM-1 (' + P + ')');
  await pg.evaluate(() => { R.sel = new Set([R.P.notes[0]]); drawInspector(); });
  /* (3.4.2, issue #31: the condition and ratchet lists closed at once) while it plays the roll is drawn again at each
   * news from the FM-1: the inspector's lists stay the same elements (an open one stays open) */
  { const same = await pg.evaluate(async () => { const s0 = R.insp.querySelector('select'); await live(5, 2); await new Promise(r => setTimeout(r, 1500));
      const s1 = R.insp.querySelector('select'), playing = LV && LV.playing; await live(5, 2); return !!s0 && s0 === s1 && s0.isConnected; });
    check(same, 'SEQ: the inspector\'s lists stay while it plays (#31)'); }
  await pg.evaluate(() => { const n = R.P.notes[0]; snapshot(); n.cond = 0x45; n.rat = 3 | SEQ_ACCENT; R.P.locks = [{ step: 1, param: 48, value: 99 }]; rollSend(true); }); await pg.waitForTimeout(900);
  P = await pg.evaluate(async () => await patRead(R.t, 5)); check(P.notes.some(n => n.cond === 0x45 && n.rat === (3 | 0x20)) && P.locks.length === 1 && P.locks[0].value === 99, 'SEQ: condition, ratchet, accent and a lock on the FM-1');
  await pg.evaluate(() => { snapshot(); R.P.notes = []; R.P.locks = []; rollSend(true); }); await pg.waitForTimeout(700);
  await pg.evaluate(() => { const o = JSON.parse(R.hist.pop()); R.P.notes = o.n; R.P.locks = o.l; rollSend(true); }); await pg.waitForTimeout(700);
  P = await pg.evaluate(async () => (await patRead(R.t, 5)).notes.length); check(P === 2, 'SEQ: undo sends the notes back');
  await shot('seq');
  /* the drums grid */
  await pg.evaluate(async () => { R.t = DRUM_T; R.pt = 6; drawSeq(); await rollLoad(); }); await pg.waitForTimeout(400);
  const cv2 = await pg.$('#tab_seq canvas.roll'); const c2 = await cv2.boundingBox(); await pg.mouse.click(c2.x + 70, c2.y + 10); await pg.waitForTimeout(700);
  P = await pg.evaluate(async () => (await patRead(DRUM_T, 6)).notes); check(P.length === 1 && P[0].note === 15, 'SEQ: a drum hit on the grid (' + JSON.stringify(P) + ')'); await shot('seq-drums');
  /* a MIDI file out, then in */
  const mid = await pg.evaluate(() => { let got = null; const keep = save; save = (b, n) => { got = Array.from(b); }; return midiFile(5).then(() => { save = keep; return got; }); });
  check(mid && mid[0] === 0x4D && mid.length > 60, 'SEQ: a MIDI file made');
  await pg.evaluate(async m => { R.t = 1; R.pt = 4; await rollLoad(); await midiImport(Uint8Array.from(m), 'e2e.mid'); }, mid);   /* (its first track: tempo only; then track 1's) */
  await pg.waitForTimeout(800);
  P = await pg.evaluate(async () => (await patRead(1, 4)).notes.length); check(P >= 1, 'SEQ: a MIDI file read into a pattern (' + P + ')');
  /* (3.4.0, Sam: imported notes all SOFT) a file at velocity 64 and 48: the loudest brought to NORM (100), the other with it */
  await pg.evaluate(async () => { const tk = [0, 0x90, 60, 64, 0x81, 0x40, 0x80, 60, 0, 0, 0x90, 64, 48, 0x81, 0x40, 0x80, 64, 0, 0, 0xFF, 0x2F, 0];
    const f = [0x4D, 0x54, 0x68, 0x64, 0, 0, 0, 6, 0, 0, 0, 1, 0, 96, 0x4D, 0x54, 0x72, 0x6B, 0, 0, 0, tk.length, ...tk];
    R.t = 1; R.pt = 5; await rollLoad(); await midiImport(Uint8Array.from(f), 'soft.mid'); });
  await pg.waitForTimeout(800);
  P = await pg.evaluate(async () => (await patRead(1, 5)).notes.map(n => n.vel)); check(P.length === 2 && P[0] === 100 && P[1] === 75, 'SEQ: a soft MIDI file brought to NORM (' + JSON.stringify(P) + ')');
  /* SONG */
  await tab('song'); await pg.evaluate(async () => { SONG.e = [{ bar: 0, mute: 0, pat: [0, 1, 0, 0, 0] }, { bar: 4, mute: 2, pat: [1, 1, 0, 0, 0] }]; SONG.len = 8; await songSend(); }); await pg.waitForTimeout(800);
  check(await pg.evaluate(() => SONG.e.length === 2 && SONG.len === 8 && SONG.e[1].mute === 2), 'SONG: sent and read back'); await shot('song');
  /* MIXER, SETTINGS */
  await tab('mixer'); await shot('mixer');
  await pg.evaluate(() => homeSet(43, 77)); await pg.waitForTimeout(400); check(await pg.evaluate(() => HOME.r[32 + 10 + 1] === 77), 'MIXER: DLY MIX set');
  await tab('settings'); await shot('settings');
  /* 3.3.6: REC when the track changes: STOP by default, KEEP from the editor, read back */
  const rk = await pg.evaluate(async () => { await homeRead(); const c = SET.ctl.find(x => x.id === 49); const v0 = c.s.value; await homeSet(49, 1); const v1 = c.s.value; await homeSet(49, 0); return [v0, v1, c.s.value].join(','); });
  check(rk === '0,1,0', 'SETTINGS: REC when the track changes: STOP, then KEEP, then STOP (' + rk + ')');
  const sel = await pg.$$('#tab_settings select'); check(sel.length >= 18, 'SETTINGS: the settings listed (' + sel.length + ')');
  await pg.evaluate(() => homeSet(4, 1)); await pg.waitForTimeout(300); check(await pg.evaluate(() => HOME.r[4] === 1), 'SETTINGS: SPEAKER on'); await pg.evaluate(() => homeSet(4, 0));
  /* LIBRARY, KITS */
  await tab('library'); await pg.waitForTimeout(1500); await shot('library');
  check(await pg.evaluate(() => LIB.f && recName(LIB.recs[5]) === 'E2E SOUND'), 'LIBRARY: your sounds read');
  await pg.evaluate(async () => { LIB.fav = [3, 1005]; const r = LIB.recs[5]; r.set([0x52, 0x45, 0x4E, 0x41, 0x4D, 0x45, 0x44], 4); for (let i = 11; i < 16; i++) r[i] = 0; await fileWrite(1, 0, usBytes()); await retryBusy(() => readNames()); }); await pg.waitForTimeout(500);
  check(await pg.evaluate(() => NAMES.user[5].name === 'RENAMED'), 'LIBRARY: a sound renamed on the FM-1');
  await tab('kits'); await shot('kits');
  await pg.evaluate(() => act(11, 4, 5, (-3) & 255)); await pg.waitForTimeout(400); s = await LVs(); check(s.pads[4].tune === 5 && s.pads[4].dec === -3, 'KITS: a pad\'s TUNE and DECAY');
  /* SAMPLES, PROJECTS, banks, theme */
  await tab('samples'); await pg.waitForTimeout(500); check(await pg.evaluate(() => SM.slots && SM.slots.length === 4), 'SAMPLES: the slots read'); await shot('samples');
  await tab('projects'); await shot('projects');
  const pj = await pg.evaluate(() => fileRead(2, 0).then(d => d.length)); check(pj > 100, 'PROJECTS: the project now read (' + pj + ' B)');
  /* 3.3.5: the one project: the project now written to the FM-1 (through its flash as it comes), read back the same */
  const rt = await pg.evaluate(async () => { const d = await fileRead(2, 0); await fileWrite(0, 0, d); await retryBusy(() => refreshInfo(), 80);
    const b = await retryBusy(() => fileRead(0, 0), 80); return d.length === b.length && d.every((x, i) => x === b[i]) ? d.length : -1; });
  check(rt > 100, 'PROJECTS: written to the FM-1 and read back the same (' + rt + ' B)');
  /* an old backup (8 places, 3.0 to 3.3.4): the one asked for goes on the FM-1; the backup of 3.3.5 holds the one project */
  const ob = await pg.evaluate(async () => {
    const d = await fileRead(2, 0), parts = [[0, 1, d], [0, 4, d.slice()]];
    let n = 8; for (const p of parts) n += 6 + p[2].length;
    const f = new Uint8Array(n); f.set([0x53, 0x33, 0x42, 0x4B, 1, parts.length, 0, 0]); let o = 8;
    for (const [k, s, x] of parts) { f.set([k, s, ...w32(x.length)], o); f.set(x, o + 6); o += 6 + x.length; }
    const asked = []; window.prompt = (q, def) => { asked.push(def); return '5'; }; window.confirm = () => true;
    await restoreAll(f); await retryBusy(() => refreshInfo(), 80);
    const b = await retryBusy(() => fileRead(0, 0), 80);
    let got = null; const keep = window.save; window.save = (x) => { got = x; }; await backupAll(); window.save = keep;
    const ok = asked[0] === '5' && b.length === d.length && got && got[5] >= 1 && got[8] === 0 && got[9] === 0;
    return ok ? 1 : JSON.stringify({ asked, b: b.length, d: d.length, got: got && Array.from(got.slice(0, 10)), log: $('log') ? $('log').textContent.slice(-300) : '' }); });
  if (ob !== 1) console.log(ob);
  check(ob === 1, 'PROJECTS: an old backup of 8 places: the one chosen written; BACKUP ALL holds the one project');
  /* 3.3.6: a BREAK: 2 bars at 100 BPM, a hit on each 8th (kick, hat, snare...) as a WAV; AUTO finds the 16 hits; sent to
   * USR2: 16 zones; SLICE on track 2 (SRC USR2), the tempo 100, the pattern: slice k on its 8th */
  await tab('samples'); await pg.waitForTimeout(300);
  const bk = await pg.evaluate(async () => {
    const R = 22050, dur = 2 * 4 * 60 / 100, n = Math.round(dur * R), x = new Float32Array(n), hits = [];
    let seed = 7; const rnd = () => (seed = (seed * 16807) % 2147483647) / 2147483647 - 0.5;
    for (let h = 0; h < 16; h++) { const at = Math.round(h * n / 16); hits.push(at); const kind = h % 4;
      for (let i = 0; i < 4000 && at + i < n; i++) { const e = Math.exp(-i / (kind === 0 ? 1500 : 500)); x[at + i] += e * (kind === 0 ? Math.sin(i * 2 * Math.PI * 60 / R) * 0.9 : rnd() * (kind === 2 ? 0.8 : 0.4)); } }
    const f = new Uint8Array(44 + 2 * n), dv = new DataView(f.buffer), w = (o, t) => { for (let i = 0; i < 4; i++) f[o + i] = t.charCodeAt(i); };
    w(0, 'RIFF'); dv.setUint32(4, 36 + 2 * n, true); w(8, 'WAVE'); w(12, 'fmt '); dv.setUint32(16, 16, true); dv.setUint16(20, 1, true); dv.setUint16(22, 1, true);
    dv.setUint32(24, R, true); dv.setUint32(28, 2 * R, true); dv.setUint16(32, 2, true); dv.setUint16(34, 16, true); w(36, 'data'); dv.setUint32(40, 2 * n, true);
    for (let i = 0; i < n; i++) dv.setInt16(44 + 2 * i, Math.max(-32768, Math.min(32767, Math.round(x[i] * 32767))), true);
    SM.mode = 2; SM.slot = 1; SM.tgt = 1; SM.auto = 16; SM.name = 'SAMPLE';
    await loadFiles([new File([f], 'AMEN TEST.wav', { type: 'audio/wav' })]);
    const cuts = SM.cuts.slice(), near = cuts.filter((c, k) => Math.abs(c - hits[k + 1]) < 300).length;
    window.confirm = () => true;
    await uploadSamples(); await sleep(1500); await readSlots();
    const s = await getSound(1), P = await patRead(1, 255); await live(0);
    return { cuts: cuts.length, near, bars: SM.bars, bpm: +bpmOf().toFixed(1), nz: SM.slots[1].nz, name: SM.slots[1].name, eng: ENGINES[s.e], src: s.v[0],
      notes: P.notes.length, steps: P.steps, at5: P.notes[5] && P.notes[5].at, n5: P.notes[5] && P.notes[5].note, tempo: LV && LV.bpm };
  });
  console.log('  break:', JSON.stringify(bk));
  check(bk.cuts === 15 && bk.near === 15, 'BREAK: AUTO finds the 16 hits (' + bk.near + ' of ' + bk.cuts + ' cuts on them)');
  check(bk.bars === 2 && Math.abs(bk.bpm - 100) < 0.2, 'BREAK: 2 bars, 100 BPM (' + bk.bars + ', ' + bk.bpm + ')');
  check(bk.nz === 16 && bk.name === 'AMEN TES', 'BREAK: sent as 16 zones (' + bk.nz + ', ' + bk.name + ')');
  check(bk.eng === 'SLICE' && bk.src === 2, 'BREAK: SLICE on track 2, SRC USR2 (' + bk.eng + ', ' + bk.src + ')');
  check(bk.notes === 16 && bk.steps === 32 && Math.abs(bk.at5 - 5 * 2 * 96) <= 6 && bk.n5 === 65, 'BREAK: its pattern, slice k on its 8th (' + bk.notes + ' notes, ' + bk.steps + ' steps, ' + bk.at5 + ', ' + bk.n5 + ')');
  await shot('samples-break');
  /* the same break to the drum track instead: USR3 as its kit, the pattern on its pads (slice k on pad k) */
  const bd2 = await pg.evaluate(async () => {
    SM.slot = 2; SM.tgt = DRUM_T; drawSamples(); await uploadSamples(); await sleep(1500); await readSlots();
    const s = await getSound(DRUM_T), P = await patRead(DRUM_T, 255), s2 = await getSound(1);
    /* (3.3.6) one of YOUR KITS, named by the sample: another kit chosen, then this one found again */
    const nameNow = s.name, pre = LV.t[DRUM_T].preset & 0x7FFF;
    const techno = NAMES.presets.find(p => p.name === 'TECHNO'); await live(10, DRUM_T, techno.i & 255, techno.i >> 8);
    const away = (await getSound(DRUM_T)).v[0]; await live(10, DRUM_T, (3000 + 39) & 255, (3000 + 39) >> 8);
    const back = await getSound(DRUM_T);
    return { nameNow, pre, away, backKit: back.v[0], backName: back.name, nz: SM.slots[2].nz, kit: NAMES.kits[s.v[0]] && NAMES.kits[s.v[0]].name, notes: P.notes.length, n5: P.notes[5] && P.notes[5].note, at5: P.notes[5] && P.notes[5].at, synth: ENGINES[s2.e], src: s2.v[0],
      opts: [...document.querySelectorAll('#tab_samples select')].some(x => [...x.options].some(o => /pads/.test(o.textContent))) };
  });
  console.log('  break on the drums:', JSON.stringify(bd2));
  check(bd2.opts && bd2.nz === 16 && bd2.kit === 'USR3', 'BREAK: on the drum track, USR3 its kit (' + bd2.kit + ')');
  check(bd2.notes === 16 && bd2.n5 === 5 && Math.abs(bd2.at5 - 960) <= 6, 'BREAK: the drums\' pattern, slice k on pad k (' + bd2.notes + ', pad ' + bd2.n5 + ' at ' + bd2.at5 + ')');
  check(bd2.synth === 'SLICE' && bd2.src === 2, 'BREAK: track 2 kept as it was (SLICE, USR2)');
  check(bd2.pre === 3039 && bd2.nameNow === 'AMEN TES', 'BREAK: the drums play one of YOUR KITS, named by the sample (' + bd2.pre + ', ' + bd2.nameNow + ')');
  check(bd2.away !== 39 && bd2.backKit === 39 && bd2.backName === 'AMEN TES', 'BREAK: TECHNO chosen, then the break\'s kit found again (' + bd2.away + ' -> ' + bd2.backKit + ', ' + bd2.backName + ')');
  await tab('kits'); await pg.waitForTimeout(800);
  check(await pg.evaluate(() => KIT.btns.some(b => b.textContent.startsWith('AMEN TES') && !b.disabled)), 'DRUM KIT: YOUR KITS shows the break by its name');
  await shot('kits-yours');
  /* 4.0.3: A KIT FROM SAMPLES (as SLOOP 2.8.5's Drum kit page): files onto the pads by their names, PITCH, MATCH LEVELS,
   * USR3+4 over two slots, the drum track on it (KIT USR3+4), FIT THE SLOT for one slot, a ZIP out and back */
  await tab('mkkit'); await pg.waitForTimeout(400);
  const mk = await pg.evaluate(async () => {
    const R = 22050, wav = (sec, f, amp, noise) => { const n = Math.round(sec * R), x = new Float32Array(n); let sd = 3;
      for (let i = 0; i < n; i++) { const e = Math.exp(-i / (R * Math.min(sec, 0.3) / 3)); x[i] = amp * e * (noise ? ((sd = (sd * 16807) % 2147483647) / 2147483647 - 0.5) * 2 : Math.sin(2 * Math.PI * f * i / R)); }
      const b = new Uint8Array(44 + 2 * n), dv = new DataView(b.buffer), w = (o, t) => { for (let i = 0; i < 4; i++) b[o + i] = t.charCodeAt(i); };
      w(0, 'RIFF'); dv.setUint32(4, 36 + 2 * n, true); w(8, 'WAVE'); w(12, 'fmt '); dv.setUint32(16, 16, true); dv.setUint16(20, 1, true); dv.setUint16(22, 1, true);
      dv.setUint32(24, R, true); dv.setUint32(28, 2 * R, true); dv.setUint16(32, 2, true); dv.setUint16(34, 16, true); w(36, 'data'); dv.setUint32(40, 2 * n, true);
      for (let i = 0; i < n; i++) dv.setInt16(44 + 2 * i, Math.round(x[i] * 32000), true); return b; };
    const fs = [['BD 808.wav', 0.6, 55, 1, 0], ['Snare_01.wav', 0.3, 200, 0.5, 1], ['hh closed.wav', 0.1, 0, 0.3, 1], ['OH open.wav', 0.5, 0, 0.3, 1], ['clap.wav', 0.3, 0, 0.8, 1],
      ['pad one.wav', 5, 220, 0.6, 0], ['pad two.wav', 5, 330, 0.6, 0]].map(([n, s, f, a, z]) => new File([wav(s, f, a, z)], n, { type: 'audio/wav' }));
    KD.pads = Array(16).fill(null); await kAdd(fs);
    const at = KD.pads.map(p => p && p.fname);
    const l0 = kLen(KD.pads[2]); KD.pads[2].tune = 12; const l1 = kLen(KD.pads[2]);
    await kMatch(); const gains = KD.pads.filter(Boolean).map(p => p.gain);
    KD.dest = 0; const over1 = kOver(); kFit(KD.pads, KROOM, 1); const over1b = kOver();
    for (const p of KD.pads) if (p) p.len = 0;                      /* (back to whole, for the pair) */
    KD.dest = KPAIR; KD.name = 'E2E KIT'; const split = kSplit(KD.pads);
    window.confirm = () => true; await kSend(); await sleep(800); await kReadSlots();
    await kUse(); await sleep(300); await live(0); const s = await getSound(DRUM_T);
    const z = await zipIn(zipOut([{ name: 'K/01 KICK.wav', data: kWav(await kRender(KD.pads[0])) }, { name: 'K/03 SNARE.wav', data: kWav(await kRender(KD.pads[2])) }]));
    const back = Array(16).fill(null), save_ = KD.pads; KD.pads = back; await kAdd(z.map(x => new File([x.data], x.name.split('/').pop()))); const zback = KD.pads.map(p => p && p.fname); KD.pads = save_; drawMkKit();
    return { at, l0, l1, gains, over1, over1b, split, n3: KD.slots[2].nz, n4: KD.slots[3].nz, name3: KD.slots[2].name, pre: LV.t[DRUM_T].preset & 0x7FFF, kit: s.v[0], zn: z.length, zback };
  });
  console.log('  kit from samples:', JSON.stringify(mk));
  check(mk.at[0] === 'BD 808.wav' && mk.at[2] === 'Snare_01.wav' && mk.at[4] === 'hh closed.wav' && mk.at[5] === 'OH open.wav' && mk.at[3] === 'clap.wav' && mk.at[1] === 'pad one.wav', 'KIT FROM SAMPLES: the files on the pads by their names');
  check(Math.abs(mk.l1 - mk.l0 / 2) < 3, 'KIT FROM SAMPLES: PITCH +12, the sound half as long (' + mk.l0 + ' -> ' + mk.l1 + ')');
  check(mk.gains.every(g => g <= 0 && g >= -24) && mk.gains.some(g => g === 0) && mk.gains.some(g => g < 0), 'KIT FROM SAMPLES: MATCH LEVELS (' + mk.gains.join(' ') + ')');
  check(mk.over1 && !mk.over1b, 'KIT FROM SAMPLES: one slot too small, FIT THE SLOT');
  check(mk.split && mk.n3 > 0 && mk.n4 > 0 && mk.n3 + mk.n4 === 7 && mk.name3 === 'E2E KIT', 'KIT FROM SAMPLES: USR3+4, the pads over the two slots (' + mk.n3 + ' + ' + mk.n4 + ')');
  check(mk.pre === 3041 && mk.kit === 41, 'KIT FROM SAMPLES: the drum track on USR3+4 (' + mk.pre + ', kit ' + mk.kit + ')');
  check(mk.zn === 2 && mk.zback[0] === '01 KICK.wav' && mk.zback[2] === '03 SNARE.wav', 'KIT FROM SAMPLES: a ZIP out and back on its pads');
  await shot('mkkit');
  await tab('drumsynth'); await pg.click('#tab_drumsynth .tools .btn >> nth=0'); await pg.waitForTimeout(1200); check(await pg.evaluate(() => !!Y.img), 'DRUM SYNTH: the kits read'); await shot('drumsynth');
  await tab('fm6'); await pg.click('#tab_fm6 .tools .btn >> nth=0'); await pg.waitForTimeout(1500); check(await pg.evaluate(() => !!F.img), 'FM6: the bank read'); await shot('fm6');
  /* 3.2: a .syx loaded with the bank not read yet: read first, the file's voices put in B1.. at once; saved; then on
   * the FM-1 a voice is a sound (2000 + k) that a track takes, and the editor names it */
  {
    const syx = Buffer.alloc(4104); syx.set([0xF0, 0x43, 0, 9, 0x20, 0]);
    for (let k = 0; k < 32; k++) { const b = 6 + 128 * k;
      for (let o = 0; o < 6; o++) syx.set([99, 60, 40, 60, 99, 90, 80, 0, 0, 0, 0, 0, 56, 0, o < 2 ? 99 : 70, (1 + o) << 1, 0], b + 17 * o);
      syx.set([99, 99, 99, 99, 50, 50, 50, 50, 31, 8, 0, 0, 0, 0, 0, 24], b + 102); syx.write(('E2E VC ' + (k + 1)).padEnd(10), b + 118, 'latin1'); }
    let cs = 0; for (let i = 6; i < 4102; i++) cs += syx[i]; syx[4102] = (128 - (cs & 127)) & 127; syx[4103] = 0xF7;
    require('fs').writeFileSync(OUT + '/e2e.syx', syx);
    await pg.evaluate(() => { F.img = null; F.used = 0; F.file = []; drawFm6(); });
    await pg.click('#tab_fm6 .tools .btn >> nth=1'); await pg.setInputFiles('#file', OUT + '/e2e.syx'); await pg.waitForTimeout(2500);
    check(await pg.evaluate(() => !!F.img && F.used === 0x7FFFFFF && F.dirty && fName(4) === 'E2E VC 5'), 'FM6: a .syx placed in the empty bank at once'); await shot('fm6-syx');
    await pg.click('#tab_fm6 .tools .btn >> nth=2'); await pg.waitForTimeout(3000);
    check(await pg.evaluate(() => !F.dirty && F.src === 2), 'FM6: the bank saved');
    await pg.evaluate(() => live(10, 0, 2004 & 255, 2004 >> 8)); await pg.waitForTimeout(700);
    check(await pg.evaluate(() => LV && (LV.t[0].preset & 0x7FFF) === 2004 && soundName(LV.t[0]) === 'E2E VC 5'), 'FM6: voice 5 on track 1, named (' + await pg.evaluate(() => LV && LV.t[0].preset + ' ' + soundName(LV.t[0])) + ')');
  }
  await tab('theme'); await shot('theme');
  /* 4.0.8: the 5 tracks' colours (synth 4 at bytes 34-35), sent and read back; the previews drawn */
  const th = await pg.evaluate(async () => {
    TH.trk = ['#ff0000', '#00ff00', '#0000ff', '#ff00ff', '#00ffff']; drawTheme();
    await ask(ED.THEME, themeBytes()); await sleep(1500);
    TH.trk = ['#000000', '#000000', '#000000', '#000000', '#000000'];
    const b = await ask(ED.THEME, []); const t = b.slice(1, 41);
    const got = TH_AT.map(a => from565(u16(t, a)));
    TH.trk = ['#287cff', '#1ecc70', '#ffc618', '#f06ec8', '#ff621a'];   /* (the defaults again, for the picture) */
    const cv = document.querySelectorAll('#tab_theme canvas');
    return { got, cv: cv.length, w: cv[0] && cv[0].width };
  });
  console.log('  theme:', JSON.stringify(th));
  check(th.got.join() === '#ff0000,#00ff00,#0000ff,#ff00ff,#00ffff' && th.cv === 2 && th.w === 480, 'THEME: the 5 tracks\' colours sent and read back, the 2 previews');
  await pg.evaluate(() => drawTheme()); await pg.waitForTimeout(300); await shot('theme-5');
  /* the language, the views */
  await pg.click('#b_lang'); await pg.waitForTimeout(300); check(await pg.$eval('#tabs button[data-t="sound"] span', e => e.textContent) === (await pg.evaluate(() => lang === 'fr' ? 'Son' : 'Sound')), 'FR / EN');
  await pg.click('#b_view'); await tab('live'); await shot('live-simple');
  await pg.evaluate(() => act(5, 0));
  check(!errors.length, 'no error in the page' + (errors.length ? ': ' + errors.slice(0, 5).join(' | ') : ''));
  tw.stdin.write('S lcd-end\nQ\n'); await new Promise(r => setTimeout(r, 500));
  await br.close(); tw.kill();
  console.log('editor e2e: ' + fails + ' failed'); process.exit(fails ? 1 : 0);
})().catch(e => { console.error(e); process.exit(2); });
