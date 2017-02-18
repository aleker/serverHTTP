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
    cout<< "\nWIADOMOŚĆ OD PRZEGLĄDARKI (DŁ): " << Len << endl << content_data << "KONIEC WIADOMOŚCI\n\n" << endl;

    return 0;
}

void HTTPManager::sendMessage(ConnectionManager* receiver, unsigned char* message) {
    // TODO upper boundary for message size
    std::vector<Record> records;
    Parser parser;
    // PARSING MESSAGE
    parser.parseBrowserMessage(message);

    // CREATE RECORDS
    int request_id = 1;                                           // TODO RANDOM ID
    parser.createRecords(&records, request_id, FCGI_RESPONDER);     // TODO rola

    std::ofstream outfile ("dear_fcgi.txt",std::ofstream::binary);
    // SENDING RECORDS
    for (Record &record: records) {
        sendto(receiver->descriptor, record.message, (size_t )record.array_size, 0,
               (sockaddr*)&(receiver->socketStruct), sizeof(receiver->socketStruct));
        //write(1, record.message, record.array_size);
        outfile.write((const char *) record.message, record.array_size);
    }
    outfile.close();
}