//
// Created by tusia on 16.02.17.
//

#include "ConnectionManager.h"
#include "constants.h"
#include <arpa/inet.h>

ConnectionManager::ConnectionManager(const char *host, int port) : port(port), host(host) {}

int ConnectionManager::createSocket() {
    descriptor = socket(PF_INET, SOCK_STREAM, 0);
    if (descriptor == -1) {
        perror("Error creating the socket!");
        return -1;
    }
    return 0;
}

void ConnectionManager::createSockaddr() {
    sockaddr_in fcgiSocket;
    fcgiSocket.sin_family = AF_INET;
    fcgiSocket.sin_port = htons((uint16_t) port);
    fcgiSocket.sin_addr.s_addr = inet_addr(host);
    socketStruct = fcgiSocket;
}





