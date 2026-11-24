/* SPDX-License-Identifier: GPL-3.0-only
 * The FM-1's USB port (USB0, full speed) as a class-compliant USB-MIDI device, polled. Written for this project
 * from facts: the registers and the order of their writes as m-bootloader's SPEC and its MIT-licensed USB-MIDI
 * device record them (the controller is a MUSB whose own registers are reached one at a time through USB_CON1,
 * with a pause before each access: accesses a few cycles apart make it babble on the bus, and the host turns the
 * port off), the descriptors from the USB-MIDI class specification 1.0 (its appendix B), the standard requests
 * from the USB 2.0 specification (chapter 9).
 * With it, a USB audio input (the USB audio class 1.0, class compliant, as SLOOP 2.8's): the FM-1's output as 16-bit
 * stereo at 44.1 or 48 kHz (the host picks) on an isochronous, asynchronous IN endpoint (endpoint 4, its own address
 * and count registers, isochronous by TXCSR2 bit 6), so a computer records the FM-1 over the cable. The audio is
 * ours (usb_audio_put, a block at a time) through a ring the port drains: a packet each millisecond, 44 or 45
 * frames (the 44.1 kHz pattern) or 48, one more or one fewer while the ring's low point leaves its band (our clock
 * and the host's differ a little). The port is served from the 2 kHz tick (irq.c): usb_tick. */
#include "usb.h"
#include "fm1.h"
#include "board.h"
#include "irq.h"

#define U_CON0        REG32(0x11800u)        /* bit 2: the SIE clocked; bit 13 a frame start pending, bit 12 acks it */
#define U_CON1        REG32(0x11804u)        /* an SIE access: register << 8 | value, bit 14 read, bit 15 done */
#define U_EP_CNT(n)   REG32(0x11808u + 4u * (n))
#define U_EP0_ADR     REG32(0x11818u)
#define U_EP1_TADR    REG32(0x1181Cu)
#define U_EP_RADR(n)  REG32(0x11820u + 8u * ((n) - 1u))
#define U_EP4_CNT     REG32(0x11834u)
#define U_EP4_TADR    REG32(0x11838u)
#define U_EP4_RADR    REG32(0x1183Cu)
#define U_IO_CON0     REG32(0x51000u)        /* the pins: D+ pull-up (bits 4-7: 0x40), the PHY */
#define U_CLK_CON1    REG32(0x10010u)        /* bits 0-1: 0 = the USB clock from PLL 48 MHz */
/* the SIE's registers */
enum { S_FADDR = 0, S_POWER = 1, S_INT_TX1 = 2, S_INT_RX1 = 4, S_INT_USB = 6, S_INT_TX1E = 7, S_INT_TX2E = 8,
       S_INT_RX1E = 9, S_INT_RX2E = 10, S_INT_USBE = 11, S_INDEX = 14, S_TX_MAX = 16, S_CSR0 = 17, S_TX_CSR1 = 17,
       S_TX_CSR2 = 18, S_RX_MAX = 19, S_RX_CSR1 = 20, S_RX_CSR2 = 21, S_RX_CNT1 = 22, S_RX_CNT2 = 23, S_COUNT0 = 22 };
/* CSR0 (endpoint 0) */
#define C0_RXRDY   0x01u
#define C0_TXRDY   0x02u
#define C0_STALLED 0x04u
#define C0_END     0x08u
#define C0_SETUPEND 0x10u
#define C0_STALL   0x20u
#define C0_SERVED  0x40u
#define C0_SERVED_SETUPEND 0x80u
#define PKT 64u
#define WAIT 20000u

#define UA_MAXF 49u                                              /* frames in the largest packet (48 kHz + 1) */
#ifdef USB_UPDATER
/* SLOOP 3's update loader (upd/upd.c): the MIDI port only, its own name, its own device version (a host keeps what it
 * read for a vendor, product and version: the firmware's descriptors are not taken for these) */
