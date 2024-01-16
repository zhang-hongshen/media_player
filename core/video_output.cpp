//
// Created by 张鸿燊 on 9/1/2024.
//
#include <spdlog/spdlog.h>

#include "video_output.h"

VideoOutput::VideoOutput(const std::shared_ptr<FrameQueue>& q, const VideoParam &param,
                         const std::shared_ptr<MPState>& mp_state):
 frame_queue_(q), param_(param), mp_state_(mp_state){

}

VideoOutput::~VideoOutput() {
    Realease();
}

void VideoOutput::Realease() {
    if(window) {
        SDL_DestroyWindow(window);
    }
    if(texture) {
        SDL_DestroyTexture(texture);
    }
    if(renderer) {
        SDL_DestroyRenderer(renderer);
    }
}


int VideoOutput::Init() {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        spdlog::error("Couldn't initialize SDL, {}\n", SDL_GetError());
        return -1;
    }
    std::string platform = SDL_GetPlatform();
    std::transform(platform.begin(), platform.end(), platform.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    spdlog::debug("platform: {}\n", platform);

    Uint32 window_flags = SDL_WINDOW_RESIZABLE;
    if(platform.find("mac") != std::string::npos ) {
        window_flags |= SDL_WINDOW_METAL | SDL_WINDOW_ALLOW_HIGHDPI;
    } else {
        window_flags |= SDL_WINDOW_OPENGL;
    }

    window = SDL_CreateWindow("ffmpeg player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              param_.width, param_.height,  window_flags);
    if(!window) {
        spdlog::error("SDL_CreateWindow error, {}\n" ,SDL_GetError());
        return -1;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(!renderer) {
        spdlog::error("SDL_CreateRenderer error, {}\n" ,SDL_GetError());
        return -1;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
                                param_.width, param_.height);
    if(!texture) {
        spdlog::error("SDL_CreateTexture error, {}\n" ,SDL_GetError());
        return -1;
    }
    return 0;
}

void VideoOutput::EventLoop() {
    SDL_Event event;
    while(true) {
        RefreshLoopWaitEvent(&event);
        switch (event.type) {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_m:
                        mp_state_->ToggleMuted();
                        break;
                    case SDLK_UP:
                        mp_state_->volume += MPState::DEFAULT_VOLUME_STEP;
                        break;
                    case SDLK_DOWN:
                        mp_state_->volume -= MPState::DEFAULT_VOLUME_STEP;
                        break;
                    case SDLK_SPACE:
                        mp_state_->TogglePaused();
                        break;
                    case SDLK_LEFT:
                        mp_state_->PrePlayBackSpeed();
                        break;
                    case SDLK_RIGHT:
                        mp_state_->NextPlayBackSpeed();
                        break;
                    case SDLK_a:
                        mp_state_->Seek(mp_state_->GetClock() - MPState::DEFAULT_SEEK_STEP);
                        spdlog::debug("mp_state_->seek_pts, {}\n" ,mp_state_->seek_pts);
                        break;
                    case SDLK_d:
                        mp_state_->Seek(mp_state_->GetClock() + MPState::DEFAULT_SEEK_STEP);
                        spdlog::debug("mp_state_->seek_pts, {}\n" ,mp_state_->seek_pts);
                        break;
                    case SDLK_q:
                        Realease();
                        SDL_Quit();
                        break;
                }
                break;
            case SDL_QUIT:
                SDL_Quit();
                break;
        }
    }
}


void VideoOutput::RefreshLoopWaitEvent(SDL_Event *event) {
    double remaining_time = 0;
    SDL_PumpEvents();
    while(!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        if(remaining_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds (int64_t(remaining_time * 1000)));
        }
        remaining_time = REFRESH_RATE;
        if(!mp_state_->paused) {
            auto frame = frame_queue_->front();
            if(!CheckIfNeedRefresh(frame)) {
                continue;
            }
            AVFrame *av_frame = frame->av_frame_;
            double diff = av_frame->pts * av_q2d(param_.time_base) - mp_state_->GetClock();
            if(diff > 0) {
                remaining_time = std::min(remaining_time, diff);
                continue;
            }
            int ret = Refresh(av_frame);
            if(0 != ret) {
                spdlog::error("Refresh error\n");
                return;
            }
            frame_queue_->pop(10);
        }
        SDL_PumpEvents();
    }
}

bool VideoOutput::CheckIfNeedRefresh(const std::shared_ptr<Frame>& frame) const {
    if(!frame) {
        return false;
    }
    if(frame->serial_ != mp_state_->serial) {
        spdlog::info("video frame serial not equal, expected {}, actual {}\n", mp_state_->serial, frame->serial_);
        return false;
    }
    return true;
}


int VideoOutput::Refresh(const AVFrame *frame) {
    int ret = SDL_UpdateYUVTexture(texture, nullptr,
                                   frame->data[0], frame->linesize[0],
                                   frame->data[1], frame->linesize[1],
                                   frame->data[2], frame->linesize[2]);
    if(0 != ret) {
        spdlog::error("SDL_UpdateYUVTexture error, {}\n" ,SDL_GetError());
        return -1;
    }
    SDL_RenderClear(renderer);
    int renderer_width, renderer_height;
    SDL_GetRendererOutputSize(renderer, &renderer_width, &renderer_height);
    float scale = std::min(static_cast<float>(renderer_width) / static_cast<float>(param_.width),
                           static_cast<float>(renderer_height) / static_cast<float>(param_.height));
    rect.w = static_cast<int>(static_cast<float>(param_.width) * scale);
    rect.h = static_cast<int>(static_cast<float>(param_.height) * scale);
    rect.x = (renderer_width - rect.w) >> 1;
    rect.y = (renderer_height - rect.h) >> 1;

    ret = SDL_RenderCopy(renderer, texture, nullptr, &rect);
    if(0 != ret) {
        spdlog::error("SDL_RenderCopy error, {}\n" ,SDL_GetError());
        return -1;
    }
    SDL_RenderPresent(renderer);
    return 0;
}






