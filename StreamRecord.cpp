//
// Created by tusia on 16.02.17.
//

#include "StreamRecord.h"

StreamRecord::StreamRecord(int array_size, int type, int request_id) : Record(array_size, type, request_id) {}

void StreamRecord::fillContentData(int shift, unsigned char *content_data, int contentLength) {
    int paddingLength = (8 - contentLength % 8) % 8;
    for (int i = 0; i < contentLength; i++) {
        message[i + shift] = content_data[i];
    }
    for (int i = shift + contentLength; i < (shift + contentLength + paddingLength); i++) {
        message[i] = 0;
    }
}

void StreamRecord::fill(int message_size, unsigned char *data) {
    fillHeader(0, message_size);
    if (message_size == 0) return;
    fillContentData(HEADER_SIZE, data, message_size);

}