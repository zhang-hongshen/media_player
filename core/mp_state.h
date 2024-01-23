//
// Created by 张鸿燊 on 13/1/2024.
//

#ifndef MEDIA_PLAYER_MP_STATE_H
#define MEDIA_PLAYER_MP_STATE_H

#include <memory>

extern "C" {
#include <SDL.h>
};

#include "packet.h"
#include "frame.h"
#include "avsync.h"

class MPState {
public:
    MPState();
    ~MPState() noexcept;
    void SetClock(double pts);
    double GetClock();
    void TogglePaused();
    void ToggleMuted();
    void NextPlayBackSpeed();
    void PrePlayBackSpeed();
    double GetPlayBackSpeed();
    void Seek(double pts);
public:
    bool paused = false;
    bool muted = false;
    int volume = SDL_MIX_MAXVOLUME >> 1;
    constexpr static int DEFAULT_VOLUME_STEP = SDL_MIX_MAXVOLUME / 16;

    bool need_seek = false;
    int serial = 0;
    double seek_pts = 0;
    constexpr static int DEFAULT_SEEK_STEP = 10;
private:
    int playback_speed_index = 2;
    double playback_speeds[5] = {0.5, 0.75, 1.0, 1.5, 2.0};
    std::unique_ptr<AVSync> sync = nullptr;
};


#endif //MEDIA_PLAYER_MP_STATE_H
