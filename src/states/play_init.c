//
// Created by springik on 04.11.26.
//

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include "../client.h"

int encode_varint(int32_t value, uint8_t *buffer);

const uint8_t registry_codec_nbt[] = {
#embed "../registry.nbt"
};

void spawn_player(Client *client) {
    //TODO: Specify 0x28 packet id

    // Needs to fit the NBT data, so it is large.
    uint8_t payload_buffer[16384];
    int offset = 0;

    int32_t entity_id = 1;
    payload_buffer[offset++] = (entity_id >> 24) & 0xFF;
    payload_buffer[offset++] = (entity_id >> 16) & 0xFF;
    payload_buffer[offset++] = (entity_id >> 8) & 0xFF;
    payload_buffer[offset++] = entity_id & 0xFF;

    payload_buffer[offset++] = 0x00; // Is HC
    payload_buffer[offset++] = 0x01; // Game Mode
    payload_buffer[offset++] = -1; // Previous Game mode

    // Amount of dimensions
    offset += encode_varint(1, payload_buffer + offset);

    // Dimension names arr
    const char* dim_name = "minecraft:overworld";
    int dim_length = strlen(dim_name);
    offset += encode_varint(dim_length, payload_buffer + offset);
    memcpy(payload_buffer + offset, dim_name, dim_length);
    offset += dim_length;

    memcpy(payload_buffer + offset, registry_codec_nbt, sizeof(registry_codec_nbt));
    offset += sizeof(registry_codec_nbt);

    // Dimension type and name
    offset += encode_varint(dim_length, payload_buffer + offset);
    memcpy(payload_buffer + offset, dim_name, dim_length);
    offset += dim_length;
    offset += encode_varint(dim_length, payload_buffer + offset);
    memcpy(payload_buffer + offset, dim_name, dim_length);
    offset += dim_length;

    // World seed(hashed)
    memset(payload_buffer + offset, 0, 8);
    offset += 8;

    // Max Players
    offset += encode_varint(20, payload_buffer + offset);
    // View Distance
    offset += encode_varint(8, payload_buffer + offset);
    // Simulation Distance
    offset += encode_varint(8, payload_buffer + offset);

    payload_buffer[offset++] = 0x00; // Reduced debug info
    payload_buffer[offset++] = 0x01; // Enable Respawn screen
    payload_buffer[offset++] = 0x00; // Is Debug
    payload_buffer[offset++] = 0x01; // Is Flat
    payload_buffer[offset++] = 0x00; // Has Death Location

    offset += encode_varint(0, payload_buffer + offset); // Portal CD

    send(client->socket_fd, payload_buffer, offset, 0);

    // Player transforms
    offset = 0;
    // X,Y,Z <- 8byte doubles
    double x = 0.0, y = 100.0, z = 0.0;
    memcpy(payload_buffer + offset, &x, 8); offset += 8;
    memcpy(payload_buffer + offset, &y, 8); offset += 8;
    memcpy(payload_buffer + offset, &z, 8); offset += 8;

    // Yaw & Pitch are 4byte floats
    float yaw = 0.0f, pitch = 0.0f;
    memcpy(payload_buffer + offset, &yaw, 4); offset += 4;
    memcpy(payload_buffer + offset, &pitch, 4); offset += 4;

    payload_buffer[offset++] = 0x00; // Flags - 0x00 -> absolute cords

    offset += encode_varint(1, payload_buffer + offset);

    //TODO: specify Ox3E packet id
    send(client->socket_fd, payload_buffer, offset, 0);
}