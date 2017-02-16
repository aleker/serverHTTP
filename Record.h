//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_RECORD_H
#define SERWERHTTP_RECORD_H

#include <fastcgi.h>


class Record {
public:
    int array_size;
    unsigned char *message;
    int type;
    int request_id;
    int contentLength;
    int paddingLength;

    Record(int array_size, int type, int request_id, int contentLength);

    void fillHeader(int shift);
};


#endif //SERWERHTTP_RECORD_H
