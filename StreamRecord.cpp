//
// Created by tusia on 16.02.17.
//

#include "StreamRecord.h"

StreamRecord::StreamRecord(int array_size, int type, int request_id, int contentLength) :
        Record(array_size, type, request_id, contentLength) {}

void StreamRecord::fillContentData(int shift, unsigned char* content_data) {
    for (int i = 0; i < contentLength; i++) {
        message[i + shift] = content_data[i];
    }
    for (int i = shift + contentLength; i < (shift + contentLength + paddingLength); i++) {
        message[i] = 0;
    }
}