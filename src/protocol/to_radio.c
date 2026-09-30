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
 *to_radio, ensemble des fonction d'intercation avec la radio
 *
 * */

#include "to_radio.h"
#include "framing.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "third_party/nanopb/pb_encode.h" 


int
to_radio_construct(const send_target_t *target, char *message, meshtastic_ToRadio *out)
{
	size_t text_size = sizeof(out->packet.decoded.payload.bytes);

	out->which_payload_variant = meshtastic_ToRadio_packet_tag;
	out->packet.to = target->to;
	out->packet.from = 0;
	out->packet.channel = target->channel;
	out->packet.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
	out->packet.decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
	out->packet.want_ack = true;
	out->packet.priority = meshtastic_MeshPacket_Priority_RELIABLE;

	if (strlen(message) <= text_size)
	{
		memcpy(out->packet.decoded.payload.bytes, message, strlen(message));
		out->packet.decoded.payload.size = strlen(message);
	}
	else
	{
		fprintf(stderr, "to_radio_construct : message is too long\n");
		return -1;
	}
	return 0;
}

/*
 * to_radio_encode: encode un meshtastic_ToRadio en protobuf brut dans
 * out_buffer. Taille réelle récupérée via stream.bytes_written (le
 * message ne remplit pas forcément tout FRAMING_MAX_PAYLOAD).
 */
int
to_radio_encode(meshtastic_ToRadio *to_radio, uint8_t *out_buffer, size_t *out_len)
{
	pb_ostream_t stream = pb_ostream_from_buffer(out_buffer, FRAMING_MAX_PAYLOAD);

	if (pb_encode(&stream, meshtastic_ToRadio_fields, to_radio) == false)
	{
		fprintf(stderr, "to_radio_encode : cant encode msg\n");
		return -1;
	}
	*out_len = stream.bytes_written;
	return 0;
}


