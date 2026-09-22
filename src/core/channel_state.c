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
 * channel_state.h
 *  */
#include "meshtastic/channel.pb.h"
#include <stdbool.h>
#include <string.h>
#include "core/channel_state.h"

void channel_state_init(channel_state_t *channels)
{
    for(int i=0;i<8;i++)
    {
    channels->channels[i].has_channel = false;
    }


}

void channel_state_update(channel_state_t *channels, const meshtastic_Channel *ch)
{
if(ch->index >=0 && ch->index <8)
{
  int8_t index =ch->index;  
  channels->channels[index].has_channel= true;
  memcpy(channels->channels[index].name,ch->settings.name,(sizeof(ch->settings.name)));
  channels->channels[index].psk = ch->settings.psk;
  channels->channels[index].role = ch->role;
  channels->channels[index].uplink_enabled = ch->settings.uplink_enabled;
  channels->channels[index].downlink_enabled = ch->settings.downlink_enabled;

}

}
