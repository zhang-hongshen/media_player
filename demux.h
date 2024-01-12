//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_DEMUXTHREAD_H
#define SIMPLEST_MEDIA_PLAYER_DEMUXTHREAD_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "utils/thread.h"
#include "avpacket_queue.h"

class Demux : public Thread {
public:
    Demux(std::shared_ptr<AVPacketQueue> video_pkt_queue, std::shared_ptr<AVPacketQueue> audio_pkt_queue);
    ~Demux() override;
    int Init(const char *url);
    int Start();
    int Stop();
    AVStream* VideoStream();
    AVStream* AudioStream();
protected:
    int Run() override;
private:
    std::shared_ptr<AVPacketQueue> video_pkt_queue_ = nullptr;
    std::shared_ptr<AVPacketQueue> audio_pkt_queue_ = nullptr;
    AVFormatContext* format_ctx_ = nullptr;
    int video_stream_index_ = -1;
    int audio_stream_index_ = -1;
};


#endif //SIMPLEST_MEDIA_PLAYER_DEMUXTHREAD_H
