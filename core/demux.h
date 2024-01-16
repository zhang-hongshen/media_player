//
// Created by 张鸿燊 on 8/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_DEMUXTHREAD_H
#define SIMPLEST_MEDIA_PLAYER_DEMUXTHREAD_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include "thread.h"
#include "avpacket_queue.h"
#include "mp_state.h"
#include "decoder.h"

class Demux : public Thread {
public:
    Demux(const std::shared_ptr<AVPacketQueue>& video_pkt_queue,
          const std::shared_ptr<AVPacketQueue>& audio_pkt_queue,
          const std::shared_ptr<MPState>& mp_state, Decoder *video_decoder, Decoder *audio_decoer);
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
    std::shared_ptr<MPState> mp_state_ = nullptr;
    Decoder *video_decoder_, *audio_decoder_;

    AVFormatContext* format_ctx_ = nullptr;
    int video_stream_index_ = -1;
    int audio_stream_index_ = -1;
};


#endif //SIMPLEST_MEDIA_PLAYER_DEMUXTHREAD_H
