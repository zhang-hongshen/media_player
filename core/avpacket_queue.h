//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H
#define SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "queue.h"

struct Packet {
    explicit Packet(AVPacket *av_packet)
    : av_packet_(av_packet_alloc()) {
        av_packet_move_ref(av_packet_, av_packet);
    }
    ~Packet() {
        if(av_packet_) {
            av_packet_free(&av_packet_);
        }
    }

    AVPacket *av_packet_;
};


class AVPacketQueue{
public:
    AVPacketQueue(size_t cap);
    ~AVPacketQueue();
    int push(std::shared_ptr<Packet> pkt);
    std::shared_ptr<Packet> pop(int timeout = 0);
    std::shared_ptr<Packet> front();
    size_t size();
    void clear();
private:
    void release();
    std::unique_ptr<Queue<std::shared_ptr<Packet>>> q_;
};


#endif //SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H
