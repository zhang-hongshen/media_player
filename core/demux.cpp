//
// Created by 张鸿燊 on 8/1/2024.
//

#include <spdlog/spdlog.h>

#include "demux.h"

Demux::Demux(const std::shared_ptr<Queue<std::shared_ptr<Packet>>> &video_pkt_queue,
             const std::shared_ptr<Queue<std::shared_ptr<Packet>>> &audio_pkt_queue,
             const std::shared_ptr<MPState> &mp_state, Decoder *video_decoder, Decoder *audio_decoer)
    : video_pkt_queue_(video_pkt_queue), audio_pkt_queue_(audio_pkt_queue), mp_state_(mp_state),
      video_decoder_(video_decoder), audio_decoder_(audio_decoer){
    spdlog::info("Demux::Demux()\n");
}

Demux::~Demux() {
    spdlog::info("Demux::~Demux()\n");
    if(format_ctx_) {
        avformat_close_input(&format_ctx_);
    }
}

int Demux::Init(const char *url) {
    // open media file
    format_ctx_ = avformat_alloc_context();
    if(!format_ctx_) {
        spdlog::error("avformat_alloc_context error\n");
        return -1;
    }
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
    thread = std::make_unique<Thread>(&Demux::Run, this);
    if(!thread) {
        spdlog::error("new thread error\n");
        return -1;
    }
    return 0;
}

int Demux::Stop() {
    return thread->Stop();
}

int Demux::Run() {
    AVPacket* pkt = av_packet_alloc();
    int ret;
    while (!thread->Aborted()) {
        if(mp_state_->need_seek) {
            ret = avformat_seek_file(format_ctx_, -1,  std::numeric_limits<int64_t>::min(),
                                     mp_state_->seek_pts * 1000, std::numeric_limits<int64_t>::max(), 0);
            if(ret < 0) {
                spdlog::error("avformat_seek_file error, {}\n", av_err2str(ret));
                return -1;
            }
            // clear frame queue and codec buffer
            video_pkt_queue_->clear();
            audio_pkt_queue_->clear();
            video_decoder_->ClearFrameQueue();
            audio_decoder_->ClearFrameQueue();
            video_decoder_->FlushCodecBuffer();
            audio_decoder_->FlushCodecBuffer();

            // change syncer pts
            ++(mp_state_->serial);
            mp_state_->SetClock(mp_state_->seek_pts);
            mp_state_->need_seek = false;

        } else {
            ret = av_read_frame(format_ctx_, pkt);
            if(ret < 0) {
                spdlog::error("av_read_frame error, {}\n", av_err2str(ret));
                break;
            }
            if (pkt->stream_index == video_stream_index_) {
                video_pkt_queue_->push(std::make_shared<Packet>(pkt));
            } else if(pkt->stream_index == audio_stream_index_) {
                audio_pkt_queue_->push(std::make_shared<Packet>(pkt));
            }
            av_packet_unref(pkt);
        }
    }
    av_packet_free(&pkt);
    spdlog::info("Demux::run() finished!\n");
    return 0;
}

AVStream* Demux::VideoStream() const {
    AVStream *stream = nullptr;
    if(video_stream_index_ >= 0) {
       stream = format_ctx_->streams[video_stream_index_];
    }
    return stream;
}

AVStream* Demux::AudioStream() const {
    AVStream *stream = nullptr;
    if(audio_stream_index_ >= 0) {
        stream = format_ctx_->streams[audio_stream_index_];
    }
    return stream;
}
