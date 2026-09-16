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
 * config_enum_name.c -- voir config_enum_name.h
 */
#include "config_enum_name.h"
 
const char *
device_role_name(meshtastic_Config_DeviceConfig_Role role)
{
	switch (role)
	{
	case meshtastic_Config_DeviceConfig_Role_CLIENT: return "CLIENT";
	case meshtastic_Config_DeviceConfig_Role_CLIENT_MUTE: return "CLIENT_MUTE";
	case meshtastic_Config_DeviceConfig_Role_ROUTER: return "ROUTER";
	case meshtastic_Config_DeviceConfig_Role_ROUTER_CLIENT: return "ROUTER_CLIENT";
	case meshtastic_Config_DeviceConfig_Role_REPEATER: return "REPEATER";
	case meshtastic_Config_DeviceConfig_Role_TRACKER: return "TRACKER";
	case meshtastic_Config_DeviceConfig_Role_SENSOR: return "SENSOR";
	case meshtastic_Config_DeviceConfig_Role_TAK: return "TAK";
	case meshtastic_Config_DeviceConfig_Role_CLIENT_HIDDEN: return "CLIENT_HIDDEN";
	case meshtastic_Config_DeviceConfig_Role_LOST_AND_FOUND: return "LOST_AND_FOUND";
	case meshtastic_Config_DeviceConfig_Role_TAK_TRACKER: return "TAK_TRACKER";
	case meshtastic_Config_DeviceConfig_Role_ROUTER_LATE: return "ROUTER_LATE";
	case meshtastic_Config_DeviceConfig_Role_CLIENT_BASE: return "CLIENT_BASE";
	default: return "unknown";
	}
}
 
const char *
lora_region_name(meshtastic_Config_LoRaConfig_RegionCode region)
{
	switch (region)
	{
	case meshtastic_Config_LoRaConfig_RegionCode_UNSET: return "UNSET";
	case meshtastic_Config_LoRaConfig_RegionCode_US: return "US";
	case meshtastic_Config_LoRaConfig_RegionCode_EU_433: return "EU_433";
	case meshtastic_Config_LoRaConfig_RegionCode_EU_868: return "EU_868";
	case meshtastic_Config_LoRaConfig_RegionCode_CN: return "CN";
	case meshtastic_Config_LoRaConfig_RegionCode_JP: return "JP";
	case meshtastic_Config_LoRaConfig_RegionCode_ANZ: return "ANZ";
	case meshtastic_Config_LoRaConfig_RegionCode_KR: return "KR";
	case meshtastic_Config_LoRaConfig_RegionCode_TW: return "TW";
	case meshtastic_Config_LoRaConfig_RegionCode_RU: return "RU";
	case meshtastic_Config_LoRaConfig_RegionCode_IN: return "IN";
	case meshtastic_Config_LoRaConfig_RegionCode_NZ_865: return "NZ_865";
	case meshtastic_Config_LoRaConfig_RegionCode_TH: return "TH";
	case meshtastic_Config_LoRaConfig_RegionCode_LORA_24: return "LORA_24";
	case meshtastic_Config_LoRaConfig_RegionCode_UA_433: return "UA_433";
	case meshtastic_Config_LoRaConfig_RegionCode_UA_868: return "UA_868";
	case meshtastic_Config_LoRaConfig_RegionCode_MY_433: return "MY_433";
	case meshtastic_Config_LoRaConfig_RegionCode_MY_919: return "MY_919";
	case meshtastic_Config_LoRaConfig_RegionCode_SG_923: return "SG_923";
	case meshtastic_Config_LoRaConfig_RegionCode_PH_433: return "PH_433";
	case meshtastic_Config_LoRaConfig_RegionCode_PH_868: return "PH_868";
	case meshtastic_Config_LoRaConfig_RegionCode_PH_915: return "PH_915";
	case meshtastic_Config_LoRaConfig_RegionCode_ANZ_433: return "ANZ_433";
	case meshtastic_Config_LoRaConfig_RegionCode_KZ_433: return "KZ_433";
	case meshtastic_Config_LoRaConfig_RegionCode_KZ_863: return "KZ_863";
	case meshtastic_Config_LoRaConfig_RegionCode_NP_865: return "NP_865";
	case meshtastic_Config_LoRaConfig_RegionCode_BR_902: return "BR_902";
	case meshtastic_Config_LoRaConfig_RegionCode_ITU1_2M: return "ITU1_2M";
	case meshtastic_Config_LoRaConfig_RegionCode_ITU2_2M: return "ITU2_2M";
	case meshtastic_Config_LoRaConfig_RegionCode_EU_866: return "EU_866";
	case meshtastic_Config_LoRaConfig_RegionCode_EU_874: return "EU_874";
	case meshtastic_Config_LoRaConfig_RegionCode_EU_917: return "EU_917";
	case meshtastic_Config_LoRaConfig_RegionCode_EU_N_868: return "EU_N_868";
	case meshtastic_Config_LoRaConfig_RegionCode_ITU3_2M: return "ITU3_2M";
	case meshtastic_Config_LoRaConfig_RegionCode_ITU1_70CM: return "ITU1_70CM";
	case meshtastic_Config_LoRaConfig_RegionCode_ITU2_70CM: return "ITU2_70CM";
	case meshtastic_Config_LoRaConfig_RegionCode_ITU3_70CM: return "ITU3_70CM";
	case meshtastic_Config_LoRaConfig_RegionCode_ITU2_125CM: return "ITU2_125CM";
	default: return "unknown";
	}
}
 
