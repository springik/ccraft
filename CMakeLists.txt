cmake_minimum_required(VERSION 4.3)
project(ccraft C)

set(CMAKE_C_STANDARD 23)

add_executable(ccraft src/main.c
        src/client.h
        src/protocol/types.c
        src/protocol/packet.c
        src/states/handshake.c
        src/states/status.c
        src/states/login.c
        src/states/play.c
        src/states/play_init.c)
