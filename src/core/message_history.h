/*
 * BSD 2-Clause License
 *
 * Copyright (c) 2026, Rémi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MESSAGE_HISTORY_H
#define MESSAGE_HISTORY_H

#include <stdint.h>
#include <stdbool.h>

#define MESSAGE_HISTORY_SIZE  20

struct message_entry
{
   uint32_t id;
   uint32_t num;
   char text[234];

};

struct message_history
{
   struct message_entry entries[MESSAGE_HISTORY_SIZE];
   uint8_t write_index;
   uint32_t next_id;

};

typedef struct message_entry message_entry_t;
typedef struct message_history message_history_t;

void message_history_init(message_history_t *hist);
uint32_t message_history_add(message_history_t *hist, uint32_t num, const char *text);
message_entry_t *message_history_find_by_id(message_history_t *hist, uint32_t id);
message_history_t *message_history_list(message_history_t *hist, uint8_t *start_index, uint8_t *count);
#endif
