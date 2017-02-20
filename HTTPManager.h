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
    int acceptConnection(ConnectionManager *client) const;
    int getMessage(ConnectionManager *client, std::string* content_data) const;
    void sendMessage(ConnectionManager *receiver, std::string *message, int id) const;

private:
    int bindSocket();
};


#endif //SERWERHTTP_HTTPMANAGER_H
