//
// Created by root on 15.02.17.
//

#ifndef SERWERHTTP_CONSTANTS_H
#define SERWERHTTP_CONSTANTS_H

#include <stdio.h>

#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

#define HEADER_SIZE         8

#define VERSION             0
#define TYPE                1
#define REQUEST_ID_B1       2
#define REQUEST_ID_B0       3
#define CONTENT_LENGTH_B1   4
#define CONTENT_LENGTH_B0   5
#define PADDING_LENGTH      6
#define RESERVED            7

#endif //SERWERHTTP_CONSTANTS_H
