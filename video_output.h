//
// Created by 张鸿燊 on 9/1/2024.
//

#ifndef SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H
#define SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H

extern "C"{
#include "SDL.h"
};

#include "avframe_queue.h"
#include "avsync.h"

class VideoOutput {
public:
    VideoOutput(std::shared_ptr<AVSync> sync, AVRational time_base, std::shared_ptr<AVFrameQueue> q, int video_width, int video_height);
    ~VideoOutput();
    int Init();
    int MainLoop();
private:
    void RefreshLoopWaitEvent(SDL_Event *event);
    void Refresh(double *remainTime);
    int Render(const AVFrame* frame);
private:
    std::shared_ptr<AVSync> sync_;
    AVRational time_base_;
    std::shared_ptr<AVFrameQueue> frame_queue_ = nullptr;
    int video_width_ = 0;
    int video_height_ = 0;

    SDL_Rect rect;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_Texture *texture = nullptr;
    // video default refresh rate
    constexpr static double REFRESH_RATE = 1 / 60;

};


#endif //SIMPLEST_MEDIA_PLAYER_VIDEOOUTPUT_H
