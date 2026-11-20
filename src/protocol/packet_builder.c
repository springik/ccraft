//
// Created by springik on 05.11.26.
//

#include <string.h>
#include "../../include/protocol/packet_builder.h"

#include <stdio.h>


void pb_init(PacketBuilder* pb, uint32_t packet_id) {
    pb->cursor = 0;
    pb->overflow = false;
    pb_write_varint(pb, packet_id);
}

void pb_write_bytes(PacketBuilder* pb, const uint8_t* data, size_t len) {
    if (pb->overflow || pb->cursor + len > MAX_PACKET_SIZE) {
        pb->overflow = true;
        return;
    }

    memcpy(pb->payload + pb->cursor, data, len);
    pb->cursor += len;
}

void pb_write_byte(PacketBuilder* pb, uint8_t value) {
    pb_write_bytes(pb, &value, 1);
}

void pb_write_bool(PacketBuilder* pb, bool value) {
    pb_write_byte(pb, value ? 1 : 0);
}

void pb_write_int(PacketBuilder* pb, int32_t value) {
    uint32_t uval = (uint32_t)value;
    uint8_t bytes[4] = {
        (uint8_t)(uval >> 24), (uint8_t)(uval >> 16),
        (uint8_t)(uval >> 8), (uint8_t)(uval)
    };

    pb_write_bytes(pb, bytes, 4);
}

void pb_write_long(PacketBuilder* pb, int64_t value) {
    uint64_t uval = (uint64_t)value;
    uint8_t bytes[8] = {
        (uint8_t)(uval >> 56), (uint8_t)(uval >> 48), (uint8_t)(uval >> 40), (uint8_t)(uval >> 32),
        (uint8_t)(uval >> 24), (uint8_t)(uval >> 16), (uint8_t)(uval >> 8), (uint8_t)(uval)
    };

    pb_write_bytes(pb, bytes, 8);
}

void pb_write_float(PacketBuilder* pb, float value) {
    uint32_t raw;
    // bit shifts on float are undefined behavior, so we bypass with memcpy()
    memcpy(&raw, &value, sizeof(float));

    pb_write_int(pb, (int32_t)raw);
}

void pb_write_double(PacketBuilder* pb, double value) {
    uint64_t raw;
    //again non-defined beh. => memcpy()
    memcpy(&raw, &value, sizeof(double));

    pb_write_long(pb, (int64_t)raw);
}

void pb_write_varint(PacketBuilder* pb, int32_t value) {
    if (pb->overflow || pb->cursor + 5 > MAX_PACKET_SIZE) {
        pb->overflow = true;
        return;
    }

    // advance pb cursor as well
    pb->cursor += write_varint(value, pb->payload + pb->cursor);
}

void pb_write_string(PacketBuilder* pb, const char* str) {
    size_t len = strlen(str);
    pb_write_varint(pb, (int32_t)len);
    pb_write_bytes(pb, (const uint8_t*)str, len);
}

void pb_write_ushort(PacketBuilder* pb, uint16_t value) {
    // bid endian encode
    uint8_t bytes[2] = {
        (uint8_t)(value >> 8),
        (uint8_t)(value & 0xFF)
    };

    pb_write_bytes(pb, bytes, 2);
}

void pb_write_uuid(PacketBuilder* pb, const uint8_t uuid[16]) {
    pb_write_bytes(pb, uuid, 16);
}

void pb_send(PacketBuilder* pb, NetworkConnection* conn) {
    if (pb->overflow) {
        fprintf(stderr, "[PacketBuilder] ERROR: Packet overflowed MAX_PACKET_SIZE (%d bytes)!\n", MAX_PACKET_SIZE);
        return;
    }

    // final frame(payload len + varint max size(5))
    uint8_t frame[MAX_PACKET_SIZE + 5];
    size_t header_len = write_varint((int32_t)pb->cursor, frame);
    memcpy(frame + header_len, pb->payload, pb->cursor);

    network_send_data(conn, frame, header_len + pb->cursor);
}