//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_PARSER_H
#define SERWERHTTP_PARSER_H

#include "constants.h"
#include "StreamRecord.h"
#include "BeginRecord.h"
#include <vector>

using namespace std;

class Parser {
public:
    string requestMethod;

    Parser() {}

    int parseBrowserMessage(string *message);

    void createRecords(vector<Record> *records, int request_id, int role);
    void createHTTPResponse(string *HTTPresponse);

private:
    vector<string> parameters;
    vector<string> values;
    vector<string> CGI_values;
    string query;
    string uri;
    string serverProtocol;
    string stdinContent;

    void prepareAdditionalParamaters(string *message);

    int prepareStandardParameters();

    int mergeIntoOneMessage(string *content_data);


    void reparse();
};


#endif //SERWERHTTP_PARSER_H
