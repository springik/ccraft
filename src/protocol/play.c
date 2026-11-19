//
// Created by springik on 11.11.26.
//

#include <stdio.h>
#include "../../include/protocol/state_machine.h"
#include "../../include/protocol/context.h"
#include "../../include/protocol/packet_builder.h"

static void send_login_play(ProtocolContext* ctx) {
    PacketBuilder pb;
    pb_init(&pb, 0x28);

    pb_write_int(&pb, 1); // Entity ID
    pb_write_bool(&pb, false); // Is Hardcore

    // Array of Dimension names (len=1)
    pb_write_int(&pb, 1);
    pb_write_string(&pb, "minecraft:overworld");

    pb_write_varint(&pb, 100); // Max Players
    pb_write_varint(&pb, 8); // View Distance
    pb_write_varint(&pb, 8); // Simulation Distance
    pb_write_bool(&pb, false); // Reduce debug info
    pb_write_bool(&pb, true); // Enable respawn screen
    pb_write_bool(&pb, false); // Do limited crafting

    pb_write_string(&pb, "minecraft:overworld"); // Dimension type
    pb_write_string(&pb, "minecraft:overworld"); // Dimension name

    pb_write_long(&pb, 0); // Hashed Seed
    pb_write_byte(&pb, 1); // Game mode: creative
    pb_write_byte(&pb, -1); // Previous game mode(-1=none)
    pb_write_bool(&pb, false); // Is Debug
    pb_write_bool(&pb, true); // Is Flat
    pb_write_bool(&pb, false); // Has Death location
    pb_write_varint(&pb, 0); // Portal CD

    pb_send(&pb, ctx->net_conn);
}

static void send_synchronize_position(ProtocolContext* ctx) {
    PacketBuilder pb;
    pb_init(&pb, 0x3E); //Warn: may not be the correct packet id for other protocol versions(target=1.20.1)

    pb_write_double(&pb, 0.0); // X
    pb_write_double(&pb, 100.0); // Y
    pb_write_double(&pb, 0.0); // Z
    pb_write_float(&pb, 0.0f); // Yaw
    pb_write_float(&pb, 0.0f); // Pitch
    pb_write_byte(&pb, 0x00); // Flags(0x00 = abs. cords)

    // Teleport ID(client echos this to confirm arrival)
    pb_write_varint(&pb, 1234);

    pb_send(&pb, ctx->net_conn);
}

static void play_enter(ProtocolContext* ctx) {
    printf("Spawning %s into the world!\n", ctx->username);
    send_login_play(ctx);
    send_synchronize_position(ctx);
}

static MinecraftState play_handle(ProtocolContext* ctx, uint32_t packet_id, const uint8_t* data, size_t len) {
    if (packet_id == 0x00) {
        printf("%s confirmed spawn location", ctx->username);
    }

    return MC_STATE_PLAY;
}

void init_play_module() {
    register_state(MC_STATE_PLAY, (StateBehaviour){
    .on_enter = play_enter,
    .handle_packet = play_handle,
    .on_exit = nullptr
    });
}
