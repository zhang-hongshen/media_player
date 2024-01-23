//
// Created by 张鸿燊 on 9/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_DECODETHREAD_H
#define SIMPLEST_MEDIA_PLAYER_DECODETHREAD_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "thread.h"
#include "packet.h"
#include "frame.h"
#include "queue.h"

class Decoder {
public:
    Decoder(const std::shared_ptr<Queue<std::shared_ptr<Packet>>> &pkt_queue,
            const std::shared_ptr<Queue<std::shared_ptr<Frame>>> &frame_queue, int* serial);
    ~Decoder() noexcept;
    int Init(AVCodecParameters *param);
    int Start();
    int Stop();
    void ClearFrameQueue();
    void FlushCodecBuffer();
protected:
    int Run();
private:
    std::unique_ptr<Thread> thread = nullptr;
    std::shared_ptr<Queue<std::shared_ptr<Packet>>> pkt_queue_ = nullptr;
    std::shared_ptr<Queue<std::shared_ptr<Frame>>> frame_queue_ = nullptr;
    int* serial_ = nullptr;
    AVCodecContext *codec_ctx_ = nullptr;
};


#endif //SIMPLEST_MEDIA_PLAYER_DECODETHREAD_H
