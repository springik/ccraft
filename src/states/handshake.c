//
// Created by springik on 04.11.26.
//

#include <stdint.h>
#include "../client.h"
#include "../protocol/types.c"

void handle_handshake_packet(Client *client, int packet_id, const uint8_t *payload, int payload_length) {
    if (packet_id != 0x00) return;

    int offset = 0;
    int bytes_read = 0;

    int32_t protocol_version = decode_varint(payload + offset, &bytes_read);
    offset += bytes_read;

    int32_t string_length = decode_varint(payload + offset, &bytes_read);
    offset += bytes_read;
    offset += string_length;

    uint16_t port = (payload[offset] << 8) | payload[offset + 1];
    offset += 2;

    int32_t next_state = decode_varint(payload + offset, &bytes_read);

    if (next_state == 1) {
        client->state = STATE_STATUS;
    } else if (next_state == 2) {
        client->state = STATE_LOGIN;
    }
}