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
 * main.c -- point d'entrée, boucle select() fusionnant fd série et fd X11.
 */
#include "platform/platform.h"
#include "protocol/framing.h"
#include "core/mesh_state.h"
#include "protocol/generated/meshtastic/mesh.pb.h"
#include "third_party/nanopb/pb_decode.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>


#define NAME_BUF_SIZE sizeof(((mesh_node_t *)0)->long_name)
typedef enum {
	READ_OPTION,
	SEND_OPTION,
	USAGE_OPTION
} option_mode_t;
option_mode_t option;
/*
 * main.c -- point d'entrée provisoire (structure finale à venir,
 * cf design.md : fusion select() avec ui_xlib). Ouvre le port série
 * du RAK4631, envoie le handshake want_config_id, puis lit le flux
 * en boucle : framing_feed() découpe les trames, pb_decode() les
 * décode en meshtastic_FromRadio, et le contenu est affiché selon
 * le tag reçu (MyNodeInfo, NodeInfo, ConfigComplete...).
 *
 * Cette logique devrait globalement survivre, réorganisée en
 * fonctions séparées et branchée sur core/mesh_state au lieu
 * des printf() directs.
 */
volatile sig_atomic_t running = 1;

void
handle_sigint(int sig)
{
	(void)sig;
	running = 0;
}

/*
 * process_frame -- décode et affiche une trame FromRadio complète.
 *
 * Extrait du corps de la boucle principale pour être réutilisable à deux
 * endroits : juste après la phase de validation du handshake (la première
 * trame complète reçue ne doit pas être perdue), et à chaque tour de la
 * boucle de lecture normale. Remet fs->frame_ready à 0 une fois traité.
 */
