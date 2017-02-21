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

void FCGIManager::sendMessage(int clientSocketFd) {
    // TODO obsługa errorów
    std::cout << "\n***MESSAGE FROM FCGI TO CLIENT\n";
    unsigned int message_buf[100];
    ssize_t readBytes = 0;
    recv(descriptor, &message_buf, 8, 0);
    write(1, &answerHeader, sizeof(answerHeader)-1);
    write(clientSocketFd, &answerHeader, sizeof(answerHeader)-1);
    while ((readBytes = recv(descriptor, &message_buf, sizeof(message_buf), 0)) != 0) {
        try {
            //write(clientSocketFd, &message_buf, (size_t) readBytes);
            send(clientSocketFd, &message_buf,(size_t) readBytes, 0);
            write(1, &message_buf, (size_t) readBytes);
        }
        catch (std::exception &e) {
            std::cout << e.what();
            perror("Connection with client canceled.");
            break;
        }
    }
    std::cout << "\n***END OF MESSAGE FROM FCGI TO CLIENT\n";
}

FCGIManager::~FCGIManager() {}
