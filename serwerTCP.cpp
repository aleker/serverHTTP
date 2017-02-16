#include "constants.h"
#include "StreamRecord.h"
#include "BeginRecord.h"
#include "ConnectionManager.h"

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

unsigned char* sendGET(int requestId, int contentLength, unsigned char* content_data, int fd_fcgi, sockaddr_in fcgiSocket) {
    // ----- SEND RECORDS: -----
    // TODO upper boundary for message size
    int paddingLength = (8 - contentLength%8)%8;
    // BEGIN_REQUEST
//    unsigned char record[HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE + HEADER_SIZE];
//    fillHeader(record, 0, FCGI_BEGIN_REQUEST, requestId, BEGIN_REQUEST_BODY_SIZE, 0);
//    fillBeginRequestBody(record, HEADER_SIZE + 0, FCGI_RESPONDER, 0);
//    fillHeader(record, HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE, FCGI_BEGIN_REQUEST, requestId, BEGIN_REQUEST_BODY_SIZE, 0);
//    sendto(fd_fcgi, &record, sizeof(record), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE + HEADER_SIZE, FCGI_BEGIN_REQUEST, requestId, contentLength);
    beginRecord.fillHeader(0);
    beginRecord.fillBeginRequestBody(HEADER_SIZE, FCGI_RESPONDER, 0);
    beginRecord.fillHeader(HEADER_SIZE+BEGIN_REQUEST_BODY_SIZE);
    sendto(fd_fcgi, beginRecord.message, (size_t )beginRecord.array_size, 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    // FCGI_PARAMS
    unsigned char record2[HEADER_SIZE + contentLength + paddingLength + HEADER_SIZE];
    fillHeader(record2, 0, FCGI_PARAMS, requestId, contentLength, paddingLength);
    fillContentData(record2, HEADER_SIZE, content_data, contentLength, paddingLength);
    fillHeader(record2, HEADER_SIZE + contentLength + paddingLength, FCGI_PARAMS, requestId, 0, 0);
    sendto(fd_fcgi, &record2, sizeof(record2), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));

    // SEND DATA3
    unsigned char record3[HEADER_SIZE];
    fillHeader(record2, 0, FCGI_STDIN, requestId, 0, 0);
    sendto(fd_fcgi, &record3, sizeof(record3), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));



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

    ConnectionManager serverMainConnection(argv[1],atoi(argv[2]));
    serverMainConnection.fullConnection();

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

            // RUNNING FCGI:
            ConnectionManager fcgiConnection("127.0.0.1", 8000);
            fcgiConnection.createFCGIConnection();
            unsigned  char *answer = sendGET(id, sizeof(content_data), content_data, fcgiConnection.descriptor, fcgiConnection.socketStruct);
            close(fcgiConnection.descriptor);
            // SENDING ANSWER TO CLIENT
            // TODO POPRAWNY MESSAGE DO KLIENTA
            ssize_t sentBytes = write(acceptedSocketFd, answer, 100);
            close(acceptedSocketFd);
        }
    }
    close(serverMainConnection.descriptor);
}
// TODO ZRÓB KLASY

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB