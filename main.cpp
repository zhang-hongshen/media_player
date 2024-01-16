#include <spdlog/spdlog.h>

#include "core/avpacket_queue.h"
#include "core/frame_queue.h"
#include "core/demux.h"
#include "core/decoder.h"
#include "core/video_output.h"
#include "core/audio_output.h"
#include "core/mp_state.h"

#ifndef STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

int main(int argc, char* argv[]) {
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S][%^%l%$][pid %t] %v");
    spdlog::set_level(spdlog::level::debug);

    auto url = argv[1];
    /**
     * Demux
     */
    std::shared_ptr<MPState> mp_state = std::make_shared<MPState>();
    std::shared_ptr<AVPacketQueue> video_pkt_queue = std::make_shared<AVPacketQueue>();
    std::shared_ptr<AVPacketQueue> audio_pkt_queue = std::make_shared<AVPacketQueue>();
    std::shared_ptr<FrameQueue> video_frame_queue = std::make_shared<FrameQueue>();
    std::shared_ptr<FrameQueue> audio_frame_queue = std::make_shared<FrameQueue>();

    Decoder video_decoder(video_pkt_queue, video_frame_queue, &mp_state->serial);
    Decoder audio_decoder(audio_pkt_queue, audio_frame_queue, &mp_state->serial);

    Demux demux(video_pkt_queue, audio_pkt_queue, mp_state, &video_decoder, &audio_decoder);
    int ret = demux.Init(url);
    if(ret < 0) {
        spdlog::error("demux initialize error\n");
        return -1;
    }
    ret = demux.Start();
    if(ret < 0) {
        spdlog::error("demux start error\n");
        return -1;
    }

    /**
     * Decode
     */
    // video decode
    auto video_stream = demux.VideoStream();
    auto video_codec_param = video_stream->codecpar;
    ret = video_decoder.Init(video_codec_param);
    if(ret < 0) {
        spdlog::error("video decode thread initialize error\n");
        return -1;
    }
    ret = video_decoder.Start();
    if(ret < 0) {
        spdlog::error("video decode thread start error\n");
        return -1;
    }
    // audio decode
    auto audio_stream = demux.AudioStream();
    auto audio_codec_param = audio_stream->codecpar;
    ret = audio_decoder.Init(audio_codec_param);
    if(ret < 0) {
        spdlog::error("audio_decoder initialize error\n");
        return -1;
    }
    ret = audio_decoder.Start();
    if(ret < 0) {
        spdlog::error("audio_decoder start error\n");
        return -1;
    }

    /**
     * Output
     */
    // init audio output
    AudioParam audio_param{};
    audio_param.channel_layout = audio_codec_param->ch_layout;
    audio_param.format = static_cast<AVSampleFormat>(audio_codec_param->format);
    audio_param.sample_rate = audio_codec_param->sample_rate;
    audio_param.frame_size = audio_codec_param->frame_size;
    audio_param.time_base = audio_stream->time_base;

    AudioOutput audio(audio_frame_queue, audio_param, mp_state);
    ret = audio.Init();
    if(ret < 0) {
        spdlog::error("audio output initialize error\n");
        return -1;
    }
    // init video output
    VideoParam video_param{};
    video_param.height = video_codec_param->height;
    video_param.width = video_codec_param->width;
    video_param.time_base = video_stream->time_base;

    VideoOutput video(video_frame_queue, video_param, mp_state);
    ret = video.Init();
    if(ret < 0) {
        spdlog::error("video output initialize error\n");
        return -1;
    }
    video.EventLoop();

    /**
     * Waiting for decoding
     */
    ret = demux.Stop();
    if(ret < 0) {
        spdlog::error("demux stop error\n");
        return -1;
    }
    ret = video_decoder.Stop();
    if(ret < 0) {
        spdlog::error("video_decoder stop error\n");
        return -1;
    }
    ret = audio_decoder.Stop();
    if(ret < 0) {
        spdlog::error("audio_decoder stop error\n");
        return -1;
    }

    return 0;
}


