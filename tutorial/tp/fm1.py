# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Peachie: the connector. Talks to an FM-1 running this firmware in the words of SLOOP 3's editor
(app/edproto.h), plus the guide (ED_GUIDE, fw/main.c): cards, lights, the keys it takes, every press read back.

Two links, the same messages on both:
  Twin    the simulated FM-1 (build/twin serve), for tests: it also has a learner's hands and can save the screen
  Midi    a real FM-1 over USB (needs the Python packages mido and python-rtmidi; not tried on hardware yet)
"""
import queue, subprocess, threading, time, zlib

ED_READ, ED_WBEGIN, ED_WDATA, ED_WEND, ED_NOTES, ED_LIVE, ED_NAMES, ED_GUIDE = 5, 6, 7, 8, 12, 15, 20, 21
K_KIT = 9                                   # a factory sound's kind: a drum kit

class Refused(RuntimeError):
    def __init__(self, cmd, reason): super().__init__("the FM-1 refused command %d (reason %d)" % (cmd, reason)); self.reason = reason
BTN = dict(OCT_DOWN=0, OCT_UP=1, FX=2, SEL=3, ENV=4, LFO=5, EDIT=6, GLO=7, HOME=8, SAVE=9, ARP=10, SEQ=11, PLAY=12, REC=13)
KNOB = dict(SELECT=0, ALGORITHM=1, KNOB1=2, KNOB2=3, KNOB3=4, KNOB4=5, PRESETS=6)
BLINK = 5                                   # a light's "how": 1..4 a level, 5 the guide's double blink
EVENT = {1: "key", 2: "key_up", 3: "button", 4: "button_up", 5: "knob_right", 6: "knob_left"}

def light_of_key(k): return 14 + k          # a light's number: buttons 0..13, keys 14..40 (key 0 = F3 .. 26 = G5)
def key_of_note(midi): return midi - 53

def frame(cmd, raw):
    out, acc, bits = [0xF0, 0x7D, 0x53, 0x33, cmd & 0x7F], 0, 0
    for b in raw:
        acc |= b << bits; bits += 8
        while bits >= 7: out.append(acc & 0x7F); acc >>= 7; bits -= 7
    if bits: out.append(acc & 0x7F)
    return bytes(out + [0xF7])

def parse(sx):
    if len(sx) < 6 or sx[0] != 0xF0 or tuple(sx[1:4]) != (0x7D, 0x53, 0x33) or sx[-1] != 0xF7: return None
    out, acc, bits = bytearray(), 0, 0
    for b in sx[5:-1]:
        acc |= b << bits; bits += 7
        if bits >= 8: out.append(acc & 0xFF); acc >>= 8; bits -= 8
    return sx[4] & 0x7F, bytes(out)

class Twin:
    """The simulated FM-1. It runs in real time, as the FM-1 does."""
    def __init__(self, out="build/tw/lesson", binary="build/twin"):
        import os; os.makedirs(out, exist_ok=True)
        self.out = out
        self.p = subprocess.Popen([binary, "serve", out, "-", "1"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
        self.frames, self.lines, self.sx = queue.Queue(), queue.Queue(), bytearray()
        threading.Thread(target=self._read, daemon=True).start()
        self._line("READY", 60)
    def _read(self):
        for line in self.p.stdout:
            line = line.strip()
            if line.startswith("M "):
                for b in bytes(int(x, 16) for x in line[2:].split()):
                    if b == 0xF0: self.sx = bytearray()
                    self.sx.append(b)
                    if b == 0xF7: self.frames.put(bytes(self.sx)); self.sx = bytearray()
            else: self.lines.put(line)
    def _line(self, prefix, timeout=5):
        end = time.time() + timeout
        while time.time() < end:
            try: l = self.lines.get(timeout=0.1)
            except queue.Empty: continue
            if l.startswith(prefix): return l
        raise TimeoutError("the twin did not say " + prefix)
    def _w(self, s): self.p.stdin.write(s + "\n"); self.p.stdin.flush()
    def send(self, sx): self._w("M " + " ".join("%02x" % b for b in sx))
    def recv(self, timeout):
        try: return self.frames.get(timeout=timeout)
        except queue.Empty: return None
    # a learner's hands, and eyes
    def key(self, k, down): self._w("K %d %d" % (k, int(down)))
    def button(self, b, down): self._w("B %d %d" % (b, int(down)))
    def knob(self, e, clicks): self._w("E %d %d" % (e, clicks))
    def lights(self): self._w("L"); return [int(x) for x in self._line("L ").split()[1:]]
    def shot(self, name): self._w("S " + name); self._line("SHOT"); return "%s/%s.ppm" % (self.out, name)
    def wav(self, name=None):
        """keep the sound as a file from here (name), or stop (no name)"""
        self._w("W " + name if name else "W"); self._line("WAV"); return "%s/%s.wav" % (self.out, name) if name else None
    def close(self):
        try: self._w("Q"); self.p.wait(timeout=5)
        except Exception: self.p.kill()

class Midi:
    """A real FM-1 over USB MIDI. Written to the same shape as Twin; not tried on hardware yet."""
    def __init__(self, names=("SLOOP 3", "Felucca")):       # (a Mac that knew SLOOP 2.8 may still call it Felucca)
        import mido
        ok = lambda n: any(x in n for x in names) and "Update" not in n
        i = [n for n in mido.get_input_names() if ok(n)]; o = [n for n in mido.get_output_names() if ok(n)]
        if not i or not o: raise RuntimeError("no FM-1 found (a MIDI port named like %s)" % " or ".join(names))
        self.mido, self.i, self.o = mido, mido.open_input(i[0]), mido.open_output(o[0])
    def send(self, sx): self.o.send(self.mido.Message("sysex", data=list(sx[1:-1])))
    def recv(self, timeout):
        end = time.time() + timeout
        while time.time() < end:
            m = self.i.poll()
            if m is None: time.sleep(0.001); continue
            if m.type == "sysex": return bytes([0xF0] + list(m.data) + [0xF7])
        return None
    def shot(self, name): return None
    def close(self): self.i.close(); self.o.close()

class FM1:
    def __init__(self, link): self.link = link; self.page = self.track = 0; self._names = None; self.lost = 0; self.pending = []   # presses and turns not looked at yet
    def ask(self, cmd, raw=b"", timeout=1.5, tries=1):
        """tries: for a request that is safe to send twice (an answer can be lost on the way), how often to ask"""
        for n in range(tries - 1):
            try: return self.ask(cmd, raw, timeout)
            except TimeoutError: self.lost += 1
        self.link.send(frame(cmd, raw))
        end = time.time() + timeout
        while time.time() < end:
            sx = self.link.recv(end - time.time())
            r = parse(sx) if sx else None
            if r and r[0] == cmd: return r[1]
            if r and r[0] == 0: raise Refused(cmd, r[1][0] if r[1] else -1)
        raise TimeoutError("no answer to command %d" % cmd)
    # the guide
    def guide(self, op, payload=b""):
        r = self.ask(ED_GUIDE, bytes([op]) + bytes(payload), 0.4, 4)
        self.page, self.track = r[1], r[2]
        self.pending += [(EVENT.get(r[5 + 2 * i], "?"), r[6 + 2 * i]) for i in range(r[4])]   # (every answer brings them)
    def off(self): return self.guide(0)
    def card(self, words, left="", right="", big=False, strip=0):
        """strip: 0 a whole card, 1 a band at the top, 2 at the bottom (the page stays live around it)"""
        t = lambda s: s.upper().encode("ascii", "replace") + b"\0"
        return self.guide(1, bytes([1 | (2 if big else 0) | strip << 2]) + t(words) + t(left) + t(right))
    def no_card(self): return self.guide(1, bytes([0, 0, 0, 0]))
    def lights(self, how=None, only=False):
        """how: {light number: 1..4 or BLINK}; nothing: the guide's lights off"""
        return self.guide(2, bytes([int(only)]) + bytes(x for kv in (how or {}).items() for x in kv))
    def take(self, keys=(), buttons=(), knobs=()):
        km = sum(1 << k for k in keys); bm = sum(1 << b for b in buttons); em = sum(1 << e for e in knobs)
        return self.guide(3, km.to_bytes(4, "little") + bm.to_bytes(2, "little") + bytes([em]))
    def events(self):
        self.guide(4); ev, self.pending = self.pending, []
        return ev
    # SLOOP 3's own LIVE page: a button, a key, PLAY
    def live(self, op=0, a=0, b=0, c=0): return self.ask(ED_LIVE, bytes([op, a, b, c]), 0.4, 4)
    def all_keys_up(self):
        for k in range(27): self.live(2, k, 0)
    def playing(self): return bool(self.live()[1] & 1)
    def play_key(self, k, down): self.live(2, k, int(down))
    def press_button(self, b, down): self.live(1, b, int(down))
    def patient(self, cmd, raw=b"", wait=20.0):
        """ask, and ask again while the FM-1 says it is busy (a save under way)"""
        end = time.time() + wait
        while True:
            try: return self.ask(cmd, raw)
            except Refused as e:
                if e.reason != 2 or time.time() > end: raise
                time.sleep(0.03)
    # SLOOP 3's sounds, tempo, mix and patterns
    def names(self, kind=0):
        out, first = [], 0
        while True:
            r = self.ask(ED_NAMES, bytes([kind, first & 255, first >> 8])); total, cnt = r[3] | r[4] << 8, r[5]
            out += [(r[8 + 15 * i: 21 + 15 * i].split(b"\0")[0].decode(), r[6 + 15 * i]) for i in range(cnt)]
            first += cnt
            if first >= total or not cnt: return out            # (name, kind) of each
    def sound(self, track, name, kit=False):
        if self._names is None: self._names = self.names(0)
        k = next(i for i, (n, kind) in enumerate(self._names) if n == name and (kind == K_KIT) == kit)
        self.live(10, track, k & 255, k >> 8)
    def tempo(self, bpm): b = int(round(bpm * 10)); self.live(13, b & 255, b >> 8, 255)
    def level(self, track, v): self.live(4, track, 0, v)
    def select(self, track): self.live(9, track)
    def write_pattern(self, track, steps, notes, pat=255):
        """the pattern rewritten: notes are (step, length in steps, note, velocity); a drum's note is its pad, 0..15"""
        raw = [(int(round(s * 96)), max(12, int(round(l * 96))), n, v) for s, l, n, v in notes]; first = 0
        while True:
            c = raw[first:first + 60]
            body = bytes([track, pat]) + first.to_bytes(2, "little") + steps.to_bytes(2, "little") + bytes([len(c)])
            r = self.patient(ED_NOTES, body + b"".join(a.to_bytes(2, "little") + b.to_bytes(2, "little") + bytes([n, v]) for a, b, n, v in c))
            first += len(c)
            if first >= len(raw): return r[2] | r[3] << 8       # the notes it holds now
    # the whole project, out and back in
    def read_project(self):
        data, total = b"", None
        while total is None or len(data) < total:
            r = self.patient(ED_READ, bytes([2, 0]) + len(data).to_bytes(4, "little") + (384).to_bytes(2, "little"))
            total = int.from_bytes(r[6:10], "little")
            if len(r) <= 14 and len(data) < total: raise RuntimeError("the project stopped coming at %d of %d bytes" % (len(data), total))
            data += r[14:]
        return data
    def write_project(self, data):
        """the project saved on the FM-1 replaced by this one, and opened"""
        self.patient(ED_WBEGIN, bytes([0, 0]) + len(data).to_bytes(4, "little")); off = 0
        while off < len(data): c = data[off:off + 240]; self.patient(ED_WDATA, off.to_bytes(4, "little") + c); off += len(c)
        self.patient(ED_WEND, (zlib.crc32(data) & 0xFFFFFFFF).to_bytes(4, "little"))
