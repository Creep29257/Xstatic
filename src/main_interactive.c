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
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include "core/device_config.h"
#include "core/message_history.h"
#include "core/dispatch.h"
#include "ui/cli_display.h"

const char *VERSION = "0.2";

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

void
print_help(void)
{
	printf("Available commands:\n");
	printf("  list          - show known nodes\n");
	printf("  send          - send a message (node, then text)\n");
	printf("  show config   - show device config summary (role, lora)\n");
	printf("  show all config - show full device config dump\n");
	printf("  reply <id>    - reply to a received message by its [id]\n");
	printf("  history       - show received message history\n");
	printf("  channels list   - show channels\n");
	printf("  quit          - exit the program\n");
	printf("\n");
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
	}
	else
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
	int fd;
	char serial_path[64];
	unsigned char handshake[HANDSHAKE_LEN];
	unsigned char wake[32];
	unsigned char buf[64];
	ssize_t n;
	struct framing_state fs = {0};
	meshtastic_FromRadio msg = meshtastic_FromRadio_init_zero;
	mesh_state_t *state = mesh_state_init();
	device_config_t device_config;
	device_config_init(&device_config);
	int attemps = 0;
	int config_complete = 0;
	interactive_state_t state_send = IDLE;
	char nom_node[MESH_LONG_NAME_MAX];
	fd_set readfds;
	int max_fd;
	message_history_t history;
	channel_state_t cstate;

	message_history_init(&history);
	channel_state_init(&cstate);
	signal(SIGINT, handle_sigint);

	/* clean screen */
	printf("\033[2J\033[H");
	printf("Xstatic %s\n", VERSION);
	printf("Copyright (c) 2026 Remi Assailly - BSD 2-Clause License\n");
	printf("Uses Meshtastic protobufs (GPLv3) - see third_party/ for details\n");
	printf("\n");
	print_help();

	if (platform_serial_find_device(serial_path, sizeof(serial_path)) == 0)
	{
		fd = platform_serial_open(serial_path);
	}
	else
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

	max_fd = (fd < STDIN_FILENO ? STDIN_FILENO : fd);

	while (config_complete == 0 && attemps < 300)
	{
		n = platform_serial_read(fd, buf, sizeof(buf));
		if (n > 0)
		{
			framing_feed(&fs, buf, n);
			if (fs.frame_ready)
			{
				dispatch_process_frame(&fs, &msg, state, &device_config, &history, &cstate);
				printf(".");
				fflush(stdout);
				attemps = 0;
				if (msg.which_payload_variant == meshtastic_FromRadio_config_complete_id_tag)
				{
					config_complete = 1;
				}
			}
		}
		attemps++;
	}
	printf("\n");

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
			if (errno == EINTR)
				{
					continue;
				}
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
					dispatch_process_frame(&fs, &msg, state, &device_config, &history, &cstate);
				}
			}
		}

		if (FD_ISSET(STDIN_FILENO, &readfds))
		{
			char input[64];

			if (fgets(input, sizeof(input), stdin) != NULL)
			{
				input[strcspn(input, "\n")] = '\0';

				interactive_state_t prev_state = state_send;
				bool known_command = false;

				if (strcmp(input, "quit") == 0)
				{
					printf("\033[31mexiting Xstatic\033[0m\n");
					known_command = true;
					running = 0;
				}
				
				if (strcmp(input, "channels list") == 0)
				{
					cli_display_channel_list(&cstate, &device_config);
					known_command=true;
				}

				if (strcmp(input, "list") == 0)
				{
					cli_display_node_list(state);
					known_command = true;
				}

				if (strcmp(input, "help") == 0)
				{
					print_help();
					known_command = true;
				}

				if (strcmp(input, "show config") == 0)
				{
					cli_display_device_config(&device_config, false);
					known_command = true;
				}

				if (strcmp(input, "show all config") == 0)
				{
					cli_display_device_config(&device_config, true);
					known_command = true;
				}

				if (strcmp(input, "history") == 0)
				{
					cli_display_message_history(&history, &cstate, &device_config);
					known_command = true;
				}

				if (strcmp(input, "send") == 0)
				{
					printf("Send message to (node number or long name): ");
					fflush(stdout);
					known_command = true;
					state_send = AWAITING_NODE;
				}
				else if (strncmp(input, "reply ", 6) == 0)
				{
					char *endptr;
					uint32_t reply_id = (uint32_t)strtoul((input + 6), &endptr, 10);
					message_entry_t *found_msg = message_history_find_repliable_by_id(&history, reply_id);

					if (found_msg == NULL)
					{
						printf("message id not found\n");
					}
					else
					{
						snprintf(nom_node, MESH_LONG_NAME_MAX, "%u", found_msg->num);
						printf("message text: ");
						fflush(stdout);
						state_send = AWAITING_MESSAGE;
					}
					known_command = true;
				}
				else if (state_send == AWAITING_NODE)
				{
					mesh_node_t *found = mesh_state_find_node_by_name(state, input);

					if (found != NULL)
					{
						/* trouvé par nom -- convertir found->num en texte dans nom_node */
						snprintf(nom_node, MESH_LONG_NAME_MAX, "%u", found->num);
						printf("message text: ");
						fflush(stdout);
						state_send = AWAITING_MESSAGE;
					}
					else
					{
						char *endptr;
						strtoul(input, &endptr, 10);

						if (input[0] != '\0' && *endptr == '\0')
						{
							/* input est entierement numerique -- accepter tel quel */
							strncpy(nom_node, input, MESH_LONG_NAME_MAX - 1);
							nom_node[MESH_LONG_NAME_MAX - 1] = '\0';
							printf("message text: ");
							fflush(stdout);
							state_send = AWAITING_MESSAGE;
						}
						else
						{
							printf("unknown long name, see list\n");
							state_send = IDLE;
						}
					}
				}
				else if (state_send == AWAITING_MESSAGE)
				{
					meshtastic_ToRadio to_radio = {0};

					if (to_radio_construct(nom_node, input, &to_radio) == -1)
					{
						state_send = IDLE;
					}
					else
					{
						uint8_t encoded_buffer[FRAMING_MAX_PAYLOAD];
						size_t encoded_len;

						if (to_radio_encode(&to_radio, encoded_buffer, &encoded_len) == -1)
						{
							state_send = IDLE;
						}
						else
						{
							unsigned char final_frame[FRAMING_MAX_PAYLOAD + 4];

							if (framing_message_construct(encoded_buffer, encoded_len, final_frame, sizeof(final_frame)) == -1)
							{
								state_send = IDLE;
							}
							else
							{
								platform_serial_write(fd, final_frame, encoded_len + 4);
								printf("\033[7;32m message sent to: %s -> %s \033[0m\n", nom_node, input);

								platform_serial_close(fd);
								fd = platform_serial_open(serial_path);
								state_send = IDLE;
							}
						}
					}
				}
				if (!known_command && prev_state == IDLE)
				{
					printf("unknown command, see 'help'\n");
				}
			}
		}
	}

	mesh_state_destroy(state);
	platform_serial_close(fd);
	return 0;
}