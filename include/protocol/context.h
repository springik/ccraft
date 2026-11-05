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
} ProtocolContext;