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
 * main.c -- point d'entrée v0.1 : lecture (-v essentiel / -vv complet),
 * envoi (-s), liste des nodes connus (-l). Structure définitive à venir
 * (fusion select() avec ui_xlib, cf design.md) -- ce fichier reste
 * volontairement simple pour cette version : un seul mode actif par
 * exécution, pas d'interactivité.
 */
#include "platform/platform.h"
#include "protocol/framing.h"
#include "core/mesh_state.h"
#include "protocol/generated/meshtastic/mesh.pb.h"
#include "third_party/nanopb/pb_encode.h"
#include "third_party/nanopb/pb_decode.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* Taille du buffer utilisé pour résoudre from/to en long_name lisible.
 * Dérivée du champ long_name de mesh_node_t plutôt que codée en dur, pour
 * ne pas dépendre d'une taille magique dupliquée à deux endroits. */
#define NAME_BUF_SIZE sizeof(((mesh_node_t *)0)->long_name)

typedef enum
{
	READ_OPTION,
	SEND_OPTION,
	LIST_OPTION,
	USAGE_OPTION
} option_mode_t;

option_mode_t option;

/* verbose_level distingue -v (0 : essentiel, nodes + messages) de -vv
 * (1 : tout, y compris config/moduleConfig/channel/etc.). N'a d'effet
 * que pour READ_OPTION -- SEND_OPTION et LIST_OPTION restent toujours
 * silencieux pendant la phase de validation, peu importe ce niveau. */
int verbose_level;

/* running passe à 0 sur SIGINT (Ctrl+C), lu par la boucle de READ_OPTION
 * pour sortir proprement (fermeture du port, destruction de mesh_state)
 * plutôt que de couper le process brutalement. */
volatile sig_atomic_t running = 1;

void
handle_sigint(int sig)
{
	(void)sig;
	running = 0;
}

/*
 * process_frame -- décode une trame FromRadio complète et met à jour
 * mesh_state en conséquence. Ne fait AUCUN affichage : c'est le rôle de
 * display_frame(), appelée séparément par l'appelant si besoin. Cette
 * séparation permet à LIST_OPTION et SEND_OPTION de peupler mesh_state
 * silencieusement pendant la phase de validation, sans dépendre d'un
 * mode "verbose" qui aurait aussi coupé le traitement.
 */
static void
process_frame(struct framing_state *fs, meshtastic_FromRadio *msg, mesh_state_t *state)
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
	}
	fs->frame_ready = 0;
}

/*
 * display_frame -- affiche le contenu d'une trame déjà décodée par
 * process_frame(). Deux niveaux : node_info_tag et packet_tag (nodes
 * connus, messages texte) s'affichent dès verbose>=0 -- c'est
 * l'essentiel, ce que montre -v. Tous les autres tags ne s'affichent
 * qu'en verbose==1, c'est-à-dire -vv.
 */
static void
display_frame(meshtastic_FromRadio *msg, mesh_state_t *state, int verbose)
{
	switch (msg->which_payload_variant)
	{
	case meshtastic_FromRadio_my_info_tag:
		if (verbose == 1)
		{
			printf("MyNodeInfo recu, mon node = %u\n", msg->my_info.my_node_num);
		}
		break;

	case meshtastic_FromRadio_node_info_tag:
		if(verbose == 1)
		{
		printf("\033[32mNode bien créé / mis a jour, node num =%u , node long_name = %s , node hw_model= %u \n\033[0m",
		       msg->node_info.num, msg->node_info.user.long_name, msg->node_info.user.hw_model);
		}
		break;

	case meshtastic_FromRadio_config_complete_id_tag:
		if (verbose == 1)
		{
			printf("ConfigComplete id recu %u\n", msg->config_complete_id);
		}
		break;

	case meshtastic_FromRadio_rebooted_tag:
		if (verbose == 1 && msg->rebooted)
		{
			printf("device vient d'etre rebooter \n");
		}
		break;

	case meshtastic_FromRadio_queueStatus_tag:
		if (verbose == 1)
		{
			printf("QueueStatus recu : res=%d free=%u maxlen=%u mesh_packet_id=%u\n",
			       msg->queueStatus.res, msg->queueStatus.free, msg->queueStatus.maxlen, msg->queueStatus.mesh_packet_id);
		}
		break;

	case meshtastic_FromRadio_fileInfo_tag:
		if (verbose == 1)
		{
			printf("nom du fichier: %s, taille %u \n", msg->fileInfo.file_name, msg->fileInfo.size_bytes);
		}
		break;

	case meshtastic_FromRadio_xmodemPacket_tag:
		if (verbose == 1)
		{
			printf("control: %d, seq: %u, crc16: %u \n", msg->xmodemPacket.control, msg->xmodemPacket.seq, msg->xmodemPacket.crc16);
		}
		break;

	case meshtastic_FromRadio_log_record_tag:
		if (verbose == 1)
		{
			printf("message: %s, time: %u, source: %s, level %d \n", msg->log_record.message, msg->log_record.time, msg->log_record.source, msg->log_record.level);
		}
		break;

	case meshtastic_FromRadio_channel_tag:
		if (verbose == 1)
		{
			printf("Channel recu : index=%d has_settings=%d role=%d\n", msg->channel.index, msg->channel.has_settings, msg->channel.role);
		}
		break;

	case meshtastic_FromRadio_lockdown_status_tag:
		if (verbose == 1)
		{
			printf("LockdownStatus recu : state=%d lock_reason=%s\n", msg->lockdown_status.state, msg->lockdown_status.lock_reason);
		}
		break;

	case meshtastic_FromRadio_deviceuiConfig_tag:
		if (verbose == 1)
		{
			printf("DeviceUIConfig recu : version=%u brightness=%u timeout=%u theme=%d\n", msg->deviceuiConfig.version, msg->deviceuiConfig.screen_brightness, msg->deviceuiConfig.screen_timeout, msg->deviceuiConfig.theme);
		}
		break;

	case meshtastic_FromRadio_metadata_tag:
		if (verbose == 1)
		{
			printf("DeviceMetadata recu : firmware=%s hw_model=%d wifi=%d bluetooth=%d ethernet=%d\n", msg->metadata.firmware_version, msg->metadata.hw_model, msg->metadata.hasWifi, msg->metadata.hasBluetooth, msg->metadata.hasEthernet);
		}
		break;

	case meshtastic_FromRadio_region_presets_tag:
		if (verbose == 1)
		{
			printf("LoRaRegionPresetMap recu : groups_count=%u region_groups_count=%u\n", msg->region_presets.groups_count, msg->region_presets.region_groups_count);
		}
		break;

	case meshtastic_FromRadio_mqttClientProxyMessage_tag:
		if (verbose == 1)
		{
			if (msg->mqttClientProxyMessage.which_payload_variant == meshtastic_MqttClientProxyMessage_text_tag)
			{
				printf("MqttClientProxyMessage (text) : topic=%s text=%s\n",
				       msg->mqttClientProxyMessage.topic, msg->mqttClientProxyMessage.payload_variant.text);
			} else
			{
				printf("payload variant binaire\n");
			}
		}
		break;

	case meshtastic_FromRadio_clientNotification_tag:
		if (verbose == 1)
		{
			printf("client notification tag message: %s\n", msg->clientNotification.message);
		}
		break;

	case meshtastic_FromRadio_config_tag:
		if (verbose == 1)
		{
			switch (msg->config.which_payload_variant)
			{
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
		}
		break;

	case meshtastic_FromRadio_moduleConfig_tag:
		if (verbose == 1)
		{
			switch (msg->moduleConfig.which_payload_variant)
			{
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
		}
		break;

	case meshtastic_FromRadio_packet_tag:
		{
			mesh_node_t *from_node = mesh_state_find_node(state, msg->packet.from);
			char buffer_from[NAME_BUF_SIZE];
			char buffer_to[NAME_BUF_SIZE];

			if (from_node != NULL)
			{
				snprintf(buffer_from, sizeof(buffer_from), "%s", from_node->long_name);
			} else
			{
				snprintf(buffer_from, sizeof(buffer_from), "inconnu: %u", msg->packet.from);
			}

			if (msg->packet.to == 4294967295)
			{
				snprintf(buffer_to, sizeof(buffer_to), "Broadcast");
			} else
			{
				mesh_node_t *to_node = mesh_state_find_node(state, msg->packet.to);
				if (to_node != NULL)
				{
					snprintf(buffer_to, sizeof(buffer_to), "%s", to_node->long_name);
				} else
				{
					snprintf(buffer_to, sizeof(buffer_to), " %u long_name inconu", msg->packet.to);
				}
			}

			if (verbose == 1)
			{
				printf("channel: %u id: %u\n", msg->packet.channel, msg->packet.id);
			}

			if (msg->packet.which_payload_variant == meshtastic_MeshPacket_decoded_tag)
			{
				/* Data.payload est un PB_BYTES_ARRAY_T (size + bytes[233]),
				 * PAS null-terminé -- copie défensive dans un buffer local
				 * avec garde sur la taille avant d'ajouter le '\0' manuel. */
				size_t text_size = sizeof(msg->packet.decoded.payload.bytes) + 1;
				if (msg->packet.decoded.portnum == meshtastic_PortNum_TEXT_MESSAGE_APP)
				{
					if (msg->packet.decoded.payload.size <= (text_size - 1))
					{
						char text[text_size];
						memcpy(text, msg->packet.decoded.payload.bytes, msg->packet.decoded.payload.size);
						text[msg->packet.decoded.payload.size] = '\0';
						printf("\033[32m===============================================================\n");
						printf("from: %s to: %s msg: %s\n", buffer_from, buffer_to, text);
						printf("===============================================================\033[0m \n");
					}
				}
			} else if (verbose == 1)
			{
				printf("message chiffré from: %s to: %s\n", buffer_from, buffer_to);
			}
		}
		break;

	default:
		if (verbose == 1)
		{
			printf("autre message, tag=%d\n", msg->which_payload_variant);
		}
		break;
	}
}

/*
 * to_radio_construct -- remplit un meshtastic_ToRadio pour l'envoi d'un
 * message texte. Paramètre de sortie par pointeur (out), retour int
 * (0 = succès, -1 = message trop long). to_str est converti en uint32_t
 * via strtoul (résolution par num uniquement -- pas par long_name, ce qui
 * demanderait une lecture préalable du mesh non faite ici).
 */
int
to_radio_construct(char *to_str, char *message, meshtastic_ToRadio *out)
{
	size_t text_size = sizeof(out->packet.decoded.payload.bytes);

	out->which_payload_variant = meshtastic_ToRadio_packet_tag;
	out->packet.to = strtoul(to_str, NULL, 10);
	out->packet.from = 0;
	out->packet.channel = 0;
	out->packet.which_payload_variant = meshtastic_MeshPacket_decoded_tag;
	out->packet.decoded.portnum = meshtastic_PortNum_TEXT_MESSAGE_APP;
	out->packet.want_ack = true;
	out->packet.priority = meshtastic_MeshPacket_Priority_RELIABLE;

	if (strlen(message) <= text_size)
	{
		memcpy(out->packet.decoded.payload.bytes, message, strlen(message));
		out->packet.decoded.payload.size = strlen(message);
	} else
	{
		fprintf(stderr, "to_radio_construct : message is too long\n");
		return -1;
	}
	return 0;
}

/*
 * to_radio_encode -- encode un meshtastic_ToRadio en protobuf brut dans
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

int
main(int argc, char *argv[])
{
	if (argc < 2 || strcmp(argv[1], "-v") == 0)
	{
		option = READ_OPTION;
		verbose_level = 0;
	} else if (strcmp(argv[1], "-vv") == 0)
	{
		option = READ_OPTION;
		verbose_level = 1;
	} else if (strcmp(argv[1], "-s") == 0)
	{
		option = SEND_OPTION;
		verbose_level = 0;
	} else if (strcmp(argv[1], "-l") == 0)
	{
		option = LIST_OPTION;
		verbose_level = 0;
	} else
	{
		option = USAGE_OPTION;
		verbose_level = 0;
	}

	if (option == USAGE_OPTION)
	{
		printf("usage: -v view essential, -vv view all, -s <nodenum> <message> send a message, -l list known nodes\n");
		return 0;
	}

	signal(SIGINT, handle_sigint);

	int fd;
	unsigned char buf[64];
	ssize_t n;
	struct framing_state fs = {0};
	meshtastic_FromRadio msg = meshtastic_FromRadio_init_zero;
	mesh_state_t *state = mesh_state_init();
	char serial_path[64];
	unsigned char handshake[HANDSHAKE_LEN];
	unsigned char wake[32];
	int attemps = 0;
	int config_complete = 0;

	if (state == NULL)
	{
		fprintf(stderr, "mesh_init failed\n");
		return 1;
	}

	if (platform_serial_find_device(serial_path, sizeof(serial_path)) == 0)
	{
		fd = platform_serial_open(serial_path);
	} else
	{
		return -1;
	}
	if (fd == -1)
	{
		printf("echec de l'ouverture\n");
		return 1;
	}
	if (framing_handshake_construct(handshake, HANDSHAKE_LEN) != 0)
	{
		fprintf(stderr, "handshake construct failed\n");
		return 1;
	}

	/*
	 * Wake-up : 32 octets 0xc3 + pause 100ms avant le handshake. Technique
	 * du client officiel meshtastic-python, nécessaire pour obtenir un
	 * dump complet du firmware (résout un état ambigu de sa machine à
	 * états de lecture série, sans quoi le dump peut être tronqué).
	 */
	memset(wake, 0xc3, sizeof(wake));
	platform_serial_write(fd, wake, sizeof(wake));
	usleep(100000);
	platform_serial_write(fd, handshake, sizeof(handshake));

	/*
	 * Phase de validation : attend le dump complet du firmware jusqu'à
	 * config_complete_id (pas juste la première trame) -- nécessaire pour
	 * que mesh_state soit peuplé avant SEND_OPTION/LIST_OPTION. attemps
	 * est réinitialisé à chaque trame reçue plutôt que d'être un compteur
	 * global, pour ne pas timeout prématurément sur un gros dump tout en
	 * détectant un vrai silence du device. Silencieuse sauf en READ_OPTION
	 * (display_frame reçoit 0 pour SEND_OPTION/LIST_OPTION).
	 */
	while (config_complete == 0 && attemps < 300)
	{
		n = platform_serial_read(fd, buf, sizeof(buf));
		if (n > 0)
		{
			framing_feed(&fs, buf, n);
			if (fs.frame_ready)
			{
				process_frame(&fs, &msg, state);
				display_frame(&msg, state, option == READ_OPTION ? verbose_level : 0);
				attemps = 0;
				if (msg.which_payload_variant == meshtastic_FromRadio_config_complete_id_tag)
				{
					config_complete = 1;
				}
			}
		}
		attemps++;
	}
	if (config_complete == 0)
	{
		printf("pas de reponse valide\n");
		return -1;
	}
	printf("device valide\n");

	switch (option)
	{
	case READ_OPTION:
		/*
		 * Boucle de lecture infinie : chaque appel à read() peut renvoyer
		 * un nombre arbitraire d'octets, sans rapport avec les frontières
		 * des trames Meshtastic (une trame peut être coupée entre deux
		 * lectures, ou plusieurs trames peuvent arriver d'un coup).
		 */
		while (running)
		{
			n = platform_serial_read(fd, buf, sizeof(buf));
			if (n > 0)
			{
				framing_feed(&fs, buf, n);
				if (fs.frame_ready)
				{
					process_frame(&fs, &msg, state);
					display_frame(&msg, state, verbose_level);
				}
			} else if (n == -1)
			{
				perror("read");
				break;
			}
		}
		mesh_state_destroy(state);
		platform_serial_close(fd);
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
			printf("\033[32m======================================================================\n");
			printf("msg envoyé a: %s contenu %s\n", argv[2], argv[3]);
			printf("======================================================================\033[0m\n");

			/*
			 * Le firmware RAK4631 ne transmet réellement le ToRadio écrit
			 * qu'après une réouverture du port série -- comportement
			 * confirmé sur device réel (25/08), pas un bug du client.
			 * Pas besoin de renvoyer wake-up/handshake sur cette réouverture.
			 */
			platform_serial_close(fd);
			fd = platform_serial_open(serial_path);
			platform_serial_close(fd);
			break;
		}

	case LIST_OPTION:
		{
			/*
			 * mesh_state est déjà peuplé par la phase de validation
			 * ci-dessus (dump initial du firmware, jusqu'à
			 * config_complete_id) -- pas de lecture supplémentaire ici,
			 * juste un parcours de la liste chaînée via l'API
			 * first_node/next_node (mesh_state reste opaque, pas d'accès
			 * direct à ->head depuis main.c).
			 */
			mesh_node_t *node_cursor = mesh_state_first_node(state);
			int pos_node = 1;
			while (node_cursor != NULL)
			{
				printf("\033[32mNode %i    num =%u , node long_name = %s , node hw_model= %u \n\033[0m",
				       pos_node, node_cursor->num, node_cursor->long_name, node_cursor->hw_model);
				pos_node++;
				node_cursor = mesh_state_next_node(node_cursor);
			}
			mesh_state_destroy(state);
			platform_serial_close(fd);
			break;
		}

	case USAGE_OPTION:
		printf("usage: -v view essential, -vv view all, -s <nodenum> <message> send a message, -l list known nodes\n");
		break;

	default:
		break;
	}

	return 0;
}
