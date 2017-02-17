//
// Created by tusia on 16.02.17.
//

#include "Record.h"
#include <fastcgi.h>

Record::Record(int array_size, int type, int request_id) : array_size(array_size), type(type), request_id(request_id) {
    this->message = new unsigned char[array_size];
}

void Record::fillHeader(int shift, int contentLength) {
    int paddingLength = (8 - contentLength % 8) % 8;
    message[VERSION + shift] = FCGI_VERSION_1;
    message[TYPE + shift] = (unsigned char) type;
    message[REQUEST_ID_B1 + shift] = (unsigned char) ((request_id >> 8) & 0xff);
    message[REQUEST_ID_B0 + shift] = (unsigned char) ((request_id) & 0xff);
    message[CONTENT_LENGTH_B1 + shift] = (unsigned char) ((contentLength >> 8) & 0xff);
    message[CONTENT_LENGTH_B0 + shift] = (unsigned char) ((contentLength) & 0xff);
    message[PADDING_LENGTH + shift] = (unsigned char) paddingLength;
    message[RESERVED + shift] = 0;

}


