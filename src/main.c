//
// Created by springik on 05.11.26.
//

#include <stdio.h>
#include "../include/protocol/states.h"
#include "../include/network/tcp_server.h"

const int SERVER_PORT=25565;

int main(void) {
    init_handshake_module();
    init_status_module();
    init_login_module();
    init_configuration_module();
    init_play_module();

    printf("Starting ccraft server on port %d...\n", SERVER_PORT);

    network_start_server(SERVER_PORT);

    return 0;
}