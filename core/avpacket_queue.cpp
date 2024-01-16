//
// Created by 张鸿燊 on 8/1/2024.
//
#include <spdlog/spdlog.h>

#include "avpacket_queue.h"

AVPacketQueue::AVPacketQueue() {
}

AVPacketQueue::~AVPacketQueue() {
    spdlog::info("AVPacketQueue::~AVPacketQueue() \n");
    release();
}

/**
 *
 * @param frame
 * @param timeout
 * @return 0 if success, negative if failed
 */
int AVPacketQueue::push(std::shared_ptr<Packet> pkt) {
    return q_.push(pkt);
}

std::shared_ptr<Packet> AVPacketQueue::pop(int timeout) {
    std::shared_ptr<Packet> res = nullptr;
    q_.pop(res, timeout);
    return res;
}

std::shared_ptr<Packet> AVPacketQueue::front() {
    std::shared_ptr<Packet> res = nullptr;
    int ret = q_.front(res);
    return res;
}


size_t AVPacketQueue::size() {
    return q_.size();
}

void AVPacketQueue::clear() {
    release();
}

void AVPacketQueue::release() {
    while (true) {
        std::shared_ptr<Packet> pkt = nullptr;
        int ret = q_.pop(pkt, 10);
        if(-1 == ret) {
            break;
        }
    }
}