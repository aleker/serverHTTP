//
// Created by ola on 17.02.17.
//

#include <cstdio>
#include <unistd.h>
#include "FCGIManager.h"
#include "constants.h"

FCGIManager::FCGIManager(const char *host, int port) : ConnectionManager(host, port) {}

int FCGIManager::connectSocket(){
    int rc = connect(descriptor, (sockaddr*)&socketStruct, sizeof(socketStruct));
    if (rc == -1) {
        perror("Error connecting to socket");
        return -1;
    }
    return 0;
}

int FCGIManager::createConnection(){

    if (createSocket() == -1) return -1;
    createSockaddr();
    if (connectSocket() == -1) return -1;
    return 0;
}

void FCGIManager::sendMessage(int clientSocketFd) {    // TODO ma być send
    // TODO obsługa errorów
    unsigned int message_buf[100];
    ssize_t readBytes = 0;
    recv(descriptor, &message_buf, 8, 0);
    //unsigned char header[] = "HTTP/1.1 200 OK\r\n";
    write(1, &answerHeader, sizeof(answerHeader));
    write(clientSocketFd, &answerHeader, sizeof(answerHeader));
    while ((readBytes = recv(descriptor, &message_buf, sizeof(message_buf), 0)) != 0) {
        write(1, &message_buf, (size_t)readBytes);
        write(clientSocketFd, &message_buf, (size_t)readBytes);
    }
}