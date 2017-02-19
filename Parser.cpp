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

int Parser::findSubstring(string substring, string mainString){
    std::size_t found = mainString.find(substring);
    if (found!=std::string::npos){
        return (int) found;
    }
    return -1;
}

void Parser::prepareAdditionalParamaters() {
    std::istringstream f(messageCopy);
    std::string line;

    while (std::getline(f, line)) {
        int index = findSubstring(":", line);
        if (index != -1) {
            std::string parameter = line.substr(0, index);
//            TODO Check if this makes any sense at all?
            for (int i = 0; i < (signed)parameter.length(); i++) {
                if (parameter[i] == '-') parameter.replace(i, 1, "_");
            }
            std::transform(parameter.begin(), parameter.end(), parameter.begin(), ::toupper);
            parameter.insert(0, "HTTP_");
            parameters.push_back(parameter);
            values.push_back(line.substr(index + 2, line.size() - index - 2));
        } else if (line.size() > 1) {
            index = findSubstring("/", line);
            if(index != -1) {
                bool queryStart = false;
                while (line[index] != ' ') {
                    if(line[index] == '?') queryStart = true;
                    if(queryStart) query.push_back(line[index]);
                    uri.push_back(line[index]);
                    index++;
                }
                serverProtocol = line.substr(index+1, line.size()-index-1);
            }
            else stdinContent = line;
        }
    }
}

int Parser::prepareStandardParameters() {
    CGI_values.push_back("CGI/1.1");
    CGI_values.push_back(serverProtocol);
    CGI_values.push_back(requestMethod);
    CGI_values.push_back(uri);
    CGI_values.push_back(query);
    for (int i = 0; i < (signed)parameters.size(); i++) {
        if (parameters[i] == "HTTP_CONTENT_LENGTH") {
            CGI_values.push_back(values[i]);
        }
        if (parameters[i] == "HTTP_CONTENT_TYPE") {
            CGI_values.push_back(values[i]);
            return 0;
        }
    }
    return 0;
}

int Parser::parseBrowserMessage(unsigned char* message){
    int i = 0;
    while(message[i]!='\0') {
        messageCopy.push_back(message[i]);
        i++;
    }
    if(findSubstring("favicon", messageCopy) != -1)
        return -1;
    else if (findSubstring("GET ", messageCopy) != -1){
        requestMethod = "GET";
    }
    else if (findSubstring("POST ", messageCopy) != -1){
        requestMethod = "POST";
    }
    prepareAdditionalParamaters();
    prepareStandardParameters();
    return 0;
}

int Parser::mergeIntoOneMessage(string* content_data) {
    // STANDARD PARAMETERS
    for (int i = 0; i < (signed)CGI_values.size(); i++) {
        try {
            if (CGI_values[i][CGI_values[i].length() - 1] == '\r')
                CGI_values[i].erase(CGI_values[i].begin() + CGI_values[i].length() - 1);
            // adding sizes of parameter name and value
            char size2 = char((int)CGI_params[i].length());
            content_data->push_back(size2);
            size2 = char((int)CGI_values[i].length());
            content_data->push_back(size2);
            // adding parameter name and value
            content_data->append(CGI_params[i]);
            content_data->append(CGI_values[i]);

        }
        catch (exception& e){
            cout << e.what() << "\n";
            perror("Error merging parameters into one message");
            return -1;
        }
    }

    // ADDITIONAL PARAMETERS
    for (int i = 0; i < (signed)parameters.size(); i++) {
        if (parameters[i][parameters[i].length() - 1] == '\r')
            parameters[i].erase(parameters[i].begin() + parameters[i].length() - 1);
        if (values[i][values[i].length() - 1] == '\r')
            values[i].erase(values[i].begin() + values[i].length() - 1);
        // adding sizes of parameter name and value
        char size2 = char((int)parameters[i].length());
        content_data->push_back(size2);
        size2 = char((int)values[i].length());
        content_data->push_back(size2);
        // adding parameter name and value
        content_data->append(parameters[i]);
        content_data->append(values[i]);
    }
    return 0;
}

void fromStringToUnsignedCharArray(string original_data, unsigned char* output) {
    for (int i = 0; i < (signed)original_data.length(); i++) {
        output[i] = (unsigned char) original_data[i];
    }
    return;
}

void Parser::createRecords(vector<Record>* records, int request_id, int role) {
    string contentData;
    mergeIntoOneMessage(&contentData);

    // FCGI_BEGIN
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE, FCGI_BEGIN_REQUEST, request_id);
    beginRecord.fill(role);

    // FCGI_PARAM
    int params_size = (int) contentData.length();
    unsigned char params_data[params_size];
//    TODO check why strcpy doesn't work
    fromStringToUnsignedCharArray(contentData, &params_data[0]);
    int padding_size = (8 - params_size%8)%8;
    int record_size = HEADER_SIZE + params_size + padding_size + HEADER_SIZE;
    if (params_size == 0) record_size -= HEADER_SIZE;
    StreamRecord paramRecord(record_size, FCGI_PARAMS, request_id);
    paramRecord.fill(params_size, params_data);

    // FCGI_STDIN
    // TODO stdin_data przypisać wartość!
    int stdin_size = (int) stdinContent.length();
    unsigned char stdin_data[stdin_size];
    strcpy((char *) stdin_data, stdinContent.c_str());
    padding_size = (8 - stdin_size%8)%8;
    record_size = HEADER_SIZE + stdin_size + padding_size + HEADER_SIZE;
    if (stdin_size == 0) record_size -= HEADER_SIZE;
    StreamRecord stdinRecord(record_size, FCGI_STDIN, request_id);
    stdinRecord.fill(stdin_size, stdin_data);

    records->push_back(beginRecord);
    records->push_back(paramRecord);
    records->push_back(stdinRecord);
}