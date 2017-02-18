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
            for (int i = 0; i < parameter.length(); i++) {
                if (parameter[i] == '-') parameter.replace(i, 1, "_");
            }
            std::transform(parameter.begin(), parameter.end(), parameter.begin(), ::toupper);
            parameter.insert(0, "HTTP_");
            parameters.push_back(parameter);
            values.push_back(line.substr(index + 2, line.size() - index - 2));
        } else if (line.size() > 1) {
            index = findSubstring("/", line);
            bool queryStart = false;
            while (line[index] != ' ') {
                if(line[index] == '?') queryStart = true;
                if(queryStart) query.push_back(line[index]);
                uri.push_back(line[index]);
                index++;
            }
            serverProtocol = line.substr(index+1, line.size()-index-1);
        }
    }
}

int Parser::prepareStandardParameters() {
    CGI_values.push_back("CGI/1.1");
    CGI_values.push_back(serverProtocol);
    CGI_values.push_back(requestMethod);
    CGI_values.push_back(uri);
    CGI_values.push_back(query);
    for (int i = 0; i < parameters.size(); i++) {
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
    for (int i = 0; i < CGI_values.size(); i++) {
        try {
            //content_data->append("\r");
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
    for (int i = 0; i < parameters.size(); i++) {
        //content_data->append("\r");
        content_data->append(parameters[i]);
        content_data->append(values[i]);
    }
    return 0;
}

unsigned char* fromStringToUnsignedCharArray(string original_data, unsigned char* output) {
    for (int i = 0; i < original_data.length(); i++) {
        output[i] = (unsigned char) original_data[i];
    }
    //output[original_data.length()] = 0;     // proper end of string
}

void Parser::createRecords(vector<Record>* records, int request_id, int role) {
    string contentData;
    //mergeIntoOneMessage(&contentData);
    contentData.append("GATEWAY_INTERFACECGI/1.1SERVER_SOFTWAREnginxQUERY_STRINGREQUEST_METHODGETCONTENT_TYPECONTENT_LENGTHSCRIPT_FILENAME/etc/nginx/html/SCRIPT_NAME/REQUEST_URI/DOCUMENT_URI/DOCUMENT_ROOT/etc/nginx/htmlSERVER_PROTOCOLHTTP/1.1\tREMOTE_ADDR127.0.0.1REMOTE_PORT46910\tSERVER_ADDR127.0.0.1SERVER_PORT80\tSERVER_NAMElocalhost");
    contentData.append("\tHTTP_HOST0.0.0.0DHTTP_USER_AGENTMozilla/5.0 (X11; Linux x86_64; rv:51.0) Gecko/20100101 Firefox/51.0?HTTP_ACCEPTtext/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8HTTP_ACCEPT_LANGUAGEen-US,en;q=0.5HTTP_ACCEPT_ENCODINGgzip, deflate");
    contentData.append("HTTP_CONNECTIONkeep-aliveHTTP_UPGRADE_INSECURE_REQUESTS1HTTP_IF_MODIFIED_SINCEFri, 10 Feb 2017 11:31:30 GMTHTTP_IF_NONE_MATCH\"589da492-264\"\tHTTP_CACHE_CONTROLmax-age=0");
    if (contentData[contentData.length()-1] == '\r') contentData.erase(contentData.begin() + contentData.length() - 1);

    // BEGIN
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE, FCGI_BEGIN_REQUEST, request_id);
    beginRecord.fill(role);

    // RECORD
    int params_size = contentData.length();
    if (contentData.length() == 0)  params_size = 0;
    unsigned char params_data[params_size];
    fromStringToUnsignedCharArray(contentData, &params_data[0]);
    cout << "sizeof(params_data) = " << sizeof(params_data) << "\n";
    int padding_size = (8 - params_size%8)%8;
    int record_size = HEADER_SIZE + params_size + padding_size + HEADER_SIZE;
    if (params_size == 0) record_size -= HEADER_SIZE;
    StreamRecord paramRecord(record_size, FCGI_PARAMS, request_id);
    paramRecord.fill(params_size, params_data);

    // STDIN
    // TODO stdin_data przypisać wartość!
    unsigned char stdin_data[] = {};
    int stdin_size = sizeof(stdin_data);
    padding_size = (8 - stdin_size%8)%8;
    record_size = HEADER_SIZE + stdin_size + padding_size + HEADER_SIZE;
    if (stdin_size == 0) record_size -= HEADER_SIZE;
    StreamRecord stdinRecord(record_size, FCGI_STDIN, request_id);
    stdinRecord.fill(stdin_size, stdin_data);

    records->push_back(beginRecord);
    records->push_back(paramRecord);
    records->push_back(stdinRecord);
}