#include "constants.h"
#include "HTTPManager.h"
#include "FCGIManager.h"
#include <thread>
#include <error.h>
#include <unordered_set>
#include <unistd.h>
#include "ConfigFile.h"

using namespace std;

// client sockets
std::vector<clientStruct> clients;

// TODO usunąć z maina bo tak
int random_int(int max) {
    if (max != -1) return rand() % (max + 1) + 0;
    else return -1;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    // MAIN SERVER CONNECTION
    HTTPManager serverMainConnection(argv[1], atoi(argv[2]));
    serverMainConnection.prepareServerSocket();
    // FCGI CONNECTION:
    int port = 0;
    string ip = "";
    if (ConfigFile::getConfigFile().readFCGI(&ip, &port) == -1) return 0;

    int res = listen(serverMainConnection.descriptor, 1);
    if (res) error(1, errno, "listen failed!");

    bool exit_server = false;
//    THREAD THAT ACCEPTS CLIENT CONNECTIONS
    std::thread t_clients([=] {
        while (!exit_server) {
            // CLIENT CONNECTION:
            ConnectionManager clientConnection = ConnectionManager();
            serverMainConnection.acceptConnection(&clientConnection);
            string message;
            int result = serverMainConnection.getMessage(&clientConnection, &message);
            if (result == -1) {
                perror("Connection with client is canceled.");
                continue;
            }
            clientStruct newClient = {clientConnection.descriptor, message};
            clients.push_back(newClient);
        }
    });
    std::thread t_fcgi([=] {
        while (!exit_server) {
            int random_index;
            // TODO zmienić na FIFO
            if ((random_index = random_int((int) (clients.size() - 1))) < 0) continue;
            cout << " client " << clients[random_index].descriptor << endl;
            // PARSING AND SENDING MESSAGE FROM SERVER TO FCGI:
            FCGIManager *fcgiConnection = new FCGIManager(ip.c_str(), port);
            fcgiConnection->createConnection();
            serverMainConnection.sendMessage(fcgiConnection, &clients[random_index].message,
                                             clients[random_index].descriptor);
            // SENDING MESSAGE FROM FCGI TO CLIENT
            fcgiConnection->sendMessage(clients[random_index].descriptor);
            close(clients[random_index].descriptor);
            clients.erase(clients.begin() + random_index);
            delete fcgiConnection;
        }

    });

    getchar();
    exit_server = true;
    // TODO joiny nie łączą się
    t_clients.join();
    t_fcgi.join();
    close(serverMainConnection.descriptor);
    cout << "\nExiting the server. Goodbye!\n";
    return 0;
}
