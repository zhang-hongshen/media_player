//
// Created by 张鸿燊 on 13/1/2024.
//

#ifndef MEDIA_PLAYER_MP_STATE_H
#define MEDIA_PLAYER_MP_STATE_H

extern "C" {
#include <SDL.h>
};

#include "avsync.h"
class MPState {
public:
    MPState();
    ~MPState();
    void SetClock(double pts);
    double GetClock();
    void TogglePaused();
    void ToggleMuted();
    void UpdateVolume(int step = DEFAULT_VOLUME_STEP);
public:
    bool paused = false;
    bool muted = false;
    int volume = SDL_MIX_MAXVOLUME;
    constexpr static int DEFAULT_VOLUME_STEP = SDL_MIX_MAXVOLUME / 16;
private:
    AVSync sync;
};


#endif //MEDIA_PLAYER_MP_STATE_H
