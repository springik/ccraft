//
// Created by springik on 05.11.26.
//

#include <string.h>
#include <stdio.h>
#include "../../include/protocol/state_machine.h"
#include "../../include/protocol/context.h"
#include "../../include/protocol/varint.h"
#include "../../include/network/connection.h"

static void send_status_response(ProtocolContext* ctx) {
    //TODO: read from file at least and do not hardcode
    const char* json = "{"
        "\"version\": {\"name\": \"1.20.1\", \"protocol\": 763},"
        "\"players\":{\"max\": 100, \"online\": 0},"
        "\"description\": {\"text\": \"Hello from C23 Server!\""
    "}";

    size_t json_len = strlen(json);

    // predefine payload
    uint8_t payload[2048];
    size_t payload_len = 0;

    // write: 1. packet_id, 2. str_len, 3. string data
    payload_len += write_varint(0x00, payload + payload_len);
    payload_len += write_varint(json_len, payload + payload_len);
    payload_len += json_len;

    // wrap payload
    uint8_t packet[2048];
    size_t packet_len = 0;

    packet_len += write_varint(payload_len, packet + packet_len);
    memcpy(packet + packet_len, payload, payload_len);
    packet_len += payload_len;

    network_send_data(ctx->net_conn, packet, packet_len);
}

static void send_ping_response(ProtocolContext* ctx, const uint8_t* ping_payload) {
    uint8_t payload[9];
    size_t payload_len = 0;

    // echo back packet_id
    payload_len += write_varint(0x01, payload + payload_len);
    // echo exactly the 8 bytes
    memcpy(payload + payload_len, ping_payload, 8);
    payload_len += 8;

    uint8_t packet[16];
    size_t packet_len = 0;

    packet_len += write_varint(payload_len, packet + packet_len);
    memcpy(packet + packet_len, payload, payload_len);
    packet_len += payload_len;

    network_send_data(ctx->net_conn, packet, packet_len);
}

static MinecraftState status_handle(ProtocolContext* ctx, uint32_t packet_id, const uint8_t* data, size_t len) {
    if (packet_id == 0x00) {
        send_status_response(ctx);
        return MC_STATE_STATUS; // Wait for the follow-up ping
    }

    if (packet_id == 0x01) {
        // malformed check
        if (len < 8)
            return MC_STATE_DISCONNECTED;

        send_ping_response(ctx, data);

        /*
         * after reponding to the ping the sequence is complete and
         * we can disconnect the client
        */
        return MC_STATE_DISCONNECTED;
    }
}

void init_status_module() {
    register_state(MC_STATE_STATUS, (StateBehaviour){
    .on_enter = nullptr,
    .handle_packet = status_handle,
    .on_exit = nullptr,
    });
}