/* SPDX-License-Identifier: GPL-3.0-only
 * Teachie Beachie: the lessons sent from the installer page. tools/teachie_page.py puts this at the end of the page's
 * script, with the pack of tools/mkcourse.py inside. By now the FM-1 runs Teachie Beachie (not m-bootloader's MIDI
 * MODE): it is found by asking each FM-1 for its course (ED_GUIDE 6, app/edproto.h), then the pack goes into a user
 * sample slot with the editor's sample messages (ED_SAMPLE: 1 begin, 2 data, 3 the header), as
 * tutorial/device.py --install does. The slot: the last one that holds no samples (the FM-1 looks for its lessons
 * from the last slot down, fw/course.inc); if all four hold samples, the user chooses the one to give up.
 * It uses the page's own $, show, problem, log, unpack, midi, UPD, T, W, go, done6 and busy. Written for this project. */
const TP_PACK = Uint8Array.from(atob("__TP_PACK__"), c => c.charCodeAt(0));   /* the lessons */
const TP_HEAD = Uint8Array.from(atob("__TP_HEAD__"), c => c.charCodeAt(0));   /* their slot's header: 480 bytes, the length and the CRC inside */
const TP_N = __TP_N__;                                                        /* how many lessons */
const TP_SAMPLE = 13, TP_GUIDE = 21;                                          /* ED_SAMPLE, ED_GUIDE */
const TP_FM1 = /SLOOP|Felucca|CRITICAL|FM-1/i;        /* a FM-1 running a firmware, by its port's name (MIDI MODE's, UPD, left out) */
const TP = { out: null, inp: null, wait: null, busy: false, sending: false, patience: 30000 };
const TP_WHY = { 1: 'the FM-1 refused them', 2: 'the FM-1 stayed busy', 4: 'they arrived damaged', 6: 'the FM-1 could not write its memory' };
const tpSleep = ms => new Promise(r => setTimeout(r, ms));
function tpFrame(cmd, raw) {                          /* F0 7D 53 33 cmd, the bytes 8 to 7, F7 */
  const o = [0xF0, 0x7D, 0x53, 0x33, cmd]; let acc = 0, bits = 0;
  for (const b of raw) { acc |= b << bits; bits += 8; while (bits >= 7) { o.push(acc & 127); acc >>= 7; bits -= 7; } }
  if (bits) o.push(acc & 127); o.push(0xF7); return o;
}
function tpIn(e) {                                    /* an answer: the command asked, or 0, a refusal */
  const d = e.data, w = TP.wait;
  if (!w || d.length < 6 || d[0] !== 0xF0 || d[1] !== 0x7D || d[2] !== 0x53 || d[3] !== 0x33 || (d[4] !== w.cmd && d[4] !== 0)) return;
  TP.wait = null; w.done(d[4], unpack(d.slice(5, d.length - 1)));
}
function tpAsk(cmd, raw, ms) {
  return new Promise((res, rej) => {
    const t = setTimeout(() => { TP.wait = null; rej(new Error('no answer')); }, ms || 8000);
    TP.wait = { cmd, done: (c, b) => { clearTimeout(t); if (c) res(b); else { const e = new Error(TP_WHY[b[0]] || 'the FM-1 refused (' + b[0] + ')'); e.code = b[0] || -1; rej(e); } } };
    try { TP.out.send(tpFrame(cmd, raw)); } catch (e) { clearTimeout(t); TP.wait = null; rej(e); }
  });
}
/* an answer that can be lost or late, safe to ask twice: up to four times */
async function tpTry(cmd, raw) { for (let n = 0; ; n++) { try { return await tpAsk(cmd, raw, 400); } catch (e) { if (e.code || n >= 3) throw e; } } }
/* the FM-1's own course: 6, its phase, the lesson, the step, how many lessons it holds (an answer to another question, come late, is passed over) */
async function tpCourse() { for (let n = 0; ; n++) { const b = await tpTry(TP_GUIDE, [6]); if (b[0] === 6 && b.length >= 5) return b; if (n >= 3) throw new Error('no answer'); } }
/* a slot's message; the FM-1 busy saving (2): again until it is done */
async function tpJob(raw) {
  for (const end = Date.now() + 30000; ;) {
    try { return await tpAsk(TP_SAMPLE, raw); } catch (e) { if (e.code !== 2 || Date.now() > end) throw e; await tpSleep(50); }
  }
}
/* the FM-1 that runs Teachie Beachie: 1 found (TP.out, TP.inp), 0 none, -1 a FM-1 answers but knows no course */
async function tpFind() {
  const on = p => TP_FM1.test(p.name) && !UPD.test(p.name) && p.state !== 'disconnected';
  const ins = [...midi.inputs.values()].filter(on); let other = false;
  for (const o of [...midi.outputs.values()].filter(on)) {
    const i = ins.find(x => x.name === o.name) || ins[0]; if (!i) continue;
    if (TP.inp && TP.inp !== i) TP.inp.onmidimessage = null;
    TP.out = o; TP.inp = i; i.onmidimessage = tpIn;
    try { await tpCourse(); return 1; } catch (e) { if (e.code) other = true; }
  }
  if (TP.inp) TP.inp.onmidimessage = null;
  TP.out = TP.inp = null;
  return other ? -1 : 0;
}
function tpBar(f) { const p = Math.floor(f * 100); $('tpFill').style.width = p + '%'; $('tpPct').textContent = p + ' %'; }
/* all four slots hold samples: the user chooses the one the lessons replace */
function tpChoose(slots) {
  const box = $('tpSlots'); box.innerHTML = '';
  slots.forEach((s, i) => {
    const b = document.createElement('button'), t = document.createElement('b'), d = document.createElement('span');
    b.className = 'choice'; t.textContent = 'USR' + (i + 1) + ' · ' + (s.name || '?');
    d.textContent = s.n + (s.n === 1 ? ' sample, ' : ' samples, ') + s.kb + ' KB. Replace it with the lessons.';
    b.appendChild(t); b.appendChild(d);
    b.onclick = () => { if (!W.confirm || W.confirm('Replace the samples in USR' + (i + 1) + ' (' + (s.name || '?') + ') with the lessons? They are erased from the FM-1.')) tpSend(i); };
    box.appendChild(b);
  });
  show('tpPick', true);
}
async function tpSend(slot) {
  if (TP.busy || busy()) return;
  TP.busy = true; $('tpSend').disabled = true;
  for (const id of ['tpErr', 'tpDone', 'tpPick', 'tpProg']) show(id, false);
  let sent = false;
  try {
    if (!navigator.requestMIDIAccess) { problem('tpErr', T.nomidi, ''); return; }
    try { midi = midi || await navigator.requestMIDIAccess({ sysex: true }); }
    catch (e) { problem('tpErr', T.denied, 'Click the icon left of the address, allow MIDI devices for this site, and click Send the lessons again.'); return; }
    /* the FM-1, which may still be restarting after the install */
    show('tpWait', true);
    let found = 0;
    for (const end = Date.now() + TP.patience; ;) { found = await tpFind(); if (found > 0 || Date.now() > end) break; await tpSleep(500); }
    show('tpWait', false);
    if (found <= 0) {
      const menu = [...midi.outputs.values()].some(p => UPD.test(p.name) && p.state !== 'disconnected');
      if (found < 0) problem('tpErr', 'This FM-1 runs SLOOP 3 without Teachie Beachie.', 'Install Teachie Beachie first (steps 1 to 5), then send the lessons.');
      else if (menu) problem('tpErr', 'The FM-1 is still in MIDI MODE.', 'Switch it off, then on without holding PLAY, and click Send the lessons again.');
      else problem('tpErr', 'No FM-1 running Teachie Beachie was found.', 'Switch it on without holding PLAY, plug it straight into the computer, close other music apps, and click Send the lessons again.');
      return;
    }
    try { await tpTry(TP_GUIDE, [5]); } catch (e) {}    /* the start's card: a lesson running on the FM-1 stops (it reads the slot) */
    const r = await tpTry(TP_SAMPLE, [0]), slots = [];
    for (let i = 0; i < r[1]; i++) {
      const o = 3 + 11 * i; let nm = ''; for (let j = 0; j < 8 && r[o + 1 + j]; j++) nm += String.fromCharCode(r[o + 1 + j]);
      slots.push({ n: r[o], name: nm, kb: r[o + 9] | r[o + 10] << 8 });
    }
    let k = slot;
    if (k === undefined) { k = -1; slots.forEach((s, i) => { if (!s.n) k = i; }); }
    if (k < 0 || k >= slots.length) { tpChoose(slots); return; }
    /* the pack: the slot begun, the data 400 bytes at a time, then its header (the FM-1 checks the length and the CRC) */
    show('tpProg', true); tpBar(0); log('Sending ' + TP_N + ' lessons (' + TP_PACK.length + ' bytes) to sample slot USR' + (k + 1) + '...');
    sent = TP.sending = true;
    await tpJob([1, k]);
    for (let off = 0; off < TP_PACK.length; off += 400) {
      const c = TP_PACK.subarray(off, off + 400);
      await tpJob([2, k, off & 255, off >> 8 & 255, off >> 16 & 255, off >>> 24, ...c]);
      tpBar((off + c.length) / TP_PACK.length);
    }
    await tpJob([3, k, ...TP_HEAD]);
    await tpSleep(300);
    const got = (await tpCourse())[4];
    TP.sending = false; show('tpProg', false);
    if (got !== TP_N) {
      problem('tpErr', 'The FM-1 took the lessons but does not find them: its Teachie Beachie is not this page\'s.', 'Install Teachie Beachie from this page (steps 1 to 5), then send the lessons again.');
      return;
    }
    try { await tpTry(TP_GUIDE, [5]); } catch (e) {}    /* the start's card again, in the lessons' own letters now */
    $('tpMeta').textContent = TP_N + ' lessons, in sample slot USR' + (k + 1) + '. On the FM-1, press OCT+ to start.';
    show('tpDone', true); log(TP_N + ' lessons on the FM-1, in sample slot USR' + (k + 1) + '.', 'ok');
    tpReady();
  } catch (e) {
    show('tpProg', false);
    problem('tpErr', (sent ? 'The lessons were not all sent: ' : 'The FM-1 did not answer: ') + (e && e.message || e) + '.', 'Leave the FM-1 on and plugged in, and click Send the lessons again.');
  } finally { TP.busy = TP.sending = false; $('tpSend').disabled = false; show('tpWait', false); }
}
$('tpSend').onclick = () => { tpSend(); };
/* the lessons alone, for a FM-1 that runs Teachie Beachie already: the last step opened, without its words about the install */
/* the last step says "ready", and is marked done at the left, only once the lessons are on the FM-1 (SLOOP's page
 * marks it done when the install ends: tpStep puts it back) */
function tpReady() {
  const p = $('p6'); if (p.classList) { p.classList.remove('tp-pending'); p.classList.remove('tp-only'); }
  stepState(6, 'done'); if (p.scrollIntoView) p.scrollIntoView({ block: 'start', behavior: 'smooth' });
}
function tpStep() { const p = $('p6'); if (p.classList && p.classList.contains('tp-pending')) stepState(6, step === 6 ? 'now' : ''); }
const tpGo = go;
go = n => { const p = $('p6'); if (p.classList && (n !== 6 || done6)) p.classList.remove('tp-only'); tpGo(n); tpStep(); setTimeout(tpStep, 0); };
$('tpOnly').onclick = e => { if (e && e.preventDefault) e.preventDefault(); if (busy()) return; const p = $('p6'); if (p.classList) p.classList.add('tp-only'); go(6); };
if (W.addEventListener) W.addEventListener('beforeunload', e => { if (TP.sending) { const s = 'The lessons are being sent. Leaving now interrupts it.'; e.preventDefault(); e.returnValue = s; return s; } });
