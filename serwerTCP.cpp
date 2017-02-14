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
    unsigned char contentData[37];
    unsigned char paddingData[3];
};

unsigned char * serialize_char(unsigned char *buffer, char value)
{
    buffer[0] = (unsigned char) value;
    return buffer + 1;
}

static Record_fcgi MakeRecord(FCGI_Header header, char* data) {
    int contentLength = header.contentLengthB0 | header.contentLengthB1 << 8;
    int paddingLength = header.paddingLength;
    //cout << "paddingLength " << paddingLength << " contentLength " << contentLength << endl;
    // TODO dynamic array allocation
    Record_fcgi record;
    record.header = header;
    unsigned char buf_data[contentLength] = "Marta jest super ekstra i Ola tez!11!";
    for (int i=0 ; i < 37; i++) {
        record.contentData[i] = buf_data[i];
    }
    unsigned char buf[paddingLength] = {0};
    for (int i=0 ; i < 3; i++) {
        record.paddingData[i] = buf[i];
    }
    return record;
}

int connectToFCGI3() {
    cout << "Wykonuję connectToFCGI3\n";
    FCGI_Header header = MakeHeader(FCGI_BEGIN_REQUEST, 300, 37, 3);
    char my_data[100] = "Marta jest super ekstra i Ola tez!11!";
    Record_fcgi record = MakeRecord(header, my_data);
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
    cout << "rc = " << rc << "\n";
    //sendto(fd_fcgi, &header, sizeof(header), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
    sendto(fd_fcgi, &record, sizeof(record), 0, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
    cout << "Po sendto. Wychodzę z connectToFCGI\n";
}


int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    char buf[8] = "Hello\n";

    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("Error creating socket");
        return -1;
    }
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
            cout<< buffer << endl;

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

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB