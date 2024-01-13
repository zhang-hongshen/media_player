//
// Created by 张鸿燊 on 9/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H
#define SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H

extern "C"{
#include <SDL.h>
};

#include "avframe_queue.h"
#include "avsync.h"
#include "mp_state.h"

struct VideoParam {
    int width;
    int height;
    AVRational time_base;
};

class VideoOutput {
public:
    VideoOutput(std::shared_ptr<AVFrameQueue> q, const VideoParam &param, std::shared_ptr<MPState> mp_state);
    ~VideoOutput();
    int Init();
    void EventLoop();
private:
    void RefreshLoopWaitEvent(SDL_Event *event);
    void Refresh(double *remainTime);
    int Render(const AVFrame* frame);
    void Realease();
private:
    std::shared_ptr<AVFrameQueue> frame_queue_ = nullptr;
    VideoParam param_;
    std::shared_ptr<MPState> mp_state_;

    SDL_Rect rect;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    // video default refresh rate
    constexpr static double REFRESH_RATE = 0.01;

    void TogglePause();
};


#endif //SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H
