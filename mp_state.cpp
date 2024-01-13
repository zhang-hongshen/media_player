//
// Created by 张鸿燊 on 13/1/2024.
//

extern "C" {
#include <libavutil/common.h>
}

#include "mp_state.h"

MPState::MPState() {

}

MPState::~MPState() {

}

void MPState::SetClock(double pts) {
    sync.SetClock(pts);
}

double MPState::GetClock() {
    return sync.GetClock();
}

void MPState::TogglePaused() {
    paused = !paused;
}

void MPState::ToggleMuted() {
    muted = !muted;
}

void MPState::UpdateVolume(int step) {
    volume = av_clip(volume + step, 0, SDL_MIX_MAXVOLUME);
}