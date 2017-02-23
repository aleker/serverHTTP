//
// Created by ola on 17.02.17.
//

#include <cstdio>
#include <unistd.h>
#include <vector>
#include <fstream>
#include "HTTPManager.h"
#include "constants.h"
#include "Record.h"
#include "Parser.h"
#include "ConfigFile.h"
#include "FCGIManager.h"
#include <sys/select.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

HTTPManager::HTTPManager(const char *host, int port) : ConnectionManager(host, port) {}

int recv_to(int fd, unsigned char *buffer, int len, int flags, int to) {

    fd_set readset;
    int result, iof = -1;
    struct timeval tv;

    // Initialize the set
    FD_ZERO(&readset);
    FD_SET(fd, &readset);

    // Initialize time out struct
    tv.tv_sec = 0;
    tv.tv_usec = to * 1000;
    // select()
    result = select(fd+1, &readset, NULL, NULL, &tv);

    // Check status
    if (result < 0)
        return -1;
    else if (result > 0 && FD_ISSET(fd, &readset)) {
        // Set non-blocking mode
        if ((iof = fcntl(fd, F_GETFL, 0)) != -1)
            fcntl(fd, F_SETFL, iof | O_NONBLOCK);
        // receive
        result = (int) recv(fd, buffer, (size_t) len, flags);
        // set as before
        if (iof != -1)
            fcntl(fd, F_SETFL, iof);
        return result;
    }
    return -2;
}


int HTTPManager::bindSocket(){
    int err = bind(descriptor, (sockaddr*)&socketStruct, sizeof(socketStruct));
    if (err == -1) {
        perror("Error binding the socket!");
        return -1;
    }
    return 0;
}

int HTTPManager::prepareServerSocket(){
    if (createSocket() == -1) return -1;
    createSockaddr();
    if(bindSocket() == -1) return -1;
    return 0;
}

int HTTPManager::isWhaleMessage(string* content_data) const {
    string content = *content_data;
    int found = (int) content.find("\nContent-Length:");
    if (found != -1) {
        string content_size;
        found += 16;
        while (content[found] == ASCII_SPACE) found++;
        while (isdigit(content[found])) {
            content_size.push_back(content[found]);
            found++;
        }
        int content_size2 = stoi(content_size);
        int found2 = (int) content.find("\n\r\n");
        if (found2 != -1 and found2 > found) {
            string content2 = content.substr((unsigned long) (found2 + 3), content.size() - found2 - 3);
            if ((int)content2.length() < content_size2)
                return -1;
        }
        return 0;
    }
    return 0;
}

int HTTPManager::getMessage(ConnectionManager* client, string* content_data) const {
#define NOT_SET -1
#define GET     1
#define POST    2
    cout << "\n***MESSAGE FROM CLIENT TO HTTP\n";
    unsigned char* buffer = new unsigned char[100];
    ssize_t Len = 0;
    // TODO SIGHUP?
    int left_to_read = -1;
    int content_size_found= NOT_SET;
    while (content_size_found == NOT_SET or left_to_read > 0) {
        if ((Len = recv(client->descriptor, buffer, 100, 0)) < 0) {
            perror("Error reading message from client.\n");
            return -1;
        }
        if (Len == 0) return isWhaleMessage(content_data);    // probably connection is canceled

        for (int i = 0; i < Len; i++) {
            content_data->push_back(buffer[i]);
        }
        if (content_size_found == NOT_SET) {
            int start_searching = max(0, (const int &) (content_data->length() - Len - 2));   // for searching \n\r\n
            int stop_searching = (int) content_data->length();
            std::string received = content_data->substr((unsigned long) start_searching,
                                                        (unsigned long) (stop_searching - start_searching));
            int end_of_parameters_in_buffer = (int) received.find("\n\r\n");
            if (end_of_parameters_in_buffer != -1) {
                // end_of_parameters_in_buffer points to the end of parameters (last \n in \n\r\n)
                end_of_parameters_in_buffer += 2;
                std::string parameters = content_data->substr(0, (unsigned long) (start_searching +
                                                                                  end_of_parameters_in_buffer + 1));
                int found = (int) parameters.find("\nContent-Length:");
                if (found != -1) {
                    string content_size;
                    found += 16;
                    while (parameters[found] == ASCII_SPACE) found++;
                    while (isdigit(parameters[found])) {
                        content_size.push_back(parameters[found]);
                        found++;
                    }
                    // left to read = content_size - (already_read - parameters)
                    left_to_read =
                            stoi(content_size) - ((int) (content_data->length() - start_searching - end_of_parameters_in_buffer - 1));
                    cout << "stoi(content_size = )" << stoi(content_size) << " left_to_read = " << left_to_read << endl;
                    content_size_found = POST;
                } else {
                    content_size_found = GET;
                    break;
                }
            }
        }
        else if (content_size_found == POST) {
            left_to_read -= Len;
        }
    }

    if (content_data->length() == 0) {
        cout << "RESET!\n";
        return -1;
    }
    delete [] buffer;
    cout << *content_data;
    cout << "***END OF MESSAGE FROM CLIENT TO HTTP\n";
    return 0;
}

int HTTPManager::sendMessage(FCGIManager* fcgi, string* message, ConnectionManager* client) const {
    Parser parser;
    // PARSING MESSAGE
    parser.parseBrowserMessage(message);

    cout << "parser.requestMethod.length() = " << parser.requestMethod.length() << endl;
    // IF POST -> CREATE RECORDS AND SEND TO FCGI
    if (parser.requestMethod == "POST") {
        cout << "POST request\n";
        std::vector<Record> records;
        parser.createRecords(&records, client->descriptor, role);
        // SENDING RECORDS
        cout << "\n***MESSAGE FROM HTTP TO FCGI\n";
        for (Record &record: records) {
            try {
                sendto(fcgi->descriptor, record.message, (size_t) record.array_size, 0,
                       (sockaddr *) &(fcgi->socketStruct), sizeof(fcgi->socketStruct));
            }
            catch (exception &e) {
                cout << e.what();
                perror("Error sending message to FCGI server.");
                return -1;
            }
        }
        cout << "***END OF MESSAGE FROM HTTP TO FCGI\n";
    }

    // IF GET -> SEND REPLY DIRECTLY TO CLIENT
    else if (parser.requestMethod == "GET") {
        // TODO GET tylko z istniejących plików
        cout << "GET request\n";
        cout << "\n***MESSAGE FROM HTTP TO CLIENT\n";
        fcgi->will_send_message = false;
        cout << "!!!!!!!!!Sending message without FCGI\n";
        string HTTPresponse;
        parser.createHTTPResponse(&HTTPresponse);
        cout << HTTPresponse;
        unsigned char response[HTTPresponse.length()];
        strcpy((char *) response, HTTPresponse.c_str());
        try {
            send(client->descriptor, &response, (size_t) HTTPresponse.length(), MSG_NOSIGNAL);
        }
        catch (exception &e) {
            cout << e.what();
            perror("Error sending message to client.");
            return -1;
        }
        close(fcgi->descriptor);
        cout << "***END OF MESSAGE FROM HTTP TO CLIENT\n";
    }
    else {
        close(fcgi->descriptor);
        return -1;
    }
    return 0;
}