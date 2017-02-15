#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <fastcgi.h>
#include <fcgimisc.h>


using namespace std;

ssize_t bufsize = 1000;

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

struct Record_fcgi {
    FCGI_Header header;
    //unsigned char *contentData;
    //unsigned char *paddingData;
    //unsigned char contentData[];
    //unsigned char paddingData[];
    string contentData;
    string paddingData;
};

unsigned char * serialize_char(unsigned char *buffer, char value)
{
    buffer[0] = (unsigned char) value;
    return buffer + 1;
}

static Record_fcgi MakeRecord(FCGI_Header header, string data) {
    int contentLength = header.contentLengthB0 | header.contentLengthB1 << 8;
    int paddingLength = header.paddingLength;
    //cout << "paddingLength " << paddingLength << " contentLength " << contentLength << endl;
    // TODO dynamic array allocation

    Record_fcgi record;
    record.header = header;
    //record.contentData = new unsigned char[contentLength];
    //record.paddingData = new unsigned char[paddingLength];
    //record.contentData = (unsigned char *)malloc(sizeof(char) * contentLength);
    //record.paddingData = (unsigned char *)malloc(sizeof(char) * paddingLength);

//    for (int i=0 ; i < contentLength; i++) {
//        record.contentData[i] = (unsigned char)data[i];
//    }
    record.contentData = data;
    for (int i = 0; i < paddingLength; i++) {
        record.paddingData[i] = '0';
    }


    //delete [] record.contentData;
    //delete [] record.paddingData;

    return record;
}

int connectToFCGI3() {
    cout << "---Wykonuję connectToFCGI3---\n";
    int contentLength = 37;
    int paddingLength = 3;
    //FCGI_Header header = MakeHeader(FCGI_BEGIN_REQUEST, 300, contentLength, paddingLength);
    string message = "Marta jest super ekstra i Ola tez!11!";
    //Record_fcgi record = MakeRecord(header, message);
    unsigned char record[8 + contentLength + paddingLength];
    record[0] = FCGI_VERSION_1;
    record[1] = (unsigned char) FCGI_BEGIN_REQUEST;
    record[2] = (unsigned char) ((300 >> 8) & 0xff);
    record[3] = (unsigned char) ((300) & 0xff);
    record[4] = (unsigned char) ((contentLength >> 8) & 0xff);
    record[5] = (unsigned char) ((contentLength) & 0xff);
    record[6] = (unsigned char) paddingLength;
    record[7] = 0;
    for (int i = 8; i < 8+contentLength; i++) {
        record[i] = (unsigned char)message[i];
    }
    for (int i = 8+contentLength; i < sizeof(record); i++) {
        record[i] = '0';
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
    cout << "ROZMIAR REKORDU: " << sizeof(record) << endl;
    //sendto(fd_fcgi, &header, sizeof(header), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
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
            connectToFCGI3();
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