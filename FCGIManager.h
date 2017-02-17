//
// Created by ola on 17.02.17.
//

#ifndef SERWERHTTP_FCGIMANAGER_H
#define SERWERHTTP_FCGIMANAGER_H


#include "ConnectionManager.h"

class FCGIManager : public ConnectionManager {
public:
    FCGIManager(const char *host, int port);

    int createConnection();
    virtual void sendMessage(int clientSocketFd);

private:
    int connectSocket();
};


#endif //SERWERHTTP_FCGIMANAGER_H