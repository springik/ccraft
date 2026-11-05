//
// Created by springik on 04.11.26.
//

#ifndef CCRAFT_CLIENT_H
#define CCRAFT_CLIENT_H

enum ConnectionState {
    STATE_HANDSHAKE = 0,
    STATE_STATUS = 1,
    STATE_LOGIN = 2,
    STATE_CONFIGURATION = 3,
    STATE_PLAY = 4,
};

typedef struct {
    int socket_fd;
    enum ConnectionState state;
    // Add buffers for partial TCP reads
} Client;

#endif //CCRAFT_CLIENT_H
