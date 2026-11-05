//
// Created by springik on 05.11.26.
//

#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>


typedef struct {
    int32_t value;
    size_t bytes_read;
} VarIntResult;

typedef struct {
    int64_t value;
    size_t bytes_read;
} VarLongResult;

// Encoding utils
size_t write_varint(int32_t value, uint8_t* buffer);
size_t write_varlong(int64_t value, uint8_t* buffer);

// Decoding utiĺs
[[nodiscard]] VarIntResult read_varint(const uint8_t* buffer, size_t max_len);
[[nodiscard]] VarLongResult read_varlong(const uint8_t* buffer, size_t max_len);