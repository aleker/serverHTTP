//
// Created by tusia on 16.02.17.
//

#include "Parser.h"
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>

// TODO usunąć messageCopy z klasy
// TODO stworzyć gotowe rekordy do wysłania

int Parser::parseBrowserMessage(unsigned char* message){
    int i = 0;
    while(message[i]!='\0') {
        messageCopy.push_back(message[i]);
        i++;
    }
    if(findSubstring("favicon") != -1)
        return -1;

    if (findSubstring("GET ") != -1){
        createGetStruct();
    }
    return 0;

}
void Parser::createGetStruct(){
    int i=0;
    int index = findSubstring("?");
    if( index!=-1){
        index++;
        while(messageCopy[index]!=' '){
            parameters.push_back(messageCopy[index]);
            index++;
        }
    }
    index = findSubstring("/");
    if(index!=-1){
        index++;
        while(messageCopy[index]!=' ' and messageCopy[index]!='?'){
            uri.push_back(messageCopy[index]);
            index++;
        }
    }
    index = findSubstring("Host");
    if(index!=-1){
        i = index+6;
        while(messageCopy[i] != '\n'){
            host.push_back(messageCopy[i]);
            i++;
        }
    }

    cout<< "*****************PARSED MESSAGE!************** \n";
    cout << "parameters: ";
    for (int k=0;k<parameters.size();k++) cout <<parameters[k];
    cout << endl << "host: ";
    for (int k=0;k<host.size();k++) cout << host[k];
    cout << endl << "uri: ";
    for (int k=0;k<uri.size();k++) cout << uri[k];
    cout << endl << "*****************PARSED MESSAGE END!************** \n";

}

int Parser::findSubstring(const char* substring){
    auto it = std::search(messageCopy.begin(), messageCopy.end(), substring, substring + strlen(substring));
    if (it != messageCopy.end()) {
        int index = (int) std::distance(messageCopy.begin(), it );
        return index;
    }
    else
        return -1;
}

void Parser::createRecords(vector<Record>* records, int request_id, int role) {
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE + HEADER_SIZE, FCGI_BEGIN_REQUEST, request_id);
    beginRecord.fill(role);

    unsigned char params_data[] = {}; // params_data
    int params_size = 0;// params_size
    int padding_size = (8 - params_size%8)%8;         // gdzieś musi zczytać message size z tych wektorów
    StreamRecord paramRecord(HEADER_SIZE + params_size + padding_size + HEADER_SIZE, FCGI_PARAMS, request_id);
    paramRecord.fill(params_size, params_data);

    unsigned char stdin_data[] = {}; // stdin_data
    int stdin_size = 0;// stdin_size
    padding_size = (8 - stdin_size%8)%8;
    StreamRecord stdinRecord(HEADER_SIZE  + stdin_size + padding_size + HEADER_SIZE, FCGI_STDIN, request_id);
    stdinRecord.fill(stdin_size, stdin_data);

    records->push_back(beginRecord);
    records->push_back(paramRecord);
    records->push_back(stdinRecord);
}