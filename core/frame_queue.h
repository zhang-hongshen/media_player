//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H
#define SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "queue.h"

struct Frame {
    Frame(AVFrame *av_frame, int serial)
    : av_frame_(av_frame_alloc()), serial_(serial) {
        av_frame_move_ref(av_frame_, av_frame);
    }
    ~Frame() {
        if(av_frame_) {
            av_frame_free(&av_frame_);
        }
    }

    AVFrame *av_frame_;
    int serial_;
};


class FrameQueue {
public:
    FrameQueue(size_t cap);
    ~FrameQueue();
    int push(const std::shared_ptr<Frame>& frame);
    int push(std::shared_ptr<Frame>&& frame);
    std::shared_ptr<Frame> front();
    std::shared_ptr<Frame> pop(int timeout = 0);

    size_t size();
    void clear();
private:
    void release();
    std::unique_ptr<Queue<std::shared_ptr<Frame>>> q;
};


#endif //SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H
