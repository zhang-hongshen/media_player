//
// Created by 张鸿燊 on 10/1/2024.
//
#include "audio_output.h"

#include <spdlog/spdlog.h>


#include "format_convert.h"


AudioOutput::AudioOutput(const std::shared_ptr<Queue<std::shared_ptr<Frame>>>& q, const AudioParam &param,
                         const std::shared_ptr<MPState>& mp_state):
    frame_queue_(q), in(param), mp_state_(mp_state) {
}

AudioOutput::~AudioOutput() {
    swr_free(&swr_ctx_);
    delete buf, buf1;
}


int AudioOutput::Init() {
    if(SDL_Init(SDL_INIT_AUDIO) != 0) {
        spdlog::error("SDL audio initialize error, {}\n", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec desired;
    desired.channels = in.channel_layout.nb_channels;
    desired.freq = in.sample_rate;
    desired.format = GetSDLAudioFormat(in.format);
    desired.silence = 0;
    desired.callback = AudioOutput::FillAudioPCM;
    desired.userdata = this;
    desired.samples = in.frame_size;

    SDL_AudioSpec obtained;
    int ret = SDL_OpenAudio(&desired, &obtained);
    if(0 != ret) {
        spdlog::error("SDL open audio error, {}\n", SDL_GetError());
        return -1;
    }
    out.channel_layout.nb_channels = obtained.channels;
    out.format = GetAVSampleFormat(obtained.format);
    out.sample_rate = obtained.freq;
    av_channel_layout_default(&out.channel_layout, in.channel_layout.nb_channels);
    out.frame_size = obtained.samples;
    out.time_base = in.time_base;
    playback_sample_rate = out.sample_rate;
    SDL_PauseAudio(0);

    spdlog::info("AudioOutput::Init() finished \n");
    return 0;
}

void AudioOutput::FillAudioPCM(void *userdata, Uint8 * stream, int len) {
    auto op = reinterpret_cast<AudioOutput*>(userdata);
    int ret;
    while(len > 0) {
        if(op->buf_index >= op->buf_size) {
            if(op->mp_state_->paused) {
                op->buf = nullptr;
                op->buf_size = DEFAULT_BUF_SIZE;
            } else {
                auto frame = op->frame_queue_->front();
                if(!frame) {
                    op->buf = nullptr;
                    op->buf_size = DEFAULT_BUF_SIZE;
                } else {
                    op->frame_queue_->pop(10);
                    if(frame->serial_ != op->mp_state_->serial) {
                        spdlog::info("audio frame serial not equal, expected {}, actual {}\n", op->mp_state_->serial, frame->serial_);
                        return;
                    }
                    AVFrame *av_frame = frame->av_frame_;
                    op->pts = av_frame->pts;
                    if(op->CheckIfNeedResample(av_frame) && op->InitSwrCtx(av_frame) < 0) {
                        spdlog::error("InitSwrCtx error\n");
                        return;
                    }
                    if(!op->swr_ctx_) {
                        auto audio_size = av_samples_get_buffer_size(nullptr, av_frame->ch_layout.nb_channels, av_frame->nb_samples,
                                                                     static_cast<AVSampleFormat>(av_frame->format), 1);
                        av_fast_malloc(&op->buf1, reinterpret_cast<unsigned int *>(&op->buf_size1), audio_size);
                        op->buf = op->buf1;
                        op->buf_size = audio_size;
                        memcpy(op->buf, av_frame->data[0], audio_size);
                    } else {
                        ret = op->Resample(av_frame);
                        if(ret < 0) {
                            spdlog::error("Resample error\n");
                            return;
                        }
                    }
                }
            }
            op->buf_index = 0;
        }
        int readLen = std::min(len, op->buf_size - op->buf_index);
        if(!op->mp_state_->muted && op->mp_state_->volume == SDL_MIX_MAXVOLUME && op->buf) {
            memcpy(stream, op->buf + op->buf_index, readLen);
        } else {
            memset(stream, 0, readLen);
            if(!op->mp_state_->muted && op->buf) {
                SDL_MixAudioFormat(stream, op->buf + op->buf_index, GetSDLAudioFormat(op->out.format), readLen, op->mp_state_->volume);
            }
        }
        len -= readLen;
        stream += readLen;
        op->buf_index += readLen;
        op->SetSyncClock();
    }
}

void AudioOutput::SetSyncClock() {
    if(AV_NOPTS_VALUE != pts) {
        mp_state_->SetClock(pts * av_q2d(out.time_base));
    }
}

bool AudioOutput::CheckIfNeedResample(const AVFrame* frame) const{
    if(frame->format != out.format
        || frame->sample_rate != playback_sample_rate
        || frame->ch_layout.nb_channels != out.channel_layout.nb_channels) {
        return true;
    }
    return false;
}

int AudioOutput::InitSwrCtx(const AVFrame* frame) {
    swr_free(&swr_ctx_);
    int target_sample_rate = out.sample_rate / mp_state_->GetPlayBackSpeed();
    int ret = swr_alloc_set_opts2(&(swr_ctx_), &(out.channel_layout),
                              out.format, target_sample_rate, &(frame->ch_layout),
                              static_cast<AVSampleFormat>(frame->format),
                              frame->sample_rate, 0, nullptr);
    if(0 != ret) {
        spdlog::error("swr_alloc_set_opts2 error, {}\n", av_err2str(ret));
        return -1;
    }
    ret = swr_init(swr_ctx_);
    if(ret < 0) {
        swr_free(&swr_ctx_);
        spdlog::error("swr_init error, {}\n", av_err2str(ret));
        return -1;
    }
    playback_sample_rate = target_sample_rate;
    return 0;
}

int AudioOutput::Resample(const AVFrame* frame) {
    int out_samples = frame->nb_samples * out.sample_rate / frame->sample_rate + 256;
    int resampled_data_size = av_samples_get_buffer_size(nullptr, out.channel_layout.nb_channels,
                                               out_samples, out.format,  0);
    if(resampled_data_size < 0) {
        spdlog::error("av_samples_get_buffer_size error\n");
        return -1;
    }
    av_fast_malloc(&buf1, reinterpret_cast<unsigned int *>(&buf_size1), resampled_data_size);
    if(!buf1) {
        spdlog::error("av_fast_malloc error\n");
        return -1;
    }
    out_samples = swr_convert(swr_ctx_, &buf1, out_samples,
                              const_cast<const uint8_t **>(frame->extended_data), frame->nb_samples);
    if(out_samples < 0) {
        spdlog::error("swr_convert error\n");
        return -1;
    }
    buf = buf1;
    resampled_data_size = av_samples_get_buffer_size(nullptr, out.channel_layout.nb_channels, out_samples,
                                           out.format, 1);
    if(resampled_data_size < 0) {
        spdlog::error("av_samples_get_buffer_size error\n");
        return -1;
    }
    buf_size = resampled_data_size;
    return 0;
}