static const uint8_t DEV[18] = {
    18, 1, 0x10, 0x01, 0, 0, 0, PKT, 0x09, 0x12, 0x01, 0x00,
    0x90, 0x03, 0, 1, 0, 1 };                                    /* device 3.90; name: string 1 */
#define CONF_LEN 101
static const uint8_t CONF[CONF_LEN] = {
    9, 2, CONF_LEN, 0, 2, 1, 0, 0x80, 50,                        /* 2 interfaces, bus powered, 100 mA */
    9, 4, 0, 0, 0, 1, 1, 0, 0,                                   /* audio control */
    9, 0x24, 1, 0x00, 0x01, 9, 0, 1, 1,                          /*   its header: the MIDI stream (1) */
    9, 4, 1, 0, 2, 1, 3, 0, 0,                                   /* MIDI streaming, 2 endpoints */
    7, 0x24, 1, 0x00, 0x01, 65, 0,
    6, 0x24, 2, 1, 1, 0,
    6, 0x24, 2, 2, 2, 0,
    9, 0x24, 3, 1, 3, 1, 2, 1, 0,
    9, 0x24, 3, 2, 4, 1, 1, 1, 0,
    9, 5, 0x01, 2, PKT, 0, 0, 0, 0,
    5, 0x25, 1, 1, 1,
    9, 5, 0x81, 2, PKT, 0, 0, 0, 0,
    5, 0x25, 1, 1, 3 };
_Static_assert(sizeof CONF == CONF_LEN, "the configuration's length");
static const uint8_t LANG[4] = { 4, 3, 0x09, 0x04 };
static const char NAME[] = "SLOOP 3 Update";
#else
static const uint8_t DEV[18] = {
    18, 1, 0x10, 0x01, 0, 0, 0, PKT, 0x09, 0x12, 0x01, 0x00,     /* USB 1.1; vendor 0x1209 (pid.codes), product 0x0001 */
    0x10, 0x03, 0, 1, 0, 1 };                                    /* device 3.10 (with the audio input: a host that
                                                                  * kept 3.00's descriptors asks again); name: string 1 */
#define CONF_LEN 178
static const uint8_t CONF[CONF_LEN] = {
    9, 2, CONF_LEN, 0, 3, 1, 0, 0x80, 50,                        /* 3 interfaces, bus powered, 100 mA */
    9, 4, 0, 0, 0, 1, 1, 0, 0,                                   /* audio control */
    10, 0x24, 1, 0x00, 0x01, 31, 0, 2, 2, 1,                     /*   its header: the audio stream (2) first, then the
                                                                  *   MIDI one (1): a host that drops MIDI from the
                                                                  *   list still finds the audio */
    12, 0x24, 2, 1, 0x03, 0x06, 0, 2, 0x03, 0x00, 0, 0,          /*   input terminal 1: a line, 2 channels (L R) */
    9, 0x24, 3, 2, 0x01, 0x01, 0, 1, 0,                          /*   output terminal 2: the USB stream, from 1 */
    9, 4, 1, 0, 2, 1, 3, 0, 0,                                   /* MIDI streaming, 2 endpoints */
    7, 0x24, 1, 0x00, 0x01, 65, 0,                               /*   its header */
    6, 0x24, 2, 1, 1, 0,                                         /*   IN jack 1, embedded (from the host) */
    6, 0x24, 2, 2, 2, 0,                                         /*   IN jack 2, external */
    9, 0x24, 3, 1, 3, 1, 2, 1, 0,                                /*   OUT jack 3, embedded (to the host), from 2 */
    9, 0x24, 3, 2, 4, 1, 1, 1, 0,                                /*   OUT jack 4, external, from 1 */
    9, 5, 0x01, 2, PKT, 0, 0, 0, 0,                              /* endpoint 1 OUT, bulk */
    5, 0x25, 1, 1, 1,                                            /*   to jack 1 */
    9, 5, 0x81, 2, PKT, 0, 0, 0, 0,                              /* endpoint 1 IN, bulk */
    5, 0x25, 1, 1, 3,                                            /*   from jack 3 */
    9, 4, 2, 0, 0, 1, 2, 0, 0,                                   /* audio streaming, alternate 0: nothing */
    9, 4, 2, 1, 1, 1, 2, 0, 0,                                   /*   alternate 1: the stream */
    7, 0x24, 1, 2, 1, 0x01, 0x00,                                /*   from terminal 2, a frame's delay, PCM */
    14, 0x24, 2, 1, 2, 2, 16, 2, 0x44, 0xAC, 0x00, 0x80, 0xBB, 0x00,   /* type I: 2 channels, 16 bits, 44100 or 48000 */
    9, 5, 0x84, 0x05, UA_MAXF * 4u, 0, 1, 0, 0,                  /*   endpoint 4 IN, isochronous, asynchronous, 1 ms */
    7, 0x25, 1, 0x01, 0, 0, 0 };                                 /*   its rate can be set */
