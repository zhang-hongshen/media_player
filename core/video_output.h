//
// Created by 张鸿燊 on 9/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H
#define SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H

extern "C"{
#include <SDL.h>
};

#include "queue.h"
#include "frame.h"
#include "avsync.h"
#include "mp_state.h"

struct VideoParam {
    int width;
    int height;
    AVRational time_base;
};

class VideoOutput {
public:
    VideoOutput(const std::shared_ptr<Queue<std::shared_ptr<Frame>>> & q, const VideoParam &param,
                const std::shared_ptr<MPState>& mp_state);
    ~VideoOutput() noexcept;
    int Init();
    void EventLoop();
private:
    int RefreshLoopWaitEvent(SDL_Event *event);
    bool CheckIfNeedRefresh(const std::shared_ptr<Frame>& frame) const;
    int Refresh(const AVFrame* frame);
    void Realease();
private:
    std::shared_ptr<Queue<std::shared_ptr<Frame>>> frame_queue_ = nullptr;
    VideoParam param_;
    std::shared_ptr<MPState> mp_state_ = nullptr;

    SDL_Rect rect;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    // video default refresh rate
    constexpr static double REFRESH_RATE = 0.01;
};


#endif //SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H
