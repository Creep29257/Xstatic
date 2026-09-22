#ifndef DISPATCH_H
#define DISPATCH_H

#include "protocol/framing.h"
#include "protocol/generated/meshtastic/mesh.pb.h"
#include "core/mesh_state.h"
#include "core/device_config.h"
#include "core/message_history.h"
#include "core/channel_state.h"

void dispatch_process_frame(struct framing_state *fs, meshtastic_FromRadio *msg,
    mesh_state_t *state, device_config_t *dconfig, message_history_t *history, channel_state_t *cstate);

#endif