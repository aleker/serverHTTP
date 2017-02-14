#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcgi_stdio.h>
#include <fastcgi.h>
#include <fcgimisc.h>
#include <cerrno>
#include <cstdio>
#include <cassert>
#include "fcgio.h"

using namespace std;

#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

/*
version: Identifies the FastCGI protocol version. This specification documents FCGI_VERSION_1.
type: Identifies the FastCGI record type, i.e. the general function that the record performs. Specific record types and their functions are detailed in later sections.
requestId: Identifies the FastCGI request to which the record belongs.
contentLength: The number of bytes in the contentData component of the record.
paddingLength: The number of bytes in the paddingData component of the record.
contentData: Between 0 and 65535 bytes of data, interpreted according to the record type.
paddingData: Between 0 and 255 bytes of data, which are ignored.

{FCGI_END_REQUEST, 1, {FCGI_REQUEST_COMPLETE,0}}:
    type == FCGI_END_REQUEST,
    requestId == 1,
    contentData == {FCGI_REQUEST_COMPLETE,0}
*/



static FCGI_Header MakeHeader(
        int type,
        int requestId,
        int contentLength,
        int paddingLength) {
    FCGI_Header header;
    ASSERT(contentLength >= 0 && contentLength <= FCGI_MAX_LENGTH);
    ASSERT(paddingLength >= 0 && paddingLength <= 0xff);
    header.version = FCGI_VERSION_1;
    header.type = (unsigned char) type;
    header.requestIdB1 = (unsigned char) ((requestId >> 8) & 0xff);
    header.requestIdB0 = (unsigned char) ((requestId) & 0xff);
    header.contentLengthB1 = (unsigned char) ((contentLength >> 8) & 0xff);
    header.contentLengthB0 = (unsigned char) ((contentLength) & 0xff);
    header.paddingLength = (unsigned char) paddingLength;
    header.reserved = 0;
    return header;
}

int connectToFCGI3() {
    cout << "Wykonuję connectToFCGI3\n";
    FCGI_Header header = MakeHeader(FCGI_BEGIN_REQUEST, 300, 0, 0);

    int fd_fcgi = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in fcgiSocket;
    fcgiSocket.sin_family = AF_INET;
    fcgiSocket.sin_port = htons(8000);       // Port serwera
    fcgiSocket.sin_addr.s_addr = inet_addr("127.0.0.1");
    int rc = connect(fd_fcgi, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
//    if (rc == -1) {
//        perror("Error connecting to socket");
//        return -1;
//    }
    sendto(fd_fcgi, &header, sizeof(header), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
    cout << "Po sendto\n";

}

//const int contentLength = 5;
//const int paddingLength = 3;
//
//typedef struct {
//    unsigned char version;
//    unsigned char type;
//    unsigned char requestIdB1;
//    unsigned char requestIdB0;
//    unsigned char contentLengthB1;
//    unsigned char contentLengthB0;
//    unsigned char paddingLength;
//    unsigned char reserved;
//    unsigned char contentData[contentLength];
//    unsigned char paddingData[paddingLength];
//} FCGI_Record;
//unsigned char * serialize_int(unsigned char *buffer, int value)
//{
//    /* Write big-endian int value into buffer; assumes 32-bit int and 8-bit char. */
//    buffer[0] = value >> 24;
//    buffer[1] = value >> 16;
//    buffer[2] = value >> 8;
//    buffer[3] = value;
//    return buffer + 4;
//}
//
//unsigned char * serialize_char(unsigned char *buffer, char value)
//{
//    buffer[0] = value;
//    return buffer + 1;
//}
//
//int connectToFCGI2() {
//    FCGI_Record message;
//    message.version = 1;                // 1B
//    message.type = FCGI_BEGIN_REQUEST;  // 1B
//    message.requestIdB1 = 0;            // 1B
//    message.requestIdB0 = 1;            // 1B
//    message.contentLengthB1 = 0;        // 1B
//    message.contentLengthB0 = 5;        // 1B
//    message.paddingLength = 3;          // 1B
//    message.reserved = 0;               // 1B
//    message.contentData = "Hello";      // 5B
//    message.paddingData = "000";        // 3B
//
//    unsigned char buffer[32], *ptr;
//    buffer = serialize_char(buffer, message.version);
//    buffer = serialize_char(buffer, message.type);
//    buffer = serialize_char(buffer, message.requestIdB1);
//    buffer = serialize_char(buffer, message.requestIdB0);
//    buffer = serialize_char(buffer, message.contentLengthB1);
//    buffer = serialize_char(buffer, message.contentLengthB0);
//    buffer = serialize_char(buffer, message.paddingLength);
//    buffer = serialize_char(buffer, message.reserved);
//    for (int i = 0; i < message.contentLengthB0; i ++)
//        buffer = serialize_char(buffer, message.contentData[i]);
//    for (int i = 0; i < message.paddingLength; i ++)
//        buffer = serialize_char(buffer, message.paddingData[i]);
//
//}


int connectToFCGI() {
    cout << "Tworzę połączenie z fcgi" << endl;
    int fd_fcgi = socket(PF_INET, SOCK_STREAM, 0);
    char buf[1000] = {'a','b','c', 'd'};
    sockaddr_in fcgiSocket;
    fcgiSocket.sin_family = AF_INET;
    fcgiSocket.sin_port = htons(8000);       // Port serwera
    fcgiSocket.sin_addr.s_addr = inet_addr("127.0.0.1");

    int rc = connect(fd_fcgi, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
//    if (rc == -1) {
//        perror("Error connecting to socket");
//        return -1;
//    }
    cout << "Po connect\n";
    write(fd_fcgi, buf, 1000);
    cout << "Wysłałem wiadomość\n";

    ssize_t readBytes = 0;
    while ((readBytes = recv(fd_fcgi, buf, sizeof(buf), 0)) != 0) {
        cout << "Jestem w recv\n";
        write(STDOUT_FILENO, buf, readBytes);
    }

}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    char buf[8] = "Hello\n";

    int fd = socket(PF_INET, SOCK_STREAM, 0);
//    if (fd == -1) {
//        perror("Error creating socket");
//        return -1;
//    }
    cout << "fd = " << fd << "\n";

    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);
    bind(fd, (sockaddr*)&server, sizeof(server));

    sockaddr_in acceptedSocket;
    int acceptedSocketFd;
    socklen_t sizeOfAcceptedSockaddr = 0;
    int enable = 1;

    if (listen(fd, 10) == 0) {
        while (true) {
            // ACCEPTING CONNECTION WITH CLIENT:
            // accept() - it fills acceptedConnection and sizeOfAcceptedSockaddr
            acceptedSocketFd = accept(fd, (sockaddr*)&acceptedSocket, &sizeOfAcceptedSockaddr);
            // There will be no TIME_WAIT
            setsockopt(acceptedSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

            // RUNNING FCGI:
            //--- http://stackoverflow.com/questions/26695738/nginx-fastcgi-without-using-spawn-fcgi
            connectToFCGI3();
            cout<< "Wyszedłem z connectToFCGI\n";
            // SENDING ANSWER TO CLIENT
            // ssize_t sentBytes = write(acceptedSocketFd, &buf, sizeof(buf));
            // cout << "Sent bytes: " << sentBytes << endl;
            close(acceptedSocketFd);
        }
    }
    close(fd);
}