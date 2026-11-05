//
// Created by springik on 05.11.26.
//

#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct ProtocolContext ProtocolContext;

typedef enum {
    MC_STATE_HANDSHAKE = 0,
    MC_STATE_STATUS = 1,
    MC_STATE_LOGIN = 2,
    MC_STATE_CONFIGURATION = 3,
    MC_STATE_PLAY = 4,
    MC_STATE_DISCONNECTED = 5,
    MC_STATE_MAX = 6,
} MinecraftState;

typedef struct {
    void (*on_enter)(ProtocolContext *client);
    [[nodiscard]] MinecraftState (*handle_packet)(ProtocolContext *client, uint32_t packet_id, const uint8_t *data, size_t len);
    void (*on_exit)(ProtocolContext *client);
} StateBehaviour;

void register_state(MinecraftState state, StateBehaviour behavior);
void process_client_packet(ProtocolContext *client, uint32_t packet_id, const uint8_t *data, size_t len);