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
    socklen_t socketSize = 0;   // TODO co z tym sajzem?

    ConnectionManager(){};
    ConnectionManager(const char *host, int port);

    int prepareServerSocket();
    int createConnection();
    int acceptConnection(ConnectionManager *client);
    void forwardMessage(int clientSocketFd);
    int getMessage(ConnectionManager *client, unsigned char *message);
    void sendMessage(ConnectionManager *receiver, unsigned char *message, int message_size);


private:
    int port;
    const char* host;

    int createSocket();
    void createSockaddr();
    int connectSocket();
    int bindSocket();


};


#endif //SERWERHTTP_CONNECTIONMANAGER_H
