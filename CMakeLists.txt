cmake_minimum_required(VERSION 4.3)
project(ccraft C)

set(CMAKE_C_STANDARD 23)

add_executable(ccraft src/main.c src/network/tcp_server.c
        src/protocol/state_machine.c
        src/protocol/handshake.c
        src/protocol/login.c
        src/game/player.c
        include/protocol/state_machine.h
        include/protocol/context.h
        include/network/connection.h)
