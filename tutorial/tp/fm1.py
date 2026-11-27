# SPDX-License-Identifier: GPL-3.0-only
"""Teachie Beachie: the connector. Talks to an FM-1 running this firmware in the words of SLOOP 3's editor
(app/edproto.h), plus the guide (ED_GUIDE, fw/main.c): cards, lights, the keys it takes, every press read back.

Two links, the same messages on both:
  Twin    the simulated FM-1 (build/twin serve), for tests: it also has a learner's hands and can save the screen
  Midi    a real FM-1 over USB (needs the Python packages mido and python-rtmidi, in tutorial/.venv)
"""
import queue, subprocess, threading, time, zlib

ED_SOUND, ED_READ, ED_WBEGIN, ED_WDATA, ED_WEND, ED_PATTERN, ED_NOTES, ED_SAMPLE, ED_LIVE, ED_NOTES2, ED_SONG, ED_NAMES, ED_GUIDE = 2, 5, 6, 7, 8, 10, 12, 13, 15, 16, 18, 20, 21
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
    def __init__(self, out="build/tw/lesson", binary="build/twin", flash=None):
        """flash: a file of the whole flash to start from (a twin that ended leaves one, flash.bin, in its folder)"""
        import os; os.makedirs(out, exist_ok=True); flash = flash or os.environ.get("TP_FLASH", "-")
        self.out = out
        self.p = subprocess.Popen([binary, "serve", out, flash, "1"], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)
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
        try: return self.frames.get(timeout=timeout) if timeout > 0 else self.frames.get_nowait()
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
    """A real FM-1 over USB MIDI. The same shape as Twin."""
    def __init__(self, names=("SLOOP 3", "Felucca")):       # (a Mac that knew SLOOP 2.8 may still call it Felucca)
        import mido
        ok = lambda n: any(x in n for x in names) and "Update" not in n
        i = [n for n in mido.get_input_names() if ok(n)]; o = [n for n in mido.get_output_names() if ok(n)]
        if not i or not o: raise RuntimeError("no FM-1 found (a MIDI port named like %s)" % " or ".join(names))
        self.mido, self.i, self.o = mido, mido.open_input(i[0]), mido.open_output(o[0])
    def send(self, sx): self.o.send(self.mido.Message("sysex", data=list(sx[1:-1])))
    def recv(self, timeout):
        end = time.time() + timeout
        while True:
            m = self.i.poll()
            if m is not None and m.type == "sysex": return bytes([0xF0] + list(m.data) + [0xF7])
            if m is None:
                if time.time() >= end: return None
                time.sleep(0.001)
    def shot(self, name): return None
    def close(self): self.i.close(); self.o.close()

