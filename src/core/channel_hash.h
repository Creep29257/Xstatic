/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2026, Rémi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * NOTE: ce module calcule le hash de channel Meshtastic (xorHash sur nom+PSK,
 * fidèle à Channels::generateHash du firmware officiel), mais N'EST PAS
 * utilisé actuellement par dispatch.c. Le champ MeshPacket.channel que le
 * client reçoit via l'API série contient déjà l'index local du channel
 * (0-7), pas un hash à résoudre côté client — le "hash trick" n'existe que
 * temporairement, en interne au firmware du device, pendant le déchiffrement.
 * Conservé pour référence / usage futur potentiel (ex: URLs de partage de
 * channel meshtastic.org/e/#, qui encodent ce même hash).
 */
#ifndef CHANNEL_HASH_H
#define CHANNEL_HASH_H

#include <stdint.h>
#include "channel_state.h"



uint8_t channel_hash_compute(const channel_slot_t *slot);
int channel_hash_find_index(const channel_state_t *cstate, uint8_t hash);

#endif