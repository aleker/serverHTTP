//
// Created by ola on 17.02.17.
//

#include <cstdio>
#include <unistd.h>
#include <vector>
#include <fstream>
#include "HTTPManager.h"
#include "constants.h"
#include "Record.h"
#include "Parser.h"
#include "ConfigFile.h"
#include <sys/select.h>
#include <fcntl.h>

using namespace std;

HTTPManager::HTTPManager(const char *host, int port) : ConnectionManager(host, port) {}

/*
   Params:
      fd       -  (int) socket file descriptor
      buffer - (char*) buffer to hold data
      len     - (int) maximum number of bytes to recv()
      flags   - (int) flags (as the fourth param to recv() )
      to       - (int) timeout in milliseconds
   Results:
      int      - The same as recv, but -2 == TIMEOUT
   Notes:
      You can only use it on file descriptors that are sockets!
      'to' must be different to 0
      'buffer' must not be NULL and must point to enough memory to hold at least 'len' bytes
      I WILL mix the C and C++ commenting styles...
*/
int recv_to(int fd, unsigned char *buffer, int len, int flags, int to) {

    fd_set readset;
    int result, iof = -1;
    struct timeval tv;

    // Initialize the set
    FD_ZERO(&readset);
    FD_SET(fd, &readset);

    // Initialize time out struct
    tv.tv_sec = 0;
    tv.tv_usec = to * 1000;
    // select()
    result = select(fd+1, &readset, NULL, NULL, &tv);

    // Check status
    if (result < 0)
        return -1;
    else if (result > 0 && FD_ISSET(fd, &readset)) {
        // Set non-blocking mode
        if ((iof = fcntl(fd, F_GETFL, 0)) != -1)
            fcntl(fd, F_SETFL, iof | O_NONBLOCK);
        // receive
        result = (int) recv(fd, buffer, (size_t) len, flags);
        // set as before
        if (iof != -1)
            fcntl(fd, F_SETFL, iof);
        return result;
    }
    return -2;
}



int HTTPManager::bindSocket(){
    int err = bind(descriptor, (sockaddr*)&socketStruct, sizeof(socketStruct));
    if (err == -1) {
        perror("Error binding the socket!");
        return -1;
    }
    return 0;
}

int HTTPManager::prepareServerSocket(){
    if (createSocket() == -1) return -1;
    createSockaddr();
    if(bindSocket() == -1) return -1;
    return 0;
}

int HTTPManager::acceptConnection(ConnectionManager *client) const {
    int enable = 1;
    client->descriptor = accept(descriptor, (sockaddr*)&client->socketStruct, &client->socketSize);
    if (client->descriptor == -1) {
        perror("Error accepting client");
        return -1;
    }
    setsockopt(client->descriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT
    return 0;
}

int HTTPManager::getMessage(ConnectionManager* client, string* content_data) const {
    cout << "***MESSAGE FROM CLIENT TO HTTP\n";
    unsigned char* buffer = new unsigned char[100];
    //ssize_t Len = read(client->descriptor, content_data, bufsize);
    //cout << content_data;
    // TODO timeout do configa
    ssize_t Len;
    int timeout;
    if (ConfigFile::getConfigFile().readTimeout(&timeout) == -1) return -1;
    while ((Len = recv_to(client->descriptor, buffer, 100, 0, timeout)) > 0) {
        for (int i = 0; i < Len; i++) {
            content_data->push_back(buffer[i]);
        }
    }
    cout << *content_data;
    cout << "\n***END OF MESSAGE FROM CLIENT TO HTTP\n";
    return 0;
}

void HTTPManager::sendMessage(ConnectionManager* receiver, string* message) const {
    // TODO upper boundary for message size
    std::vector<Record> records;
    Parser parser;
    // PARSING MESSAGE
    parser.parseBrowserMessage(message);

    // CREATE RECORDS
    int request_id = receiver->descriptor;                                           // TODO RANDOM ID
    int role;
    if (ConfigFile::getConfigFile().readRole(&role) == -1) return;
    parser.createRecords(&records, request_id, role);     // TODO rola

    // SENDING RECORDS
    cout << "***MESSAGE FROM HTTP TO FCGI\n";
    for (Record &record: records) {
        sendto(receiver->descriptor, record.message, (size_t )record.array_size, 0,
               (sockaddr*)&(receiver->socketStruct), sizeof(receiver->socketStruct));
        write(1, record.message, (size_t) record.array_size);
    }
    cout << "\n***END OF MESSAGE FROM HTTP TO FCGI\n";
}