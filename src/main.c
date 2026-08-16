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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * main.c -- point d'entrée, boucle select() fusionnant fd série et fd X11.
 */
#include "platform/platform.h"
#include "protocol/framing.h"
#include <stdio.h>

/*
 * Programme de test manuel (temporaire, pas la structure finale du
 * client) : ouvre le port série du RAK4631, envoie une requête
 * ToRadio.want_config_id pour faire basculer le firmware du mode
 * "logs texte" au mode "API protobuf", puis affiche en boucle tout
 * ce qui est reçu, en hexadécimal.
 *
 * Le prochain module (protocol/framing.c) remplacera cette boucle
 * naïve par un vrai découpage des trames 0x94 0xc3 [longueur] [payload].
 */
int
main(void)
{
	int fd;
	unsigned char buf[64];
	ssize_t n;
	struct framing_state fs = {0}; 

	/* Trame ToRadio minimale, construite à la main (protobuf) :
	 *   94 c3        -- octets magiques de début de trame (START1/START2)
	 *   00 02        -- longueur du payload qui suit, en big-endian (2 octets)
	 *   18 01        -- payload : champ want_config_id (numéro de champ 3,
	 *                   encodé comme tag=0x18) avec la valeur 1
	 * Cette requête indique au firmware qu'un client attend le dump
	 * complet de sa configuration et de sa base de nodes connus. */
	unsigned char handshake[] = {0x94, 0xc3, 0x00, 0x02, 0x18, 0x01};

	fd = platform_serial_open("/dev/cuaU0");
	if (fd == -1) {
		printf("echec de l'ouverture\n");
		return 1;
	}

	platform_serial_write(fd, handshake, sizeof(handshake));

	/* Boucle de lecture infinie : chaque appel à read() peut renvoyer
	 * un nombre arbitraire d'octets, sans rapport avec les frontières
	 * des trames Meshtastic (une trame peut être coupée entre deux
	 * lectures, ou plusieurs trames peuvent arriver d'un coup). */
for (;;) {
    n = platform_serial_read(fd, buf, sizeof(buf));
    printf("n=%zd\n", n);
    for (ssize_t j = 0; j < n; j++) {
    printf("%02x ", buf[j]);
}
printf("\n");
	fflush(stdout);
    if (n > 0) {
       
        framing_feed(&fs, buf, n);
        if (fs.frame_ready) {
            printf("trame recue, %u octets: ", fs.payload_pos);
            for (uint16_t i = 0; i < fs.payload_pos; i++) {
                printf("%02x ", fs.payload[i]);
            }
            printf("\n");
            fflush(stdout);
            fs.frame_ready = 0;  
        }
    } else if (n == -1) {
        perror("read");
        break;
    }
}

	return 0;
}
