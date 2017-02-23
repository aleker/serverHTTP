//
// Created by tusia on 16.02.17.
//

#include "Parser.h"
#include <string.h>
#include <algorithm>
#include <sstream>
#include <cmath>

int findSubstring(string substring, string mainString) {
    std::size_t found = mainString.find(substring);
    if (found != std::string::npos) {
        return (int) found;
    }
    return -1;
}

string getBoundary(string value) {
    int index = findSubstring("boundary=", value);
    if (index != -1) {
        string substring = value.substr((unsigned long) index, value.size() - index);
        int index2 = findSubstring(";", substring);
        if (index2 != -1) {
            return value.substr((unsigned long) (index + 9), (unsigned long) (index2 - index - 9 - 1));
        }
        return value.substr((unsigned long) (index + 9), value.size() - index - 9 - 1);
    }
    return "";
}


void Parser::prepareAdditionalParamaters(string *message) {
    std::istringstream f(*message);
    std::string line;
    bool request_method_founded = false;
    string boundary = "";

    while (std::getline(f, line)) {
        int index = findSubstring(": ", line);
        // READ PARAMETERS
        if (index != -1) {
            std::string parameter = line.substr(0, (unsigned long) index);
            for (int i = 0; i < (signed) parameter.length(); i++) {
                if (parameter[i] == '-') parameter.replace((unsigned long) i, 1, "_");
            }
            std::transform(parameter.begin(), parameter.end(), parameter.begin(), ::toupper);
            parameter.insert(0, "HTTP_");
            parameters.push_back(parameter);
            string value = line.substr((unsigned long) (index + 2), line.size() - index - 2);
            values.push_back(value);
            if (parameter == "HTTP_CONTENT_TYPE" and boundary.empty()) {
                boundary = getBoundary(value);
            }
        }// READ GET/POST ONLY ONCE
        else if (!request_method_founded and (index = findSubstring("/", line)) != -1) {
            request_method_founded = true;
            bool queryStart = false;
            while (line[index] != ' ') {
                if (line[index] == '?') queryStart = true;
                if (queryStart) query.push_back(line[index]);
                uri.push_back(line[index]);
                index++;
            }
            serverProtocol = line.substr((unsigned long) (index + 1), line.size() - index - 1);
        } // READ CONTENT
        else {
            if (line == "\r") continue;
            index = findSubstring(boundary, line);
            if (index >= 0 and !boundary.empty()) stdinContent.append(line.substr(0, (unsigned long) index));
            else stdinContent.append(line);
            stdinContent.append("\n");
        }
    }
}

