#include <unistd.h>
#include "constants.h"
#include "HTTPManager.h"
#include "FCGIManager.h"
#include <thread>
#include <error.h>
#include <unordered_set>
#include "ConfigFile.h"
#include <signal.h>

using namespace std;

unordered_set<int> clients_descriptors;
int serverDescriptor;

void ctrl_c(int){
    for(int clientFd : clients_descriptors)
        close(clientFd);
    close(serverDescriptor);
    printf("Closing server\n");
    exit(0);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    // FCGI CONNECTION - read info:
    int port = 0;
    string ip = "";
    if (ConfigFile::getConfigFile().readFCGI(&ip, &port) == -1) return -1;

    // MAIN SERVER CONNECTION
    HTTPManager serverMainConnection(argv[1],atoi(argv[2]));
    if (ConfigFile::getConfigFile().readTimeout(&serverMainConnection.timeout) == -1) return -1;
    if (ConfigFile::getConfigFile().readRole(&serverMainConnection.role) == -1) return -1;
    serverMainConnection.prepareServerSocket();
    int res = listen(serverMainConnection.descriptor, 1);
    if (res) error(1, errno, "listen failed!");
    serverDescriptor = serverMainConnection.descriptor;

    signal(SIGINT, ctrl_c);

    // CLIENT CONNECTION:
    ConnectionManager clientConnection = ConnectionManager();
    while ((clientConnection.descriptor =
                    accept(serverMainConnection.descriptor, (sockaddr *) &clientConnection.socketStruct,
                           &clientConnection.socketSize)) != -1) {
        struct timeval tv;
        tv.tv_sec = 2;
        setsockopt(clientConnection.descriptor, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
        std::thread t_client([=] (ConnectionManager client) {
            clients_descriptors.insert(client.descriptor);
            cout << "clients_count: " << clients_descriptors.size() << endl;
            string message;
            if (serverMainConnection.getMessage(&client, &message) == -1) {
                perror("Connection with client is canceled.");
                clients_descriptors.erase(client.descriptor);
                return;
            }
            // PARSING AND SENDING MESSAGE FROM SERVER TO FCGI:
            FCGIManager *fcgiConnection = new FCGIManager(ip.c_str(), port);
            if (fcgiConnection->createConnection() != -1) {
                if (serverMainConnection.sendMessage(fcgiConnection, &message, &client) == -1){
                    goto end_connection;
                }
                // SENDING MESSAGE FROM FCGI TO CLIENT
                if (fcgiConnection->will_send_message) fcgiConnection->sendMessage(client.descriptor);
            }
            end_connection:
            clients_descriptors.erase(client.descriptor);
            close(client.descriptor);
            delete fcgiConnection;
            return;
        }, clientConnection);

        t_client.detach();
    }

    perror("Error accepting client.\n");
    return 0;
}
