//
// Created by 张鸿燊 on 10/1/2024.
//
#include "audio_output.h"

#include <spdlog/spdlog.h>


#include "format_convert.h"


AudioOutput::AudioOutput(const std::shared_ptr<FrameQueue>& q, const AudioParam &param,
                         const std::shared_ptr<MPState>& mp_state):
    frame_queue_(q), src(param), mp_state_(mp_state) {
}

AudioOutput::~AudioOutput() {
    if(swr_ctx_) {
        swr_free(&swr_ctx_);
    }
    delete buf, buf1;
}


int AudioOutput::Init() {
    if(SDL_Init(SDL_INIT_AUDIO) != 0) {
        spdlog::error("SDL audio initialize error, {}\n", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec desired;
    desired.channels = src.channel_layout.nb_channels;
    desired.freq = src.sample_rate;
    desired.format = GetSDLAudioFormat(src.format);
    desired.silence = 0;
    desired.callback = AudioOutput::FillAudioPCM;
    desired.userdata = this;
    desired.samples = src.frame_size;

    SDL_AudioSpec obtained;
    int ret = SDL_OpenAudio(&desired, &obtained);
    if(0 != ret) {
        spdlog::error("SDL open audio error, {}\n", SDL_GetError());
        return -1;
    }
    dst.channel_layout.nb_channels = obtained.channels;
    dst.format = GetAVSampleFormat(obtained.format);
    dst.sample_rate = obtained.freq;
    av_channel_layout_default(&dst.channel_layout, src.channel_layout.nb_channels);
    dst.frame_size = obtained.samples;
    dst.time_base = src.time_base;

    SDL_PauseAudio(UNPAUSE);

    spdlog::info("AudioOutput::Init() finished \n");
    return 0;
}

void AudioOutput::FillAudioPCM(void *userdata, Uint8 * stream, int len) {
    auto output = reinterpret_cast<AudioOutput*>(userdata);
    int ret;
    while(len > 0) {
        if(output->buf_index >= output->buf_size) {
            if(output->mp_state_->paused) {
                output->buf = nullptr;
                output->buf_size = DEFAULT_BUF_SIZE;
            } else {
                auto frame = output->frame_queue_->pop(10);
                if(frame->serial_ != output->mp_state_->serial) {
                    spdlog::info("audio frame serial not equal, expected {}, actual {}\n", output->mp_state_->serial, frame->serial_);
                    return;
                }
                if(!frame) {
                    output->buf = nullptr;
                    output->buf_size = DEFAULT_BUF_SIZE;
                } else {
                    AVFrame *av_frame = frame->av_frame_;
                    output->pts = av_frame->pts;
                    if(!output->swr_ctx_ && output->CheckIfNeedResample(av_frame)) {
                        if(output->InitSwrCtx(av_frame) < 0) {
                            spdlog::error("InitSwrCtx error\n");
                            swr_free(&output->swr_ctx_);
                            return;
                        }
                    }
                    if(!output->swr_ctx_) {
                        auto audio_size = av_samples_get_buffer_size(nullptr, av_frame->ch_layout.nb_channels, av_frame->nb_samples,
                                                                     static_cast<AVSampleFormat>(av_frame->format), 1);
                        av_fast_malloc(&output->buf1, reinterpret_cast<unsigned int *>(&output->buf_size1), audio_size);
                        output->buf = output->buf1;
                        output->buf_size = audio_size;
                        memcpy(output->buf, av_frame->data[0], audio_size);
                    } else {
                        ret = output->Resample(av_frame);
                        if(ret < 0) {
                            spdlog::error("Resample error\n");
                            return;
                        }
                    }
                }
            }

            output->buf_index = 0;
        }
        int readLen = std::min(len, output->buf_size - output->buf_index);
        if(!output->mp_state_->muted && output->mp_state_->volume == SDL_MIX_MAXVOLUME && output->buf) {
            memcpy(stream, output->buf + output->buf_index, readLen);
        } else {
            memset(stream, 0, readLen);
            if(!output->mp_state_->muted && output->buf) {
                SDL_MixAudioFormat(stream, output->buf + output->buf_index, GetSDLAudioFormat(output->dst.format), readLen, output->mp_state_->volume);
            }
        }
        len -= readLen;
        stream += readLen;
        output->buf_index += readLen;
        output->SetSyncClock();
    }

    if(output->swr_ctx_) {
        swr_free(&output->swr_ctx_);
    }
}

void AudioOutput::SetSyncClock() {
    if(AV_NOPTS_VALUE != pts) {
        mp_state_->SetClock(pts * av_q2d(dst.time_base));
    }
}

bool AudioOutput::CheckIfNeedResample(const AVFrame* frame) const{
    if(frame->format != dst.format
        || frame->sample_rate != dst.sample_rate / mp_state_->GetPlayBackSpeed()
        || frame->ch_layout.nb_channels != dst.channel_layout.nb_channels) {
        return true;
    }
    return false;
}

int AudioOutput::InitSwrCtx(const AVFrame* frame) {
    int ret = swr_alloc_set_opts2(&(swr_ctx_), &(dst.channel_layout),
                              dst.format, dst.sample_rate / mp_state_->GetPlayBackSpeed(), &(frame->ch_layout),
                              static_cast<AVSampleFormat>(frame->format),
                              frame->sample_rate, 0, nullptr);
    if(0 != ret) {
        spdlog::error("swr_alloc_set_opts2 error, {}\n", av_err2str(ret));
        return -1;
    }
    ret = swr_init(swr_ctx_);
    if(ret < 0) {
        spdlog::error("swr_init error, {}\n", av_err2str(ret));
        return -1;
    }
    return 0;
}

int AudioOutput::Resample(const AVFrame* frame) {
    int out_samples = frame->nb_samples * dst.sample_rate / frame->sample_rate + 256;
    int resampled_data_size = av_samples_get_buffer_size(nullptr, dst.channel_layout.nb_channels,
                                               out_samples, dst.format,  0);
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
    resampled_data_size = av_samples_get_buffer_size(nullptr, dst.channel_layout.nb_channels, out_samples,
                                           dst.format, 1);
    if(resampled_data_size < 0) {
        spdlog::error("av_samples_get_buffer_size error\n");
        return -1;
    }
    buf_size = resampled_data_size;
    return 0;
}