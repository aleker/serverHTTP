//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_BEGINRECORD_H
#define SERWERHTTP_BEGINRECORD_H

#include "Record.h"

class BeginRecord : public Record {
public:

    BeginRecord(int array_size, int type, int request_id);

    void fillBeginRequestBody(int shift, int role, int flags);

    void fill(int role);
};


#endif //SERWERHTTP_BEGINRECORD_H
