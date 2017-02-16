//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_PARSER_H
#define SERWERHTTP_PARSER_H

#include "constants.h"
#include <vector>

using namespace std;

class Parser {
public:
    struct GETMessage{
        vector<unsigned char> parameters;
        vector<unsigned char> host;
        unsigned char* connection;
    };

    vector<unsigned char> messageCopy;
    int parseBrowserMessage(unsigned char *message);

    Parser(){}

    int findSubstring(const char *substring);

    void createGetStruct();
};


#endif //SERWERHTTP_PARSER_H
