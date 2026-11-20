//
// Created by springik on 05.11.26.
//

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <poll.h>

#include "../../include/network/connection.h"
#include "../../include/protocol/context.h"
#include "../../include/network/tcp_server.h"
#include "../../include/protocol/state_machine.h"
#include "../../include/protocol/varint.h"

void play_tick(ProtocolContext* ctx);

void handle_new_connection(int client_fd) {
    NetworkConnection *net_conn = calloc(1, sizeof(NetworkConnection));
    ProtocolContext *protoc_ctx = calloc(1, sizeof(ProtocolContext));

    net_conn->socket_fd = client_fd;
    net_conn->proto_ctx = protoc_ctx;

    protoc_ctx->current_state = MC_STATE_HANDSHAKE;
    protoc_ctx->net_conn = net_conn;

    // TODO: register socket with net solution like epoll
}

void network_send_data(NetworkConnection* conn, const uint8_t* data, size_t len) {
    if (conn == nullptr || conn->socket_fd == 0)
        return;

    printf("[NET] sending %zu bytes\n", len);
    for (size_t i = 0; i < len && i < len; i++) {
        printf(" %02x", data[i]);
    }
    printf("\n");

    size_t total_sent = 0;
    // "Temporary" solution for testing. Should be replaced when there's a network solution
    while (total_sent < len) {
        ssize_t sent = send(conn->socket_fd, data + total_sent, len - total_sent, MSG_NOSIGNAL);

        if (sent <= 0) {
            //socket closed or send buff filled
            break;
        }

        total_sent += sent;
    }
}

static void handle_client(int client_fd) {
    NetworkConnection *net_conn = calloc(1, sizeof(NetworkConnection));
    ProtocolContext *proto_ctx = calloc(1, sizeof(ProtocolContext));

    net_conn->socket_fd = client_fd;
    net_conn->proto_ctx = proto_ctx;

    proto_ctx->current_state = MC_STATE_HANDSHAKE;
    proto_ctx->net_conn = net_conn;

    struct pollfd pfd = {
        .fd = client_fd,
        .events = POLLIN,
    };

    while (proto_ctx->current_state != MC_STATE_DISCONNECTED) {
        int ready = poll(&pfd, 1, 1000);
        printf("[NET] poll returned %d (state=%d, cursor=%zu)\n", ready, proto_ctx->current_state, net_conn->read_cursor);
        if (ready < 0)
            break;

        if (ready == 0) {
            play_tick(proto_ctx);
            continue;
        }

        size_t space_left = sizeof(net_conn->read_buffer) - net_conn->read_cursor;
        if (space_left == 0)
            break; // Single packet over 4KB buff

        ssize_t n = recv(client_fd, net_conn->read_buffer + net_conn->read_cursor, space_left, 0);
        printf("[NET] recv returned %zd bytes\n", n);
        if (n <= 0)
            break; // Client closed socket or netwrk err

        net_conn->read_cursor += (size_t)n;

        // Extract all complete packets sitting in read buff
        while (net_conn->read_cursor > 0 && proto_ctx->current_state != MC_STATE_DISCONNECTED) {
            VarIntResult frame_len = read_varint(net_conn->read_buffer, net_conn->read_cursor);
            if (frame_len.bytes_read == 0) {
                // If VarInt failed at over 4 bytes data must be malformed
                // or the read_varint implementation is wrong oopsie
                if (net_conn->read_cursor >= 5)
                    proto_ctx->current_state = MC_STATE_DISCONNECTED;

                break; // Wait for more from recv()
            }

            if (frame_len.value <= 0) {
                proto_ctx->current_state = MC_STATE_DISCONNECTED;
                break;
            }

            size_t total_frame_bytes = frame_len.bytes_read + (size_t)frame_len.value;

            // Full packet hasn't arrived wait for next
            if (net_conn->read_cursor < total_frame_bytes)
                break;

            // Parse packet_id from frame body
            const uint8_t* frame_body = net_conn->read_buffer + frame_len.bytes_read;
            size_t body_len = (size_t)frame_len.value;

            VarIntResult packet_id = read_varint(frame_body, body_len);
            if (packet_id.bytes_read == 0) {
                proto_ctx->current_state = MC_STATE_DISCONNECTED;
                break;
            }

            const uint8_t* payload = frame_body + packet_id.bytes_read;
            size_t payload_len = body_len - packet_id.bytes_read;

            // Hand over to state machine
            process_client_packet(proto_ctx, (uint32_t)packet_id.value, payload, payload_len);

            size_t remaining = net_conn->read_cursor - total_frame_bytes;
            // Shift the remaining bytes to the front of read_buffer
            if (remaining > 0) {
                memmove(net_conn->read_buffer, net_conn->read_buffer + total_frame_bytes, remaining);
            }
            net_conn->read_cursor = remaining;
        }

        play_tick(proto_ctx);
    }

    shutdown(client_fd, SHUT_WR); // We still want to potentially receive
    close(client_fd);
    free(proto_ctx);
    free(net_conn);
}

void network_start_server(uint16_t port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in sock_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };

    if (bind(server_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 16) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0)
            continue;

        handle_client(client_fd);
    }
}