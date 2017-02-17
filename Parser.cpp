//
// Created by tusia on 16.02.17.
//

#include "Parser.h"
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <algorithm>
#include <string>
#include <sstream>

// TODO usunąć messageCopy z klasy
// TODO stworzyć gotowe rekordy do wysłania

int Parser::parseBrowserMessage(unsigned char* message){
    int i = 0;
    while(message[i]!='\0') {
        messageCopy.push_back(message[i]);
        i++;
    }
    if(findSubstring("favicon", messageCopy) != -1)
        return -1;
    else if (findSubstring("GET ", messageCopy) != -1){
        requestMethod = GET_METHOD;
    }
    else if (findSubstring("POST ", messageCopy) != -1){
        requestMethod = POST_METHOD;
    }

    prepareParamaters();
    cout << "parameters prepared" <<endl;
    return 0;

}

void Parser::prepareParamaters() {
    std::istringstream f(messageCopy);
    std::string line;

    while (std::getline(f, line)) {
        int index = findSubstring(":", line);
        if (index!=-1){
            parameters.push_back(line.substr(0,index));
            values.push_back(line.substr(index+2, line.size()-index-2));
        } else continue;
    }
}

int Parser::findSubstring(string substring, string mainString){
    std::size_t found = mainString.find(substring);
    if (found!=std::string::npos){
        return (int) found;
    }
    return -1;
}

void Parser::createRecords(vector<Record>* records, int request_id, int role) {
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE, FCGI_BEGIN_REQUEST, request_id);
    beginRecord.fill(role);

    unsigned char params_data[] = "";  // params_data
    int params_size = sizeof(params_data);              // params_size
    int padding_size = (8 - params_size%8)%8;           // gdzieś musi zczytać message size z tych wektorów
    StreamRecord paramRecord(HEADER_SIZE + params_size + padding_size + HEADER_SIZE, FCGI_PARAMS, request_id);
    paramRecord.fill(params_size, params_data);

    unsigned char stdin_data[] = {};                    // stdin_data
    int stdin_size = 0;                                 // stdin_size
    padding_size = (8 - stdin_size%8)%8;
    StreamRecord stdinRecord(HEADER_SIZE  + stdin_size + padding_size + HEADER_SIZE, FCGI_STDIN, request_id);
    stdinRecord.fill(stdin_size, stdin_data);

    records->push_back(beginRecord);
    records->push_back(paramRecord);
    records->push_back(stdinRecord);
}