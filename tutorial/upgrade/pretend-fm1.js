/* SPDX-License-Identifier: GPL-3.0-only
 * Teachie Beachie: a pretend FM-1 for trying the installer page's lessons step in a real browser, after the fm1() of
 * tests/lessons_sim.js. Nothing loads it: it is put into the open page by hand (or by an assistant's browser tool).
 *   1. copy it beside the built page (build/fw/), and serve that folder;
 *   2. in the page: load it (a script element whose src is pretend-fm1.js), then TP.patience = 1200 so a FM-1 that
 *      is not there is given up on quickly, and window.confirm = () => true if nobody is there to answer;
 *   3. __fm1({ slots: [3, 0, 0, 0] }) before the first click on "Send the lessons" (the page asks for MIDI once;
 *      for another pretend FM-1, load the page again). Other kinds: { sloop: true } SLOOP 3 alone, { older: true }
 *      an older Teachie Beachie, { none: true } nothing plugged in, { menu: true } the menu with MIDI MODE;
 *   4. __tp() says what the step shows and what the pretend FM-1 got: __dev.written, by slot. */
(() => {
  const frame = (cmd, raw) => { const o = [0xF0, 0x7D, 0x53, 0x33, cmd]; let acc = 0, bits = 0; for (const b of raw) { acc |= b << bits; bits += 8; while (bits >= 7) { o.push(acc & 127); acc >>= 7; bits -= 7; } } if (bits) o.push(acc & 127); o.push(0xF7); return o; };
  const unpack = d => { const o = []; let acc = 0, bits = 0; for (const b of d) { acc |= (b & 127) << bits; bits += 7; if (bits >= 8) { o.push(acc & 255); acc >>= 8; bits -= 8; } } return o; };
  const T = new Uint32Array(256); for (let i = 0; i < 256; i++) { let c = i; for (let j = 0; j < 8; j++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1; T[i] = c >>> 0; }
  const crc32 = d => { let c = 0xFFFFFFFF; for (const b of d) c = T[(c ^ b) & 255] ^ (c >>> 8); return (c ^ 0xFFFFFFFF) >>> 0; };
  window.__fm1 = (o = {}) => {
    const dev = { slots: (o.slots || [0, 0, 0, 0]).slice(), written: {}, home: 0, lessons: o.lessons || 0, cur: null, sent: 0 };
    const name = o.name || (o.menu ? 'FM-1 Updater' : 'FM-1 SLOOP 3');
    const inp = { name, state: 'connected', onmidimessage: null };
    const say = (cmd, raw) => setTimeout(() => { if (inp.onmidimessage) inp.onmidimessage({ data: Uint8Array.from(frame(cmd, raw)) }); }, 0);
    const rx = d => {
      if (d[0] !== 0xF0 || d[1] !== 0x7D || d[2] !== 0x53 || d[3] !== 0x33) return;
      const cmd = d[4], r = unpack(d.slice(5, d.length - 1));
      if (cmd === 21) { if (o.sloop) { say(0, [1]); return; } if (r[0] === 6) say(21, [6, 0, 0, 0, dev.lessons]); else { if (r[0] === 5) dev.home++; say(21, [r[0], 0, 0, 2, 0]); } return; }
      if (cmd !== 13) { say(0, [1]); return; }
      const sub = r[0], k = r[1];
      if (sub === 0) { const a = [0, 4, 80]; for (let i = 0; i < 4; i++) { const nm = dev.slots[i] ? 'KIT ' + (i + 1) : ''; a.push(dev.slots[i], ...Array.from({ length: 8 }, (_, j) => nm.charCodeAt(j) || 0), dev.slots[i] ? 60 : 0, 0); } say(13, a); return; }
      if (sub === 1) { dev.cur = { k, data: [] }; dev.slots[k] = 0; if (dev.written[k]) { delete dev.written[k]; dev.lessons = 0; } say(13, [1, k]); return; }
      if (!dev.cur || dev.cur.k !== k) { say(0, [1]); return; }
      if (sub === 2) { const off = (r[2] | r[3] << 8 | r[4] << 16 | r[5] << 24) >>> 0; if (off !== dev.cur.data.length || r.length < 6) { say(0, [1]); return; } dev.cur.data.push(...r.slice(6)); const g = dev.cur.data.length; say(13, [2, k, g & 255, g >> 8 & 255, g >> 16 & 255, g >>> 24]); return; }
      if (sub === 3) { const h = r.slice(2, 482), data = dev.cur.data; const len = (h[16] | h[17] << 8 | h[18] << 16 | h[19] << 24) >>> 0, crc = (h[20] | h[21] << 8 | h[22] << 16 | h[23] << 24) >>> 0;
        if (r.length < 482 || String.fromCharCode(...h.slice(0, 4)) !== 'TPCO' || len !== data.length || crc !== crc32(data)) { dev.cur = null; say(0, [4]); return; }
        dev.written[k] = { mark: String.fromCharCode(...data.slice(0, 4)), bytes: data.length, crc: crc.toString(16) }; dev.cur = null;
        if (!o.older && String.fromCharCode(...data.slice(0, 4)) === 'TPC4') dev.lessons = data[6];
        say(13, [3, k, 0]); return; }
      say(0, [1]);
    };
    const out = { name, state: 'connected', send: d => { dev.sent++; if (!o.menu) setTimeout(() => rx(Array.from(d)), 0); } };
    const outputs = new Map(o.none ? [] : [[name, out]]), inputs = new Map(o.none ? [] : [[name, inp]]);
    navigator.requestMIDIAccess = async () => ({ outputs, inputs, sysexEnabled: true, onstatechange: null, addEventListener() {}, removeEventListener() {} });
    window.__dev = dev; return 'pretend FM-1: ' + (o.none ? 'none plugged in' : name);
  };
  /* what the lessons step shows now */
  window.__tp = () => { const vis = id => { const e = document.getElementById(id); return !!e && !e.classList.contains('hide') && e.getBoundingClientRect().height > 0; };
    const p6 = document.getElementById('p6');
    return { head: (p6.querySelector('h2') || {}).innerText, done: vis('tpDone') ? document.getElementById('tpMeta').textContent : null,
      err: vis('tpErr') ? document.getElementById('tpErr').innerText : null, pick: vis('tpPick') ? document.getElementById('tpPick').innerText : null,
      written: window.__dev && __dev.written, slots: window.__dev && __dev.slots, lessons: window.__dev && __dev.lessons }; };
})();