_Static_assert(sizeof CONF == CONF_LEN, "the configuration's length");
static const uint8_t LANG[4] = { 4, 3, 0x09, 0x04 };
static const char NAME[] = "FM-1 SLOOP 3";
#endif

/* the controller's DMA writes a little past a full packet when it receives: room to spare */
static uint8_t ep0[PKT + 4] __attribute__((aligned(4)));
static uint8_t ep1_in[PKT] __attribute__((aligned(4)));
static uint8_t ep1_out[PKT + 4] __attribute__((aligned(4)));
static uint8_t name_desc[2 + 2 * (sizeof NAME - 1)];
static const uint8_t *e0_p; static uint32_t e0_left;
static uint8_t addr, conf, open_, addr_pending, e0_sending, e0_zlp, rx_pend, ticking;
/* ---- the audio input: a ring of 16-bit frames (L | R << 16) we fill a block at a time, drained a packet a
 * millisecond */
#define UA_N 512u
#define UA_PRIME 100u                                /* silence ahead of the first frame */
#define UA_LO 56u                                    /* the ring's low point (just before we write a half): its band */
#define UA_HI 150u
static uint32_t ua_ring[UA_N];
static volatile uint32_t ua_w, ua_r;
static uint8_t ep4_in[UA_MAXF * 4u] __attribute__((aligned(4)));
static struct {
    volatile uint8_t alt, r48, feed, go;             /* streaming; 48 kHz; the host reads; the ring primed */
    uint8_t e0_rx, queued, wait, force;              /* SET_CUR's data to come; a packet out; ms it waits; USB RATE */
    uint32_t acc, minf, cnt, last;
    float ph; int32_t pl, pr;                        /* 48 kHz: the resampler's place and last frame */
} ua;
/* the packets each way: rings of 4-byte packets */
#define RXN 64u
#define TXN 64u
static uint8_t rxq[RXN][4] __attribute__((section(".lowbss"))), txq[TXN][4] __attribute__((section(".lowbss")));   /* (written before read) */
static uint32_t rx_w, rx_r, tx_w, tx_r, lost;

static void pause_(void) { for (int i = 0; i < 40; i++) __asm__ volatile("nop" ::: "memory"); }
static void sie_wr(uint32_t r, uint32_t v)
{
    if (!(U_CON0 & 4u)) return;
    pause_();
    U_CON1 = (r << 8 & 0xFFFF00u) | (v & 0xFFu);
    CSYNC();
    for (uint32_t n = WAIT; n && !(U_CON1 & 0x8000u); n--) { }
}
static uint32_t sie_rd(uint32_t r)
{
    if (!(U_CON0 & 4u)) return 0;
    pause_();
    U_CON1 = (r << 8 & 0xFFBF00u) | 0x4000u;
    CSYNC();
    for (uint32_t n = WAIT; n; n--) if (U_CON1 & 0x8000u) return U_CON1 & 0xFFu;
    return 0;
}

