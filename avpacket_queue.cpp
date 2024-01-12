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
int AVPacketQueue::push(AVPacket* frame) {
    AVPacket* e = av_packet_alloc();
    av_packet_move_ref(e, frame);
    return q_.push(e);
}

AVPacket* AVPacketQueue::pop(int timeout) {
    AVPacket *res = nullptr;
    q_.pop(res, timeout);
    return res;
}

AVPacket* AVPacketQueue::front() {
    AVPacket *res = nullptr;
    int ret = q_.front(res);
    return res;
}


size_t AVPacketQueue::size() {
    return q_.size();
}

void AVPacketQueue::release() {
    while (true) {
        AVPacket *pkt = nullptr;
        int ret = q_.pop(pkt, 10);
        if(-1 == ret) {
            break;
        }
        if(pkt) {
            av_packet_free(&pkt);
        }
    }
}