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
 * platform.h -- interface OS-agnostique pour l'accès au port série.
 */

#ifndef MESHTASTIC_PLATFORM_H
#define MESHTASTIC_PLATFORM_H

#include <stddef.h>     /* pour size_t */
#include <sys/types.h>  /* pour ssize_t */

/*
 * Interface OS-agnostique pour l'accès au port série.
 * Chaque OS supporté (FreeBSD, Linux...) fournit sa propre implémentation
 * de ces fonctions dans un fichier platform_<os>.c séparé, mais main.c
 * et le reste du programme n'appellent jamais que ces signatures-ci.
 */

/* Ouvre le device série (ex: "/dev/cuaU0"), le configure en mode brut
 * à 115200 bauds. Retourne un descripteur de fichier valide, ou -1
 * en cas d'erreur (voir stderr via perror pour le détail). */
int platform_serial_open(const char *device);

/* Lit jusqu'à `count` octets depuis `fd` dans `buf`.
 * Retourne le nombre d'octets effectivement lus (peut être < count),
 * 0 si rien de disponible, ou -1 en cas d'erreur. */
ssize_t platform_serial_read(int fd, void *buf, size_t count);

/* Ferme proprement le descripteur de fichier série. */
void platform_serial_close(int fd);

/* Écrit `count` octets depuis `buf` vers `fd`.
 * Retourne le nombre d'octets effectivement écrits, ou -1 en cas d'erreur. */
ssize_t platform_serial_write(int fd, const void *buf, size_t count);

/* cherche le device série, remplit son chemin dans serial_path retourne 0 succés ou -1 echec
*/
int platform_serial_find_device(char *serial_path, size_t serial_path_size);

#endif /* MESHTASTIC_PLATFORM_H */
