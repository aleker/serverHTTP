//
// Created by ola on 17.02.17.
//

#include <cstdio>
#include <unistd.h>
#include <string>
#include <cstring>
#include "FCGIManager.h"
#include "constants.h"

FCGIManager::FCGIManager(const char *host, int port) : ConnectionManager(host, port) {}

int FCGIManager::connectSocket() {
    int rc = connect(descriptor, (sockaddr *) &socketStruct, sizeof(socketStruct));
    if (rc == -1) {
        perror("Error connecting to socket");
        return -1;
    }
    return 0;
}

int FCGIManager::createConnection() {
    if (createSocket() == -1) return -1;
    createSockaddr();
    if (connectSocket() == -1) return -1;
    return 0;
}


void FCGIManager::sendMessage(int clientSocketFd) {
    std::cout << "\n***MESSAGE FROM FCGI TO CLIENT\n";
    unsigned char message_buf[8];
    ssize_t readBytes = 0;
    unsigned char end_header[8];
    end_header[0] = FCGI_VERSION;
    end_header[1] = FCGI_END_REQUEST;
    end_header[4] = ZERO;      // content length1
    end_header[5] = 8;      // content length0
    end_header[6] = ZERO;      // padding length
    end_header[7] = ZERO;      // reserved
    // Copying the two bytes of requestId
    recv(descriptor, &message_buf, 8, 0);
    end_header[2] = message_buf[2];
    end_header[3] = message_buf[3];

    bool stop_reading = false;
    while (1) {
        // Reading 8 bytes at a time
        readBytes = recv(descriptor, &message_buf, sizeof(message_buf), 0);
        // Check if these 8 bytes are equal to end_header
        if (strcmp((const char *) message_buf, (const char *) end_header) == 0) {
            stop_reading = true;
        } else {
            // Not an end header - either last 8 bytes or content
            if(stop_reading) {
                // get the appStatus here
                break;
            }
            try {
                send(clientSocketFd, &message_buf, (size_t) readBytes, MSG_NOSIGNAL);
            }
            catch (std::exception &e) {
                std::cout << e.what();
                perror("Connection with client canceled.");
                return;
            }
        }

    }
    std::cout << "***END OF MESSAGE FROM FCGI TO CLIENT\n";

//    // TODO sparsować wiadomość od FCGI
//    // TODO czytać do FCGI_END_REQUEST
//    std::cout << "\n***MESSAGE FROM FCGI TO CLIENT\n";
//    unsigned char message_buf[8];
//    ssize_t readBytes = 0;
//    std::string message_from_fcgi;
//    while ((readBytes = recv(descriptor, &message_buf, sizeof(message_buf), 0)) != 0) {
//        std::string received(message_buf, message_buf + readBytes);
//        message_from_fcgi.append(received);
//    }
//    std::cout << "***END OF MESSAGE FROM FCGI TO CLIENT\n";
}

FCGIManager::~FCGIManager() {}