/* ---------------------------------------------------------------- endpoint 0: the standard requests */
static void e0_chunk(void)
{
    uint32_t n = e0_left > PKT ? PKT : e0_left;
    for (uint32_t i = 0; i < n; i++) ep0[i] = e0_p[i];
    CSYNC();
    U_EP0_ADR = (uint32_t)(uintptr_t)ep0;
    U_EP_CNT(0) = n;
    e0_p += n; e0_left -= n;
    int last = !e0_left && !(n == PKT && e0_zlp);                /* a full last packet: an empty one after it */
    if (!e0_left && n == PKT) e0_zlp = 0;
    sie_wr(S_INDEX, 0);
    sie_wr(S_CSR0, last ? (C0_TXRDY | C0_END) : C0_TXRDY);
    e0_sending = (uint8_t)!last;
}
static void e0_send(const uint8_t *d, uint32_t n, uint32_t asked)
{
    if (n > asked) n = asked;
    e0_p = d; e0_left = n;
    e0_zlp = (uint8_t)(n < asked && !(n % PKT));
    sie_wr(S_INDEX, 0);
    sie_wr(S_CSR0, C0_SERVED);
    e0_chunk();
}
static void ep1_setup(void)
{
    U_EP1_TADR = (uint32_t)(uintptr_t)ep1_in;
    U_EP_RADR(1) = (uint32_t)(uintptr_t)ep1_out;
    sie_wr(S_INDEX, 1);
    sie_wr(S_TX_MAX, 0xFF); sie_wr(S_TX_CSR1, 0x48); sie_wr(S_TX_CSR2, 0);    /* data toggle cleared, emptied */
    sie_wr(S_RX_MAX, 0xFF); sie_wr(S_RX_CSR1, 0x90); sie_wr(S_RX_CSR2, 0);
    sie_wr(S_INT_RX1E, 0x02);
    U_CON0 &= ~(1u << 20);
    rx_pend = 0;
}
static void ua_stream(uint32_t alt)                  /* SET_INTERFACE, a configuration, a bus reset */
{
    ua.go = 0; ua.feed = 0; ua.queued = 0; ua.wait = 0; ua.acc = 0; ua.cnt = 0; ua.minf = 0xFFFFFFFFu;
    ua.alt = (uint8_t)alt;
}
static void ua_rate(uint32_t hz)                     /* SET_CUR: the nearer of 44.1 and 48 (or HOME's USB RATE) */
{
    uint8_t r48 = (uint8_t)(hz >= 46050u);
    if (ua.force) r48 = (uint8_t)(ua.force == 2);
    if (r48 == ua.r48) return;
    ua.go = 0; ua.r48 = r48; ua.acc = 0;
}
static void ep4_setup(void)
{
    U_EP4_TADR = (uint32_t)(uintptr_t)ep4_in;
    sie_wr(S_INDEX, 4);
    sie_wr(S_TX_MAX, 0xFF);
    sie_wr(S_TX_CSR1, 0x48); sie_wr(S_TX_CSR2, 0);      /* the data toggle cleared, emptied */
    sie_wr(S_TX_CSR1, 0); sie_wr(S_TX_CSR2, 0x40);      /* isochronous */
    U_CON0 &= ~(1u << (19 + 4));
}
static const uint8_t *descriptor(uint32_t value, uint32_t *n)
{
    switch (value >> 8) {
    case 1: *n = sizeof DEV; return DEV;
    case 2: *n = sizeof CONF; return CONF;
    case 3:
        if ((value & 0xFFu) == 0) { *n = sizeof LANG; return LANG; }
        if ((value & 0xFFu) == 1) {
            name_desc[0] = sizeof name_desc; name_desc[1] = 3;
            for (uint32_t i = 0; i < sizeof NAME - 1; i++) { name_desc[2 + 2 * i] = (uint8_t)NAME[i]; name_desc[3 + 2 * i] = 0; }
            *n = sizeof name_desc; return name_desc;
        }
        return 0;
    default: return 0;
    }
}
static void e0_service(void)
{
    static const uint8_t zero[2] = { 0, 0 };
    if (addr_pending) { sie_wr(S_FADDR, addr); addr_pending = 0; }   /* (after the status stage) */
    sie_wr(S_INDEX, 0);
    uint32_t csr = sie_rd(S_CSR0);
    if (csr & C0_STALLED) { sie_wr(S_CSR0, 0); e0_sending = 0; return; }
    if (csr & C0_SETUPEND) { sie_wr(S_CSR0, C0_SERVED_SETUPEND); e0_sending = 0; }   /* the host gave up on it */
    if (e0_sending) { if (!(csr & C0_TXRDY)) e0_chunk(); return; }
    if (!(csr & C0_RXRDY)) return;
    __asm__ volatile("ssync" ::: "memory");
    if (ua.e0_rx) {                                                          /* SET_CUR's 3 bytes: Hz */
        ua.e0_rx = 0;
        if (sie_rd(S_COUNT0) >= 3u) ua_rate((uint32_t)ep0[0] | (uint32_t)ep0[1] << 8 | (uint32_t)ep0[2] << 16);
        sie_wr(S_INDEX, 0); sie_wr(S_CSR0, C0_SERVED | C0_END);
        return;
    }
    uint32_t type = ep0[0], req = ep0[1], value = ep0[2] | (uint32_t)ep0[3] << 8, asked = ep0[6] | (uint32_t)ep0[7] << 8;
    switch (type << 8 | req) {
    case 0x0005: addr = ep0[2] & 0x7Fu; addr_pending = 1; break;              /* SET_ADDRESS */
    case 0x8006: {                                                           /* GET_DESCRIPTOR */
        uint32_t n; const uint8_t *d = descriptor(value, &n);
        if (!d) goto stall;
        if (!asked) break;
        e0_send(d, n, asked); return;
    }
    case 0x0009:                                                             /* SET_CONFIGURATION */
        if (ep0[2] > 1) goto stall;
        conf = ep0[2];
        if (conf) { ep1_setup(); ep4_setup(); }
        ua_stream(0); ua.r48 = (uint8_t)(ua.force == 2);
        break;
    case 0x8008: e0_send(&conf, 1, asked); return;                           /* GET_CONFIGURATION */
    case 0x810A: { static uint8_t a; a = ep0[4] == 2 ? ua.alt : 0; e0_send(&a, 1, asked); return; }   /* GET_INTERFACE */
    case 0x010B: if (ep0[4] == 2) ua_stream(ep0[2] & 1u); break;               /* SET_INTERFACE: the audio's alternate */
    case 0x2201:                                                             /* SET_CUR, endpoint 4: its rate */
        if (ep0[4] != 0x84u || ep0[3] != 1u) goto stall;
        if (asked) { ua.e0_rx = 1; sie_wr(S_INDEX, 0); sie_wr(S_CSR0, C0_SERVED); return; }   /* (the 3 bytes follow) */
        break;
    case 0xA281: case 0xA282: case 0xA283: {                                 /* GET_CUR / MIN / MAX: the rate */
        static const uint8_t R44[3] = { 0x44, 0xAC, 0x00 }, R48[3] = { 0x80, 0xBB, 0x00 };
        if (ep0[4] != 0x84u || ep0[3] != 1u) goto stall;
        int r48 = req == 0x81 ? ua.r48 : req == 0x83 ? ua.force != 1 : ua.force == 2;
        e0_send(r48 ? R48 : R44, 3, asked); return;
    }
    case 0x8000: case 0x8100: case 0x8200: e0_send(zero, 2, asked); return;  /* GET_STATUS */
    case 0x0201: case 0x0203:                                                /* CLEAR / SET_FEATURE (endpoint) */
        if (req == 1 && (ep0[4] & 0x7Fu) == 1) {                             /* halt cleared: data toggle back to 0 */
            sie_wr(S_INDEX, 1);
            if (ep0[4] & 0x80u) sie_wr(S_TX_CSR1, 0x48); else sie_wr(S_RX_CSR1, 0x90);
            sie_wr(S_INDEX, 0);
        }
        break;
    case 0x0001: case 0x0003: case 0x000B: break;                            /* device features */
    default: goto stall;
    }
    sie_wr(S_INDEX, 0);
    sie_wr(S_CSR0, C0_SERVED | C0_END);
    return;
stall:
    sie_wr(S_INDEX, 0);
    sie_wr(S_CSR0, C0_SERVED | C0_STALL);
}

