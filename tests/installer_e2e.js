/* SPDX-License-Identifier: GPL-3.0-only
 * SLOOP 3's installer page in Chromium, as a user goes through it, against a fake m-bootloader in MIDI MODE (Web MIDI
 * replaced in the page): the beta's agreement, m-bootloader there, Connect while the FM-1 is not plugged yet (it is
 * found by itself when it appears), the backup (1 MB read, a file downloaded), the install (the package read twice),
 * the end. In English (Teachie Beachie's page has no French; SLOOP's ran it in French too). Pictures of each step:
 * OUT/installer-*.png.
 *   node tests/installer_e2e.js PAGE.html OUT */
const { chromium } = require('playwright');
const fs = require('fs'), path = require('path');
const [page, OUT] = process.argv.slice(2);
fs.mkdirSync(OUT, { recursive: true });
let fails = 0;
const check = (c, what) => { if (!c) { console.log('FAIL ' + what); fails++; } };

/* the fake FM-1: m-bootloader's MIDI MODE ("FM-1 Update"), inside the page */
const FAKE = () => {
  const KEY = 0x980F, outputs = new Map(), inputs = new Map();
  const access = { outputs, inputs, onstatechange: null };
  function frame(cmd, body) {
    const raw = [0, 0x59, cmd, body.length & 255, body.length >> 8 & 255, body.length >> 16, ...body];
    let s = 0; for (const b of body) s += b; raw.push(~s & 255);
    const o = [0xF0]; let acc = 0, bits = 0;
    for (const b of raw) { acc |= b << bits; bits += 8; while (bits >= 7) { o.push(acc & 127); acc >>= 7; bits -= 7; } }
    if (bits) o.push(acc & 127); o.push(0xF7); return o;
  }
  function unpack(d) { const o = []; let acc = 0, bits = 0; for (const b of d) { acc |= (b & 127) << bits; bits += 7; if (bits >= 8) { o.push(acc & 255); acc >>= 8; bits -= 8; } } return o; }
  const inp = { name: 'FM-1 Update', state: 'connected', onmidimessage: null };
  const toPage = d => setTimeout(() => inp.onmidimessage && inp.onmidimessage({ data: Uint8Array.from(d) }), 0);
  let pending = null; window.__fm1 = { reads: 0, wrote: false, slow: 0 };
  const read = (addr, len) => new Promise(res => { pending = res; toPage(frame(0x30, [0, addr & 255, addr >> 8 & 255, addr >> 16 & 255, addr >>> 24, len & 255, len >> 8 & 255, len >> 16 & 255])); });
  const wait = ms => new Promise(r => setTimeout(r, ms));
  async function install() {
    const data = m => Uint8Array.from(m.slice(14, m.length - 1));
    function enc(b, o, n, k) { k &= 0xFFFF; for (let i = 0; i < n; i++) { b[o + i] ^= k & 255; k = k & 0x8000 ? ((k << 1) ^ 0x1021) & 0xFFFF : (k << 1) & 0xFFFF; } }
    const rd32 = (a, i) => (a[i] | a[i + 1] << 8 | a[i + 2] << 16 | a[i + 3] << 24) >>> 0;
    const h = data(await read(0, 64)); enc(h, 0, 64, 0xFFFF); const n = h[8] | h[9] << 8;
    const e = data(await read(0x40, 80 * n)).slice(0, 80); enc(e, 0, 80, 0xFFFF);
    const at = rd32(e, 8), size = rd32(e, 12);
    for (let pass = 0; pass < 2; pass++) for (let o = 0; o < size; o += 512) { await read(at + o, 512); window.__fm1.reads++; if (window.__fm1.slow) await wait(window.__fm1.slow); }
    window.__fm1.wrote = true;
    await read(0xF0000000, 8);
  }
  const out = { name: 'FM-1 Update', state: 'connected', send: d => setTimeout(() => {
    if (d.length === 6 && d[4] === 0x7F) { install(); return; }
    const m = unpack(d.slice(1, d.length - 1));
    if (m[2] === 0x12) {
      const body = new Array(48).fill(0);
      'MBOOT'.split('').forEach((c, i) => body[i] = c.charCodeAt(0));
      body[5] = 1; body[6] = 1; body[7] = 1; body[8] = KEY & 255; body[9] = KEY >> 8; body[12] = 0x10;
      'v1.2'.split('').forEach((c, i) => body[28 + i] = c.charCodeAt(0));
      toPage(frame(0x12, body));
    } else if (m[2] === 0x23) {               /* a backup read: 512 bytes from the address asked */
      const a = m.slice(7, 11), body = [0, ...a, 0, 2, 0, ...new Array(512).fill(0).map((_, i) => (i * 7) & 255)];
      toPage(frame(0x23, body));
    } else if (m[2] === 0x30 && pending) { const r = pending; pending = null; r(m); }
  }, 0) };
  window.__plug = () => { outputs.set('fm1', out); inputs.set('fm1', inp); if (access.onstatechange) access.onstatechange({ port: inp }); };
  navigator.requestMIDIAccess = async () => access;
};

