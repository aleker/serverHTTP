//
// Created by ola on 17.02.17.
//

#ifndef SERWERHTTP_FCGIMANAGER_H
#define SERWERHTTP_FCGIMANAGER_H


#include "ConnectionManager.h"

class FCGIManager : public ConnectionManager {
public:
    bool will_send_message = true;

    FCGIManager(const char *host, int port);


    int createConnection();

    virtual void sendMessage(int clientSocketFd);

    virtual ~FCGIManager();

private:
    int connectSocket();
};


#endif //SERWERHTTP_FCGIMANAGER_H
