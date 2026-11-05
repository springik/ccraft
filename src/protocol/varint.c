//
// Created by springik on 05.11.26.
//

/*
 * This strategy was chosen, because the buffer will contain fields represented by Var[Int/Long]s and
 * as such this way we can extract fields in a simple process one by one, when reading the data sent by the client
 */

#include "../../include/protocol/varint.h"

size_t write_varint(int32_t value, uint8_t* buffer) {
    size_t bytes_written = 0;
    uint32_t uval = (uint32_t)value;

    while (true) {
        // remaining bits are 0 => write last byte then exit
        if ((uval & ~0x07F) == 0) {
            buffer[bytes_written++] = (uint8_t)uval;
            return bytes_written;
        }

        // Write 7 bits until reaching the end(8th is defined as "continue" flag)
        // of course setting cont. bit to 1
        buffer[bytes_written++] = (uint8_t)((uval & 0x7F) | 0x80);
        uval >>= 7;
    }
}

size_t write_varlong(int64_t value, uint8_t* buffer) {
    // The same happening as in 'write_varint', but for long(int64)
    size_t bytes_written = 0;
    uint64_t uval = (uint64_t)value;

    while (true) {
        if ((uval & ~0x07F) == 0) {
            buffer[bytes_written++] = (uint8_t)uval;
            return bytes_written;
        }

        buffer[bytes_written++] = (uint8_t)((uval & 0x7F) | 0x80);
        uval >>= 7;
    }
}

VarIntResult read_varint(const uint8_t* buffer, size_t max_len) {
    uint32_t value = 0;
    int position = 0;
    size_t bytes_read = 0;

    while (true) {
        /* Prevent buff overflow
         * not trusting the network layer
        */
        if (bytes_read >= max_len) {
            return (VarIntResult){
                .value = 0,
                .bytes_read = 0,
            };
        }

        uint8_t current_byte = buffer[bytes_read++];
        // Again going 7 bits at a time
        value |= (current_byte & 0x7F) << position;
        /*
         * check cont. bit
         * essentially: Are we done?
        */
        if ((current_byte & 0x80) == 0)
            break;

        position += 7;

        /* Another sanity check
         * not trusting the client to respect the type
         * malicious client prevention
        */
        if (position >= 32) {
            return (VarIntResult){
                .value = 0,
                .bytes_read = 0,
            };
        }
    }

    return (VarIntResult){
        .value = (int32_t)value,
        .bytes_read = bytes_read,
    };
}

VarLongResult read_varlong(const uint8_t* buffer, size_t max_len) {
    uint64_t value = 0;
    int position = 0;
    size_t bytes_read = 0;

    while (true) {
        if (bytes_read >= max_len) {
            return (VarLongResult){
                .value = 0,
                .bytes_read = 0,
            };
        }

        uint8_t current_byte = buffer[bytes_read++];
        value |= (current_byte & 0x7F) << position;
        if ((current_byte & 0x80) == 0)
            break;

        if (position >= 64) {
            return (VarLongResult){
                .value = 0,
                .bytes_read = 0,
            };
        }
    }

    return (VarLongResult){
        .value = (int64_t)value,
        .bytes_read = bytes_read,
    };
}