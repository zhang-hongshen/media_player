//
// Created by 张鸿燊 on 8/1/2024.
//
#include <spdlog/spdlog.h>

#include "frame_queue.h"

FrameQueue::FrameQueue(size_t cap) {
    q = std::make_unique<Queue<std::shared_ptr<Frame>>>(cap);
}


FrameQueue::~FrameQueue() {
    release();
}

int FrameQueue::push(const std::shared_ptr<Frame>& frame) {
    return q->push(frame);
}

int FrameQueue::push(std::shared_ptr<Frame>&& frame) {

    return q->push(frame);
}

std::shared_ptr<Frame> FrameQueue::pop(int timeout) {
    std::shared_ptr<Frame> res = nullptr;
    q->pop(res, timeout);
    return res;
}

std::shared_ptr<Frame> FrameQueue::front() {
    std::shared_ptr<Frame> res = nullptr;
    q->front(res);
    return res;
}


size_t FrameQueue::size() {
    return q->size();
}

void FrameQueue::clear() {
    release();
}

void FrameQueue::release() {
    while (true) {
        std::shared_ptr<Frame> frame = nullptr;
        int ret = q->pop(frame, 5);
        if(0 != ret) {
            break;
        }
    }
}