/*
 * Copyright (c) 2026 Rémi Assailly All rights reserved.
 * remi@assailly.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer. 2.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * device config.c */

 #include "device_config.h"
 
 void device_config_init(device_config_t *state)
{
    state->has_device = false;
    state->has_position = false;
    state->has_power = false;
    state->has_network = false;
    state->has_display =false;
    state->has_lora = false;
    state->has_bluetooth =false;
    state->has_security = false;
    state->has_sessionkey =false;
    state->has_metadata = false;

}
void device_config_update(device_config_t *state, const meshtastic_Config *cfg)
{
     switch (cfg->which_payload_variant)
    {
        case meshtastic_Config_device_tag:
            state->device = cfg->payload_variant.device;
            state->has_device = true;
            break;

        case meshtastic_Config_position_tag:
        state->position =cfg->payload_variant.position;
        state->has_position = true;
        break;

        case meshtastic_Config_power_tag:
        state->power = cfg->payload_variant.power;
        state->has_power = true;
        break;
        
        case meshtastic_Config_network_tag:
        state->network = cfg->payload_variant.network;
        state->has_network = true;
        break;

        case meshtastic_Config_display_tag:
        state->display = cfg->payload_variant.display;
        state->has_display = true;
        break;

        case meshtastic_Config_lora_tag:
        state->lora = cfg->payload_variant.lora;
        state->has_lora = true;
        break;

        case meshtastic_Config_bluetooth_tag:
        state->bluetooth = cfg->payload_variant.bluetooth;
        state->has_bluetooth = true;
        break;

        case meshtastic_Config_security_tag:
        state->security = cfg->payload_variant.security;
        state->has_security = true;
        break;

        case meshtastic_Config_sessionkey_tag:
        state->sessionkey = cfg->payload_variant.sessionkey;
        state->has_sessionkey = true;
        break;

        default:
            break;
    }
   
}
void device_config_set_metadata(device_config_t *state, const meshtastic_DeviceMetadata *metadata)
{
    state->metadata= *metadata;
    state->has_metadata = true;
}