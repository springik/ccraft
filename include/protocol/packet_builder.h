//
// Created by springik on 05.11.26.
//

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "varint.h"
#include "../network/connection.h"

// This might overflow with world chunk packets
#define MAX_PACKET_SIZE 4096

typedef struct {
    uint8_t payload[MAX_PACKET_SIZE];
    size_t cursor;
    bool overflow;
} PacketBuilder;

void pb_init(PacketBuilder *pb, uint32_t packet_id);
void pb_send(PacketBuilder *pb, NetworkConnection* conn);

void pb_write_varint(PacketBuilder* pb, int32_t value);
void pb_write_string(PacketBuilder* pb, const char* value);
void pb_write_ushort(PacketBuilder* pb, uint16_t value);
void pb_write_bytes(PacketBuilder* pb, uint8_t* value, size_t len);