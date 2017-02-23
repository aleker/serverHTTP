//
// Created by ola on 17.02.17.
//

#ifndef SERWERHTTP_HTTPMANAGER_H
#define SERWERHTTP_HTTPMANAGER_H


#include "ConnectionManager.h"
#include "FCGIManager.h"

class HTTPManager : public ConnectionManager {
public:
    int timeout;
    int role;

    HTTPManager(const char *host, int port);

    int prepareServerSocket();
    int getMessage(ConnectionManager *client, std::string* content_data) const;
    int sendMessage(FCGIManager* fcgi, std::string* message, ConnectionManager* client) const;

private:

    int bindSocket();
    int isWhaleMessage(std::string *content_data) const;
};


#endif //SERWERHTTP_HTTPMANAGER_H
