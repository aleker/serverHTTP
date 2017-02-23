//
// Created by root on 15.02.17.
//

#ifndef SERWERHTTP_CONSTANTS_H
#define SERWERHTTP_CONSTANTS_H

#include <iostream>
#include <vector>
#include <string>

#define ASCII_SPACE             32
static const size_t bufsize = 1000;
static const unsigned char answerHeader[] = "HTTP/1.1 200 OK\r\n";
#define MAX_SIZE                61440.0

#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9

#define HEADER_SIZE             8
// HEADER:
#define VERSION                 0
#define TYPE                    1
#define REQUEST_ID_B1           2
#define REQUEST_ID_B0           3
#define CONTENT_LENGTH_B1       4
#define CONTENT_LENGTH_B0       5
#define PADDING_LENGTH          6
#define RESERVED                7

#define BEGIN_REQUEST_BODY_SIZE 8
// BEGIN REQUEST BODY:
#define ROLE_B1                 0
#define ROLE_B0                 1
#define FLAGS                   2
#define RESERVED_BEGIN          3

// ROLES:
#define FCGI_RESPONDER          1
#define FCGI_AUTHORIZER         2
#define FCGI_FILTER             3

// FLAGS:
#define FCGI_KEEP_CONN          1


static std::vector<std::string> CGI_params = {
        "GATEWAY_INTERFACE",    // CGI/1.1
        "SERVER_PROTOCOL",      // HTTP/1.1
        "REQUEST_METHOD",       // POST vs GET
        "REQUEST_URI",
        "QUERY_STRING",         // the part of URL after ? character (GET)
        "CONTENT_LENGTH",
        "CONTENT_TYPE",
};

#endif //SERWERHTTP_CONSTANTS_H
