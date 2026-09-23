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


#include "core/dispatch.h"
#include "third_party/nanopb/pb_decode.h"
#include "protocol/hw_model_name.h"
#include "protocol/config_enum_name.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "channel_hash.h"

#define NAME_BUF_SIZE sizeof(((mesh_node_t *)0)->long_name)

void
dispatch_process_frame(struct framing_state *fs, meshtastic_FromRadio *msg,
    mesh_state_t *state, device_config_t *dconfig, message_history_t *history,channel_state_t *cstate)
{
	pb_istream_t stream = pb_istream_from_buffer(fs->payload, fs->payload_pos);

	if (pb_decode(&stream, meshtastic_FromRadio_fields, msg))
	{
		if (msg->which_payload_variant == meshtastic_FromRadio_node_info_tag)
		{
			mesh_node_info_t info;

			info.num = msg->node_info.num;
			strncpy(info.long_name, msg->node_info.user.long_name, MESH_LONG_NAME_MAX);
			info.long_name[MESH_LONG_NAME_MAX - 1] = '\0';
			info.hw_model = msg->node_info.user.hw_model;
			info.position.valid = msg->node_info.has_position;
			info.position.latitude_i = msg->node_info.position.latitude_i;
			info.position.longitude_i = msg->node_info.position.longitude_i;
			info.position.altitude = msg->node_info.position.altitude;
			info.custom_name = NULL;

			bool ok = mesh_state_add_or_update_node(state, &info);
			if (!ok)
			{
				fprintf(stderr, "mesh_state_add_or_update_node failed\n");
			}
		}

		if (msg->which_payload_variant == meshtastic_FromRadio_packet_tag)
		{
			mesh_node_t *from_node = mesh_state_find_node(state, msg->packet.from);
			mesh_node_t *to_node;
			char buffer_from[NAME_BUF_SIZE];
			char buffer_to[NAME_BUF_SIZE];

			if (from_node != NULL)
			{
				snprintf(buffer_from, sizeof(buffer_from), "%s", from_node->long_name);
			} else
			{
				snprintf(buffer_from, sizeof(buffer_from), "unknown: %u", msg->packet.from);
			}

			if (msg->packet.to == 4294967295)
			{
				snprintf(buffer_to, sizeof(buffer_to), "Broadcast");
			} else
			{
				to_node = mesh_state_find_node(state, msg->packet.to);
				if (to_node != NULL)
				{
					snprintf(buffer_to, sizeof(buffer_to), "%s", to_node->long_name);
				} else
				{
					snprintf(buffer_to, sizeof(buffer_to), "unknown: %u", msg->packet.to);
				}
			}

			if (msg->packet.which_payload_variant == meshtastic_MeshPacket_decoded_tag)
			{
				if (msg->packet.decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP)
				{
					size_t text_size = sizeof(msg->packet.decoded.payload.bytes) + 1;

					if (msg->packet.decoded.payload.size <= (text_size - 1))
					{
						char text[text_size];

						memcpy(text, msg->packet.decoded.payload.bytes, msg->packet.decoded.payload.size);
						text[msg->packet.decoded.payload.size] = '\0';
						uint32_t assigned_id = message_history_add(history, msg->packet.from, text, channel_hash_find_index(cstate, msg->packet.channel));

						printf("\033[7;32m  [%u] from: %s -> %s \033[0m\n", assigned_id, buffer_from, buffer_to);
						printf("\033[7;33m %s \033[0m\n", text);
					}
				}
			} else
			{
				printf("\033[7;31m encrypted message from: %s to: %s \033[0m\n", buffer_from, buffer_to);
			}
		}
		if (msg->which_payload_variant == meshtastic_FromRadio_channel_tag)
		{
			channel_state_update(cstate, &msg->channel);
		}

		if (msg->which_payload_variant == meshtastic_FromRadio_config_tag)
		{
			device_config_update(dconfig, &msg->config);
		}
        if (msg->which_payload_variant == meshtastic_FromRadio_rebooted_tag)
        {
            time_t now;
            struct tm *tm_now;
            char date_buf[32];

            now = time(NULL);
            tm_now = localtime(&now);
            strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", tm_now);
            printf("device has rebooted at %s\n",date_buf);
        }
        if (msg->which_payload_variant == meshtastic_FromRadio_metadata_tag)
        {   
            const char *hw_model = hw_model_name(msg->metadata.hw_model);
            const char *node_role = device_role_name(msg->metadata.role);
            device_config_set_metadata(dconfig, &msg->metadata);
            printf(" node firmware version %s \n", msg->metadata.firmware_version);
            printf("node model %s \n", hw_model );
            printf("node_role %s \n", node_role );

        }
        if (msg->which_payload_variant == meshtastic_FromRadio_queueStatus_tag)
        {
            if (msg->queueStatus.res==0)
            {
                printf("queue: packet %u accepted (%d/%d slots free)\n",
                (unsigned)msg->queueStatus.mesh_packet_id,
                msg->queueStatus.free, msg->queueStatus.maxlen);
            }
            else
            {
                printf("queue: packet %u rejected (error %d, %d/%d slots free)\n",
                (unsigned)msg->queueStatus.mesh_packet_id,
                msg->queueStatus.res,
                msg->queueStatus.free, msg->queueStatus.maxlen);
            }
        }
        
	}
	fs->frame_ready = 0;
}