/* ---------------------------------------------------------------- endpoint 1: the MIDI */
static void ep1_receive(void)
{
    if (RXN - (rx_w - rx_r) < PKT / 4u) { rx_pend = 1; return; }  /* no room: the host waits (NAK), taken later */
    rx_pend = 0;
    sie_wr(S_INDEX, 1);
    uint32_t csr = sie_rd(S_RX_CSR1) | sie_rd(S_RX_CSR2) << 8;
    if (!(csr & 1u)) return;
    uint32_t n = sie_rd(S_RX_CNT1) | sie_rd(S_RX_CNT2) << 8;
    if (n > PKT) n = PKT;
    __asm__ volatile("ssync" ::: "memory");
    for (uint32_t i = 0; i + 3 < n; i += 4) {
        if (!(ep1_out[i] & 15u)) continue;                         /* code index 0: reserved, nothing */
        uint8_t *q = rxq[rx_w % RXN];
        q[0] = ep1_out[i]; q[1] = ep1_out[i + 1]; q[2] = ep1_out[i + 2]; q[3] = ep1_out[i + 3];
        rx_w++;
    }
    csr = (csr & ~0x164u) | 0x10u;                                 /* the packet taken: the endpoint free again */
    sie_wr(S_RX_CSR1, csr & 0xFFu);
    sie_wr(S_RX_CSR2, csr >> 8);
}
static void ep1_send(void)
{
    if (tx_r == tx_w) return;
    sie_wr(S_INDEX, 1);
    uint32_t csr = sie_rd(S_TX_CSR1);
    if (csr & 1u) return;                                          /* the last one not yet taken by the host */
    if (csr & 0x80u) sie_wr(S_TX_CSR1, csr & ~0x80u);
    uint32_t n = 0;
    while (tx_r != tx_w && n < PKT) { const uint8_t *q = txq[tx_r % TXN]; for (int i = 0; i < 4; i++) ep1_in[n + (uint32_t)i] = q[i]; n += 4; tx_r++; }
    CSYNC();
    U_EP1_TADR = (uint32_t)(uintptr_t)ep1_in;
    U_EP_CNT(1) = n;
    sie_wr(S_INDEX, 1);
    sie_wr(S_TX_CSR1, sie_rd(S_TX_CSR1) | 1u);
}

