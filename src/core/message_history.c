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

#include "message_history.h"

void message_history_init(message_history_t *hist)
{
    hist->write_index = 0;
    hist->next_id = 0;
}

uint32_t message_history_add(message_history_t *hist, uint32_t num, const char *text)
{
    
    hist->entries[hist->write_index].id = hist->next_id;
    hist->entries[hist->write_index].num = num;
    strncpy(hist->entries[hist->write_index].text, text, sizeof(hist->entries[hist->write_index].text) - 1);
    hist->entries[hist->write_index].text[233] ='\0';
    hist->write_index = (hist->write_index + 1) % MESSAGE_HISTORY_SIZE;
    uint32_t assigned_id = hist->next_id;
    hist->next_id = hist->next_id + 1;
    return assigned_id;

}

message_entry_t *message_history_find_by_id(message_history_t *hist, uint32_t id)
{
    
    for(uint8_t i=0;i<MESSAGE_HISTORY_SIZE; i++)
    {
        if(hist->entries[i].id == id)
        {
            return &hist->entries[i];
        }
        
    }
    return NULL;
}