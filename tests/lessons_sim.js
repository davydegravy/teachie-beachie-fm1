/* SPDX-License-Identifier: GPL-3.0-only
 * Teachie Beachie: the installer page's last step, the lessons (web/teachie_lessons.js), run in node as
 * tests/install_sim.js runs the page: against a fake FM-1 that answers as Teachie Beachie does (ED_GUIDE, ED_SAMPLE,
 * app/edproto.h), then against the PC twin (build/twin serve: the real firmware on a simulated FM-1), whose flash is
 * read afterwards. Checked: the pack the page carries is PACK (tools/mkcourse.py's), it goes whole into the last slot
 * that holds no samples, a busy FM-1 is asked again, a full FM-1 asks which slot to give up, and each thing that can
 * be wrong (no FM-1, MIDI MODE, SLOOP 3 without Teachie Beachie, an older Teachie Beachie) says so.
 *   node tests/lessons_sim.js PAGE.html [PACK] [OUT] */
const fs = require('fs'), path = require('path'), zlib = require('zlib');
const { spawn } = require('child_process');
const [page, packFile, OUT = 'build/lessons-sim'] = process.argv.slice(2);
const html = fs.readFileSync(page, 'utf8');
const js = html.slice(html.indexOf('<script>') + 8, html.lastIndexOf('</script>'));
let fails = 0;
const check = (c, what) => { if (!c) { console.log('FAIL ' + what); fails++; } };
const sleep = ms => new Promise(r => setTimeout(r, ms));
const b64 = name => { const m = html.match(new RegExp('const ' + name + ' = Uint8Array\\.from\\(atob\\("([A-Za-z0-9+/=]+)"\\)')); return m ? Buffer.from(m[1], 'base64') : null; };
const PACK = b64('TP_PACK'), HEAD = b64('TP_HEAD');
const same = (a, b) => a && b && a.length === b.length && Buffer.compare(Buffer.from(a), Buffer.from(b)) === 0;

function frame(cmd, raw) {
  const o = [0xF0, 0x7D, 0x53, 0x33, cmd]; let acc = 0, bits = 0;
  for (const b of raw) { acc |= b << bits; bits += 8; while (bits >= 7) { o.push(acc & 127); acc >>= 7; bits -= 7; } }
  if (bits) o.push(acc & 127); o.push(0xF7); return o;
}
function unpack(d) { const o = []; let acc = 0, bits = 0; for (const b of d) { acc |= (b & 127) << bits; bits += 7; if (bits >= 8) { o.push(acc & 255); acc >>= 8; bits -= 8; } } return o; }

/* the page, with a small DOM (what it touches) and the MIDI ports given: { name: device } */
function open(ports, win) {
  const els = {}, logs = [];
  function el(id) {
    const e = { id, style: {}, disabled: false, checked: false, hidden: false, className: '', dataset: {}, attrs: {}, kids: [], _text: '',
      classList: { set: new Set(), add(c) { this.set.add(c); }, remove(c) { this.set.delete(c); }, contains(c) { return this.set.has(c); },
        toggle(c, on) { if (on === undefined ? !this.set.has(c) : on) this.set.add(c); else this.set.delete(c); } },
      setAttribute(k, v) { this.attrs[k] = v; }, getAttribute(k) { return this.attrs[k]; },
      appendChild(d) { this.kids.push(d); if (id === 'log') logs.push(d.textContent); return d; },
      scrollIntoView() {}, remove() {}, click() {},
      set innerHTML(v) { this._text = String(v); this.kids = []; }, get innerHTML() { return this._text; },
      set textContent(v) { this._text = String(v); }, get textContent() { return this._text; },
      get firstChild() { return null; }, get lastChild() { return null; } };
    return e;
  }
  const document = { documentElement: {}, hidden: false, getElementById: id => els[id] || (els[id] = el(id)),
    createElement: () => el(''), createTextNode: t => ({ textContent: t }), body: { appendChild() {} },
    addEventListener() {}, querySelectorAll: () => [], querySelector: () => null };
  const outputs = new Map(), inputs = new Map();
  for (const [name, dev] of Object.entries(ports)) {
    const inp = { name, state: 'connected', onmidimessage: null };
    outputs.set(name, { name, state: 'connected', send: d => setImmediate(() => dev.rx(Array.from(d))) }); inputs.set(name, inp);
    dev.toPage = d => setImmediate(() => { if (inp.onmidimessage) inp.onmidimessage({ data: Uint8Array.from(d) }); });
  }
  const navigator = { language: 'en', requestMIDIAccess: async () => ({ outputs, inputs }) };
  const atob = s => Buffer.from(s, 'base64').toString('binary');
  const api = new Function('document', 'navigator', 'atob', 'window', js + '\nreturn { TP, send: tpSend };')(document, navigator, atob, win || {});
  const shown = id => !!els[id] && !els[id].classList.contains('hide');
  async function send() { els.tpSend.onclick(); await sleep(20); while (api.TP.busy) await sleep(20); }
  return { els, logs, api, shown, send, el: id => document.getElementById(id) };
}

