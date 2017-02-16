//
// Created by tusia on 16.02.17.
//


#include <cstdio>
#include <arpa/inet.h>
#include <unistd.h>
#include "ConnectionManager.h"

ConnectionManager::ConnectionManager(const char *host,int port) : port(port), host(host) {}

int ConnectionManager::createSocket(){
    descriptor = socket(PF_INET, SOCK_STREAM, 0);
    if(descriptor == -1){
        perror("Error creating the socket!");
        return -1;
    }
    return 0;
}
int ConnectionManager::bindSocket(){
    int err = bind(descriptor, (sockaddr*)&socketStruct, sizeof(socketStruct));
    if (err == -1) {
        perror("Error binding the socket!");
        return -1;
    }
    return 0;
}

int ConnectionManager::connectSocket(){
    int rc = connect(descriptor, (sockaddr*)&socketStruct, sizeof(socketStruct));
    if (rc == -1) {
        perror("Error connecting to socket");
        return -1;
    }
    return 0;
}
void ConnectionManager::createSockaddr(){
    sockaddr_in fcgiSocket;
    fcgiSocket.sin_family = AF_INET;
    fcgiSocket.sin_port = htons(port);       // Port serwera
    fcgiSocket.sin_addr.s_addr = inet_addr(host);
    socketStruct = fcgiSocket;
}

int ConnectionManager::createFCGIConnection(){

    if (createSocket() == -1) return -1;
    createSockaddr();
    if (connectSocket() == -1) return -1;
    return 0;
}

int ConnectionManager::fullConnection(){
    if (createSocket() == -1) return -1;
    createSockaddr();
    if(bindSocket() == -1) return -1;
    return 0;
}

void ConnectionManager::forwardMessage(int clientSocketFd) {
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


