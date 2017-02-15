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

int connectToFCGI3(int type, int requestId, int contentLength) {
    cout << "---Wykonuję connectToFCGI3---\n";
    int paddingLength = 8 - contentLength%8;
    string message = "Marta jest super ekstra i Ola tez!11!";

    // TODO upper boundary for message size
    unsigned char record[HEADER_SIZE + contentLength + paddingLength];
    fillHeader(record, type, requestId, contentLength, paddingLength);

    for (int i = 0; i < contentLength; i++) {
        record[i + HEADER_SIZE] = (unsigned char)message[i];
    }
    for (int i = HEADER_SIZE+contentLength; i < sizeof(record); i++) {
        record[i] = 0;
    }

    // TODO sending two messages
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
    //cout << "ROZMIAR REKORDU: " << sizeof(record) << endl;
    sendto(fd_fcgi, &record, sizeof(record), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
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

    int RecvMsgSize = 0;
    int Len;
    char buffer[bufsize];

    if (listen(fd, 10) == 0) {
        while (true) {
            // ACCEPTING CONNECTION WITH CLIENT:
            acceptedSocketFd = accept(fd, (sockaddr*)&acceptedSocket, &sizeOfAcceptedSockaddr);
            setsockopt(acceptedSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); // There will be no TIME_WAIT

            // READING MESSAGE FROM CLIENT
            Len = read(acceptedSocketFd, buffer, bufsize);
            buffer[Len] = 0;// make sure it's a proper string
            cout<< "\nWIADOMOŚĆ OD PRZEGLĄDARKI\n" << buffer << "KONIEC WIADOMOŚCI\n\n" << endl;

            // RUNNING FCGI:
            //--- http://stackoverflow.com/questions/26695738/nginx-fastcgi-without-using-spawn-fcgi
            connectToFCGI3(FCGI_BEGIN_REQUEST, 300, 37);
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