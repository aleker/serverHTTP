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

int HTTPManager::isWhaleMessage(string* content_data) const {
    string content = *content_data;
    int found = (int) content.find("\nContent-Length:");
    if (found != -1) {
        string content_size;
        found += 16;
        while (content[found] == ASCII_SPACE) found++;
        while (isdigit(content[found])) {
            content_size.push_back(content[found]);
            found++;
        }
        int content_size2 = stoi(content_size);
        int found2 = (int) content.find("\n\r\n");
        if (found2 != -1 and found2 > found) {
            string content2 = content.substr((unsigned long) (found2 + 3), content.size() - found2 - 3);
            if ((int)content2.length() < content_size2)
                return -1;
        }
        return 0;
    }
    return 0;
}

int HTTPManager::getMessage(ConnectionManager* client, string* content_data) const {
    cout << "\n***MESSAGE FROM CLIENT TO HTTP\n";
    unsigned char* buffer = new unsigned char[100];
    ssize_t Len;
    // TODO SIGHUP?
    // TODO zamiast resv_to -> setsockopt SO_RCVTIME0
    // TODO czytacie aż nie będzie timeouta; czy to nie spowoduje że odpowiecie
    // timeout milisekund po tym jak dotrze ostatni kawałek zapytania?
    while ((Len = recv_to(client->descriptor, buffer, 100, 0, timeout)) > 0) {
        for (int i = 0; i < Len; i++) {
            content_data->push_back(buffer[i]);
        }
    }
    if (content_data->length() == 0) {
        cout << "RESET!\n";
        return -1;
    }
    delete [] buffer;
    cout << "***END OF MESSAGE FROM CLIENT TO HTTP\n";
    if (Len == -1) return -1;
    return isWhaleMessage(content_data);   // ;____;
}

int HTTPManager::sendMessage(ConnectionManager* receiver, string* message, int id) const {
    std::vector<Record> records;
    Parser parser;
    // PARSING MESSAGE
    parser.parseBrowserMessage(message);

    // CREATE RECORDS
    parser.createRecords(&records, id, role);

    // SENDING RECORDS
    cout << "\n***MESSAGE FROM HTTP TO FCGI\n";
    for (Record &record: records) {
        try {
            sendto(receiver->descriptor, record.message, (size_t) record.array_size, 0,
                   (sockaddr *) &(receiver->socketStruct), sizeof(receiver->socketStruct));
        }
        catch (exception &e) {
            cout << e.what();
            perror("Error sending message to FCGI server.");
            return -1;
        }
    }
    cout << "***END OF MESSAGE FROM HTTP TO FCGI\n";
    return 0;
}