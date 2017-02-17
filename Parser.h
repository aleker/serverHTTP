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

private:
    vector<unsigned char> parameters;
    vector<unsigned char> host;
    vector<unsigned char> uri;

    vector<unsigned char> messageCopy;

    int findSubstring(const char *substring);
    void createGetStruct();


};


#endif //SERWERHTTP_PARSER_H
