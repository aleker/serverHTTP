#include <unistd.h>
#include "constants.h"
#include "HTTPManager.h"
#include "FCGIManager.h"
#include <thread>
#include <error.h>
#include <unordered_set>

using namespace std;

// client sockets
std::unordered_set<int> clientFds;

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    HTTPManager serverMainConnection(argv[1],atoi(argv[2]));
    serverMainConnection.prepareServerSocket();      // prepare server socket

    int res = listen(serverMainConnection.descriptor, 1);
    if (res) error(1, errno, "listen failed!");

    std::thread t_clients([=] {
        while(1) {
            // CLIENT CONNECTION:
            ConnectionManager clientConnection = ConnectionManager();
            serverMainConnection.acceptConnection(&clientConnection);
            clientFds.insert(clientConnection);
        }
    });
    std::thread t_messages_from_clients([=] {
       while(1){
           // READING MESSAGE FROM CLIENT:
           unsigned char message[bufsize];
           serverMainConnection.getMessage(&clientConnection, message);
       }
    });

    if (listen(serverMainConnection.descriptor, 10) == 0) {
        while (true) {




            // FCGI CONNECTION:
            // TODO parametry 127.0.0.1 8000 w pliku konfiguracyjnym
            FCGIManager fcgiConnection("127.0.0.1", 8000);
            fcgiConnection.createConnection();

            // PARSING AND SENDING MESSAGE FROM SERVER TO FCGI:
            serverMainConnection.sendMessage(&fcgiConnection, message);

            // SENDING MESSAGE FROM FCGI TO CLIENT
            fcgiConnection.sendMessage(clientConnection.descriptor);

            close(fcgiConnection.descriptor);
            close(clientConnection.descriptor);
        }
    }
    close(serverMainConnection.descriptor);
}

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB