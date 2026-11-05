//
// Created by springik on 05.11.26.
//

#pragma once
#include <stdint.h>
#include "state_machine.h"

typedef struct NetworkConnection NetworkConnection;

typedef struct ProtocolContext {
    MinecraftState current_state;
    int protocol_version;
    NetworkConnection *net_conn;

    char username[17]; // longest nickname(16 chars) + null terminator at the end
    uint8_t uuid[16];
} ProtocolContext;