//
// Created by springik on 05.11.26.
//

#include <string.h>
#include "../../include/protocol/packet_builder.h"


void pb_init(PacketBuilder* pb, uint32_t packet_id) {
    pb->cursor = 0;
    pb->overflow = false;
    pb_write_varint(pb, packet_id);
}

void pb_write_bytes(PacketBuilder* pb, uint8_t* data, size_t len) {
    if (pb->overflow || pb->cursor + len > MAX_PACKET_SIZE) {
        pb->overflow = true;
        return;
    }

    memcpy(pb->payload + pb->cursor, data, len);
    pb->cursor += len;
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

void pb_send(PacketBuilder* pb, NetworkConnection* conn) {
    if (pb->overflow) {
        // Consider logging an err here right now the overflowed packets are silently dropped
        return;
    }

    // final frame(payload len + varint max size(5))
    uint8_t frame[MAX_PACKET_SIZE + 5];
    size_t header_len = write_varint((int32_t)pb->cursor, frame);
    memcpy(frame + header_len, pb->payload, pb->cursor);
    network_send_data(conn, frame, header_len + pb->cursor);
}