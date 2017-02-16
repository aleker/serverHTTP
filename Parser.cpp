//
// Created by tusia on 16.02.17.
//
#include <iostream>
#include <algorithm>
#include <string.h>
#include "Parser.h"

int Parser::parseBrowserMessage(unsigned char* message){
    int i = 0;
    std::vector<unsigned char> copy;
    while(message[i]!='\0') {
        copy.push_back(message[i]);
        i++;
    }
    messageCopy = copy;
    if (findSubstring("GET ") != -1){
        createGetStruct();
    }


    return 1;

}
void Parser::createGetStruct(){
    GETMessage getMsg = Parser::GETMessage();
    int i=0;
    int index = findSubstring("?");
    if( index!=-1){
        while(messageCopy[index]!=' '){
            getMsg.parameters.push_back(messageCopy[index]);
            index++;
        }
    }
    index = findSubstring("Host");
    if(index!=-1){
        i = index+6;
        while(messageCopy[i] != '\n'){
            getMsg.host.push_back(messageCopy[i]);
            i++;
        }
    }

}

int Parser::findSubstring(const char* substring){
    auto it = std::search(messageCopy.begin(), messageCopy.end(), substring, substring + strlen(substring));
    if (it != messageCopy.end()) {
        int index = (int) std::distance(messageCopy.begin(), it );
        return index;
    }
    else
        return -1;

}