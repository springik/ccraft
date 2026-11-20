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
#define MAX_PACKET_SIZE 65536

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
void pb_write_bytes(PacketBuilder* pb, const uint8_t* value, size_t len);
void pb_write_uuid(PacketBuilder* pb, const uint8_t uuid[16]);
void pb_write_byte(PacketBuilder* pb, uint8_t value);
void pb_write_bool(PacketBuilder* pb, bool value);
void pb_write_int(PacketBuilder* pb, int32_t value);
void pb_write_long(PacketBuilder* pb, int64_t value);
void pb_write_float(PacketBuilder* pb, float value);
void pb_write_double(PacketBuilder* pb, double value);