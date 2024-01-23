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
#include "queue.h"
#include "mp_state.h"
#include "decoder.h"

class Demux {
public:
    Demux(const std::shared_ptr<Queue<std::shared_ptr<Packet>>> &video_pkt_queue,
          const std::shared_ptr<Queue<std::shared_ptr<Packet>>> & audio_pkt_queue,
          const std::shared_ptr<MPState>& mp_state, Decoder *video_decoder, Decoder *audio_decoer);
    ~Demux() noexcept;
    int Init(const char *url);
    int Start();
    int Stop();
    AVStream* VideoStream() const;
    AVStream* AudioStream() const;
private:
    int Run();
private:
    std::unique_ptr<Thread> thread = nullptr;
    std::shared_ptr<Queue<std::shared_ptr<Packet>>> video_pkt_queue_ = nullptr;
    std::shared_ptr<Queue<std::shared_ptr<Packet>>> audio_pkt_queue_ = nullptr;
    std::shared_ptr<MPState> mp_state_ = nullptr;
    Decoder *video_decoder_, *audio_decoder_;

    AVFormatContext* format_ctx_ = nullptr;
    int video_stream_index_ = -1;
    int audio_stream_index_ = -1;
};


#endif //SIMPLEST_MEDIA_PLAYER_DEMUXTHREAD_H
