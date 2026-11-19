//
// Created by springik on 11.11.26.
//

#include <stdio.h>
#include "../../include/protocol/state_machine.h"
#include "../../include/protocol/context.h"
#include "../../include/protocol/packet_builder.h"

static const uint8_t registry_nbt_blob[] = {
    // Compiler directly inserts the binary at compile time(this lives in executable ro mem and no need for fs io operations
    #embed "../../include/registry_1_20_1.nbt"
};

static void send_registry_data(ProtocolContext* ctx) {
    PacketBuilder pb;
    pb_init(&pb, 0x05);

    // we know the size at compile tome => can use sizeof()
    pb_write_bytes(&pb, registry_nbt_blob, sizeof(registry_nbt_blob));

    pb_send(&pb, ctx->net_conn);
}

static void send_finish_configuration(ProtocolContext* ctx) {
    PacketBuilder pb;
    pb_init(&pb, 0x02);

    pb_send(&pb, ctx->net_conn);
}

static void configuration_enter(ProtocolContext* ctx) {
    printf("Sending registries to %s...\n", ctx->username);

    send_registry_data(ctx);
    send_finish_configuration(ctx);
}

static MinecraftState configuration_handle(ProtocolContext* ctx, uint32_t packet_id, const uint8_t* data, size_t len) {
    if (packet_id == 0x00) {
        // Received: client info(locale, view distance, main hand, etc.)
        printf("Received Client information from %s.\n", ctx->username);
        return MC_STATE_CONFIGURATION;
    }
    if (packet_id == 0x01) {
        // plugin mess
        return MC_STATE_CONFIGURATION;
    }
    if (packet_id == 0x02) {
        printf("%s finished configuration. Joining world!\n", ctx->username);
        return MC_STATE_PLAY;
    }

    return MC_STATE_DISCONNECTED;
}

void init_configuration_module(void) {
    register_state(MC_STATE_CONFIGURATION, (StateBehaviour) {
        .on_enter = configuration_enter,
        .handle_packet = configuration_handle,
        .on_exit = nullptr,
    });
}