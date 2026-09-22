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


#include "ui/cli_display.h"
#include "protocol/hw_model_name.h"
#include "protocol/config_enum_name.h"
#include "core/channel_state.h"
#include <stdio.h>

void
cli_display_node_list(mesh_state_t *state)
{
	mesh_node_t *node_cursor = mesh_state_first_node(state);
	int pos_node = 1;

	while (node_cursor != NULL)
	{
		printf("\033[36m[%d] %s\033[0m\n", pos_node, node_cursor->long_name);
		printf("    num:      %u\n", node_cursor->num);
		printf("    hw model: %s\n", hw_model_name(node_cursor->hw_model));
		if (node_cursor->position.valid == 1)
		{
			double lat = node_cursor->position.latitude_i / 10000000.0;
			double lon = node_cursor->position.longitude_i / 10000000.0;

			printf("    position: %f, %f\n", lat, lon);
		}
		printf("\n");

		pos_node++;
		node_cursor = mesh_state_next_node(node_cursor);
	}
}

void
cli_display_device_config(device_config_t *dconfig, bool show_all)
{
	if (dconfig->has_device)
	{
		printf("role: %s\n", device_role_name(dconfig->device.role));
	} else
	{
		printf("role: not received\n");
	}

	if (dconfig->has_lora)
	{
		printf("lora region: %s\n", lora_region_name(dconfig->lora.region));
		printf("lora modem preset: %s\n", modem_preset_name(dconfig->lora.modem_preset));
		printf("lora tx power: %d dBm\n", dconfig->lora.tx_power);
		printf("lora hop limit: %d\n", dconfig->lora.hop_limit);
	} else
	{
		printf("lora: not received\n");
	}

	if (!show_all)
	{
		return;
	}

	printf("\n-- show all config --\n\n");

	if (dconfig->has_position)
	{
		printf("position broadcast secs: %u\n", dconfig->position.position_broadcast_secs);
		printf("position gps enabled: %s\n", dconfig->position.gps_enabled ? "true" : "false");
	} else
	{
		printf("position: not received\n");
	}

	if (dconfig->has_power)
	{
		printf("power is_power_saving: %s\n", dconfig->power.is_power_saving ? "true" : "false");
		printf("power ls_secs: %u\n", dconfig->power.ls_secs);
	} else
	{
		printf("power: not received\n");
	}

	if (dconfig->has_network)
	{
		printf("network wifi_enabled: %s\n", dconfig->network.wifi_enabled ? "true" : "false");
		printf("network eth_enabled: %s\n", dconfig->network.eth_enabled ? "true" : "false");
	} else
	{
		printf("network: not received\n");
	}

	if (dconfig->has_display)
	{
		printf("display screen_on_secs: %u\n", dconfig->display.screen_on_secs);
		printf("display units: %d\n", dconfig->display.units);
	} else
	{
		printf("display: not received\n");
	}

	if (dconfig->has_bluetooth)
	{
		printf("bluetooth enabled: %s\n", dconfig->bluetooth.enabled ? "true" : "false");
		printf("bluetooth mode: %d\n", dconfig->bluetooth.mode);
	} else
	{
		printf("bluetooth: not received\n");
	}
    if (dconfig->has_metadata)
	{
		printf("firmware: %s\n", dconfig->metadata.firmware_version);
        printf("hw model: %s\n", hw_model_name(dconfig->metadata.hw_model));
        printf("role: %s\n", device_role_name(dconfig->metadata.role));
	} else
	{
		printf("metadata: not received\n");
	}

	if (dconfig->has_security)
	{
		printf("security serial_enabled: %s\n", dconfig->security.serial_enabled ? "true" : "false");
		printf("security debug_log_api_enabled: %s\n", dconfig->security.debug_log_api_enabled ? "true" : "false");
	} else
	{
		printf("security: not received\n");
	}

	if (dconfig->has_sessionkey)
	{
		printf("sessionkey: received (pas de champ utile a afficher)\n");
	} else
	{
		printf("sessionkey: not received\n");
	}
}

void cli_display_message_history(message_history_t *hist)
{
    uint8_t start_index, count;
message_entry_t *entries = message_history_list(hist, &start_index, &count);

for (uint8_t i = 0; i < count; i++)
	{
		uint8_t idx = (start_index + i) % MESSAGE_HISTORY_SIZE;
		printf("[%u] %s\n", entries[idx].id, entries[idx].text);
	}
}

void cli_display_channel_list(channel_state_t *cstate)
{
	 for(uint8_t i = 0; i<8; i++)
	 { 
		if(cstate->channels[i].role !=meshtastic_Channel_Role_DISABLED)
		{
			printf("[%u] channel %s \n",i, cstate->channels[i].name);
		}
	 }
}