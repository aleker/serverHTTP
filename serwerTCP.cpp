#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "fcgio.h"

using namespace std;

int connectToFCGI() {
    cout << "Tworzę połączenie z fcgi" << endl;
    int fd_fcgi = socket(PF_INET, SOCK_STREAM, 0);
    char buf[1000] = {'a','b','c'};
    sockaddr_in fcgiSocket;
    fcgiSocket.sin_family = AF_INET;
    fcgiSocket.sin_port = htons(8000);       // Port serwera
    fcgiSocket.sin_addr.s_addr = inet_addr("127.0.0.1");

    int rc = connect(fd_fcgi, (sockaddr*)&fcgiSocket, sizeof(fcgiSocket));
    if (rc == -1) {
        perror("Error connecting to socket");
        return -1;
    }
    cout << "Po connect\n";
    write(fd_fcgi, buf, 1000);
    cout << "Wysłałem wiadomość\n";

    ssize_t readBytes = 0;
    while ((readBytes = recv(fd_fcgi, buf, sizeof(buf), 0)) != 0) {
        cout << "Jestem\n";
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

    if (listen(fd, 10) == 0) {
        while (true) {
            // ACCEPTING CONNECTION WITH CLIENT:
            // accept() - it fills acceptedConnection and sizeOfAcceptedSockaddr
            acceptedSocketFd = accept(fd, (sockaddr*)&acceptedSocket, &sizeOfAcceptedSockaddr);
            // There will be no TIME_WAIT
            setsockopt(acceptedSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

            // RUNNING FCGI:
            //--- http://stackoverflow.com/questions/26695738/nginx-fastcgi-without-using-spawn-fcgi
            connectToFCGI();
            cout<< "Dobra nara\n";
            // SENDING ANSWER TO CLIENT
            // ssize_t sentBytes = write(acceptedSocketFd, &buf, sizeof(buf));
            // cout << "Sent bytes: " << sentBytes << endl;
            close(acceptedSocketFd);
        }
    }
    close(fd);
}