/* a fake FM-1 running Teachie Beachie: its four sample slots (how many samples each holds), its course */
function fm1(o = {}) {
  const dev = { slots: (o.slots || [0, 0, 0, 0]).slice(), written: {}, home: 0, lessons: o.lessons || 0, refused: 0, msgs: 0, cur: null };
  const say = (cmd, raw) => dev.toPage(frame(cmd, raw));
  dev.rx = d => {
    if (d[0] !== 0xF0 || d[1] !== 0x7D || d[2] !== 0x53 || d[3] !== 0x33) return;
    const cmd = d[4], r = unpack(d.slice(5, d.length - 1));
    if (o.silent) return;
    if (cmd === 21) {                                       /* ED_GUIDE */
      if (o.sloop) { say(0, [1]); return; }                 /* (SLOOP 3 itself: a command it does not know) */
      if (r[0] === 6) say(21, [6, 0, 0, 0, dev.lessons]); else { if (r[0] === 5) dev.home++; say(21, [r[0], 0, 0, 2, 0]); }
      return;
    }
    if (cmd !== 13) { say(0, [1]); return; }                /* ED_SAMPLE */
    const sub = r[0], k = r[1];
    if (sub === 0) {
      const a = [0, 4, 80];
      for (let i = 0; i < 4; i++) { const nm = dev.slots[i] ? 'KIT ' + (i + 1) : ''; a.push(dev.slots[i], ...Array.from({ length: 8 }, (_, j) => nm.charCodeAt(j) || 0), dev.slots[i] ? 60 : 0, 0); }
      say(13, a); return;
    }
    dev.msgs++;
    if (o.busy && dev.msgs % o.busy === 0 && !dev.again) { dev.again = true; dev.refused++; say(0, [2]); return; }   /* busy: asked again */
    dev.again = false;
    if (sub === 1) { dev.cur = { k, data: [] }; dev.slots[k] = 0; if (dev.written[k]) { delete dev.written[k]; dev.lessons = 0; } say(13, [1, k]); return; }
    if (!dev.cur || dev.cur.k !== k) { say(0, [1]); return; }
    if (sub === 2) {
      const off = (r[2] | r[3] << 8 | r[4] << 16 | r[5] << 24) >>> 0;
      if (off !== dev.cur.data.length || r.length < 6) { say(0, [1]); return; }
      dev.cur.data.push(...r.slice(6)); const g = dev.cur.data.length;
      say(13, [2, k, g & 255, g >> 8 & 255, g >> 16 & 255, g >>> 24]); return;
    }
    if (sub === 3) {
      const h = r.slice(2, 482), data = Buffer.from(dev.cur.data);
      const len = (h[16] | h[17] << 8 | h[18] << 16 | h[19] << 24) >>> 0, crc = (h[20] | h[21] << 8 | h[22] << 16 | h[23] << 24) >>> 0;
      if (r.length < 482 || String.fromCharCode(...h.slice(0, 4)) !== 'TPCO' || len !== data.length || crc !== zlib.crc32(data)) { dev.cur = null; say(0, [4]); return; }
      dev.written[k] = { head: Buffer.from(h), data }; dev.cur = null;
      if (!o.older && data.slice(0, 4).toString() === 'TPC3') dev.lessons = data[6];   /* (an older Teachie Beachie does not read this pack) */
      say(13, [3, k, 0]); return;
    }
    say(0, [1]);
  };
  return dev;
}
const said = (p, re) => p.logs.some(l => re.test(l));

