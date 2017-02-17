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

private:
    vector<unsigned char> parameters;
    vector<unsigned char> host;
    vector<unsigned char> uri;

    vector<unsigned char> messageCopy;

    int findSubstring(const char *substring);
    void createGetStruct();

};


#endif //SERWERHTTP_PARSER_H
