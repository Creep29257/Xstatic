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
 * hw_model_name.c -- table de correspondance meshtastic_HardwareModel ->
 * texte lisible. Générée depuis l'enum meshtastic_HardwareModel de
 * mesh.pb.h (protocole Meshtastic officiel).
 */
#include "hw_model_name.h"

const char *
hw_model_name(meshtastic_HardwareModel model)
{
	switch (model)
	{
	case meshtastic_HardwareModel_UNSET: return "unset";
	case meshtastic_HardwareModel_TLORA_V2: return "TLORA_V2";
	case meshtastic_HardwareModel_TLORA_V1: return "TLORA_V1";
	case meshtastic_HardwareModel_TLORA_V2_1_1P6: return "TLORA_V2_1_1P6";
	case meshtastic_HardwareModel_TBEAM: return "TBEAM";
	case meshtastic_HardwareModel_HELTEC_V2_0: return "HELTEC_V2_0";
	case meshtastic_HardwareModel_TBEAM_V0P7: return "TBEAM_V0P7";
	case meshtastic_HardwareModel_T_ECHO: return "T_ECHO";
	case meshtastic_HardwareModel_TLORA_V1_1P3: return "TLORA_V1_1P3";
	case meshtastic_HardwareModel_RAK4631: return "RAK4631";
	case meshtastic_HardwareModel_HELTEC_V2_1: return "HELTEC_V2_1";
	case meshtastic_HardwareModel_HELTEC_V1: return "HELTEC_V1";
	case meshtastic_HardwareModel_LILYGO_TBEAM_S3_CORE: return "LILYGO_TBEAM_S3_CORE";
	case meshtastic_HardwareModel_RAK11200: return "RAK11200";
	case meshtastic_HardwareModel_NANO_G1: return "NANO_G1";
	case meshtastic_HardwareModel_TLORA_V2_1_1P8: return "TLORA_V2_1_1P8";
	case meshtastic_HardwareModel_TLORA_T3_S3: return "TLORA_T3_S3";
	case meshtastic_HardwareModel_NANO_G1_EXPLORER: return "NANO_G1_EXPLORER";
	case meshtastic_HardwareModel_NANO_G2_ULTRA: return "NANO_G2_ULTRA";
	case meshtastic_HardwareModel_LORA_TYPE: return "LORA_TYPE";
	case meshtastic_HardwareModel_WIPHONE: return "WIPHONE";
	case meshtastic_HardwareModel_WIO_WM1110: return "WIO_WM1110";
	case meshtastic_HardwareModel_RAK2560: return "RAK2560";
	case meshtastic_HardwareModel_HELTEC_HRU_3601: return "HELTEC_HRU_3601";
	case meshtastic_HardwareModel_HELTEC_WIRELESS_BRIDGE: return "HELTEC_WIRELESS_BRIDGE";
	case meshtastic_HardwareModel_STATION_G1: return "STATION_G1";
	case meshtastic_HardwareModel_RAK11310: return "RAK11310";
	case meshtastic_HardwareModel_MAKERFABS_TRACKER: return "MAKERFABS_TRACKER";
	case meshtastic_HardwareModel_MAKERFABS_RESERVED: return "MAKERFABS_RESERVED";
	case meshtastic_HardwareModel_CANARYONE: return "CANARYONE";
	case meshtastic_HardwareModel_RP2040_LORA: return "RP2040_LORA";
	case meshtastic_HardwareModel_STATION_G2: return "STATION_G2";
	case meshtastic_HardwareModel_LORA_RELAY_V1: return "LORA_RELAY_V1";
	case meshtastic_HardwareModel_T_ECHO_PLUS: return "T_ECHO_PLUS";
	case meshtastic_HardwareModel_PPR: return "PPR";
	case meshtastic_HardwareModel_GENIEBLOCKS: return "GENIEBLOCKS";
	case meshtastic_HardwareModel_NRF52_UNKNOWN: return "NRF52_UNKNOWN";
	case meshtastic_HardwareModel_PORTDUINO: return "PORTDUINO";
	case meshtastic_HardwareModel_ANDROID_SIM: return "ANDROID_SIM";
	case meshtastic_HardwareModel_DIY_V1: return "DIY_V1";
	case meshtastic_HardwareModel_NRF52840_PCA10059: return "NRF52840_PCA10059";
	case meshtastic_HardwareModel_DR_DEV: return "DR_DEV";
	case meshtastic_HardwareModel_M5STACK: return "M5STACK";
	case meshtastic_HardwareModel_HELTEC_V3: return "HELTEC_V3";
	case meshtastic_HardwareModel_HELTEC_WSL_V3: return "HELTEC_WSL_V3";
	case meshtastic_HardwareModel_BETAFPV_2400_TX: return "BETAFPV_2400_TX";
	case meshtastic_HardwareModel_BETAFPV_900_NANO_TX: return "BETAFPV_900_NANO_TX";
	case meshtastic_HardwareModel_RPI_PICO: return "RPI_PICO";
	case meshtastic_HardwareModel_HELTEC_WIRELESS_TRACKER: return "HELTEC_WIRELESS_TRACKER";
	case meshtastic_HardwareModel_HELTEC_WIRELESS_PAPER: return "HELTEC_WIRELESS_PAPER";
	case meshtastic_HardwareModel_T_DECK: return "T_DECK";
	case meshtastic_HardwareModel_T_WATCH_S3: return "T_WATCH_S3";
	case meshtastic_HardwareModel_PICOMPUTER_S3: return "PICOMPUTER_S3";
	case meshtastic_HardwareModel_HELTEC_HT62: return "HELTEC_HT62";
	case meshtastic_HardwareModel_EBYTE_ESP32_S3: return "EBYTE_ESP32_S3";
	case meshtastic_HardwareModel_ESP32_S3_PICO: return "ESP32_S3_PICO";
	case meshtastic_HardwareModel_CHATTER_2: return "CHATTER_2";
	case meshtastic_HardwareModel_HELTEC_WIRELESS_PAPER_V1_0: return "HELTEC_WIRELESS_PAPER_V1_0";
	case meshtastic_HardwareModel_HELTEC_WIRELESS_TRACKER_V1_0: return "HELTEC_WIRELESS_TRACKER_V1_0";
	case meshtastic_HardwareModel_UNPHONE: return "UNPHONE";
	case meshtastic_HardwareModel_TD_LORAC: return "TD_LORAC";
	case meshtastic_HardwareModel_CDEBYTE_EORA_S3: return "CDEBYTE_EORA_S3";
	case meshtastic_HardwareModel_TWC_MESH_V4: return "TWC_MESH_V4";
	case meshtastic_HardwareModel_NRF52_PROMICRO_DIY: return "NRF52_PROMICRO_DIY";
	case meshtastic_HardwareModel_RADIOMASTER_900_BANDIT_NANO: return "RADIOMASTER_900_BANDIT_NANO";
	case meshtastic_HardwareModel_HELTEC_CAPSULE_SENSOR_V3: return "HELTEC_CAPSULE_SENSOR_V3";
	case meshtastic_HardwareModel_HELTEC_VISION_MASTER_T190: return "HELTEC_VISION_MASTER_T190";
	case meshtastic_HardwareModel_HELTEC_VISION_MASTER_E213: return "HELTEC_VISION_MASTER_E213";
	case meshtastic_HardwareModel_HELTEC_VISION_MASTER_E290: return "HELTEC_VISION_MASTER_E290";
	case meshtastic_HardwareModel_HELTEC_MESH_NODE_T114: return "HELTEC_MESH_NODE_T114";
	case meshtastic_HardwareModel_SENSECAP_INDICATOR: return "SENSECAP_INDICATOR";
	case meshtastic_HardwareModel_TRACKER_T1000_E: return "TRACKER_T1000_E";
	case meshtastic_HardwareModel_RAK3172: return "RAK3172";
	case meshtastic_HardwareModel_WIO_E5: return "WIO_E5";
	case meshtastic_HardwareModel_RADIOMASTER_900_BANDIT: return "RADIOMASTER_900_BANDIT";
	case meshtastic_HardwareModel_ME25LS01_4Y10TD: return "ME25LS01_4Y10TD";
	case meshtastic_HardwareModel_RP2040_FEATHER_RFM95: return "RP2040_FEATHER_RFM95";
	case meshtastic_HardwareModel_M5STACK_COREBASIC: return "M5STACK_COREBASIC";
	case meshtastic_HardwareModel_M5STACK_CORE2: return "M5STACK_CORE2";
	case meshtastic_HardwareModel_RPI_PICO2: return "RPI_PICO2";
	case meshtastic_HardwareModel_M5STACK_CORES3: return "M5STACK_CORES3";
	case meshtastic_HardwareModel_SEEED_XIAO_S3: return "SEEED_XIAO_S3";
	case meshtastic_HardwareModel_MS24SF1: return "MS24SF1";
	case meshtastic_HardwareModel_TLORA_C6: return "TLORA_C6";
	case meshtastic_HardwareModel_WISMESH_TAP: return "WISMESH_TAP";
	case meshtastic_HardwareModel_ROUTASTIC: return "ROUTASTIC";
	case meshtastic_HardwareModel_MESH_TAB: return "MESH_TAB";
	case meshtastic_HardwareModel_MESHLINK: return "MESHLINK";
	case meshtastic_HardwareModel_XIAO_NRF52_KIT: return "XIAO_NRF52_KIT";
	case meshtastic_HardwareModel_THINKNODE_M1: return "THINKNODE_M1";
	case meshtastic_HardwareModel_THINKNODE_M2: return "THINKNODE_M2";
	case meshtastic_HardwareModel_T_ETH_ELITE: return "T_ETH_ELITE";
	case meshtastic_HardwareModel_HELTEC_SENSOR_HUB: return "HELTEC_SENSOR_HUB";
	case meshtastic_HardwareModel_MUZI_BASE: return "MUZI_BASE";
	case meshtastic_HardwareModel_HELTEC_MESH_POCKET: return "HELTEC_MESH_POCKET";
	case meshtastic_HardwareModel_SEEED_SOLAR_NODE: return "SEEED_SOLAR_NODE";
	case meshtastic_HardwareModel_NOMADSTAR_METEOR_PRO: return "NOMADSTAR_METEOR_PRO";
	case meshtastic_HardwareModel_CROWPANEL: return "CROWPANEL";
	case meshtastic_HardwareModel_LINK_32: return "LINK_32";
	case meshtastic_HardwareModel_SEEED_WIO_TRACKER_L1: return "SEEED_WIO_TRACKER_L1";
	case meshtastic_HardwareModel_SEEED_WIO_TRACKER_L1_EINK: return "SEEED_WIO_TRACKER_L1_EINK";
	case meshtastic_HardwareModel_MUZI_R1_NEO: return "MUZI_R1_NEO";
	case meshtastic_HardwareModel_T_DECK_PRO: return "T_DECK_PRO";
	case meshtastic_HardwareModel_T_LORA_PAGER: return "T_LORA_PAGER";
	case meshtastic_HardwareModel_M5STACK_RESERVED: return "M5STACK_RESERVED";
	case meshtastic_HardwareModel_WISMESH_TAG: return "WISMESH_TAG";
	case meshtastic_HardwareModel_RAK3312: return "RAK3312";
	case meshtastic_HardwareModel_THINKNODE_M5: return "THINKNODE_M5";
	case meshtastic_HardwareModel_HELTEC_MESH_SOLAR: return "HELTEC_MESH_SOLAR";
	case meshtastic_HardwareModel_T_ECHO_LITE: return "T_ECHO_LITE";
	case meshtastic_HardwareModel_HELTEC_V4: return "HELTEC_V4";
	case meshtastic_HardwareModel_M5STACK_C6L: return "M5STACK_C6L";
	case meshtastic_HardwareModel_M5STACK_CARDPUTER_ADV: return "M5STACK_CARDPUTER_ADV";
	case meshtastic_HardwareModel_HELTEC_WIRELESS_TRACKER_V2: return "HELTEC_WIRELESS_TRACKER_V2";
	case meshtastic_HardwareModel_T_WATCH_ULTRA: return "T_WATCH_ULTRA";
	case meshtastic_HardwareModel_THINKNODE_M3: return "THINKNODE_M3";
	case meshtastic_HardwareModel_WISMESH_TAP_V2: return "WISMESH_TAP_V2";
	case meshtastic_HardwareModel_RAK3401: return "RAK3401";
	case meshtastic_HardwareModel_RAK6421: return "RAK6421";
	case meshtastic_HardwareModel_THINKNODE_M4: return "THINKNODE_M4";
	case meshtastic_HardwareModel_THINKNODE_M6: return "THINKNODE_M6";
	case meshtastic_HardwareModel_MESHSTICK_1262: return "MESHSTICK_1262";
	case meshtastic_HardwareModel_TBEAM_1_WATT: return "TBEAM_1_WATT";
	case meshtastic_HardwareModel_T5_S3_EPAPER_PRO: return "T5_S3_EPAPER_PRO";
	case meshtastic_HardwareModel_TBEAM_BPF: return "TBEAM_BPF";
	case meshtastic_HardwareModel_MINI_EPAPER_S3: return "MINI_EPAPER_S3";
	case meshtastic_HardwareModel_TDISPLAY_S3_PRO: return "TDISPLAY_S3_PRO";
	case meshtastic_HardwareModel_HELTEC_MESH_NODE_T096: return "HELTEC_MESH_NODE_T096";
	case meshtastic_HardwareModel_MESH_TRACKER_X1: return "MESH_TRACKER_X1";
	case meshtastic_HardwareModel_THINKNODE_M7: return "THINKNODE_M7";
	case meshtastic_HardwareModel_THINKNODE_M8: return "THINKNODE_M8";
	case meshtastic_HardwareModel_THINKNODE_M9: return "THINKNODE_M9";
	case meshtastic_HardwareModel_HELTEC_V4_R8: return "HELTEC_V4_R8";
	case meshtastic_HardwareModel_HELTEC_MESH_NODE_T1: return "HELTEC_MESH_NODE_T1";
	case meshtastic_HardwareModel_STATION_G3: return "STATION_G3";
	case meshtastic_HardwareModel_T_IMPULSE_PLUS: return "T_IMPULSE_PLUS";
	case meshtastic_HardwareModel_T_ECHO_CARD: return "T_ECHO_CARD";
	case meshtastic_HardwareModel_SEEED_WIO_TRACKER_L2: return "SEEED_WIO_TRACKER_L2";
	case meshtastic_HardwareModel_CROWPANEL_P4: return "CROWPANEL_P4";
	case meshtastic_HardwareModel_HELTEC_MESH_TOWER_V2: return "HELTEC_MESH_TOWER_V2";
	case meshtastic_HardwareModel_MESHNOLOGY_W10: return "MESHNOLOGY_W10";
	case meshtastic_HardwareModel_HELTEC_RC32: return "HELTEC_RC32";
	case meshtastic_HardwareModel_HELTEC_RC52: return "HELTEC_RC52";
	case meshtastic_HardwareModel_HELTEC_RCC6: return "HELTEC_RCC6";
	case meshtastic_HardwareModel_PRIVATE_HW: return "PRIVATE_HW";
	default: return "unknown";
	}
}