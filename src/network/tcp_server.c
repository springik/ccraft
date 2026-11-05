//
// Created by springik on 05.11.26.
//

#include <stdlib.h>
#include "../../include/network/connection.h"
#include "../../include/protocol/context.h"

void handle_new_connection(int client_fd) {
    NetworkConnection *net_conn = calloc(1, sizeof(NetworkConnection));
    ProtocolContext *protoc_ctx = calloc(1, sizeof(ProtocolContext));

    net_conn->socket_fd = client_fd;
    net_conn->proto_ctx = protoc_ctx;

    protoc_ctx->current_state = MC_STATE_HANDSHAKE;
    protoc_ctx->net_conn = net_conn;

    // TODO: register socket with net solution like epoll
}