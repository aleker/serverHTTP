//
// Created by tusia on 16.02.17.
//

#include "ConnectionManager.h"
#include "constants.h"
#include "Parser.h"
#include <unistd.h>
#include <arpa/inet.h>

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

//-----------------------------

int ConnectionManager::prepareServerSocket(){
    if (createSocket() == -1) return -1;
    createSockaddr();
    if(bindSocket() == -1) return -1;
    return 0;
}

int ConnectionManager::createConnection(){

    if (createSocket() == -1) return -1;
    createSockaddr();
    if (connectSocket() == -1) return -1;
    return 0;
}

int ConnectionManager::acceptConnection(ConnectionManager *client) {
    int enable = 1;
    client->descriptor = accept(descriptor, (sockaddr*)&client->socketStruct, &client->socketSize);
    if (client->descriptor == -1) {
        perror("Error accepting client");
        return -1;
    }
    setsockopt(client->descriptor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT
    return 0;
}

int ConnectionManager::getMessage(ConnectionManager* client, unsigned char* content_data) {
    ssize_t Len = read(client->descriptor, content_data, bufsize);
    content_data[Len] = 0;// make sure it's a proper string
    cout<< "\nWIADOMOŚĆ OD PRZEGLĄDARKI: " << Len << endl << content_data << "KONIEC WIADOMOŚCI\n\n" << endl;

    return 0;
}

void ConnectionManager::forwardMessage(int clientSocketFd) {    // TODO ma być send
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


void ConnectionManager::sendMessage(ConnectionManager* receiver, unsigned char* message, int message_size) {
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

