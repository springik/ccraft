//
// Created by springik on 05.11.26.
//

#include <string.h>
#include <stdio.h>
#include "../../include/protocol/state_machine.h"
#include "../../include/protocol/context.h"
#include "../../include/protocol/varint.h"

static MinecraftState handshake_handle(ProtocolContext* ctx, uint32_t packet_id, const uint8_t* data, size_t len) {
    // Check for onlůy valid packet(0x00)
    if (packet_id != 0x00)
        return MC_STATE_DISCONNECTED;

    size_t cursor = 0;

    // Read prot version
    VarIntResult version = read_varint(data + cursor, len - cursor);
    if (version.bytes_read == 0)
        return MC_STATE_DISCONNECTED;
    ctx->protocol_version = version.value;
    cursor += version.bytes_read;

    // Read server addr (String = VarInt Len + UTF-8 bytes)
    VarIntResult addr_len = read_varint(data + cursor, len - cursor);
    if (addr_len.bytes_read == 0)
        return MC_STATE_DISCONNECTED;
    cursor += addr_len.bytes_read;

    /*
     * safety check: are we getting len that is bigger than remaining buffer
     * malicious client prevention #2
    */
    if (cursor + addr_len.value > len)
        return MC_STATE_DISCONNECTED;

    cursor += addr_len.value;

    /*
     * Read server port(ushort)
     * Minecraft uses Java's network byte order => big endian
    */
    if (cursor + 2 > len)
        return MC_STATE_DISCONNECTED;

    uint16_t port = (data[cursor] << 8) | data[cursor + 1];
    cursor += 2;

    // Read next state intention
    VarIntResult next_state = read_varint(data + cursor, len - cursor);
    if (next_state.bytes_read == 0)
        return MC_STATE_DISCONNECTED;
    cursor += next_state.bytes_read;

    // Debug logging
    printf("Handshake Received: Version=%d, Port=%u, NextState=%d\n",
        ctx->protocol_version, port, next_state.value);

    // route to next state
    //TODO: consider cleaner implementation(not sure if even possible in C)
    if (next_state.value == MC_STATE_STATUS) {
        return MC_STATE_STATUS;
    } else if (next_state.value == MC_STATE_LOGIN) {
        return MC_STATE_LOGIN;
    }

    // invalid state request(from current state)
    return MC_STATE_DISCONNECTED;
};