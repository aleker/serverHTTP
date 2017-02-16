//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_CONNECTIONMANAGER_H
#define SERWERHTTP_CONNECTIONMANAGER_H

#include "constants.h"

class ConnectionManager {
public:
    int descriptor;
    sockaddr_in socketStruct;
    int port;
    const char* host;

    ConnectionManager(){};
    ConnectionManager(const char *host, int port);

    int createSocket();
    void createSockaddr();
    int connectSocket();
    int bindSocket();
    int fullConnection();
    int createFCGIConnection();
    void forwardMessage(int clientSocketFd);
};


#endif //SERWERHTTP_CONNECTIONMANAGER_H