(async () => {
  check(PACK && HEAD && HEAD.length === 480, 'the page carries the lessons and their header');
  if (!PACK || !HEAD) { console.log('1 FAILED'); process.exit(1); }
  const N = PACK[6];
  check(PACK.slice(0, 4).toString() === 'TPC3' && N > 0, 'the pack inside is a course (TPC3, ' + N + ' lessons)');
  check(HEAD.slice(0, 4).toString() === 'TPCO' && HEAD.readUInt32LE(16) === PACK.length && HEAD.readUInt32LE(20) === zlib.crc32(PACK), 'its header: the length and the CRC');
  if (packFile) check(same(PACK, fs.readFileSync(packFile)), 'the pack inside is ' + packFile);
  check(new RegExp('const TP_N = ' + N + ';').test(html), 'the page knows how many lessons (' + N + ')');

  /* 1. a FM-1 with empty slots: the last one, the pack whole, the start's card before and after */
  let d = fm1(), p = open({ 'SLOOP 3': d });
  p.el('p6').classList.add('tp-pending');                   /* (as the page has it: nothing says "ready" yet) */
  await p.send();
  check(!p.el('p6').classList.contains('tp-pending') && p.el('st6').className === 'done', 'empty slots: the last step says ready, and is marked done, only now');
  check(p.shown('tpDone') && !p.shown('tpErr'), 'empty slots: done');
  check(d.written[3] && same(d.written[3].data, PACK) && same(d.written[3].head, HEAD), 'empty slots: the pack whole in the last slot');
  check(Object.keys(d.written).length === 1, 'empty slots: one slot written');
  check(d.home === 2, 'empty slots: the start\'s card before and after (' + d.home + ')');
  check(/30 lessons/.test(p.els.tpMeta.textContent.replace(String(N), '30')) && /USR4/.test(p.els.tpMeta.textContent), 'empty slots: what the page says (' + p.els.tpMeta.textContent + ')');
  check(p.els.tpFill.style.width === '100%', 'empty slots: the bar full');
  check(!p.els.tpSend.disabled, 'empty slots: the button back');

  /* 2. samples in USR3 and USR4, a FM-1 busy now and then: the pack in USR2 */
  d = fm1({ slots: [0, 0, 3, 1], busy: 7 }); p = open({ 'SLOOP 3': d });
  await p.send();
  check(p.shown('tpDone') && d.written[1] && same(d.written[1].data, PACK) && Object.keys(d.written).length === 1, 'samples in the last slots: the pack in USR2');
  check(d.refused > 5 && d.slots[2] === 3 && d.slots[3] === 1, 'a busy FM-1 is asked again (' + d.refused + ' times), the samples stay');

  /* 3. all four slots hold samples: nothing written until one is chosen, and agreed to */
  let asked = '', yes = false;
  d = fm1({ slots: [2, 1, 3, 1] }); p = open({ 'SLOOP 3': d }, { confirm: q => { asked = q; return yes; } });
  await p.send();
  check(p.shown('tpPick') && !p.shown('tpDone') && Object.keys(d.written).length === 0 && p.els.tpSlots.kids.length === 4, 'a full FM-1: the four slots offered, nothing written');
  p.els.tpSlots.kids[1].onclick(); await sleep(50);
  check(/USR2/.test(asked) && Object.keys(d.written).length === 0 && d.slots[1] === 1, 'a full FM-1: it asks first, and "no" writes nothing');
  yes = true; p.els.tpSlots.kids[1].onclick(); await sleep(20); while (p.api.TP.busy) await sleep(20);
  check(p.shown('tpDone') && !p.shown('tpPick') && d.written[1] && same(d.written[1].data, PACK) && d.slots[0] === 2 && d.slots[2] === 3 && d.slots[3] === 1, 'a full FM-1: the chosen slot only');

  /* 4. what can be wrong */
  p = open({}); p.api.TP.patience = 300; await p.send();
  check(p.shown('tpErr') && said(p, /No FM-1 running Teachie Beachie/), 'no FM-1: said');
  p = open({ 'FM-1 Update': fm1({ silent: true }) }); p.api.TP.patience = 300; await p.send();
  check(p.shown('tpErr') && said(p, /still in MIDI MODE/), 'MIDI MODE: said');
  d = fm1({ sloop: true }); p = open({ 'SLOOP 3': d }); p.api.TP.patience = 300; await p.send();
  check(p.shown('tpErr') && said(p, /SLOOP 3 without Teachie Beachie/) && Object.keys(d.written).length === 0, 'SLOOP 3 without Teachie Beachie: said, nothing written');
  d = fm1({ older: true }); p = open({ 'SLOOP 3': d }); await p.send();
  check(p.shown('tpErr') && !p.shown('tpDone') && said(p, /does not find them/), 'an older Teachie Beachie: said');
  d = fm1({ silent: true }); p = open({ 'SLOOP 3': d, 'Felucca': fm1() }); p.api.TP.patience = 300; await p.send();
  check(p.shown('tpDone'), 'two FM-1s, one silent: the one that answers');

  /* 5. the lessons alone: the last step opened from the first */
  p = open({ 'SLOOP 3': fm1() });
  p.els.tpOnly.onclick({ preventDefault() {} });
  check(p.shown('p6') && !p.shown('p1') && p.els.p6.classList.contains('tp-only'), 'the lessons alone: the last step, without the install\'s words');
  p.els.n1.onclick();
  check(p.shown('p2') && !p.els.p6.classList.contains('tp-only'), 'the lessons alone: left again');

  /* 6. the real firmware, on the PC twin */
  let twin = 'no build/twin: skipped';
  if (fs.existsSync('build/twin')) {
    fs.mkdirSync(OUT, { recursive: true }); try { fs.unlinkSync(path.join(OUT, 'flash.bin')); } catch (e) {}
    const tw = spawn('./build/twin', ['serve', OUT, '-', '1'], { env: { ...process.env, ASAN_OPTIONS: 'detect_leaks=0:handle_segv=0:allow_user_segv_handler=1' } });
    let buf = '', sx = null, ready; const readyP = new Promise(r => ready = r), dev = { toPage() {}, tap: null };
    tw.stdout.on('data', c => { buf += c; let i; while ((i = buf.indexOf('\n')) >= 0) { const l = buf.slice(0, i).trim(); buf = buf.slice(i + 1);
      if (l === 'READY') ready();
      else if (l.startsWith('M ')) for (const b of l.slice(2).split(' ').map(h => parseInt(h, 16))) { if (b === 0xF0) sx = []; if (sx) { sx.push(b); if (b === 0xF7) { const m = sx; sx = null; if (dev.tap) dev.tap(m); dev.toPage(m); } } } } });
    tw.stderr.on('data', c => process.stderr.write(c));
    const gone = new Promise(r => tw.on('exit', r));
    dev.rx = m => tw.stdin.write('M ' + m.map(x => x.toString(16)).join(' ') + '\n');
    await Promise.race([readyP, sleep(60000)]);
    p = open({ 'SLOOP 3 MIDI': dev });
    const t0 = Date.now(); await p.send(); const took = ((Date.now() - t0) / 1000).toFixed(1);
    check(p.shown('tpDone') && !p.shown('tpErr'), 'the twin: done (' + p.logs.slice(-2).join(' | ') + ')');
    check(/USR4/.test(p.els.tpMeta.textContent), 'the twin: the last slot (' + p.els.tpMeta.textContent + ')');
    const count = await new Promise(res => { const t = setTimeout(() => res(-1), 2000); dev.tap = m => { if (m[4] === 21) { const r = unpack(m.slice(5, m.length - 1)); if (r[0] === 6) { clearTimeout(t); res(r[4]); } } }; dev.rx(frame(21, [6])); });
    check(count === N, 'the twin: the FM-1 finds ' + N + ' lessons (' + count + ')');
    /* a second time, as newer lessons would go: the same slot again */
    await p.send();
    check(p.shown('tpDone') && /USR4/.test(p.els.tpMeta.textContent), 'the twin: sent again, the same slot');
    tw.stdin.write('Q\n'); await Promise.race([gone, sleep(8000)]); try { tw.kill(); } catch (e) {}
    const fl = fs.existsSync(path.join(OUT, 'flash.bin')) ? fs.readFileSync(path.join(OUT, 'flash.bin')) : null, AT = 0xE7000;   /* (USR4: SLOT_AT[3], fw/main.c) */
    check(fl && same(fl.subarray(AT, AT + 480), HEAD) && same(fl.subarray(AT + 512, AT + 512 + PACK.length), PACK), 'the twin: its flash holds the header and the pack, byte for byte');
    check(fl && [0xA0000, 0xB4000, 0xC8000].every(a => fl.subarray(a, a + 4).toString() !== 'TPCO'), 'the twin: the other slots untouched');
    twin = 'the twin took them in ' + took + ' s';
  }
  console.log(fails ? fails + ' FAILED' : 'lessons_sim: OK (' + N + ' lessons, ' + PACK.length + ' bytes: the last empty slot, a busy FM-1, a full one, what can be wrong, the lessons alone; ' + twin + ')');
  process.exit(fails ? 1 : 0);
})();
