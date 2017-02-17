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
    int requestMethod;

private:

    vector<std::string> parameters;
    vector<std::string> values;
    std::string query;
    std::string host;
    std::string uri;

    string messageCopy;



    void prepareParamaters();

    int findSubstring(string substring, string mainString);
};


#endif //SERWERHTTP_PARSER_H
