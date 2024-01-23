//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H
#define SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


struct Frame {
    Frame(AVFrame *av_frame, int serial)
    : av_frame_(av_frame_alloc()), serial_(serial) {
        av_frame_move_ref(av_frame_, av_frame);
    }
    ~Frame() noexcept {
        if(av_frame_) {
            av_frame_free(&av_frame_);
        }
    }

    AVFrame *av_frame_;
    int serial_;
};


#endif //SIMPLEST_MEDIA_PLAYER_AVFRAMEQUEUE_H
