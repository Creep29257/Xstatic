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

/* Taille du buffer utilisé pour résoudre from/to en long_name lisible.
 * Dérivée du champ long_name de mesh_node_t  */
#define NAME_BUF_SIZE sizeof(((mesh_node_t *)0)->long_name)

typedef enum
{
	READ_OPTION,
	SEND_OPTION,
	LIST_OPTION,
	USAGE_OPTION
} option_mode_t;

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

int main(void)
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

if (platform_serial_find_device(serial_path, sizeof(serial_path)) == 0)
    {
       fd= platform_serial_open(serial_path);
    }    
else 
    {
        return -1;
    }
    if (fd == -1)
    {
        fprintf(stderr, " cant open device");
        return -1;

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
	int ready = select((max_fd +1), &readfds, NULL, NULL, NULL);
	if (ready <0)
	{
		perror("select");
		break;
	}
	if (FD_ISSET(fd, &readfds))
	{
		n = platform_serial_read(fd, buf, sizeof(buf));
		if(n>0)
		{
			framing_feed(&fs, buf, n);
			if (fs.frame_ready)
			{
				process_frame(&fs,&msg, state);
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
				printf("\033[32mNode %i    num =%u , node long_name = %s , node hw_model= %u \n\033[0m",
				       pos_node, node_cursor->num, node_cursor->long_name, node_cursor->hw_model);
				pos_node++;
				node_cursor = mesh_state_next_node(node_cursor);
			}
			
			}
			
		}
	}

}
	mesh_state_destroy(state);
	platform_serial_close(fd);
	return 0;
}