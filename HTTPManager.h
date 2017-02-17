//
// Created by ola on 17.02.17.
//

#ifndef SERWERHTTP_HTTPMANAGER_H
#define SERWERHTTP_HTTPMANAGER_H


#include "ConnectionManager.h"

class HTTPManager : public ConnectionManager {
public:

    HTTPManager(const char *host, int port);

    int prepareServerSocket();
    int acceptConnection(ConnectionManager *client);
    int getMessage(ConnectionManager *client, unsigned char *content_data);
    void sendMessage(ConnectionManager *receiver, unsigned char *message);

private:
    int bindSocket();
};


#endif //SERWERHTTP_HTTPMANAGER_H
