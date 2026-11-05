//
// Created by springik on 04.11.26.
//

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include "../client.h"

int encode_varint(int32_t value, uint8_t *buffer);
int32_t decode_varint(const uint8_t *buffer, int *bytes_read);

void handle_login_packet(Client *client, int packet_id, const uint8_t *payload, int length) {
    if (packet_id == 0x00) {
        int offset = 0;
        int bytes_read = 0;

        int32_t name_length = decode_varint(payload + offset, &bytes_read);
        offset += bytes_read;

        const uint8_t *username = payload + offset;

        uint8_t payload_buffer[256];
        int payload_offset = 0;

        for (int i = 0; i < 16; i++) {
            payload_buffer[payload_offset++] = 0x01;
        }

        payload_offset += encode_varint(name_length, payload_buffer + payload_offset);
        memcpy(payload_buffer + payload_offset, username, name_length);
        payload_offset += name_length;

        payload_offset += encode_varint(0, payload_buffer + payload_offset);

        uint8_t final_packet[256];
        int packet_id_length = 1;
        int total_length = packet_id_length + payload_offset;

        int final_offset = encode_varint(total_length, final_packet);
        final_packet[final_offset++] = 0x02;

        memcpy(final_packet + final_offset, payload_buffer, payload_offset);
        final_offset += payload_offset;

        send(client->socket_fd, final_packet, final_offset, 0);

        client->state = STATE_PLAY;

        //NOTE: Immediately after transitioning to STATE_PLAY, the server MUST send
        // the "Login (Play)" packet to initialize the world, or the client will hang.
    }
}
