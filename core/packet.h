//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H
#define SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

struct Packet {
    explicit Packet(AVPacket *av_packet)
    : av_packet_(av_packet_alloc()) {
        av_packet_move_ref(av_packet_, av_packet);
    }
    ~Packet() noexcept {
        if(av_packet_) {
            av_packet_free(&av_packet_);
        }
    }

    AVPacket *av_packet_;
};



#endif //SIMPLEST_MEDIA_PLAYER_AVPACKETQUEUE_H
