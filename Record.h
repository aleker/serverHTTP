//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_RECORD_H
#define SERWERHTTP_RECORD_H

#include <fastcgi.h>
#include "constants.h"

class Record {
public:
    int array_size;
    unsigned char *message;
    int type;
    int request_id;

    Record(int array_size, int type, int request_id);

    void fillHeader(int shift, int contentLength);
};


#endif //SERWERHTTP_RECORD_H