/* ---------------------------------------------------------------- endpoint 4: the audio input */
static uint32_t pk16(int32_t v) { v = (v + 128) >> 8; return (uint32_t)(uint16_t)(int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v); }
void usb_audio_put(const int32_t *o, uint32_t n)       /* (the render: a block of our frames, 24 bits in 32) */
{
    if (!ua.feed) { ua.go = 0; return; }
    if (!ua.go) {                                        /* a stream starts: silence ahead of us */
        irq_off();
        uint32_t r = ua_r;
        for (uint32_t i = 0; i < UA_PRIME; i++) ua_ring[(r + i) & (UA_N - 1u)] = 0;
        ua_w = r + UA_PRIME; ua.ph = 0.0f; ua.pl = ua.pr = 0; ua.go = 1;
        irq_on();
    }
    uint32_t w = ua_w;
    if (UA_N - (w - ua_r) < n + n / 8u + 2u) return;    /* (nobody reads it: dropped) */
    if (!ua.r48) for (uint32_t i = 0; i < n; i++) ua_ring[w++ & (UA_N - 1u)] = pk16(o[2 * i]) | pk16(o[2 * i + 1]) << 16;
    else {                                               /* 48 kHz: our frames read between them, 147 for 160 */
        float ph = ua.ph; int32_t pl = ua.pl, pr = ua.pr;
        for (uint32_t i = 0; i < n; i++) {
            int32_t l = o[2 * i], r = o[2 * i + 1];
            while (ph < 1.0f) {
                ua_ring[w++ & (UA_N - 1u)] = pk16(pl + (int32_t)((float)(l - pl) * ph)) | pk16(pr + (int32_t)((float)(r - pr) * ph)) << 16;
                ph += 0.91875f;
            }
            ph -= 1.0f; pl = l; pr = r;
        }
        ua.ph = ph; ua.pl = pl; ua.pr = pr;
    }
    ua_w = w;
}
void usb_audio_rate(int mode) { ua.force = (uint8_t)(mode > 2 ? 0 : mode); }
static void ep4_service(void)                          /* (the tick) a packet as soon as the last has gone */
{
    if (!conf || !ua.alt) return;
    sie_wr(S_INDEX, 4);
    uint32_t csr = sie_rd(S_TX_CSR1);
    if (csr & 1u) { if (ua.wait < 255u && ++ua.wait >= 40u) ua.feed = 0; return; }   /* (not taken for 20 ms: the host stopped) */
    if (ua.queued) ua.feed = 1;                          /* the host took the last one */
    ua.queued = 1; ua.wait = 0;
    uint32_t n;
    if (ua.r48) n = 48u; else { ua.acc += 44100u; n = ua.acc / 1000u; ua.acc %= 1000u; }
    uint32_t fill = ua.go ? ua_w - ua_r : 0u;
    if (ua.go) {                                         /* the ring's low point over 8 packets keeps its band */
        if (fill < ua.minf) ua.minf = fill;
        if (++ua.cnt >= 8u) { n += ua.minf > UA_HI ? 1u : 0u; n -= ua.minf < UA_LO && n > 1u ? 1u : 0u; ua.cnt = 0; ua.minf = 0xFFFFFFFFu; }
    }
    uint32_t *d = (uint32_t *)(void *)ep4_in, r = ua_r;
    for (uint32_t i = 0; i < n; i++) {
        if (ua.go && r != ua_w) ua.last = ua_ring[r++ & (UA_N - 1u)];
        else if (!ua.go) ua.last = 0;
        d[i] = ua.last;                                  /* (the ring empty: the last frame again) */
    }
    ua_r = r;
    CSYNC();
    U_EP4_TADR = (uint32_t)(uintptr_t)ep4_in;
    U_EP4_CNT = n * 4u;
    CSYNC();
    sie_wr(S_TX_CSR1, 0x01);                             /* ready (and an underrun's flag cleared) */
}

