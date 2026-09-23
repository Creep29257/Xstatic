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

#include <stdbool.h>
#include <string.h>
#include "core/channel_hash.h"
#include "pb.h"


uint8_t channel_hash_compute(const channel_slot_t *slot)
{
	static const uint8_t defaultpsk[16] = {0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59,
	                                        0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01};
	uint8_t hash = 0;
	uint8_t psk_bytes[32];
	pb_size_t psk_size = slot->psk.size;

	memcpy(psk_bytes, slot->psk.bytes, slot->psk.size);

	if (psk_size == 1)
	{
		uint8_t psk_index = psk_bytes[0];

		if (psk_index == 0)
		{
			psk_size = 0;
		}
		else
		{
			memcpy(psk_bytes, defaultpsk, sizeof(defaultpsk));
			psk_size = sizeof(defaultpsk);
			psk_bytes[psk_size - 1] = psk_bytes[psk_size - 1] + psk_index - 1;
		}
	}

	for (size_t i = 0; i < sizeof(slot->name); i++)
	{
		hash ^= slot->name[i];
	}
	for (pb_size_t i = 0; i < psk_size; i++)
	{
		hash ^= psk_bytes[i];
	}
	return hash;
}

int channel_hash_find_index(const channel_state_t *cstate, uint8_t hash)
{
    int index = -1;
    for(int i=0; i<8;i++)
    {
        if(cstate->channels[i].hash == hash)
        {
            index = i;
            return index;
        }
    }
    return index;
}
