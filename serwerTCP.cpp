#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <fastcgi.h>
#include "constants.h"
#include <fcgimisc.h>

#include "StreamRecord.h"
#include "BeginRecord.h"

using namespace std;

void fillHeader(unsigned char* record, int shift, int type, int request_id, int contentLength, int paddingLength) {
    record[VERSION + shift] = FCGI_VERSION_1;
    record[TYPE + shift] = (unsigned char) type;
    record[REQUEST_ID_B1 + shift] = (unsigned char) ((request_id >> 8) & 0xff);
    record[REQUEST_ID_B0 + shift] = (unsigned char) ((request_id) & 0xff);
    record[CONTENT_LENGTH_B1 + shift] = (unsigned char) ((contentLength >> 8) & 0xff);
    record[CONTENT_LENGTH_B0 + shift] = (unsigned char) ((contentLength) & 0xff);
    record[PADDING_LENGTH + shift] = (unsigned char) paddingLength;
    record[RESERVED + shift] = 0;
}

void fillContentData(unsigned char* record, int shift, unsigned char* content_data, int contentLength, int paddingLength) {
    for (int i = 0; i < contentLength; i++) {
        record[i + shift] = content_data[i];
    }
    for (int i = shift + contentLength; i < (shift + contentLength + paddingLength); i++) {
        record[i] = 0;
    }
}

void fillBeginRequestBody(unsigned char* record, int shift, int role, int flags) {
    record[ROLE_B1 + shift] = (unsigned char) ((role >> 8) & 0xff);
    record[ROLE_B0 + shift] = (unsigned char) ((role) & 0xff);
    record[FLAGS + shift] = (unsigned char) flags;
    for (int i = RESERVED_BEGIN + shift; i < BEGIN_REQUEST_BODY_SIZE + shift; i++) {
        record[i] = 0;
    }
}

sockaddr_in createFCGIConnection(int fd_fcgi) {
    // ----- CREATE SOCKET: -----
    //int fd_fcgi = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in fcgiSocket;
    fcgiSocket.sin_family = AF_INET;
    fcgiSocket.sin_port = htons(8000);       // Port serwera
    fcgiSocket.sin_addr.s_addr = inet_addr("127.0.0.1");
    int rc = connect(fd_fcgi, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
    if (rc == -1) {
        perror("Error connecting to socket");
        //return -1;
    }
    return fcgiSocket;
}

unsigned char* sendGET(int requestId, int contentLength, unsigned char* content_data, int fd_fcgi, sockaddr_in fcgiSocket) {
    // ----- SEND RECORDS: -----
    // TODO upper boundary for message size

    // BEGIN_REQUEST
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE + HEADER_SIZE, FCGI_BEGIN_REQUEST, requestId);
    beginRecord.fillHeader(0, BEGIN_REQUEST_BODY_SIZE);
    beginRecord.fillBeginRequestBody(HEADER_SIZE, FCGI_RESPONDER, 0);
    beginRecord.fillHeader(HEADER_SIZE+BEGIN_REQUEST_BODY_SIZE, 0);
    sendto(fd_fcgi, beginRecord.message, (size_t )beginRecord.array_size, 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

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


    // -----RECEIVE MESSAGE: -----
    unsigned char from_fcgi[100], *ptr;
    ptr = from_fcgi;
    ssize_t readBytes = 0;
    while ((readBytes = recv(fd_fcgi, from_fcgi, sizeof(from_fcgi), 0)) != 0) {
        write(1, from_fcgi, (size_t)readBytes);
    }
    cout << endl << "CAŁA WIADOMOŚĆ PRZESŁANA? " << endl;

    return ptr;
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
    // TODO RANDOM ID
    int id = 300;

    if (listen(fd, 10) == 0) {
        while (true) {
            // ACCEPTING CONNECTION WITH CLIENT:
            acceptedSocketFd = accept(fd, (sockaddr*)&acceptedSocket, &sizeOfAcceptedSockaddr);
            setsockopt(acceptedSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT

            // READING MESSAGE FROM CLIENT
            // TODO PARSOWANIE
            ssize_t Len = read(acceptedSocketFd, content_data, bufsize);
            content_data[Len] = 0;// make sure it's a proper string
            cout<< "\nWIADOMOŚĆ OD PRZEGLĄDARKI: " << sizeof(content_data) << endl << content_data << "KONIEC WIADOMOŚCI\n\n" << endl;

            // RUNNING FCGI:
            //--- http://stackoverflow.com/questions/26695738/nginx-fastcgi-without-using-spawn-fcgi
            int fd_fcgi = socket(PF_INET, SOCK_STREAM, 0);
            sockaddr_in sock = createFCGIConnection(fd_fcgi);
            unsigned  char *answer = sendGET(id, sizeof(content_data), content_data, fd_fcgi, sock);
            close(fd_fcgi);
            // SENDING ANSWER TO CLIENT
            // TODO POPRAWNY MESSAGE DO KLIENTA
            ssize_t sentBytes = write(acceptedSocketFd, answer, 100);
            close(acceptedSocketFd);
        }
    }
    close(fd);
}
// TODO ZRÓB KLASY

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB