//
// Created by tusia on 16.02.17.
//

#include "Parser.h"

int Parser::parseBrowserMessage(unsigned char* message){
    int i = 0;
    while(message[i]!='\0') {
        messageCopy.push_back(message[i]);
        i++;
    }
    if(findSubstring("favicon") != -1)
        return -1;

    if (findSubstring("GET ") != -1){
        createGetStruct();
    }
    return 0;

}
void Parser::createGetStruct(){
    int i=0;
    int index = findSubstring("?");
    if( index!=-1){
        index++;
        while(messageCopy[index]!=' '){
            parameters.push_back(messageCopy[index]);
            index++;
        }
    }
    index = findSubstring("/");
    if(index!=-1){
        index++;
        while(messageCopy[index]!=' ' and messageCopy[index]!='?'){
            uri.push_back(messageCopy[index]);
            index++;
        }
    }
    index = findSubstring("Host");
    if(index!=-1){
        i = index+6;
        while(messageCopy[i] != '\n'){
            host.push_back(messageCopy[i]);
            i++;
        }
    }


    cout<< "*****************PARSED MESSAGE!************** \n";
    cout << "parameters: ";
    for (int k=0;k<parameters.size();k++) cout <<parameters[k];
    cout << endl << "host: ";
    for (int k=0;k<host.size();k++) cout << host[k];
    cout << endl << "uri: ";
    for (int k=0;k<uri.size();k++) cout << uri[k];
    cout << endl;


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