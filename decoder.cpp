//
// Created by 张鸿燊 on 9/1/2024.
//
#include <spdlog/spdlog.h>

#include "decoder.h"

Decoder::Decoder(std::shared_ptr<AVPacketQueue> pkt_queue, std::shared_ptr<AVFrameQueue> frame_queue):
        pkt_queue_(pkt_queue), frame_queue_(frame_queue) {

}

Decoder::~Decoder() {
    avcodec_free_context(&codec_ctx_);
}

int Decoder::Init(AVCodecParameters *param) {
    if(!param) {
        spdlog::error("param is null\n");
        return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(nullptr);
    if(!codec_ctx_) {
        spdlog::error("codec_ctx_ is null\n");
        return -1;
    }
    int ret = avcodec_parameters_to_context(codec_ctx_, param);
    if(ret < 0) {
        spdlog::error("avcodec_parameters_to_context error, {}\n", av_err2str(ret));
        return -1;
    }
    auto codec = avcodec_find_decoder(codec_ctx_->codec_id);
    if(!codec) {
        spdlog::error("Couldn't find codec\n");
        return -1;
    }
    ret = avcodec_open2(codec_ctx_, codec, nullptr);
    if(ret < 0) {
        spdlog::error("avcodec_open2 error, {}\n", av_err2str(ret));
        return -1;
    }
    spdlog::info("Decoder::init() finished\n");
    return 0;
}

int Decoder::Start() {
    thread = new std::thread(&Decoder::Run, this);
    if(!thread) {
        spdlog::error("new thread error\n");
        return -1;
    }
    return 0;
}

int Decoder::Stop() {
    return Thread::Stop();
}

int Decoder::Run() {
    AVFrame* frame = av_frame_alloc();
    while(Thread::EXIT != abort_) {
        AVPacket* pkt = pkt_queue_->front();
        if(!pkt) {
            continue;
        }
        int ret = avcodec_send_packet(codec_ctx_, pkt);
        if(0 == ret) {
            pkt = pkt_queue_->pop(10);
            av_packet_free(&pkt);
        } else {
            spdlog::error("avcodec_send_packet error, {}\n", av_err2str(ret));
            if(AVERROR(EAGAIN) != ret) {
                break;
            }
        }
        while(true) {
            ret = avcodec_receive_frame(codec_ctx_, frame);
            if(0 != ret) {
                if(AVERROR_EOF == ret) {
                    abort_ = Thread::EXIT;
                }
                spdlog::warn("avcodec_receive_frame error, {}\n", av_err2str(ret));
                break;
            }
            frame_queue_->push(frame);
        }
    }
    av_frame_free(&frame);
    spdlog::info("Decoder::run() finished \n");
    abort_ = Thread::EXIT;
    return 0;
}