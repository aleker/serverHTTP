#include <unistd.h>
#include "constants.h"
#include "HTTPManager.h"
#include "FCGIManager.h"
#include <thread>
#include <error.h>
#include <unordered_set>
#include "ConfigFile.h"

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
            clientFds.insert(clientConnection.descriptor);
            //unsigned char message[bufsize];
            string message;
            serverMainConnection.getMessage(&clientConnection, &message);

            // FCGI CONNECTION:
            int port = 0;
            string ip = "";
            if (ConfigFile::getConfigFile().readFCGI(&ip, &port) == -1) break;
            FCGIManager fcgiConnection(ip.c_str(), port);
            fcgiConnection.createConnection();

            // PARSING AND SENDING MESSAGE FROM SERVER TO FCGI:
            serverMainConnection.sendMessage(&fcgiConnection, &message);

            // SENDING MESSAGE FROM FCGI TO CLIENT
            fcgiConnection.sendMessage(clientConnection.descriptor);

            close(fcgiConnection.descriptor);
            close(clientConnection.descriptor);
        }
    });

    t_clients.join();
    close(serverMainConnection.descriptor);
}

// TODO usunąć komentarze
// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB