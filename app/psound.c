/* SPDX-License-Identifier: GPL-3.0-only
 * Each pattern's own sound: see psound.h. */
#include "psound.h"

static uint8_t q8(float x) { x = x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; return (uint8_t)(x * 255.0f + 0.5f); }
void ps_take(psounds *ps, int t, int pat, const gb_patch *live, const float *own, int preset)
{
    if (t < 0 || t >= SEQ_NTRK || pat < 0 || pat >= SEQ_NPAT) return;
    psound *s = &ps->s[t][pat];
    s->engine = live->engine; s->poly = live->poly; s->unison = live->unison; s->protect = live->protect;
    s->preset = (int16_t)(preset < -32768 ? -1 : preset > 32767 ? -1 : preset);
    float g = live->gain * 1000.0f + 0.5f; s->gain = (uint16_t)(g < 0.0f ? 0.0f : g > 65535.0f ? 65535.0f : g);
    for (int i = 0; i < GB_NPARAM; i++) s->p[i] = q8(own[i]);
    if (t == PS_DRUM) {
        pdrum *d = &ps->d[pat];
        for (int i = 0; i < 16; i++) { d->dtune[i] = live->dtune[i]; d->ddec[i] = live->ddec[i]; }
        d->dmute = live->dmute;
    }
}
void ps_give(const psounds *ps, int t, int pat, gb_patch *live, float *own, int *preset)
{
    if (t < 0 || t >= SEQ_NTRK || pat < 0 || pat >= SEQ_NPAT) return;
    const psound *s = &ps->s[t][pat];
    if ((live->engine == ENG_DRUM) != (s->engine == ENG_DRUM)) return;   /* (the drum track stays drums) */
    live->engine = s->engine; live->poly = s->poly ? s->poly : 1; live->unison = s->unison ? s->unison : 1;
    live->protect = s->protect; live->gain = s->gain ? (float)s->gain * 0.001f : 1.0f;
    for (int i = 0; i < GB_NPARAM; i++) live->p[i] = own[i] = (float)s->p[i] * (1.0f / 255.0f);
    *preset = s->preset;
    if (t == PS_DRUM) {
        const pdrum *d = &ps->d[pat];
        for (int i = 0; i < 16; i++) { live->dtune[i] = d->dtune[i]; live->ddec[i] = d->ddec[i]; }
        live->dmute = d->dmute;
    }
}
void ps_fill(psounds *ps, int t, int cur, const gb_patch *live, const float *own, int preset)
{
    for (int i = 0; i < SEQ_NPAT; i++) ps_take(ps, t, i, live, own, preset);
    ps->cur[t] = (uint8_t)cur;
}
int ps_same(const psounds *ps, int t, int a, int b)
{
    const psound *x = &ps->s[t][a], *y = &ps->s[t][b];          /* (field by field: no padding compared) */
    if (x->engine != y->engine || x->poly != y->poly || x->unison != y->unison || x->protect != y->protect ||
        x->preset != y->preset || x->gain != y->gain) return 0;
    for (int i = 0; i < GB_NPARAM; i++) if (x->p[i] != y->p[i]) return 0;
    if (t == PS_DRUM) {
        const pdrum *p = &ps->d[a], *q = &ps->d[b];
        for (int i = 0; i < 16; i++) if (p->dtune[i] != q->dtune[i] || p->ddec[i] != q->ddec[i]) return 0;
        if (p->dmute != q->dmute) return 0;
    }
    return 1;
}
void ps_copy(psounds *ps, int t, int from, int to)
{
    if (from == to) return;
    ps->s[t][to] = ps->s[t][from];
    if (t == PS_DRUM) ps->d[to] = ps->d[from];
}
int ps_write(const psounds *ps, int t, int pat, uint8_t *o)
{
    const psound *s = &ps->s[t][pat];
    int n = 0;
    o[n++] = s->engine; o[n++] = s->poly; o[n++] = s->unison; o[n++] = s->protect;
    o[n++] = (uint8_t)((uint16_t)s->preset & 0xFF); o[n++] = (uint8_t)((uint16_t)s->preset >> 8);
    o[n++] = (uint8_t)(s->gain & 0xFF); o[n++] = (uint8_t)(s->gain >> 8);
    for (int i = 0; i < GB_NPARAM; i++) o[n++] = s->p[i];
    if (t == PS_DRUM) {
        const pdrum *d = &ps->d[pat];
        for (int i = 0; i < 16; i++) { o[n++] = (uint8_t)d->dtune[i]; o[n++] = (uint8_t)d->ddec[i]; }
        o[n++] = (uint8_t)(d->dmute & 0xFF); o[n++] = (uint8_t)(d->dmute >> 8);
    }
    return n;
}
int ps_read(psounds *ps, int t, int pat, const uint8_t *in, uint32_t len, int old)
{
    uint32_t need = (old ? PS_BYTES12 : PS_BYTES) + (t == PS_DRUM ? PS_DRUM_BYTES : 0);
    if (len < need) return 0;
    psound *s = &ps->s[t][pat];
    int n = 0;
    s->engine = in[n] < (uint8_t)gb_nengines ? in[n] : 0; n++;
    s->poly = in[n++]; s->unison = in[n++]; s->protect = in[n++];
    if (old) { s->preset = (int8_t)in[n++]; if (s->preset > 127 || s->preset < -129) s->preset = -1; }
    else { s->preset = (int16_t)(in[n] | in[n + 1] << 8); n += 2; }
    s->gain = (uint16_t)(in[n] | in[n + 1] << 8); n += 2;
    if (!s->unison || s->unison > 4) s->unison = 1;
    for (int i = 0; i < GB_NPARAM; i++) s->p[i] = in[n++];
    if (t == PS_DRUM) {
        pdrum *d = &ps->d[pat];
        for (int i = 0; i < 16; i++) {
            int a = (int8_t)in[n++], b = (int8_t)in[n++];
            d->dtune[i] = (int8_t)(a < -8 ? -8 : a > 7 ? 7 : a); d->ddec[i] = gb_dpack(GB_DDEC(b), gb_dlev((int8_t)b));
        }
        d->dmute = (uint16_t)(in[n] | in[n + 1] << 8); n += 2;
    }
    return n;
}
