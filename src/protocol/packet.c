//
// Created by springik on 04.11.26.
//

#include <stdint.h>
#include "../client.h"

void route_packet(Client *client, int packet_id, const uint8_t *payload, int payload_length) {
    switch (client->state) {
        case STATE_HANDSHAKE:
            handle_handshake_packet(client, packet_id, payload);
            break;
        case STATE_STATUS:
            handle_status_packet(client, packet_id, payload);
            break;
        case STATE_LOGIN:
            handle_login_packet(client, packet_id, payload);
            break;
        case STATE_PLAY:
            handle_play_packet(client, packet_id, payload);
            break;
    }
}