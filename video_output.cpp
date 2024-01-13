//
// Created by 张鸿燊 on 9/1/2024.
//
#include <spdlog/spdlog.h>

#include "video_output.h"

VideoOutput::VideoOutput(std::shared_ptr<AVFrameQueue> q, const VideoParam &param, std::shared_ptr<MPState> mp_state):
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
        spdlog::debug("mac\n", platform);
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
                        mp_state_->UpdateVolume(MPState::DEFAULT_VOLUME_STEP);
                        break;
                    case SDLK_DOWN:
                        mp_state_->UpdateVolume(-MPState::DEFAULT_VOLUME_STEP);
                        break;
                    case SDLK_SPACE:
                        mp_state_->TogglePaused();
                        break;
                    case SDLK_q:
                        Realease();
                        SDL_Quit();
                        exit(0);
                        break;
                }
                break;
            case SDL_QUIT:
                break;
        }
    }
}


void VideoOutput::RefreshLoopWaitEvent(SDL_Event *event) {
    double remainTime = 0;
    SDL_PumpEvents();
    while(!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)) {
        if(remainTime > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds (int64_t(remainTime * 1000)));
        }
        remainTime = REFRESH_RATE;
        if(!mp_state_->paused) {
            Refresh(&remainTime);
        }
        SDL_PumpEvents();
    }
}

void VideoOutput::Refresh(double *remainTime) {
    AVFrame *frame = frame_queue_->front();
    if(!frame) {
        return;
    }
    double diff = frame->pts * av_q2d(param_.time_base) - mp_state_->GetClock();
    if(diff > 0) {
        *remainTime = std::min(*remainTime, diff);
        return;
    }

    int ret = Render(frame);
    if(0 != ret) {
        spdlog::error("Render error\n");
        return;
    }
    frame = frame_queue_->pop(10);
    av_frame_free(&frame);
}

int VideoOutput::Render(const AVFrame *frame) {
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






