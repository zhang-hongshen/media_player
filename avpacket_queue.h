//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H
#define SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "utils/queue.h"

class AVPacketQueue{
public:
    AVPacketQueue();
    ~AVPacketQueue();
    int push(AVPacket* packet);
    AVPacket* pop(int timeout = 0);
    AVPacket *front();
    size_t size();

private:
    void release();
    Queue<AVPacket*> q_;

};


#endif //SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H
