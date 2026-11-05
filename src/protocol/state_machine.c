//
// Created by springik on 05.11.26.
//

#include "../../include/protocol/state_machine.h"

constexpr size_t STATE_CAPACITY = 32;
static StateBehaviour state_machine_table[STATE_CAPACITY] = {};

void register_state(MinecraftState state, StateBehaviour behaviour) {
    if (state < STATE_CAPACITY) {
        state_machine_table[state] = behaviour;
    }
}

void process_client_packet(ProtocolContext *client, uint32_t packet_id, const uint8_t *data, size_t len) {
    StateBehaviour *current_behaviour = &state_machine_table[client->current_state];

    if (current_behaviour->handle_packet == nullptr) {
        client->current_state = MC_STATE_DISCONNECTED;
        return;
    }

    MinecraftState next_state = current_behaviour->handle_packet(client, packet_id, data, len);

    if (next_state != client->current_state) {
        if (current_behaviour->on_exit != nullptr)
            current_behaviour->on_exit(client);

        client->current_state = next_state;

        StateBehaviour *new_behaviour = &state_machine_table[next_state];

        if (new_behaviour->on_enter != nullptr) {
            new_behaviour->on_enter(client);
        }
    }
}