//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_PARSER_H
#define SERWERHTTP_PARSER_H

#include "constants.h"
#include <vector>
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include <iostream>

using namespace std;

class Parser {
public:
    struct GETMessage{
        vector<unsigned char> parameters;
        vector<unsigned char> host;
        vector<unsigned char> uri;
        int favicon;
    };

    vector<unsigned char> messageCopy;
    Parser(){}

    int parseBrowserMessage(unsigned char *message);
    int findSubstring(const char *substring);

    void createGetStruct();
};


#endif //SERWERHTTP_PARSER_H
