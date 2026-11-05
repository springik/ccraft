//
// Created by springik on 04.11.26.
//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include "../client.h"

int encode_varint(int32_t value, uint8_t *buffer);

void handle_status_packet(Client *client, int packet_id, const uint8_t *payload, int payload_length) {
    if (packet_id == 0x00) {
        // TODO: get this from a config file no just with *magic*
        const char *json_response = "{\"version\":{\"name\":\"C23 MVP\",\"protocol\":763},\"players\":{\"max\":20,\"online\":0},\"description\":{\"text\":\"Hello from a C23 Socket!\"}}";
        int json_length = strlen(json_response);

        uint8_t payload_buffer[1024];
        int payload_offset = encode_varint(json_length, payload_buffer);
        memcpy(payload_buffer + payload_offset, json_response, json_length);
        payload_offset += json_length;

        uint8_t final_packet[1024];
        int packet_id_length = 1;
        int total_length = packet_id_length + payload_length;

        int final_offset = encode_varint(total_length, final_packet);
        final_packet[final_offset++] = 0x00;
        memcpy(final_packet + final_offset, payload_buffer, payload_length);
        final_offset += payload_length;

        send(client->socket_fd, final_packet, final_offset, 0);
    } else if (packet_id == 0x01) {
        uint8_t final_packet[16];
        int packet_id_length = 1;
        int total_length = packet_id_length + 8;

        int final_offset = encode_varint(total_length, final_packet);
        final_packet[final_offset++] = 0x01;

        memcpy(final_packet + final_offset, payload, 8);
        final_offset += 8;

        send(client->socket_fd, final_packet, final_offset, 0);
    }
}