/* (3.3.8, as SLOOP 2.8.5) a host there: the frame number (FRAME1, FRAME2) moves only with a real host, a millisecond a
 * frame; frozen 100 ms, or a suspend (the bus idle 3 ms), the cable is out (or the host asleep); a resume or a reset:
 * back. No reset comes when the cable is pulled, so the configuration alone said "there" until the next start */
enum { S_FRAME1 = 12, S_FRAME2 = 13 };
static volatile uint8_t suspended;
static uint32_t t_frame, t_moved; static uint16_t frame_n;
static void frame_watch(uint32_t bus)
{
    uint32_t now = time_ticks(), mhz = time_mhz();
    if (bus & 0x01u) suspended = 1;                                /* suspend */
    if (bus & 0x06u) { suspended = 0; t_moved = now; }             /* resume, reset */
    if (!conf || now - t_frame < 10000u * mhz) return;             /* (every 10 ms) */
    t_frame = now;
    uint16_t f = (uint16_t)(sie_rd(S_FRAME1) | (sie_rd(S_FRAME2) & 7u) << 8);
    if (f != frame_n) { frame_n = f; t_moved = now; suspended = 0; }
    else if (now - t_moved > 100000u * mhz) suspended = 1;
}
static void usb_service(void)
{
    if (!open_) return;
    if (U_CON0 & (1u << 13)) U_CON0 |= 1u << 12;                   /* a frame's start: acknowledged (it goes on with the
                                                                    * cable out: no sign of a host, SLOOP 2.8.5 found) */
    uint32_t bus = sie_rd(S_INT_USB), tx = sie_rd(S_INT_TX1), rx = sie_rd(S_INT_RX1);
    frame_watch(bus);
    if (bus & 0x04u) {                                             /* a bus reset: back to the start */
        sie_wr(S_FADDR, 0);
        conf = 0; e0_sending = 0; addr_pending = 0; rx_pend = 0; ua.e0_rx = 0; ua_stream(0); ua.r48 = (uint8_t)(ua.force == 2);
        tx_r = tx_w;                                               /* (nothing stale for the next host) */
        U_EP0_ADR = (uint32_t)(uintptr_t)ep0;
        sie_wr(S_INT_USBE, 0x07); sie_wr(S_INT_TX1E, 0x01); sie_wr(S_INT_RX1E, 0);
    }
    if (tx & 0x01u) e0_service();
    if (conf && ((rx & 0x02u) || rx_pend)) ep1_receive();
    if (conf) ep1_send();
    ep4_service();
}
void usb_midi_poll(void) { if (!ticking) usb_service(); }   /* (once the tick serves the port: nothing here) */
void usb_tick(void) { usb_service(); }
void usb_by_tick(void) { ticking = 1; }   /* (before the tick starts: the main loop never serves the port again) */

