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
#include "protocol/generated/meshtastic/mesh.pb.h"
#include "third_party/nanopb/pb_decode.h"
#include <stdio.h>

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
 
int
main(void)
{
	int		fd;
	unsigned char	buf[64];
	ssize_t		n;
	struct framing_state fs = {0};
	meshtastic_FromRadio msg = meshtastic_FromRadio_init_zero;
	mesh_state_t *state = mesh_state_init();
	if (state == NULL) {
		fprintf(stderr, "mesh_init failed\n");
		return 1;
	}
	/*
	 * Trame ToRadio minimale, construite à la main (protobuf) : 94 c3
	 * -- octets magiques de début de trame (START1/START2) 00 02 --
	 * longueur du payload qui suit, en big-endian (2 octets) 18 01 --
	 * payload : champ want_config_id (numéro de champ 3, encodé comme
	 * tag=0x18) avec la valeur 1 Cette requête indique au firmware
	 * qu'un client attend le dump complet de sa configuration et de sa
	 * base de nodes connus.
	 */
	unsigned char	handshake[] = {0x94, 0xc3, 0x00, 0x02, 0x18, 0x01};
	fd = platform_serial_open("/dev/cuaU0"); //seter a la main pour le moment
	if (fd == -1) {
		printf("echec de l'ouverture\n");
		return 1;
	}
	platform_serial_write(fd, handshake, sizeof(handshake));
	/*
	 * Boucle de lecture infinie : chaque appel à read() peut renvoyer
	 * un nombre arbitraire d'octets, sans rapport avec les frontières
	 * des trames Meshtastic (une trame peut être coupée entre deux
	 * lectures, ou plusieurs trames peuvent arriver d'un coup).
	 */
	for (;;) {
		n = platform_serial_read(fd, buf, sizeof(buf));
		if (n > 0) {
			framing_feed(&fs, buf, n);
			if (fs.frame_ready) {
				printf("frame_ready=1, payload_pos=%u\n", fs.payload_pos);
				pb_istream_t	stream = pb_istream_from_buffer(fs.payload, fs.payload_pos);
				if (pb_decode(&stream, meshtastic_FromRadio_fields, &msg)) {
					switch (msg.which_payload_variant) {
					case meshtastic_FromRadio_my_info_tag:
						printf("MyNodeInfo recu, mon node = %u\n", msg.my_info.my_node_num);
						break;
					case meshtastic_FromRadio_node_info_tag:
						{
							mesh_node_info_t info;
							info.num = msg.node_info.num;
							strncpy(info.long_name, msg.node_info.user.long_name, MESH_LONG_NAME_MAX);
							info.long_name[MESH_LONG_NAME_MAX - 1] = '\0';
							info.hw_model = msg.node_info.user.hw_model;
							info.position.valid = msg.node_info.has_position;
							info.position.latitude_i = msg.node_info.position.latitude_i;
							info.position.longitude_i = msg.node_info.position.longitude_i;
							info.position.altitude = msg.node_info.position.altitude;
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
						printf("ConfigComplete id recu %u\n", msg.config_complete_id);
						break;
					default:
						printf("autre message, tag=%d\n", msg.which_payload_variant);
						break;
					}
				} else {
					printf("echec\n");
				}
				fs.frame_ready = 0;
			}
		} else if (n == -1) {
			perror("read");
			break;
		}
	}
	mesh_state_destroy(state);
	return 0;
}