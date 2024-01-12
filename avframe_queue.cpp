//
// Created by 张鸿燊 on 8/1/2024.
//
#include <spdlog/spdlog.h>

#include "avframe_queue.h"

AVFrameQueue::AVFrameQueue() {
}


AVFrameQueue::~AVFrameQueue() {
    release();
}

int AVFrameQueue::push(AVFrame* frame) {
    AVFrame* e = av_frame_alloc();
    av_frame_move_ref(e, frame);
    return q.push(e);
}

AVFrame* AVFrameQueue::pop(int timeout) {
    AVFrame *res = nullptr;
    q.pop(res, timeout);
    return res;
}

AVFrame* AVFrameQueue::front() {
    AVFrame *res = nullptr;
    q.front(res);
    return res;
}

size_t AVFrameQueue::size() {
    return q.size();
}

void AVFrameQueue::release() {
    while (true) {
        AVFrame *frame = nullptr;
        int ret = q.pop(frame, 1);
        if(-1 == ret) {
            break;
        }
        if(frame) {
            av_frame_free(&frame);
        }
    }
}