void usb_midi_open(void)
{
    U_CON0 = 0; U_CON1 = 0;
    U_IO_CON0 = 0x0C;
    time_wait_us(25000);
    U_CLK_CON1 &= ~3u;                                             /* the USB clock: PLL 48 MHz */
    U_EP0_ADR = (uint32_t)(uintptr_t)ep0;
    for (uint32_t n = 1; n <= 3; n++) U_EP_RADR(n) = (uint32_t)(uintptr_t)ep0;
    U_EP4_RADR = (uint32_t)(uintptr_t)ep0;
    U_IO_CON0 |= (1u << 10) | (1u << 9);
    U_IO_CON0 &= ~(1u << 11);
    U_IO_CON0 |= 1u << 12;
    U_CON0 &= ~0x4035u;
    U_CON1 = 0;
    U_CON0 |= 1u;
    U_IO_CON0 &= ~0xF0u;
    U_IO_CON0 |= 0x40u;                                            /* D+ pulled up: we are on the bus */
    U_CON0 |= 0x3Cu;
    sie_wr(S_POWER, 0x60);
    sie_wr(S_INT_USBE, 0x07); sie_wr(S_INT_TX1E, 0x01); sie_wr(S_INT_TX2E, 0);
    sie_wr(S_INT_RX1E, 0); sie_wr(S_INT_RX2E, 0);
    U_CON0 &= ~(0x1Fu << 19);
    conf = 0; open_ = 1;
}
int usb_midi_ready(void) { return open_ && conf; }
int usb_host_live(void) { return open_ && conf && !suspended; }
void usb_midi_close(void) { U_IO_CON0 &= ~0xF0u; open_ = 0; conf = 0; }   /* off the bus (D+ let go) */
int usb_midi_take(uint8_t pk[4])
{
    if (rx_r == rx_w) return 0;
    const uint8_t *q = rxq[rx_r % RXN];
    pk[0] = q[0]; pk[1] = q[1]; pk[2] = q[2]; pk[3] = q[3];
    rx_r++;
    return 1;
}
int usb_midi_put(const uint8_t pk[4])
{
    if (!conf) return 0;
    if (tx_w - tx_r >= TXN) { lost++; return 0; }
    uint8_t *q = txq[tx_w % TXN];
    q[0] = pk[0]; q[1] = pk[1]; q[2] = pk[2]; q[3] = pk[3];
    tx_w++;
    return 1;
}
uint32_t usb_midi_lost(void) { return lost; }
