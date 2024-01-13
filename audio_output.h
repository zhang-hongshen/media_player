//
// Created by 张鸿燊 on 10/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_AUDIOOUTPUT_H
#define SIMPLEST_MEDIA_PLAYER_AUDIOOUTPUT_H

extern "C" {
#include <libswresample/swresample.h>
#include <SDL.h>
}

#include "avframe_queue.h"
#include "avsync.h"
#include "mp_state.h"

struct AudioParam {

    AVChannelLayout channel_layout;
    AVSampleFormat format;
    int sample_rate;
    int frame_size;
    AVRational time_base;
};


class AudioOutput {
public:
    AudioOutput(std::shared_ptr<AVFrameQueue> q, const AudioParam &param, std::shared_ptr<MPState> mp_state);
    ~AudioOutput();
    int Init();
private:
    static void FillAudioPCM(void *userdata, Uint8 * stream, int len);
    bool CheckIfNeedResample(const AVFrame* frame) const;
    int InitSwrCtx(const AVFrame* frame);
    int Resample(const AVFrame* frame);
    void SetSyncClock();
private:
    std::shared_ptr<AVFrameQueue> frame_queue_ = nullptr;
    AudioParam src, dst;
    std::shared_ptr<MPState> mp_state_ = nullptr;
    enum {
        UNPAUSE = 0,
        PAUSE = 1
    };
    SwrContext *swr_ctx_ = nullptr;
    uint8_t *buf = nullptr;
    int buf_size = 0, buf_index = 0;
    uint8_t *buf1 = nullptr;
    int buf_size1 = 0;
    int64_t pts = AV_NOPTS_VALUE;
    constexpr static const int DEFAULT_BUF_SIZE = 512;
};


#endif //SIMPLEST_MEDIA_PLAYER_AUDIOOUTPUT_H
