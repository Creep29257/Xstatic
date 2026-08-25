/*
 * Copyright (c) 2026 Rémi Assailly
 * remi@assailly.com
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
 * mesh_state.h -- interface de l'état du mesh (liste des nodes connus), type opaque mesh_state_t.
 */

#ifndef MESHTASTIC_MESH_STATE_H
#define MESHTASTIC_MESH_STATE_H

#include <stdbool.h>
#include <stdint.h>

#define MESH_LONG_NAME_MAX 40  /* doit matcher meshtastic_User.long_name (mesh.pb.h) */


struct mesh_position {
    bool valid;
    int32_t latitude_i;
    int32_t longitude_i;
    int32_t altitude;
};


struct mesh_node {
    uint32_t num;
    char long_name[MESH_LONG_NAME_MAX];
    char *custom_name;
    uint8_t hw_model;
    struct mesh_position position;
    struct mesh_node *next;
};

struct mesh_node_info {
    uint32_t num;
    char long_name[MESH_LONG_NAME_MAX];
    char *custom_name;
    uint8_t hw_model;
    struct mesh_position position;
};


typedef struct mesh_position mesh_position_t;
typedef struct mesh_node mesh_node_t;
typedef struct mesh_node_info mesh_node_info_t;
typedef struct mesh_state mesh_state_t;

mesh_state_t *mesh_state_init(void);
void mesh_state_destroy(mesh_state_t *state);
bool mesh_state_add_or_update_node(mesh_state_t *state, mesh_node_info_t *node_info);
mesh_node_t *mesh_state_find_node(mesh_state_t *state, uint32_t num);
mesh_node_t *mesh_state_first_node(mesh_state_t *state);
mesh_node_t *mesh_state_next_node(mesh_node_t *node);

#endif /* MESHTASTIC_MESH_STATE_H */
