//
// Created by springik on 05.11.26.
//

#pragma once
#include <stddef.h>
#include <stdint.h>

typedef struct ProtocolContext ProtocolContext;

typedef struct NetworkConnection {
    int socket_fd;
    uint8_t read_buffer[4096];
    size_t read_cursor;

    ProtocolContext *proto_ctx;
} NetworkConnection;

void network_send_data(NetworkConnection *conn, const uint8_t *data, size_t len);