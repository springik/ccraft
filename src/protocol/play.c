//
// Created by springik on 11.11.26.
//

#include <stdio.h>
#include <time.h>
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

static void send_set_center_chunk(ProtocolContext* ctx, int32_t chunk_x, int32_t chunk_z) {
    PacketBuilder pb;

    pb_init(&pb, 0x50); // Set senter chunk packet_id
    pb_write_varint(&pb, chunk_x);
    pb_write_varint(&pb, chunk_z);
    pb_send(&pb, ctx->net_conn);
}

static void send_empty_chunk(ProtocolContext* ctx, int32_t chunk_x, int32_t chunk_z) {
    PacketBuilder pb;
    pb_init(&pb, 0x25); // chunk data & light update(1.20.1)

    pb_write_int(&pb, chunk_x);
    pb_write_int(&pb, chunk_z);

    // Heightmaps nameless root tag + tag_end
    const uint8_t empty_nbt[] = { 0x0A, 0x00 };
    pb_write_bytes(&pb, empty_nbt, sizeof(empty_nbt));

    // Chunk sections:
    //  24 sections * 8 zero bytes = 192 bytes of *air*(for now hopefuly)
    constexpr size_t SECTION_DATA_LEN = 24 * 8;
    const uint8_t empty_sections[SECTION_DATA_LEN] = { 0 };
    pb_write_varint(&pb, (int32_t)SECTION_DATA_LEN);
    pb_write_bytes(&pb, empty_sections, SECTION_DATA_LEN);

    // Block entities
    pb_write_varint(&pb, 0);

    // Light data
    pb_write_varint(&pb, 0); // Sky light mask
    pb_write_varint(&pb, 0); // block light mask
    pb_write_varint(&pb, 0); // empty sky light mask
    pb_write_varint(&pb, 0); // empty block light mask
    pb_write_varint(&pb, 0); // sky light array count
    pb_write_varint(&pb, 0); // block light array count

    pb_send(&pb, ctx->net_conn);
}

static void send_keep_alive(ProtocolContext* ctx, int64_t id) {
    PacketBuilder pb;
    pb_init(&pb, 0x24); // clientbound keep alive
    pb_write_long(&pb, id);
    pb_send(&pb, ctx->net_conn);
}

void play_tick(ProtocolContext* ctx) {
    if (ctx->current_state != MC_STATE_PLAY)
        return;

    time_t now = time(nullptr); // no need for timer inst.
    if (now - ctx->last_keep_alive >= 5) {
        ctx->last_keep_alive = now;
        send_keep_alive(ctx, (int64_t)now);
    }
}

static void play_enter(ProtocolContext* ctx) {
    printf("Spawning %s into the world!\n", ctx->username);
    ctx->last_keep_alive = time(nullptr);

    send_login_play(ctx);
    send_set_center_chunk(ctx, 0, 0);
    send_empty_chunk(ctx, 0, 0);
    send_synchronize_position(ctx);
    send_keep_alive(ctx, (int64_t)ctx->last_keep_alive);
}

static MinecraftState play_handle(ProtocolContext* ctx, uint32_t packet_id, const uint8_t* data, size_t len) {
    if (packet_id == 0x00) {
        printf("%s confirmed spawn location", ctx->username);
    } else if (packet_id == 0x14) {
        // serverbound keep alive response (8byte long)
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