class FM1:
    def __init__(self, link): self.link = link; self.page = self.track = 0; self._names = None; self.lost = 0; self.state = 0; self.picked = 0; self.step = None; self.pending = []   # presses and turns not looked at yet
    def _absorb(self, r):
        """a guide's answer taken in: the page, the track, the state, and the presses and turns it brings (each answer
        brings them once, so every one that arrives is read, even a late one)"""
        self.page, self.track, self.state = r[1], r[2], r[3]   # state: bit 0 a card, 1 the start's card, 2 START pressed
        # (SLOOP 3.4.2's Teachie Beachie) bits 4..7, what a key writes to: the drum picked, the one boxes write (0 KICK .. 15);
        # on NOTES (page 23) of a track that is not the drums (3), the step shown, from 0 (bit 3: it is past the 16th)
        if r[1] == 23 and r[2] != 3: self.step = None if r[3] & 8 else r[3] >> 4
        else: self.picked = r[3] >> 4; self.step = None
        self.pending += [(EVENT.get(r[5 + 2 * i], "?"), r[6 + 2 * i]) for i in range(r[4])]
    def _drain(self):
        """answers that came too late for their question (the FM-1 was busy, saving for instance): out of the way"""
        while True:
            sx = self.link.recv(0)
            if not sx: return
            r = parse(sx)
            if r and r[0] == ED_GUIDE and len(r[1]) >= 5 and r[1][0] != 6: self._absorb(r[1])
    def ask(self, cmd, raw=b"", timeout=1.5, tries=1):
        """tries: for a request that is safe to send twice (an answer can be late or lost), how often to ask"""
        for n in range(tries):
            self._drain(); self.link.send(frame(cmd, raw))
            end = time.time() + timeout
            while time.time() < end:
                sx = self.link.recv(end - time.time())
                r = parse(sx) if sx else None
                if r and r[0] == ED_GUIDE and len(r[1]) >= 5 and r[1][0] != 6: self._absorb(r[1])
                if r and r[0] == cmd: return r[1]
                if r and r[0] == 0: raise Refused(cmd, r[1][0] if r[1] else -1)
            if n + 1 < tries: self.lost += 1
        raise TimeoutError("no answer to command %d" % cmd)
    # the guide
    def guide(self, op, payload=b""): self.ask(ED_GUIDE, bytes([op]) + bytes(payload), 0.4, 4)
    def off(self): return self.guide(0)
    def card(self, words, left="", right="", big=False, strip=0):
        """strip: 0 a whole card, 1 a band at the top, 2 at the bottom, 3 a tag in the top left corner (the page stays live around it)"""
        t = lambda s: s.encode("ascii", "replace") + b"\0"
        if "@" in words and not self.has_lessons():                     # (no lessons on this FM-1: SLOOP's own letters, which have no pictures)
            import re; words = re.sub(r"\[@(\d*)\]", lambda m: "[BLACK %s]" % m.group(1) if m.group(1) else "[BLACK]", words)
        return self.guide(1, bytes([1 | (2 if big else 0) | strip << 2]) + t(words) + t(left) + t(right))
    def has_lessons(self):
        """the lessons are on this FM-1 (so its cards are drawn in their font, pictures and all); asked once"""
        if getattr(self, "_has", None) is None: self._has = self.course()[3] > 0
        return self._has
    def no_card(self): return self.guide(1, bytes([0, 0, 0, 0]))
    def knob_fill(self, knob, fill, from_right=False):
        """a knob's framed name on the card: how much of its frame is filled, 0..255 (how far the turn a step asks for is
        done), from the left, or from the right for a turn to the left. A new card starts every frame empty"""
        return self.guide(8, bytes([knob, max(0, min(255, int(fill))), 1 if from_right else 0]))
    def keep(self, tracks):
        """(SLOOP 3.4) these tracks' patterns and sounds, as they play now, made their saved copy, so a change of pattern
        does not put an older one back (ED_GUIDE 7; a firmware before it answers and does nothing)"""
        return self.guide(7, bytes([sum(1 << t for t in tracks) & 0x1F]))
    def lights(self, how=None, only=False):
        """how: {light number: 1..4 or BLINK}; nothing: the guide's lights off. only: SLOOP's own lights dark too, but the
        fourteen buttons keep a faint glow (level 1: as bright as HOME > LIGHTS says), so their names can still be read"""
        how = dict(how or {})
        if only: how = {**{b: 1 for b in range(14)}, **how}
        return self.guide(2, bytes([int(only)]) + bytes(x for kv in how.items() for x in kv))
    def take(self, keys=(), buttons=(), knobs=()):
        km = sum(1 << k for k in keys); bm = sum(1 << b for b in buttons); em = sum(1 << e for e in knobs)
        return self.guide(3, km.to_bytes(4, "little") + bm.to_bytes(2, "little") + bytes([em]))
    def home(self): return self.guide(5)                  # the start's card: START and JUST PLAY
    def start_asked(self): self.guide(4); return bool(self.state & 4)
    def events(self):
        self.guide(4); ev, self.pending = self.pending, []
        return ev
    # SLOOP 3's own LIVE page: a button, a key, PLAY
    def live(self, op=0, a=0, b=0, c=0): return self.ask(ED_LIVE, bytes([op, a, b, c]), 0.4, 4)
    def status(self):
        """what the FM-1 is doing now, from LIVE's answer: the transport, the tempo, the message on its screen, each track"""
        r = self.live(); o = r[1:]; f = o[0]
        st = dict(playing=bool(f & 1), recording=bool(f & 2), armed=bool(f & 4), song=bool(f & 8), song_rec=bool(f & 16), fill=bool(f & 32),
                  countin=bool(f & 64), tempo=(o[1] | o[2] << 8) / 10.0, bar=o[3] | o[4] << 8, beat=o[5], track=o[6], punch=None if o[13] == 255 else o[13],
                  dust=o[14], duck=o[15], mfilt=o[16] - 100, swing=o[17], msg=bytes(o[21:45]).split(b"\0")[0].decode("ascii", "replace") if o[20] else "", tracks=[])
        for t in range(5):
            q = o[45 + 22 * t: 67 + 22 * t]
            st["tracks"].append(dict(cur=q[0], next=q[1], mute=bool(q[2]), solo=bool(q[3]), level=round(q[4] * 100 / 127), pan=round(q[5] * 100 / 127) - 50,
                                     chorus=round(q[6] * 100 / 127), delay=round(q[7] * 100 / 127), reverb=round(q[8] * 100 / 127), filter=round((q[9] - 64) * 100 / 63),
                                     engine=q[10], preset=q[11] | q[12] << 8, used=q[14] | q[15] << 8, steps=q[17], div=q[18], gate=q[19], swing=q[20], quant=q[21]))
        return st
    def pattern(self, track, pat=255):
        """a pattern's notes: (its length in steps, [(step, length in steps, note or pad, velocity, condition, ratchet)])"""
        out, first = [], 0
        while True:
            r = self.ask(ED_PATTERN, bytes([track, pat, first & 255, first >> 8, 1]), 0.5, 4); steps, total, cnt = r[2] | r[3] << 8, r[6] | r[7] << 8, r[10]
            for i in range(cnt):
                o = r[11 + 8 * i: 19 + 8 * i]; out.append(((o[0] | o[1] << 8) / 96.0, (o[2] | o[3] << 8) / 96.0, o[4], o[5], o[6], o[7]))
            first += cnt
            if first >= total or not cnt: return steps, out
    def sound_values(self, track):
        """a track's sound: (its name, its 48 values as the screen counts them)"""
        r = self.ask(ED_SOUND, bytes([track]), 0.5, 4)
        return r[4:20].split(b"\0")[0].decode("ascii", "replace"), [int.from_bytes(r[20 + 2 * k: 22 + 2 * k], "little", signed=True) for k in range(48)]
    def song_lines(self): return len(self.song())
    def tap(self, b, hold=0.08):
        """a button tapped from here, as the editor's LIVE page does (the guide does not see these)"""
        self.press_button(b, True); time.sleep(hold); self.press_button(b, False); time.sleep(0.06)
    def turn(self, e, clicks): self.ask(ED_LIVE, bytes([3, e, clicks & 255, 0]), 2.5)       # (asked once: twice would turn it twice)
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
        """the pattern rewritten: notes are (step, length in steps, note, velocity) and, if wanted, a condition (1: only in a
        fill); a drum's note is its pad, 0..15"""
        raw = [(int(round(n[0] * 96)), max(12, int(round(n[1] * 96))), n[2], n[3], n[4] if len(n) > 4 else 0) for n in notes]; first = 0
        wide = any(n[4] for n in raw); per = 50 if wide else 60
        while True:
            c = raw[first:first + per]
            body = bytes([track, pat]) + first.to_bytes(2, "little") + steps.to_bytes(2, "little") + bytes([len(c)])
            r = self.patient(ED_NOTES2 if wide else ED_NOTES, body + b"".join(a.to_bytes(2, "little") + b.to_bytes(2, "little") + bytes([n, v] + ([cd, 1] if wide else [])) for a, b, n, v, cd in c))
            first += len(c)
            if first >= len(raw): return r[2] | r[3] << 8       # the notes it holds now
    def write_song(self, lines):
        """the song written: lines are (pattern, bars), one after the other, every track on that pattern"""
        body, bar = b"", 0
        for pat, bars in lines: body += bar.to_bytes(2, "little") + bytes([0] + [pat] * 5); bar += bars
        self.ask(ED_SONG, bytes([0, len(lines)]) + bar.to_bytes(2, "little") + body, 0.8, 2)
    def song(self):
        """the song's lines: [(the bar it starts on, each track's pattern, its length in bars)]"""
        r = self.ask(ED_SONG, bytes([0]), 0.5, 3); total = r[2] | r[3] << 8
        at = [r[7 + 8 * i] | r[8 + 8 * i] << 8 for i in range(r[6])]
        return [(at[i], list(r[10 + 8 * i: 15 + 8 * i]), (at[i + 1] if i + 1 < len(at) else total) - at[i]) for i in range(len(at))]
    # the course on the FM-1 itself (fw/course.inc): its lessons kept in a user sample slot
    def slots(self):
        """what each user sample slot holds: [(how many samples, its name, its size in KB)]"""
        r = self.ask(ED_SAMPLE, bytes([0]), 1.0, 3)
        return [(r[3 + 11 * i], r[4 + 11 * i: 12 + 11 * i].split(b"\0")[0].decode("ascii", "replace"), r[12 + 11 * i] | r[13 + 11 * i] << 8) for i in range(r[1])]
    def write_slot(self, k, head, data):
        """a user sample slot written: its 480 bytes of header, and the data behind it (the FM-1 checks its length and CRC)"""
        def job(raw, wait=30.0):
            end = time.time() + wait
            while True:
                try: return self.ask(ED_SAMPLE, raw, 8.0)
                except Refused as e:
                    if e.reason != 2 or time.time() > end: raise
                    time.sleep(0.05)
        self._has = None                                                # (the lessons may come or go with this slot)
        job(bytes([1, k])); off = 0
        while off < len(data): c = data[off:off + 400]; job(bytes([2, k]) + off.to_bytes(4, "little") + c); off += len(c)
        return job(bytes([3, k]) + head)
    def course(self):
        """the FM-1's own course: (its phase, the lesson's number, the step from 0, how many lessons it holds)"""
        r = self.ask(ED_GUIDE, bytes([6]), 0.4, 4); return r[1], r[2], r[3], r[4]
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
