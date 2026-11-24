/* SPDX-License-Identifier: GPL-3.0-only
 * The FM-1's USB port as a class-compliant USB-MIDI device (usb.c): one port each way, "FM-1 SLOOP 3", and a USB
 * audio input (the output, for a computer to record). Served by usb_tick() from the 2 kHz interrupt (irq.c); before
 * it runs, usb_midi_poll() from the main loop. Nothing in it waits for the host. A packet is a USB-MIDI event
 * packet (4 bytes: cable << 4 | code index, then the message's bytes). */
#pragma once
#include <stdint.h>
void usb_midi_open(void);                  /* on the bus (25 ms: before the sound starts) */
void usb_midi_poll(void);
int usb_midi_ready(void);                  /* a host has configured us */
int usb_host_live(void);                  /* (3.3.8) configured and its frames still coming: 0 within 0.1 s of the cable pulled */
void usb_midi_close(void);                 /* off the bus: the host sees us go */
int usb_midi_take(uint8_t pk[4]);          /* a packet received: 1, or 0 none */
int usb_midi_put(const uint8_t pk[4]);     /* a packet queued to send: 1, or 0 the queue full (it is lost) */
uint32_t usb_midi_lost(void);              /* packets lost (in or out) since the start */
void usb_tick(void);                       /* (the 2 kHz interrupt) */
void usb_by_tick(void);                    /* (then usb_midi_poll does nothing: call before the tick starts) */
void usb_audio_put(const int32_t *lr24, uint32_t frames);   /* a block of the output (24 bits in 32, L R ...) */
void usb_audio_rate(int mode);             /* HOME's USB RATE: 0 the host's choice, 1 44.1 kHz, 2 48 kHz */
