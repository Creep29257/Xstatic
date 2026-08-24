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
 * framing.h -- interface de désencapsulation des trames Meshtastic (magique + longueur + payload protobuf).
 */
#ifndef MESHTASTIC_FRAMING_H
#define MESHTASTIC_FRAMING_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FRAMING_MAX_PAYLOAD 512
#define HANDSHAKE_LEN 6
extern const unsigned char	fix_handshake_part[];


enum framing_phase {
    FRAMING_WAIT_START1,
    FRAMING_WAIT_START2,
    FRAMING_READ_LEN_HI,
    FRAMING_READ_LEN_LO,
    FRAMING_READ_PAYLOAD
};

struct framing_state {
    enum framing_phase phase;
    uint16_t expected_len;
    unsigned char payload[FRAMING_MAX_PAYLOAD];
    uint16_t payload_pos;
    int frame_ready;   /* passe à 1 quand une trame vient d'être complétée */
};

void framing_feed(struct framing_state *state, const unsigned char *buf, size_t count);

int framing_handshake_construct(unsigned char *out_handshake, size_t len);

int framing_message_construct(unsigned char *payload, size_t payload_len, unsigned char *out_frame, size_t out_frame_len);
#endif /* MESHTASTIC_FRAMING_H */
