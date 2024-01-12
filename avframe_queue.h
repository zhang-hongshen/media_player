//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H
#define SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "utils/queue.h"

class AVFrameQueue {
public:
    AVFrameQueue();
    ~AVFrameQueue();
    int push(AVFrame* frame);
    AVFrame* front();
    AVFrame* pop(int timeout = 0);
    size_t size();
private:
    void release();
    Queue<AVFrame*> q;

};


#endif //SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H
