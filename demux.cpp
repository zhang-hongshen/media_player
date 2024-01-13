//
// Created by 张鸿燊 on 8/1/2024.
//

#include <spdlog/spdlog.h>

#include "demux.h"

Demux::Demux(std::shared_ptr<AVPacketQueue> video_pkt_queue, std::shared_ptr<AVPacketQueue> audio_pkt_queue)
    : video_pkt_queue_(video_pkt_queue), audio_pkt_queue_(audio_pkt_queue), format_ctx_(avformat_alloc_context()){
    spdlog::info("Demux::Demux()\n");
}

Demux::~Demux() {
    spdlog::info("Demux::~Demux()\n");
    avformat_close_input(&format_ctx_);
}

int Demux::Init(const char *url) {
    // open media file
    int ret = avformat_open_input(&format_ctx_, url, nullptr, nullptr);
    if (0 != ret) {
        spdlog::error("avformat_open_input error, {}\n", av_err2str(ret));
        return -1;
    }
    ret = avformat_find_stream_info(format_ctx_, nullptr);
    if (ret < 0) {
        spdlog::error("avformat_find_stream_info error, {}\n", av_err2str(ret));
        return -1;
    }

    av_dump_format(format_ctx_, 0, url, 0);

    // find video stream
    video_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_index_ < 0) {
        spdlog::error("Couldn't find a video stream\n");
        return -1;
    }
    // find audio stream
    audio_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (audio_stream_index_ < 0) {
        spdlog::error("Couldn't find a audio stream\n");
        return -1;
    }

    spdlog::info("Demux::init() finished\n");

    return 0;
}

int Demux::Start() {
    thread = new std::thread(&Demux::Run, this);
    if(!thread) {
        spdlog::error("new thread error\n");
        return -1;
    }
    return 0;
}

int Demux::Stop() {
    return Thread::Stop();
}

int Demux::Run() {
    AVPacket* packet = av_packet_alloc();
    int ret;
    while (Thread::EXIT != abort_) {
        ret = av_read_frame(format_ctx_, packet);
        if(ret < 0) {
            spdlog::error("av_read_frame error, {}\n", av_err2str(ret));
            break;
        }
        if (packet->stream_index == video_stream_index_) {
            video_pkt_queue_->push(packet);

        } else if(packet->stream_index == audio_stream_index_) {
            audio_pkt_queue_->push(packet);
        }
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
    spdlog::info("Demux::run() finished!\n");
    abort_ = Thread::EXIT;
    return 0;
}

AVStream* Demux::VideoStream() {
    AVStream *stream = nullptr;
    if(video_stream_index_ >= 0) {
       stream = format_ctx_->streams[video_stream_index_];
    }
    return stream;
}

AVStream* Demux::AudioStream() {
    AVStream *stream = nullptr;
    if(audio_stream_index_ >= 0) {
        stream = format_ctx_->streams[audio_stream_index_];
    }
    return stream;
}
