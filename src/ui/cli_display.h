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
#ifndef CLI_DISPLAY_H
#define CLI_DISPLAY_H

#include <stdbool.h>
#include "core/mesh_state.h"
#include "core/device_config.h"
#include "core/message_history.h"
#include "core/channel_state.h"
#include <stddef.h>
void cli_display_node_list(mesh_state_t *state);
void cli_display_device_config(device_config_t *dconfig, bool show_all);
void cli_display_message_history(message_history_t *hist,channel_state_t *cstate, device_config_t *dconfig);
void cli_display_channel_list(channel_state_t *cstate, const device_config_t *dconfig);
const char *channel_display_name(const channel_state_t *cstate, const device_config_t *dconfig, int8_t channel_index, char *buf, size_t buflen);

#endif
