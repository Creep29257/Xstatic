/*
 * Copyright (c) 2026 Rémi
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * framing.c -- implémentation du désencapsulage/encapsulage des trames Meshtastic.
 */

#include "framing.h"

/* TODO: implémentation framing */
void
framing_feed(struct framing_state *state, const unsigned char *buf, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        unsigned char byte = buf[i];

        switch (state->phase) {
        case FRAMING_WAIT_START1:
            if (byte == 0x94) {
                state->phase = FRAMING_WAIT_START2;
            }
            break;

        case FRAMING_WAIT_START2:
            if (byte == 0xc3) {
                state->phase = FRAMING_READ_LEN_HI;
            } else if (byte != 0x94) {
                state->phase = FRAMING_WAIT_START1;
            }
            break;

        case FRAMING_READ_LEN_HI:
            state->expected_len = (uint16_t)(byte << 8);
            state->phase = FRAMING_READ_LEN_LO;
            break;

        case FRAMING_READ_LEN_LO:
            state->expected_len = state->expected_len | byte;
            state->payload_pos = 0;
            state->phase = FRAMING_READ_PAYLOAD;
            break;

        case FRAMING_READ_PAYLOAD:
            state->payload[state->payload_pos] = byte;
            state->payload_pos++;

            if (state->payload_pos == state->expected_len) {
                state->phase = FRAMING_WAIT_START1;
                state->frame_ready = 1;
            }
            break;
        }
    }
}
