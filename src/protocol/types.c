//
// Created by springik on 04.11.26.
//

#include <stdint.h>
#include <stdbool.h>

// Decodes a VarInt from a byte buffer.
// Returns the decoded 32-bit signed integer.
// 'bytes_read is updated with the number of bytes consumed from the buffer.
int32_t decode_varint(const uint8_t *buffer, int *bytes_read) {
    int32_t value = 0;
    int position = 0;
    uint8_t current_byte;

    while (true) {
        current_byte = buffer[position];

        value |= (current_byte & 0x7f) << (position * 7);

        position++;

        if ((current_byte & 0x80) == 0)
            break;

        if (position >= 5) {
            // Exceeded 5 bytes thus breaking the protocol standard
            // Consider dropping the conn here, when we have the conn
            if (bytes_read) *bytes_read = 0;
            return 0;
        }
    }

    if (bytes_read) *bytes_read = position;
    return value;
}

// Encodes a 32-bit integer into a VarInt byte buffer.
// Returns the number of bytes written (1 to 5).
int encode_varint(int32_t value, uint8_t *buffer) {
    int position = 0;
    uint32_t uval = (uint32_t) value;

    while (true) {
        if ((uval & ~0x7F) == 0) {
            buffer[position++] = (uint8_t) uval;
            break;
        }

        buffer[position++] = (uint8_t) ((uval & 0x7F) | 0x80);
        uval >>= 7;
    }

    return position;
}