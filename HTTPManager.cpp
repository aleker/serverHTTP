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
#include <fcntl.h>

using namespace std;

HTTPManager::HTTPManager(const char *host, int port) : ConnectionManager(host, port) {}

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
    result = select(fd + 1, &readset, NULL, NULL, &tv);

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


int HTTPManager::bindSocket() {
    int err = bind(descriptor, (sockaddr *) &socketStruct, sizeof(socketStruct));
    if (err == -1) {
        perror("Error binding the socket!");
        return -1;
    }
    return 0;
}

int HTTPManager::prepareServerSocket() {
    if (createSocket() == -1) return -1;
    createSockaddr();
    if (bindSocket() == -1) return -1;
    return 0;
}

int HTTPManager::acceptConnection(ConnectionManager *client) const {
    int enable = 1;
    client->descriptor = accept(descriptor, (sockaddr *) &client->socketStruct, &client->socketSize);
    if (client->descriptor == -1) {
        perror("Error accepting client");
        return -1;
    }
    setsockopt(client->descriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT
    return 0;
}

int HTTPManager::isWhaleMessage(string *content_data) const {
    string content = *content_data;
    int found = (int) content.find("Content-Length:");
    if (found != -1) {
        string content_size;
        found += 16;
        while (isdigit(content[found])) {
            content_size.push_back(content[found]);
            found++;
        }
        int content_size2 = stoi(content_size);
        cout << "NO MASZ! Content_length powinien =  " << content_size2 << "\n";
        int found2 = (int) content.find("\n\r\n");
        if (found2 != -1 and found2 > found) {
            string content2 = content.substr((unsigned long) (found2 + 3), content.size() - found2 - 3);
            cout << "NO MASZ! Content_length od karotki ma " << (int) content2.length() << "\n";
            if ((int) content2.length() < content_size2) return -1;
        }
        return 0;
    }
    return 0;
}

int HTTPManager::getMessage(ConnectionManager *client, string *content_data) const {
    cout << "***STARTED READING FROM CLIENT TO HTTP\n";
    unsigned char *buffer = new unsigned char[100];
    ssize_t Len;
    int timeout;
    if (ConfigFile::getConfigFile().readTimeout(&timeout) == -1) return -1;
    while ((Len = recv_to(client->descriptor, buffer, 100, 0, timeout)) > 0) {
        for (int i = 0; i < Len; i++) {
            content_data->push_back(buffer[i]);
        }
    }
    delete[] buffer;
    cout << "\n***FINISHED READING FROM CLIENT TO HTTP\n";
    return isWhaleMessage(content_data);   // ;____;
}

void HTTPManager::sendMessage(ConnectionManager *receiver, string *message, int id) const {
    std::vector<Record> records;
    Parser parser;
    // PARSING MESSAGE
    parser.parseBrowserMessage(message);

    // CREATE RECORDS
    int request_id = id;
    int role;
    if (ConfigFile::getConfigFile().readRole(&role) == -1) return;
    parser.createRecords(&records, request_id, role);

    // SENDING RECORDS
    cout << "***STARTED READING FROM HTTP TO FCGI\n";
    for (Record &record: records) {
        sendto(receiver->descriptor, record.message, (size_t) record.array_size, 0,
               (sockaddr *) &(receiver->socketStruct), sizeof(receiver->socketStruct));
    }
    cout << "\n***FINISHED READING FROM HTTP TO FCGI\n";
}