static void
process_frame(struct framing_state *fs, meshtastic_FromRadio *msg, mesh_state_t *state)
{
	printf("frame_ready=1, payload_pos=%u\n", fs->payload_pos);
	pb_istream_t stream = pb_istream_from_buffer(fs->payload, fs->payload_pos);
	if (pb_decode(&stream, meshtastic_FromRadio_fields, msg)) {
		switch (msg->which_payload_variant) {
		case meshtastic_FromRadio_my_info_tag:
			printf("MyNodeInfo recu, mon node = %u\n", msg->my_info.my_node_num);
			break;
		case meshtastic_FromRadio_node_info_tag:
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
				if (!ok) {
					fprintf(stderr, "mesh_state_add_or_update_node failed\n");
				} else {
					printf("Node bien créé / mis a jour, node num =%u , node long_name = %s , node hw_model= %u \n",
					    info.num, info.long_name, info.hw_model);
				}
			}
			break;
		case meshtastic_FromRadio_config_complete_id_tag:
			printf("ConfigComplete id recu %u\n", msg->config_complete_id);
			break;
		case meshtastic_FromRadio_rebooted_tag:
			if (msg->rebooted) {
				printf("device vient d'etre rebooter \n");
			}
			break;
		case meshtastic_FromRadio_queueStatus_tag:
			printf("QueueStatus recu : res=%d free=%u maxlen=%u mesh_packet_id=%u\n",
			    msg->queueStatus.res, msg->queueStatus.free, msg->queueStatus.maxlen, msg->queueStatus.mesh_packet_id);
			break;
		case meshtastic_FromRadio_fileInfo_tag:
			printf("nom du fichier: %s, taille %u \n", msg->fileInfo.file_name, msg->fileInfo.size_bytes);
			break;
		case meshtastic_FromRadio_xmodemPacket_tag:
			printf("control: %d, seq: %u, crc16: %u \n", msg->xmodemPacket.control, msg->xmodemPacket.seq, msg->xmodemPacket.crc16);
			break;
		case meshtastic_FromRadio_log_record_tag:
			printf("message: %s, time: %u, source: %s, level %d \n", msg->log_record.message, msg->log_record.time, msg->log_record.source, msg->log_record.level);
			break;
		case meshtastic_FromRadio_channel_tag:
			printf("Channel recu : index=%d has_settings=%d role=%d\n", msg->channel.index, msg->channel.has_settings, msg->channel.role);
			break;
		case meshtastic_FromRadio_lockdown_status_tag:
			printf("LockdownStatus recu : state=%d lock_reason=%s\n", msg->lockdown_status.state, msg->lockdown_status.lock_reason);
			break;
		case meshtastic_FromRadio_deviceuiConfig_tag:
			printf("DeviceUIConfig recu : version=%u brightness=%u timeout=%u theme=%d\n", msg->deviceuiConfig.version, msg->deviceuiConfig.screen_brightness, msg->deviceuiConfig.screen_timeout, msg->deviceuiConfig.theme);
			break;
		case meshtastic_FromRadio_metadata_tag:
			printf("DeviceMetadata recu : firmware=%s hw_model=%d wifi=%d bluetooth=%d ethernet=%d\n", msg->metadata.firmware_version, msg->metadata.hw_model, msg->metadata.hasWifi, msg->metadata.hasBluetooth, msg->metadata.hasEthernet);
			break;
		case meshtastic_FromRadio_region_presets_tag:
			printf("LoRaRegionPresetMap recu : groups_count=%u region_groups_count=%u\n", msg->region_presets.groups_count, msg->region_presets.region_groups_count);
			break;
		case meshtastic_FromRadio_mqttClientProxyMessage_tag:
			if (msg->mqttClientProxyMessage.which_payload_variant == meshtastic_MqttClientProxyMessage_text_tag) {
				printf("MqttClientProxyMessage (text) : topic=%s text=%s\n",
				    msg->mqttClientProxyMessage.topic, msg->mqttClientProxyMessage.payload_variant.text);
			} else {
				printf("payload variant binaire\n");
			}
			break;
		case meshtastic_FromRadio_clientNotification_tag:
			printf("client notification tag message: %s\n", msg->clientNotification.message);
			break;
		case meshtastic_FromRadio_config_tag:
			switch (msg->config.which_payload_variant) {
			case meshtastic_Config_device_tag:
				printf("Config recu : sous-type=device\n");
				break;
			case meshtastic_Config_position_tag:
				printf("Config recu : sous-type=position\n");
				break;
			case meshtastic_Config_power_tag:
				printf("Config recu : sous-type=power\n");
				break;
			case meshtastic_Config_network_tag:
				printf("Config recu : sous-type=network\n");
				break;
			case meshtastic_Config_display_tag:
				printf("Config recu : sous-type=display\n");
				break;
			case meshtastic_Config_lora_tag:
				printf("Config recu : sous-type=lora\n");
				break;
			case meshtastic_Config_bluetooth_tag:
				printf("Config recu : sous-type=bluetooth\n");
				break;
			case meshtastic_Config_security_tag:
				printf("Config recu : sous-type=security\n");
				break;
			case meshtastic_Config_sessionkey_tag:
				printf("Config recu : sous-type=sessionkey\n");
				break;
			case meshtastic_Config_device_ui_tag:
				printf("Config recu : sous-type=device_ui\n");
				break;
			default:
				printf("Config recu : sous-type inconnu, tag=%d\n", msg->config.which_payload_variant);
				break;
			}
			break;
		case meshtastic_FromRadio_moduleConfig_tag:
			switch (msg->moduleConfig.which_payload_variant) {
			case meshtastic_ModuleConfig_mqtt_tag:
				printf("ModuleConfig recu : sous-type=mqtt\n");
				break;
			case meshtastic_ModuleConfig_serial_tag:
				printf("ModuleConfig recu : sous-type=serial\n");
				break;
			case meshtastic_ModuleConfig_external_notification_tag:
				printf("ModuleConfig recu : sous-type=external_notification\n");
				break;
			case meshtastic_ModuleConfig_store_forward_tag:
				printf("ModuleConfig recu : sous-type=store_forward\n");
				break;
			case meshtastic_ModuleConfig_range_test_tag:
				printf("ModuleConfig recu : sous-type=range_test\n");
				break;
			case meshtastic_ModuleConfig_telemetry_tag:
				printf("ModuleConfig recu : sous-type=telemetry\n");
				break;
			case meshtastic_ModuleConfig_canned_message_tag:
				printf("ModuleConfig recu : sous-type=canned_message\n");
				break;
			case meshtastic_ModuleConfig_audio_tag:
				printf("ModuleConfig recu : sous-type=audio\n");
				break;
			case meshtastic_ModuleConfig_remote_hardware_tag:
				printf("ModuleConfig recu : sous-type=remote_hardware\n");
				break;
			case meshtastic_ModuleConfig_neighbor_info_tag:
				printf("ModuleConfig recu : sous-type=neighbor_info\n");
				break;
			case meshtastic_ModuleConfig_ambient_lighting_tag:
				printf("ModuleConfig recu : sous-type=ambient_lighting\n");
				break;
			case meshtastic_ModuleConfig_detection_sensor_tag:
				printf("ModuleConfig recu : sous-type=detection_sensor\n");
				break;
			case meshtastic_ModuleConfig_paxcounter_tag:
				printf("ModuleConfig recu : sous-type=paxcounter\n");
				break;
			case meshtastic_ModuleConfig_statusmessage_tag:
				printf("ModuleConfig recu : sous-type=statusmessage\n");
				break;
			case meshtastic_ModuleConfig_traffic_management_tag:
				printf("ModuleConfig recu : sous-type=traffic_management\n");
				break;
			case meshtastic_ModuleConfig_tak_tag:
				printf("ModuleConfig recu : sous-type=tak\n");
				break;
			case meshtastic_ModuleConfig_mesh_beacon_tag:
				printf("ModuleConfig recu : sous-type=mesh_beacon\n");
				break;
			default:
				printf("ModuleConfig recu : sous-type inconnu, tag=%d\n", msg->moduleConfig.which_payload_variant);
				break;
			}
			break;
		case meshtastic_FromRadio_packet_tag:
			{
				mesh_node_t *from_node = mesh_state_find_node(state, msg->packet.from);
				char buffer_from[NAME_BUF_SIZE];
				char buffer_to[NAME_BUF_SIZE];
				if (from_node != NULL) {
				snprintf(buffer_from, sizeof(buffer_from), "%s", from_node->long_name);
				} 
				else
				{
					snprintf(buffer_from,sizeof(buffer_from), "inconnu: %u", msg->packet.from);
					
				}
				if (msg->packet.to == 4294967295) {
					snprintf(buffer_to, sizeof(buffer_to), "Broadcast");
					
				} else {
					mesh_node_t *to_node = mesh_state_find_node(state, msg->packet.to);
					if (to_node != NULL) {
						snprintf(buffer_to, sizeof(buffer_to), "%s",to_node->long_name);
						
					} else {
						snprintf(buffer_to, sizeof(buffer_to), " %u long_name inconu", msg->packet.to);
						
					}
				}
				printf("channel: %u id: %u\n", msg->packet.channel, msg->packet.id);
				if (msg->packet.which_payload_variant == meshtastic_MeshPacket_decoded_tag) 
				{
				printf("message en clair\n");
						size_t text_size;
						text_size = sizeof(msg->packet.decoded.payload.bytes) +1;
					if (msg->packet.decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP) {
						if (msg->packet.decoded.payload.size <= (text_size -1)) 
						{
							
							char text[text_size];
							memcpy(text, msg->packet.decoded.payload.bytes, msg->packet.decoded.payload.size);
							text[msg->packet.decoded.payload.size] = '\0';
							printf("from: %s to: %s msg: %s\n", buffer_from, buffer_to, text);
						}
					}
			} 
			else 
			{
				printf("message chiffré from: %s to: %s\n",buffer_from, buffer_to);
			}
			}
			break;
		default:
			printf("autre message, tag=%d\n", msg->which_payload_variant);
			break;
		}
	} else {
		printf("echec\n");
	}
	fs->frame_ready = 0;
}

