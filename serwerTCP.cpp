#include <unistd.h>
#include "constants.h"
#include "ConnectionManager.h"

using namespace std;

int main(int argc, char** argv) {
    if (argc < 3) {
        cerr << "Basic TCP server - set hosting IP and port by args" << endl;
        cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << endl;
        return -1;
    }

    ConnectionManager serverMainConnection(argv[1],atoi(argv[2]));
    serverMainConnection.prepareServerSocket();      // prepare server socket

    if (listen(serverMainConnection.descriptor, 10) == 0) {
        while (true) {
            // CLIENT CONNECTION:
            ConnectionManager clientConnection = ConnectionManager();
            serverMainConnection.acceptConnection(&clientConnection);

            // READING MESSAGE FROM CLIENT:
            unsigned char message[bufsize];
            serverMainConnection.getMessage(&clientConnection, message);

            // FCGI CONNECTION:
            // TODO parametry 127.0.0.1 8000 w pliku konfiguracyjnym
            ConnectionManager fcgiConnection("127.0.0.1", 8000);
            fcgiConnection.createConnection();

            // PARSING AND SENDING MESSAGE FROM SERVER TO FCGI:
            serverMainConnection.sendMessage(&fcgiConnection, message, sizeof(message));

            // SENDING MESSAGE FROM FCGI TO CLIENT
            fcgiConnection.forwardMessage(clientConnection.descriptor);

            close(fcgiConnection.descriptor);
            close(clientConnection.descriptor);
        }
    }
    close(serverMainConnection.descriptor);
}

// https://fossies.org/linux/FCGI/fcgiapp.c#l_2190
// http://web.archive.org/web/20160306081510/http://fastcgi.com/drupal/node/6?q=node/22#SB