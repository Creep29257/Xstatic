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
#include "ui/cli_display.h"

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
				channel_display_name(cstate, dconfig,(int8_t)msg->packet.channel , buffer_to, sizeof(buffer_to));
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
						uint32_t assigned_id = message_history_add(history, msg->packet.from, text, (int8_t)msg->packet.channel);
						printf("\033[7;32m  [%u] from: %s -> %s \033[0m\n", assigned_id, buffer_from, buffer_to);
						printf("\033[7;33m %s \033[0m\n", text);
					}
				}
				if (msg->packet.decoded.portnum == meshtastic_PortNum_POSITION_APP)
				{
					pb_istream_t stream = pb_istream_from_buffer(msg->packet.decoded.payload.bytes, msg->packet.decoded.payload.size);
					meshtastic_Position  node_position = meshtastic_Position_init_zero;
					if (pb_decode(&stream, meshtastic_Position_fields, &node_position))
					{
						mesh_node_t *node_to_update_position = mesh_state_find_node(state, msg->packet.from);
						if(node_to_update_position!= NULL)
						{
							node_to_update_position->position.valid = true;
							node_to_update_position->position.latitude_i = node_position.latitude_i;
							node_to_update_position->position.longitude_i = node_position.longitude_i;
							node_to_update_position->position.altitude = node_position.altitude;
						}
						else
						{
							fprintf(stderr,"mesh_state_find_node for position update not found\n");
						}

					}
				}

				if (msg->packet.decoded.portnum == meshtastic_PortNum_TELEMETRY_APP)
				{
					pb_istream_t stream = pb_istream_from_buffer(msg->packet.decoded.payload.bytes, msg->packet.decoded.payload.size);
					meshtastic_Telemetry node_telemetry = meshtastic_Telemetry_init_zero;
					if (pb_decode(&stream, meshtastic_Telemetry_fields, &node_telemetry))
					{
						mesh_node_t *node_to_update_telemetry = mesh_state_find_node(state, msg->packet.from);
						if(node_to_update_telemetry != NULL)
						{
							if (node_telemetry.which_variant == meshtastic_Telemetry_device_metrics_tag)
							{
								node_to_update_telemetry->device_metrics.has_battery_level = node_telemetry.variant.device_metrics.has_battery_level;
								node_to_update_telemetry->device_metrics.battery_level = node_telemetry.variant.device_metrics.battery_level;
								node_to_update_telemetry->device_metrics.has_voltage = node_telemetry.variant.device_metrics.has_voltage;
								node_to_update_telemetry->device_metrics.voltage = node_telemetry.variant.device_metrics.voltage;
							}
							if (node_telemetry.which_variant == meshtastic_Telemetry_environment_metrics_tag)
							{
								node_to_update_telemetry->environment_metrics.has_temperature = node_telemetry.variant.environment_metrics.has_temperature;
								node_to_update_telemetry->environment_metrics.temperature = node_telemetry.variant.environment_metrics.temperature;
								node_to_update_telemetry->environment_metrics.has_relative_humidity = node_telemetry.variant.environment_metrics.has_relative_humidity;
								node_to_update_telemetry->environment_metrics.relative_humidity = node_telemetry.variant.environment_metrics.relative_humidity;
								node_to_update_telemetry->environment_metrics.has_barometric_pressure = node_telemetry.variant.environment_metrics.has_barometric_pressure;
								node_to_update_telemetry->environment_metrics.barometric_pressure = node_telemetry.variant.environment_metrics.barometric_pressure;
							}

						}
						else
						{
							fprintf(stderr, "mesh_state_find_node for telemetry update: not found\n");
						}
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
