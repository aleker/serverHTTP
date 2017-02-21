//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_CONNECTIONMANAGER_H
#define SERWERHTTP_CONNECTIONMANAGER_H

#include <netinet/in.h>


class ConnectionManager {
public:
    int descriptor;
    sockaddr_in socketStruct;
    socklen_t socketSize = 0;

    ConnectionManager() {};

    ConnectionManager(const char *host, int port);

protected:
    int port;
    const char *host;

    int createSocket();

    void createSockaddr();

};


#endif //SERWERHTTP_CONNECTIONMANAGER_H
