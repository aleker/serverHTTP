#include "constants.h"
#include "StreamRecord.h"
#include "BeginRecord.h"
#include "ConnectionManager.h"
#include "Parser.h"
#include <algorithm>
#include <unistd.h>
#include <iostream>


using namespace std;

void sendGET(int requestId, int contentLength, unsigned char* content_data, ConnectionManager* connection) {
    // ----- SEND RECORDS: -----
    // TODO upper boundary for message size
    sockaddr_in fcgiSocket = connection->socketStruct;
    int fd_fcgi = connection->descriptor;
    // BEGIN_REQUEST
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE + HEADER_SIZE, FCGI_BEGIN_REQUEST, requestId);
    beginRecord.fillHeader(0, BEGIN_REQUEST_BODY_SIZE);
    beginRecord.fillBeginRequestBody(HEADER_SIZE, FCGI_RESPONDER, 0);
    beginRecord.fillHeader(HEADER_SIZE+BEGIN_REQUEST_BODY_SIZE, 0);
    sendto(fd_fcgi, beginRecord.message, (size_t )beginRecord.array_size, 0, (sockaddr*)&(fcgiSocket), sizeof(fcgiSocket));

    // FCGI_PARAMS
    StreamRecord paramRecord(HEADER_SIZE + contentLength + (8 - contentLength%8)%8 + HEADER_SIZE, FCGI_PARAMS, requestId);
    paramRecord.fillHeader(0, contentLength);
    paramRecord.fillContentData(HEADER_SIZE, content_data, contentLength);
    paramRecord.fillHeader(HEADER_SIZE + contentLength + (8 - contentLength%8)%8, 0);
    sendto(fd_fcgi, paramRecord.message, (size_t)paramRecord.array_size, 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    // SEND DATA3
    StreamRecord stdinRecord(HEADER_SIZE, FCGI_STDIN, requestId);
    stdinRecord.fillHeader(0, 0);
    sendto(fd_fcgi, stdinRecord.message, (size_t)stdinRecord.array_size, 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    ConnectionManager serverMainConnection(argv[1],atoi(argv[2]));
    serverMainConnection.fullConnection();      // prepare server socket

    sockaddr_in acceptedSocket;
    int acceptedSocketFd;
    socklen_t sizeOfAcceptedSockaddr = 0;
    int enable = 1;

    unsigned char content_data[bufsize];
    // TODO RANDOM ID
    int id = 300;

    if (listen(serverMainConnection.descriptor, 10) == 0) {
        while (true) {
            // ACCEPTING CONNECTION WITH CLIENT:
            acceptedSocketFd = accept(serverMainConnection.descriptor, (sockaddr*)&acceptedSocket, &sizeOfAcceptedSockaddr);
            setsockopt(acceptedSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT

            // READING MESSAGE FROM CLIENT
            // TODO PARSOWANIE
            ssize_t Len = read(acceptedSocketFd, content_data, bufsize);
            content_data[Len] = 0;// make sure it's a proper string
            cout<< "\nWIADOMOŚĆ OD PRZEGLĄDARKI: " << sizeof(content_data) << endl << content_data << "KONIEC WIADOMOŚCI\n\n" << endl;

            // FCGI CONNECTION:
            // TODO parametry 127.0.0.1 8000 w pliku konfiguracyjnym
            ConnectionManager fcgiConnection("127.0.0.1", 8000);
            fcgiConnection.createFCGIConnection();
            sendGET(id, sizeof(content_data), content_data, &fcgiConnection);
            // get message from fcgi and send it to client:

            unsigned char* ptr;
            ptr = content_data;
            Parser parser;
            parser.parseBrowserMessage(ptr);
            fcgiConnection.forwardMessage(acceptedSocketFd);
            close(fcgiConnection.descriptor);

            close(acceptedSocketFd);
        }
    }
    close(serverMainConnection.descriptor);
}
// TODO ZRÓB KLASY

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB