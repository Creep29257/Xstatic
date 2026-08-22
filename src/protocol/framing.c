/*
 * Copyright (c) 2026 Rémi Assailly
 * Rémi@assailly.com
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

const unsigned char	fix_handshake_part[] = {0x94, 0xc3, 0x00, 0x02, 0x18};
/*
 * framing_feed -- avance le parsing incrémental d'une trame Meshtastic.
 *
 * Le flux série peut être coupé n'importe où par un read() : l'état du
 * parsing (state->phase, state->expected_len, state->payload_pos...) est
 * donc conservé entre les appels via la struct framing_state, passée par
 * pointeur. Chaque appel reprend là où le précédent s'est arrêté.
 *
 * Repère les octets magiques 0x94 0xc3 en début de trame, lit la longueur
 * du payload sur 2 octets big-endian, puis recopie le payload dans
 * state->payload (borné à FRAMING_MAX_PAYLOAD, voir framing.h).
 *
 * state->frame_ready passe à 1 quand une trame complète est disponible.
 * La fonction ne le remet jamais à 0 : c'est à l'appelant (main.c) de le
 * faire une fois la trame lue.
 */
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
            /* 0xc3 après un 0x94 confirme le début d'une trame.
            * Un 0x94 répété ne fait rien (on reste ici, au cas où
            * ce serait un nouveau début de trame qui se prépare).
            * Tout autre octet = bruit/resync, on repart à zéro. */

            if (byte == 0xc3) {
                state->phase = FRAMING_READ_LEN_HI;
            } else if (byte != 0x94) {
                state->phase = FRAMING_WAIT_START1;
            }
            break;

        case FRAMING_READ_LEN_HI:
         FRAMING_READ_LEN_HI:
            /* Longueur du payload encodée en big-endian sur 2 octets :
            * on reçoit d'abord l'octet de poids fort. */
            state->expected_len = (uint16_t)(byte << 8);
            state->phase = FRAMING_READ_LEN_LO;
            break;

        case FRAMING_READ_LEN_LO:
             /* Octet de poids faible : on complète expected_len. */
            state->expected_len = state->expected_len | byte;
            state->payload_pos = 0;
            /* Protection contre débordement de state->payload : si la longueur
            * annoncée dépasse la taille du buffer, on rejette la trame et on
            * repart de zéro plutôt que d'écrire hors des limites du tableau. */
            if (state->expected_len > 512) {
                state->phase = FRAMING_WAIT_START1;
            } else {
                state->phase = FRAMING_READ_PAYLOAD;
            }
           
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

int framing_handshake_construct(unsigned char *out_handshake, size_t len)
{
    
    
    if(out_handshake == NULL || len < HANDSHAKE_LEN)
    {
        fprintf(stderr, "handshake_construct: invalid argument\n");
        return -1;
    }
    memcpy(out_handshake, fix_handshake_part, sizeof(fix_handshake_part));
    out_handshake[5] = arc4random_uniform(127) + 1;
    return 0;
}