int Parser::prepareStandardParameters() {
    CGI_values.push_back("CGI/1.1");
    CGI_values.push_back(serverProtocol);
    CGI_values.push_back(requestMethod);
    CGI_values.push_back(uri);
    CGI_values.push_back(query);
    for (int i = 0; i < (signed) parameters.size(); i++) {
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

int Parser::parseBrowserMessage(string *message) {
    if (findSubstring("favicon", *message) != -1)
        return -1;
    else if (findSubstring("GET ", *message) != -1) {
        requestMethod = "GET";
    } else if (findSubstring("POST ", *message) != -1) {
        requestMethod = "POST";
    }
    prepareAdditionalParamaters(message);
    prepareStandardParameters();
    return 0;
}

int Parser::mergeIntoOneMessage(string *content_data) {
    // STANDARD PARAMETERS
    for (int i = 0; i < (signed) CGI_values.size(); i++) {
        try {
            if (CGI_values[i][CGI_values[i].length() - 1] == '\r')
                CGI_values[i].erase(CGI_values[i].begin() + CGI_values[i].length() - 1);
            // adding sizes of parameter name and value
            char size2 = char((int) CGI_params[i].length());
            content_data->push_back(size2);
            size2 = char((int) CGI_values[i].length());
            content_data->push_back(size2);
            // adding parameter name and value
            content_data->append(CGI_params[i]);
            content_data->append(CGI_values[i]);
        }
        catch (exception &e) {
            cout << e.what() << "\n";
            perror("Error merging parameters into one message");
            return -1;
        }
    }

    // ADDITIONAL PARAMETERS
    for (int i = 0; i < (signed) parameters.size(); i++) {
        if (parameters[i][parameters[i].length() - 1] == '\r')
            parameters[i].erase(parameters[i].begin() + parameters[i].length() - 1);
        if (values[i][values[i].length() - 1] == '\r')
            values[i].erase(values[i].begin() + values[i].length() - 1);
        // adding sizes of parameter name and value
        char size2 = char((int) parameters[i].length());
        content_data->push_back(size2);
        size2 = char((int) values[i].length());
        content_data->push_back(size2);
        // adding parameter name and value
        content_data->append(parameters[i]);
        content_data->append(values[i]);
    }
    return 0;
}

void fromStringToUnsignedCharArray(string original_data, unsigned char *output) {
    for (int i = 0; i < (signed) original_data.length(); i++) {
        output[i] = (unsigned char) original_data[i];
    }
    return;
}

void Parser::createRecords(vector<Record> *records, int request_id, int role) {
    string contentData;
    mergeIntoOneMessage(&contentData);

    // FCGI_BEGIN
    BeginRecord beginRecord(HEADER_SIZE + BEGIN_REQUEST_BODY_SIZE, FCGI_BEGIN_REQUEST, request_id);
    beginRecord.fill(role);
    records->push_back(beginRecord);

    // FCGI_PARAM
    int params_size = (int) contentData.length();
    unsigned char params_data[params_size];
    fromStringToUnsignedCharArray(contentData, &params_data[0]);
    int padding_size = (8 - params_size % 8) % 8;
    int record_size = HEADER_SIZE + params_size + padding_size;
    StreamRecord paramRecord(record_size, FCGI_PARAMS, request_id);
    paramRecord.fill(params_size, params_data);
    records->push_back(paramRecord);
    if (params_size != 0) {
        Record endParamsRecord(HEADER_SIZE, FCGI_PARAMS, request_id);
        records->push_back(endParamsRecord);
    }

    // FCGI_STDIN
    int stdin_size = (int) stdinContent.length();
    int number_of_parts = max((int) ceil(stdin_size / MAX_SIZE), 1);
    for (int i = 1; i <= number_of_parts; i++) {
        int part_content_size = (int) min(MAX_SIZE, stdin_size - (i - 1) * MAX_SIZE);
        unsigned char part_content_data[part_content_size];
        string part_content = stdinContent.substr((unsigned long) ((i - 1) * MAX_SIZE),
                                                  (unsigned long) part_content_size);
        strcpy((char *) part_content_data, part_content.c_str());
        padding_size = (8 - part_content_size % 8) % 8;
        record_size = HEADER_SIZE + part_content_size + padding_size;
        StreamRecord stdinRecord(record_size, FCGI_STDIN, request_id);
        stdinRecord.fill(part_content_size, part_content_data);
        records->push_back(stdinRecord);
    }
    // send end stdin record
    if (stdin_size != 0) {
        Record endRecord(HEADER_SIZE, FCGI_STDIN, request_id);
        records->push_back(endRecord);
    }

}

void Parser::reparse() {
    for (int i = 0; i < (signed) parameters.size(); i++) {
        parameters[i].erase(0, 5);
        parameters[i].append(": ");
        values[i].append("\r\n");
        //cout << "parametr i wartość = " << parameters[i] << values[i];
    }
}

void Parser::createHTTPResponse(string* HTTPresponse){
    HTTPresponse->append(answerHeader);
    HTTPresponse->append("Content-type: text/html\r\n");
    HTTPresponse->append("\r\n");
    HTTPresponse->append("<html>\n<head>\n<title>Hello, World!</title>\n</head>\n<body>\n<h1>Hello from ");
    HTTPresponse->append(uri);
    HTTPresponse->append(" !</h1>\n</body>\n</html>\n\r\n");
}