int to_radio_construct(char *to_str, char *message, meshtastic_ToRadio *out)
{
		size_t text_size;
		text_size = sizeof(out->packet.decoded.payload.bytes);

		out->which_payload_variant = meshtastic_ToRadio_packet_tag;
		out->packet.to = strtoul(to_str, NULL,10);
		out->packet.from = 0;
		out->packet.channel = 0;
		out->packet.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
		out->packet.decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;

		if(strlen( message) <= (text_size ))
		{
				
			memcpy(out->packet.decoded.payload.bytes,message, strlen(message));
			out->packet.decoded.payload.size = strlen(message);
				
		}
		else 
		{
				
			fprintf(stderr, "to_radio_construct : message is too long\n");
			return -1;
		}
		return 0;
}

int to_radio_encode(meshtastic_ToRadio *to_radio, uint8_t *out_buffer, size_t *out_len)
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

int
main(int argc, char *argv[])
{
	if (argc<2 || strcmp(argv[1], "-v")== 0)
	{
		option = READ_OPTION;
	}
	else if (strcmp(argv[1], "-s")== 0)
	{
		option = SEND_OPTION;
	}
	else 
	{
		option = USAGE_OPTION;
	}

	signal(SIGINT, handle_sigint);
	int		fd;
	unsigned char	buf[64];
	ssize_t		n;
	struct framing_state fs = {0};
	meshtastic_FromRadio msg = meshtastic_FromRadio_init_zero;
	mesh_state_t *state = mesh_state_init();
	char serial_path[64];
	unsigned char handshake[HANDSHAKE_LEN];
	int attemps = 0;
	unsigned char wake[32];

	if (state == NULL) {
		fprintf(stderr, "mesh_init failed\n");
		return 1;
	}
	/*
	 * Trame ToRadio minimale (protobuf) : 94 c3 -- octets magiques de
	 * début de trame (START1/START2) 00 02 -- longueur du payload qui
	 * suit, en big-endian (2 octets) 18 + valeur randomisée -- payload :
	 * champ want_config_id (numéro de champ 3, encodé comme tag=0x18)
	 * avec une valeur randomisée par run (voir framing_handshake_construct,
	 * nécessaire car le firmware garde en RAM les want_config_id déjà
	 * traités). Cette requête indique au firmware qu'un client attend le
	 * dump complet de sa configuration et de sa base de nodes connus.
	 */

	if (platform_serial_find_device(serial_path, sizeof(serial_path)) == 0) {
		fd = platform_serial_open(serial_path);
	} else {
		return -1;
	}
	if (fd == -1) {
		printf("echec de l'ouverture\n");
		return 1;
	}
	if (framing_handshake_construct(handshake, HANDSHAKE_LEN) != 0) {
		fprintf(stderr, "handshake construct failed\n");
		return 1;
	}

	memset(wake, 0xc3, sizeof(wake));
	platform_serial_write(fd, wake, sizeof(wake));
	usleep(100000);

	platform_serial_write(fd, handshake, sizeof(handshake));
	
	/*
 	* Phase de validation : on envoie d'abord 32 octets 0xc3 (technique du
 	* client officiel meshtastic-python) pour réveiller le device et
 	* resynchroniser sa machine à états de lecture, avant le vrai handshake.
 	* Sans ce réveil, le firmware ne renvoie pas le dump complet de sa
 	* config/nodeDB. fs (le même state que la boucle principale plus bas)
 	* est utilisé directement pour ne pas perdre le début du flux entre
 	* validation et lecture normale.
 	*/
	
		
	
	while (fs.frame_ready == 0 && attemps < 30) {
		n = platform_serial_read(fd, buf, sizeof(buf));
		if (n > 0) {
			framing_feed(&fs, buf, n);
		}
		attemps++;
	}
	if (fs.frame_ready) {
		printf("device valide\n");
		process_frame(&fs, &msg, state);
	} else {
		printf("pas de reponse valide\n");
	}

	/*
	 * Boucle de lecture infinie : chaque appel à read() peut renvoyer
	 * un nombre arbitraire d'octets, sans rapport avec les frontières
	 * des trames Meshtastic (une trame peut être coupée entre deux
	 * lectures, ou plusieurs trames peuvent arriver d'un coup).
	 */
	switch (option)
	{
		case READ_OPTION :
		while (running) {
			n = platform_serial_read(fd, buf, sizeof(buf));
			if (n > 0) {
				framing_feed(&fs, buf, n);
				if (fs.frame_ready) {
					process_frame(&fs, &msg, state);
				}
			} else if (n == -1) {
				perror("read");
				break;
			}
		}
		mesh_state_destroy(state);
		break;

		case SEND_OPTION:
		{
			meshtastic_ToRadio to_radio = {0};
			if (to_radio_construct(argv[2], argv[3], &to_radio) == -1)
				{
					return -1;
				}
			uint8_t encoded_buffer[FRAMING_MAX_PAYLOAD];
			size_t encoded_len;
				if (to_radio_encode(&to_radio, encoded_buffer, &encoded_len) == -1)
				{
					return -1;
				}
			unsigned char final_frame[FRAMING_MAX_PAYLOAD + 4];
			if (framing_message_construct(encoded_buffer, encoded_len, final_frame, sizeof(final_frame)) == -1)
				{
    				return -1;
				}
			platform_serial_write(fd, final_frame, encoded_len + 4);
			break;
		}

		case USAGE_OPTION:
		printf("usage: -v view data stream, -s send a message -s nodenum or long name message");
			break;
		default :
		break;
	}

		return 0;
}
