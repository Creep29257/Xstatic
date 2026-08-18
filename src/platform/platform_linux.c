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
 * platform_linux.c -- implémentation Linux (termios, /dev/ttyACM*) de platform.h.
 */

#include "platform.h"
/*
 * Copyright (c) 2026 Rémi Assailly
 * remi@assailly.COM
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
 * platform_linux.c -- implémentation linux (termios, /dev/ttyACM*, cdc_acm) de platform.h.
 */
#include "platform.h"

#include <fcntl.h>      /* open(), O_RDWR, O_NOCTTY */
#include <termios.h>    /* struct termios, cfmakeraw, tcgetattr, tcsetattr, B115200 */
#include <unistd.h>     /* close(), read(), write() */
#include <stdio.h>      /* perror() */

/*
 * Ouvre et configure le port série en mode "brut" (raw mode) :
 * aucune interprétation des octets par le noyau (pas de traduction
 * de caractères, pas de gestion de signaux comme Ctrl+C), pour que
 * chaque octet reçu/envoyé corresponde exactement au flux binaire
 * du protocole Meshtastic.
 */
int
platform_serial_open(const char *device)
{
	int fd;
	struct termios tty;

	/* Ouverture du device. O_NOCTTY évite que ce port devienne le
	 * terminal de contrôle de notre process (comportement hérité de
	 * l'époque des vrais terminaux physiques, à éviter ici). */
	fd = open(device, O_RDWR | O_NOCTTY);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	/* On récupère la config actuelle du port avant de la modifier :
	 * cfmakeraw() ne touche que certains champs, il faut donc partir
	 * d'une config existante cohérente plutôt que d'une struct vide. */
	if (tcgetattr(fd, &tty) == -1) {
		perror("tcgetattr");
		close(fd);
		return -1;
	}

	/* Bascule tty en mode brut : désactive le mode canonique, l'écho,
	 * l'interprétation des caractères de contrôle, etc. Modifie tty
	 * uniquement en mémoire, rien n'est encore appliqué au device. */
	cfmakeraw(&tty);

	/* Fixe la vitesse de la liaison série à 115200 bauds, celle
	 * qu'utilise le firmware Meshtastic. Toujours en mémoire pour
	 * l'instant. */
	if (cfsetspeed(&tty, B115200) == -1) {
		perror("cfsetspeed");
		close(fd);
		return -1;
	}

	/* Applique réellement la config (mode brut + vitesse) au port
	 * physique. TCSANOW = application immédiate. C'est cette ligne,
	 * et seulement elle, qui rend tout ce qui précède effectif. */
	if (tcsetattr(fd, TCSANOW, &tty) == -1) {
		perror("tcsetattr");
		close(fd);
		return -1;
	}

	return fd;
}

/* Simple wrapper autour de read(2) : read() est un appel système
 * générique (fonctionne sur n'importe quel fd, pas seulement un port
 * série) ; toute la spécificité série a déjà été réglée dans
 * platform_serial_open() via termios. */
ssize_t
platform_serial_read(int fd, void *buf, size_t count)
{
	return read(fd, buf, count);
}

/* Pendant de platform_serial_read() côté écriture, même principe :
 * write() est générique, la config série est déjà en place sur fd. */
ssize_t
platform_serial_write(int fd, const void *buf, size_t count)
{
	return write(fd, buf, count);
}

/* TODO: implémentation Linux */
