//
// Created by tusia on 16.02.17.
//

#ifndef SERWERHTTP_STREAMRECORD_H
#define SERWERHTTP_STREAMRECORD_H

#include "Record.h"

class StreamRecord: public Record {
public:

    StreamRecord(int array_size, int type, int request_id, int contentLength);

    void fillContentData(int shift, unsigned char *content_data);
};


#endif //SERWERHTTP_STREAMRECORD_H
