//
// Created by springik on 05.11.26.
//

#include <string.h>
#include <stdio.h>
#include "../../include/protocol/state_machine.h"
#include "../../include/protocol/context.h"
#include "../../include/protocol/packet_builder.h"

static void generate_offline_uuid(const char* username, uint8_t uuid_out[16]) {
    /*
     * TODO: REDO AS SUCH:
     * 1. Create string "OfflinePlayer:" + username
     * 2. MD5 hash that string to get 16bytes
     * 3. set version to 3
     * 4. set variant to 1
    */

    // Right now we generate a dummy UUID base on the first few letters
    memset(uuid_out, 0, 16);
    strncpy((char*)uuid_out, username, 16);

    uuid_out[6]= (uuid_out[6] & 0x0F) | 0x30; // version 3
    uuid_out[8]= (uuid_out[8] & 0x3F) | 0x80; // variant 1
}

static MinecraftState login_handle(ProtocolContext* ctx, uint32_t packet_id, const uint8_t* data, size_t len) {
    // The client started the login process
    if (packet_id == 0x00) {
        size_t cursor = 0;

        VarIntResult name_len = read_varint(data + cursor, len - cursor);
        if (name_len.bytes_read == 0 || name_len.value > 16)
            return MC_STATE_DISCONNECTED;

        char username[17] = {0};
        memcpy(username, data + cursor, name_len.value);
        cursor += name_len.value;

        printf("Player '%s' is logging in...\n", username);

        uint8_t uuid[16];
        generate_offline_uuid(username, uuid);

        memcpy(ctx->uuid, uuid, 16);
        strncpy(ctx->username, username, 16);

        PacketBuilder pb;
        pb_init(&pb, 0x02);
        pb_write_uuid(&pb, uuid);
        pb_write_string(&pb, username);
        pb_write_varint(&pb, 0); // number of properties for offline mode 0
        pb_send(&pb, ctx->net_conn);

        return MC_STATE_LOGIN;
    }

    // Client returned login ack
    if (packet_id == 0x03) {
        printf("Player '%s' acknowledged login. Moving to Configuration.\n", ctx->username);
        return MC_STATE_CONFIGURATION;
    }

    return MC_STATE_DISCONNECTED;
}

void init_login_module() {
    register_state(MC_STATE_LOGIN, (StateBehaviour){
    .on_enter = nullptr,
    .handle_packet = login_handle,
    .on_exit = nullptr,
    });
}