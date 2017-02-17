//
// Created by ola on 17.02.17.
//

#include <cstdio>
#include <unistd.h>
#include <vector>
#include "HTTPManager.h"
#include "constants.h"
#include "Record.h"
#include "Parser.h"

using namespace std;

HTTPManager::HTTPManager(const char *host, int port) : ConnectionManager(host, port) {}

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

int HTTPManager::acceptConnection(ConnectionManager *client) {
    int enable = 1;
    client->descriptor = accept(descriptor, (sockaddr*)&client->socketStruct, &client->socketSize);
    if (client->descriptor == -1) {
        perror("Error accepting client");
        return -1;
    }
    setsockopt(client->descriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT
    return 0;
}

int HTTPManager::getMessage(ConnectionManager* client, unsigned char* content_data) {
    ssize_t Len = read(client->descriptor, content_data, bufsize);
    content_data[Len] = 0;// make sure it's a proper string
    cout<< "\nWIADOMOŚĆ OD PRZEGLĄDARKI: " << Len << endl << content_data << "KONIEC WIADOMOŚCI\n\n" << endl;

    return 0;
}

void HTTPManager::sendMessage(ConnectionManager* receiver, unsigned char* message, int message_size) {
    std::vector<Record> records;
    Parser parser;
    parser.parseBrowserMessage(message);

    //------------------------------------------------------
    // ----- SEND RECORDS: -----
    // TODO RANDOM ID
    int requestId = 300;
    // TODO upper boundary for message size
    sockaddr_in fcgiSocket = receiver->socketStruct;
    int fd_fcgi = receiver->descriptor;
    // BEGIN_REQUEST
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE + HEADER_SIZE, FCGI_BEGIN_REQUEST, requestId);
    beginRecord.fillHeader(0, BEGIN_REQUEST_BODY_SIZE);
    beginRecord.fillBeginRequestBody(HEADER_SIZE, FCGI_RESPONDER, 0);
    beginRecord.fillHeader(HEADER_SIZE+BEGIN_REQUEST_BODY_SIZE, 0);
    sendto(fd_fcgi, beginRecord.message, (size_t )beginRecord.array_size, 0, (sockaddr*)&(fcgiSocket), sizeof(fcgiSocket));

    // FCGI_PARAMS
    StreamRecord paramRecord(HEADER_SIZE + message_size + (8 - message_size%8)%8 + HEADER_SIZE, FCGI_PARAMS, requestId);
    paramRecord.fillHeader(0, message_size);
    paramRecord.fillContentData(HEADER_SIZE, message, message_size);
    paramRecord.fillHeader(HEADER_SIZE + message_size + (8 - message_size%8)%8, 0);
    sendto(fd_fcgi, paramRecord.message, (size_t)paramRecord.array_size, 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    // SEND DATA3
    StreamRecord stdinRecord(HEADER_SIZE, FCGI_STDIN, requestId);
    stdinRecord.fillHeader(0, 0);
    sendto(fd_fcgi, stdinRecord.message, (size_t)stdinRecord.array_size, 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

}