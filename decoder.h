//
// Created by 张鸿燊 on 9/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_DECODETHREAD_H
#define SIMPLEST_MEDIA_PLAYER_DECODETHREAD_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "utils/thread.h"
#include "avpacket_queue.h"
#include "avframe_queue.h"

class Decoder: public Thread {
public:
    Decoder(std::shared_ptr<AVPacketQueue> pkt_queue, std::shared_ptr<AVFrameQueue> frame_queue);
    ~Decoder() override;
    int Init(AVCodecParameters *param);
    int Start();
    int Stop();
protected:
    int Run() override;
private:
    std::shared_ptr<AVPacketQueue> pkt_queue_ = nullptr;
    std::shared_ptr<AVFrameQueue> frame_queue_ = nullptr;

    AVCodecContext *codec_ctx_ = nullptr;
};


#endif //SIMPLEST_MEDIA_PLAYER_DECODETHREAD_H
