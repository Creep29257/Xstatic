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
 * mesh_state.c -- implémentation de l'état du mesh, logique métier pure, aucune dépendance OS ou UI.
 */

#include "mesh_state.h"
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h>

struct mesh_state {
    mesh_node_t *head;   // pointeur vers le premier node de la liste (NULL si liste vide)
};


mesh_state_t *mesh_state_init(void) 
{
    mesh_state_t *state;
    state = malloc(sizeof(mesh_state_t));
    if(state == NULL) {
        perror("mesh_state_init: malloc");
        return NULL;
    }
    state->head = NULL;
    return state;
}

void mesh_state_destroy(mesh_state_t *state)
{
    mesh_node_t *current;
    mesh_node_t *next_node;
    if(state == NULL) {
        perror("mesh_state_destroy: no node");
        return;
    }
    current = state->head;
    while(current!= NULL) {
        next_node = current->next;
        if (current->custom_name != NULL) {
            free(current->custom_name);
        }
        free(current);
        current =next_node;
    }
    free(state);
}

mesh_node_t *
mesh_state_find_node(mesh_state_t *state, uint32_t num)
{
    mesh_node_t *current;

    current = state->head;
    while (current != NULL) {
        if (current->num == num) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

bool mesh_state_add_or_update_node(mesh_state_t *state, mesh_node_info_t *node_info) 
{
   mesh_node_t *existing;
   existing = mesh_state_find_node(state, node_info->num);
   if(existing != NULL)
   {
    existing->hw_model = node_info->hw_model;
    existing->position= node_info->position;
    strcpy(existing->long_name, node_info->long_name);

    if (existing->custom_name != NULL)
     {
        free(existing->custom_name);
        existing->custom_name =NULL;
    }
    if(node_info->custom_name !=NULL){
    existing->custom_name = malloc(strlen(node_info->custom_name) + 1);    
        if(existing->custom_name == NULL)
            {
                perror("mesh_state_add_or_update_node  update custom_name, malloc");
                return false;
            }
        strcpy(existing->custom_name, node_info->custom_name);
        }
    
        return true;


   }
   else 
   {
    mesh_node_t *new_node;
    new_node = malloc(sizeof(mesh_node_t));
    if (new_node == NULL)
        {
            perror("mesh_state_add_or_update_node add , malloc");
            return false;
        } 
        new_node->num = node_info->num;
        new_node->hw_model = node_info->hw_model;
        new_node->position= node_info->position;
        strcpy(new_node->long_name, node_info->long_name);
        new_node->custom_name = NULL;
        if(node_info->custom_name != NULL)
        {
            new_node->custom_name = malloc(strlen(node_info->custom_name)+1);
            if(new_node->custom_name == NULL)
            {
                perror("mesh_state_add_or_update_node:  add , custom_name malloc");
                free(new_node);
                return false;
            }
            strcpy(new_node->custom_name, node_info->custom_name);

        }
        new_node->next = state->head;
        state ->head = new_node;
        return true;


    
   }


}

mesh_node_t *mesh_state_first_node(mesh_state_t *state)
{
   
    mesh_node_t *first_node;
    first_node = state->head;
     if ( first_node == NULL)
    {
        return NULL;
    }
    
    return first_node;
}

mesh_node_t *mesh_state_next_node(mesh_node_t *node)
{
  return node->next;
}
