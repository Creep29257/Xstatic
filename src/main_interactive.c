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

int main(void)
{
	signal(SIGINT, handle_sigint);
    int fd;
    char serial_path[64];
    unsigned char handshake[HANDSHAKE_LEN];
    unsigned char wake[32];
	unsigned char buf[64];
	ssize_t n;

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
	}
if (FD_ISSET(STDIN_FILENO, &readfds))
	{
	char input[64];
	if (fgets(input, sizeof(input), stdin) != NULL)	
		{
			printf("tu as tape: %s", input);
		}
	}
}

	return 0;
}