(async () => {
  const b = await chromium.launch();
  for (const lang of ['en']) {
    const ctx = await b.newContext({ viewport: { width: 1440, height: 900 }, locale: lang === 'fr' ? 'fr-FR' : 'en-US', acceptDownloads: true });
    const pg = await ctx.newPage();
    const errs = []; pg.on('pageerror', e => errs.push(e.message));
    await pg.addInitScript(FAKE);
    await pg.goto('file://' + path.resolve(page));
    const shot = async (n) => pg.screenshot({ path: path.join(OUT, `installer-${lang}-${n}.png`) });
    check(await pg.evaluate(() => document.documentElement.lang) === lang, lang + ': the language from the browser');
    await shot('0-top');
    // 1: the agreement
    check(await pg.isDisabled('#n1'), lang + ': Continue waits for the agreement');
    await pg.click('label.check:has(#ok1)'); await pg.click('#n1');
    // 2: m-bootloader
    await pg.click('#mbNo'); check(await pg.isVisible('#mbGuide'), lang + ': the m-bootloader guide opens');
    await pg.locator('#p2').scrollIntoViewIfNeeded(); await shot('2-bootloader');
    await pg.click('#mbDone');
    // 3: connect, the FM-1 plugged a moment later
    check(await pg.isVisible('#p3'), lang + ': step 3 shown');
    await pg.click('#con'); await pg.waitForTimeout(200);
    check(await pg.isVisible('#waiting'), lang + ': waiting for the FM-1');
    await shot('3-waiting');
    await pg.evaluate(() => window.__plug());
    await pg.waitForSelector('#device:not(.hide)', { timeout: 5000 });
    check((await pg.textContent('#devMeta')).includes('980F'), lang + ': the chip key shown');
    await shot('3-connected');
    await pg.click('#n3');
    // 4: the backup
    check(await pg.isDisabled('#n4'), lang + ': Continue waits for a backup');
    const [dl] = await Promise.all([pg.waitForEvent('download', { timeout: 60000 }), pg.click('#bak')]);
    check(/^fm1-backup-980F-\d{4}-\d\d-\d\d\.bin$/.test(dl.suggestedFilename()), lang + ': the backup file name ' + dl.suggestedFilename());
    const bp = await dl.path(); check(fs.statSync(bp).size === 0x100000, lang + ': the backup is 1 MB');
    await pg.waitForSelector('#bdone:not(.hide)'); await shot('4-backup');
    await pg.click('#n4');
    // 5: install
    check(!(await pg.isDisabled('#go')), lang + ': Install enabled');
    await pg.evaluate(() => { window.__fm1.slow = 2; });
    await pg.click('#go');
    await pg.waitForFunction(() => window.__fm1.reads > 1000, null, { timeout: 60000 });
    await shot('5-installing');
    check(await pg.isDisabled('[data-go="3"]'), lang + ': the steps locked while installing');
    await pg.evaluate(() => { window.__fm1.slow = 0; });
    await pg.waitForSelector('#p6:not(.hide)', { timeout: 120000 });
    check(await pg.evaluate(() => window.__fm1.wrote), lang + ': the package read twice');
    await shot('6-done');
    // the page at phone width
    await pg.setViewportSize({ width: 390, height: 844 }); await pg.evaluate(() => scrollTo(0, 0)); await shot('7-phone');
    check(errs.length === 0, lang + ': no page error ' + errs.join(' | '));
    await ctx.close();
  }
  await b.close();
  console.log(fails ? fails + ' FAILED' : 'installer_e2e: OK (en: agreement, m-bootloader, connect when plugged, backup, install, done)');
  process.exit(fails ? 1 : 0);
})();
