#include <unistd.h>
#include "constants.h"
#include "HTTPManager.h"
#include "FCGIManager.h"
#include <thread>
#include <error.h>
#include <unordered_set>


using namespace std;

// client sockets
std::vector<clientStruct> clients;

// TODO usunąć z maina bo tak
int random_int(int max) {
    if (max != -1) return rand()%(max+1) + 0;
    else return -1;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    // MAIN SERVER CONNECTION
    HTTPManager serverMainConnection(argv[1],atoi(argv[2]));
    serverMainConnection.prepareServerSocket();      // prepare server socket
    // FCGI CONNECTION:
    // TODO parametry 127.0.0.1 8000 w pliku konfiguracyjnym
    FCGIManager* fcgiConnection = new FCGIManager("127.0.0.1", 8000);
    fcgiConnection->createConnection();

    int res = listen(serverMainConnection.descriptor, 1);
    if (res) error(1, errno, "listen failed!");


//    THREAD THAT ACCEPTS CLIENT CONNECTIONS
    std::thread t_clients([=] {
        while(1) {
            // CLIENT CONNECTION:
            ConnectionManager clientConnection = ConnectionManager();
            serverMainConnection.acceptConnection(&clientConnection);
            string message;
            serverMainConnection.getMessage(&clientConnection, &message);
            clientStruct newClient = {clientConnection.descriptor, message};
            clients.push_back(newClient);
        }
    });
    std::thread t_fcgi([=] {
        while(1) {
            int random_index;
            if ((random_index = random_int((int) (clients.size() - 1))) < 0 ) continue;
            cout << " client " << clients[random_index].descriptor << endl;
            // PARSING AND SENDING MESSAGE FROM SERVER TO FCGI:
            serverMainConnection.sendMessage(fcgiConnection, &clients[random_index].message);
            // SENDING MESSAGE FROM FCGI TO CLIENT
            fcgiConnection->sendMessage(clients[random_index].descriptor);
            close(clients[random_index].descriptor);
            clients.erase(clients.begin() + random_index);
        }
    });

    t_clients.join();
    t_fcgi.join();
    close(fcgiConnection->descriptor);
    delete fcgiConnection;
    close(serverMainConnection.descriptor);
}

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB