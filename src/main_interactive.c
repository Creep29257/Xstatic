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
 * main_interactive.c -- mode interactif (send + read + list + quit dans
 * une seule session), via select() sur le fd série et stdin.
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
#include <sys/select.h>

const char *VERSION ="0.2"

typedef enum
{
	IDLE,
	AWAITING_NODE,
	AWAITING_MESSAGE
} interactive_state_t;

volatile sig_atomic_t running = 1;

void
handle_sigint(int sig)
{
	(void)sig;
	running = 0;
}

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

int
main(void)
{
	signal(SIGINT, handle_sigint);

	int fd;
	char serial_path[64];
	unsigned char handshake[HANDSHAKE_LEN];
	unsigned char wake[32];
	unsigned char buf[64];
	ssize_t n;
	struct framing_state fs = {0};
	meshtastic_FromRadio msg = meshtastic_FromRadio_init_zero;
	mesh_state_t *state = mesh_state_init();
	int attemps = 0;
	int config_complete = 0;
	interactive_state_t state_send = IDLE;
	char nom_node[MESH_LONG_NAME_MAX];
	
	//clean screen
	printf("\033[2J\033[H");
	printf("Xstasic %s \n", VERSION);
	printf("Copyright (c) 2026 Remi Assailly - BSD 2-Clause License\n");
	printf("Uses Meshtastic protobufs (GPLv3) - see third_party/ for details\n");
	printf("\n");
	printf("Available commands:\n");
	printf("  list          - show known nodes\n");
	printf("  send          - send a message (node, then text)\n");
	printf("  quit          - exit the program\n");
	rintf("\n");
	

	if (platform_serial_find_device(serial_path, sizeof(serial_path)) == 0)
	{
		fd = platform_serial_open(serial_path);
	} else
	{
		return -1;
	}
	if (fd == -1)
	{
		fprintf(stderr, "cant open device\n");
		return -1;
	}
	if (framing_handshake_construct(handshake, HANDSHAKE_LEN) != 0)
	{
		fprintf(stderr, "handshake construct failed\n");
		return 1;
	}

	memset(wake, 0xc3, sizeof(wake));
	platform_serial_write(fd, wake, sizeof(wake));
	usleep(100000);
	platform_serial_write(fd, handshake, sizeof(handshake));

	fd_set readfds;
	int max_fd = (fd < STDIN_FILENO ? STDIN_FILENO : fd);

	while (config_complete == 0 && attemps < 300)
	{
		n = platform_serial_read(fd, buf, sizeof(buf));
		if (n > 0)
		{
			framing_feed(&fs, buf, n);
			if (fs.frame_ready)
			{
				process_frame(&fs, &msg, state);
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

	while (running)
	{
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		FD_SET(STDIN_FILENO, &readfds);
		int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
		if (ready < 0)
		{
			perror("select");
			break;
		}

		if (FD_ISSET(fd, &readfds))
		{
			n = platform_serial_read(fd, buf, sizeof(buf));
			if (n > 0)
			{
				framing_feed(&fs, buf, n);
				if (fs.frame_ready)
				{
					process_frame(&fs, &msg, state);
				}
			}
		}

		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			char input[64];
			if (fgets(input, sizeof(input), stdin) != NULL)
			{
				input[strcspn(input, "\n")] = '\0';

				if (strcmp(input, "quit") == 0)
				{
					running = 0;
				}

				if (strcmp(input, "list") == 0)
				{
					mesh_node_t *node_cursor = mesh_state_first_node(state);
					int pos_node = 1;
					while (node_cursor != NULL)
					{
						printf("\033[32mNode %i    num =%u , node long_name = %s , node hw_model= %u \033[0m\n",
						       pos_node, node_cursor->num, node_cursor->long_name, node_cursor->hw_model);
						if (node_cursor->position.valid == 1)
						{
							double lat = node_cursor->position.latitude_i / 10000000.0;
							double lon = node_cursor->position.longitude_i / 10000000.0;
							printf("\033[32m latitude %f longitude %f \033[0m\n", lat, lon);
						}
						pos_node++;
						node_cursor = mesh_state_next_node(node_cursor);
					}
				}

				if (strcmp(input, "send") == 0)
				{
					printf("Send message to (node number or long name): ");
					fflush(stdout);
					state_send = AWAITING_NODE;
				} else if (state_send == AWAITING_NODE)
				{
					strncpy(nom_node, input, MESH_LONG_NAME_MAX - 1);
					nom_node[MESH_LONG_NAME_MAX - 1] = '\0';
					printf("message text: ");
					fflush(stdout);
					state_send = AWAITING_MESSAGE;
				} else if (state_send == AWAITING_MESSAGE)
				{
					meshtastic_ToRadio to_radio = {0};
					if (to_radio_construct(nom_node, input, &to_radio) == -1)
					{
						state_send = IDLE;
					} else
					{
						uint8_t encoded_buffer[FRAMING_MAX_PAYLOAD];
						size_t encoded_len;
						if (to_radio_encode(&to_radio, encoded_buffer, &encoded_len) == -1)
						{
							state_send = IDLE;
						} else
						{
							unsigned char final_frame[FRAMING_MAX_PAYLOAD + 4];
							if (framing_message_construct(encoded_buffer, encoded_len, final_frame, sizeof(final_frame)) == -1)
							{
								state_send = IDLE;
							} else
							{
								platform_serial_write(fd, final_frame, encoded_len + 4);
								printf("\033[32m======================================================================\n");
								printf("msg envoye a: %s  %s\n", nom_node, input);
								printf("======================================================================\033[0m\n");

								platform_serial_close(fd);
								fd = platform_serial_open(serial_path);
								state_send = IDLE;
							}
						}
					}
				}
			}
		}
	}

	mesh_state_destroy(state);
	platform_serial_close(fd);
	return 0;
}