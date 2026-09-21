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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

#include "meshtastic/config.pb.h"
#include "meshtastic/mesh.pb.h"

/*
 * device_config_t garde l'etat cumule des 9 sous-configs recues via le
 * tag "config" de FromRadio. Le firmware envoie une variante par paquet
 * pendant le dump initial (pas tout d'un coup) : chaque has_X indique si
 * cette variante a effectivement ete recue depuis le dernier
 * device_config_init().
 *
 * device_ui (DeviceUIConfig) est volontairement absent ici : non
 * pertinent sans ecran tactile (RAK4631), et deja couvert separement par
 * le tag FromRadio.deviceuiConfig si besoin plus tard.
 */
typedef struct
{
    bool has_device;
    meshtastic_Config_DeviceConfig device;

    bool has_position;
    meshtastic_Config_PositionConfig position;

    bool has_power;
    meshtastic_Config_PowerConfig power;

    bool has_network;
    meshtastic_Config_NetworkConfig network;

    bool has_display;
    meshtastic_Config_DisplayConfig display;

    bool has_lora;
    meshtastic_Config_LoRaConfig lora;

    bool has_bluetooth;
    meshtastic_Config_BluetoothConfig bluetooth;

    bool has_security;
    meshtastic_Config_SecurityConfig security;

    bool has_sessionkey;
    meshtastic_Config_SessionkeyConfig sessionkey;

    bool has_metadata;
    meshtastic_DeviceMetadata metadata;

} device_config_t;

/*
 * Remet state a zero : tous les has_X a false, structs sous-jacentes
 * indefinies tant que has_X n'est pas passe a true par device_config_update.
 * A appeler une fois avant le premier dump (ou avant un re-dump complet).
 */
void device_config_init(device_config_t *state);

/*
 * Sous-switch sur cfg->which_payload_variant : copie la variante recue
 * dans le champ correspondant de state et met son has_X a true.
 * A appeler depuis main_interactive.c a chaque paquet FromRadio dont le
 * tag est "config".
 */
void device_config_update(device_config_t *state, const meshtastic_Config *cfg);

void device_config_set_metadata(device_config_t *state, const meshtastic_DeviceMetadata *metadata);
#endif /* DEVICE_CONFIG_H */