const char *
modem_preset_name(meshtastic_Config_LoRaConfig_ModemPreset preset)
{
	switch (preset)
	{
	case meshtastic_Config_LoRaConfig_ModemPreset_LONG_FAST: return "LONG_FAST";
	case meshtastic_Config_LoRaConfig_ModemPreset_LONG_SLOW: return "LONG_SLOW";
	case meshtastic_Config_LoRaConfig_ModemPreset_VERY_LONG_SLOW: return "VERY_LONG_SLOW";
	case meshtastic_Config_LoRaConfig_ModemPreset_MEDIUM_SLOW: return "MEDIUM_SLOW";
	case meshtastic_Config_LoRaConfig_ModemPreset_MEDIUM_FAST: return "MEDIUM_FAST";
	case meshtastic_Config_LoRaConfig_ModemPreset_SHORT_SLOW: return "SHORT_SLOW";
	case meshtastic_Config_LoRaConfig_ModemPreset_SHORT_FAST: return "SHORT_FAST";
	case meshtastic_Config_LoRaConfig_ModemPreset_LONG_MODERATE: return "LONG_MODERATE";
	case meshtastic_Config_LoRaConfig_ModemPreset_SHORT_TURBO: return "SHORT_TURBO";
	case meshtastic_Config_LoRaConfig_ModemPreset_LONG_TURBO: return "LONG_TURBO";
	case meshtastic_Config_LoRaConfig_ModemPreset_LITE_FAST: return "LITE_FAST";
	case meshtastic_Config_LoRaConfig_ModemPreset_LITE_SLOW: return "LITE_SLOW";
	case meshtastic_Config_LoRaConfig_ModemPreset_NARROW_FAST: return "NARROW_FAST";
	case meshtastic_Config_LoRaConfig_ModemPreset_NARROW_SLOW: return "NARROW_SLOW";
	case meshtastic_Config_LoRaConfig_ModemPreset_TINY_FAST: return "TINY_FAST";
	case meshtastic_Config_LoRaConfig_ModemPreset_TINY_SLOW: return "TINY_SLOW";
	case meshtastic_Config_LoRaConfig_ModemPreset_MEDIUM_TURBO: return "MEDIUM_TURBO";
	default: return "unknown";
	}
}
 
