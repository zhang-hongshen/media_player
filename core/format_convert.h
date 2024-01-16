//
// Created by 张鸿燊 on 13/1/2024.
//

#ifndef MEDIA_PLAYER_FORMAT_CONVERT_H
#define MEDIA_PLAYER_FORMAT_CONVERT_H


extern "C" {
#include <libavutil/samplefmt.h>
#include <SDL.h>
};

SDL_AudioFormat GetSDLAudioFormat(enum AVSampleFormat format) {
    switch (format) {
        case AV_SAMPLE_FMT_U8:
        case AV_SAMPLE_FMT_U8P:
            return AUDIO_U8;
        case AV_SAMPLE_FMT_S16:
        case AV_SAMPLE_FMT_S16P:
            return AUDIO_S16;
        case AV_SAMPLE_FMT_S32:
        case AV_SAMPLE_FMT_S32P:
            return AUDIO_S32;
        case AV_SAMPLE_FMT_FLT:
        case AV_SAMPLE_FMT_FLTP:
            return AUDIO_F32;
        default:
            return AV_SAMPLE_FMT_S16;
    }
}

AVSampleFormat GetAVSampleFormat(SDL_AudioFormat format) {
    switch (format) {
        case AUDIO_U8:
            return AV_SAMPLE_FMT_U8;
        case AUDIO_S16:
            return AV_SAMPLE_FMT_S16;
        case AUDIO_S32:
            return AV_SAMPLE_FMT_S32;
        case AUDIO_F32:
            return AV_SAMPLE_FMT_FLT;
        default:
            return AV_SAMPLE_FMT_S16;
    }
}


#endif //MEDIA_PLAYER_FORMAT_CONVERT_H
