#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <fastcgi.h>
#include <fcgimisc.h>
#include "constants.h"

using namespace std;

void fillHeader(unsigned char* record, int type, int request_id, int contentLength, int paddingLength) {
    record[VERSION] = FCGI_VERSION_1;
    record[TYPE] = (unsigned char) type;
    record[REQUEST_ID_B1] = (unsigned char) ((request_id >> 8) & 0xff);
    record[REQUEST_ID_B0] = (unsigned char) ((request_id) & 0xff);
    record[CONTENT_LENGTH_B1] = (unsigned char) ((contentLength >> 8) & 0xff);
    record[CONTENT_LENGTH_B0] = (unsigned char) ((contentLength) & 0xff);
    record[PADDING_LENGTH] = (unsigned char) paddingLength;
    record[RESERVED] = 0;
}

void fillContentData(unsigned char* record, unsigned char* content_data, int contentLength, int paddingLength) {
    for (int i = 0; i < contentLength; i++) {
        record[i + HEADER_SIZE] = content_data[i];
    }
    for (int i = HEADER_SIZE+contentLength; i < (HEADER_SIZE + contentLength + paddingLength); i++) {
        record[i] = 0;
    }
}

void fillBeginRequestBody(unsigned char* record, int role, int flags) {
    record[HEADER_SIZE + ROLE_B1] = (unsigned char) ((role >> 8) & 0xff);
    record[HEADER_SIZE + ROLE_B0] = (unsigned char) ((role) & 0xff);
    record[HEADER_SIZE + FLAGS] = (unsigned char) flags;
    for (int i = HEADER_SIZE+RESERVED_BEGIN; i < HEADER_SIZE+BEGIN_REQUEST_BODY_SIZE; i++) {
        record[i] = 0;
    }
}

void createBeginMessage(unsigned char* record, int requestId, int role) {
    fillHeader(record, FCGI_BEGIN_REQUEST, requestId, BEGIN_REQUEST_BODY_SIZE, 0);
    fillBeginRequestBody(record, role, 0);
}

int connectToFCGI(int requestId, int contentLength, unsigned char* content_data) {
    cout << "---Wykonuję connectToFCGI3---\n";

    int fd_fcgi = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in fcgiSocket;
    fcgiSocket.sin_family = AF_INET;
    fcgiSocket.sin_port = htons(8000);       // Port serwera
    fcgiSocket.sin_addr.s_addr = inet_addr("127.0.0.1");
    int rc = connect(fd_fcgi, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
    if (rc == -1) {
        perror("Error connecting to socket");
        return -1;
    }
    // --------------------------------------------------
    // TODO upper boundary for message size
    // TODO sending two messages
    int paddingLength = (8 - contentLength%8)%8;
    // SEND BEGIN MESSAGE
    unsigned char record[HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE];
    createBeginMessage(record, requestId, FCGI_RESPONDER);
    cout << "ROZMIAR REKORDU B: " << sizeof(record) << endl;
    sendto(fd_fcgi, &record, sizeof(record), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
    free(record);

    // SEND DATA
    unsigned char record2[HEADER_SIZE + contentLength + paddingLength];
    fillHeader(record2, FCGI_PARAMS, requestId, contentLength, paddingLength);
    fillContentData(record2, content_data, contentLength, paddingLength);
    cout << "ROZMIAR REKORDU P: " << sizeof(record2) << endl;
    sendto(fd_fcgi, &record2, sizeof(record2), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

//    // SEND DATA2
//    unsigned char content_data2[] = {'a','b','c','d','e','f','g','h'};
//    unsigned char record3[HEADER_SIZE + 8];
//    fillHeader(record3, FCGI_PARAMS, requestId, 8, 0);
//    fillContentData(record3, content_data2, 8, 0);
//    cout << "ROZMIAR REKORDU P: " << sizeof(record3) << endl;
//    sendto(fd_fcgi, &record3, sizeof(record3), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    // SEND DATA3
    unsigned char record4[HEADER_SIZE];
    fillHeader(record4, FCGI_PARAMS, requestId, 0, 0);
    cout << "ROZMIAR REKORDU P: " << sizeof(record4) << endl;
    sendto(fd_fcgi, &record4, sizeof(record4), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    // SEND DATA3
    unsigned char record5[HEADER_SIZE];
    fillHeader(record2, FCGI_STDIN, requestId, 0, 0);
    cout << "ROZMIAR REKORDU S: " << sizeof(record5) << endl;
    sendto(fd_fcgi, &record5, sizeof(record5), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    cout << "---Po sendto. Wychodzę z connectToFCGI---\n";
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Error creating socket");
        return -1;
    }

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    bind(fd, (sockaddr*)&server, sizeof(server));

    sockaddr_in acceptedSocket;
    int acceptedSocketFd;
    socklen_t sizeOfAcceptedSockaddr = 0;
    int enable = 1;

    unsigned char content_data[bufsize];
    int id = 300;

    if (listen(fd, 10) == 0) {
        while (true) {
            // ACCEPTING CONNECTION WITH CLIENT:
            acceptedSocketFd = accept(fd, (sockaddr*)&acceptedSocket, &sizeOfAcceptedSockaddr);
            setsockopt(acceptedSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT

            // READING MESSAGE FROM CLIENT
            ssize_t Len = read(acceptedSocketFd, content_data, bufsize);
            content_data[Len] = 0;// make sure it's a proper string
            cout<< "\nWIADOMOŚĆ OD PRZEGLĄDARKI: " << sizeof(content_data) << endl << content_data << "KONIEC WIADOMOŚCI\n\n" << endl;

            // RUNNING FCGI:
            //--- http://stackoverflow.com/questions/26695738/nginx-fastcgi-without-using-spawn-fcgi
            connectToFCGI(id, sizeof(content_data), content_data);
            // SENDING ANSWER TO CLIENT
            // ssize_t sentBytes = write(acceptedSocketFd, &buf, sizeof(buf));
            // cout << "Sent bytes: " << sentBytes << endl;
            close(acceptedSocketFd);
        }
    }
    close(fd);
}

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB