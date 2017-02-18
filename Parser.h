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
    Parser(){}

    int parseBrowserMessage(unsigned char* message);
    void createRecords(vector<Record> *records, int request_id, int role);
    string requestMethod;

private:

    vector<std::string> parameters;
    vector<std::string> values;
    vector<std::string> CGI_values;
    std::string query;
    std::string uri;
    std::string serverProtocol;

    string messageCopy;



    void prepareAdditionalParamaters();

    int findSubstring(string substring, string mainString);

    int prepareStandardParameters();

    int mergeIntoOneMessage(string *content_data);
};


#endif //SERWERHTTP_PARSER_H
